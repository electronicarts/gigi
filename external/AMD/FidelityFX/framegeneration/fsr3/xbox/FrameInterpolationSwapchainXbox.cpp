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

#include "../../../backend/xbox/ffx_xbox.h"
#include "FrameInterpolationSwapchainXbox.h"

#include <XDisplay.h>

#include <deque>

FfxErrorCode ffxRegisterFrameinterpolationUiResourceX(FfxSwapchain gameSwapChain, FfxApiResource uiResource, uint32_t flags)
{
    FFX_UNUSED(gameSwapChain);
    FFX_UNUSED(uiResource);
    FFX_UNUSED(flags);
    return FFX_OK;
}

FfxABIVersion ffxGetSwapchainABIX(FfxSwapchain swapchain)
{
    if (swapchain)
    {
        return FfxABIVersion::FFX_ABI_VALID;
    }
    else
    {
        return FfxABIVersion::FFX_ABI_INVALID;
    }
}

FfxErrorCode ffxSetFrameGenerationConfigToSwapchainX(FfxFrameGenerationConfig const* config)
{
    FfxErrorCode result = FFX_ERROR_INVALID_ARGUMENT;

    if (config->swapChain)
    {
        FrameInterpolationSwapChainXbox* frameInterpolationSwapchain = reinterpret_cast<FrameInterpolationSwapChainXbox*>(config->swapChain);
        frameInterpolationSwapchain->setFrameGenerationConfig(config);
        result = FFX_OK;
    }

    return result;
}

FfxApiResource ffxGetFrameinterpolationTextureX(FfxSwapchain gameSwapChain)
{
    // Should not be called on Xbox (does not apply)
    FFX_UNUSED(gameSwapChain);
    FfxApiResource res = { nullptr };
    __debugbreak();
    return res;
}

FfxErrorCode ffxGetFrameinterpolationCommandlistX(FfxSwapchain gameSwapChain, FfxCommandList& gameCommandlist)
{
    // 1) query FrameInterpolationSwapChainXBox from gameSwapChain
    // 2) call  FrameInterpolationSwapChainXBox::getInterpolationCommandList()
    FrameInterpolationSwapChainXbox* frameInterpolationSwapchain = reinterpret_cast<FrameInterpolationSwapChainXbox*>(gameSwapChain);
    gameCommandlist = frameInterpolationSwapchain->getInterpolationCommandList();
    return FFX_OK;
}

FfxErrorCode ffxCreateFrameinterpolationSwapchainX(FfxCommandQueue gameQueue,
                                                   FfxCommandQueue computeQueue,
                                                   FfxCommandQueue presentQueue,
                                                   PfnFfxPresentXFunc gamePresentXCallback,
                                                   void *gamePresentXCallbackContext,
                                                   FfxSwapchain& outGameSwapChain)
{
    FFX_ASSERT(gameQueue);
    // computeQueue and presentQueue can be null

    FFX_ASSERT(gamePresentXCallback);

    ID3D12CommandQueue* dx12GameQueue = reinterpret_cast<ID3D12CommandQueue*>(gameQueue);
    ID3D12CommandQueue* dx12ComputeQueue = reinterpret_cast<ID3D12CommandQueue*>(computeQueue);
    ID3D12CommandQueue* dx12PresentQueue = reinterpret_cast<ID3D12CommandQueue*>(presentQueue);

    FfxErrorCode err = FFX_ERROR_INVALID_ARGUMENT;

	// Create the frame interpolation swapchain object
	FrameInterpolationSwapChainXbox* frameInterpolationSwapchain = new FrameInterpolationSwapChainXbox();
	if (frameInterpolationSwapchain)
	{
		if (SUCCEEDED(frameInterpolationSwapchain->init(dx12GameQueue, dx12ComputeQueue, dx12PresentQueue, gamePresentXCallback, gamePresentXCallbackContext)))
		{
			outGameSwapChain = reinterpret_cast<FfxSwapchain>(frameInterpolationSwapchain);
			err = FFX_OK;
		}
		else
		{
			delete frameInterpolationSwapchain;
			err = FFX_ERROR_INVALID_ARGUMENT;
		}
	}
	else
	{
		err = FFX_ERROR_OUT_OF_MEMORY;
	}

	return err;
}

FfxErrorCode ffxWaitForPresentX(FfxSwapchain gameSwapChain)
{
    FrameInterpolationSwapChainXbox* frameInterpolationSwapchain = reinterpret_cast<FrameInterpolationSwapChainXbox*>(gameSwapChain);
    frameInterpolationSwapchain->waitForPresentEvent();

    return FFX_OK;
}

