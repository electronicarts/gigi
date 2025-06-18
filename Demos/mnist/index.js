import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import mnist from "./mnist_Module.js"

// Initialize WebGPU
const navigator = { gpu: create(["enable-dawn-features=use_dxc"]) };

const adapter = await navigator.gpu?.requestAdapter();
const device = await adapter?.requestDevice({
    requiredLimits: { maxStorageTexturesPerShaderStage: 8, maxComputeWorkgroupStorageSize: 32768 },
    requiredFeatures: [ 'float32-filterable', 'subgroups' ],
});
device.pushErrorScope("validation");

let numExecutions = 1;

/* TODO:
provide imported resources or variable values to the technique:
                    mnist.buffer_NN_Weights = null;
                    mnist.buffer_NN_Weights_count = 0;
                    mnist.buffer_NN_Weights_stride = 0;
                    mnist.texture_Imported_Image = null;
                    mnist.texture_Imported_Image_size = [0, 0, 0];
                    mnist.texture_Imported_Image_format = "";
                Host Variables:
                    mnist.variable_MouseState = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    mnist.variable_MouseStateLastFrame = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                    mnist.variable_iResolution = 0.000000, 0.000000, 0.000000; // (Float3)
                    mnist.variable_iTime = 0.000000; // (Float)
                    mnist.variable_iTimeDelta = 0.000000; // (Float)
                    mnist.variable_iFrameRate = 0.000000; // (Float)
                    mnist.variable_iFrame = 0; // (Int)
                    mnist.variable_iMouse = 0.000000, 0.000000, 0.000000, 0.000000; // (Float4)
                User Variables (These show in the UI):
                    mnist.variable_Clear = false; // (Bool)
                    mnist.variable_PenSize = 10.000000; // (Float)
                    mnist.variable_UseImportedImage = false; // (Bool)
                    mnist.variable_NormalizeDrawing = true; // (Bool) MNIST normalization: shrink image to 20x20 and put center of mass in the middle of a 28x28 image
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await mnist.Execute(device, encoder, true))
        Shared.LogError("Could not execute mnist");

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
