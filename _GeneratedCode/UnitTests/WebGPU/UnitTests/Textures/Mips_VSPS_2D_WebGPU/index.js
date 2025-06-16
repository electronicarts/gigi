import { test_Textures_Mips_VSPS_2D_WebGPU } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import Mips_VSPS_2D_WebGPU from "./Mips_VSPS_2D_WebGPU_Module.js"

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
numExecutions = await test_Textures_Mips_VSPS_2D_WebGPU.Init(device, Mips_VSPS_2D_WebGPU);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Textures_Mips_VSPS_2D_WebGPU.PreExecute(device, encoder, Mips_VSPS_2D_WebGPU, frameIndex);
    if (!await Mips_VSPS_2D_WebGPU.Execute(device, encoder, true))
        Shared.LogError("Could not execute Mips_VSPS_2D_WebGPU");

    // Do post technique execution work
    test_Textures_Mips_VSPS_2D_WebGPU.NodeOnly_PostExecute(device, encoder, Mips_VSPS_2D_WebGPU, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Textures_Mips_VSPS_2D_WebGPU.NodeOnly_Validate(Mips_VSPS_2D_WebGPU, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