void SubmitUiCompositionAndPresent(FrameinterpolationPresentInfo *presentInfo,
                                   FfxApiPresentCallbackFunc presentCallback,
                                   void *presentCallbackContext,
                                   D3D12XBOX_FRAME_PIPELINE_TOKEN framePipelineTokenOriginal,
                                   D3D12XBOX_FRAME_PIPELINE_TOKEN framePipelineTokenToSubmit,
                                   FfxApiResource realBackBufferPlane,
                                   FfxApiResource uiBackBufferPlane,
                                   bool asyncComputePresentEnabled,
                                   bool interpolatedFrameEnabled,
                                   bool premultipledAlphaEnabled,
                                   uint64_t currentFrameId,
                                   uint64_t presentIndex,
                                   ID3D12Fence *optionalGameQueueWaitFence,
                                   uint64_t optionalGameQueueWaitValue)
{
    ID3D12CommandQueue* pPresentQueue = asyncComputePresentEnabled ? presentInfo->asyncPresentQueue : presentInfo->gameQueue;

    if (presentCallback != nullptr)
    {
        if (optionalGameQueueWaitFence != nullptr)
        {
            presentInfo->gameQueue->Wait(optionalGameQueueWaitFence, optionalGameQueueWaitValue);
        }

        auto uiCompositionList = presentInfo->commandPool.get(presentInfo->gameQueue, L"uiCompositionList()");
        auto list = uiCompositionList->reset();

        ffxCallbackDescFrameGenerationPresent desc{};
        desc.header.type            = FFX_API_CALLBACK_DESC_TYPE_FRAMEGENERATION_PRESENT;
        desc.header.pNext           = nullptr;
        desc.commandList            = ffxGetCommandListX(list);
        desc.device                 = presentInfo->device;
        desc.isGeneratedFrame       = interpolatedFrameEnabled;
        desc.outputSwapChainBuffer  = realBackBufferPlane;
        desc.currentUI              = FfxApiResource({});
        desc.frameID                = currentFrameId;

        ffxCallbackDescFrameGenerationPresentPremulAlpha premulDesc = {};
        premulDesc.header.type = FFX_API_CALLBACK_DESC_TYPE_FRAMEGENERATION_PRESENT_PREMUL_ALPHA;
        premulDesc.header.pNext = nullptr;
        premulDesc.usePremulAlpha = premultipledAlphaEnabled;

        desc.header.pNext = &premulDesc.header;

        presentCallback(&desc, presentCallbackContext);

        uiCompositionList->execute(true);

        // NOTE: we only need to Signal 'compositionFence' when presentCallback exists which means UI composition was done on Graphics queue
        //       (if 'presentCallback' doesn't exist we are doing UI composition in PresentX on the queue that calls it)
        presentInfo->gameQueue->Signal(presentInfo->compositionFence, presentIndex);

        if (asyncComputePresentEnabled)
        {
            // Make sure composition has finished before presenting
            // NOTE: we only need to Wait for 'compositionFence' when presentCallback exists which means UI composition was done on Graphics queue
            //       (if 'presentCallback' doesn't exist we are doing UI composition in PresentX on the queue that calls it)
            //       we also wait only when we are presenting from Async queue (so Async queue needs to wait for Graphics queue)
            pPresentQueue->Wait(presentInfo->compositionFence, presentIndex);
        }
    }

    FfxPresentXParams presentXParams = {};
    presentXParams.presentQueue                 = pPresentQueue;
    presentXParams.framePipelineTokenOriginal   = framePipelineTokenOriginal;
    presentXParams.framePipelineTokenToSubmit   = framePipelineTokenToSubmit;
    presentXParams.sceneBackBufferToPresent     = reinterpret_cast<ID3D12Resource*>(realBackBufferPlane.resource);
    presentXParams.uiBackBufferToPresent        = reinterpret_cast<ID3D12Resource*>(uiBackBufferPlane.resource);
    presentXParams.interpolatedFrame            = interpolatedFrameEnabled;
    presentXParams.presentContext               = presentInfo->presentXContext;

    // Let the application side call the real PresentX
    FFX_ASSERT(presentInfo->presentX);
    presentInfo->presentX(&presentXParams);

    // NOTE: the queue that does present also ticks both fences
    if (presentCallback == nullptr)
    {
        pPresentQueue->Signal(presentInfo->compositionFence, presentIndex);
    }
    pPresentQueue->Signal(presentInfo->presentFence, presentIndex);
}

