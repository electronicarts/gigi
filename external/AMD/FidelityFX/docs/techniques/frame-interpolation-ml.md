<!-- @page page_techniques_frame-interpolation-ml FSR™ Frame Interpolation 4.0.0 -->

<h1>AMD FSR™ Frame Generation 4.0.0</h1>

<h2>Table of contents</h2>

- [Introduction](#introduction)
    - [Shading language requirements](#shading-language-requirements)
- [Integration](#integration)
    - [Performance](#performance)
    - [Memory Usage](#memory-usage)
    - [Call order](#call-order)
    - [Camera data](#camera-data)
    - [Debug View Unsupported](#debug-view-unsupported)
- [Limitations](#limitations)
- [Version history](#version-history)
- [See also](#see-also)

<h2>Introduction</h2>

**FSR™ Frame Generation** is an advanced frame-generating solution that leverages state-of-the-art machine learning algorithms to generate a high-quality intermediate frame from two consecutive source images, interpolating the motion of pixels between the start & end images. 

The frame generation context computes the interpolated image. Once this is accomplished, the interpolated and real back buffers still need to be used, i.e. usually sent to the swapchain. On the topic of how to handle presentation and pacing of the back buffers, please refer to the [frame generation swapchain](frame-interpolation-swap-chain.md) documentation.

<h3>Shading language requirements</h3>

- `HLSL`
  - `CS_6_6†`

† `CS_6_6` requires DirectX 12 Agility SDK 1.4.9 or later.

<h2>Integration</h2>

FSR™ Frame Generation should be integrated using the [FSR™ Frame Generation API](frame-interpolation-api.md). This document describes API and other aspects specific to AMD FSR™ Frame Generation 4.0.0.

<h3>Performance</h3>
Depending on your target hardware and operating configuration FSR™ Frame Generation will operate at different performance levels. Below you can find reference performance numbers for RX 9070 XT & RX 9060 XT running at recommended resolutions.

| GPU               | Target Resolution | Microseconds (ms)|
| -----------       | -----------       |------------------|
| RX 9070 XT        | 3840x2160         |  2.2ms           |
| RX 9060 XT        | 2560x1440         |  2.1ms           |

<h3>Memory Usage</h3>

Figures are given to the nearest MB, taken on Radeon RX 9070 XT using DirectX 12, and are subject to change. Does not include frame generation swapchain overheads.

| Output resolution | Total Memory usage(MB)          | Memory aliasable(MB)            |
|-------------------|---------------------------------|---------------------------------|
| 3840x2160         | 410                             | 360                             |
| 2560x1440         | 313                             | 284                             |
| 1920x1080         | 111                             | 90                              |

<h3>Call order</h3>

For FSR™ Frame Generation 4.0.0 it is *required* that the application follow the specified call-order:
1. [`ffxConfigure`](../../api/include/ffx_api.h#L144) with a filled in [`ffxConfigureDescFrameGeneration`](../../framegeneration/include/ffx_framegeneration.h#L104).
2. [`ffxDispatch`](../../api/include/ffx_api.h#L155) with a filled in [`ffxDispatchDescFrameGenerationPrepareV2`](../../framegeneration/include/ffx_framegeneration.h#L122).
3. [`ffxDispatch`](../../api/include/ffx_api.h#L155) with a filled in [`ffxDispatchDescFrameGeneration`](../../framegeneration/include/ffx_framegeneration.h#L86).

With FSR™ Frame Generation 4.0.0 this call order is mandatory as data provided in each call is required for the next to function correctly. Calling these functions out of order may result in incorrect rendering results and *prohibits* future automatic driver upgrades.

<h3>Camera data</h3>

FSR™ Frame Generation 4.0.0 uses the [`cameraPosition`](../../framegeneration/include/ffx_framegeneration.h#L248), [`cameraUp`](../../framegeneration/include/ffx_framegeneration.h#L249), [`cameraRight`](../../framegeneration/include/ffx_framegeneration.h#L250) and [`cameraForward`](../../framegeneration/include/ffx_framegeneration.h#L251) fields within the [`ffxDispatchDescFrameGenerationPrepareV2`](../../framegeneration/include/ffx_framegeneration.h#L231) structure. Failing to specify these fields, providing incorrect or low-precision values may result in incorrect rendering.

<h3>Debug View Unsupported</h3>

The [`FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW`](../../framegeneration/include/ffx_framegeneration.h#L49) setting is not supported by FSR™ Frame Generation 4.0.0.

All other debug functions are provided.

<h2>Limitations</h2>

FSR™ Frame Generation requires Windows 11, DirectX 12 Agility SDK 1.4.9 and an AMD 9000 series GPU or later.

<h2>Version history</h2>

| Version        | Date              |
| ---------------|-------------------|
| **4.0.0**      | 2025-12-10        |

Refer to changelog for more detail on versions.

<h2>See also</h2>

- [FSR™ Frame Generation API](frame-interpolation-api.md)
- [FidelityFX™ Super Resolution Frame Generation 3.1.6](frame-interpolation.md)
- [FidelityFX™ Super Resolution Frame Generation Swapchain 3.1.6](frame-interpolation-swap-chain.md)
- [FidelityFX™ FSR Sample](../../../../docs/samples/super-resolution.md)
- [FSR™ Naming guidelines](../getting-started/naming-guidelines.md)
