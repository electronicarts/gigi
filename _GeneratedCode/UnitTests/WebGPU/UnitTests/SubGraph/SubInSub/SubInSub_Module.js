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

class class_SubInSub
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Inner1_Inner2_Inner2CS", node "Inner1_Inner2_Rotate_Colors"
static ShaderCode_Inner1_Inner2_Rotate_Colors_Inner1_Inner2_Inner2CS = `
@binding(0) @group(0) var Input : texture_2d<f32>;

struct Struct_Inner1_Inner2_Inner2CSCB_std140_0
{
    @align(16) Inner1_Inner1Mult_0 : vec4<f32>,
};

@binding(3) @group(0) var<uniform> _Inner1_Inner2_Inner2CSCB : Struct_Inner1_Inner2_Inner2CSCB_std140_0;
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S1 : vec3<i32> = vec3<i32>(vec3<u32>(px_0, u32(0)));
    var _S2 : vec4<f32> = (textureLoad((Input), ((_S1)).xy, ((_S1)).z)).yzxw * _Inner1_Inner2_Inner2CSCB.Inner1_Inner1Mult_0;
    // Manual fix for slang bug: https://github.com/shader-slang/slang/issues/6551
    textureStore((Output), (px_0), (_S2));
    return;
}

`;

// -------------------- Private Members

// Texture Inner1_Inner2_Rotate_Colors_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly = null;
texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_size = [0, 0, 0];
texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format = "";
texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _Inner1_Inner2_Inner2CSCB
constantBuffer__Inner1_Inner2_Inner2CSCB = null;
constantBuffer__Inner1_Inner2_Inner2CSCB_size = 16;
constantBuffer__Inner1_Inner2_Inner2CSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Inner1_Inner2_Rotate_Colors
Hash_Compute_Inner1_Inner2_Rotate_Colors = 0;
ShaderModule_Compute_Inner1_Inner2_Rotate_Colors = null;
BindGroupLayout_Compute_Inner1_Inner2_Rotate_Colors = null;
PipelineLayout_Compute_Inner1_Inner2_Rotate_Colors = null;
Pipeline_Compute_Inner1_Inner2_Rotate_Colors = null;

// -------------------- Imported Members

// Texture Input
texture_Input = null;
texture_Input_size = [0, 0, 0];
texture_Input_format = "";
texture_Input_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// -------------------- Exported Members

// Texture Inner1_Inner2_Output
texture_Inner1_Inner2_Output = null;
texture_Inner1_Inner2_Output_size = [0, 0, 0];
texture_Inner1_Inner2_Output_format = "";
texture_Inner1_Inner2_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_Inner1_Inner1Mult = [ 1.000000, 0.500000, 2.000000, 1.000000 ];
variableDefault_Inner1_Inner1Mult = [ 1.000000, 0.500000, 2.000000, 1.000000 ];
variableChanged_Inner1_Inner1Mult = [ false, false, false, false ];

// -------------------- Structs

