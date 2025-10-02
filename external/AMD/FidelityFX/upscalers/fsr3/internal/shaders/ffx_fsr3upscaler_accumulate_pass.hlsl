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

#define FSR3UPSCALER_BIND_SRV_INPUT_EXPOSURE                        0
#define FSR3UPSCALER_BIND_SRV_DILATED_REACTIVE_MASKS                1
#if FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
#define FSR3UPSCALER_BIND_SRV_DILATED_MOTION_VECTORS                2
#else
#define FSR3UPSCALER_BIND_SRV_INPUT_MOTION_VECTORS                  2
#endif
#define FSR3UPSCALER_BIND_SRV_INTERNAL_UPSCALED                     3
#define FSR3UPSCALER_BIND_SRV_LANCZOS_LUT                           4
#define FSR3UPSCALER_BIND_SRV_FARTHEST_DEPTH_MIP1                   5

#define FSR3UPSCALER_BIND_SRV_CURRENT_LUMA                          6
#define FSR3UPSCALER_BIND_SRV_LUMA_INSTABILITY                      7
#define FSR3UPSCALER_BIND_SRV_INPUT_COLOR                           8

#define FSR3UPSCALER_BIND_UAV_INTERNAL_UPSCALED                     0
#define FSR3UPSCALER_BIND_UAV_UPSCALED_OUTPUT                       1
#define FSR3UPSCALER_BIND_UAV_NEW_LOCKS                             2

#define FSR3UPSCALER_BIND_CB_FSR3UPSCALER                           0

#if defined(__XBOX_SCARLETT)
    // Enable extra optimisations for 16-bit math to help the compiler with auto-vectorisation
    #define __XBATG_EXTRA_16_BIT_OPTIMISATION 1

    // Force wave32 on Xbox
    #define __XBOX_ENABLE_WAVE32 1

    #if defined(FFX_FSR3_GDK_VERSION)
        #if FFX_FSR3_GDK_VERSION == 221000
            // In GDK Version == 221000, this define is preferrable to enable VMem operation batching
            #define __XBOX_ENABLE_SCHEDULER_CLAUSING 31
        #elif FFX_FSR3_GDK_VERSION > 221000
            // In GDK Version > 221000, this define is preferrable to enable VMem operation batching
            #define __XBOX_PARALLEL_MEM_READS 31
        #endif
    #endif
#endif

#include "fsr3upscaler/ffx_fsr3upscaler_callbacks_hlsl.h"
#include "fsr3upscaler/ffx_fsr3upscaler_common.h"
#include "fsr3upscaler/ffx_fsr3upscaler_sample.h"
#include "fsr3upscaler/ffx_fsr3upscaler_upsample.h"
#include "fsr3upscaler/ffx_fsr3upscaler_reproject.h"
#include "fsr3upscaler/ffx_fsr3upscaler_accumulate.h"

#ifndef FFX_FSR3UPSCALER_THREAD_GROUP_WIDTH
#define FFX_FSR3UPSCALER_THREAD_GROUP_WIDTH 8
#endif // #ifndef FFX_FSR3UPSCALER_THREAD_GROUP_WIDTH
#ifndef FFX_FSR3UPSCALER_THREAD_GROUP_HEIGHT
#define FFX_FSR3UPSCALER_THREAD_GROUP_HEIGHT 8
#endif // FFX_FSR3UPSCALER_THREAD_GROUP_HEIGHT
#ifndef FFX_FSR3UPSCALER_THREAD_GROUP_DEPTH
#define FFX_FSR3UPSCALER_THREAD_GROUP_DEPTH 1
#endif // #ifndef FFX_FSR3UPSCALER_THREAD_GROUP_DEPTH
#ifndef FFX_FSR3UPSCALER_NUM_THREADS
	#if defined(__XBOX_SCARLETT) && defined(__XBATG_EXTRA_16_BIT_OPTIMISATION) && (__XBATG_EXTRA_16_BIT_OPTIMISATION == 1)
        #define FFX_FSR3UPSCALER_NUM_THREADS [numthreads(FFX_FSR3UPSCALER_THREAD_GROUP_WIDTH, FFX_FSR3UPSCALER_THREAD_GROUP_HEIGHT / 2, FFX_FSR3UPSCALER_THREAD_GROUP_DEPTH)]
    #else
        #define FFX_FSR3UPSCALER_NUM_THREADS [numthreads(FFX_FSR3UPSCALER_THREAD_GROUP_WIDTH, FFX_FSR3UPSCALER_THREAD_GROUP_HEIGHT, FFX_FSR3UPSCALER_THREAD_GROUP_DEPTH)]
    #endif // #if defined(__XBOX_SCARLETT) && defined(__XBATG_EXTRA_16_BIT_OPTIMISATION) && (__XBATG_EXTRA_16_BIT_OPTIMISATION == 1)
#endif // #ifndef FFX_FSR3UPSCALER_NUM_THREADS

FFX_FSR3UPSCALER_NUM_THREADS
FFX_FSR3UPSCALER_EMBED_ROOTSIG_CONTENT
#if defined(__XBOX_SCARLETT)
void CS(int2 iGroupId : SV_GroupId, int2 iGroupThreadId : SV_GroupThreadId)
{
    int2 iDispatchThreadId = iGroupId * int2(FFX_FSR3UPSCALER_THREAD_GROUP_WIDTH, FFX_FSR3UPSCALER_THREAD_GROUP_HEIGHT) + iGroupThreadId;
    Accumulate(iDispatchThreadId);

#if defined(__XBOX_SCARLETT) && defined(__XBATG_EXTRA_16_BIT_OPTIMISATION) && (__XBATG_EXTRA_16_BIT_OPTIMISATION == 1)
    iDispatchThreadId.y += FFX_FSR3UPSCALER_THREAD_GROUP_HEIGHT / 2;
    Accumulate(iDispatchThreadId);
#endif
}
#else
FFX_PREFER_WAVE64
void CS(int2 iDispatchThreadId : SV_DispatchThreadID)
{
    Accumulate(iDispatchThreadId);
}
#endif // #if defined(__XBOX_SCARLETT)
