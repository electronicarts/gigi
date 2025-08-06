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

class class_SubGraphTest
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Swap_Colors", node "Swap_Colors"
static ShaderCode_Swap_Colors_Swap_Colors = `
@binding(1) @group(0) var OutputReadOnly : texture_storage_2d</*(OutputReadOnly_format)*/, read>;

@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : vec2<u32> = DTid_0.xy;
    var _S2 : vec4<f32> = (textureLoad((OutputReadOnly), (vec2<i32>(_S1))));
    var _S3 : vec4<f32> = _S2.yzxw;
    // Manual fix for slang bug: https://github.com/shader-slang/slang/issues/6551
    textureStore((Output), (_S1), (_S3));
    return;
}

`;

// -------------------- Private Members

// Texture Swap_Colors_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Swap_Colors_Output_ReadOnly = null;
texture_Swap_Colors_Output_ReadOnly_size = [0, 0, 0];
texture_Swap_Colors_Output_ReadOnly_format = "";
texture_Swap_Colors_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader Swap_Colors
Hash_Compute_Swap_Colors = 0;
ShaderModule_Compute_Swap_Colors = null;
BindGroupLayout_Compute_Swap_Colors = null;
PipelineLayout_Compute_Swap_Colors = null;
Pipeline_Compute_Swap_Colors = null;

// -------------------- Imported Members

// Texture Test
texture_Test = null;
texture_Test_size = [0, 0, 0];
texture_Test_format = "";
texture_Test_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC;

// -------------------- Exported Members

