import * as Shared from '../../../Shared.js';

function isNode() {
    return typeof process !== 'undefined' && process.versions?.node;
}

let fs;
let PNG;
if (isNode()) {
    await import('webgpu')
        .then((module) => { Object.assign(globalThis, module.globals); })
        .catch((error) => console.error('Error loading module:', error));

    fs = await import('node:fs');

    await import('pngjs')
        .then((module) => {PNG = module.PNG;})
        .catch((error) => console.error('Error loading module:', error));
}

String.prototype.hashCode = function ()
{
    let hash = 0;
    for (let i = 0; i < this.length; i++)
    {
        let chr = this.charCodeAt(i);
        hash = ((hash << 5) - hash) + chr;
        hash |= 0;
    }
    return hash;
}

class class_buffertest_webgpu
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "BufferTest_0", node "BufferTest"
static ShaderCode_BufferTest_BufferTest_0 = `
@binding(0) @group(0) var<storage, read> InputTyped : array<f32>;

@binding(1) @group(0) var<storage, read_write> OutputTyped : array<f32>;

struct Struct_BufferTest_0CB_std140_0
{
    @align(16) alpha1_0 : f32,
    @align(4) alpha2_0 : f32,
    @align(8) gain_0 : f32,
    @align(4) _padding0_0 : f32,
};

@binding(4) @group(0) var<uniform> _BufferTest_0CB : Struct_BufferTest_0CB_std140_0;
struct Struct_TestStruct_std430_0
{
    @align(16) TheFloat4_0 : vec4<f32>,
    @align(16) TheInt4_0 : vec4<i32>,
    @align(16) TheBool_0 : u32,
    @align(4) _padding0_1 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
};

@binding(2) @group(0) var<storage, read> InputStructured : array<Struct_TestStruct_std430_0>;

@binding(3) @group(0) var<storage, read_write> OutputStructured : array<Struct_TestStruct_std430_0>;

struct Struct_TestStruct_0
{
    @align(16) TheFloat4_0 : vec4<f32>,
    @align(16) TheInt4_0 : vec4<i32>,
    @align(16) TheBool_0 : u32,
    @align(4) _padding0_1 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
};

@compute
@workgroup_size(64, 1, 1)
fn Main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : u32 = DTid_0.x;
    var n_minus_2_0 : f32;
    if(_S1 >= u32(2))
    {
        n_minus_2_0 = InputTyped[_S1 - u32(2)];
    }
    else
    {
        n_minus_2_0 = 0.0f;
    }
    var n_minus_1_0 : f32;
    if(_S1 >= u32(1))
    {
        n_minus_1_0 = InputTyped[_S1 - u32(1)];
    }
    else
    {
        n_minus_1_0 = 0.0f;
    }
    OutputTyped[_S1] = _BufferTest_0CB.gain_0 * (InputTyped[_S1] + _BufferTest_0CB.alpha1_0 * n_minus_1_0 + _BufferTest_0CB.alpha2_0 * n_minus_2_0);
    if(_S1 == u32(0))
    {
        var _S2 : vec4<i32> = InputStructured[i32(0)].TheInt4_0;
        var _S3 : u32 = InputStructured[i32(0)].TheBool_0;
        var _S4 : f32 = InputStructured[i32(0)]._padding0_1;
        var _S5 : f32 = InputStructured[i32(0)]._padding1_0;
        var _S6 : f32 = InputStructured[i32(0)]._padding2_0;
        var s_0 : Struct_TestStruct_0;
        s_0.TheFloat4_0 = InputStructured[i32(0)].TheFloat4_0;
        s_0.TheInt4_0 = _S2;
        s_0.TheBool_0 = _S3;
        s_0._padding0_1 = _S4;
        s_0._padding1_0 = _S5;
        s_0._padding2_0 = _S6;
        s_0.TheFloat4_0[i32(0)] = s_0.TheFloat4_0[i32(0)] + 0.10000000149011612f;
        s_0.TheFloat4_0[i32(1)] = s_0.TheFloat4_0[i32(1)] + 0.20000000298023224f;
        s_0.TheFloat4_0[i32(2)] = s_0.TheFloat4_0[i32(2)] + 0.30000001192092896f;
        s_0.TheFloat4_0[i32(3)] = s_0.TheFloat4_0[i32(3)] + 0.40000000596046448f;
        s_0.TheInt4_0[i32(0)] = s_0.TheInt4_0[i32(0)] + i32(1);
        s_0.TheInt4_0[i32(1)] = s_0.TheInt4_0[i32(1)] + i32(2);
        s_0.TheInt4_0[i32(2)] = s_0.TheInt4_0[i32(2)] + i32(3);
        s_0.TheInt4_0[i32(3)] = s_0.TheInt4_0[i32(3)] + i32(4);
        s_0.TheBool_0 = u32(!bool(s_0.TheBool_0));
        var _S7 : Struct_TestStruct_0 = s_0;
        OutputStructured[i32(0)].TheFloat4_0 = s_0.TheFloat4_0;
        OutputStructured[i32(0)].TheInt4_0 = _S7.TheInt4_0;
        OutputStructured[i32(0)].TheBool_0 = _S7.TheBool_0;
        OutputStructured[i32(0)]._padding0_1 = _S7._padding0_1;
        OutputStructured[i32(0)]._padding1_0 = _S7._padding1_0;
        OutputStructured[i32(0)]._padding2_0 = _S7._padding2_0;
    }
    return;
}

`;

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct TestStruct
static StructVertexBufferAttributes_TestStruct =
[
    // TheFloat4
    {
        format: "float32x4",
        offset: 0,
        shaderLocation: 0,
    },
    // TheInt4
    {
        format: "sint32x4",
        offset: 16,
        shaderLocation: 1,
    },
    // TheBool
    {
        format: "uint32",
        offset: 32,
        shaderLocation: 2,
    },
];

