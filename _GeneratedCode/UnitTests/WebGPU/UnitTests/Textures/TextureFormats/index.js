import { test_Textures_TextureFormats } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import TextureFormats from "./TextureFormats_Module.js"

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
numExecutions = await test_Textures_TextureFormats.Init(device, TextureFormats);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Textures_TextureFormats.PreExecute(device, encoder, TextureFormats, frameIndex);
    if (!await TextureFormats.Execute(device, encoder, true))
        Shared.LogError("Could not execute TextureFormats");

    // Do post technique execution work
    test_Textures_TextureFormats.NodeOnly_PostExecute(device, encoder, TextureFormats, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Textures_TextureFormats.NodeOnly_Validate(TextureFormats, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
