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

class class_Texture3DRW_CS
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "RWCS_0", node "RW"
static ShaderCode_RW_RWCS_0 = `
@binding(5) @group(0) var _loadedTexture_0 : texture_3d<f32>;

@binding(4) @group(0) var importedTextureReadOnly : texture_storage_3d</*(importedTextureReadOnly_format)*/, read>;

@binding(2) @group(0) var importedColor : texture_3d<f32>;

@binding(0) @group(0) var nodeTexture : texture_storage_3d</*(nodeTexture_format)*/, write>;

@binding(1) @group(0) var importedTexture : texture_storage_3d</*(importedTexture_format)*/, write>;

@compute
@workgroup_size(4, 4, 4)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : vec3<u32> = DTid_0.xyz;
    var _S2 : vec4<i32> = vec4<i32>(vec4<u32>(_S1, u32(0)));
    var loadedTexturePx_0 : vec3<f32> = (textureLoad((_loadedTexture_0), ((_S2)).xyz, ((_S2)).w)).xyz;
    var _S3 : vec4<f32> = (textureLoad((importedTextureReadOnly), (vec3<i32>(_S1))));
    var importedTexturePx_0 : vec3<f32> = _S3.xyz;
    var importedColorPx_0 : vec3<f32> = (textureLoad((importedColor), ((_S2)).xyz, ((_S2)).w)).xyz;
    textureStore((nodeTexture), (_S1), (vec4<f32>(loadedTexturePx_0 * importedTexturePx_0 * importedColorPx_0, 1.0f)));
    textureStore((importedTexture), (_S1), (vec4<f32>((loadedTexturePx_0 + importedTexturePx_0) / vec3<f32>(2.0f) * importedColorPx_0, 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture RW_nodeTexture_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_RW_nodeTexture_ReadOnly = null;
texture_RW_nodeTexture_ReadOnly_size = [0, 0, 0];
texture_RW_nodeTexture_ReadOnly_format = "";
texture_RW_nodeTexture_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture RW_importedTexture_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_RW_importedTexture_ReadOnly = null;
texture_RW_importedTexture_ReadOnly_size = [0, 0, 0];
texture_RW_importedTexture_ReadOnly_format = "";
texture_RW_importedTexture_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Compute Shader RW
Hash_Compute_RW = 0;
ShaderModule_Compute_RW = null;
BindGroupLayout_Compute_RW = null;
PipelineLayout_Compute_RW = null;
Pipeline_Compute_RW = null;

// -------------------- Imported Members

// Texture ImportedTexture
texture_ImportedTexture = null;
texture_ImportedTexture_size = [0, 0, 0];
texture_ImportedTexture_format = "";
texture_ImportedTexture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture ImportedColor
texture_ImportedColor = null;
texture_ImportedColor_size = [0, 0, 0];
texture_ImportedColor_format = "";
texture_ImportedColor_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// -------------------- Exported Members

// Texture NodeTexture
texture_NodeTexture = null;
texture_NodeTexture_size = [0, 0, 0];
texture_NodeTexture_format = "";
texture_NodeTexture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate texture ImportedTexture
    if (this.texture_ImportedTexture === null)
    {
        Shared.LogError("Imported resource texture_ImportedTexture was not provided");
        return false;
    }

    // Validate texture ImportedColor
    if (this.texture_ImportedColor === null)
    {
        Shared.LogError("Imported resource texture_ImportedColor was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture NodeTexture
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 3) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm";
        if (this.texture_NodeTexture !== null && (this.texture_NodeTexture_format != desiredFormat || this.texture_NodeTexture_size[0] != desiredSize[0] || this.texture_NodeTexture_size[1] != desiredSize[1] || this.texture_NodeTexture_size[2] != desiredSize[2]))
        {
            this.texture_NodeTexture.destroy();
            this.texture_NodeTexture = null;
        }

        if (this.texture_NodeTexture === null)
        {
            this.texture_NodeTexture_size = desiredSize.slice();
            this.texture_NodeTexture_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_NodeTexture_format))
                viewFormats.push(this.texture_NodeTexture_format);

            this.texture_NodeTexture = device.createTexture({
                label: "texture Texture3DRW_CS.NodeTexture",
                size: this.texture_NodeTexture_size,
                format: Shared.GetNonSRGBFormat(this.texture_NodeTexture_format),
                usage: this.texture_NodeTexture_usageFlags,
                viewFormats: viewFormats,
                dimension: "3d",
            });
        }
    }

    // Handle (re)creation of texture RW_nodeTexture_ReadOnly
    {
        const baseSize = this.texture_NodeTexture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_NodeTexture_format;
        if (this.texture_RW_nodeTexture_ReadOnly !== null && (this.texture_RW_nodeTexture_ReadOnly_format != desiredFormat || this.texture_RW_nodeTexture_ReadOnly_size[0] != desiredSize[0] || this.texture_RW_nodeTexture_ReadOnly_size[1] != desiredSize[1] || this.texture_RW_nodeTexture_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_RW_nodeTexture_ReadOnly.destroy();
            this.texture_RW_nodeTexture_ReadOnly = null;
        }

        if (this.texture_RW_nodeTexture_ReadOnly === null)
        {
            this.texture_RW_nodeTexture_ReadOnly_size = desiredSize.slice();
            this.texture_RW_nodeTexture_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_RW_nodeTexture_ReadOnly_format))
                viewFormats.push(this.texture_RW_nodeTexture_ReadOnly_format);

            this.texture_RW_nodeTexture_ReadOnly = device.createTexture({
                label: "texture Texture3DRW_CS.RW_nodeTexture_ReadOnly",
                size: this.texture_RW_nodeTexture_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_RW_nodeTexture_ReadOnly_format),
                usage: this.texture_RW_nodeTexture_ReadOnly_usageFlags,
                viewFormats: viewFormats,
                dimension: "3d",
            });
        }
    }

    // Handle (re)creation of texture RW_importedTexture_ReadOnly
    {
        const baseSize = this.texture_ImportedTexture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_ImportedTexture_format;
        if (this.texture_RW_importedTexture_ReadOnly !== null && (this.texture_RW_importedTexture_ReadOnly_format != desiredFormat || this.texture_RW_importedTexture_ReadOnly_size[0] != desiredSize[0] || this.texture_RW_importedTexture_ReadOnly_size[1] != desiredSize[1] || this.texture_RW_importedTexture_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_RW_importedTexture_ReadOnly.destroy();
            this.texture_RW_importedTexture_ReadOnly = null;
        }

        if (this.texture_RW_importedTexture_ReadOnly === null)
        {
            this.texture_RW_importedTexture_ReadOnly_size = desiredSize.slice();
            this.texture_RW_importedTexture_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_RW_importedTexture_ReadOnly_format))
                viewFormats.push(this.texture_RW_importedTexture_ReadOnly_format);

            this.texture_RW_importedTexture_ReadOnly = device.createTexture({
                label: "texture Texture3DRW_CS.RW_importedTexture_ReadOnly",
                size: this.texture_RW_importedTexture_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_RW_importedTexture_ReadOnly_format),
                usage: this.texture_RW_importedTexture_ReadOnly_usageFlags,
                viewFormats: viewFormats,
                dimension: "3d",
            });
        }
    }

    // Load texture _loadedTexture_0 from "ImageB%i.png"
    if (this.texture__loadedTexture_0 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/ImageB%i.png", this.texture__loadedTexture_0_usageFlags, "3d");
        this.texture__loadedTexture_0 = loadedTex.texture;
        this.texture__loadedTexture_0_size = loadedTex.size;
        this.texture__loadedTexture_0_format = "rgba8unorm";
        this.texture__loadedTexture_0_usageFlags = loadedTex.usageFlags;
    }
    // (Re)create compute shader RW
    {
        const bindGroupEntries =
        [
            {
                // nodeTexture
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_NodeTexture_format), viewDimension: "3d" }
            },
            {
                // importedTexture
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_ImportedTexture_format), viewDimension: "3d" }
            },
            {
                // importedColor
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "3d", sampleType: Shared.GetTextureFormatInfo(this.texture_ImportedColor_format).sampleType }
            },
            {
                // nodeTextureReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_RW_nodeTexture_ReadOnly_format), viewDimension: "3d" }
            },
            {
                // importedTextureReadOnly
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_RW_importedTexture_ReadOnly_format), viewDimension: "3d" }
            },
            {
                // _loadedTexture_0
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "3d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_RW === null || newHash !== this.Hash_Compute_RW)
        {
            this.Hash_Compute_RW = newHash;

            let shaderCode = class_Texture3DRW_CS.ShaderCode_RW_RWCS_0;
            shaderCode = shaderCode.replace("/*(nodeTexture_format)*/", Shared.GetNonSRGBFormat(this.texture_NodeTexture_format));
            shaderCode = shaderCode.replace("/*(importedTexture_format)*/", Shared.GetNonSRGBFormat(this.texture_ImportedTexture_format));
            shaderCode = shaderCode.replace("/*(nodeTextureReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_RW_nodeTexture_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(importedTextureReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_RW_importedTexture_ReadOnly_format));

            this.ShaderModule_Compute_RW = device.createShaderModule({ code: shaderCode, label: "Compute Shader RW"});
            this.BindGroupLayout_Compute_RW = device.createBindGroupLayout({
                label: "Compute Bind Group Layout RW",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_RW = device.createPipelineLayout({
                label: "Compute Pipeline Layout RW",
                bindGroupLayouts: [this.BindGroupLayout_Compute_RW],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_RW = device.createComputePipeline({
                    label: "Compute Pipeline RW",
                    layout: this.PipelineLayout_Compute_RW,
                    compute: {
                        module: this.ShaderModule_Compute_RW,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("RW");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline RW",
                    layout: this.PipelineLayout_Compute_RW,
                    compute: {
                        module: this.ShaderModule_Compute_RW,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_RW = handle; this.loadingPromises.delete("RW"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("Texture3DRW_CS.NodeTexture");

    encoder.popDebugGroup(); // "Texture3DRW_CS.NodeTexture"

    encoder.pushDebugGroup("Texture3DRW_CS.ImportedTexture");

    encoder.popDebugGroup(); // "Texture3DRW_CS.ImportedTexture"

    encoder.pushDebugGroup("Texture3DRW_CS.ImportedColor");

    encoder.popDebugGroup(); // "Texture3DRW_CS.ImportedColor"

    encoder.pushDebugGroup("Texture3DRW_CS.RW_nodeTexture_ReadOnly");

    encoder.popDebugGroup(); // "Texture3DRW_CS.RW_nodeTexture_ReadOnly"

    encoder.pushDebugGroup("Texture3DRW_CS.Copy_RW_nodeTexture");

    // Copy texture NodeTexture to texture RW_nodeTexture_ReadOnly
    {
        const numMips = Math.min(this.texture_NodeTexture.mipLevelCount, this.texture_RW_nodeTexture_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_NodeTexture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_NodeTexture.height >> mipIndex, 1);
            let mipDepth = this.texture_NodeTexture.depthOrArrayLayers;

            if (this.texture_NodeTexture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_NodeTexture, mipLevel: mipIndex },
                { texture: this.texture_RW_nodeTexture_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Texture3DRW_CS.Copy_RW_nodeTexture"

    encoder.pushDebugGroup("Texture3DRW_CS.RW_importedTexture_ReadOnly");

    encoder.popDebugGroup(); // "Texture3DRW_CS.RW_importedTexture_ReadOnly"

    encoder.pushDebugGroup("Texture3DRW_CS.Copy_RW_importedTexture");

    // Copy texture ImportedTexture to texture RW_importedTexture_ReadOnly
    {
        const numMips = Math.min(this.texture_ImportedTexture.mipLevelCount, this.texture_RW_importedTexture_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_ImportedTexture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_ImportedTexture.height >> mipIndex, 1);
            let mipDepth = this.texture_ImportedTexture.depthOrArrayLayers;

            if (this.texture_ImportedTexture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_ImportedTexture, mipLevel: mipIndex },
                { texture: this.texture_RW_importedTexture_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Texture3DRW_CS.Copy_RW_importedTexture"

    encoder.pushDebugGroup("Texture3DRW_CS._loadedTexture_0");

    encoder.popDebugGroup(); // "Texture3DRW_CS._loadedTexture_0"

    encoder.pushDebugGroup("Texture3DRW_CS.RW");

    // Run compute shader RW
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group RW",
            layout: this.BindGroupLayout_Compute_RW,
            entries: [
                {
                    // nodeTexture
                    binding: 0,
                    resource: this.texture_NodeTexture.createView({ dimension: "3d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // importedTexture
                    binding: 1,
                    resource: this.texture_ImportedTexture.createView({ dimension: "3d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // importedColor
                    binding: 2,
                    resource: this.texture_ImportedColor.createView({ dimension: "3d", format: this.texture_ImportedColor_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // nodeTextureReadOnly
                    binding: 3,
                    resource: this.texture_RW_nodeTexture_ReadOnly.createView({ dimension: "3d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // importedTextureReadOnly
                    binding: 4,
                    resource: this.texture_RW_importedTexture_ReadOnly.createView({ dimension: "3d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 5,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "3d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_NodeTexture_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 4 - 1) / 4),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 4 - 1) / 4),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 4 - 1) / 4)
        ];

        if (this.Pipeline_Compute_RW !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_RW);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "Texture3DRW_CS.RW"

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

var Texture3DRW_CS = new class_Texture3DRW_CS;

export default Texture3DRW_CS;
