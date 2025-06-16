import { test_RayTrace_TwoRayGensSubgraph } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import TwoRayGensSubgraph from "./TwoRayGensSubgraph_Module.js"

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
numExecutions = await test_RayTrace_TwoRayGensSubgraph.Init(device, TwoRayGensSubgraph);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_RayTrace_TwoRayGensSubgraph.PreExecute(device, encoder, TwoRayGensSubgraph, frameIndex);
    if (!await TwoRayGensSubgraph.Execute(device, encoder, true))
        Shared.LogError("Could not execute TwoRayGensSubgraph");

    // Do post technique execution work
    test_RayTrace_TwoRayGensSubgraph.NodeOnly_PostExecute(device, encoder, TwoRayGensSubgraph, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_RayTrace_TwoRayGensSubgraph.NodeOnly_Validate(TwoRayGensSubgraph, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
