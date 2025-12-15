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

#include "../../include/ffx_framegeneration.hpp"
#include "../../../api/include/xbox/ffx_api_xbox.hpp"
#include "../../include/xbox/ffx_api_framegeneration_xbox.hpp"
#include "../include/ffx_provider_fsr3framegenerationswapchain.h"

#include "../../../backend/xbox/ffx_xbox.h"

#include <stdlib.h>

bool ffxProvider_Fsr3FrameGenerationSwapChain::CanProvide(uint64_t type) const
{
    return ((type & FFX_API_EFFECT_MASK) == FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN &&
            (type & FFX_API_BACKEND_MASK) == FFX_API_BACKEND_ID_XBOX);
}

struct InternalFgScContext
{
    InternalContextHeader header;
    FfxSwapchain          fiSwapChain;
};

ffxProvider_Fsr3FrameGenerationSwapChain::ffxProvider_Fsr3FrameGenerationSwapChain() :
    ffxProvider(/*version 1.1.2*/ 0xF65C'DD12'01'001'002ui64, FFX_API_EFFECT_ID_FRAMEGENERATIONSWAPCHAIN, "1.1.2")
{
}

ffxReturnCode_t ffxProvider_Fsr3FrameGenerationSwapChain::CreateContext(ffxContext* context, ffxCreateContextDescHeader* header, Allocator& alloc)
{
    if (auto desc = ffx::DynamicCast<ffxCreateContextDescFrameGenerationSwapChainXbox>(header))
    {
        InternalFgScContext* internal_context = alloc.construct<InternalFgScContext>();
        VERIFY(internal_context, FFX_API_RETURN_ERROR_MEMORY);
        internal_context->header.provider = this;

        // Create the internal ffxSwapchain representation
        FfxErrorCode err = ffxCreateFrameinterpolationSwapchainX(desc->gameQueue, desc->computeQueue,
                                                                 desc->presentQueue, desc->pfnFfxPresentX,
                                                                 desc->presentXContext, internal_context->fiSwapChain);
        if (err != FFX_OK)
        {
            alloc.dealloc(internal_context);

            if (err == FFX_ERROR_INVALID_ARGUMENT) {
                return FFX_API_RETURN_ERROR_PARAMETER;
            }
            else {
                return FFX_API_RETURN_ERROR_MEMORY;
            }
        }

        *context = internal_context;
        return FFX_API_RETURN_OK;
    }
    else
    {
        return FFX_API_RETURN_ERROR_UNKNOWN_DESCTYPE;
    }
}

ffxReturnCode_t ffxProvider_Fsr3FrameGenerationSwapChain::DestroyContext(ffxContext* context, Allocator& alloc)
{
    VERIFY(context, FFX_API_RETURN_ERROR_PARAMETER);
    VERIFY(*context, FFX_API_RETURN_ERROR_PARAMETER);

    InternalFgScContext* internal_context = reinterpret_cast<InternalFgScContext*>(*context);

    // Destroy the internal ffxSwapchain representation
    ffxDestroyFrameinterpolationSwapchainX(internal_context->fiSwapChain);

    alloc.dealloc(internal_context);

    return FFX_API_RETURN_OK;
}

ffxReturnCode_t ffxProvider_Fsr3FrameGenerationSwapChain::Configure(ffxContext* context, const ffxConfigureDescHeader* header) const
{
    VERIFY(header, FFX_API_RETURN_ERROR_PARAMETER);
    VERIFY(context, FFX_API_RETURN_ERROR_PARAMETER);
    VERIFY(*context, FFX_API_RETURN_ERROR_PARAMETER);

    InternalFgScContext* internal_context = reinterpret_cast<InternalFgScContext*>(*context);
    // This should not currently get called on Xbox
    {
        return FFX_API_RETURN_ERROR_PARAMETER;
    }
}