DWORD WINAPI presenterThread(LPVOID param)
{
    FrameinterpolationPresentInfo* presenter = static_cast<FrameinterpolationPresentInfo*>(param);

    if (presenter)
    {
        UINT64 numFramesSentForPresentation = 0;

        while (!presenter->shutdown)
        {
            WaitForSingleObject(presenter->presentEvent, INFINITE);

            if (!presenter->shutdown)
            {
                // Copy the presentation data
                EnterCriticalSection(&presenter->criticalSectionScheduledFrame);

                PacingData entry = presenter->scheduledPresents;
                presenter->scheduledPresents.invalidate();

                LeaveCriticalSection(&presenter->criticalSectionScheduledFrame);

                if (entry.numFramesToPresent > 0)
                {
                    // we might have dropped entries so have to update here, otherwise we might deadlock
                    ID3D12CommandQueue* pPresentQueue = entry.useAsyncComputePresent ? presenter->asyncPresentQueue : presenter->gameQueue;
                    pPresentQueue->Signal(presenter->presentFence, entry.numFramesSentForPresentationBase);
                    pPresentQueue->Wait(presenter->interpolationFence, entry.interpolationCompletedFenceValue);

                    for (uint32_t frameType = 0; frameType < PacingData::FrameType::Count; frameType++)
                    {
                        PacingData::FrameInfo& frameInfo = entry.frames[frameType];
                        if (frameInfo.doPresent)
                        {
                            // if we are presenting both a real and interpolated frame, need a new frame token
                            // which should already be ready for acquisition
                            if (frameType == PacingData::FrameType::Real)
                            {
                                D3D12XBOX_FRAME_PIPELINE_TOKEN token = D3D12XBOX_FRAME_PIPELINE_TOKEN_NULL;
                                presenter->device->WaitFrameEventX(D3D12XBOX_FRAME_EVENT_ORIGIN,
                                    INFINITE, nullptr,
                                    D3D12XBOX_WAIT_FRAME_EVENT_FLAG_NONE,
                                    &token);

                                // Set our token for the presentation of the next frame
                                frameInfo.framePipelineTokenToSubmit = token;
                            }

                            SubmitUiCompositionAndPresent(
                                presenter,
                                entry.presentCallback,
                                entry.presentCallbackContext,
                                frameInfo.framePipelineTokenOriginal,
                                frameInfo.framePipelineTokenToSubmit,
                                frameInfo.sceneBackBufferToPresent,
                                frameInfo.uiBackBufferToPresent,
                                entry.useAsyncComputePresent,
                                frameType != PacingData::FrameType::Real,
                                entry.usePremulAlphaComposite,
                                entry.currentFrameID,
                                frameInfo.presentIndex,
                                presenter->interpolationFence,
                                frameInfo.interpolationCompletedFenceValue
                            );

                            if (frameType == PacingData::FrameType::Real)
                            {
                                // let the main thread the "real" presentX was called
                                SetEvent(presenter->presentXEvent);
                            }
                        }
                    }

                    numFramesSentForPresentation = entry.numFramesSentForPresentationBase + entry.numFramesToPresent;
                }
            }
        }

        waitForFenceValue(presenter->presentFence, numFramesSentForPresentation);
    }

    return 0;
}


