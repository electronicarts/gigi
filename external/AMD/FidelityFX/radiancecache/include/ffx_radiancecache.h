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
#include "../../api/include/ffx_api.h"
#include "../../api/include/ffx_api_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define FFX_RADIANCECACHE_VERSION_MAJOR 0
#define FFX_RADIANCECACHE_VERSION_MINOR 9
#define FFX_RADIANCECACHE_VERSION_PATCH 0

#define FFX_RADIANCECACHE_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))
#define FFX_RADIANCECACHE_VERSION FFX_RADIANCECACHE_MAKE_VERSION(FFX_RADIANCECACHE_VERSION_MAJOR, FFX_RADIANCECACHE_VERSION_MINOR, FFX_RADIANCECACHE_VERSION_PATCH)

/// Configuration flags supplied to ffxCreateContextDescRadianceCache.
/// 
/// @ingroup ffxNRC
typedef enum ffxApiCreateContextRadianceCacheFlags  
{
    FFX_RADIANCE_CACHE_CONTEXT_TRY_FORCE_WMMA             = (1u << 0),  ///< Tries to force the context to use the WMMA backend. If WMMA is not available, ffx::CreateContext will fail with ffx::ReturnCode::ErrorParameter.

} ffxApiCreateContextRadianceCacheFlags;

#define FFX_API_CREATE_CONTEXT_DESC_TYPE_RADIANCECACHE FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_RADIANCECACHE, 0x02)

/// A structure encapsulating the parameters required to initialize a FidelityFX neural radiance cache
///
/// @ingroup ffxNRC
typedef struct ffxCreateContextDescRadianceCache
{
    ffxCreateContextDescHeader              header;
    uint32_t                                flags;                      ///< A collection of <c><i>FfxNRCInitializationFlagBits</i></c>.
    uint32_t                                version;                    ///< The version of the API the application was built against. This must be set to FFX_RADIANCECACHE_VERSION.
    uint32_t                                maxInferenceSampleCount;    ///< The maximum number of inference samples (usually corresponds to the viewport resolution)
    uint32_t                                maxTrainingSampleCount;     ///< The maximum number of training samples
} ffxCreateContextDescRadianceCache;

#define FFX_API_DISPATCH_DESC_TYPE_RADIANCECACHE FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_RADIANCECACHE, 0x04)

/// Configuration flags supplied to ffxDispatchDescRadianceCache.
/// 
/// @ingroup ffxNRC
typedef enum ffxApiDispatchRadianceCacheFlags
{
    FFX_RADIANCE_CACHE_DISPATCH_INFERENCE           = (1u << 0),  ///< Signals that an inference pass should be dispatched. This is executed <i>before</i> the training pass (if applicable.)
    FFX_RADIANCE_CACHE_DISPATCH_TRAINING            = (1u << 1),  ///< Signals that a training pass should be dispatched. This is executed <i>after</i> the inference pass (if applicable.)

    FFX_RADIANCE_CACHE_CLEAR_INFERENCE_COUNTER      = (1u << 2),  ///< Signals that the counter defining the number of inference samples should be cleared <i>after</i> inference/training has completed
    FFX_RADIANCE_CACHE_CLEAR_TRAINING_COUNTER       = (1u << 3),  ///< Signals that the counter defining the number of training samples should be cleared. This is always executed <i>after</i> inference/training has completed
    FFX_RADIANCE_CACHE_CLEAR_ALL_COUNTERS           = (FFX_RADIANCE_CACHE_CLEAR_INFERENCE_COUNTER | FFX_RADIANCE_CACHE_CLEAR_TRAINING_COUNTER),  

    FFX_RADIANCE_CACHE_RESET                        = (1u << 4),  ///< Signals that the cache should be reset to its default start-up parameters (not including the sample counters.) This is executed <i>before</i> inference/training.

    FFX_RADIANCE_CACHE_OVERRIDE_LEARNING_RATE       = (1u << 5),  ///< Signals that the learning rate should be overridden by the value in overrides::learningRate
    FFX_RADIANCE_CACHE_OVERRIDE_WEIGHT_SMOOTHING    = (1u << 6)   ///< Signals that the smoothing factor should be overridden by the value in overrides::weightSmoothing

} ffxApiDispatchRadianceCacheFlags;

/// A structure encapsulating the parameters for dispatching inference and training passes for FidelityFX Neural Radiance Cache.
///
/// @ingroup ffxNRC
typedef struct ffxDispatchDescRadianceCache
{
    ffxDispatchDescHeader                   header;             
    void*                                   commandList;        ///< A pointer to an open command list
    FfxApiResource                          predictionInputs;   ///< A handle to a buffer containing prediction input data. 
    FfxApiResource                          predictionOutputs;  ///< A handle to a buffer containing prediction output data. This buffer is populated by the results from the inference dispatch.
    FfxApiResource                          trainInputs;        ///< A handle to a buffer containing training input data. 
    FfxApiResource                          trainTargets;       ///< A handle to a buffer containing training target data. 
    FfxApiResource                          sampleCounters;     ///< A handle to a buffer indicating the number of samples in the prediction and training buffers
    uint32_t                                flags;              ///< A collection of <c><i>ffxApiDispatchRadianceCacheFlags</i></c>    
    struct 
    {
        float learningRate;                                     ///< Overrides the learning weight of the model. The flag FFX_RADIANCE_CACHE_OVERRIDE_LEARNING_RATE must be specified for this to take effect.
        float weightSmoothing;                                  ///< Overrides the smoothing factor applied to the model weights. The flag FFX_RADIANCE_CACHE_OVERRIDE_WEIGHT_SMOOTHING must be specified for this to take effect.
    } 
    overrides;
} ffxDispatchDescRadianceCache;

#define FFX_API_DEBUG_DISPATCH_DESC_TYPE_RADIANCECACHE FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_RADIANCECACHE, 0x05)

typedef struct ffxDebugDispatchDescRadianceCache
{
    ffxDispatchDescHeader                   header;
    void* commandList;        ///< A pointer to an open command list
    FfxApiResource                          debugTexture;       ///< A debug textures, and empty canvas for the debug artist
    FfxApiResource                          debugPixelInfo;     ///< A handle to a buffer with additional pixel information
} ffxDebugDispatchDescRadianceCache;

#if defined(__cplusplus)
}
#endif
