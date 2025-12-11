<!-- @page page_getting-started_index Introduction to the MD FSR™ SDK -->

<h1>Introduction to the AMD FSR™ SDK</h1>

The AMD FSR™ SDK is a collection of highly optimized effects (headers and DLLs) that developers can integrate into DirectX®12 applications to enhance performance and visual quality. The AMD FSR™ SDK includes:

- [FidelityFX™ Super Resolution 2.3.4](../techniques/super-resolution-temporal.md)
- [FidelityFX™ Super Resolution 3.1.5](../techniques/super-resolution-upscaler.md)
- [FSR™ Upscaling 4.0.3](../techniques/super-resolution-ml.md)
- [FidelityFX™ Super Resolution Frame Generation 3.1.6](../techniques/frame-interpolation.md)
- [FidelityFX™ Super Resolution Frame Generation Swapchain 3.1.6](../techniques/frame-interpolation-swap-chain.md)
- [FSR™ Frame Generation 4.0.0](../techniques/frame-interpolation-ml.md)
- [FSR™ Ray Regeneration 1.0.0](../techniques/denoising.md)
- [FSR™ Radiance Caching (Technical Preview)](../techniques/radiance-cache.md)

<h2>Supported ecosystems</h2>

This version of the AMD FSR™ SDK comes with samples that run on the following APIs:

- DirectX®12

The shader code used by our sample framework is written in HLSL, and can easily be ported to other platforms which support modern shader models.

If you are a registered Xbox developer, you can find AMD FSR™ features available as part of the Microsoft Game Development Kit (GDK).

<h2>Samples</h2>

Most samples are written in C++, and use the [Cauldron Framework](../../Kits/Cauldron2/dx12/) sample framework.

<h2>Open source</h2>

The AMD FSR™ SDK is open source, and distributed under the MIT license.

For more information on the license terms please refer to the [license](../license.md).

<h2>Support</h2>

AMD maintains the FSR™ SDK with regular updates, new features, bug fixes, and compatibility and performance testing across a wide range of hardware.

If you encounter issues or have feature requests, please consider opening an issue.

<!-- - @subpage page_getting-started_sdk-structure "SDK Structure" -->
<!-- - @subpage page_getting-started_naming-guidelines "FSR™ naming guidelines for game applications" -->
<!-- - @subpage page_ffx-api "Introduction to FSR™ API" -->
