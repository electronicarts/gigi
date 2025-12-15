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

/// @defgroup GDKBackend GDK Backend
/// FidelityFX SDK native backend implementation for Microsoft GDK.
/// 
/// @ingroup Backends

/// @defgroup GDKFrameInterpolation GDK FrameInterpolation
/// FidelityFX SDK native frame interpolation implementation for GDK backend.
/// 
/// @ingroup GDKBackend

#pragma once

#if !defined(_GAMING_XBOX)
#pragma error "Xbox backend is only meant for XBOX gaming targets"
#endif // !defined(_GAMING_XBOX)

#if defined(_GAMING_XBOX_SCARLETT)

#include <d3d12_xs.h>
#include <d3dx12_xs.h>
#ifndef IID_GDK_PPV_ARGS
#define IID_GDK_PPV_ARGS(x) IID_GRAPHICS_PPV_ARGS(x)
#endif // IID_GDK_PPV_ARGS

#elif defined(_GAMING_XBOX_XBOXONE)

#include <d3d12_x.h>
#include <d3dx12_x.h>
#ifndef IID_GDK_PPV_ARGS
#define IID_GDK_PPV_ARGS(x) IID_GRAPHICS_PPV_ARGS(x)
#endif // IID_GDK_PPV_ARGS

#else

#pragma error "Unsupported XBOX gaming target"

#endif // #ifdef FFX_API_GDK_SCARLETT

#include "../../api/internal/ffx_interface.h"
#include "../../api/include/xbox/ffx_api_xbox.h"

#if defined(__cplusplus)
extern "C" {
#endif // #if defined(__cplusplus)
    
    /// Query how much memory is required for the DirectX 12 backend's scratch buffer.
    /// 
    /// @param [in] maxContexts                 The maximum number of simultaneous effect contexts that will share the backend.
    ///                                         (Note that some effects contain internal contexts which count towards this maximum)
    ///
    /// @returns
    /// The size (in bytes) of the required scratch memory buffer for the DX12 backend.
    /// @ingroup GDKBackend
    FFX_API size_t ffxGetScratchMemorySizeX(size_t maxContexts);

    /// Create a <c><i>FfxDevice</i></c> from a <c><i>ID3D12Device</i></c>.
    ///
    /// @param [in] device                      A pointer to the DirectX12 device.
    ///
    /// @returns
    /// An abstract FidelityFX device.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxDevice ffxGetDeviceX(ID3D12Device* device);

    /// Populate an interface with pointers for the DX12 backend.
    ///
    /// @param [out] backendInterface           A pointer to a <c><i>FfxInterface</i></c> structure to populate with pointers.
    /// @param [in] device                      A pointer to the DirectX12 device.
    /// @param [in] scratchBuffer               A pointer to a buffer of memory which can be used by the DirectX(R)12 backend.
    /// @param [in] scratchBufferSize           The size (in bytes) of the buffer pointed to by <c><i>scratchBuffer</i></c>.
    /// @param [in] maxContexts                 The maximum number of simultaneous effect contexts that will share the backend.
    ///                                         (Note that some effects contain internal contexts which count towards this maximum)
    ///
    /// @retval
    /// FFX_OK                                  The operation completed successfully.
    /// @retval
    /// FFX_ERROR_CODE_INVALID_POINTER          The <c><i>interface</i></c> pointer was <c><i>NULL</i></c>.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxErrorCode ffxGetInterfaceX(
        FfxInterface* backendInterface,
        FfxDevice device,
        void* scratchBuffer,
        size_t scratchBufferSize,
        size_t maxContexts);

    /// Create a <c><i>FfxCommandList</i></c> from a <c><i>ID3D12CommandList</i></c>.
    ///
    /// @param [in] cmdList                     A pointer to the DirectX12 command list.
    ///
    /// @returns
    /// An abstract FidelityFX command list.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxCommandList ffxGetCommandListX(ID3D12CommandList* cmdList);

    /// Create a <c><i>FfxPipeline</i></c> from a <c><i>ID3D12PipelineState</i></c>.
    ///
    /// @param [in] pipelineState               A pointer to the DirectX12 pipeline state.
    ///
    /// @returns
    /// An abstract FidelityFX pipeline.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxPipeline ffxGetPipelineX(ID3D12PipelineState* pipelineState);

    /// Fetch a <c><i>FfxApiResource</i></c> from a <c><i>GPUResource</i></c>.
    ///
    /// @param [in] dx12Resource                A pointer to the DX12 resource.
    /// @param [in] ffxResDescription           An <c><i>FfxApiResourceDescription</i></c> for the resource representation.
    /// @param [in] ffxResName                  (optional) A name string to identify the resource in debug mode.
    /// @param [in] state                       The state the resource is currently in.
    ///
    /// @returns
    /// An abstract FidelityFX resources.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxApiResource ffxGetResourceX(const ID3D12Resource*  dx12Resource,
        FfxApiResourceDescription ffxResDescription,
        const wchar_t* ffxResName,
        FfxApiResourceState state = FFX_API_RESOURCE_STATE_COMPUTE_READ);


    /// Fetch a <c><i>FfxApiSurfaceFormat</i></c> from a DXGI_FORMAT.
    ///
    /// @param [in] format              The DXGI_FORMAT to convert to <c><i>FfxApiSurfaceFormat</i></c>.
    ///
    /// @returns
    /// An <c><i>FfxApiSurfaceFormat</i></c>.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxApiSurfaceFormat ffxGetSurfaceFormatX(DXGI_FORMAT format);

    /// Fetch a <c><i>FfxApiResourceDescription</i></c> from an existing ID3D12Resource.
    ///
    /// @param [in] pResource           The ID3D12Resource resource to create a <c><i>FfxApiResourceDescription</i></c> for.
    /// @param [in] additionalUsages    Optional <c><i>FfxApiResourceUsage</i></c> flags needed for select resource mapping.
    ///
    /// @returns
    /// An <c><i>FfxApiResourceDescription</i></c>.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxApiResourceDescription ffxGetResourceDescriptionX(const ID3D12Resource* pResource,
                                                              FfxApiResourceUsage      additionalUsages = FFX_API_RESOURCE_USAGE_READ_ONLY);

    /// Fetch a <c><i>FfxCommandQueue</i></c> from an existing ID3D12CommandQueue.
    ///
    /// @param [in] pCommandQueue       The ID3D12CommandQueue to create a <c><i>FfxCommandQueue</i></c> from.
    ///
    /// @returns
    /// An <c><i>FfxCommandQueue</i></c>.
    ///
    /// @ingroup GDKBackend
    FFX_API FfxCommandQueue ffxGetCommandQueueX(ID3D12CommandQueue* pCommandQueue);

    FFX_API void ffxRegisterResourceAllocatorX(PfnFfxResourceAllocatorFunc fpResourceAllocator);
    FFX_API void ffxRegisterResourceDeallocatorX(PfnFfxResourceDeallocatorFunc fpResourceDeallocator);
    FFX_API void ffxRegisterConstantBufferAllocatorX(FfxApiConstantBufferAllocator fpConstantBufferAllocator);

