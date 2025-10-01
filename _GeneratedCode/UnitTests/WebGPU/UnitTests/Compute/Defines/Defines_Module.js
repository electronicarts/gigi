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

class class_Defines
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "WriteOutputCS_0", node "WriteOutput"
static ShaderCode_WriteOutput_WriteOutputCS_0 = `
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    textureStore((Output), (DTid_0.xy), (vec4<f32>(0.75f, 0.5f, 1.0f, 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture WriteOutput_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_WriteOutput_Output_ReadOnly = null;
texture_WriteOutput_Output_ReadOnly_size = [0, 0, 0];
texture_WriteOutput_Output_ReadOnly_format = "";
texture_WriteOutput_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader WriteOutput
Hash_Compute_WriteOutput = 0;
ShaderModule_Compute_WriteOutput = null;
BindGroupLayout_Compute_WriteOutput = null;
PipelineLayout_Compute_WriteOutput = null;
Pipeline_Compute_WriteOutput = null;

// -------------------- Exported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Output
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm";
        if (this.texture_Output !== null && (this.texture_Output_format != desiredFormat || this.texture_Output_size[0] != desiredSize[0] || this.texture_Output_size[1] != desiredSize[1] || this.texture_Output_size[2] != desiredSize[2]))
        {
            this.texture_Output.destroy();
            this.texture_Output = null;
        }

        if (this.texture_Output === null)
        {
            this.texture_Output_size = desiredSize.slice();
            this.texture_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Output_format))
                viewFormats.push(this.texture_Output_format);

            this.texture_Output = device.createTexture({
                label: "texture Defines.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture WriteOutput_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_WriteOutput_Output_ReadOnly !== null && (this.texture_WriteOutput_Output_ReadOnly_format != desiredFormat || this.texture_WriteOutput_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_WriteOutput_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_WriteOutput_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_WriteOutput_Output_ReadOnly.destroy();
            this.texture_WriteOutput_Output_ReadOnly = null;
        }

        if (this.texture_WriteOutput_Output_ReadOnly === null)
        {
            this.texture_WriteOutput_Output_ReadOnly_size = desiredSize.slice();
            this.texture_WriteOutput_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_WriteOutput_Output_ReadOnly_format))
                viewFormats.push(this.texture_WriteOutput_Output_ReadOnly_format);

            this.texture_WriteOutput_Output_ReadOnly = device.createTexture({
                label: "texture Defines.WriteOutput_Output_ReadOnly",
                size: this.texture_WriteOutput_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_WriteOutput_Output_ReadOnly_format),
                usage: this.texture_WriteOutput_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader WriteOutput
    {
        const bindGroupEntries =
        [
            {
                // Output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_WriteOutput_Output_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_WriteOutput === null || newHash !== this.Hash_Compute_WriteOutput)
        {
            this.Hash_Compute_WriteOutput = newHash;

            let shaderCode = class_Defines.ShaderCode_WriteOutput_WriteOutputCS_0;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_WriteOutput_Output_ReadOnly_format));

            this.ShaderModule_Compute_WriteOutput = device.createShaderModule({ code: shaderCode, label: "Compute Shader WriteOutput"});
            this.BindGroupLayout_Compute_WriteOutput = device.createBindGroupLayout({
                label: "Compute Bind Group Layout WriteOutput",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_WriteOutput = device.createPipelineLayout({
                label: "Compute Pipeline Layout WriteOutput",
                bindGroupLayouts: [this.BindGroupLayout_Compute_WriteOutput],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_WriteOutput = device.createComputePipeline({
                    label: "Compute Pipeline WriteOutput",
                    layout: this.PipelineLayout_Compute_WriteOutput,
                    compute: {
                        module: this.ShaderModule_Compute_WriteOutput,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("WriteOutput");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline WriteOutput",
                    layout: this.PipelineLayout_Compute_WriteOutput,
                    compute: {
                        module: this.ShaderModule_Compute_WriteOutput,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_WriteOutput = handle; this.loadingPromises.delete("WriteOutput"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("Defines.Output");

    encoder.popDebugGroup(); // "Defines.Output"

    encoder.pushDebugGroup("Defines.WriteOutput_Output_ReadOnly");

    encoder.popDebugGroup(); // "Defines.WriteOutput_Output_ReadOnly"

    encoder.pushDebugGroup("Defines.Copy_WriteOutput_Output");

    // Copy texture Output to texture WriteOutput_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_WriteOutput_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_WriteOutput_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Defines.Copy_WriteOutput_Output"

    encoder.pushDebugGroup("Defines.WriteOutput");

    // Run compute shader WriteOutput
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group WriteOutput",
            layout: this.BindGroupLayout_Compute_WriteOutput,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_WriteOutput_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
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

        if (this.Pipeline_Compute_WriteOutput !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_WriteOutput);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "Defines.WriteOutput"

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

var Defines = new class_Defines;

export default Defines;
