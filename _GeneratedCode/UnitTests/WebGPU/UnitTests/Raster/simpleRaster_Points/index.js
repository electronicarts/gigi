import { test_Raster_simpleRaster_Points } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import simpleRaster_Points from "./simpleRaster_Points_Module.js"

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
numExecutions = await test_Raster_simpleRaster_Points.Init(device, simpleRaster_Points);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Raster_simpleRaster_Points.PreExecute(device, encoder, simpleRaster_Points, frameIndex);
    if (!await simpleRaster_Points.Execute(device, encoder, true))
        Shared.LogError("Could not execute simpleRaster_Points");

    // Do post technique execution work
    test_Raster_simpleRaster_Points.NodeOnly_PostExecute(device, encoder, simpleRaster_Points, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Raster_simpleRaster_Points.NodeOnly_Validate(simpleRaster_Points, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
