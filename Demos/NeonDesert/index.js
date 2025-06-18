import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import DemofoxNeonDesert from "./DemofoxNeonDesert_Module.js"

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
                    DemofoxNeonDesert.variable_MouseState = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    DemofoxNeonDesert.variable_MouseStateLastFrame = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    DemofoxNeonDesert.variable_iResolution = 0.000000, 0.000000, 0.000000; // (Float3)
                    DemofoxNeonDesert.variable_iTime = 0.000000; // (Float)
                    DemofoxNeonDesert.variable_iTimeDelta = 0.000000; // (Float)
                    DemofoxNeonDesert.variable_iFrameRate = 0.000000; // (Float)
                    DemofoxNeonDesert.variable_iFrame = 0; // (Int)
                    DemofoxNeonDesert.variable_iMouse = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    DemofoxNeonDesert.variable_ViewMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxNeonDesert.variable_InvViewMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxNeonDesert.variable_ProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxNeonDesert.variable_InvProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxNeonDesert.variable_ViewProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxNeonDesert.variable_InvViewProjMtx = 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000; // (Float4x4)
                    DemofoxNeonDesert.variable_CameraPos = 0.000000, 0.000000, 0.000000; // (Float3)
                    DemofoxNeonDesert.variable_CameraChanged = false; // (Bool)
                User Variables (These show in the UI):
                    DemofoxNeonDesert.variable_RenderSize = 1280, 720; // (Uint2)
                    DemofoxNeonDesert.variable_Reset = false; // (Bool)
                    DemofoxNeonDesert.variable_exposure = 0.000000; // (Float) in FStops. -inf to +inf.
                    DemofoxNeonDesert.variable_apertureRadius = 0.400000; // (Float) in world units. 0 for pinhole camera.
                    DemofoxNeonDesert.variable_focalPlaneDistance = 80.000000; // (Float) in world units. How far from the camera things are in focus at.
                    DemofoxNeonDesert.variable_numBounces = 4; // (Int)
                    DemofoxNeonDesert.variable_rayMarchSteps = 256; // (Int)
                    DemofoxNeonDesert.variable_minStepDistance = 0.100000; // (Float)
                    DemofoxNeonDesert.variable_ACESToneMapping = true; // (Bool)
                    DemofoxNeonDesert.variable_sRGB = true; // (Bool)
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await DemofoxNeonDesert.Execute(device, encoder, true))
        Shared.LogError("Could not execute DemofoxNeonDesert");

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