ffxReturnCode_t ffxProvider_Fsr3FrameGenerationSwapChain::Query(ffxContext* context, ffxQueryDescHeader* header) const
{
    VERIFY(header, FFX_API_RETURN_ERROR_PARAMETER);
    VERIFY(context, FFX_API_RETURN_ERROR_PARAMETER);
    VERIFY(*context, FFX_API_RETURN_ERROR_PARAMETER);

    InternalFgScContext* internal_context = reinterpret_cast<InternalFgScContext*>(*context);
    if (auto desc = ffx::DynamicCast<ffxQueryDescFrameGenerationSwapChainInterpolationCommandListXbox>(header))
    {
        FfxCommandList outCommandList{};
        TRY2(ffxGetFrameinterpolationCommandlistX(internal_context->fiSwapChain, outCommandList));
        *desc->pOutCommandList = outCommandList;
        return FFX_API_RETURN_OK;
    }
    else if (auto desc = ffx::DynamicCast<ffxQueryDescFrameGenerationSwapChainInterpolationTextureXbox>(header))
    {
        *desc->pOutTexture = ffxGetFrameinterpolationTextureX(internal_context->fiSwapChain);
        return FFX_API_RETURN_OK;
    }
    else if (auto desc = ffx::DynamicCast<ffxQueryDescFrameGenerationSwapChainContextXbox>(header))
    {
        desc->ffxSwapchain = internal_context->fiSwapChain;
        return FFX_API_RETURN_OK;
    }
    else
    {
        return FFX_API_RETURN_ERROR_PARAMETER;
    }
}

ffxReturnCode_t ffxProvider_Fsr3FrameGenerationSwapChain::Dispatch(ffxContext* context, const ffxDispatchDescHeader* header) const
{
    VERIFY(*context, FFX_API_RETURN_ERROR_PARAMETER);

    InternalFgScContext* internal_context = reinterpret_cast<InternalFgScContext*>(*context);
    if (auto desc = ffx::DynamicCast<ffxDispatchDescFrameGenerationSwapChainPresentXbox>(header))
    {
        // TODO, clean this up
        // Need to create FfxResources from FfxApiResources
        FfxApiResourceDescription resDesc     = ffxGetResourceDescriptionX((ID3D12Resource*)desc->realBackBufferPlane.resource);
        FfxApiResource            realBBPlane = ffxGetResourceX((ID3D12Resource*)desc->realBackBufferPlane.resource, resDesc,
                                                            L"", (FfxApiResourceState)desc->realBackBufferPlane.state);
                               resDesc       = ffxGetResourceDescriptionX((ID3D12Resource*)desc->interpolatedBackBufferPlane.resource);
        FfxApiResource            interpBBPlane = ffxGetResourceX((ID3D12Resource*)desc->interpolatedBackBufferPlane.resource, resDesc,
                                                            L"", (FfxApiResourceState)desc->interpolatedBackBufferPlane.state);
                               resDesc   = ffxGetResourceDescriptionX((ID3D12Resource*)desc->uiBackBufferPlane.resource);
        FfxApiResource            uiBBPlane = ffxGetResourceX((ID3D12Resource*)desc->uiBackBufferPlane.resource, resDesc,
                                                            L"", (FfxApiResourceState)desc->uiBackBufferPlane.state);

        ffxPresentX(internal_context->fiSwapChain, desc->realBackBufferToken, realBBPlane,
                    interpBBPlane, uiBBPlane);

        return FFX_API_RETURN_OK;
    }
    else if (auto desc = ffx::DynamicCast<ffxDispatchDescFrameGenerationSwapChainWaitForPresentXbox>(header))
    {
        ffxWaitForPresentX(internal_context->fiSwapChain);
        return FFX_API_RETURN_OK;
    }
    else
    {
        return FFX_API_RETURN_ERROR;
    }
}

ffxProvider_Fsr3FrameGenerationSwapChain& ffxProvider_Fsr3FrameGenerationSwapChain::GetInstance()
{
    static ffxProvider_Fsr3FrameGenerationSwapChain instance;
    return instance;
}
