import { test_Barrier_BarrierTest } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import BarrierTest from "./BarrierTest_Module.js"

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
numExecutions = await test_Barrier_BarrierTest.Init(device, BarrierTest);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Barrier_BarrierTest.PreExecute(device, encoder, BarrierTest, frameIndex);
    if (!await BarrierTest.Execute(device, encoder, true))
        Shared.LogError("Could not execute BarrierTest");

    // Do post technique execution work
    test_Barrier_BarrierTest.NodeOnly_PostExecute(device, encoder, BarrierTest, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Barrier_BarrierTest.NodeOnly_Validate(BarrierTest, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
