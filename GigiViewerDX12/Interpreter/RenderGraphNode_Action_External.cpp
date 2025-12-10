///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"

#include "api/include/dx12/ffx_api_dx12.hpp"
#include "upscalers/include/ffx_upscale.hpp"
#include "framegeneration/include/ffx_framegeneration.hpp"
#include "framegeneration/include/dx12/ffx_api_framegeneration_dx12.hpp"
#include "Shared/HashAll.h"
#include "Shared/ffx_utils.h"

static void SetLogFunction(ffxApiMessage& messageFunc, GigiInterpreterPreviewWindowDX12* interpreter)
{
    static auto LogMessage = [](uint32_t type, const wchar_t* message)
        {
            GigiInterpreterPreviewWindowDX12::GetLogFn()(LogLevel::Warn, "FidelityFX: %s", FromWideString(message).c_str());
        }
    ;
    messageFunc = LogMessage;
}

static void PublishTexture(const RenderGraphNode_Action_External& node, RuntimeTypes::RenderGraphNode_Action_External& runtimeData, GigiInterpreterPreviewWindowDX12& interpreter, const RenderGraphNode_Resource_Texture* textureNode, const RuntimeTypes::RenderGraphNode_Resource_Texture& texture, const char* textureName, const char* extraLabel, bool resultOfWrite)
{
    std::string label = node.name + std::string(".") + std::string(textureName) + std::string(": ") + textureNode->name + std::string(extraLabel);
    runtimeData.HandleViewableTexture(interpreter, TextureDimensionTypeToViewableResourceType(textureNode->dimension), label.c_str(), texture.m_resource, texture.m_format, texture.m_size, texture.m_numMips, false, resultOfWrite);
}

// Get the texture nodes and make sure the required ones exist
static const RenderGraphNode_Resource_Texture* GetTextureResourceNode(int resourceNodeIndex, const std::vector<RenderGraphNode>& nodes)
{
    if (resourceNodeIndex < 0)
        return nullptr;

    if (nodes[resourceNodeIndex]._index != RenderGraphNode::c_index_resourceTexture)
        return nullptr;

    return &nodes[resourceNodeIndex].resourceTexture;
}

static void SetFfxApiResourceToTexture(const RuntimeTypes::RenderGraphNode_Resource_Texture& src, const RenderGraphNode_Resource_Texture* srcNode, FfxApiResource& dest, TransitionTracker& transitions)
{
    if (!srcNode || !src.m_resource)
        return;

    uint32_t additionalUses = 0;

    additionalUses |= ((srcNode->accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget)) ? FFX_API_RESOURCE_USAGE_RENDERTARGET : 0);
    additionalUses |= ((srcNode->accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV)) ? FFX_API_RESOURCE_USAGE_UAV : 0);
    additionalUses |= ((srcNode->accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget)) ? FFX_API_RESOURCE_USAGE_DEPTHTARGET : 0);
    additionalUses |= ((srcNode->accessedAs & (1 << (unsigned int)ShaderResourceAccessType::Indirect)) ? FFX_API_RESOURCE_USAGE_INDIRECT : 0);

    if (srcNode->dimension == TextureDimensionType::Texture2DArray || srcNode->dimension == TextureDimensionType::TextureCube)
        additionalUses |= FFX_API_RESOURCE_USAGE_ARRAYVIEW;

    if (srcNode->accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget) && Get_DXGI_FORMAT_Info(src.m_format).isStencil)
        additionalUses |= FFX_API_RESOURCE_USAGE_STENCILTARGET;

    D3D12_RESOURCE_STATES dxState = transitions.GetCurrentState(src.m_resource);
    dest = ffxApiGetResourceDX12(src.m_resource, D3D12State_To_FfxState(dxState), additionalUses);
}

