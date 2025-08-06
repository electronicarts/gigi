import { test_Textures_Mips_CS_2DArray } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import Mips_CS_2DArray from "./Mips_CS_2DArray_Module.js"

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
numExecutions = await test_Textures_Mips_CS_2DArray.Init(device, Mips_CS_2DArray);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Textures_Mips_CS_2DArray.PreExecute(device, encoder, Mips_CS_2DArray, frameIndex);
    if (!await Mips_CS_2DArray.Execute(device, encoder, true))
        Shared.LogError("Could not execute Mips_CS_2DArray");

    // Do post technique execution work
    test_Textures_Mips_CS_2DArray.NodeOnly_PostExecute(device, encoder, Mips_CS_2DArray, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Textures_Mips_CS_2DArray.NodeOnly_Validate(Mips_CS_2DArray, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
