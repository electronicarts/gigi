import { test_Raster_YesVertexStruct_YesIndex_NoInstance } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import YesVertexStruct_YesIndex_NoInstance from "./YesVertexStruct_YesIndex_NoInstance_Module.js"

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
numExecutions = await test_Raster_YesVertexStruct_YesIndex_NoInstance.Init(device, YesVertexStruct_YesIndex_NoInstance);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Raster_YesVertexStruct_YesIndex_NoInstance.PreExecute(device, encoder, YesVertexStruct_YesIndex_NoInstance, frameIndex);
    if (!await YesVertexStruct_YesIndex_NoInstance.Execute(device, encoder, true))
        Shared.LogError("Could not execute YesVertexStruct_YesIndex_NoInstance");

    // Do post technique execution work
    test_Raster_YesVertexStruct_YesIndex_NoInstance.NodeOnly_PostExecute(device, encoder, YesVertexStruct_YesIndex_NoInstance, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Raster_YesVertexStruct_YesIndex_NoInstance.NodeOnly_Validate(YesVertexStruct_YesIndex_NoInstance, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
