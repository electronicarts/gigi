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

#include "../../api/include/xbox/ffx_api_xbox.h"
#include "../../api/internal/ffx_backends.h"

#include "ffx_xbox.h"

ffxReturnCode_t CreateBackend(const ffxCreateContextDescHeader *desc, bool& backendFound, FfxInterface *iface, size_t contexts, Allocator& alloc)
{
    for (const auto* it = desc->pNext; it; it = it->pNext)
    {
        switch (it->type)
        {
        case FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_XBOX:
        {
            // check for double backend just to make sure.
            if (backendFound) {
                return FFX_API_RETURN_ERROR;
            }
                
            backendFound = true;

            const auto *backendDesc = reinterpret_cast<const ffxCreateBackendDescXbox*>(it);
            FfxDevice device = ffxGetDeviceX(backendDesc->device);
            size_t scratchBufferSize = ffxGetScratchMemorySizeX(contexts);
            void* scratchBuffer = alloc.alloc(scratchBufferSize);
            memset(scratchBuffer, 0, scratchBufferSize);
            TRY2(ffxGetInterfaceX(iface, device, scratchBuffer, scratchBufferSize, contexts));

            // if resource allocator/deallocator was provided, set them
            if (backendDesc->pfnFfxResourceAllocator && backendDesc->pfnFfxResourceDeallocator)
            {
                ffxRegisterResourceAllocatorX(backendDesc->pfnFfxResourceAllocator);
                ffxRegisterResourceDeallocatorX(backendDesc->pfnFfxResourceDeallocator);
            }

            // if constant buffer allocator was provided, set it
            if (backendDesc->pfnFfxConstantBufferAllocator) {
                ffxRegisterConstantBufferAllocatorX(backendDesc->pfnFfxConstantBufferAllocator);
            }
            break;
        }
        }
    }
    return FFX_API_RETURN_OK;
}

void* GetDevice(const ffxApiHeader* desc)
{
    for (const auto* it = desc; it; it = it->pNext)
    {
        switch (it->type)
        {
        case FFX_API_QUERY_DESC_TYPE_GET_VERSIONS:
        {
            return reinterpret_cast<const ffxQueryDescGetVersions*>(it)->device;
        }
        case FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_XBOX:
        {
            return reinterpret_cast<const ffxCreateBackendDescXbox*>(it)->device;
        }
        }
    }
    return nullptr;
}
