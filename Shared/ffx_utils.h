///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "api/include/ffx_api_types.h"

static inline FfxApiResourceState D3D12State_To_FfxState(D3D12_RESOURCE_STATES state)
{
    switch (state)
    {
        case D3D12_RESOURCE_STATE_COMMON: return FFX_API_RESOURCE_STATE_COMMON;
        case D3D12_RESOURCE_STATE_UNORDERED_ACCESS: return FFX_API_RESOURCE_STATE_UNORDERED_ACCESS;
        case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE: return FFX_API_RESOURCE_STATE_COMPUTE_READ;
        case D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE: return FFX_API_RESOURCE_STATE_PIXEL_READ;
        case D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE: return FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ;
        case D3D12_RESOURCE_STATE_COPY_SOURCE: return FFX_API_RESOURCE_STATE_COPY_SRC;
        case D3D12_RESOURCE_STATE_COPY_DEST: return FFX_API_RESOURCE_STATE_COPY_DEST;
        case D3D12_RESOURCE_STATE_GENERIC_READ: return FFX_API_RESOURCE_STATE_GENERIC_READ;
        case D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT: return FFX_API_RESOURCE_STATE_INDIRECT_ARGUMENT;
        //case D3D12_RESOURCE_STATE_PRESENT: return FFX_API_RESOURCE_STATE_PRESENT;
        case D3D12_RESOURCE_STATE_RENDER_TARGET: return FFX_API_RESOURCE_STATE_RENDER_TARGET;
        case D3D12_RESOURCE_STATE_DEPTH_WRITE: return FFX_API_RESOURCE_STATE_DEPTH_ATTACHMENT;
        default:
        {
            Assert(false, "Unhandled D3D12_RESOURCE_STATES in " __FUNCTION__);
            return FFX_API_RESOURCE_STATE_COMMON;
        }
    }
}