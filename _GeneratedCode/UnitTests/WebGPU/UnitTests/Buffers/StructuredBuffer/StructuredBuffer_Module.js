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

class class_StructuredBuffer
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "csmain_0", node "csmain"
static ShaderCode_csmain_csmain_0 = `
struct Struct_TheStructure_std430_0
{
    @align(8) TheInt_0 : i32,
    @align(4) TheFloat_0 : f32,
    @align(8) TheEnum_0 : i32,
    @align(4) TheBool_0 : u32,
    @align(8) TheUINT_0 : u32,
    @align(4) _padding0_0 : f32,
    @align(8) TheFloat2_0 : vec2<f32>,
};

@binding(0) @group(0) var<storage, read_write> buff : array<Struct_TheStructure_std430_0>;

struct Struct_csmain_0CB_std140_0
{
    @align(16) frameDeltaTime_0 : f32,
    @align(4) frameIndex_0 : i32,
    @align(8) _padding0_1 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(1) @group(0) var<uniform> _csmain_0CB : Struct_csmain_0CB_std140_0;
@compute
@workgroup_size(1, 1, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    buff[i32(0)].TheInt_0 = i32(1);
    buff[i32(0)].TheFloat_0 = 1.29999995231628418f;
    buff[i32(0)].TheEnum_0 = i32(1);
    buff[i32(0)].TheBool_0 = u32(0);
    buff[i32(0)].TheUINT_0 = u32(31337);
    buff[i32(0)].TheFloat2_0 = vec2<f32>(0.30000001192092896f, 3.0f);
    buff[i32(1)].TheInt_0 = _csmain_0CB.frameIndex_0;
    buff[i32(1)].TheFloat_0 = _csmain_0CB.frameDeltaTime_0;
    buff[i32(1)].TheEnum_0 = i32(2);
    buff[i32(1)].TheBool_0 = u32(2);
    buff[i32(1)].TheUINT_0 = u32(255);
    buff[i32(1)].TheFloat2_0 = vec2<f32>(99.0f, 0.0101010100916028f);
    return;
}

`;

// -------------------- Enums

static Enum_Trinary =
{
    True: 0,
    False: 1,
    Maybe: 2,
    _count: 3,
}

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct TheStructure
static StructVertexBufferAttributes_TheStructure =
[
    // TheInt
    {
        format: "sint32",
        offset: 0,
        shaderLocation: 0,
    },
    // TheFloat
    {
        format: "float32",
        offset: 4,
        shaderLocation: 1,
    },
    // TheEnum
    {
        format: "sint32",
        offset: 8,
        shaderLocation: 2,
    },
    // TheBool
    {
        format: "uint32",
        offset: 12,
        shaderLocation: 3,
    },
    // TheUINT
    {
        format: "uint32",
        offset: 16,
        shaderLocation: 4,
    },
    // TheFloat2
    {
        format: "float32x2",
        offset: 24,
        shaderLocation: 5,
    },
];

// Vertex buffer attributes for struct TheStructure_Unpadded
static StructVertexBufferAttributes_TheStructure_Unpadded =
[
    // TheInt
    {
        format: "sint32",
        offset: 0,
        shaderLocation: 0,
    },
    // TheFloat
    {
        format: "float32",
        offset: 4,
        shaderLocation: 1,
    },
    // TheEnum
    {
        format: "sint32",
        offset: 8,
        shaderLocation: 2,
    },
    // TheBool
    {
        format: "uint32",
        offset: 12,
        shaderLocation: 3,
    },
    // TheUINT
    {
        format: "uint32",
        offset: 16,
        shaderLocation: 4,
    },
    // TheFloat2
    {
        format: "float32x2",
        offset: 20,
        shaderLocation: 5,
    },
];

// -------------------- Private Members

// Constant buffer _csmain_0CB
constantBuffer__csmain_0CB = null;
constantBuffer__csmain_0CB_size = 16;
constantBuffer__csmain_0CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader csmain
Hash_Compute_csmain = 0;
ShaderModule_Compute_csmain = null;
BindGroupLayout_Compute_csmain = null;
PipelineLayout_Compute_csmain = null;
Pipeline_Compute_csmain = null;

