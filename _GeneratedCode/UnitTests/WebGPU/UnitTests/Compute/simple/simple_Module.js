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

class class_simple
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Simple_0", node "DoSimpleCS"
static ShaderCode_DoSimpleCS_Simple_0 = `
@binding(1) @group(0) var InputReadOnly : texture_storage_2d</*(InputReadOnly_format)*/, read>;

@binding(0) @group(0) var Input : texture_storage_2d</*(Input_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn Main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : vec2<u32> = DTid_0.xy;
    var _S2 : vec4<f32> = (textureLoad((InputReadOnly), (vec2<i32>(_S1))));
    var shade_0 : f32 = dot(_S2.xzy, vec3<f32>(0.30000001192092896f, 0.5899999737739563f, 0.10999999940395355f));
    textureStore((Input), (_S1), (vec4<f32>(shade_0, shade_0, shade_0, _S2.w)));
    return;
}

`;

// -------------------- Private Members

// Texture DoSimpleCS_Input_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_DoSimpleCS_Input_ReadOnly = null;
texture_DoSimpleCS_Input_ReadOnly_size = [0, 0, 0];
texture_DoSimpleCS_Input_ReadOnly_format = "";
texture_DoSimpleCS_Input_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader DoSimpleCS
Hash_Compute_DoSimpleCS = 0;
ShaderModule_Compute_DoSimpleCS = null;
BindGroupLayout_Compute_DoSimpleCS = null;
PipelineLayout_Compute_DoSimpleCS = null;
Pipeline_Compute_DoSimpleCS = null;

// -------------------- Imported Members

// Texture Input : This is the input texture to be modified
texture_Input = null;
texture_Input_size = [0, 0, 0];
texture_Input_format = "";
texture_Input_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Private Variables

variable_DummyConstVar = 1;
variableDefault_DummyConstVar = 1;
variableChanged_DummyConstVar = false;

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
    // Handle (re)creation of texture DoSimpleCS_Input_ReadOnly
    {
        const baseSize = this.texture_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Input_format;
        if (this.texture_DoSimpleCS_Input_ReadOnly !== null && (this.texture_DoSimpleCS_Input_ReadOnly_format != desiredFormat || this.texture_DoSimpleCS_Input_ReadOnly_size[0] != desiredSize[0] || this.texture_DoSimpleCS_Input_ReadOnly_size[1] != desiredSize[1] || this.texture_DoSimpleCS_Input_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_DoSimpleCS_Input_ReadOnly.destroy();
            this.texture_DoSimpleCS_Input_ReadOnly = null;
        }

        if (this.texture_DoSimpleCS_Input_ReadOnly === null)
        {
            this.texture_DoSimpleCS_Input_ReadOnly_size = desiredSize.slice();
            this.texture_DoSimpleCS_Input_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_DoSimpleCS_Input_ReadOnly_format))
                viewFormats.push(this.texture_DoSimpleCS_Input_ReadOnly_format);

            this.texture_DoSimpleCS_Input_ReadOnly = device.createTexture({
                label: "texture simple.DoSimpleCS_Input_ReadOnly",
                size: this.texture_DoSimpleCS_Input_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_DoSimpleCS_Input_ReadOnly_format),
                usage: this.texture_DoSimpleCS_Input_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader DoSimpleCS
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Input_format), viewDimension: "2d" }
            },
            {
                // InputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_DoSimpleCS_Input_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_DoSimpleCS === null || newHash !== this.Hash_Compute_DoSimpleCS)
        {
            this.Hash_Compute_DoSimpleCS = newHash;

            let shaderCode = class_simple.ShaderCode_DoSimpleCS_Simple_0;
            shaderCode = shaderCode.replace("/*(Input_format)*/", Shared.GetNonSRGBFormat(this.texture_Input_format));
            shaderCode = shaderCode.replace("/*(InputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_DoSimpleCS_Input_ReadOnly_format));

            this.ShaderModule_Compute_DoSimpleCS = device.createShaderModule({ code: shaderCode, label: "Compute Shader DoSimpleCS"});
            this.BindGroupLayout_Compute_DoSimpleCS = device.createBindGroupLayout({
                label: "Compute Bind Group Layout DoSimpleCS",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_DoSimpleCS = device.createPipelineLayout({
                label: "Compute Pipeline Layout DoSimpleCS",
                bindGroupLayouts: [this.BindGroupLayout_Compute_DoSimpleCS],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_DoSimpleCS = device.createComputePipeline({
                    label: "Compute Pipeline DoSimpleCS",
                    layout: this.PipelineLayout_Compute_DoSimpleCS,
                    compute: {
                        module: this.ShaderModule_Compute_DoSimpleCS,
                        entryPoint: "Main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("DoSimpleCS");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline DoSimpleCS",
                    layout: this.PipelineLayout_Compute_DoSimpleCS,
                    compute: {
                        module: this.ShaderModule_Compute_DoSimpleCS,
                        entryPoint: "Main",
                    }
                }).then( handle => { this.Pipeline_Compute_DoSimpleCS = handle; this.loadingPromises.delete("DoSimpleCS"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("simple.Input");

    encoder.popDebugGroup(); // "simple.Input"

    encoder.pushDebugGroup("simple.DoSimpleCS_Input_ReadOnly");

    encoder.popDebugGroup(); // "simple.DoSimpleCS_Input_ReadOnly"

    encoder.pushDebugGroup("simple.Copy_DoSimpleCS_Input");

    // Copy texture Input to texture DoSimpleCS_Input_ReadOnly
    {
        const numMips = Math.min(this.texture_Input.mipLevelCount, this.texture_DoSimpleCS_Input_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Input.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Input.height >> mipIndex, 1);
            let mipDepth = this.texture_Input.depthOrArrayLayers;

            if (this.texture_Input.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Input, mipLevel: mipIndex },
                { texture: this.texture_DoSimpleCS_Input_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "simple.Copy_DoSimpleCS_Input"

    encoder.pushDebugGroup("simple.DoSimpleCS");

    // Run compute shader DoSimpleCS
    // Runs the shader
    if (this.variable_DummyConstVar == 1)
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group DoSimpleCS",
            layout: this.BindGroupLayout_Compute_DoSimpleCS,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Input.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // InputReadOnly
                    binding: 1,
                    resource: this.texture_DoSimpleCS_Input_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
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

        if (this.Pipeline_Compute_DoSimpleCS !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_DoSimpleCS);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "simple.DoSimpleCS"

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

var simple = new class_simple;

export default simple;