#if defined(FFX_FRAMEGENERATION)
#include "../../framegeneration/include/xbox/ffx_api_framegeneration_xbox.h"
    /// Creates a <c><i>FfxSwapchain</i></c> from passed in parameters. On console, the FfxSwapChain struct represents an overloaded command queue.
    ///
    /// @param [in] gameQueue               The ID3D12CommandQueue (graphics) from the calling application.
    /// @param [in] computeQueue            The ID3D12CommandQueue (compute) from the calling application to use for async workloads (see FfxFrameGenerationConfig to enable compute workloads).
    /// @param [in] presentQueue            The ID3D12CommandQueue (compute) from the calling application to use for async presentation (see FfxFrameGenerationConfig to enable asyncPresent).
    /// @param [in] swapChainResourceDesc	The D3D12_RESOURCE_DESC describing the presentation buffer resources.
    /// @param [out] outGameSwapChain       The created <c><i>FfxSwapchain</i></c>.
    ///
    /// @retval
    /// FFX_OK                              The operation completed successfully.
    /// @retval
    /// FFX_ERROR_INVALID_ARGUMENT          One of the parameters is invalid.
    /// FFX_ERROR_OUT_OF_MEMORY             Insufficient memory available to allocate <c><i>FfxSwapchain</i></c> or underlying component.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxErrorCode ffxCreateFrameinterpolationSwapchainX(FfxCommandQueue gameQueue,
        FfxCommandQueue computeQueue,
        FfxCommandQueue presentQueue,
        PfnFfxPresentXFunc gamePresentXCallback,
        void *gamePresentXCallbackContext,
        FfxSwapchain& outGameSwapChain);

    /// Release a <c><i>FfxSwapchain</i></c> when done with it.
    ///
    /// @param [in] gameSwapChain			The <c><i>FfxSwapchain</i></c> to release.
    ///
    /// @retval
    /// FFX_OK                              The operation completed successfully.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxErrorCode ffxDestroyFrameinterpolationSwapchainX(FfxSwapchain gameSwapChain);

    /// Will wait until the PresentX call on the real frame buffer has been called. This is
    /// to control the pacing in calls to WaitFrameEventX
    ///
    /// @param [in] gameSwapChain			The <c><i>FfxSwapchain</i></c> object to query for present completion.
    ///
    /// @retval
    /// FFX_OK                              The operation completed successfully.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxErrorCode ffxWaitForPresentX(FfxSwapchain gameSwapChain);

    /// Replacement for PresentX to be called at the end of the frame when frame interpolation is enabled.
    /// This will either call the application callback for presentX presentation directly or will schedule
    /// frame interpolation to occur and pass presentation calls off to an asynchronous thread.
    ///
    /// @param [in] gameSwapChain               The <c><i>FfxSwapchain</i></c> to use for presentation.
    /// @param [in] realBackBufferPlane         The <c><i>ID3D12Resource</i></c> representing the back buffer prepared by the title
    /// @param [in] interpolatedBackBufferPlane The <c><i>ID3D12Resource</i></c> representing the back buffer where interpolated data will be written
    ///
    /// @retval
    /// FFX_OK                                  The operation completed successfully.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxErrorCode ffxPresentX(FfxSwapchain gameSwapChain,
                                     D3D12XBOX_FRAME_PIPELINE_TOKEN realBackBufferToken,
                                     FfxApiResource realBackBufferPlane,
                                     FfxApiResource interpolatedBackBufferPlane,
                                     FfxApiResource uiBackBufferPlane);

    /// Registers a <c><i>FfxApiResource</i></c> to use for UI with the provided <c><i>FfxSwapchain</i></c>.
    ///
    /// @param [in] gameSwapChain           The <c><i>FfxSwapchain</i></c> to to register the UI resource with.
    /// @param [in] uiResource              The <c><i>FfxApiResource</i></c> representing the UI resource.
    /// @param [in] flags                   A set of <c><i>FfxUiCompositionFlags</i></c>.
    ///
    /// @retval
    /// FFX_OK                              The operation completed successfully.
    /// @retval
    /// FFX_ERROR_INVALID_ARGUMENT          Could not query the interface for the frame interpolation swap chain.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxErrorCode ffxRegisterFrameinterpolationUiResourceX(FfxSwapchain gameSwapChain, FfxApiResource uiResource, uint32_t flags);

    /// Fetches a <c><i>FfxCommandList</i></c> from the <c><i>FfxSwapchain</i></c>.
    ///
    /// @param [in] gameSwapChain           The <c><i>FfxSwapchain</i></c> to get a <c><i>FfxCommandList</i></c> from.
    /// @param [out] gameCommandlist        The <c><i>FfxCommandList</i></c> from the provided <c><i>FfxSwapchain</i></c>.
    ///
    /// @retval
    /// FFX_OK                              The operation completed successfully.
    /// @retval
    /// FFX_ERROR_INVALID_ARGUMENT          Could not query the interface for the frame interpolation swap chain.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxErrorCode ffxGetFrameinterpolationCommandlistX(FfxSwapchain gameSwapChain, FfxCommandList& gameCommandlist);

    /// Fetches a <c><i>FfxApiResource</i></c>  representing the backbuffer from the <c><i>FfxSwapchain</i></c>.
    ///
    /// @param [in] gameSwapChain           The <c><i>FfxSwapchain</i></c> to get a <c><i>FfxApiResource</i></c> backbuffer from.
    ///
    /// @returns
    /// An abstract FidelityFX resources for the swapchain backbuffer.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxApiResource ffxGetFrameinterpolationTextureX(FfxSwapchain gameSwapChain);

    /// Sets a <c><i>FfxFrameGenerationConfig</i></c> to the internal FrameInterpolationSwapChain (in the backend).
    ///
    /// @param [in] config                  The <c><i>FfxFrameGenerationConfig</i></c> to set.
    ///
    /// @retval
    /// FFX_OK                              The operation completed successfully.
    /// @retval
    /// FFX_ERROR_INVALID_ARGUMENT          Could not query the interface for the frame interpolation swap chain.
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxErrorCode ffxSetFrameGenerationConfigToSwapchainX(FfxFrameGenerationConfig const* config);

    /// Query The ABI version used by the swapchain
    ///
    /// @param [in] gameSwapChain           The <c><i>FfxSwapchain</i></c>
    ///
    /// @retval
    /// FFX_ABI_INVALID An error occurred
    /// @retval
    /// FFX_ABI_VALID The latest stable ABI
    ///
    /// @ingroup GDKFrameInterpolation
    FFX_API FfxABIVersion ffxGetSwapchainABIX(FfxSwapchain swapchain);
#endif // defined(FFX_FRAMEGENERATION)

#if defined(__cplusplus)
}
#endif // #if defined(__cplusplus)
