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

#define FFX_DENOISER_VERSION_MAJOR 1
#define FFX_DENOISER_VERSION_MINOR 0
#define FFX_DENOISER_VERSION_PATCH 0

#define FFX_DENOISER_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))
#define FFX_DENOISER_VERSION FFX_DENOISER_MAKE_VERSION(FFX_DENOISER_VERSION_MAJOR, FFX_DENOISER_VERSION_MINOR, FFX_DENOISER_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FfxApiCreateContextDenoiserFlags
{
    FFX_DENOISER_ENABLE_DEBUGGING      = (1 << 0),          ///< A bit indicating that debug features may be enabled, memory consumption may increase.
    FFX_DENOISER_ENABLE_DOMINANT_LIGHT = (1 << 1),          ///< A bit indicating that dominant light visibility denoising should be enabled. This requires the dominant light direction and emission to be provided.
} FfxApiCreateContextDenoiserFlags;

typedef enum FfxApiDenoiserMode
{
    FFX_DENOISER_MODE_4_SIGNALS,                            ///< The denoiser expects 4 split radiance signals as input to perform denoising on. (Direct Specular, Direct Diffuse, Indirect Specular, Indirect Diffuse)
    FFX_DENOISER_MODE_2_SIGNALS,                            ///< The denoiser expects 2 fused radiance signals as input to perform denoising on. (Specular, Diffuse)
    FFX_DENOISER_MODE_1_SIGNAL,                             ///< The denoiser expects 1 fused radiance signal as input to perform denoising on. (Composited)
} FfxApiDenoiserMode;

#define FFX_API_CREATE_CONTEXT_DESC_TYPE_DENOISER FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x01)
typedef struct ffxCreateContextDescDenoiser
{
    ffxCreateContextDescHeader header;
    uint32_t                   version;                     ///< The version of the API the application was built against. This must be set to <c>FFX_DENOISER_VERSION</c>.
    struct FfxApiDimensions2D  maxRenderSize;               ///< The maximum size that rendering will be performed at.
    ffxApiMessage              fpMessage;                   ///< A pointer to a function that can receive messages from the runtime. May be null.
    FfxApiDenoiserMode         mode;                        ///< An entry of <c>FfxApiDenoiserMode</c> that selects the number of signals to denoise.
    uint32_t                   flags;                       ///< Zero or a combination of values from <c>FfxApiCreateContextDenoiserFlags</c>.
} ffxCreateContextDescDenoiser;

typedef enum FfxApiDispatchDenoiserFlags
{
    FFX_DENOISER_DISPATCH_RESET               = (1 << 0),   ///< A bit indicating that the we need to reset history accumulation.
    FFX_DENOISER_DISPATCH_NON_GAMMA_ALBEDO    = (1 << 1),   ///< A bit indicating that the input albedo textures are not gamma encoded.
} FfxApiDispatchDenoiserFlags;

typedef struct FfxApiDenoiserSignal
{
    struct FfxApiResource input;                            ///< Input signal to be denoised.
    struct FfxApiResource output;                           ///< Resulting denoised signal.

    uint32_t _reserved[2];                                  ///< Reserved for future use.
} FfxApiDenoiserSignal;

typedef struct FfxApiDenoiserSettings
{
    float historyRejectionStrength;                         ///< The strength of the history rejection test.
    float crossBilateralNormalStrength;                     ///< The strength of the cross bilateral normal term.
    float stabilityBias;                                    ///< Biases the temporal accumulation to be more stable but less responsive.
    float maxRadiance;                                      ///< Maximum radiance value.
	float radianceClipStdK;                                 ///< The standard deviation K value used for radiance clipping.
	float gaussianKernelRelaxation;                         ///< The Gaussian kernel relaxation factor.

    uint32_t _reserved[26];                                 ///< Reserved for future use.
} FfxApiDenoiserSettings;