void RuntimeTypes::RenderGraphNode_Action_External::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
    // m_AMD_FidelityFXSDK_Upscaling
    {
        for (auto& it : m_AMD_FidelityFXSDK_Upscaling.m_contexts)
        {
            if (it.m_UpscalingContext)
            {
                ffx::DestroyContext(it.m_UpscalingContext);
                it.m_UpscalingContext = nullptr;
            }
        }
        m_AMD_FidelityFXSDK_Upscaling.m_contexts.clear();
    }
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction_External_AMD_FidelityFXSDK_Upscaling(const RenderGraphNode_Action_External& node, RuntimeTypes::RenderGraphNode_Action_External& runtimeData_, NodeAction nodeAction)
{
    if (nodeAction == NodeAction::Init)
        return true;

    if (!runtimeData_.m_conditionIsTrue)
        return true;

    RuntimeTypes::RenderGraphNode_Action_External::AMD_FidelityFXSDK_Upscaling& runtimeData = runtimeData_.m_AMD_FidelityFXSDK_Upscaling;
    const ExternalNode_AMD_FidelityFXSDK_Upscaling& nodeData = node.externalNodeData.AMD_FidelityFXSDK_Upscaling;

    auto GetContext = [&runtimeData](const ffx::CreateContextDescUpscale& desc, ExternalNode_AMD_FidelityFXSDK_Upscaling_Version desiredVersion) -> RuntimeTypes::RenderGraphNode_Action_External::AMD_FidelityFXSDK_Upscaling::Context&
        {
            // Use an existing context if it exists
            size_t createFsrDescHash = HashAll(desc.flags, desiredVersion);
            for (auto& context : runtimeData.m_contexts)
            {
                if (context.m_UpscalingContextHash == createFsrDescHash &&
                    context.m_maxRenderSize[0] >= desc.maxRenderSize.width && context.m_maxRenderSize[1] >= desc.maxRenderSize.height &&
                    context.m_maxUpscaleSize[0] >= desc.maxUpscaleSize.width && context.m_maxUpscaleSize[1] >= desc.maxUpscaleSize.height)
                {
                    context.m_age = 0;
                    return context;
                }
            }

            // Otherwise create and return a new context
            RuntimeTypes::RenderGraphNode_Action_External::AMD_FidelityFXSDK_Upscaling::Context newContext;
            newContext.m_UpscalingContextHash = createFsrDescHash;
            newContext.m_maxRenderSize[0] = desc.maxRenderSize.width;
            newContext.m_maxRenderSize[1] = desc.maxRenderSize.height;
            newContext.m_maxUpscaleSize[0] = desc.maxUpscaleSize.width;
            newContext.m_maxUpscaleSize[1] = desc.maxUpscaleSize.height;
            runtimeData.m_contexts.push_back(newContext);
            return *runtimeData.m_contexts.rbegin();
        }
    ;

    #define HandleTexture(NAME, extraLabel) \
        const RenderGraphNode_Resource_Texture* node_##NAME = GetTextureResourceNode(nodeData.##NAME.resourceNodeIndex, m_renderGraph.nodes); \
        bool textureExists_##NAME = false; \
        RuntimeTypes::RenderGraphNode_Resource_Texture& texture_##NAME = GetRuntimeNodeData_RenderGraphNode_Resource_Texture( node_##NAME ? node_##NAME->name.c_str() : "", textureExists_##NAME); \
        if (textureExists_##NAME) \
            PublishTexture(node, runtimeData_, *this, node_##NAME, texture_##NAME, #NAME, extraLabel, false);

    HandleTexture(color, " (SRV)")
    HandleTexture(colorOpaqueOnly, " (SRV)")
    HandleTexture(depth, " (SRV)")
    HandleTexture(motionVectors, " (SRV)")
    HandleTexture(exposure, " (SRV)")
    HandleTexture(reactive, " (UAV - Before)")
    //HandleTexture(transparencyAndComposition, " (SRV)")
    HandleTexture(output, " (UAV - Before)")

    #undef HandleTexture

    if (!textureExists_color || !textureExists_depth || !textureExists_motionVectors || !textureExists_output)
        return true;

    // Input and output cannot be zero sized
    uint32_t renderSize[2] = { (uint32_t)texture_color.m_size[0], (uint32_t)texture_color.m_size[1] };
    uint32_t upscaleSize[2] = { (uint32_t)texture_output.m_size[0], (uint32_t)texture_output.m_size[1] };
    if (renderSize[0] == 0 || renderSize[1] == 0 || upscaleSize[0] == 0 || upscaleSize[1] == 0)
        return true;

    ffx::CreateContextDescUpscale createFsrDesc = {};
    createFsrDesc.maxRenderSize = { max(renderSize[0], nodeData.initialMaxRenderSize[0]), max(renderSize[1], nodeData.initialMaxRenderSize[1]) };
    createFsrDesc.maxUpscaleSize = { max(upscaleSize[0], nodeData.initialMaxUpscaleSize[0]), max(upscaleSize[1], nodeData.initialMaxUpscaleSize[1]) };
    createFsrDesc.flags = 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_HIGH_DYNAMIC_RANGE.variable.variableIndex) ? FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS.variable.variableIndex) ? FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_MOTION_VECTORS_JITTER_CANCELLATION.variable.variableIndex) ? FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DEPTH_INVERTED.variable.variableIndex) ? FFX_UPSCALE_ENABLE_DEPTH_INVERTED : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DEPTH_INFINITE.variable.variableIndex) ? FFX_UPSCALE_ENABLE_DEPTH_INFINITE : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_AUTO_EXPOSURE.variable.variableIndex) ? FFX_UPSCALE_ENABLE_AUTO_EXPOSURE : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DYNAMIC_RESOLUTION.variable.variableIndex) ? FFX_UPSCALE_ENABLE_DYNAMIC_RESOLUTION : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DEBUG_CHECKING.variable.variableIndex) ? FFX_UPSCALE_ENABLE_DEBUG_CHECKING : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_NON_LINEAR_COLORSPACE.variable.variableIndex) ? FFX_UPSCALE_ENABLE_NON_LINEAR_COLORSPACE : 0;
    createFsrDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DEBUG_VISUALIZATION.variable.variableIndex) ? FFX_UPSCALE_ENABLE_DEBUG_VISUALIZATION : 0;
    SetLogFunction(createFsrDesc.fpMessage, this);

    ExternalNode_AMD_FidelityFXSDK_Upscaling_Version desiredVersion = (ExternalNode_AMD_FidelityFXSDK_Upscaling_Version)GetRuntimeVariableValueAllowCast_NoFail<int>(nodeData.version.variable.variableIndex);
    auto& context = GetContext(createFsrDesc, desiredVersion);

    // Create the upscaling context if it doesn't exist
    if (context.m_UpscalingContext == nullptr)
    {
        // Get and show available versions
        std::vector<uint64_t> FsrVersionIds;
        std::vector<const char*> FsrVersionNames;
        {
            ffxQueryDescGetVersions versionQuery = { 0 };
            versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
            versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
            versionQuery.device = m_device;
            uint64_t versionCount = 0;
            versionQuery.outputCount = &versionCount;
            ffxReturnCode_t retCode_t = ffxQuery(nullptr, &versionQuery.header);

            FsrVersionIds.resize(versionCount);
            FsrVersionNames.resize(versionCount);
            versionQuery.versionIds = FsrVersionIds.data();
            versionQuery.versionNames = FsrVersionNames.data();
            retCode_t = ffxQuery(nullptr, &versionQuery.header);

            m_logFn(LogLevel::Info, "Initializing FSR. Versions available:");
            for (uint64_t versionIndex = 0; versionIndex < versionCount; ++versionIndex)
                m_logFn(LogLevel::Info, "    %s (0x%016llx)", FsrVersionNames[versionIndex], FsrVersionIds[versionIndex]);
        }

        // If the have chosen a version, match it by string version
        uint64_t overrideVersionId = 0;
        {
            const char* matchString = nullptr;

            switch (desiredVersion)
            {
                case ExternalNode_AMD_FidelityFXSDK_Upscaling_Version::Default: break;
                case ExternalNode_AMD_FidelityFXSDK_Upscaling_Version::v2_3_4:
                {
                    matchString = "2.3.4";
                    break;
                }
                case ExternalNode_AMD_FidelityFXSDK_Upscaling_Version::v3_1_5:
                {
                    matchString = "3.1.5";
                    break;
                }
                default:
                {
                    m_logFn(LogLevel::Error, "Unhandled FSR version \"%s\"", EnumToString(desiredVersion));
                    return false;
                }
            }

            // get overrideVersionId
            if (matchString)
            {
                for (size_t versionIndex = 0; versionIndex < FsrVersionNames.size(); ++versionIndex)
                {
                    if (strcmp(FsrVersionNames[versionIndex], matchString) == 0)
                    {
                        overrideVersionId = FsrVersionIds[versionIndex];
                        break;
                    }
                }
                if (overrideVersionId == 0)
                {
                    m_logFn(LogLevel::Error, "Could not find requested FSR version \"%s\"", matchString);
                    return false;
                }
            }
        }

        ffx::CreateBackendDX12Desc backendDesc = {};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDesc.device = m_device;

        // Get RAM usage for settings
        FfxApiEffectMemoryUsage gpuMemoryUsageUpscaler = { 0 };
        ffxQueryDescUpscaleGetGPUMemoryUsageV2 upscalerGetGPUMemoryUsageV2 = {};
        upscalerGetGPUMemoryUsageV2.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GPU_MEMORY_USAGE_V2;
        upscalerGetGPUMemoryUsageV2.device = m_device;
        upscalerGetGPUMemoryUsageV2.maxRenderSize = createFsrDesc.maxRenderSize;
        upscalerGetGPUMemoryUsageV2.maxUpscaleSize = createFsrDesc.maxUpscaleSize;
        upscalerGetGPUMemoryUsageV2.flags = createFsrDesc.flags;
        upscalerGetGPUMemoryUsageV2.gpuMemoryUsageUpscaler = &gpuMemoryUsageUpscaler;

        // Apply version override if we should
        ffxOverrideVersion versionOverride = { 0 };
        if (overrideVersionId != 0)
        {
            versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
            versionOverride.versionId = overrideVersionId;
            upscalerGetGPUMemoryUsageV2.header.pNext = &versionOverride.header;
        }

        ffx::ReturnCode retCode = ffx::Query(upscalerGetGPUMemoryUsageV2);
        if (retCode != ffx::ReturnCode::Ok)
        {
            m_logFn(LogLevel::Error, "Could not query memory usage for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            return false;
        }

        // Create the context
        if (overrideVersionId != 0)
            retCode = ffx::CreateContext(context.m_UpscalingContext, nullptr, createFsrDesc, backendDesc, versionOverride);
        else
            retCode = ffx::CreateContext(context.m_UpscalingContext, nullptr, createFsrDesc, backendDesc);
        if (retCode != ffx::ReturnCode::Ok)
        {
            m_logFn(LogLevel::Error, "Could not create upscaling context for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            return false;
        }

        // Get the version created
        ffxQueryGetProviderVersion getVersion = {};
        getVersion.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
        ffxReturnCode_t retCode_t = ffxQuery(&context.m_UpscalingContext, &getVersion.header);
        if (retCode_t != FFX_API_RETURN_OK)
        {
            m_logFn(LogLevel::Error, "Could not query upscaling context version for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            return false;
        }

        // Report details of the success
        m_logFn(LogLevel::Info, "Initialized AMD FidelityFXSDK upscaling context for node \"%s\"\nversionid 0x%016llx, %s\ntotalUsageInBytes %0.2f MB aliasableUsageInBytes %0.2f MB", node.name.c_str(), getVersion.versionId, getVersion.versionName, gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);
    }

    // If using a reactive mask, make sure the texture is connected
    ExternalNode_AMD_FidelityFXSDK_Upscaling_GenerateReactiveMask_ReactiveMaskMode reactiveMaskMode = (ExternalNode_AMD_FidelityFXSDK_Upscaling_GenerateReactiveMask_ReactiveMaskMode)GetRuntimeVariableValueAllowCast_NoFail<int>(nodeData.reactiveMask.reactiveMaskMode.variable.variableIndex);
    if (reactiveMaskMode != ExternalNode_AMD_FidelityFXSDK_Upscaling_GenerateReactiveMask_ReactiveMaskMode::Off && !textureExists_reactive)
    {
        m_logFn(LogLevel::Error, "Node \"%s\" is set to use a reactive mask, but no texture is connected to the reactive pin.\n", node.name.c_str());
        return false;
    }

    // flush the transitions so we can get the current state of the textures
    m_transitions.Flush(m_commandList);

    // Generate a reactive mask if we should
    if (reactiveMaskMode == ExternalNode_AMD_FidelityFXSDK_Upscaling_GenerateReactiveMask_ReactiveMaskMode::Generate)
    {
        if (!textureExists_colorOpaqueOnly)
        {
            m_logFn(LogLevel::Error, "Node \"%s\" is set to generate a reactive mask, but no texture is connected to the colorOpaqueOnly pin.\n", node.name.c_str());
            return false;
        }

        ffx::DispatchDescUpscaleGenerateReactiveMask dispatchDesc{};
        dispatchDesc.commandList = m_commandList;

        SetFfxApiResourceToTexture(texture_colorOpaqueOnly, node_colorOpaqueOnly, dispatchDesc.colorOpaqueOnly, m_transitions);
        SetFfxApiResourceToTexture(texture_color, node_color, dispatchDesc.colorPreUpscale, m_transitions);
        SetFfxApiResourceToTexture(texture_reactive, node_reactive, dispatchDesc.outReactive, m_transitions);

        dispatchDesc.renderSize = { renderSize[0], renderSize[1] };

        dispatchDesc.scale = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.reactiveMask.scale.variable.variableIndex);
        dispatchDesc.cutoffThreshold = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.reactiveMask.cutoffThreshold.variable.variableIndex);
        dispatchDesc.binaryValue = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.reactiveMask.binaryValue.variable.variableIndex);

        dispatchDesc.flags = 0;
        dispatchDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.reactiveMask.APPLY_TONEMAP.variable.variableIndex) ? FFX_UPSCALE_AUTOREACTIVEFLAGS_APPLY_TONEMAP : 0;
        dispatchDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.reactiveMask.APPLY_INVERSETONEMAP.variable.variableIndex) ? FFX_UPSCALE_AUTOREACTIVEFLAGS_APPLY_INVERSETONEMAP : 0;
        dispatchDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.reactiveMask.APPLY_THRESHOLD.variable.variableIndex) ? FFX_UPSCALE_AUTOREACTIVEFLAGS_APPLY_THRESHOLD : 0;
        dispatchDesc.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.reactiveMask.USE_COMPONENTS_MAX.variable.variableIndex) ? FFX_UPSCALE_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX : 0;

        ffx::ReturnCode retCode = ffx::Dispatch(context.m_UpscalingContext, dispatchDesc);
        if (retCode != ffx::ReturnCode::Ok)
        {
            m_logFn(LogLevel::Error, "Could not dispatch generateing reactive mask for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            return false;
        }
    }

    // Execute the upscaling
    {
        ffx::DispatchDescUpscale dispatchUpscale{};
        dispatchUpscale.commandList = m_commandList;

        // Set textures
        SetFfxApiResourceToTexture(texture_color, node_color, dispatchUpscale.color, m_transitions);
        SetFfxApiResourceToTexture(texture_depth, node_depth, dispatchUpscale.depth, m_transitions);
        SetFfxApiResourceToTexture(texture_motionVectors, node_motionVectors, dispatchUpscale.motionVectors, m_transitions);
        SetFfxApiResourceToTexture(texture_exposure, node_exposure, dispatchUpscale.exposure, m_transitions);
        SetFfxApiResourceToTexture(texture_reactive, node_reactive, dispatchUpscale.reactive, m_transitions);
        //SetFfxApiResourceToTexture(texture_transparencyAndComposition, node_transparencyAndComposition, dispatchUpscale.transparencyAndComposition, m_transitions);
        SetFfxApiResourceToTexture(texture_output, node_output, dispatchUpscale.output, m_transitions);

        // Set Params
        GetRuntimeVariableValueAllowCast_NoFail(nodeData.jitterOffset.variable.variableIndex, &dispatchUpscale.jitterOffset.x, 2);

        // This can happen on reload sometimes
        if (isnan(dispatchUpscale.jitterOffset.x) || isnan(dispatchUpscale.jitterOffset.y))
        {
            m_logFn(LogLevel::Warn, "nan jitter offset component for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            // Jitter shouldn't ever be zero
            dispatchUpscale.jitterOffset.x = 0.1f;
            dispatchUpscale.jitterOffset.y = 0.1f;
        }
        // Jitter shouldn't ever be zero
        else if (dispatchUpscale.jitterOffset.x == 0.0f && dispatchUpscale.jitterOffset.y == 0.0f)
        {
            m_logFn(LogLevel::Warn, "zero jitter offset component for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            dispatchUpscale.jitterOffset.x = 0.2f;
            dispatchUpscale.jitterOffset.y = 0.2f;
        }

        dispatchUpscale.jitterOffset.y *= -1.0f;

        GetRuntimeVariableValueAllowCast_NoFail(nodeData.motionVectorScale.variable.variableIndex, &dispatchUpscale.motionVectorScale.x, 2);
        dispatchUpscale.renderSize = { renderSize[0], renderSize[1] };
        dispatchUpscale.upscaleSize = { upscaleSize[0], upscaleSize[1] };
        dispatchUpscale.enableSharpening = GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.enableSharpening.variable.variableIndex);
        dispatchUpscale.sharpness = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.sharpness.variable.variableIndex);
        dispatchUpscale.frameTimeDelta = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.frameTimeDelta.variable.variableIndex);
        dispatchUpscale.preExposure = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.preExposure.variable.variableIndex);

        dispatchUpscale.reset = false;
        if (nodeData.reset.variableIndex != -1)
            dispatchUpscale.reset = GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.reset.variableIndex);

        // Near and far plane
        dispatchUpscale.cameraNear = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.cameraNear.variable.variableIndex);
        dispatchUpscale.cameraFar = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.cameraFar.variable.variableIndex);
        if (GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DEPTH_INVERTED.variable.variableIndex))
        {
            if (GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.ENABLE_DEPTH_INFINITE.variable.variableIndex))
                dispatchUpscale.cameraNear = FLT_MAX;
            else
                std::swap(dispatchUpscale.cameraNear, dispatchUpscale.cameraFar);
        }

        dispatchUpscale.cameraFovAngleVertical = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.cameraFovAngleVertical.variable.variableIndex);
        dispatchUpscale.viewSpaceToMetersFactor = GetRuntimeVariableValueAllowCast_NoFail<float>(nodeData.viewSpaceToMetersFactor.variable.variableIndex);

        // Set flags
        dispatchUpscale.flags = 0;
        dispatchUpscale.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.DRAW_DEBUG_VIEW.variable.variableIndex) ? FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW : 0;
        dispatchUpscale.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.NON_LINEAR_COLOR_SRGB.variable.variableIndex) ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_SRGB : 0;
        dispatchUpscale.flags |= GetRuntimeVariableValueAllowCast_NoFail<bool>(nodeData.NON_LINEAR_COLOR_PQ.variable.variableIndex) ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_PQ : 0;

        // Dispatch
        ffx::ReturnCode retCode = ffx::Dispatch(context.m_UpscalingContext, dispatchUpscale);
        if (retCode != ffx::ReturnCode::Ok)
        {
            m_logFn(LogLevel::Error, "Could not dispatch upscaling for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            return false;
        }
    }

    // Restore the descriptor heaps
    SetDescriptorHeaps();

    // Publish the output
    PublishTexture(node, runtimeData_, *this, node_output, texture_output, "output", " (UAV - After)", true);
    if (textureExists_reactive)
        PublishTexture(node, runtimeData_, *this, node_reactive, texture_reactive, "reactive", " (UAV - After)", true);

    // Destroy any contexts that haven't been used for a while
    auto it = std::remove_if(runtimeData.m_contexts.begin(), runtimeData.m_contexts.end(),
        [this](auto& context) {
            context.m_age++;
            if (context.m_age > 10)
            {
                if (context.m_UpscalingContext)
                    ffx::DestroyContext(context.m_UpscalingContext);
                return true;
            }
            return false;
        });
    runtimeData.m_contexts.erase(it, runtimeData.m_contexts.end());

    return true;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_External& node, RuntimeTypes::RenderGraphNode_Action_External& runtimeData, NodeAction nodeAction)
{
    ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, true);

    bool executionConditionMet = EvaluateCondition(node.condition);
    runtimeData.m_conditionIsTrue = executionConditionMet;

    switch (node.externalNodeData._index)
    {
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case ExternalNodeData::c_index_##_NAME: return OnNodeAction_External_##_NAME(node, runtimeData, nodeAction);
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/ExternalNodeVariant.h"

        default:
        {
            GigiAssert(false, "Unknown External Node Type");
            return false;
        }
    }
}