// Vertex buffer attributes for struct TestStruct_Unpadded
static StructVertexBufferAttributes_TestStruct_Unpadded =
[
    // TheFloat4
    {
        format: "float32x4",
        offset: 0,
        shaderLocation: 0,
    },
    // TheInt4
    {
        format: "sint32x4",
        offset: 16,
        shaderLocation: 1,
    },
    // TheBool
    {
        format: "uint32",
        offset: 32,
        shaderLocation: 2,
    },
];

// -------------------- Private Members

// Constant buffer _BufferTest_0CB
constantBuffer__BufferTest_0CB = null;
constantBuffer__BufferTest_0CB_size = 16;
constantBuffer__BufferTest_0CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader BufferTest
Hash_Compute_BufferTest = 0;
ShaderModule_Compute_BufferTest = null;
BindGroupLayout_Compute_BufferTest = null;
PipelineLayout_Compute_BufferTest = null;
Pipeline_Compute_BufferTest = null;

// -------------------- Imported Members

// Buffer InputTypedBuffer : This is the buffer to be filtered.
buffer_InputTypedBuffer = null;
buffer_InputTypedBuffer_count = 0;
buffer_InputTypedBuffer_stride = 0;
buffer_InputTypedBuffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer InputStructuredBuffer
buffer_InputStructuredBuffer = null;
buffer_InputStructuredBuffer_count = 0;
buffer_InputStructuredBuffer_stride = 0;
buffer_InputStructuredBuffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Exported Members

// Buffer OutputTypedBuffer : An internal buffer used during the filtering process.
buffer_OutputTypedBuffer = null;
buffer_OutputTypedBuffer_count = 0;
buffer_OutputTypedBuffer_stride = 0;
buffer_OutputTypedBuffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer OutputStructuredBuffer
buffer_OutputStructuredBuffer = null;
buffer_OutputStructuredBuffer_count = 0;
buffer_OutputStructuredBuffer_stride = 0;
buffer_OutputStructuredBuffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Public Variables

variable_gain = 0.500000;  // Overall Volume Adjustment
variableDefault_gain = 0.500000;  // Overall Volume Adjustment
variableChanged_gain = false;
variable_alpha1 = 1.000000;  // Adjusts the contribution of sample n-1
variableDefault_alpha1 = 1.000000;  // Adjusts the contribution of sample n-1
variableChanged_alpha1 = false;
variable_alpha2 = 0.000000;  // Adjusts the contribution of sample n-2
variableDefault_alpha2 = 0.000000;  // Adjusts the contribution of sample n-2
variableChanged_alpha2 = false;

// -------------------- Structs

static StructOffsets_TestStruct =
{
    TheFloat4_0: 0,
    TheFloat4_1: 4,
    TheFloat4_2: 8,
    TheFloat4_3: 12,
    TheInt4_0: 16,
    TheInt4_1: 20,
    TheInt4_2: 24,
    TheInt4_3: 28,
    TheBool: 32,
    _padding0: 36,
    _padding1: 40,
    _padding2: 44,
    _size: 48,
}

static StructOffsets__BufferTest_0CB =
{
    alpha1: 0,
    alpha2: 4,
    gain: 8,
    _padding0: 12,
    _size: 16,
}

