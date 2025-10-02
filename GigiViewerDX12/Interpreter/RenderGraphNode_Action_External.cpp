///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"

bool OnExternalNodeAction_AMD_FidelityFXSDK_Upscaling(const RenderGraphNode_Action_External& node, RuntimeTypes::RenderGraphNode_Action_External::AMD_FidelityFXSDK_Upscaling& runtimeData, NodeAction nodeAction, ID3D12Device* device, LogFn& logFn)
{
    const ExternalNode_AMD_FidelityFXSDK_Upscaling& nodeData = node.externalNodeData.AMD_FidelityFXSDK_Upscaling;

    static bool doThis = false;
    //if (nodeAction == NodeAction::Init)
    if (doThis)
    {

        {
            std::vector<uint64_t> m_FsrVersionIds;
            std::vector<const char*> m_FsrVersionNames;

            struct ffxQueryDescGetVersions versionQuery = { 0 };
            versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
            ffxReturnCode_t retCode_t;

            versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
            versionQuery.device = device;

            uint64_t versionCount = 0;
            versionQuery.outputCount = &versionCount;
            retCode_t = ffxQuery(nullptr, &versionQuery.header);
            //CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
            //    L"ffxQuery(nullptr,GetVersionsUpscaleCount) returned %d", retCode_t);

            m_FsrVersionIds.resize(versionCount);
            m_FsrVersionNames.resize(versionCount);
            versionQuery.versionIds = m_FsrVersionIds.data();
            versionQuery.versionNames = m_FsrVersionNames.data();
            retCode_t = ffxQuery(nullptr, &versionQuery.header);
            //CauldronAssert(ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
            //    L"ffxQuery(nullptr,GetVersionsUpscaleIdsNames) returned %d", retCode_t);
            int ijkl = 0;
        }

        // TODO: may need to recreate if the max size changed or anything

        if (runtimeData.m_initialized)
            return true;
        runtimeData.m_initialized = true;

        // TODO: temp. query the available versions!
        {
            ffxQueryDescGetVersions versionQuery = { 0 };
            versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
            versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
            versionQuery.device = device;
            uint64_t versionCount = 0;
            versionQuery.outputCount = &versionCount;
            ffxReturnCode_t retCode_t = ffxQuery(nullptr, &versionQuery.header);

            std::vector<uint64_t> FsrVersionIds;
            std::vector<const char*> FsrVersionNames;
            FsrVersionIds.resize(versionCount);
            FsrVersionNames.resize(versionCount);
            versionQuery.versionIds = FsrVersionIds.data();
            versionQuery.versionNames = FsrVersionNames.data();
            retCode_t = ffxQuery(nullptr, &versionQuery.header);


            int ijkl = 0;
        }



        runtimeData.m_backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        runtimeData.m_backendDesc.device = device;

        // TODO: need to set the maxRenderSize and maxUpscaleSize
        // TODO: max render size should come from what is plugged in i think?
        // TODO: max up scale size should be a setting. should be a literal value, a variable, or a node that decides that.

        runtimeData.m_createFsrDesc.maxRenderSize = { nodeData.maxRenderSize[0], nodeData.maxRenderSize[1] };
        runtimeData.m_createFsrDesc.maxUpscaleSize = { nodeData.maxUpscaleSize[0], nodeData.maxUpscaleSize[1] };

        runtimeData.m_createFsrDesc.flags = 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_HIGH_DYNAMIC_RANGE ? FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS ? FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_MOTION_VECTORS_JITTER_CANCELLATION ? FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_DEPTH_INVERTED ? FFX_UPSCALE_ENABLE_DEPTH_INVERTED : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_DEPTH_INFINITE ? FFX_UPSCALE_ENABLE_DEPTH_INFINITE : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_AUTO_EXPOSURE ? FFX_UPSCALE_ENABLE_AUTO_EXPOSURE : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_DYNAMIC_RESOLUTION ? FFX_UPSCALE_ENABLE_DYNAMIC_RESOLUTION : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_DEBUG_CHECKING ? FFX_UPSCALE_ENABLE_DEBUG_CHECKING : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_NON_LINEAR_COLORSPACE ? FFX_UPSCALE_ENABLE_NON_LINEAR_COLORSPACE : 0;
        runtimeData.m_createFsrDesc.flags |= nodeData.ENABLE_DEBUG_VISUALIZATION ? FFX_UPSCALE_ENABLE_DEBUG_VISUALIZATION : 0;

        // TODO: make this work and log to the output window
        auto FFX_Log = [](uint32_t type, const wchar_t* message)
            {
                int ijkl = 0;
            };

        runtimeData.m_createFsrDesc.fpMessage = FFX_Log;

        //Before creating any of FSR contexts, query VRAM size
        {
            runtimeData.m_upscalerGetGPUMemoryUsageV2.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GPU_MEMORY_USAGE_V2;
            runtimeData.m_upscalerGetGPUMemoryUsageV2.device = device;

            runtimeData.m_upscalerGetGPUMemoryUsageV2.maxRenderSize = runtimeData.m_createFsrDesc.maxRenderSize;
            runtimeData.m_upscalerGetGPUMemoryUsageV2.maxUpscaleSize = runtimeData.m_createFsrDesc.maxUpscaleSize;
            runtimeData.m_upscalerGetGPUMemoryUsageV2.flags = runtimeData.m_createFsrDesc.flags;
            runtimeData.m_upscalerGetGPUMemoryUsageV2.gpuMemoryUsageUpscaler = &runtimeData.m_gpuMemoryUsageUpscaler;

            ffx::ReturnCode retCode = ffx::Query(runtimeData.m_upscalerGetGPUMemoryUsageV2);

            if (retCode != ffx::ReturnCode::Ok)
            {
                logFn(LogLevel::Error, "Could not query memory usage for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
                return false;
            }

            logFn(LogLevel::Info, "Default Upscaler Query GPUMemoryUsageV2 totalUsageInBytes %f MB aliasableUsageInBytes %f MB", runtimeData.m_gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, runtimeData.m_gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);
        }

        ffx::ReturnCode retCode = ffx::CreateContext(runtimeData.m_UpscalingContext, nullptr, runtimeData.m_createFsrDesc, runtimeData.m_backendDesc);

        if (retCode != ffx::ReturnCode::Ok)
        {
            logFn(LogLevel::Error, "Could not create upscaling context for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            return false;
        }

        runtimeData.m_getVersion.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
        ffxReturnCode_t retCode_t = ffxQuery(&runtimeData.m_UpscalingContext, &runtimeData.m_getVersion.header);

        if (retCode_t != FFX_API_RETURN_OK)
        {
            logFn(LogLevel::Error, "Could not query upscaling context version for node \"%s\" in " __FUNCTION__ "\n", node.name.c_str());
            return false;
        }

        logFn(LogLevel::Info, "Initialized upscaling context for node \"%s\", versionid 0x%016llx, %S", runtimeData.m_getVersion.versionId, runtimeData.m_getVersion.versionName);
        return true;
    }

    // TODO: implement execute

    return true;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_External& node, RuntimeTypes::RenderGraphNode_Action_External& runtimeData, NodeAction nodeAction)
{
    switch (node.externalNodeData._index)
    {
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case ExternalNodeData::c_index_##_NAME: return OnExternalNodeAction_##_NAME(node, runtimeData.m_##_NAME, nodeAction, m_device, m_logFn);
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/ExternalNodeVariant.h"

        default:
        {
            return false;
        }
    }
}

/*
TODO: have the editor show only the variant actually used.  Maybe it already works that way, try it out.
TODO: do i have to destroy the context at some point? yes! fsrapirendermodule.cpp does it on resize and shutdown. ffx::DestroyContext
TODO: frame generation external node too
*/