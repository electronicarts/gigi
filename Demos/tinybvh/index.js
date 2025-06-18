import { create, globals } from 'webgpu';
import * as Shared from './Shared.js';

Object.assign(globalThis, globals);

// Import the technique
import tinybvh from "./tinybvh_Module.js"

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
                    tinybvh.buffer_Vertices = null;
                    tinybvh.buffer_Vertices_count = 0;
                    tinybvh.buffer_Vertices_stride = 0;
                    tinybvh.buffer_TriIndices = null;
                    tinybvh.buffer_TriIndices_count = 0;
                    tinybvh.buffer_TriIndices_stride = 0;
                    tinybvh.buffer_BVHNodes = null;
                    tinybvh.buffer_BVHNodes_count = 0;
                    tinybvh.buffer_BVHNodes_stride = 0;
                    tinybvh.buffer_BVHCombinedData = null;
                    tinybvh.buffer_BVHCombinedData_count = 0;
                    tinybvh.buffer_BVHCombinedData_stride = 0;
                User Variables (These show in the UI):
                    tinybvh.variable_RenderSize = 1024, 768; // (Uint2)
                    tinybvh.variable_ViewMode = this.constructor.Enum_ViewModes.Shaded; // (Int)
                    tinybvh.variable_TMax = 1000.000000; // (Float) The maximum length ray allowed
                    tinybvh.variable_CostDivider = 100.000000; // (Float) Cost View Mode: divides by this number to help show the range.
                    tinybvh.variable_DistanceDivider = 10.000000; // (Float) Distance View Mode: divides by this number to help show the range.
                    tinybvh.variable_LightDirection = 1.000000, -1.000000, 0.000000; // (Float3) The direction the light is shining.
                    tinybvh.variable_LightColor = 1.000000, 1.000000, 1.000000; // (Float3)
                    tinybvh.variable_LightBrightness = 3.000000; // (Float)
                    tinybvh.variable_AmbientColor = 0.020000, 0.020000, 0.020000; // (Float3)
                    tinybvh.variable_AmbientBrightness = 1.000000; // (Float)
                    tinybvh.variable_Albedo = 0.800000, 0.800000, 0.800000; // (Float3)
                    tinybvh.variable_Roughness = 0.400000; // (Float)
                    tinybvh.variable_SpecularLevel = 0.500000; // (Float)
                    tinybvh.variable_Metalic = 0.000000; // (Float)
                    tinybvh.variable_RayTestAgainst = this.constructor.Enum_RayTestAgainsts.BVHSeparate; // (Int)
*/

for (let frameIndex = 0; frameIndex < numExecutions; ++frameIndex)
{
    // Run the technique
    const encoder = device.createCommandEncoder();
    if (!await tinybvh.Execute(device, encoder, true))
        Shared.LogError("Could not execute tinybvh");

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
