import { test_RayTrace_TwoRayGens } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import TwoRayGens from "./TwoRayGens_Module.js"

// Initialize WebGPU
const navigator = { gpu: create(["enable-dawn-features=use_dxc"]) };

const adapter = await navigator.gpu?.requestAdapter();
const device = await adapter?.requestDevice({
    requiredLimits: { maxStorageTexturesPerShaderStage: 8, maxComputeWorkgroupStorageSize: 32768 },
    requiredFeatures: [ 'float32-filterable', 'subgroups' ],
});
device.pushErrorScope("validation");

let numExecutions = 1;

// initialize the module
numExecutions = await test_RayTrace_TwoRayGens.Init(device, TwoRayGens);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_RayTrace_TwoRayGens.PreExecute(device, encoder, TwoRayGens, frameIndex);
    if (!await TwoRayGens.Execute(device, encoder, true))
        Shared.LogError("Could not execute TwoRayGens");

    // Do post technique execution work
    test_RayTrace_TwoRayGens.NodeOnly_PostExecute(device, encoder, TwoRayGens, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_RayTrace_TwoRayGens.NodeOnly_Validate(TwoRayGens, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
