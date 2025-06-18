import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import CasualPathtracing from "./CasualPathtracing_Module.js"

// Initialize WebGPU
const navigator = { gpu: create(["enable-dawn-features=use_dxc"]) };

const adapter = await navigator.gpu?.requestAdapter();
const device = await adapter?.requestDevice({
    requiredLimits: { maxStorageTexturesPerShaderStage: 8, maxComputeWorkgroupStorageSize: 32768 },
    requiredFeatures: [ 'float32-filterable', 'subgroups' ],
});
device.pushErrorScope("validation");

let numExecutions = 1;

/* TODO:
provide imported resources or variable values to the technique:
                User Variables (These show in the UI):
                    CasualPathtracing.variable_RenderSize = 1280, 720; // (Uint2)
                    CasualPathtracing.variable_Scene = this.constructor.Enum_Scenes.Transparent_Orange; // (Int)
                    CasualPathtracing.variable_Animate = true; // (Bool)
                    CasualPathtracing.variable_Reset = false; // (Bool)
                    CasualPathtracing.variable_NumBounces = 8; // (Int) number of ray bounces allowed max
                    CasualPathtracing.variable_SkyboxBrightness = 1.000000; // (Float) a multiplier for the skybox brightness
                    CasualPathtracing.variable_Exposure = 0.000000; // (Float) a pixel value multiplier of light before tone mapping and sRGB. In FStops. Every +1 doubles brightness, every -1 halves brightness/
                    CasualPathtracing.variable_RaysPerPixel = 16; // (Int) How many rays per pixel, per frame?
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await CasualPathtracing.Execute(device, encoder, true))
        Shared.LogError("Could not execute CasualPathtracing");

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
