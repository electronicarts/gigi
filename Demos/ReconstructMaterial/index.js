import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import SlangAutoDiff from "./SlangAutoDiff_Module.js"

// Initialize WebGPU
const navigator = { gpu: create(["enable-dawn-features=use_dxc"]) };

const adapter = await navigator.gpu?.requestAdapter();
const device = await adapter?.requestDevice({
    requiredLimits: { maxStorageTexturesPerShaderStage: 8 },
    requiredFeatures: [ 'float32-filterable' ],
});
device.pushErrorScope("validation");

let numExecutions = 1;

/* TODO:
provide imported resources or variable values to the technique:
                Host Variables:
                    SlangAutoDiff.variable_initialized = false; // (Bool)
                    SlangAutoDiff.variable_FrameIndex = 0; // (Int)
                User Variables (These show in the UI):
                    SlangAutoDiff.variable_Reset = true; // (Bool)
                    SlangAutoDiff.variable_UseBackwardAD = false; // (Bool)

                An example of how to load a texture:
                    const loadedTexture = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", SlangAutoDiff.texture_someTexture_usageFlags);
                    SlangAutoDiff.texture_someTexture = loadedTexture.texture;
                    SlangAutoDiff.texture_someTexture_size = loadedTexture.size;
                    SlangAutoDiff.texture_someTexture_format = loadedTexture.format;

                See _GeneratedCode\UnitTests\WebGPU\UnitTestLogic.js for more examples of loading files of various kinds.
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await SlangAutoDiff.Execute(device, encoder, true))
        Shared.LogError("Could not execute SlangAutoDiff");

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
