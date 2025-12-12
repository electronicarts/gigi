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

#include "FrameInterpolationSwapchainXbox_Helpers.h"
#include "../include/ffx_frameinterpolation.h"

void waitForPerformanceCount(const int64_t targetCount)
{
    int64_t currentCount = 0;
    do
    {
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentCount));
    } while (currentCount < targetCount);
}

bool waitForFenceValue(ID3D12Fence* fence, UINT64 value, DWORD dwMilliseconds)
{
    bool status = false;

    if (fence)
    {
        if (dwMilliseconds == INFINITE)
        {
            while (fence->GetCompletedValue() < value);
            status = true;
        }
        else
        {
            status = fence->GetCompletedValue() >= value;

            if (!status)
            {
                HANDLE handle = CreateEvent(0, false, false, 0);

                if (isValidHandle(handle))
                {
                    //Wait until command queue is done.
                    if (!status)
                    {
                        if (SUCCEEDED(fence->SetEventOnCompletion(value, handle)))
                        {
                            status = (WaitForSingleObject(handle, dwMilliseconds) == WAIT_OBJECT_0);
                        }
                    }

                    CloseHandle(handle);
                }
            }
        }
    }

    return status;
}

inline bool isValidHandle(HANDLE handle)
{
    return handle != NULL;
}