HRESULT FrameInterpolationSwapChainXbox::init(ID3D12CommandQueue* gameQueue,
                                              ID3D12CommandQueue* computeQueue,
                                              ID3D12CommandQueue* presentQueue,
                                              PfnFfxPresentXFunc gamePresentXCallback,
                                              void *gamePresentXCallbackContext)
{
    FFX_ASSERT(gameQueue);
    FFX_ASSERT(gamePresentXCallback);

    // set default UI composition / Frame Interpolation Present function
    // setting 'nullptr' means that UI composition is done at PresentX call
    presentCallback = nullptr;
    presentCallbackContext = nullptr;

    // set the PresentX() callback to handle hardware frame buffer present
    presentInfo.presentX = gamePresentXCallback;
    presentInfo.presentXContext = gamePresentXCallbackContext;

    // get the device
    gameQueue->GetDevice(IID_GDK_PPV_ARGS(&presentInfo.device));

    presentInfo.gameQueue = gameQueue;

    InitializeCriticalSection(&criticalSection);
    InitializeCriticalSection(&criticalSectionUpdateConfig);
    InitializeCriticalSection(&presentInfo.criticalSectionScheduledFrame);

    presentInfo.presentEvent = CreateEvent(NULL, FALSE, FALSE, nullptr);
    presentInfo.framePipelineTokenAcquiredEvent = CreateEvent(NULL, FALSE, FALSE, nullptr);
    presentInfo.framePipelineTokenEvent = CreateEvent(NULL, FALSE, FALSE, nullptr);
    presentInfo.presentXEvent = CreateEvent(NULL, FALSE, FALSE, nullptr);

    // Query the HDR mode settings
    XDisplayHdrModeInfo displayModeHdrInfo;
    if (XDisplayHdrModeResult::Enabled == XDisplayTryEnableHdrMode(XDisplayHdrModePreference::PreferHdr, &displayModeHdrInfo))
    {
        minLuminance = displayModeHdrInfo.minToneMapLuminance;
        maxLuminance = displayModeHdrInfo.maxToneMapLuminance;
        backBufferTransferFunction = FFX_API_BACKBUFFER_TRANSFER_FUNCTION_PQ;
    }
    else
    {
        // Use default LDR luminance values (300 nits max)
        minLuminance = 0.f;
        maxLuminance = 300.0f;
        backBufferTransferFunction = FFX_API_BACKBUFFER_TRANSFER_FUNCTION_SRGB;
    }

    presentInfo.device->CreateFence(gameFenceValue, D3D12_FENCE_FLAG_NONE, IID_GDK_PPV_ARGS(&presentInfo.gameFence));
    presentInfo.gameFence->SetName(L"AMD FSR GameFence");

    presentInfo.device->CreateFence(interpolationPresentFenceValue, D3D12_FENCE_FLAG_NONE, IID_GDK_PPV_ARGS(&presentInfo.interpolationPresentFence));
    presentInfo.interpolationPresentFence->SetName(L"AMD FSR InterpolationPresentFence");

    presentInfo.device->CreateFence(interpolationFenceValue, D3D12_FENCE_FLAG_NONE, IID_GDK_PPV_ARGS(&presentInfo.interpolationFence));
    presentInfo.interpolationFence->SetName(L"AMD FSR InterpolationFence");

    presentInfo.device->CreateFence(framesSentForPresentation, D3D12_FENCE_FLAG_NONE, IID_GDK_PPV_ARGS(&presentInfo.presentFence));
    presentInfo.presentFence->SetName(L"AMD FSR PresentFence");

    presentInfo.device->CreateFence(framesSentForPresentation, D3D12_FENCE_FLAG_NONE, IID_GDK_PPV_ARGS(&presentInfo.compositionFence));
    presentInfo.compositionFence->SetName(L"AMD FSR CompositionFence");

    // assign the passed in compute and present queues (will validate later if requested)
    presentInfo.asyncComputeQueue = computeQueue;
    presentInfo.asyncPresentQueue = presentQueue;

    // default to dispatch interpolation workloads and presentation on the game queue
    presentInfo.interpolationQueue = gameQueue;

    return S_OK;
}

FrameInterpolationSwapChainXbox::FrameInterpolationSwapChainXbox()
{

}

FrameInterpolationSwapChainXbox::~FrameInterpolationSwapChainXbox()
{
	shutdown();
}

HRESULT FrameInterpolationSwapChainXbox::shutdown()
{
    // m_pDevice will be nullptr if already shutdown
    if (presentInfo.device)
    {
        destroyReplacementResources();

        killPresenterThread();
        SafeCloseHandle(presentInfo.presentEvent);
        SafeCloseHandle(presentInfo.framePipelineTokenAcquiredEvent);
        SafeCloseHandle(presentInfo.framePipelineTokenEvent);
        SafeCloseHandle(presentInfo.presentXEvent);

        // if we failed initialization, we may not have an interpolation queue or fence
        if (presentInfo.interpolationQueue)
        {
            if (presentInfo.interpolationFence)
            {
                presentInfo.interpolationQueue->Signal(presentInfo.interpolationFence, ++interpolationFenceValue);
                waitForFenceValue(presentInfo.interpolationFence, interpolationFenceValue);
            }
        }

        SafeRelease(presentInfo.interpolationPresentFence);
        SafeRelease(presentInfo.interpolationFence);
        SafeRelease(presentInfo.presentFence);
        SafeRelease(presentInfo.compositionFence);

        if (presentInfo.gameFence)
        {
            waitForFenceValue(presentInfo.gameFence, gameFenceValue);
        }
        SafeRelease(presentInfo.gameFence);

        DeleteCriticalSection(&criticalSection);
        DeleteCriticalSection(&criticalSectionUpdateConfig);
        DeleteCriticalSection(&presentInfo.criticalSectionScheduledFrame);

        SafeRelease(presentInfo.device);
    }

    return S_OK;
}




