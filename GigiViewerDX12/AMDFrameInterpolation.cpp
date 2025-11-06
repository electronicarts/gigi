///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "AMDFrameInterpolation.h"

#include "Nodes/nodes.h"
#include "Interpreter/GigiInterpreterPreviewWindowDX12.h"
#include "Shared/HashAll.h"
#include "Shared/ffx_utils.h"

#include "api/include/dx12/ffx_api_dx12.hpp"
#include "framegeneration/include/ffx_framegeneration.hpp"
#include "framegeneration/include/dx12/ffx_api_framegeneration_dx12.hpp"

#include "DX12Utils/Camera.h"
#include "DX12Utils/ShaderCompilerCommon.h"
#include "DX12Utils/CompileShaders.h"

#include "CopyResizeTextureHLSL.h"

static void SetFfxApiResourceToTexture(ID3D12Resource* texture, FfxApiResource& dest, unsigned int additionalUses, D3D12_RESOURCE_STATES state)
{
    if (!texture)
        return;

    D3D12_RESOURCE_DESC desc = texture->GetDesc();

    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D  && desc.DepthOrArraySize > 1)
        additionalUses |= FFX_API_RESOURCE_USAGE_ARRAYVIEW;

    dest = ffxApiGetResourceDX12(texture, D3D12State_To_FfxState(state), additionalUses);
}

struct DelayDestroyFrameGenContext
{
    void* m_context = nullptr;
    int m_age = 0;
};

struct State
{
    bool m_failState = false;

    std::vector<AMDFrameInterpolation::Version> m_versionInfo;

    DXGI_FORMAT m_swapChainFormat = DXGI_FORMAT_UNKNOWN;
    void* m_swapChainContext = nullptr;

    void* m_FrameGenContext = nullptr;
    size_t m_FrameGenContextHash = 0;

    std::vector<DelayDestroyFrameGenContext> m_delayDestroyFrameGenContexts;

    ID3D12Resource* m_motionVectors = nullptr;
    ID3D12Resource* m_depth = nullptr;

    // for resize texture shader
    ID3D12RootSignature* m_rootSignature = nullptr;
    ID3D12PipelineState* m_pso = nullptr;
};

static State s_state;

static bool CompileResizeTextureShader(const GigiInterpreterPreviewWindowDX12& interpreter, ID3D12Device14* device)
{
    // Make the root signature
    {
        // samplers
        D3D12_STATIC_SAMPLER_DESC samplers[1] = {};
        samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        samplers[0].AddressU = samplers[0].AddressV = samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].MipLODBias = 0.0f;
        samplers[0].MaxAnisotropy = 0;
        samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        samplers[0].MinLOD = 0.0f;
        samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
        samplers[0].ShaderRegister = 0;
        samplers[0].RegisterSpace = 0;
        samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // Descriptor table
        // t0 - input srv
        // u0 - output uav
        // b0 - constant cbv
        D3D12_DESCRIPTOR_RANGE ranges[3] = {};

        ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        ranges[0].NumDescriptors = 1;
        ranges[0].BaseShaderRegister = 0;
        ranges[0].RegisterSpace = 0;
        ranges[0].OffsetInDescriptorsFromTableStart = 0;

        ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        ranges[1].NumDescriptors = 1;
        ranges[1].BaseShaderRegister = 0;
        ranges[1].RegisterSpace = 0;
        ranges[1].OffsetInDescriptorsFromTableStart = 1;

        ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        ranges[2].NumDescriptors = 1;
        ranges[2].BaseShaderRegister = 0;
        ranges[2].RegisterSpace = 0;
        ranges[2].OffsetInDescriptorsFromTableStart = 2;

        // Root parameter
        D3D12_ROOT_PARAMETER rootParam;
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParam.DescriptorTable.NumDescriptorRanges = 3;
        rootParam.DescriptorTable.pDescriptorRanges = ranges;

        D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
        rootDesc.NumParameters = 1;
        rootDesc.pParameters = &rootParam;
        rootDesc.NumStaticSamplers = 1;
        rootDesc.pStaticSamplers = samplers;
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        // Create it
        ID3DBlob* sig = nullptr;
        ID3DBlob* error = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
        if (FAILED(hr))
        {
            const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
            if (errorMsg)
                interpreter.GetLogFn()(LogLevel::Error, "Could not serialize root signature: %s", errorMsg);
            if (sig) sig->Release();
            if (error) error->Release();
            return false;
        }

        hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&s_state.m_rootSignature));
        if (FAILED(hr))
        {
            const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
            if (errorMsg)
                interpreter.GetLogFn()(LogLevel::Error, "Could not create root signature: %s", errorMsg);
            if (sig) sig->Release();
            if (error) error->Release();
            return false;
        }

        if (sig)
            sig->Release();

        if (error)
            error->Release();

        s_state.m_rootSignature->SetName(L"" __FUNCTION__);
    }

    // Make the PSO
    {
        std::string fullFileName = (std::filesystem::path(interpreter.GetTempDirectory()) / "__GIGI__" / "ResizeTexture.hlsl").string();

        // write the shader to a temporary file, since we don't have an interface to compile a string yet
        std::filesystem::create_directories(std::filesystem::path(fullFileName).remove_filename());
        FILE* file = nullptr;
        fopen_s(&file, fullFileName.c_str(), "wb");
        if (!file)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not open file for writing: \"%s\"", fullFileName.c_str());
            return false;
        }
        fwrite(s_CopyResizeTextureHLSL, 1, strlen(s_CopyResizeTextureHLSL), file);
        fclose(file);

        // Shader compilation depends on which shader compiler they opted to use for this technique
        ShaderCompilationInfo shaderCompilationInfo;
        shaderCompilationInfo.fileName = fullFileName;
        shaderCompilationInfo.entryPoint = "main";
        shaderCompilationInfo.shaderModel = "cs_6_1";
        shaderCompilationInfo.debugName = __FUNCTION__;
        //shaderCompilationInfo.defines

        shaderCompilationInfo.flags |= ShaderCompilationFlags::WarningsAsErrors;

        // Turn on for debugging
        if (false)
        {
            shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.flags |= ShaderCompilationFlags::CreatePDBsAndBinaries;
        }

        MakeComputePSO_dxc(
            device,
            shaderCompilationInfo,
            s_state.m_rootSignature,
            &s_state.m_pso,
            interpreter.GetLogFn()
        );

        if (!s_state.m_pso)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not create PSO in " __FUNCTION__);
            return false;
        }

        s_state.m_pso->SetName(L"" __FUNCTION__);
    }

    return true;
}

