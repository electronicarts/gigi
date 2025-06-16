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

class class_ReadbackSequence
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "ReadbackSequenceCS", node "Node_1"
static ShaderCode_Node_1_ReadbackSequenceCS = `
struct Struct_ReadbackSequenceCSCB_std140_0
{
    @align(16) frameIndex_0 : i32,
    @align(4) _padding0_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
};

@binding(2) @group(0) var<uniform> _ReadbackSequenceCSCB : Struct_ReadbackSequenceCSCB_std140_0;
@binding(0) @group(0) var output : texture_storage_2d</*(output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var c_0 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    if(bool(((_ReadbackSequenceCSCB.frameIndex_0) & (i32(1)))))
    {
        c_0[i32(2)] = 1.0f;
    }
    if(bool(((_ReadbackSequenceCSCB.frameIndex_0) & (i32(2)))))
    {
        c_0[i32(1)] = 1.0f;
    }
    if(bool(((_ReadbackSequenceCSCB.frameIndex_0) & (i32(4)))))
    {
        c_0[i32(0)] = 1.0f;
    }
    textureStore((output), (DTid_0.xy), (vec4<f32>(c_0, 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture Node_1_output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Node_1_output_ReadOnly = null;
texture_Node_1_output_ReadOnly_size = [0, 0, 0];
texture_Node_1_output_ReadOnly_format = "";
texture_Node_1_output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _ReadbackSequenceCSCB
constantBuffer__ReadbackSequenceCSCB = null;
constantBuffer__ReadbackSequenceCSCB_size = 16;
constantBuffer__ReadbackSequenceCSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Node_1
Hash_Compute_Node_1 = 0;
ShaderModule_Compute_Node_1 = null;
BindGroupLayout_Compute_Node_1 = null;
PipelineLayout_Compute_Node_1 = null;
Pipeline_Compute_Node_1 = null;

// -------------------- Imported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_frameIndex = 0;
variableDefault_frameIndex = 0;
variableChanged_frameIndex = false;

// -------------------- Structs

static StructOffsets__ReadbackSequenceCSCB =
{
    frameIndex: 0,
    _padding0: 4,
    _padding1: 8,
    _padding2: 12,
    _size: 16,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate texture Output
    if (this.texture_Output === null)
    {
        Shared.LogError("Imported resource texture_Output was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Node_1_output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Node_1_output_ReadOnly !== null && (this.texture_Node_1_output_ReadOnly_format != desiredFormat || this.texture_Node_1_output_ReadOnly_size[0] != desiredSize[0] || this.texture_Node_1_output_ReadOnly_size[1] != desiredSize[1] || this.texture_Node_1_output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Node_1_output_ReadOnly.destroy();
            this.texture_Node_1_output_ReadOnly = null;
        }

        if (this.texture_Node_1_output_ReadOnly === null)
        {
            this.texture_Node_1_output_ReadOnly_size = desiredSize.slice();
            this.texture_Node_1_output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Node_1_output_ReadOnly_format))
                viewFormats.push(this.texture_Node_1_output_ReadOnly_format);

            this.texture_Node_1_output_ReadOnly = device.createTexture({
                label: "texture ReadbackSequence.Node_1_output_ReadOnly",
                size: this.texture_Node_1_output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Node_1_output_ReadOnly_format),
                usage: this.texture_Node_1_output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Node_1
    {
        const bindGroupEntries =
        [
            {
                // output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // outputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Node_1_output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _ReadbackSequenceCSCB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Node_1 === null || newHash !== this.Hash_Compute_Node_1)
        {
            this.Hash_Compute_Node_1 = newHash;

            let shaderCode = class_ReadbackSequence.ShaderCode_Node_1_ReadbackSequenceCS;
            shaderCode = shaderCode.replace("/*(output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(outputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Node_1_output_ReadOnly_format));

            this.ShaderModule_Compute_Node_1 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Node_1"});
            this.BindGroupLayout_Compute_Node_1 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Node_1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Node_1 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Node_1",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Node_1],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Node_1 = device.createComputePipeline({
                    label: "Compute Pipeline Node_1",
                    layout: this.PipelineLayout_Compute_Node_1,
                    compute: {
                        module: this.ShaderModule_Compute_Node_1,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Node_1");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Node_1",
                    layout: this.PipelineLayout_Compute_Node_1,
                    compute: {
                        module: this.ShaderModule_Compute_Node_1,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Node_1 = handle; this.loadingPromises.delete("Node_1"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("ReadbackSequence.Output");

    encoder.popDebugGroup(); // "ReadbackSequence.Output"

    encoder.pushDebugGroup("ReadbackSequence.Node_1_output_ReadOnly");

    encoder.popDebugGroup(); // "ReadbackSequence.Node_1_output_ReadOnly"

    encoder.pushDebugGroup("ReadbackSequence.Copy_Node_1_output");

    // Copy texture Output to texture Node_1_output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Node_1_output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Node_1_output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "ReadbackSequence.Copy_Node_1_output"

    encoder.pushDebugGroup("ReadbackSequence._ReadbackSequenceCSCB");

    // Create constant buffer _ReadbackSequenceCSCB
    if (this.constantBuffer__ReadbackSequenceCSCB === null)
    {
        this.constantBuffer__ReadbackSequenceCSCB = device.createBuffer({
            label: "ReadbackSequence._ReadbackSequenceCSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__ReadbackSequenceCSCB._size),
            usage: this.constantBuffer__ReadbackSequenceCSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _ReadbackSequenceCSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__ReadbackSequenceCSCB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__ReadbackSequenceCSCB.frameIndex, this.variable_frameIndex, true);
        device.queue.writeBuffer(this.constantBuffer__ReadbackSequenceCSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "ReadbackSequence._ReadbackSequenceCSCB"

    encoder.pushDebugGroup("ReadbackSequence.Node_1");

    // Run compute shader Node_1
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Node_1",
            layout: this.BindGroupLayout_Compute_Node_1,
            entries: [
                {
                    // output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // outputReadOnly
                    binding: 1,
                    resource: this.texture_Node_1_output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _ReadbackSequenceCSCB
                    binding: 2,
                    resource: { buffer: this.constantBuffer__ReadbackSequenceCSCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Node_1 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Node_1);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "ReadbackSequence.Node_1"

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

var ReadbackSequence = new class_ReadbackSequence;

export default ReadbackSequence;