FfxErrorCode ffxDestroyFrameinterpolationSwapchainX(FfxSwapchain gameSwapChain)
{
    FrameInterpolationSwapChainXbox* frameInterpolationSwapchain = reinterpret_cast<FrameInterpolationSwapChainXbox*>(gameSwapChain);
	delete(frameInterpolationSwapchain);

	return FFX_OK;
}

FfxErrorCode ffxPresentX(FfxSwapchain gameSwapChain,
                         D3D12XBOX_FRAME_PIPELINE_TOKEN realBackBufferToken,
                         FfxApiResource realBackBufferPlane,
                         FfxApiResource interpolatedBackBufferPlane,
                         FfxApiResource uiBackBufferPlane)
{
    FrameInterpolationSwapChainXbox* frameInterpolationSwapchain = reinterpret_cast<FrameInterpolationSwapChainXbox*>(gameSwapChain);
    return frameInterpolationSwapchain->PresentX(realBackBufferToken, realBackBufferPlane, interpolatedBackBufferPlane, uiBackBufferPlane);
}

bool FrameInterpolationSwapChainXbox::killPresenterThread()
{
    if (presenterThreadHandle != NULL)
    {
        // prepare present CPU thread for shutdown
        presentInfo.shutdown = true;

        // signal event to allow thread to finish
        SetEvent(presentInfo.presentEvent);
        WaitForSingleObject(presenterThreadHandle, INFINITE);
        SafeCloseHandle(presenterThreadHandle);
    }

    return presenterThreadHandle == NULL;
}

bool FrameInterpolationSwapChainXbox::spawnPresenterThread()
{
    if (presenterThreadHandle == NULL)
    {
        presentInfo.shutdown = false;
        presenterThreadHandle = CreateThread(nullptr, 0, presenterThread, reinterpret_cast<void*>(&presentInfo), 0, nullptr);

        FFX_ASSERT(presenterThreadHandle != NULL);

        if (presenterThreadHandle != 0)
        {
            SetThreadPriority(presenterThreadHandle, THREAD_PRIORITY_HIGHEST);
            SetThreadDescription(presenterThreadHandle, L"AMD FSR Presentation Thread");
        }

        //SetEvent(presentInfo.interpolationEvent);
    }

    return presenterThreadHandle != NULL;
}

void FrameInterpolationSwapChainXbox::discardOutstandingInterpolationCommandLists()
{
    // drop any outstanding interpolation command lists
    for (size_t i = 0; i < _countof(registeredInterpolationCommandLists); i++)
    {
        if (registeredInterpolationCommandLists[i] != nullptr)
        {
            registeredInterpolationCommandLists[i]->drop(true);
            registeredInterpolationCommandLists[i] = nullptr;
        }
    }
}