// -------------------- Imported Members

// Buffer buff
buffer_buff = null;
buffer_buff_count = 0;
buffer_buff_stride = 0;
buffer_buff_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Public Variables

variable_frameIndex = 0;
variableDefault_frameIndex = 0;
variableChanged_frameIndex = false;
variable_frameDeltaTime = 0.;
variableDefault_frameDeltaTime = 0.;
variableChanged_frameDeltaTime = false;

// -------------------- Structs

static StructOffsets_TheStructure =
{
    TheInt: 0,
    TheFloat: 4,
    TheEnum: 8,
    TheBool: 12,
    TheUINT: 16,
    _padding0: 20,
    TheFloat2_0: 24,
    TheFloat2_1: 28,
    _size: 32,
}

static StructOffsets__csmain_0CB =
{
    frameDeltaTime: 0,
    frameIndex: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets_TheStructure_Unpadded =
{
    TheInt: 0,
    TheFloat: 4,
    TheEnum: 8,
    TheBool: 12,
    TheUINT: 16,
    TheFloat2_0: 20,
    TheFloat2_1: 24,
    _size: 28,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate buffer buff
    if (this.buffer_buff === null)
    {
        Shared.LogError("Imported resource buffer_buff was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // (Re)create compute shader csmain
    {
        const bindGroupEntries =
        [
            {
                // buff
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // _csmain_0CB
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_csmain === null || newHash !== this.Hash_Compute_csmain)
        {
            this.Hash_Compute_csmain = newHash;

            let shaderCode = class_StructuredBuffer.ShaderCode_csmain_csmain_0;

            this.ShaderModule_Compute_csmain = device.createShaderModule({ code: shaderCode, label: "Compute Shader csmain"});
            this.BindGroupLayout_Compute_csmain = device.createBindGroupLayout({
                label: "Compute Bind Group Layout csmain",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_csmain = device.createPipelineLayout({
                label: "Compute Pipeline Layout csmain",
                bindGroupLayouts: [this.BindGroupLayout_Compute_csmain],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_csmain = device.createComputePipeline({
                    label: "Compute Pipeline csmain",
                    layout: this.PipelineLayout_Compute_csmain,
                    compute: {
                        module: this.ShaderModule_Compute_csmain,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("csmain");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline csmain",
                    layout: this.PipelineLayout_Compute_csmain,
                    compute: {
                        module: this.ShaderModule_Compute_csmain,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_csmain = handle; this.loadingPromises.delete("csmain"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("StructuredBuffer.buff");

    encoder.popDebugGroup(); // "StructuredBuffer.buff"

    encoder.pushDebugGroup("StructuredBuffer._csmain_0CB");

    // Create constant buffer _csmain_0CB
    if (this.constantBuffer__csmain_0CB === null)
    {
        this.constantBuffer__csmain_0CB = device.createBuffer({
            label: "StructuredBuffer._csmain_0CB",
            size: Shared.Align(16, this.constructor.StructOffsets__csmain_0CB._size),
            usage: this.constantBuffer__csmain_0CB_usageFlags,
        });
    }

    // Upload values to constant buffer _csmain_0CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__csmain_0CB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__csmain_0CB.frameDeltaTime, this.variable_frameDeltaTime, true);
        view.setInt32(this.constructor.StructOffsets__csmain_0CB.frameIndex, this.variable_frameIndex, true);
        device.queue.writeBuffer(this.constantBuffer__csmain_0CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "StructuredBuffer._csmain_0CB"

    encoder.pushDebugGroup("StructuredBuffer.csmain");

    // Run compute shader csmain
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group csmain",
            layout: this.BindGroupLayout_Compute_csmain,
            entries: [
                {
                    // buff
                    binding: 0,
                    resource: { buffer: this.buffer_buff }
                },
                {
                    // _csmain_0CB
                    binding: 1,
                    resource: { buffer: this.constantBuffer__csmain_0CB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = [this.buffer_buff_count, 1, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_csmain !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_csmain);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "StructuredBuffer.csmain"

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

var StructuredBuffer = new class_StructuredBuffer;

export default StructuredBuffer;
