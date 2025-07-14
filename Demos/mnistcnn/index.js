import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import mnist from "./mnist_Module.js"

// Initialize WebGPU
const navigator = { gpu: create(["enable-dawn-features=use_dxc"]) };

const adapter = await navigator.gpu?.requestAdapter();
const device = await adapter?.requestDevice({
    requiredLimits: {  },
    requiredFeatures: [ 'float32-filterable' ],
});
device.pushErrorScope("validation");

let numExecutions = 1;

/* TODO:
provide imported resources or variable values to the technique:
                    mnist.texture_Imported_Image = null;
                    mnist.texture_Imported_Image_size = [0, 0, 0];
                    mnist.texture_Imported_Image_format = "";
                    mnist.buffer_Conv1Weights = null;
                    mnist.buffer_Conv1Weights_count = 0;
                    mnist.buffer_Conv1Weights_stride = 0;
                    mnist.buffer_Conv1Bias = null;
                    mnist.buffer_Conv1Bias_count = 0;
                    mnist.buffer_Conv1Bias_stride = 0;
                    mnist.buffer_Conv2Weights = null;
                    mnist.buffer_Conv2Weights_count = 0;
                    mnist.buffer_Conv2Weights_stride = 0;
                    mnist.buffer_Conv2Bias = null;
                    mnist.buffer_Conv2Bias_count = 0;
                    mnist.buffer_Conv2Bias_stride = 0;
                    mnist.buffer_LinearWeights = null;
                    mnist.buffer_LinearWeights_count = 0;
                    mnist.buffer_LinearWeights_stride = 0;
                    mnist.buffer_LinearBias = null;
                    mnist.buffer_LinearBias_count = 0;
                    mnist.buffer_LinearBias_stride = 0;
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
                    mnist.variable_Conv1OutputScale = 4; // (Int) Scale Conv1 output. 24x24x2 unscaled.
                    mnist.variable_MaxPool1OutputScale = 8; // (Int) Scale MaxPool1 output. 12x12x2 unscaled.
                    mnist.variable_Conv2OutputScale = 8; // (Int) Scale Conv2 output. 10x10x4 unscaled.
                    mnist.variable_MaxPool2OutputScale = 16; // (Int) Scale MaxPool2 output. 5x5x4 unscaled.

                An example of how to load a texture:
                    const loadedTexture = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", mnist.texture_someTexture_usageFlags);
                    mnist.texture_someTexture = loadedTexture.texture;
                    mnist.texture_someTexture_size = loadedTexture.size;
                    mnist.texture_someTexture_format = loadedTexture.format;

                See _GeneratedCode\UnitTests\WebGPU\UnitTestLogic.js for more examples of loading files of various kinds.
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
