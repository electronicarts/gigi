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

class class_TextureFormats
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "csmain_0", node "ComputeShader"
static ShaderCode_ComputeShader_csmain_0 = `
@binding(0) @group(0) var tex : texture_storage_2d</*(tex_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    // NOTE: I added this to make the unit test be able to do what it needed to do.
    textureStore((tex), (DTid_0.xy), (/*(write_value)*/));
    return;
}

`;

// -------------------- Private Members

// Texture ComputeShader_tex_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_ComputeShader_tex_ReadOnly = null;
texture_ComputeShader_tex_ReadOnly_size = [0, 0, 0];
texture_ComputeShader_tex_ReadOnly_format = "";
texture_ComputeShader_tex_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader ComputeShader
Hash_Compute_ComputeShader = 0;
ShaderModule_Compute_ComputeShader = null;
BindGroupLayout_Compute_ComputeShader = null;
PipelineLayout_Compute_ComputeShader = null;
Pipeline_Compute_ComputeShader = null;

// -------------------- Imported Members

// Texture Texture
texture_Texture = null;
texture_Texture_size = [0, 0, 0];
texture_Texture_format = "";
texture_Texture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate texture Texture
    if (this.texture_Texture === null)
    {
        Shared.LogError("Imported resource texture_Texture was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture ComputeShader_tex_ReadOnly
    {
        const baseSize = this.texture_Texture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Texture_format;
        if (this.texture_ComputeShader_tex_ReadOnly !== null && (this.texture_ComputeShader_tex_ReadOnly_format != desiredFormat || this.texture_ComputeShader_tex_ReadOnly_size[0] != desiredSize[0] || this.texture_ComputeShader_tex_ReadOnly_size[1] != desiredSize[1] || this.texture_ComputeShader_tex_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_ComputeShader_tex_ReadOnly.destroy();
            this.texture_ComputeShader_tex_ReadOnly = null;
        }

        if (this.texture_ComputeShader_tex_ReadOnly === null)
        {
            this.texture_ComputeShader_tex_ReadOnly_size = desiredSize.slice();
            this.texture_ComputeShader_tex_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_ComputeShader_tex_ReadOnly_format))
                viewFormats.push(this.texture_ComputeShader_tex_ReadOnly_format);

            this.texture_ComputeShader_tex_ReadOnly = device.createTexture({
                label: "texture TextureFormats.ComputeShader_tex_ReadOnly",
                size: this.texture_ComputeShader_tex_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_ComputeShader_tex_ReadOnly_format),
                usage: this.texture_ComputeShader_tex_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader ComputeShader
    {
        const bindGroupEntries =
        [
            {
                // tex
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Texture_format), viewDimension: "2d" }
            },
            {
                // texReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_ComputeShader_tex_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_ComputeShader === null || newHash !== this.Hash_Compute_ComputeShader)
        {
            this.Hash_Compute_ComputeShader = newHash;

            let shaderCode = class_TextureFormats.ShaderCode_ComputeShader_csmain_0;
            shaderCode = shaderCode.replace("/*(tex_format)*/", Shared.GetNonSRGBFormat(this.texture_Texture_format));
            shaderCode = shaderCode.replace("/*(texReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_ComputeShader_tex_ReadOnly_format));

            // NOTE: I added this to make the unit test be able to do what it needed to do.
            shaderCode = shaderCode.replace("/*(write_value)*/", this.valueToWrite);

            this.ShaderModule_Compute_ComputeShader = device.createShaderModule({ code: shaderCode, label: "Compute Shader ComputeShader"});
            this.BindGroupLayout_Compute_ComputeShader = device.createBindGroupLayout({
                label: "Compute Bind Group Layout ComputeShader",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_ComputeShader = device.createPipelineLayout({
                label: "Compute Pipeline Layout ComputeShader",
                bindGroupLayouts: [this.BindGroupLayout_Compute_ComputeShader],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_ComputeShader = device.createComputePipeline({
                    label: "Compute Pipeline ComputeShader",
                    layout: this.PipelineLayout_Compute_ComputeShader,
                    compute: {
                        module: this.ShaderModule_Compute_ComputeShader,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("ComputeShader");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline ComputeShader",
                    layout: this.PipelineLayout_Compute_ComputeShader,
                    compute: {
                        module: this.ShaderModule_Compute_ComputeShader,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_ComputeShader = handle; this.loadingPromises.delete("ComputeShader"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("TextureFormats.Texture");

    encoder.popDebugGroup(); // "TextureFormats.Texture"

    encoder.pushDebugGroup("TextureFormats.ComputeShader_tex_ReadOnly");

    encoder.popDebugGroup(); // "TextureFormats.ComputeShader_tex_ReadOnly"

    encoder.pushDebugGroup("TextureFormats.Copy_ComputeShader_tex");

    // Copy texture Texture to texture ComputeShader_tex_ReadOnly
    {
        const numMips = Math.min(this.texture_Texture.mipLevelCount, this.texture_ComputeShader_tex_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Texture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Texture.height >> mipIndex, 1);
            let mipDepth = this.texture_Texture.depthOrArrayLayers;

            if (this.texture_Texture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Texture, mipLevel: mipIndex },
                { texture: this.texture_ComputeShader_tex_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "TextureFormats.Copy_ComputeShader_tex"

    encoder.pushDebugGroup("TextureFormats.ComputeShader");

    // Run compute shader ComputeShader
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group ComputeShader",
            layout: this.BindGroupLayout_Compute_ComputeShader,
            entries: [
                {
                    // tex
                    binding: 0,
                    resource: this.texture_Texture.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // texReadOnly
                    binding: 1,
                    resource: this.texture_ComputeShader_tex_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Texture_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_ComputeShader !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_ComputeShader);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "TextureFormats.ComputeShader"

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

var TextureFormats = new class_TextureFormats;

export default TextureFormats;