#define FFX_API_DISPATCH_DESC_TYPE_DENOISER FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x02)
typedef struct ffxDispatchDescDenoiser
{
    ffxDispatchDescHeader      header;
    void*                      commandList;                 ///< Command list to record upscaling rendering commands into.
                               
    struct FfxApiResource      linearDepth;                 ///< R:   Linear depth values for the current frame.
    struct FfxApiResource      motionVectors;               ///< RGB: 2D motion vectors, B: Linear Depth delta
    struct FfxApiResource      normals;                     ///< RG:  Octahedrally encoded normals, B: Linear Roughness, A: Material Type - See docs for more info, 
    struct FfxApiResource      specularAlbedo;              ///< RGB: Specular albedo, A: NoV - sqrt encoding assumed unless <c>FFX_DENOISER_DISPATCH_NON_GAMMA_ALBEDO</c> is provided. (sqrt(specular_albedo, NoV))
    struct FfxApiResource      diffuseAlbedo;               ///< RGB: Diffuse albedo, A: metalness - sqrt encoding assumed unless <c>FFX_DENOISER_DISPATCH_NON_GAMMA_ALBEDO</c> is provided. (sqrt(diffuse_albedo, metalness))
                                                            
    struct FfxApiFloatCoords2D motionVectorScale;           ///< The scale factor to apply to the 2D motion vectors.
    struct FfxApiFloatCoords2D jitterOffsets;               ///< The subpixel jitter offset applied to the camera projection.
                                                            
    struct FfxApiFloatCoords3D cameraPositionDelta;         ///< The position delta of the camera since last frame. (PrevPos - CurrentPos)
    struct FfxApiFloatCoords3D cameraRight;                 ///< The right vector of the camera in world space.
    struct FfxApiFloatCoords3D cameraUp;                    ///< The up vector of the camera in world space.
    struct FfxApiFloatCoords3D cameraForward;               ///< The forward vector of the camera in world space.
    float                      cameraAspectRatio;           ///< The aspect ratio of the camera.
    float                      cameraNear;                  ///< The view z distance to the near plane of the camera.
    float                      cameraFar;                   ///< The view z distance to the far plane of the camera.
    float                      cameraFovAngleVertical;      ///< The vertical field of view of the camera. (Specified in radians)
                                                            
    struct FfxApiDimensions2D  renderSize;                  ///< The resolution that was used for rendering the input resources.
    float                      deltaTime;                   ///< The time, in milliseconds, since the last frame was rendered.
    uint32_t                   frameIndex;                  ///< The index of the current frame.
                                                            
    uint32_t                   flags;                       ///< Zero or a combination of values from <c>FfxApiDispatchDenoiserFlags</c>.
} ffxDispatchDescDenoiser;

#define FFX_API_DISPATCH_DESC_INPUT_4_SIGNALS_TYPE_DENOISER FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x03)
typedef struct ffxDispatchDescDenoiserInput4Signals        ///< Requires FFX_DENOISER_MODE_4_SIGNALS to be set in the create flags.
{
    ffxDispatchDescHeader       header;
    struct FfxApiDenoiserSignal indirectSpecularRadiance;  ///< Indirect specular radiance signal in/out. Input alpha channel should contain specular ray length.
    struct FfxApiDenoiserSignal indirectDiffuseRadiance;   ///< Indirect diffuse radiance signal in/out.
    struct FfxApiDenoiserSignal directSpecularRadiance;    ///< Direct specular radiance signal in/out.
    struct FfxApiDenoiserSignal directDiffuseRadiance;     ///< Direct diffuse radiance signal in/out.
} ffxDispatchDescDenoiserInput4Signals;

#define FFX_API_DISPATCH_DESC_INPUT_2_SIGNALS_TYPE_DENOISER FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x04)
typedef struct ffxDispatchDescDenoiserInput2Signals         ///< Requires FFX_DENOISER_MODE_2_SIGNALS to be set in the create flags.
{
    ffxDispatchDescHeader       header;
    struct FfxApiDenoiserSignal specularRadiance;           ///< Specular radiance signal in/out. Input alpha channel should contain specular ray length.
    struct FfxApiDenoiserSignal diffuseRadiance;            ///< Diffuse radiance signal in/out.
} ffxDispatchDescDenoiserInput2Signals;

#define FFX_API_DISPATCH_DESC_INPUT_1_SIGNAL_TYPE_DENOISER FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x05)
typedef struct ffxDispatchDescDenoiserInput1Signal          ///< Requires FFX_DENOISER_MODE_1_SIGNAL to be set in the create flags.
{
    ffxDispatchDescHeader       header;
    struct FfxApiDenoiserSignal radiance;                   ///< Composited radiance signal in/out. Input alpha channel should contain specular ray length.
    struct FfxApiResource       fusedAlbedo;                ///< RGB: max(specularAlbedo, diffuseAlbedo) A: NoV - sqrt encoding assumed unless <c>FFX_DENOISER_DISPATCH_NON_GAMMA_ALBEDO</c> is provided. (sqrt(fused_albedo, NoV))
} ffxDispatchDescDenoiserInput1Signal;

