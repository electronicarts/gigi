<!-- @page page_whats-new_index AMD FSR™ SDK: What's new in FSR™ SDK 2.1.0 -->

<h1>What's new in the AMD FSR™ SDK 2.1.0?</h1>

Welcome to the AMD FSR™ SDK. This revision to the SDK introduces the AMD FSR™ Redstone DLL-based ML technologies. 

<h2>New effects and features</h2>

<h3>AMD FSR™ Frame Generation 4.0.0</h3>
Introducing our new inference-based FSR™ Frame Generation, an advanced frame-generating solution that leverages state-of-the-art machine learning algorithms to generate high-quality interpolated frames.

<h3>AMD FSR™ Ray Regeneration 1.0.0</h3>
Introducing our new inference-based FSR™ Ray Regeneration, an advanced denoising solution that leverages state-of-the-art machine learning algorithms to generate high-quality output from noisy source inputs.

<h3>AMD FSR™ Radiance Caching (Preview)</h3>
Introducing our new inference-based FSR™ Radiance Caching, an advanced path tracing caching solution that leverages state-of-the-art machine learning algorithms to facilitate high-performance path tracing calculations.

<h2>Updated effects</h2>

<h3>AMD FSR™ Upscaling 4.0.3</h3>
* Fixed a rendering error when surface dimensions are not multiples of 8.<br/>
* API changes:<br/>
- Added ffxCreateContextDescUpscaleVersion which must be linked to ffxCreateContextDescUpscale when creating a context to ensure correct behavior with future updates.<br/>

<h3>AMD FidelityFX™ Super Resolution Frame Generation 3.1.6</h3>
* API changes:<br/>
- Deprecated ffxDispatchDescFrameGenerationPrepare & ffxDispatchDescFrameGenerationPrepareCameraInfo, replaced by ffxDispatchDescFrameGenerationPrepareV2.<br/>
- Added ffxCreateContextDescFrameGenerationVersion which must be linked to ffxCreateContextDescFrameGeneration when creating a context to ensure correct behavior with future updates.<br/>

<h3>AMD FidelityFX™ Super Resolution Frame Generation Swapchain 3.1.6</h3>
* Modified internal implementation to ensure future ABI compatibility.<br/>
* API changes:<br/>
- Added ffxCreateContextDescFrameGenerationSwapChainVersionDX12 which must be linked to ffxCreateContextDescFrameGenerationSwapChain*DX12 when creating a context to ensure correct behavior with future updates.<br/>

<h3>AMD FSR™ API</h3>
* Minor non-API breaking additions:<br/>

<h2>Updated Components</h2>

Starting with AMD FidelityFX™ SDK 2.0.0 the effects, previously combined in amd_fidelityfx_dx12.dll, are split into multiple DLLs based on effect type. Please see [Introduction to FSR™ API](../getting-started/ffx-api.md#dlls-structure) for details.
<br/>
<br/>
PDBs are provided for the effects DLLs.

<h2>Updated documentation</h2>

* FSR™ 4 documentation.

<h2>Deprecated effects</h2>

None.

<h2>Deprecated components</h2>

All SDK version 1 effects are now deprecated to that version of the SDK. 
For any pre-existing FidelityFX features (including the legacy FidelityFX Super Resolution sample), please refer to FidelityFX SDK 1.1.4.

<!-- - @subpage page_whats-new_index_2_1_0 "AMD FSR™ SDK: What's new in FSR™ SDK 2.1.0" -->