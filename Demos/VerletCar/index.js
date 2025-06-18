import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import DemofoxVerletCar from "./DemofoxVerletCar_Module.js"

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
                Host Variables:
                    DemofoxVerletCar.variable_MouseState = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    DemofoxVerletCar.variable_MouseStateLastFrame = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    DemofoxVerletCar.variable_iResolution = 0.000000, 0.000000, 0.000000; // (Float3)
                    DemofoxVerletCar.variable_iTime = 0.000000; // (Float)
                    DemofoxVerletCar.variable_iTimeDelta = 0.000000; // (Float)
                    DemofoxVerletCar.variable_iFrameRate = 0.000000; // (Float)
                    DemofoxVerletCar.variable_iFrame = 0; // (Int)
                    DemofoxVerletCar.variable_iMouse = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    DemofoxVerletCar.variable_ViewMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxVerletCar.variable_InvViewMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxVerletCar.variable_ProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxVerletCar.variable_InvProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxVerletCar.variable_ViewProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxVerletCar.variable_InvViewProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxVerletCar.variable_CameraPos = 0.000000, 0.000000, 0.000000; // (Float3)
                    DemofoxVerletCar.variable_CameraChanged = false; // (Bool)
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await DemofoxVerletCar.Execute(device, encoder, true))
        Shared.LogError("Could not execute DemofoxVerletCar");

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