void FrameInterpolationSwapChainXbox::setFrameGenerationConfig(FfxFrameGenerationConfig const* config)
{
    EnterCriticalSection(&criticalSectionUpdateConfig);
    FFX_ASSERT(config);

    // if config is a pointer to the internal config ::present called this function to apply the changes
    bool applyChangesNow = (config == &nextFrameGenerationConfig);

    FfxApiPresentCallbackFunc inputPresentCallback = nullptr;
    void  *inputPresentCallbackContext = nullptr;
    if (nullptr != config->presentCallback)
    {
        inputPresentCallback = config->presentCallback;
        inputPresentCallbackContext = config->presentCallbackContext;
    }

    ID3D12CommandQueue* inputInterpolationQueue = presentInfo.gameQueue;

    // if this is called externally just copy the new config to the internal copy to avoid potentially stalling on criticalSection
    if (!applyChangesNow)
    {
        nextFrameGenerationConfig = *config;

        // in case of actual reconfiguration: apply the changes immediately
        if (presentInfo.interpolationQueue != inputInterpolationQueue
            || interpolationEnabled != config->frameGenerationEnabled
            || presentCallback != inputPresentCallback
            || frameGenerationCallback != config->frameGenerationCallback
            || frameGenerationCallbackContext != config->frameGenerationCallbackContext)
        {
            applyChangesNow = true;
        }
    }

    if (applyChangesNow)
    {
        EnterCriticalSection(&criticalSection);

        currentFrameID          = config->frameID;
        presentInterpolatedOnly = config->onlyPresentInterpolated;
        interpolationRect       = config->interpolationRect;

        if (config->allowAsyncWorkloads)
        {
            FFX_ASSERT_MESSAGE(presentInfo.asyncComputeQueue, "allowAsyncWorkloads requested but passed in async compute queue was NULL");
            inputInterpolationQueue = presentInfo.asyncComputeQueue ? presentInfo.asyncComputeQueue : inputInterpolationQueue;
        }

        if (config->allowAsyncPresent != asyncComputePresentEnabled)
        {
            waitForPresents();
            FFX_ASSERT_MESSAGE(presentInfo.asyncPresentQueue, "allowAsyncPresent requested but passed in async present queue was NULL");
            asyncComputePresentEnabled = config->allowAsyncPresent;
        }

        if (presentInfo.interpolationQueue != inputInterpolationQueue)
        {
            waitForPresents();
            discardOutstandingInterpolationCommandLists();

            // change interpolation queue and reset fence value
            presentInfo.interpolationQueue = inputInterpolationQueue;
            interpolationFenceValue = 0;
            presentInfo.interpolationQueue->Signal(presentInfo.interpolationFence, interpolationFenceValue);
        }

        if (interpolationEnabled != config->frameGenerationEnabled ||
            presentCallback != inputPresentCallback ||
            frameGenerationCallback != config->frameGenerationCallback ||
            configFlags != (FfxApiDispatchFramegenerationFlags)config->flags ||
            presentCallbackContext != inputPresentCallbackContext ||
            frameGenerationCallbackContext != config->frameGenerationCallbackContext)
        {
            waitForPresents();
            presentCallback                 = inputPresentCallback;
            presentCallbackContext          = inputPresentCallbackContext;
            frameGenerationCallback         = config->frameGenerationCallback;
            configFlags                     = FfxApiDispatchFramegenerationFlags(config->flags);
            frameGenerationCallbackContext  = config->frameGenerationCallbackContext;

            // handle interpolation mode change
            if (interpolationEnabled != config->frameGenerationEnabled)
            {
                interpolationEnabled = config->frameGenerationEnabled;
                if (interpolationEnabled)
                {
                    frameInterpolationResetCondition = true;
                    spawnPresenterThread();
                }
                else
                {
                    killPresenterThread();
                }
            }
        }

        LeaveCriticalSection(&criticalSection);
    }

    LeaveCriticalSection(&criticalSectionUpdateConfig);
}

bool FrameInterpolationSwapChainXbox::destroyReplacementResources()
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&criticalSection);

    waitForPresents();

    const bool recreatePresenterThread = presenterThreadHandle != nullptr;
    if (recreatePresenterThread)
    {
        killPresenterThread();
    }

    discardOutstandingInterpolationCommandLists();

    // reset counters used in buffer management
    framesSentForPresentation = 0;
    interpolationCommandListIndex = 0;
    presentCount = 0;
    interpolationFenceValue = 0;
    gameFenceValue = 0;

    // if we didn't init correctly, some parameters may not exist
    if (presentInfo.gameFence)
    {
        presentInfo.gameFence->Signal(gameFenceValue);
    }

    if (presentInfo.interpolationFence)
    {
        presentInfo.interpolationFence->Signal(interpolationFenceValue);
    }

    if (presentInfo.presentFence)
    {
        presentInfo.presentFence->Signal(framesSentForPresentation);
    }

    if (presentInfo.compositionFence)
    {
        presentInfo.compositionFence->Signal(framesSentForPresentation);
    }

    frameInterpolationResetCondition = true;

    if (recreatePresenterThread)
    {
        spawnPresenterThread();
    }

    discardOutstandingInterpolationCommandLists();

    LeaveCriticalSection(&criticalSection);

    return SUCCEEDED(hr);
}

bool FrameInterpolationSwapChainXbox::waitForPresents()
{
    waitForFenceValue(presentInfo.gameFence, gameFenceValue);
    waitForFenceValue(presentInfo.interpolationFence, interpolationFenceValue);
    waitForFenceValue(presentInfo.presentFence, framesSentForPresentation);
    return true;
}

bool FrameInterpolationSwapChainXbox::waitForPresentEvent()
{
    // wait for the game fence if we are only doing interpolation to make sure everything is clear of the GPU (avoids possible GPU hang on fence)
    if (presentInterpolatedOnly)
    {
        waitForFenceValue(presentInfo.gameFence, gameFenceValue);
    }

    // wait until PresentX for the "real" back buffer has been scheduled before returning to not mess with application logic too much
    if (interpolationEnabled && !presentInterpolatedOnly)
    {
        WaitForSingleObject(presentInfo.presentXEvent, INFINITE);
    }

    return true;
}


