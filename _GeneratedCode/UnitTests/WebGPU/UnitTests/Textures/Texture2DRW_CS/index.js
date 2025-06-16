import { test_Textures_Texture2DRW_CS } from "../../../UnitTestLogic.js";
import { create, globals } from 'webgpu';
import * as Shared from '../../../Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import Texture2DRW_CS from "./Texture2DRW_CS_Module.js"

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
numExecutions = await test_Textures_Texture2DRW_CS.Init(device, Texture2DRW_CS);

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    test_Textures_Texture2DRW_CS.PreExecute(device, encoder, Texture2DRW_CS, frameIndex);
    if (!await Texture2DRW_CS.Execute(device, encoder, true))
        Shared.LogError("Could not execute Texture2DRW_CS");

    // Do post technique execution work
    test_Textures_Texture2DRW_CS.NodeOnly_PostExecute(device, encoder, Texture2DRW_CS, frameIndex);

    // Submit the command encoder
    device.queue.submit([encoder.finish()]);

    // Wait for all work to be done
    await device.queue.onSubmittedWorkDone();

    // Do validation work
    await test_Textures_Texture2DRW_CS.NodeOnly_Validate(Texture2DRW_CS, frameIndex);
}

// Validation errors are test failures
await device.popErrorScope().then((error) => {
    if (error)
        Shared.LogError(`An error occurred during execution: ${error.message}`);
});

// Exit with success
process.exit(0)
