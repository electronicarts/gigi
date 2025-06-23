import { test_Buffers_buffertest_webgpu } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import buffertest_webgpu from "./buffertest_webgpu_Module.js"

// Initialize WebGPU
const navigator = { gpu: create(["enable-dawn-features=use_dxc"]) };

const adapter = await navigator.gpu?.requestAdapter();
const device = await adapter?.requestDevice({
    requiredLimits: {  },
    requiredFeatures: [  ],
});
device.pushErrorScope("validation");

let numExecutions = 1;

// initialize the module
numExecutions = await test_Buffers_buffertest_webgpu.Init(device, buffertest_webgpu);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Buffers_buffertest_webgpu.PreExecute(device, encoder, buffertest_webgpu, frameIndex);
    if (!await buffertest_webgpu.Execute(device, encoder, true))
        Shared.LogError("Could not execute buffertest_webgpu");

    // Do post technique execution work
    test_Buffers_buffertest_webgpu.NodeOnly_PostExecute(device, encoder, buffertest_webgpu, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Buffers_buffertest_webgpu.NodeOnly_Validate(buffertest_webgpu, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
