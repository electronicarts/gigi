import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import BlueNoiseAreaLights from "./BlueNoiseAreaLights_Module.js"

// Initialize WebGPU
const navigator = { gpu: create(["enable-dawn-features=use_dxc"]) };

const adapter = await navigator.gpu?.requestAdapter();
const device = await adapter?.requestDevice({
    requiredLimits: {  },
    requiredFeatures: [  ],
});
device.pushErrorScope("validation");

let numExecutions = 1;

/* TODO:
provide imported resources or variable values to the technique:
                Host Variables:
                    BlueNoiseAreaLights.variable_FrameIndex = 0; // (Uint)
                User Variables (These show in the UI):
                    BlueNoiseAreaLights.variable_NoiseType = this.constructor.Enum_NoiseTypes.FAST_IS; // (Int)
                    BlueNoiseAreaLights.variable_ExposureFStops = 0.000000; // (Float) Exposure, in FStops. Every +1 is double brightness. Every -1 is half brightness.
                    BlueNoiseAreaLights.variable_ToneMapping = this.constructor.Enum_ToneMappingOperation.ACES; // (Int)
                    BlueNoiseAreaLights.variable_Animate = true; // (Bool)
                    BlueNoiseAreaLights.variable_AmbientLight = 0.050000, 0.050000, 0.050000; // (Float3)
                    BlueNoiseAreaLights.variable_TemporalFilter = this.constructor.Enum_TemporalFilters.None; // (Int)
                    BlueNoiseAreaLights.variable_EMAAlpha = 0.100000; // (Float) How much to lerp towards the new from the old, when using EMA temporal filtering
                    BlueNoiseAreaLights.variable_DirLightEnabled = true; // (Bool)
                    BlueNoiseAreaLights.variable_DirLightSampleCount = 16; // (Int) How many stochastic samples to take per frame
                    BlueNoiseAreaLights.variable_DirLightColor = 1.000000, 0.800000, 0.500000; // (Float3)
                    BlueNoiseAreaLights.variable_DirLightBrightness = 1.000000; // (Float)
                    BlueNoiseAreaLights.variable_DirLightDirection = 1.000000, -1.000000, -1.000000; // (Float3) What direction the light shines in
                    BlueNoiseAreaLights.variable_DirLightRadius = 0.100000; // (Float) radius of light disk at 1 unit away
                    BlueNoiseAreaLights.variable_PosLightEnabled = true; // (Bool)
                    BlueNoiseAreaLights.variable_PosLightSampleCount = 16; // (Int) How many stochastic samples to take per frame
                    BlueNoiseAreaLights.variable_PosLightColor = 0.500000, 0.800000, 1.000000; // (Float3)
                    BlueNoiseAreaLights.variable_PosLightBrightness = 1.000000; // (Float)
                    BlueNoiseAreaLights.variable_PosLightPosition = 0.000000, 60.000000, 40.000000; // (Float3)
                    BlueNoiseAreaLights.variable_PosLightRadius = 5.000000; // (Float)
                    BlueNoiseAreaLights.variable_PosLightShineDir = 0.000000, -1.000000, -1.000000; // (Float3) The direction of the spot light. Cos Theta Inner / Outer control the spot light
                    BlueNoiseAreaLights.variable_PosLightShineCosThetaInner = 0.800000; // (Float) direction to light, dotted by -c_lightDir. light starts to fade here.
                    BlueNoiseAreaLights.variable_PosLightShineCosThetaOuter = 0.700000; // (Float) direction to light, dotted by -c_lightDir. light finishes fading here.
                    BlueNoiseAreaLights.variable_ResetAccum = false; // (Bool)

                An example of how to load a texture:
                    const loadedTexture = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", BlueNoiseAreaLights.texture_someTexture_usageFlags);
                    BlueNoiseAreaLights.texture_someTexture = loadedTexture.texture;
                    BlueNoiseAreaLights.texture_someTexture_size = loadedTexture.size;
                    BlueNoiseAreaLights.texture_someTexture_format = loadedTexture.format;

                See _GeneratedCode\UnitTests\WebGPU\UnitTestLogic.js for more examples of loading files of various kinds.
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await BlueNoiseAreaLights.Execute(device, encoder, true))
        Shared.LogError("Could not execute BlueNoiseAreaLights");

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
