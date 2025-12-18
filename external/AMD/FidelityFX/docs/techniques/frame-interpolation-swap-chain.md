<!-- @page page_techniques_frame-interpolation-swap-chain AMD FidelityFX™ Super Resolution Frame Generation Swapchain 3.1.6 -->

<h1>AMD FidelityFX™ Super Resolution Frame Generation Swapchain 3.1.6</h1>

<h2>Table of contents</h2>

- [Introduction](#introduction)
- [Description](#description)
- [Integration](#integration)
    - [New APIs](#new-apis)
    - [Context creation](#context-creation)
    - [Recording and dispatching the frame interpolation workload](#recording-and-dispatching-the-frame-interpolation-workload)
    - [UI composition](#ui-composition)
    - [Memory Usage](#memory-usage)
    - [Waitable Object](#waitable-object)
    - [Frame pacing and presentation](#frame-pacing-and-presentation)
    - [Implementation Recommendations](#implementation-recommendations)
    - [Shutdown](#shutdown)
- [Additional Information](#additional-information)
- [Limitations](#limitations)
- [Version history](#version-history)
- [See also](#see-also)
    
<h2>Introduction</h2>

The `FSR™ Frame Generation Swapchain` implements the `IDXGISwapChain4` interface to provide an easy way to handle dispatching the workloads required for frame interpolation and pacing presentation.

Though this implementation may not work for all engines or applications, it was designed to provide an easy way to integrate `FSR™ Frame Generation` in a way that it is (almost) transparent to the underlying application.

<h2>Description</h2>

`FSR™ Frame Generation Swapchain` can be used as a replacement of the APIs swapchain and from the application point of view behavior should be similar.

When `FSR™ Frame Generation` is disabled, the main difference will be that present is slightly more expensive (one extra surface copy) compared to using the API swapchain directly.

In this case, the `FSR™ Frame Generation Swapchain` still supports handling the UI composition, so applications don't have to handle their UI differently when disabling `FSR™ Frame Generation`.

Internally the `FSR™ Frame Generation Swapchain` will create 2 additional CPU threads:

- The first thread is used to not stall the application while waiting for interpolation to finish on the GPU. After that the thread will get the current CPU time and compute pacing information.
- The second thread dispatches the GPU workloads for UI composition (calling the callback function if needed) and pacing the presents.

![alt text](media/frame-interpolation-swapchain/frame-interpolation-swap-chain.svg "A diagram showing the CPU workflow of the `FSR™ Frame Generation Swapchain`.")

<h2>Integration</h2>

The `FSR™ Frame Generation Swapchain` should be integrated using the [FidelityFX API](../getting-started/ffx-api.md), this document describes the API constructs specific to the `FSR™ Frame Generation Swapchain`.

`FSR™ Frame Generation Swapchain` for DirectX 12 implements the `IDXGISwapChain4` interface, so once created it behaves just like a normal swapchain.

<h2>New APIs</h2>

As of SDK 2.1 it is necessary to create and fill in a [`ffxCreateContextDescFrameGenerationSwapChainVersionDX12`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L129), setting the [`version`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L132) field to [`FFX_FRAMEGENERATION_SWAPCHAIN_DX12_VERSION`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L31) and link this in the `header.pNext` field of whichever context-creation structure is used. This is used to ensure compatibility between different versions of the API.

<h2>Context creation</h2>

Creation can be performed in several ways:
- Create and fill in a [`ffxCreateContextDescFrameGenerationSwapChainWrapDX12`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L29) structure then call [`ffxCreateContext`](../../api/include/ffx_api.h#L132). This will wrap & replace an existing swapchain.
- Create and fill in a [`ffxCreateContextDescFrameGenerationSwapChainNewDX12`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L37) structure then call [`ffxCreateContext`](../../api/include/ffx_api.h#L132). This will create a new swapchain from the given `dxgiFactory`.
- Create and fill in a [`ffxCreateContextDescFrameGenerationSwapChainForHwndDX12`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L47) structure then call [`ffxCreateContext`](../../api/include/ffx_api.h#L132). This will create a new swapchain from the provided `hwnd`, `desc`, `fullscreenDesc` and `dxgiFactory`.

<h3>Recording and dispatching the frame interpolation workload</h3>

The `FSR™ Frame Generation Swapchain` has been designed to be independent of the FidelityFX Frame Interpolation interface. To achieve this, it does not interact directly with those interfaces. The frame interpolation workload can be provided to the `FSR Frame Generation Swapchain` in 2 ways:

1. Provide a callback function [`frameGenerationCallback`](../../framegeneration/include/ffx_framegeneration.h#L110) in the [`ffxConfigureDescFrameGeneration`](../../framegeneration/include/ffx_framegeneration.h#L104).
   This function will get called from the `FSR™ Frame Generation Swapchain` during the call to `::Present` on the game thread, if frame interpolation is enabled to record the command list containing the frame interpolation workload.
2. Call [`ffxQueryDescFrameGenerationSwapChainInterpolationCommandListDX12`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L67) to obtain a command list from the `FSR™ Frame Generation Swapchain` and record the frame interpolation workload into it.
   In this case the command list will be executed when present is called.

The command list can either be executed on the same command queue present is being called on, or on an asynchronous compute queue: 
- Synchronous execution is more resilient to issues if an application calls upscale but then decides not to call present on a frame.
- Asynchronous execution may result in higher performance depending on the hardware and what workloads are running alongside the frame interpolation workload.

Either way, UI composition and present will be executed an a second graphics queue in order to not restrict UI composition to compute and allow the driver to schedule the present calls during preparation of the next frame.

Note: to ensure presents can execute at the time intended by the `FSR™ Frame Generation` frame pacing logic, avoid micro stuttering and assure good VRR response by the display, it is recommended to ensure the frame consists of multiple command list submissions.

<h3>UI Composition</h3>

When using `FSR™ Frame Generation`, it is highly advisable to treat the UI with special care, since distortion due to game motion vectors that would hardly be noticeable in 3D scenes will significantly impact readability of any UI text and result in very noticeable artifacts, especially on any straight, hard edges of the UI.

To combat any artifacts and keep the UI nice and readable, there are 3 ways to handle UI composition in the `FSR™ Frame Generation Swapchain`:

1. Register a call back function, which will render the UI on top of the back buffer.
   This function will get called for every back buffer presented (interpolated and real) so it allows the application to render UI animations at display rate or apply effects like film grain differently for each frame sent to the monitor. 
   However this approach obviously has some impact on performance as the UI will have to be rendered twice, so care should be taken to only record small workloads in the UI callback.
2. Render the UI to a separate surface, so it can be alpha-blended to the final back buffer.
   This way the UI can be applied to the interpolated and real back buffers without any distortion.

3. Provide a surface containing the HUD-less scene to the `FSR™ Frame Generation Swapchain` in addition to the final back buffer.
   In this case the frame interpolation shader will detect UI areas in the frame and suppress distortion in those areas.

<h3>Memory Usage</h3>

Figures are given to the nearest MB, taken on Radeon RX 9070 XTX using DirectX 12, and are subject to change.

| Output resolution | UI Mode           |  Memory usage(MB)                |
|-------------------|-------------------|---------------------------------|
| 3840x2160         | Separate UI surface + FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING  | 191                             |
| 3840x2160         | All other modes   | 159                             |
| 2560x1440         | Separate UI surface + FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING  | 90                             |
| 2560x1440         | All other modes   | 75                             |
| 1920x1080         | Separate UI surface + FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING  | 51                             |
| 1920x1080         | All other modes   | 42                             |


An application can get amount of GPU local memory required by `FSR™ Frame Generation Swapchain` context after context creation by calling [`ffxQuery`](../../api/include/ffx_api.h#L150) with the valid context and [`ffxQueryFrameGenerationSwapChainGetGPUMemoryUsageDX12`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L103).

An application can get GPU local memory required by default `FSR™ Frame Generation Swapchain` version before context creation by calling [`ffxQuery`](../../api/include/ffx_api.h#L150) with `NULL` context and filling out [`ffxQueryFrameGenerationSwapChainGetGPUMemoryUsageDX12V2`](../../framegeneration/include/dx12/ffx_api_framegeneration_dx12.h#L110). To get the memory requirement info for a different `FSR™ Frame Generation Swapchain` version, additionally link [`ffxOverrideVersion`](../../api/include/ffx_api.h#L99). 

See code examples how to call [Query](../getting-started/ffx-api.md#Query).

<h3>Waitable Object</h3>

It is advisable that the game uses GetFrameLatencyWaitableObject to get a waitable object, then use that object to prevent the CPU from running too far ahead of the GPU. This is espcially important when VSync is on on a low refresh rate monitor, as the GPU render rate can fall far below the CPU submission rate. Alternatively, the app can use a frame limiter set to half monitor refresh rate.

<h3>Frame pacing and presentation</h3>

The `FSR™ Frame Generation Swapchain` handles frame pacing automatically. Since Windows is not a real-time operating system and variable refresh rate displays are sensitive to timing imprecisions, the `FSR™ Frame Generation Swapchain` has been designed to use a busy wait loop in order to achieve the best possible timing behavior.

With `FSR™ Frame Generation` enabled, frames can take wildly different amounts of time to render. The workload for interpolated frames can be much smaller than for application rendered frames ("real" frames). It is therefore important to properly pace presentation of frames to ensure a smooth experience. The goal is to display each frame for an equal amount of time.

Presentation and pacing are done using two additional CPU threads separate from the main render loop. A high-priority pacing thread keeps track of average frame time, including UI composition time, and calculates the target presentation time delta. It also waits for GPU work to finish to avoid long GPU-side waits after the CPU-side presentation call.

To prevent any frame time spikes from impacting pacing too much, the moving average of several frames is used to estimate the frame time.

A present thread dispatches UI composition work for the generated frame, waits until the calculated present time delta has passed since the last presentation, then presents the generated frame. It repeats this for the real frame.

The application should ensure that the rendered frame rate is slightly below half the desired output frame rate. When VSync is enabled, the render performance will be implicitly limited to half the monitors maximum refresh rate.

It is recommended to use normal priority for any GPU queues created by the application to allow interpolation work to be scheduled with higher priority. In addition, developers should take care that command lists running concurrently with interpolation and composition are short (in terms of execution time) to allow presentation to be scheduled at a precise time.

![](media/frame-pacing/pacing-overview.svg)

<h4>Expected behavior</h4>

To further illustrate the pacing method and rationale behind it, the following sections will lay out expected behavior in different scenarios. We differentiate based on the post-interpolation frame rate as well as whether the display uses a fixed or variable refresh rate.

<h5>Fixed refresh rate</h5>

<h6>VSync enabled</h6>

Here, tearing is disabled and every frame is displayed for at least one sync interval. Presentation is synchronized to the display's vertical blanking period ("vsync"). This may result in uneven display timings and may increase input latency (by up to one refresh period).

![](media/frame-pacing/fixed-low.svg)

In the diagram, the first real frame is presented slightly after the vertical blanking interval, leading to the prior interpolated frame being shown for two refresh intervals and increased latency compared to immediate display.

<h6>VSync disabled</h6>

In this case, tearing is likely to occur. Presentation is not synchronized with the display. The benefit of this is reduced input latency compared to lower frame rates.

![](media/frame-pacing/fixed-high.svg)

<h5>Variable refresh rate</h5>

This section applies to display and GPU combinations with support for variable refresh rate (VRR) technologies, such as AMD FreeSync, NVIDIA G-SYNC® and VESA AdaptiveSync.

The timing between display refreshes is dictated by the variable refresh rate window. The delta time between two refreshes can be any time inside the window. As an example, if the VRR window is 64-120Hz, then the delta time must be between 8.33 and 15.625 milliseconds. If the delta is outside this window, tearing will likely occur.

If no new present happens inside the window, the prior frame is displayed again.

<h6>Interpolated frame rate inside VRR window</h6>

The variable refresh window usually does not extend above the reported native refresh rate of the display, so tearing will be disabled in this case.

![](media/frame-pacing/variable-inside.svg)

<h6>Interpolated frame rate outside VRR window</h6>

If the frame rate is below the lower bound of the VRR window, the expected behavior is the same as if the frame rate is below the refresh rate of a fixed refresh rate display (see above).

If the frame rate is above the upper bound of the VRR window, the expected behavior is the same as if the frame rate is above the refresh rate of a fixed refresh rate display (see above).

<h3>Implementation Recommendations</h3>

This section covers questions about implementation details regarding pacing, expected behavior and recommendations learned from working with various developers integrating `FSR™ Frame Generation`.

<h4>Backbuffer Count</h4>

Without `FSR™ Frame Generation`, usually applications use double buffering when VSync is disabled or tripple buffering with VSync enabled:
  - One backbuffer that gets currently displayed
  - If VSync is enabled: one backbuffer that has finished rendering and waits for the next vertical refresh so it can get displayed
  - One backbuffer that the application is preparing.

A higher backbuffer count usually is not used as it will not provide any benefits, but will result in unnecessarily high input latency.

The `FSR™ Frame Generation Swapchain` implements a swapchain with the amount of backbuffers specified by the application, plus a real swapchain, with supports tripple buffering. From the application side it is **sufficient to always specify double buffering**. 

Regardless if VSync is enabled or disabled, the present call will kick off the interpolation task with the current backbuffer. Once interpolation has finished, the UI composition will copy the backbuffer to the real swapchain, so the application facing backbuffer can be re-used.

**To minimize input latency, the present call of the `FSR™ Frame Generation Swapchain` will wait for the previous frames interpolation and ui composition to be finished.**

The present waiting on the previous UI composition should not have a negative effect in CPU limited cases. In GPU limited applications this will reduce input latency.

As a result, the previous backbuffer will always be available for rendering the next frame once the present call returns, so the application can preserve some GPU memory by createing the swapchain with no more than 2 backbuffers, even with VSync enabled.

<h4>Minimum Framerate</h4>

`AMD FidelityFX™ Super Resolution Frame Generation 3.1.6` has been designed for an input framerate of at least 60 FPS.

`AMD FSR™ Frame Generation 4.0.0` has been designed for an input framerate of at least 30 FPS.

If the frame rate of the application drops below 60FPS (FSR-SRFG 3.1.6) or 30FPS (FSR-FG 4.0.0) generated frames may result in visual artifacts severe enough to negatively impact the user experience. The exact framerate where artifacts become noticeable will depend on application, scene and movement of objects and camera.

`AMD FidelityFX™ Super Resolution Frame Generation Swapchain 3.1.6` has been designed to give optimal results with an input framerate of 60 FPS or higher. The implementation will function with framerates of at least 30 FPS, but frame-pacing may not be optimal.

Low framerates typically come with high input latency, so depending on the application, it may be preferable to recoup some performance by disabling  `FSR™ Frame Generation` by setting the `ffx::ConfigureDescFrameGeneration::frameGenerationEnabled` to `false` if the frame rate consistently drops below that threshold. 
This can help performance in both CPU and GPU limited scenarios, since interpolation happens on the GPU and disabling `FSR™ Frame Generation` will also disable the busy wait loops in the pacing logic and free up CPU resources. 

It is recommended to re-compute the average frametime for a couple of seconds after enabling or disabling `FSR™ Frame Generation` and compute the performance difference from disabling `FSR™ Frame Generation`.
Reverting the decision to enable/disable `FSR™ Frame Generation` should only be done if the framerate changes more than what can be attributed to the performance difference from disabling it.

<h4>VSync and Variable Refresh Rate</h4>

<h5>Windowed mode</h5>

VRR and tearing are only available in fullscreen mode.

In windowed mode the operating system is responsible for composition and presentation of the final frame. 
Even with VSync disabled, it is not expected to result in tearing artifacts, however not all frames generated will get displayed, so it is possible multiple generated frames will get displayed back to back.
Additionally, generating more frames than can get presented is a waste of resources, so it is recommended to cap the frame rate to half the monitor refresh rate.
This can also be achieved by using `Frame Rate Target Control` in `Adrenalin Control Center`, so it does not necessarily have to be implemented as an application side feature.

If low input latency is the primary goal, it is safe to disable VSync in windowed mode and benefit from uncapped framerate without tearing.

<h5>VSync enabled</h5>

Enabling VSync will prevent tearing artifacts and results in best display quality, however the refresh rate constraints of the monitor can have undesired results:

- Ideally the application runs at slightly above half the monitors refresh rate, or within half the VRR window supported by the monitor. In this case enabling VSync will result in optimal pacing with every frame getting displayed for one refresh period.

- If VSync is enabled and the application runs at less than half the monitor refresh rate (or minimum VRR rate if VRR is supported) this will result in some frames getting displayed for 2 refreshes. As a consequence frames will get displayed for an uneven time resulting in micro-stuttering.

- If the application could run significantly faster than half the monitor refresh rate, `FSR™ Frame Generation` will slow down the application to half refresh rate, so every interpolated and real frame gets displayed for one refresh. In this case disabling `FSR™ Frame Generation` or VSync can result in less input latency.

<h5>VSync disabled</h5>

Running with VSync disabled has lowest impact on performance and input latency but can result in noticeable tearing arifacts.
Tearing artifacts may appear even with VRR enabled and the applicatication presenting within the VRR window.

To combine the best of both worlds, VSync enabled or disabled, it is possible to dynamically toggle VSync state based on the current frame rate, but doing so will result in VRR getting reset. Additionally pacing will need a couple of frames to adjust to the new setting, which will also affect framerate.
As a result, the decision to enable/disable VSync should only be revised once every few seconds based on a long average.

<h3>Shutdown</h3>

Prior to destroying the proxy swapchain, need to first destroy frame generation context. See [`Shutdown`](../techniques/frame-interpolation.md#Shutdown).

Then call [`ffxDestroyContext`](../../api/include/ffx_api.h#L138) on the proxy swap chain context.

Optionally save swapchain description needed to recreate DXGI swapchain. 

Release the final COM reference to trigger the proxy swap chain destructor.

```C++
if (m_FrameGenContext)
{
    // Frame generation must have been disabled via ffx::Configure(...) beforehand.
    ffx::DestroyContext(m_FrameGenContext);
    m_FrameGenContext = nullptr;
}

if (m_SwapChainContext != nullptr)
{
    // This doesn't call proxy swapchain destructor. There is still a swapchain ref held by cauldron
    ffx::DestroyContext(m_SwapChainContext);
    m_SwapChainContext = nullptr;
}

cauldron::SwapChain* pSwapchain  = cauldron::GetSwapChain();
IDXGISwapChain4*     pSwapChain4 = pSwapchain->GetImpl()->DX12SwapChain();

pSwapChain4->AddRef();// Hold strong ref while clearing engine pointer.

// Engine no longer owns COM pointer; safe to recreate after Release().
cauldron::GetSwapChain()->GetImpl()->SetDXGISwapChain(nullptr);

// Capture data required to recreate a normal DXGI swap chain BEFORE releasing COM object.
HWND windowHandle = pSwapchain->GetImpl()->DX12SwapChainDesc().OutputWindow;
DXGI_SWAP_CHAIN_DESC1 desc1 = pSwapchain->GetImpl()->DX12SwapChainDesc1();
DXGI_SWAP_CHAIN_FULLSCREEN_DESC  fsDesc = pSwapchain->GetImpl()->DX12SwapChainFullScreenDesc();

// Release final reference – triggers proxy swap chain destructor when refCount reaches 0.
ULONG refCount = pDxgiSwap->Release();
CauldronAssert(ASSERT_ERROR, refCount == 0, L"SwapChain not fully released. RefCount=%u (expected 0).", refCount);

// Recreate normal DXGI swap chain using hwnd, desc1, fsDesc.

```

<h2>Additional Information</h2>

List of resources created by the `FSR Frame Generation Swapchain`:

- Two CPU worker threads. One of those will be partially spinning between present of the interpolated frame and the real frame to precisely time the presents
- One asynchronous compute queue  - only used when [`FFX_FRAMEGENERATION_ENABLE_ASYNC_WORKLOAD_SUPPORT`](../../framegeneration/include/ffx_framegeneration.h#L36) is set on FSR3 context creation and [`allowAsyncWorkloads`](../../framegeneration/include/ffx_framegeneration.h#L113) is true in the [`ffxConfigureDescFrameGeneration`](../../framegeneration/include/ffx_framegeneration.h#L104).
- One asynchronous present queue. This queue will be used to execute UI composition workloads and present
- A set of command lists, allocators and fences for the interpolation and UI composition workloads
- The GPU resources required to blit the back buffer to the swapchain and compose the UI (if no callback is used)
- The swapchain attached to the actual game window

The `FSR Frame Generation Swapchain` has been designed to minimize dynamic allocations during runtime:

- System memory usage of the class is constant during the lifetime of the swapchain, no STL is being used
- DirectX resources are created on first use and kept alive for reuse

<h2>Limitations</h2>

FSR requires a GPU with typed UAV load and R16G16B16A16_UNORM support.

<h2>Version history</h2>

| Version        | Date              |
| ---------------|-------------------|
| **1.1.1**      | 2023-11-28        |
| **1.1.2**      | 2024-06-05        |
| **1.1.3**      | 2025-05-08        |
| **3.1.5**      | 2025-08-20        |
| **3.1.6**      | 2025-12-10        |

Refer to changelog for more detail on versions.

<h2>See also</h2>

- [FidelityFX™ FSR Sample](../../../../docs/samples/super-resolution.md)
- [FSR™ Naming guidelines](../getting-started/naming-guidelines.md)