// Texture Inner_Exported_Tex
texture_Inner_Exported_Tex = null;
texture_Inner_Exported_Tex_size = [0, 0, 0];
texture_Inner_Exported_Tex_format = "";
texture_Inner_Exported_Tex_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate texture Test
    if (this.texture_Test === null)
    {
        Shared.LogError("Imported resource texture_Test was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Inner_Exported_Tex
    {
        const baseSize = this.texture_Test_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Test_format;
        if (this.texture_Inner_Exported_Tex !== null && (this.texture_Inner_Exported_Tex_format != desiredFormat || this.texture_Inner_Exported_Tex_size[0] != desiredSize[0] || this.texture_Inner_Exported_Tex_size[1] != desiredSize[1] || this.texture_Inner_Exported_Tex_size[2] != desiredSize[2]))
        {
            this.texture_Inner_Exported_Tex.destroy();
            this.texture_Inner_Exported_Tex = null;
        }

        if (this.texture_Inner_Exported_Tex === null)
        {
            this.texture_Inner_Exported_Tex_size = desiredSize.slice();
            this.texture_Inner_Exported_Tex_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Inner_Exported_Tex_format))
                viewFormats.push(this.texture_Inner_Exported_Tex_format);

            this.texture_Inner_Exported_Tex = device.createTexture({
                label: "texture SubGraphTest.Inner_Exported_Tex",
                size: this.texture_Inner_Exported_Tex_size,
                format: Shared.GetNonSRGBFormat(this.texture_Inner_Exported_Tex_format),
                usage: this.texture_Inner_Exported_Tex_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Swap_Colors_Output_ReadOnly
    {
        const baseSize = this.texture_Inner_Exported_Tex_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Inner_Exported_Tex_format;
        if (this.texture_Swap_Colors_Output_ReadOnly !== null && (this.texture_Swap_Colors_Output_ReadOnly_format != desiredFormat || this.texture_Swap_Colors_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Swap_Colors_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Swap_Colors_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Swap_Colors_Output_ReadOnly.destroy();
            this.texture_Swap_Colors_Output_ReadOnly = null;
        }

        if (this.texture_Swap_Colors_Output_ReadOnly === null)
        {
            this.texture_Swap_Colors_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Swap_Colors_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Swap_Colors_Output_ReadOnly_format))
                viewFormats.push(this.texture_Swap_Colors_Output_ReadOnly_format);

            this.texture_Swap_Colors_Output_ReadOnly = device.createTexture({
                label: "texture SubGraphTest.Swap_Colors_Output_ReadOnly",
                size: this.texture_Swap_Colors_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Swap_Colors_Output_ReadOnly_format),
                usage: this.texture_Swap_Colors_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Swap_Colors
    {
        const bindGroupEntries =
        [
            {
                // Output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Inner_Exported_Tex_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Swap_Colors_Output_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Swap_Colors === null || newHash !== this.Hash_Compute_Swap_Colors)
        {
            this.Hash_Compute_Swap_Colors = newHash;

            let shaderCode = class_SubGraphTest.ShaderCode_Swap_Colors_Swap_Colors;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Inner_Exported_Tex_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Swap_Colors_Output_ReadOnly_format));

            this.ShaderModule_Compute_Swap_Colors = device.createShaderModule({ code: shaderCode, label: "Compute Shader Swap_Colors"});
            this.BindGroupLayout_Compute_Swap_Colors = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Swap_Colors",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Swap_Colors = device.createPipelineLayout({
                label: "Compute Pipeline Layout Swap_Colors",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Swap_Colors],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Swap_Colors = device.createComputePipeline({
                    label: "Compute Pipeline Swap_Colors",
                    layout: this.PipelineLayout_Compute_Swap_Colors,
                    compute: {
                        module: this.ShaderModule_Compute_Swap_Colors,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Swap_Colors");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Swap_Colors",
                    layout: this.PipelineLayout_Compute_Swap_Colors,
                    compute: {
                        module: this.ShaderModule_Compute_Swap_Colors,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Swap_Colors = handle; this.loadingPromises.delete("Swap_Colors"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("SubGraphTest.Test");

    encoder.popDebugGroup(); // "SubGraphTest.Test"

    encoder.pushDebugGroup("SubGraphTest.Inner_Exported_Tex");

    encoder.popDebugGroup(); // "SubGraphTest.Inner_Exported_Tex"

    encoder.pushDebugGroup("SubGraphTest.Inner_Copy_Import_To_Export");

    // Copy texture Test to texture Inner_Exported_Tex
    {
        const numMips = Math.min(this.texture_Test.mipLevelCount, this.texture_Inner_Exported_Tex.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Test.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Test.height >> mipIndex, 1);
            let mipDepth = this.texture_Test.depthOrArrayLayers;

            if (this.texture_Test.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Test, mipLevel: mipIndex },
                { texture: this.texture_Inner_Exported_Tex, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphTest.Inner_Copy_Import_To_Export"

    encoder.pushDebugGroup("SubGraphTest.Swap_Colors_Output_ReadOnly");

    encoder.popDebugGroup(); // "SubGraphTest.Swap_Colors_Output_ReadOnly"

    encoder.pushDebugGroup("SubGraphTest.Copy_Swap_Colors_Output");

    // Copy texture Inner_Exported_Tex to texture Swap_Colors_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Inner_Exported_Tex.mipLevelCount, this.texture_Swap_Colors_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Inner_Exported_Tex.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Inner_Exported_Tex.height >> mipIndex, 1);
            let mipDepth = this.texture_Inner_Exported_Tex.depthOrArrayLayers;

            if (this.texture_Inner_Exported_Tex.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Inner_Exported_Tex, mipLevel: mipIndex },
                { texture: this.texture_Swap_Colors_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphTest.Copy_Swap_Colors_Output"

    encoder.pushDebugGroup("SubGraphTest.Swap_Colors");

    // Run compute shader Swap_Colors
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Swap_Colors",
            layout: this.BindGroupLayout_Compute_Swap_Colors,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Inner_Exported_Tex.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Swap_Colors_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Test_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Swap_Colors !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Swap_Colors);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SubGraphTest.Swap_Colors"

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

var SubGraphTest = new class_SubGraphTest;

export default SubGraphTest;
