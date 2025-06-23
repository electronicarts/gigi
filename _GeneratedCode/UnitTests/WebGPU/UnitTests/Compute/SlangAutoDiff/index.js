import { test_Compute_SlangAutoDiff } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import SlangAutoDiff from "./SlangAutoDiff_Module.js"

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
numExecutions = await test_Compute_SlangAutoDiff.Init(device, SlangAutoDiff);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Compute_SlangAutoDiff.PreExecute(device, encoder, SlangAutoDiff, frameIndex);
    if (!await SlangAutoDiff.Execute(device, encoder, true))
        Shared.LogError("Could not execute SlangAutoDiff");

    // Do post technique execution work
    test_Compute_SlangAutoDiff.NodeOnly_PostExecute(device, encoder, SlangAutoDiff, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Compute_SlangAutoDiff.NodeOnly_Validate(SlangAutoDiff, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