void FrameInterpolationSwapChainXbox::presentPassthrough(D3D12XBOX_FRAME_PIPELINE_TOKEN realBackBufferToken,
                                                         FfxApiResource realBackBufferPlane,
                                                         FfxApiResource uiBackBufferPlane)
{
    SubmitUiCompositionAndPresent(&presentInfo,
                                  presentCallback,
                                  presentCallbackContext,
                                  realBackBufferToken,
                                  realBackBufferToken,
                                  realBackBufferPlane,
                                  uiBackBufferPlane,
                                  asyncComputePresentEnabled,
                                  /*interpolatedFrameEnabled */false,
                                  /*premultipliedAlphaEnabled */(presentInfo.uiCompositionFlags & FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_USE_PREMUL_ALPHA) != 0,
                                  currentFrameID,
                                  ++framesSentForPresentation,
                                  /*optionalGameQueueWaitFence*/nullptr,
                                  /*optionalGameQueueWaitFenceValue*/0);
}

void FrameInterpolationSwapChainXbox::dispatchInterpolationCommands(FfxApiResource interpolatedFrame, FfxApiResource realFrame)
{
    FFX_ASSERT(interpolatedFrame.resource);
    FFX_ASSERT(realFrame.resource);

    auto pRegisteredCommandList = registeredInterpolationCommandLists[interpolationCommandListIndex];
    if (pRegisteredCommandList != nullptr)
    {
        pRegisteredCommandList->execute(true);

        // currently no separate interpolation queue
        // NOTE: in theory, we need only to Signal 'interpolationFence'
        //       only when we submit interpolation command on Async queue, and
        //       only when we need to submit UI composition commands (either retrieved through `registeredInterpolationCommandLists` or `frameGenerationCallback`) on Graphics queue,
        //       so the graphics queue might need to wait for 'interpolationFence' before any UI composition
        presentInfo.interpolationQueue->Signal(presentInfo.interpolationFence, ++interpolationFenceValue);
    }
    else {
        Dx12Commands* interpolationCommandList = presentInfo.commandPool.get(presentInfo.interpolationQueue, L"getInterpolationCommandList()");
        auto dx12CommandList = interpolationCommandList->reset();

        ffxDispatchDescFrameGeneration desc{};
        desc.commandList = dx12CommandList;
        desc.outputs[0] = interpolatedFrame;
        desc.presentColor = realFrame;
        desc.reset = frameInterpolationResetCondition;
        desc.numGeneratedFrames = 1;
        desc.backbufferTransferFunction = static_cast<FfxApiBackbufferTransferFunction>(backBufferTransferFunction);
        desc.minMaxLuminance[0] = minLuminance;
        desc.minMaxLuminance[1] = maxLuminance;
        desc.generationRect  = interpolationRect;
        desc.frameID = currentFrameID;

        if (frameGenerationCallback(&desc, frameGenerationCallbackContext) == FFX_OK)
        {
            interpolationCommandList->execute(true);

            // NOTE: in theory, we need only to Signal 'interpolationFence'
            //       only when we submit interpolation command on Async queue, and
            //       only when we need to submit UI composition commands (either retrieved through `registeredInterpolationCommandLists` or `frameGenerationCallback`) on Graphics queue,
            //       so the graphics queue needs to wait for 'interpolationFence' before doing any UI composition
            presentInfo.interpolationQueue->Signal(presentInfo.interpolationFence, ++interpolationFenceValue);
        }

        // reset condition if at least one frame was interpolated
        if (desc.numGeneratedFrames > 0)
        {
            frameInterpolationResetCondition = false;
        }
    }
}

