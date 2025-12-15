// This file is part of the FidelityFX SDK.
//
// Copyright (C) 2025 Advanced Micro Devices, Inc.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include "../../../api/include/xbox/ffx_api_xbox.h"

#if !defined(_GAMING_XBOX)
#pragma error "GDK backend is only meant for XBOX gaming targets"
#endif  // !defined(_GAMING_XBOX)

#if defined(_GAMING_XBOX_SCARLETT)

#include <d3d12_xs.h>
#include <d3dx12_xs.h>
#ifndef IID_GDK_PPV_ARGS
#define IID_GDK_PPV_ARGS(x) IID_GRAPHICS_PPV_ARGS(x)
#endif  // IID_GDK_PPV_ARGS

#elif defined(_GAMING_XBOX_XBOXONE)

#include <d3d12_x.h>
#include <d3dx12_x.h>
#ifndef IID_GDK_PPV_ARGS
#define IID_GDK_PPV_ARGS(x) IID_GRAPHICS_PPV_ARGS(x)
#endif  // IID_GDK_PPV_ARGS

#else
#pragma error "Unsupported XBOX gaming target"
#endif  // #ifdef FFX_API_GDK_SCARLETT

#define FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_VERSION_MAJOR 3
#define FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_VERSION_MINOR 1
#define FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_VERSION_PATCH 5

#define FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))
#define FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_VERSION FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_MAKE_VERSION(FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_VERSION_MAJOR, FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_VERSION_MINOR, FFX_FRAMEGENERATION_SWAPACHAIN_XBOX_VERSION_PATCH)

/// A structure encapsulating the data sent to the FfxPresentXFunc callback
typedef struct FfxPresentXParams
{
    ID3D12CommandQueue*            presentQueue;                ///< The ID3D12CommandQueue to call PresentX on
    D3D12XBOX_FRAME_PIPELINE_TOKEN framePipelineTokenOriginal;  ///< The D3D12XBOX_FRAME_PIPELINE_TOKEN for which ffxPresentX was called
    D3D12XBOX_FRAME_PIPELINE_TOKEN framePipelineTokenToSubmit;  ///< The D3D12XBOX_FRAME_PIPELINE_TOKEN which must be passed into PresentX
    ID3D12Resource*                sceneBackBufferToPresent;    ///< The ID3D12Resource representing scene back buffer with optional UI that must be passed into PresentX
    ID3D12Resource*                uiBackBufferToPresent;       ///< The optional ID3D12Resource that contains UI (in case sceneBackBufferToPresent doesn't contain UI) that must be passed into PresentX
    void*                          presentContext;              ///< A pointer to be passed to the PfnFfxPresentXFunc callback
    bool                           interpolatedFrame;           ///< Whether this frame was generated or not
} FfxPresentXParams;

/// PresentX callback function.
/// The callback function to be used when FrameInterpolationSwapChain is ready to call PresentX().
typedef void (*PfnFfxPresentXFunc)(FfxPresentXParams* ffxPresentXParams);

#define FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_XBOX FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x01)
struct ffxCreateContextDescFrameGenerationSwapChainXbox
{
    ffxCreateContextDescHeader header;
    ID3D12CommandQueue*        gameQueue;       ///< Input command queue to be used for game queue workloads.
    ID3D12CommandQueue*        computeQueue;    ///< Input command queue to be used for async compute workloads.
    ID3D12CommandQueue*        presentQueue;    ///< Input command queue to be used for presentation.
    PfnFfxPresentXFunc         pfnFfxPresentX;  ///< The PfnFfxPresentXFunc callback to be used for swapchain presentation
    void*                      presentXContext; ///< A pointer to be passed to the PfnFfxPresentXFunc callback
};

#define FFX_API_DISPATCH_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_PRESENT_XBOX FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x02)
struct ffxDispatchDescFrameGenerationSwapChainPresentXbox
{
    ffxDispatchDescHeader           header;
    D3D12XBOX_FRAME_PIPELINE_TOKEN  realBackBufferToken;
    struct FfxApiResource           realBackBufferPlane;
    struct FfxApiResource           interpolatedBackBufferPlane;
    struct FfxApiResource           uiBackBufferPlane;
};

#define FFX_API_DISPATCH_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_WAIT_FOR_PRESENTS_XBOX FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x03)
struct ffxDispatchDescFrameGenerationSwapChainWaitForPresentXbox
{
    ffxDispatchDescHeader header;
};

#define FFX_API_QUERY_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_INTERPOLATIONCOMMANDLIST_XBOX FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x04)
struct ffxQueryDescFrameGenerationSwapChainInterpolationCommandListXbox
{
    ffxQueryDescHeader header;
    void**             pOutCommandList;  ///< Output command list (ID3D12GraphicsCommandList) to be used for frame generation dispatch.
};

#define FFX_API_QUERY_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_INTERPOLATIONTEXTURE_XBOX FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x05)
struct ffxQueryDescFrameGenerationSwapChainInterpolationTextureXbox
{
    ffxQueryDescHeader     header;
    struct FfxApiResource* pOutTexture;  ///< Output resource in which the frame interpolation result should be placed.
};

#define FFX_API_QUERY_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_CONTEXT_XBOX FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x06)
struct ffxQueryDescFrameGenerationSwapChainContextXbox
{
    ffxQueryDescHeader              header;
    void*                           ffxSwapchain;
};

#define FFX_API_QUERY_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_GPU_MEMORY_USAGE_XBOX FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x07)
struct ffxQueryFrameGenerationSwapChainGetGPUMemoryUsageXbox
{
    ffxQueryDescHeader header;
    struct FfxApiEffectMemoryUsage* gpuMemoryUsageFrameGenerationSwapchain;
};

#define FFX_API_QUERY_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_GPU_MEMORY_USAGE_XBOX_V2 FFX_API_MAKE_BACKEND_EFFECT_SUB_ID(FFX_API_BACKEND_ID_XBOX, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, 0x08)
struct ffxQueryFrameGenerationSwapChainGetGPUMemoryUsageXboxV2
{
    ffxQueryDescHeader header;
    void* device;                            ///< For DX12: pointer to ID3D12Device. App needs to fill out before Query() call.
    struct FfxApiDimensions2D  displaySize;  ///< App needs to fill out before Query() call.
    uint32_t backBufferFormat;               ///< The surface format for the backbuffer. One of the values from FfxApiSurfaceFormat. App needs to fill out before Query() call.
    uint32_t backBufferCount;                ///< The number of backbuffers in the swapchain. App needs to fill out before Query() call.
    struct FfxApiDimensions2D uiResourceSize;///< This is the resolution of the resource that will be used for UI composition. Set to (0,0) if providing null uiResource in  ffxConfigureDescFrameGenerationSwapChainRegisterUiResourceDX12. App needs to fill out before Query() call.
    uint32_t uiResourceFormat;               ///< The surface format for the uiResource. One of the values from FfxApiSurfaceFormat. Set to FFX_API_SURFACE_FORMAT_UNKNOWN(0) if providing null uiResource in  ffxConfigureDescFrameGenerationSwapChainRegisterUiResourceDX12. App needs to fill out before Query() call.
    uint32_t flags;                          ///< Zero or combination of values from FfxApiUiCompositionFlags. App needs to fill out before Query() call.
    struct FfxApiEffectMemoryUsage* gpuMemoryUsageFrameGenerationSwapchain;
};
