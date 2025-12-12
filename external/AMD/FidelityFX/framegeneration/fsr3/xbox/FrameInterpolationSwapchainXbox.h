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

#include "FrameInterpolationSwapchainXbox_Helpers.h"
#include "../../../backend/xbox/ffx_xbox.h"
#include "../../include/ffx_framegeneration.h"

#define FFX_FRAME_INTERPOLATION_SWAP_CHAIN_XBOX_VERSION_MAJOR    3
#define FFX_FRAME_INTERPOLATION_SWAP_CHAIN_XBOX_VERSION_MINOR    1
#define FFX_FRAME_INTERPOLATION_SWAP_CHAIN_XBOX_VERSION_PATCH    5

#define FFX_FRAME_INTERPOLATION_SWAP_CHAIN_MAX_BUFFER_COUNT DXGI_MAX_SWAP_CHAIN_BUFFERS

typedef struct PacingData
{
    FfxApiPresentCallbackFunc       presentCallback = nullptr;
    void *                          presentCallbackContext = nullptr;
    UINT64                          interpolationCompletedFenceValue;

    bool                            useAsyncComputePresent;
    bool                            usePremulAlphaComposite;
    UINT64                          numFramesSentForPresentationBase;
    UINT32                          numFramesToPresent;
    UINT64                          currentFrameID;

    typedef enum FrameType
    {
        Interpolated_1,
        Real,
        Count
    } FrameType;

    struct FrameInfo
    {
        bool                            doPresent;
        FfxApiResource                     sceneBackBufferToPresent;
        FfxApiResource                     uiBackBufferToPresent;
        UINT64                          interpolationCompletedFenceValue;
        UINT64                          presentIndex;
        D3D12XBOX_FRAME_PIPELINE_TOKEN  framePipelineTokenOriginal;
        D3D12XBOX_FRAME_PIPELINE_TOKEN  framePipelineTokenToSubmit;
    };

    FrameInfo frames[FrameType::Count];

    void invalidate()
    {
        memset(this, 0, sizeof(PacingData));
    }

} PacingData;

typedef struct FrameinterpolationPresentInfo
{
    CRITICAL_SECTION		criticalSectionScheduledFrame;

    PfnFfxPresentXFunc      presentX                    = nullptr;
    void *                  presentXContext             = nullptr;

	ID3D12Device*			device                      = nullptr;

    Dx12CommandPool<32>      commandPool;

    PacingData              scheduledPresents;
    uint32_t                uiCompositionFlags          = 0;

    // Passed in queues from application
    ID3D12CommandQueue*		gameQueue				    = nullptr;
    ID3D12CommandQueue*     asyncComputeQueue           = nullptr;
    ID3D12CommandQueue*     asyncPresentQueue           = nullptr;

    // representative queues mapped to passed in queues
    ID3D12CommandQueue*     interpolationQueue          = nullptr;

    ID3D12Fence*			gameFence                   = nullptr;
    ID3D12Fence*            interpolationPresentFence   = nullptr;
    ID3D12Fence*			interpolationFence          = nullptr;
    ID3D12Fence*			presentFence                = nullptr;
    ID3D12Fence*			compositionFence            = nullptr;

    HANDLE					presentEvent                    = 0;
    HANDLE                  framePipelineTokenAcquiredEvent = 0;
    HANDLE                  framePipelineTokenEvent         = 0;
    HANDLE                  presentXEvent                   = 0;

    volatile bool			shutdown                    = false;
} FrameinterpolationPresentInfo;

typedef struct ReplacementResource
{
    ID3D12Resource* resource                = nullptr;
    UINT64          availabilityFenceValue  = 0;

    void destroy(bool releaseResource)
    {
        if (releaseResource)
        {
            SafeRelease(resource);
        }
        resource = nullptr;
        availabilityFenceValue = 0;
    }
} ReplacementResource;

typedef struct FfxFrameInterpolationSwapChainResourceInfo
{
    int  version;
    bool isInterpolated;
} FfxFrameInterpolationSwapChainResourceInfo;

class FrameInterpolationSwapChainXbox
{
protected:

	HRESULT               shutdown();

	FrameinterpolationPresentInfo	presentInfo = {};
    FfxFrameGenerationConfig        nextFrameGenerationConfig = {};

	CRITICAL_SECTION    criticalSection{};
    CRITICAL_SECTION    criticalSectionUpdateConfig{};

    UINT64              interpolationPresentFenceValue = 0;
	UINT64              interpolationFenceValue = 0;
	UINT64              gameFenceValue = 0;
	bool                frameInterpolationResetCondition = false;
    FfxApiRect2D           interpolationRect;

	Dx12Commands*       registeredInterpolationCommandLists[FFX_FRAME_INTERPOLATION_SWAP_CHAIN_MAX_BUFFER_COUNT] = {};
    int                 interpolationCommandListIndex = 0;

	UINT64              presentCount = 0;

    FfxApiDispatchFramegenerationFlags configFlags = {};

	bool                tearingSupported = false;

	bool                interpolationEnabled = false;
	bool                presentInterpolatedOnly = false;
    bool                asyncComputePresentEnabled = false;
    bool                previousFrameWasInterpolated = false;

    UINT64              currentFrameID = 0;

	UINT64              framesSentForPresentation = 0;
    HANDLE              presenterThreadHandle = 0;
	ULONG               refCount = 1;

	UINT                backBufferTransferFunction = 0;

	float               minLuminance = 0.0f;
	float               maxLuminance = 0.0f;

    FfxApiPresentCallbackFunc           presentCallback = nullptr;
    void *                              presentCallbackContext = nullptr;

	FfxApiFrameGenerationDispatchFunc   frameGenerationCallback  = nullptr;
    void*                               frameGenerationCallbackContext = nullptr;

    void presentPassthrough(D3D12XBOX_FRAME_PIPELINE_TOKEN realBackBufferToken,
                            FfxApiResource realBackBufferPlane,
                            FfxApiResource uiBackBufferPlane);

    void presentInterpolated(D3D12XBOX_FRAME_PIPELINE_TOKEN realBackBufferToken,
                             FfxApiResource realBackBufferPlane,
                             FfxApiResource interpolatedBackBufferPlane,
                             FfxApiResource uiBackBufferPlane);

	void dispatchInterpolationCommands(FfxApiResource interpolatedFrame, FfxApiResource realFrame);

	bool destroyReplacementResources();
	bool killPresenterThread();
	bool spawnPresenterThread();
	void discardOutstandingInterpolationCommandLists();


public:

    void setFrameGenerationConfig(FfxFrameGenerationConfig const* config);
	bool waitForPresents();
    bool waitForPresentEvent();

    ID3D12GraphicsCommandList* getInterpolationCommandList();

    FrameInterpolationSwapChainXbox();
	virtual ~FrameInterpolationSwapChainXbox();

    HRESULT init(ID3D12CommandQueue* gameQueue,
                 ID3D12CommandQueue* computeQueue,
                 ID3D12CommandQueue* presentQueue,
                 PfnFfxPresentXFunc gamePresentXCallback,
                 void *gamePresentXCallbackContext);

    FfxErrorCode PresentX(D3D12XBOX_FRAME_PIPELINE_TOKEN realBackBufferToken,
                          FfxApiResource realBackBufferPlane,
                          FfxApiResource interpolatedBackBufferPlane,
                          FfxApiResource uiBackBufferPlane);
};