static StructOffsets_TestStruct_Unpadded =
{
    TheFloat4_0: 0,
    TheFloat4_1: 4,
    TheFloat4_2: 8,
    TheFloat4_3: 12,
    TheInt4_0: 16,
    TheInt4_1: 20,
    TheInt4_2: 24,
    TheInt4_3: 28,
    TheBool: 32,
    _size: 36,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate buffer InputTypedBuffer
    if (this.buffer_InputTypedBuffer === null)
    {
        Shared.LogError("Imported resource buffer_InputTypedBuffer was not provided");
        return false;
    }

    // Validate buffer InputStructuredBuffer
    if (this.buffer_InputStructuredBuffer === null)
    {
        Shared.LogError("Imported resource buffer_InputStructuredBuffer was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of buffer OutputTypedBuffer
    {
        const baseCount = this.buffer_InputTypedBuffer_count;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = this.buffer_InputTypedBuffer_stride;
        if (this.buffer_OutputTypedBuffer !== null && (this.buffer_OutputTypedBuffer_count != desiredCount || this.buffer_OutputTypedBuffer_stride != desiredStride))
        {
            this.buffer_OutputTypedBuffer.destroy();
            this.buffer_OutputTypedBuffer = null;
        }

        if (this.buffer_OutputTypedBuffer === null)
        {
            this.buffer_OutputTypedBuffer_count = desiredCount;
            this.buffer_OutputTypedBuffer_stride = desiredStride;
            this.buffer_OutputTypedBuffer = device.createBuffer({
                label: "buffer buffertest_webgpu.OutputTypedBuffer",
                size: Shared.Align(16, this.buffer_OutputTypedBuffer_count * this.buffer_OutputTypedBuffer_stride),
                usage: this.buffer_OutputTypedBuffer_usageFlags,
            });
        }
    }

    // Handle (re)creation of buffer OutputStructuredBuffer
    {
        const baseCount = this.buffer_InputStructuredBuffer_count;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = this.buffer_InputStructuredBuffer_stride;
        if (this.buffer_OutputStructuredBuffer !== null && (this.buffer_OutputStructuredBuffer_count != desiredCount || this.buffer_OutputStructuredBuffer_stride != desiredStride))
        {
            this.buffer_OutputStructuredBuffer.destroy();
            this.buffer_OutputStructuredBuffer = null;
        }

        if (this.buffer_OutputStructuredBuffer === null)
        {
            this.buffer_OutputStructuredBuffer_count = desiredCount;
            this.buffer_OutputStructuredBuffer_stride = desiredStride;
            this.buffer_OutputStructuredBuffer = device.createBuffer({
                label: "buffer buffertest_webgpu.OutputStructuredBuffer",
                size: Shared.Align(16, this.buffer_OutputStructuredBuffer_count * this.buffer_OutputStructuredBuffer_stride),
                usage: this.buffer_OutputStructuredBuffer_usageFlags,
            });
        }
    }

    // (Re)create compute shader BufferTest
    {
        const bindGroupEntries =
        [
            {
                // InputTyped
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // OutputTyped
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // InputStructured
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // OutputStructured
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // _BufferTest_0CB
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_BufferTest === null || newHash !== this.Hash_Compute_BufferTest)
        {
            this.Hash_Compute_BufferTest = newHash;

            let shaderCode = class_buffertest_webgpu.ShaderCode_BufferTest_BufferTest_0;

            this.ShaderModule_Compute_BufferTest = device.createShaderModule({ code: shaderCode, label: "Compute Shader BufferTest"});
            this.BindGroupLayout_Compute_BufferTest = device.createBindGroupLayout({
                label: "Compute Bind Group Layout BufferTest",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_BufferTest = device.createPipelineLayout({
                label: "Compute Pipeline Layout BufferTest",
                bindGroupLayouts: [this.BindGroupLayout_Compute_BufferTest],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_BufferTest = device.createComputePipeline({
                    label: "Compute Pipeline BufferTest",
                    layout: this.PipelineLayout_Compute_BufferTest,
                    compute: {
                        module: this.ShaderModule_Compute_BufferTest,
                        entryPoint: "Main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("BufferTest");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline BufferTest",
                    layout: this.PipelineLayout_Compute_BufferTest,
                    compute: {
                        module: this.ShaderModule_Compute_BufferTest,
                        entryPoint: "Main",
                    }
                }).then( handle => { this.Pipeline_Compute_BufferTest = handle; this.loadingPromises.delete("BufferTest"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("buffertest_webgpu.InputTypedBuffer");

    encoder.popDebugGroup(); // "buffertest_webgpu.InputTypedBuffer"

    encoder.pushDebugGroup("buffertest_webgpu.OutputTypedBuffer");

    encoder.popDebugGroup(); // "buffertest_webgpu.OutputTypedBuffer"

    encoder.pushDebugGroup("buffertest_webgpu.InputStructuredBuffer");

    encoder.popDebugGroup(); // "buffertest_webgpu.InputStructuredBuffer"

    encoder.pushDebugGroup("buffertest_webgpu.OutputStructuredBuffer");

    encoder.popDebugGroup(); // "buffertest_webgpu.OutputStructuredBuffer"

    encoder.pushDebugGroup("buffertest_webgpu._BufferTest_0CB");

    // Create constant buffer _BufferTest_0CB
    if (this.constantBuffer__BufferTest_0CB === null)
    {
        this.constantBuffer__BufferTest_0CB = device.createBuffer({
            label: "buffertest_webgpu._BufferTest_0CB",
            size: Shared.Align(16, this.constructor.StructOffsets__BufferTest_0CB._size),
            usage: this.constantBuffer__BufferTest_0CB_usageFlags,
        });
    }

    // Upload values to constant buffer _BufferTest_0CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__BufferTest_0CB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__BufferTest_0CB.alpha1, this.variable_alpha1, true);
        view.setFloat32(this.constructor.StructOffsets__BufferTest_0CB.alpha2, this.variable_alpha2, true);
        view.setFloat32(this.constructor.StructOffsets__BufferTest_0CB.gain, this.variable_gain, true);
        device.queue.writeBuffer(this.constantBuffer__BufferTest_0CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "buffertest_webgpu._BufferTest_0CB"

    encoder.pushDebugGroup("buffertest_webgpu.BufferTest");

    // Run compute shader BufferTest
    // BufferTest compute shader
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group BufferTest",
            layout: this.BindGroupLayout_Compute_BufferTest,
            entries: [
                {
                    // InputTyped
                    binding: 0,
                    resource: { buffer: this.buffer_InputTypedBuffer }
                },
                {
                    // OutputTyped
                    binding: 1,
                    resource: { buffer: this.buffer_OutputTypedBuffer }
                },
                {
                    // InputStructured
                    binding: 2,
                    resource: { buffer: this.buffer_InputStructuredBuffer }
                },
                {
                    // OutputStructured
                    binding: 3,
                    resource: { buffer: this.buffer_OutputStructuredBuffer }
                },
                {
                    // _BufferTest_0CB
                    binding: 4,
                    resource: { buffer: this.constantBuffer__BufferTest_0CB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = [this.buffer_InputTypedBuffer_count, 1, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 64 - 1) / 64),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_BufferTest !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_BufferTest);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "buffertest_webgpu.BufferTest"

}

/*
When useBlockingAPIs is false, this function works differently:
1) When first called, it does frame start SetVariables, and starts asynchronous operations.
2) If any async operations are in flight, it exits out.
3) When this function is called again, if there are still async operations in flight it will do nothing.
4) if there are not async operations in flight, it will add passes to the encoder and do end frame SetVariables.

This dance is needed because we don't want to block the main thread in browsers.
However, some logic in the render graph may only happen during the first execution (driven by variables) which
could cause it never to happen if the async operation took longer than one Execute() call, and the start frame
set variables were allowed to be called more than once before execution, or the end frame variables were allowed
to be called at all.

When useBlockingAPIs is true, this function does everything every frame, blocking as needed.
Node.js has useBlockingAPIs=true to simplify logic. Node has no shader compilation timeouts, unlike the browser.

*/
async Execute(device, encoder, useBlockingAPIs = false)
{
    // Always make sure we have our required imported resources
    if (!await this.ValidateImports())
        return false;

    // If we are waiting on promises it means we've already done these steps and are waiting for the promises to be done
    if (!this.waitingOnPromises)
    {
        await this.SetVarsBefore();
        await this.Init(device, encoder, useBlockingAPIs);
    }

    // If we are waiting on promises, remember that and exit out
    if (this.loadingPromises.size > 0)
    {
        this.waitingOnPromises = true;
        return true;
    }
    this.waitingOnPromises = false;

    // If we got here, all async work is done, so fill the encoder and do the end of frame set variable logic
    await this.FillEncoder(device, encoder);
    await this.SetVarsAfter();

    return true;
}

};

var buffertest_webgpu = new class_buffertest_webgpu;

export default buffertest_webgpu;