#define FFX_API_DISPATCH_DESC_INPUT_DOMINANT_LIGHT_TYPE_DENOISER FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x07)
typedef struct ffxDispatchDescDenoiserInputDominantLight    ///< Requires <c>FFX_DENOISER_ENABLE_DOMINANT_LIGHT</c> to be set in the create flags.
{
    ffxDispatchDescHeader       header;
    struct FfxApiDenoiserSignal dominantLightVisibility;    ///< Dominant light visibility signal in/out, input should be described as distance to occluder (0 -> FP16_MAX).
    struct FfxApiFloatCoords3D  dominantLightDirection;     ///< Dominant light direction. (from light source to target) 
    struct FfxApiFloatCoords3D  dominantLightEmission;      ///< Dominant light emission. (emission == (lightColor * lightIntensity))
} ffxDispatchDescDenoiserInputDominantLight;

#define FFX_API_CONFIGURE_DESC_TYPE_DENOISER_SETTINGS FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x08)
typedef struct ffxConfigureDescDenoiserSettings
{
    ffxConfigureDescHeader        header;
    struct FfxApiDenoiserSettings settings;                 ///< Values to set.
} ffxConfigureDescDenoiserSettings;

#define FFX_API_QUERY_DESC_TYPE_DENOISER_GPU_MEMORY_USAGE FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x09)
typedef struct ffxQueryDescDenoiserGetGPUMemoryUsage        ///< If a valid FfxContext is passed into ffx::Query with this structure, current context usage will be reported. If no context is passed, the memory usage will be estimated based on the provided parameters.
{
    ffxQueryDescHeader        header;
    void*                     device;                       ///< For DX12: pointer to ID3D12Device. For Vulkan: pointer to VkDevice.
    struct FfxApiDimensions2D maxRenderSize;                ///< Maximum size that rendering will be performed at.
    FfxApiDenoiserMode        mode;                         ///< An entry of <c>FfxApiDenoiserMode</c> that selects the number of signals to denoise.
    uint32_t                  flags;                        ///< Zero or a combination of values from <c>FfxApiCreateContextDenoiserFlags</c>.

    struct FfxApiEffectMemoryUsage* gpuMemoryUsage;         ///< A pointer to a <c>FfxApiEffectMemoryUsage</c> structure that will hold the GPU memory usage.
} ffxQueryDescDenoiserGetGPUMemoryUsage;

#define FFX_API_QUERY_DESC_TYPE_DENOISER_GET_VERSION FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x0a)
typedef struct ffxQueryDescDenoiserGetVersion               ///< If a valid FfxContext is passed into ffx::Query with this structure, info based on current context will be reported. If no context is passed, the info will be evaluated based on the provided parameters.
{
    ffxConfigureDescHeader  header;
    void*                   device;                         ///< For DX12: pointer to ID3D12Device. For Vulkan: pointer to VkDevice.

    uint32_t*               major;                          ///< A pointer to a <c>uint32_t</c> variable that will hold the major version number.
    uint32_t*               minor;                          ///< A pointer to a <c>uint32_t</c> variable that will hold the minor version number.
    uint32_t*               patch;                          ///< A pointer to a <c>uint32_t</c> variable that will hold the patch version number.
} ffxQueryDescDenoiserGetVersion;

#define FFX_API_QUERY_DESC_TYPE_DENOISER_GET_DEFAULT_SETTINGS FFX_API_MAKE_EFFECT_SUB_ID(FFX_API_EFFECT_ID_DENOISER, 0x0b)
typedef struct ffxQueryDescDenoiserGetDefaultSettings       ///< If a valid FfxContext is passed into ffx::Query with this structure, info based on current context will be reported. If no context is passed, the info will be evaluated based on the provided parameters.
{
    ffxConfigureDescHeader         header;
    void*                          device;                  ///< For DX12: pointer to ID3D12Device. For Vulkan: pointer to VkDevice.

    struct FfxApiDenoiserSettings* defaultSettings;         ///< A pointer to a <c>FfxApiDenoiserSettings</c> structure that will hold the default setting values.
} ffxQueryDescDenoiserGetDefaultSettings;

#ifdef __cplusplus
}
#endif
