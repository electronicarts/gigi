import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import DemofoxLife from "./DemofoxLife_Module.js"

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
                    DemofoxLife.variable_Clear = false; // (Bool)
                    DemofoxLife.variable_GridSize = 32, 32; // (Uint2) The size of the simulation grid
                    DemofoxLife.variable_Grid = true; // (Bool)
                    DemofoxLife.variable_WrapAround = true; // (Bool) If true, the right most cells are neighbors to the left most, and the top are neighbors to the bottom.
                    DemofoxLife.variable_RenderSize = 512, 512; // (Uint2) The size of the rendered output
                    DemofoxLife.variable_SimPeriod = 5; // (Uint) A frame step is done when this many frames have been rendered. 1 is as fast as possible.
                    DemofoxLife.variable_Pause = false; // (Bool)
                    DemofoxLife.variable_SingleStep = false; // (Bool)
                    DemofoxLife.variable_RandomDensity = 0.500000; // (Float)
                    DemofoxLife.variable_Randomize = false; // (Bool)
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await DemofoxLife.Execute(device, encoder, true))
        Shared.LogError("Could not execute DemofoxLife");

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
