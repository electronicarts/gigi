import { test_Textures_Mips_DrawCall } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import Mips_DrawCall from "./Mips_DrawCall_Module.js"

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
numExecutions = await test_Textures_Mips_DrawCall.Init(device, Mips_DrawCall);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Textures_Mips_DrawCall.PreExecute(device, encoder, Mips_DrawCall, frameIndex);
    if (!await Mips_DrawCall.Execute(device, encoder, true))
        Shared.LogError("Could not execute Mips_DrawCall");

    // Do post technique execution work
    test_Textures_Mips_DrawCall.NodeOnly_PostExecute(device, encoder, Mips_DrawCall, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Textures_Mips_DrawCall.NodeOnly_Validate(Mips_DrawCall, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
