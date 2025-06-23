import { test_Raster_simpleRaster2 } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import simpleRaster2 from "./simpleRaster2_Module.js"

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
numExecutions = await test_Raster_simpleRaster2.Init(device, simpleRaster2);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Raster_simpleRaster2.PreExecute(device, encoder, simpleRaster2, frameIndex);
    if (!await simpleRaster2.Execute(device, encoder, true))
        Shared.LogError("Could not execute simpleRaster2");

    // Do post technique execution work
    test_Raster_simpleRaster2.NodeOnly_PostExecute(device, encoder, simpleRaster2, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Raster_simpleRaster2.NodeOnly_Validate(simpleRaster2, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