static StructOffsets__Inner1_Inner2_Inner2CSCB =
{
    Inner1_Inner1Mult_0: 0,
    Inner1_Inner1Mult_1: 4,
    Inner1_Inner1Mult_2: 8,
    Inner1_Inner1Mult_3: 12,
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
    // Validate texture Input
    if (this.texture_Input === null)
    {
        Shared.LogError("Imported resource texture_Input was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Inner1_Inner2_Output
    {
        const baseSize = this.texture_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Input_format;
        if (this.texture_Inner1_Inner2_Output !== null && (this.texture_Inner1_Inner2_Output_format != desiredFormat || this.texture_Inner1_Inner2_Output_size[0] != desiredSize[0] || this.texture_Inner1_Inner2_Output_size[1] != desiredSize[1] || this.texture_Inner1_Inner2_Output_size[2] != desiredSize[2]))
        {
            this.texture_Inner1_Inner2_Output.destroy();
            this.texture_Inner1_Inner2_Output = null;
        }

        if (this.texture_Inner1_Inner2_Output === null)
        {
            this.texture_Inner1_Inner2_Output_size = desiredSize.slice();
            this.texture_Inner1_Inner2_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Inner1_Inner2_Output_format))
                viewFormats.push(this.texture_Inner1_Inner2_Output_format);

            this.texture_Inner1_Inner2_Output = device.createTexture({
                label: "texture SubInSub.Inner1_Inner2_Output",
                size: this.texture_Inner1_Inner2_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Inner1_Inner2_Output_format),
                usage: this.texture_Inner1_Inner2_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Inner1_Inner2_Rotate_Colors_Output_ReadOnly
    {
        const baseSize = this.texture_Inner1_Inner2_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Inner1_Inner2_Output_format;
        if (this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly !== null && (this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format != desiredFormat || this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly.destroy();
            this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly = null;
        }

        if (this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly === null)
        {
            this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format))
                viewFormats.push(this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format);

            this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly = device.createTexture({
                label: "texture SubInSub.Inner1_Inner2_Rotate_Colors_Output_ReadOnly",
                size: this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format),
                usage: this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Inner1_Inner2_Rotate_Colors
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Input_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Inner1_Inner2_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _Inner1_Inner2_Inner2CSCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Inner1_Inner2_Rotate_Colors === null || newHash !== this.Hash_Compute_Inner1_Inner2_Rotate_Colors)
        {
            this.Hash_Compute_Inner1_Inner2_Rotate_Colors = newHash;

            let shaderCode = class_SubInSub.ShaderCode_Inner1_Inner2_Rotate_Colors_Inner1_Inner2_Inner2CS;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Inner1_Inner2_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly_format));

            this.ShaderModule_Compute_Inner1_Inner2_Rotate_Colors = device.createShaderModule({ code: shaderCode, label: "Compute Shader Inner1_Inner2_Rotate_Colors"});
            this.BindGroupLayout_Compute_Inner1_Inner2_Rotate_Colors = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Inner1_Inner2_Rotate_Colors",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Inner1_Inner2_Rotate_Colors = device.createPipelineLayout({
                label: "Compute Pipeline Layout Inner1_Inner2_Rotate_Colors",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Inner1_Inner2_Rotate_Colors],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Inner1_Inner2_Rotate_Colors = device.createComputePipeline({
                    label: "Compute Pipeline Inner1_Inner2_Rotate_Colors",
                    layout: this.PipelineLayout_Compute_Inner1_Inner2_Rotate_Colors,
                    compute: {
                        module: this.ShaderModule_Compute_Inner1_Inner2_Rotate_Colors,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Inner1_Inner2_Rotate_Colors");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Inner1_Inner2_Rotate_Colors",
                    layout: this.PipelineLayout_Compute_Inner1_Inner2_Rotate_Colors,
                    compute: {
                        module: this.ShaderModule_Compute_Inner1_Inner2_Rotate_Colors,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Inner1_Inner2_Rotate_Colors = handle; this.loadingPromises.delete("Inner1_Inner2_Rotate_Colors"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("SubInSub.Input");

    encoder.popDebugGroup(); // "SubInSub.Input"

    encoder.pushDebugGroup("SubInSub.Inner1_Inner2_Output");

    encoder.popDebugGroup(); // "SubInSub.Inner1_Inner2_Output"

    encoder.pushDebugGroup("SubInSub.Inner1_Inner2_Rotate_Colors_Output_ReadOnly");

    encoder.popDebugGroup(); // "SubInSub.Inner1_Inner2_Rotate_Colors_Output_ReadOnly"

    encoder.pushDebugGroup("SubInSub.Copy_Inner1_Inner2_Rotate_Colors_Output");

    // Copy texture Inner1_Inner2_Output to texture Inner1_Inner2_Rotate_Colors_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Inner1_Inner2_Output.mipLevelCount, this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Inner1_Inner2_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Inner1_Inner2_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Inner1_Inner2_Output.depthOrArrayLayers;

            if (this.texture_Inner1_Inner2_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Inner1_Inner2_Output, mipLevel: mipIndex },
                { texture: this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubInSub.Copy_Inner1_Inner2_Rotate_Colors_Output"

    encoder.pushDebugGroup("SubInSub._Inner1_Inner2_Inner2CSCB");

    // Create constant buffer _Inner1_Inner2_Inner2CSCB
    if (this.constantBuffer__Inner1_Inner2_Inner2CSCB === null)
    {
        this.constantBuffer__Inner1_Inner2_Inner2CSCB = device.createBuffer({
            label: "SubInSub._Inner1_Inner2_Inner2CSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__Inner1_Inner2_Inner2CSCB._size),
            usage: this.constantBuffer__Inner1_Inner2_Inner2CSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _Inner1_Inner2_Inner2CSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Inner1_Inner2_Inner2CSCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__Inner1_Inner2_Inner2CSCB.Inner1_Inner1Mult_0, this.variable_Inner1_Inner1Mult[0], true);
        view.setFloat32(this.constructor.StructOffsets__Inner1_Inner2_Inner2CSCB.Inner1_Inner1Mult_1, this.variable_Inner1_Inner1Mult[1], true);
        view.setFloat32(this.constructor.StructOffsets__Inner1_Inner2_Inner2CSCB.Inner1_Inner1Mult_2, this.variable_Inner1_Inner1Mult[2], true);
        view.setFloat32(this.constructor.StructOffsets__Inner1_Inner2_Inner2CSCB.Inner1_Inner1Mult_3, this.variable_Inner1_Inner1Mult[3], true);
        device.queue.writeBuffer(this.constantBuffer__Inner1_Inner2_Inner2CSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SubInSub._Inner1_Inner2_Inner2CSCB"

    encoder.pushDebugGroup("SubInSub.Inner1_Inner2_Rotate_Colors");

    // Run compute shader Inner1_Inner2_Rotate_Colors
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Inner1_Inner2_Rotate_Colors",
            layout: this.BindGroupLayout_Compute_Inner1_Inner2_Rotate_Colors,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Input.createView({ dimension: "2d", format: this.texture_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_Inner1_Inner2_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_Inner1_Inner2_Rotate_Colors_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _Inner1_Inner2_Inner2CSCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__Inner1_Inner2_Inner2CSCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Input_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Inner1_Inner2_Rotate_Colors !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Inner1_Inner2_Rotate_Colors);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SubInSub.Inner1_Inner2_Rotate_Colors"

    encoder.pushDebugGroup("SubInSub.Inner1_Inner2_Copy_Back");

    // Copy texture Inner1_Inner2_Output to texture Input
    {
        const numMips = Math.min(this.texture_Inner1_Inner2_Output.mipLevelCount, this.texture_Input.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Inner1_Inner2_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Inner1_Inner2_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Inner1_Inner2_Output.depthOrArrayLayers;

            if (this.texture_Inner1_Inner2_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Inner1_Inner2_Output, mipLevel: mipIndex },
                { texture: this.texture_Input, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubInSub.Inner1_Inner2_Copy_Back"

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

var SubInSub = new class_SubInSub;

export default SubInSub;