void FrameInterpolationSwapChainXbox::presentInterpolated(D3D12XBOX_FRAME_PIPELINE_TOKEN realFrameToken,
                                                          FfxApiResource realFrame,
                                                          FfxApiResource interpolatedFrame,
                                                          FfxApiResource uiFrame)
{
    // interpolation needs to wait for the game queue in case we are doing interpolation on async compute
    presentInfo.gameQueue->Signal(presentInfo.gameFence, ++gameFenceValue);
    presentInfo.interpolationQueue->Wait(presentInfo.gameFence, gameFenceValue);

    dispatchInterpolationCommands(interpolatedFrame, realFrame);

    EnterCriticalSection(&presentInfo.criticalSectionScheduledFrame);

    PacingData entry{};
    entry.presentCallback                   = presentCallback;
    entry.presentCallbackContext            = presentCallbackContext;
    entry.numFramesSentForPresentationBase  = framesSentForPresentation;
    entry.interpolationCompletedFenceValue  = interpolationPresentFenceValue;
    entry.usePremulAlphaComposite           = (presentInfo.uiCompositionFlags & FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_USE_PREMUL_ALPHA) != 0;
    entry.currentFrameID                    = currentFrameID;
    entry.useAsyncComputePresent            = asyncComputePresentEnabled;

    // interpolated
    PacingData::FrameInfo& fiInterpolated = entry.frames[PacingData::FrameType::Interpolated_1];
    if (interpolatedFrame.resource != nullptr)
    {
        fiInterpolated.doPresent                        = true;
        fiInterpolated.sceneBackBufferToPresent         = interpolatedFrame;
        fiInterpolated.uiBackBufferToPresent            = uiFrame;
        fiInterpolated.interpolationCompletedFenceValue = interpolationFenceValue;
        fiInterpolated.presentIndex                     = ++framesSentForPresentation;

        // Interpolated frame will use the original data from the application but redirect to the interpolation resource for the frame
        fiInterpolated.framePipelineTokenOriginal       = realFrameToken;
        fiInterpolated.framePipelineTokenToSubmit       = realFrameToken;
    }

    // real
    if (!presentInterpolatedOnly)
    {
        PacingData::FrameInfo& fiReal = entry.frames[PacingData::FrameType::Real];
        if (realFrame.resource != nullptr)
        {
            fiReal.doPresent                    = true;
            fiReal.sceneBackBufferToPresent     = realFrame;
            fiReal.uiBackBufferToPresent        = uiFrame;
            fiReal.presentIndex                 = ++framesSentForPresentation;

            // Real frame will use original data + frame token generated after interpolation presentation
            fiReal.framePipelineTokenOriginal   = realFrameToken;
            fiReal.framePipelineTokenToSubmit   = D3D12XBOX_FRAME_PIPELINE_TOKEN_NULL;
        }
    }

    entry.numFramesToPresent = UINT32(framesSentForPresentation - entry.numFramesSentForPresentationBase);

    presentInfo.scheduledPresents = entry;
    LeaveCriticalSection(&presentInfo.criticalSectionScheduledFrame);

    // Set event to kick off async CPU present thread
    SetEvent(presentInfo.presentEvent);
}

FfxErrorCode FrameInterpolationSwapChainXbox::PresentX(D3D12XBOX_FRAME_PIPELINE_TOKEN realBackBufferToken,
                                                       FfxApiResource realBackBufferPlane,
                                                       FfxApiResource interpolatedBackBufferPlane,
                                                       FfxApiResource uiBackBufferPlane)
{
	const UINT64 previousFramesSentForPresentation = framesSentForPresentation;

    setFrameGenerationConfig(&nextFrameGenerationConfig);

    EnterCriticalSection(&criticalSection);

    // determine what present path to execute
    const bool fgCallbackConfigured = frameGenerationCallback != nullptr;
    const bool fgCommandListConfigured = registeredInterpolationCommandLists[interpolationCommandListIndex] != nullptr;
    const bool runInterpolation = interpolationEnabled && (fgCallbackConfigured || fgCommandListConfigured);

    presentInfo.gameQueue->Wait(presentInfo.compositionFence, previousFramesSentForPresentation);

    previousFrameWasInterpolated = runInterpolation;

    if (runInterpolation)
    {
        presentInterpolated(realBackBufferToken, realBackBufferPlane, interpolatedBackBufferPlane, uiBackBufferPlane);
    }
    else
    {
        presentPassthrough(realBackBufferToken, realBackBufferPlane, uiBackBufferPlane);
    }

    // Unregister any potential command list
    registeredInterpolationCommandLists[interpolationCommandListIndex] = nullptr;

    // Increment frame counters for interpolation and back buffers
    presentCount++;
    interpolationCommandListIndex = presentCount % 2;

    LeaveCriticalSection(&criticalSection);

    return FFX_OK;
}

ID3D12GraphicsCommandList* FrameInterpolationSwapChainXbox::getInterpolationCommandList()
{
    ID3D12GraphicsCommandList* dx12CommandList = nullptr;

    EnterCriticalSection(&criticalSection);
    if (interpolationEnabled)
    {
        Dx12Commands* registeredCommands = registeredInterpolationCommandLists[interpolationCommandListIndex];

        // drop if already existing
        if (registeredCommands != nullptr)
        {
            registeredCommands->drop(true);
            registeredCommands = nullptr;
        }

        registeredCommands = presentInfo.commandPool.get(presentInfo.interpolationQueue, L"getInterpolationCommandList()");
        FFX_ASSERT(registeredCommands);
        dx12CommandList = registeredCommands->reset();

        registeredInterpolationCommandLists[interpolationCommandListIndex] = registeredCommands;
    }
    LeaveCriticalSection(&criticalSection);

    return dx12CommandList;
}