namespace AMDFrameInterpolation
{

const std::vector<Version>& GetVersions(ID3D12Device14* device)
{
    return s_state.m_versionInfo;
}

void Init(const GigiInterpreterPreviewWindowDX12& interpreter, ID3D12Device14* device, IDXGISwapChain4*& swapChain, ID3D12CommandQueue* commandQueue)
{
    if (!CompileResizeTextureShader(interpreter, device))
    {
        interpreter.GetLogFn()(LogLevel::Error, "CompileResizeTextureShader() failed in " __FUNCTION__ "\n");
        return;
    }

    // Get the versions available
    {
        std::vector<uint64_t> FsrVersionIds;
        std::vector<const char*> FsrVersionNames;

        // Get how many versions there are
        ffxQueryDescGetVersions versionQuery = { 0 };
        versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
        versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATION;
        versionQuery.device = device;
        uint64_t versionCount = 0;
        versionQuery.outputCount = &versionCount;
        ffxReturnCode_t retCode_t = ffxQuery(nullptr, &versionQuery.header);

        // Get those version
        FsrVersionIds.resize(versionCount);
        FsrVersionNames.resize(versionCount);
        versionQuery.versionIds = FsrVersionIds.data();
        versionQuery.versionNames = FsrVersionNames.data();
        retCode_t = ffxQuery(nullptr, &versionQuery.header);

        // Copy versions into the cached version info
        s_state.m_versionInfo.resize(versionCount + 1);
        for (size_t versionIndex = 0; versionIndex < versionCount; ++versionIndex)
        {
            s_state.m_versionInfo[versionIndex + 1].versionId = FsrVersionIds[versionIndex];
            s_state.m_versionInfo[versionIndex + 1].versionName = FsrVersionNames[versionIndex];
        }
        s_state.m_versionInfo[0].versionId = 0;
        s_state.m_versionInfo[0].versionName = "default";
    }

    // Fill out the information using the old swap chain
    ffx::CreateContextDescFrameGenerationSwapChainForHwndDX12 createSwapChainDesc{};
    swapChain->GetHwnd(&createSwapChainDesc.hwnd);
    DXGI_SWAP_CHAIN_DESC1 desc1;
    swapChain->GetDesc1(&desc1);
    createSwapChainDesc.desc = &desc1;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
    swapChain->GetFullscreenDesc(&fullscreenDesc);
    createSwapChainDesc.fullscreenDesc = &fullscreenDesc;
    swapChain->GetParent(IID_PPV_ARGS(&createSwapChainDesc.dxgiFactory));

    // Release the old swap chain
    swapChain->Release();
    swapChain = nullptr;

    // Create the new swap chain
    createSwapChainDesc.gameQueue = commandQueue;
    createSwapChainDesc.swapchain = &swapChain;
    ffx::ReturnCode retCode = ffx::CreateContext(s_state.m_swapChainContext, nullptr, createSwapChainDesc);
    if (retCode != ffx::ReturnCode::Ok)
        interpreter.GetLogFn()(LogLevel::Error, "Could not create swap chain context in " __FUNCTION__ "\n");

    // Clean up
    createSwapChainDesc.dxgiFactory->Release();

    // Store useful data off
    s_state.m_swapChainFormat = desc1.Format;
}

void Release()
{
    for (DelayDestroyFrameGenContext& context : s_state.m_delayDestroyFrameGenContexts)
        ffx::DestroyContext(context.m_context);
    s_state.m_delayDestroyFrameGenContexts.clear();

    if (s_state.m_FrameGenContext)
    {
        ffx::DestroyContext(s_state.m_FrameGenContext);
        s_state.m_FrameGenContext = nullptr;
    }

    if (s_state.m_swapChainContext != nullptr)
    {
        ffx::DestroyContext(s_state.m_swapChainContext);
        s_state.m_swapChainContext = nullptr;
    }

    if (s_state.m_depth)
    {
        s_state.m_depth->Release();
        s_state.m_depth = nullptr;
    }

    if (s_state.m_motionVectors)
    {
        s_state.m_motionVectors->Release();
        s_state.m_motionVectors = nullptr;
    }

    if (s_state.m_rootSignature)
    {
        s_state.m_rootSignature->Release();
        s_state.m_rootSignature = nullptr;
    }

    if (s_state.m_pso)
    {
        s_state.m_pso->Release();
        s_state.m_pso = nullptr;
    }
}

void Tick(const GGUserFile_AMD_FidelityFXSDK_FrameInterpolation& settings, GigiInterpreterPreviewWindowDX12& interpreter, IDXGISwapChain4*& swapChain, const GGUserFile_Camera& cameraSettings, const Camera& cameraState, const Desc& desc)
{
    BasicPixScopeProfiler _p(desc.commandList, "AMD Frame Interpolation");

    if (s_state.m_swapChainContext == nullptr || s_state.m_failState)
        return;

    if (s_state.m_rootSignature == nullptr || s_state.m_pso == nullptr)
        return;

    const RenderGraph& renderGraph = interpreter.GetRenderGraph();

    #define HANDLE_TEXTURE(NAME) \
        int nodeIndex_##NAME = FrontEndNodesNoCaching::GetNodeIndexByName(renderGraph, settings.##NAME.c_str()); \
        bool textureExists_##NAME = false; \
        const RuntimeTypes::RenderGraphNode_Resource_Texture& texture_##NAME = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(nodeIndex_##NAME >= 0 ? settings.##NAME.c_str() : "", textureExists_##NAME); \
        textureExists_##NAME = textureExists_##NAME && texture_##NAME.m_resource != nullptr; \
        const RenderGraphNode_Resource_Texture* node_##NAME = nodeIndex_##NAME >= 0 ? &renderGraph.nodes[nodeIndex_##NAME].resourceTexture : nullptr;

    HANDLE_TEXTURE(depth);
    HANDLE_TEXTURE(motionVectors);

    if (!textureExists_depth || !textureExists_motionVectors)
        return;

    #undef HANDLE_TEXTURE

    // Input and output cannot be zero sized
    DXGI_SWAP_CHAIN_DESC1 desc1;
    swapChain->GetDesc1(&desc1);
    unsigned int renderSize[3] = { desc1.Width, desc1.Height, 1 };
    if (renderSize[0] == 0 || renderSize[1] == 0)
        return;

    // (Re)make motion and depth textures as needed
    {
        if (s_state.m_depth)
        {
            D3D12_RESOURCE_DESC existingDesc = s_state.m_depth->GetDesc();
            if (existingDesc.Format != texture_depth.m_format || existingDesc.Width != renderSize[0] || existingDesc.Height != renderSize[1])
            {
                interpreter.getDelayedReleaseTracker().Add(s_state.m_depth);
                s_state.m_depth = nullptr;
            }
        }

        if (s_state.m_motionVectors)
        {
            D3D12_RESOURCE_DESC existingDesc = s_state.m_motionVectors->GetDesc();
            if (existingDesc.Format != texture_motionVectors.m_format || existingDesc.Width != renderSize[0] || existingDesc.Height != renderSize[1])
            {
                interpreter.getDelayedReleaseTracker().Add(s_state.m_motionVectors);
                s_state.m_motionVectors = nullptr;
            }
        }

        if (!s_state.m_depth)
            s_state.m_depth = CreateTexture(desc.device, renderSize, 1, texture_depth.m_format, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, ResourceType::Texture2D, "Depth - AMD Frame Interpolation");

        if (!s_state.m_motionVectors)
            s_state.m_motionVectors = CreateTexture(desc.device, renderSize, 1, texture_motionVectors.m_format, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, ResourceType::Texture2D, "Motion - AMD Frame Interpolation");
    }

    // Make full screen depth and motion vectors by pasting the input ones into them, handling scaling and clipping
    {
        TransitionTracker& transitions = interpreter.GetTransitionsNonConst();

        // Depth
        {
            // Make a constant buffer
            CopyResizeTexture_CBStruct cb;
            memcpy(cb.imagePosition, desc.imagePosition, sizeof(desc.imagePosition));
            memcpy(cb.imageSize, desc.imageSize, sizeof(desc.imageSize));
            memcpy(cb.imageClipMin, desc.imageClipMin, sizeof(desc.imageClipMin));
            memcpy(cb.imageClipMax, desc.imageClipMax, sizeof(desc.imageClipMax));

            UploadBufferTracker::Buffer* cbBuffer = interpreter.getUploadBufferTracker().GetBufferT(desc.device, true, cb);

            // Descriptor table
            // t0 - input srv
            // u0 - output uav
            // b0 - constant cbv
            DescriptorTableCache::ResourceDescriptor descriptors[3] = {};

            D3D12_RESOURCE_DESC inputDesc = texture_depth.m_resource->GetDesc();
            D3D12_RESOURCE_DESC outputDesc = s_state.m_depth->GetDesc();

            descriptors[0].m_resource = texture_depth.m_resource;
            descriptors[0].m_format = inputDesc.Format;
            descriptors[0].m_access = DescriptorTableCache::AccessType::SRV;
            descriptors[0].m_resourceType = DescriptorTableCache::ResourceType::Texture2D;

            descriptors[1].m_resource = s_state.m_depth;
            descriptors[1].m_format = s_state.m_depth->GetDesc().Format;
            descriptors[1].m_access = DescriptorTableCache::AccessType::UAV;
            descriptors[1].m_resourceType = DescriptorTableCache::ResourceType::Texture2D;

            descriptors[2].m_resource = cbBuffer->buffer;
            descriptors[2].m_format = DXGI_FORMAT_UNKNOWN;
            descriptors[2].m_access = DescriptorTableCache::AccessType::CBV;
            descriptors[2].m_stride = (UINT)cbBuffer->size;
            descriptors[2].m_count = 1;

            // Do transitions before making descriptor table
            transitions.Transition(TRANSITION_DEBUG_INFO(texture_depth.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
            transitions.Flush(desc.commandList);

            // Make the descriptor table
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable;
            std::string error;
            if (!interpreter.getDescriptorTableCache().GetDescriptorTable(desc.device, interpreter.getSRVHeapAllocationTracker(), descriptors, _countof(descriptors), descriptorTable, error, HEAP_DEBUG_TEXT()))
            {
                interpreter.GetLogFn()(LogLevel::Error, "Could not allocate a descriptor table in " __FUNCTION__ ": %s", error.c_str());
                return;
            }

            // set the root signature, PSO and descriptor table
            desc.commandList->SetComputeRootSignature(s_state.m_rootSignature);
            desc.commandList->SetPipelineState(s_state.m_pso);
            desc.commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            // dispatch
            unsigned int dispatchX = (unsigned int)(outputDesc.Width + 7) / 8;
            unsigned int dispatchY = (outputDesc.Height + 7) / 8;
            desc.commandList->Dispatch(dispatchX, dispatchY, 1);
        }

        // Motion Vectors
        {
            // Make a constant buffer
            CopyResizeTexture_CBStruct cb;
            memcpy(cb.imagePosition, desc.imagePosition, sizeof(desc.imagePosition));
            memcpy(cb.imageSize, desc.imageSize, sizeof(desc.imageSize));
            memcpy(cb.imageClipMin, desc.imageClipMin, sizeof(desc.imageClipMin));
            memcpy(cb.imageClipMax, desc.imageClipMax, sizeof(desc.imageClipMax));

            UploadBufferTracker::Buffer* cbBuffer = interpreter.getUploadBufferTracker().GetBufferT(desc.device, true, cb);

            // Descriptor table
            // t0 - input srv
            // u0 - output uav
            // b0 - constant cbv
            DescriptorTableCache::ResourceDescriptor descriptors[3] = {};

            D3D12_RESOURCE_DESC inputDesc = texture_motionVectors.m_resource->GetDesc();
            D3D12_RESOURCE_DESC outputDesc = s_state.m_motionVectors->GetDesc();

            descriptors[0].m_resource = texture_motionVectors.m_resource;
            descriptors[0].m_format = inputDesc.Format;
            descriptors[0].m_access = DescriptorTableCache::AccessType::SRV;
            descriptors[0].m_resourceType = DescriptorTableCache::ResourceType::Texture2D;

            descriptors[1].m_resource = s_state.m_motionVectors;
            descriptors[1].m_format = s_state.m_motionVectors->GetDesc().Format;
            descriptors[1].m_access = DescriptorTableCache::AccessType::UAV;
            descriptors[1].m_resourceType = DescriptorTableCache::ResourceType::Texture2D;

            descriptors[2].m_resource = cbBuffer->buffer;
            descriptors[2].m_format = DXGI_FORMAT_UNKNOWN;
            descriptors[2].m_access = DescriptorTableCache::AccessType::CBV;
            descriptors[2].m_stride = (UINT)cbBuffer->size;
            descriptors[2].m_count = 1;

            // Do transitions before making descriptor table
            transitions.Transition(TRANSITION_DEBUG_INFO(texture_motionVectors.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
            transitions.Flush(desc.commandList);

            // Make the descriptor table
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable;
            std::string error;
            if (!interpreter.getDescriptorTableCache().GetDescriptorTable(desc.device, interpreter.getSRVHeapAllocationTracker(), descriptors, _countof(descriptors), descriptorTable, error, HEAP_DEBUG_TEXT()))
            {
                interpreter.GetLogFn()(LogLevel::Error, "Could not allocate a descriptor table in " __FUNCTION__ ": %s", error.c_str());
                return;
            }

            // set the root signature, PSO and descriptor table
            desc.commandList->SetComputeRootSignature(s_state.m_rootSignature);
            desc.commandList->SetPipelineState(s_state.m_pso);
            desc.commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            // dispatch
            unsigned int dispatchX = (unsigned int)(outputDesc.Width + 7) / 8;
            unsigned int dispatchY = (outputDesc.Height + 7) / 8;
            desc.commandList->Dispatch(dispatchX, dispatchY, 1);
        }
    }
    // Fill out the frame generation context desc
    ffx::CreateContextDescFrameGeneration createFgDesc = {};
    createFgDesc.displaySize = { renderSize[0], renderSize[1] };
    createFgDesc.maxRenderSize = { renderSize[0], renderSize[1] };
    createFgDesc.flags = 0;
    createFgDesc.flags |= settings.ENABLE_ASYNC_WORKLOAD_SUPPORT ? FFX_FRAMEGENERATION_ENABLE_ASYNC_WORKLOAD_SUPPORT : 0;
    createFgDesc.flags |= settings.ENABLE_MOTION_VECTORS_JITTER_CANCELLATION ? FFX_FRAMEGENERATION_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION : 0;
    createFgDesc.flags |= (cameraSettings.reverseZ || cameraSettings.reverseZInfiniteDepth) ? FFX_FRAMEGENERATION_ENABLE_DEPTH_INVERTED : 0;
    createFgDesc.flags |= cameraSettings.reverseZInfiniteDepth ? FFX_FRAMEGENERATION_ENABLE_DEPTH_INFINITE : 0;
    createFgDesc.flags |= settings.ENABLE_HIGH_DYNAMIC_RANGE ? FFX_FRAMEGENERATION_ENABLE_HIGH_DYNAMIC_RANGE : 0;
    createFgDesc.flags |= settings.ENABLE_DEBUG_CHECKING ? FFX_FRAMEGENERATION_ENABLE_DEBUG_CHECKING : 0;
    createFgDesc.backBufferFormat = ffxApiGetSurfaceFormatDX12(s_state.m_swapChainFormat);
    if (createFgDesc.backBufferFormat == FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB)
        createFgDesc.backBufferFormat = FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM;

    // Fill out the frame generation configuration desc
    ffx::ConfigureDescFrameGeneration frameGenerationConfigDesc = {};
    frameGenerationConfigDesc.flags = 0;
    frameGenerationConfigDesc.flags |= settings.DRAW_DEBUG_TEAR_LINES ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_TEAR_LINES : 0;
    frameGenerationConfigDesc.flags |= settings.DRAW_DEBUG_RESET_INDICATORS ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_RESET_INDICATORS : 0;
    frameGenerationConfigDesc.flags |= settings.DRAW_DEBUG_VIEW ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW : 0;
    frameGenerationConfigDesc.flags |= settings.DRAW_DEBUG_PACING_LINES ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_PACING_LINES : 0;

    // If the frame generation context needs to be recreated, put the old one in a delayed destruction list
    size_t createFgDescHash = HashAll(settings.version, createFgDesc.flags, createFgDesc.displaySize.width, createFgDesc.displaySize.height, createFgDesc.maxRenderSize.width, createFgDesc.maxRenderSize.height, createFgDesc.backBufferFormat);
    if (s_state.m_FrameGenContext != nullptr && createFgDescHash != s_state.m_FrameGenContextHash)
    {
        s_state.m_delayDestroyFrameGenContexts.push_back({ s_state.m_FrameGenContext, 0 });
        s_state.m_FrameGenContext = nullptr;
    }

    // Make a frame generation context if we need to
    if (!s_state.m_FrameGenContext)
    {
        s_state.m_FrameGenContextHash = createFgDescHash;

        ffx::CreateBackendDX12Desc backendDesc = {};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDesc.device = desc.device;

        // Get RAM usage for settings
        FfxApiEffectMemoryUsage gpuMemoryUsageFrameGenerationV2;
        ffx::QueryDescFrameGenerationGetGPUMemoryUsageV2 frameGenGetGPUMemoryUsageV2{};
        frameGenGetGPUMemoryUsageV2.device = desc.device;
        frameGenGetGPUMemoryUsageV2.maxRenderSize = createFgDesc.maxRenderSize;
        frameGenGetGPUMemoryUsageV2.displaySize = createFgDesc.displaySize;
        frameGenGetGPUMemoryUsageV2.createFlags = createFgDesc.flags;
        frameGenGetGPUMemoryUsageV2.dispatchFlags = frameGenerationConfigDesc.flags;
        frameGenGetGPUMemoryUsageV2.backBufferFormat = createFgDesc.backBufferFormat;
        frameGenGetGPUMemoryUsageV2.gpuMemoryUsageFrameGeneration = &gpuMemoryUsageFrameGenerationV2;
        frameGenGetGPUMemoryUsageV2.hudlessBackBufferFormat = FFX_API_SURFACE_FORMAT_UNKNOWN;
        ffx::ReturnCode retCode = ffx::Query(frameGenGetGPUMemoryUsageV2);
        if (retCode != ffx::ReturnCode::Ok)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not query memory usage in " __FUNCTION__ "\n");
            s_state.m_failState = true;
            return;
        }

        // Get the override version
        uint64_t overrideVersionId = 0;
        for (const Version& versionInfo : s_state.m_versionInfo)
        {
            if (versionInfo.versionName == settings.version)
            {
                overrideVersionId = versionInfo.versionId;
                break;
            }
        }

        // Make the override version
        ffxOverrideVersion versionOverride = { 0 };
        if (overrideVersionId != 0)
        {
            versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
            versionOverride.versionId = overrideVersionId;
            frameGenGetGPUMemoryUsageV2.header.pNext = &versionOverride.header;
        }

        if (overrideVersionId != 0)
            retCode = ffx::CreateContext(s_state.m_FrameGenContext, nullptr, createFgDesc, backendDesc, versionOverride);
        else
            retCode = ffx::CreateContext(s_state.m_FrameGenContext, nullptr, createFgDesc, backendDesc);
        if (retCode != ffx::ReturnCode::Ok)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not create frame generation context in " __FUNCTION__ "\n");
            s_state.m_failState = true;
            return;
        }

        // Get the version created
        ffxQueryGetProviderVersion getVersion = { 0 };
        getVersion.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
        ffxReturnCode_t retCode_t = ffxQuery(&s_state.m_FrameGenContext, &getVersion.header);
        if (retCode_t != FFX_API_RETURN_OK)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not query frame generation context version in " __FUNCTION__ "\n");
            s_state.m_failState = true;
            return;
        }

        // Report details of the success
        interpreter.GetLogFn()(LogLevel::Info, "Initialized AMD FidelityFXSDK frame generation context\nversionid 0x%016llx, %s\ntotalUsageInBytes %f MB aliasableUsageInBytes %f MB", getVersion.versionId, getVersion.versionName, gpuMemoryUsageFrameGenerationV2.totalUsageInBytes / 1048576.f, gpuMemoryUsageFrameGenerationV2.aliasableUsageInBytes / 1048576.f);
    }

    // Configure frame generation
    {
        frameGenerationConfigDesc.swapChain = swapChain;

        // A UI composition callback to call when finalizing the frame image.
        frameGenerationConfigDesc.presentCallback = nullptr;
        frameGenerationConfigDesc.presentCallbackUserContext = nullptr;

        // The frame generation callback to use to generate a frame.
        frameGenerationConfigDesc.frameGenerationCallback = [](ffxDispatchDescFrameGeneration* params, void* pUserCtx) ->ffxReturnCode_t
            {
                // params->presentColor is the back buffer of the swap chain
                // params->outputs[0] is the interpolation output
                return ffxDispatch(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
            }
        ;
        frameGenerationConfigDesc.frameGenerationCallbackUserContext = &s_state.m_FrameGenContext;

        frameGenerationConfigDesc.frameGenerationEnabled = settings.enabled;
        frameGenerationConfigDesc.allowAsyncWorkloads = settings.allowAsyncWorkloads;
        frameGenerationConfigDesc.onlyPresentGenerated = settings.onlyPresentGenerated;

        frameGenerationConfigDesc.generationRect.left = 0;
        frameGenerationConfigDesc.generationRect.top = 0;
        frameGenerationConfigDesc.generationRect.width = renderSize[0];
        frameGenerationConfigDesc.generationRect.height = renderSize[1];

        // Note: Setting the generation rect to just the visible portion of the viewed image makes the quality a lot worse. I'm not sure why.
        if (settings.constrainToRectangle)
        {
            frameGenerationConfigDesc.generationRect.left = desc.imageClipMin[0];
            frameGenerationConfigDesc.generationRect.top = desc.imageClipMin[1];
            frameGenerationConfigDesc.generationRect.width = min(desc.imageSize[0], desc.imageClipMax[0] - desc.imageClipMin[0]);
            frameGenerationConfigDesc.generationRect.height = min(desc.imageSize[1], desc.imageClipMax[1] - desc.imageClipMin[1]);
        }

        frameGenerationConfigDesc.frameID = desc.frameIndex;

        ffx::ReturnCode retCode = ffx::Configure(s_state.m_FrameGenContext, frameGenerationConfigDesc);
        if (retCode != ffx::ReturnCode::Ok)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not configure frame generation in " __FUNCTION__ "\n");
            s_state.m_failState = true;
            return;
        }
    }

    // Dispatch frame generation
    {
        ffx::DispatchDescFrameGenerationPrepare dispatchFgPrep{};

        dispatchFgPrep.frameID = desc.frameIndex;

        dispatchFgPrep.flags = frameGenerationConfigDesc.flags;
        dispatchFgPrep.commandList = desc.commandList;
        dispatchFgPrep.renderSize = { renderSize[0], renderSize[1] };

        // Handle camera jitter vectors
        {
            dispatchFgPrep.jitterOffset.x = desc.jitterOffset[0];
            dispatchFgPrep.jitterOffset.y = desc.jitterOffset[1];

            // This can happen on reload sometimes
            if (isnan(dispatchFgPrep.jitterOffset.x) || isnan(dispatchFgPrep.jitterOffset.y))
            {
                interpreter.GetLogFn()(LogLevel::Warn, "nan jitter offset component in " __FUNCTION__ "\n");
                // Jitter shouldn't ever be zero
                dispatchFgPrep.jitterOffset.x = 0.1f;
                dispatchFgPrep.jitterOffset.y = 0.1f;
            }
            // Jitter shouldn't ever be zero
            else if (dispatchFgPrep.jitterOffset.x == 0.0f && dispatchFgPrep.jitterOffset.y == 0.0f)
            {
                interpreter.GetLogFn()(LogLevel::Warn, "zero jitter offset component in " __FUNCTION__ "\n");
                dispatchFgPrep.jitterOffset.x = 0.2f;
                dispatchFgPrep.jitterOffset.y = 0.2f;
            }
            dispatchFgPrep.jitterOffset.y *= -1.0f;
        }

        dispatchFgPrep.motionVectorScale.x = 1.0f;
        dispatchFgPrep.motionVectorScale.y = 1.0f;

        // TODO: should we add an option to let user specify motion vector scale? maybe as a variable? or is it ok that we assume motion vectors are in UV?
        dispatchFgPrep.motionVectorScale.x = (float)desc.imageSize[0];
        dispatchFgPrep.motionVectorScale.y = (float)desc.imageSize[1];

        dispatchFgPrep.frameTimeDelta = desc.frameTimeMS;

        // Near and far plane
        dispatchFgPrep.cameraNear = cameraSettings.nearPlane;
        dispatchFgPrep.cameraFar = cameraSettings.farPlane;
        if (cameraSettings.reverseZ)
        {
            if (cameraSettings.reverseZInfiniteDepth)
                dispatchFgPrep.cameraNear = FLT_MAX;
            else
                std::swap(dispatchFgPrep.cameraNear, dispatchFgPrep.cameraFar);
        }

        dispatchFgPrep.cameraFovAngleVertical = cameraSettings.FOV * 3.14159265359f / 180.0f;

        SetFfxApiResourceToTexture(s_state.m_depth, dispatchFgPrep.depth, FFX_API_RESOURCE_USAGE_UAV, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        SetFfxApiResourceToTexture(s_state.m_motionVectors, dispatchFgPrep.motionVectors, FFX_API_RESOURCE_USAGE_UAV, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        ffx::DispatchDescFrameGenerationPrepareCameraInfo cameraConfig{};
        memcpy(&cameraConfig.cameraPosition, cameraState.m_lastPos, sizeof(float) * 3);

        DirectX::XMMATRIX viewMtx = cameraState.GetViewMatrix(cameraState.m_lastPos, cameraState.m_lastAltitudeAzimuth);
        const float cameraUp[3] = {viewMtx.r[0].m128_f32[1], viewMtx.r[1].m128_f32[1], viewMtx.r[2].m128_f32[1]};
        const float cameraLeft[3] = {viewMtx.r[0].m128_f32[0] , viewMtx.r[1].m128_f32[0] , viewMtx.r[2].m128_f32[0]};
        const float cameraFwd[3] = {viewMtx.r[0].m128_f32[2], viewMtx.r[1].m128_f32[2], viewMtx.r[2].m128_f32[2]};
        memcpy(&cameraConfig.cameraUp, cameraUp, sizeof(float) * 3);
        memcpy(&cameraConfig.cameraForward, cameraFwd, sizeof(float) * 3);

        // flip cameraRight since we expose it as cameraLeft
        memcpy(&cameraConfig.cameraRight, cameraLeft, sizeof(float) * 3);
        cameraConfig.cameraRight[0] *= -1.0f;
        cameraConfig.cameraRight[1] *= -1.0f;
        cameraConfig.cameraRight[2] *= -1.0f;

        ffx::ReturnCode retCode = ffx::Dispatch(s_state.m_FrameGenContext, dispatchFgPrep, cameraConfig);
        if (retCode != ffx::ReturnCode::Ok)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not dispatch frame generation in " __FUNCTION__ "\n");
            s_state.m_failState = true;
            return;
        }
    }

    // Restore the descriptor heaps
    interpreter.SetDescriptorHeaps();

    // Handle delayed destruction of frame gen contexts
    auto it = std::remove_if(s_state.m_delayDestroyFrameGenContexts.begin(), s_state.m_delayDestroyFrameGenContexts.end(),
        [](auto& context) {
            context.m_age++;
            if (context.m_age > 10)
            {
                ffx::DestroyContext(context.m_context);
                return true;
            }
            return false;
        });
    s_state.m_delayDestroyFrameGenContexts.erase(it, s_state.m_delayDestroyFrameGenContexts.end());
}

} //namespace AMDFrameInterpolation
