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

class class_BarrierTest
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Left", node "Draw_Left"
static ShaderCode_Draw_Left_Left = `
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    textureStore((Output), (DTid_0.xy), (vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "Right", node "Draw_Right"
static ShaderCode_Draw_Right_Right = `
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var dims_0 : vec2<u32>;
    var _S1 : u32 = dims_0[i32(0)];
    var _S2 : u32 = dims_0[i32(1)];
    {var dim = textureDimensions((Output));((_S1)) = dim.x;((_S2)) = dim.y;};
    dims_0[i32(0)] = _S1;
    dims_0[i32(1)] = _S2;
    textureStore((Output), (DTid_0.xy + vec2<u32>(dims_0.x / u32(2), u32(0))), (vec4<f32>(0.0f, 0.5f, 0.0f, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "After", node "After"
static ShaderCode_After_After = `
@binding(1) @group(0) var OutputReadOnly : texture_storage_2d</*(OutputReadOnly_format)*/, read>;

@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : vec2<u32> = DTid_0.xy;
    var _S2 : vec4<f32> = (textureLoad((OutputReadOnly), (vec2<i32>(_S1))));
    textureStore((Output), (_S1), (vec4<f32>(_S2.xyz * vec3<f32>(2.0f), 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture Draw_Left_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Draw_Left_Output_ReadOnly = null;
texture_Draw_Left_Output_ReadOnly_size = [0, 0, 0];
texture_Draw_Left_Output_ReadOnly_format = "";
texture_Draw_Left_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader Draw_Left
Hash_Compute_Draw_Left = 0;
ShaderModule_Compute_Draw_Left = null;
BindGroupLayout_Compute_Draw_Left = null;
PipelineLayout_Compute_Draw_Left = null;
Pipeline_Compute_Draw_Left = null;

// Texture Draw_Right_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Draw_Right_Output_ReadOnly = null;
texture_Draw_Right_Output_ReadOnly_size = [0, 0, 0];
texture_Draw_Right_Output_ReadOnly_format = "";
texture_Draw_Right_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader Draw_Right
Hash_Compute_Draw_Right = 0;
ShaderModule_Compute_Draw_Right = null;
BindGroupLayout_Compute_Draw_Right = null;
PipelineLayout_Compute_Draw_Right = null;
Pipeline_Compute_Draw_Right = null;

// Texture After_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_After_Output_ReadOnly = null;
texture_After_Output_ReadOnly_size = [0, 0, 0];
texture_After_Output_ReadOnly_format = "";
texture_After_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader After
Hash_Compute_After = 0;
ShaderModule_Compute_After = null;
BindGroupLayout_Compute_After = null;
PipelineLayout_Compute_After = null;
Pipeline_Compute_After = null;

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
            Math.floor(((parseInt(baseSize[0]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 512) / 1) + 0,
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
                label: "texture BarrierTest.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Draw_Left_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Draw_Left_Output_ReadOnly !== null && (this.texture_Draw_Left_Output_ReadOnly_format != desiredFormat || this.texture_Draw_Left_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Draw_Left_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Draw_Left_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Draw_Left_Output_ReadOnly.destroy();
            this.texture_Draw_Left_Output_ReadOnly = null;
        }

        if (this.texture_Draw_Left_Output_ReadOnly === null)
        {
            this.texture_Draw_Left_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Draw_Left_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Draw_Left_Output_ReadOnly_format))
                viewFormats.push(this.texture_Draw_Left_Output_ReadOnly_format);

            this.texture_Draw_Left_Output_ReadOnly = device.createTexture({
                label: "texture BarrierTest.Draw_Left_Output_ReadOnly",
                size: this.texture_Draw_Left_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Draw_Left_Output_ReadOnly_format),
                usage: this.texture_Draw_Left_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Draw_Left
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
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Draw_Left_Output_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Draw_Left === null || newHash !== this.Hash_Compute_Draw_Left)
        {
            this.Hash_Compute_Draw_Left = newHash;

            let shaderCode = class_BarrierTest.ShaderCode_Draw_Left_Left;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Draw_Left_Output_ReadOnly_format));

            this.ShaderModule_Compute_Draw_Left = device.createShaderModule({ code: shaderCode, label: "Compute Shader Draw_Left"});
            this.BindGroupLayout_Compute_Draw_Left = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Draw_Left",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Draw_Left = device.createPipelineLayout({
                label: "Compute Pipeline Layout Draw_Left",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Draw_Left],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Draw_Left = device.createComputePipeline({
                    label: "Compute Pipeline Draw_Left",
                    layout: this.PipelineLayout_Compute_Draw_Left,
                    compute: {
                        module: this.ShaderModule_Compute_Draw_Left,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Draw_Left");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Draw_Left",
                    layout: this.PipelineLayout_Compute_Draw_Left,
                    compute: {
                        module: this.ShaderModule_Compute_Draw_Left,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Draw_Left = handle; this.loadingPromises.delete("Draw_Left"); } );
            }
        }
    }

    // Handle (re)creation of texture Draw_Right_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Draw_Right_Output_ReadOnly !== null && (this.texture_Draw_Right_Output_ReadOnly_format != desiredFormat || this.texture_Draw_Right_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Draw_Right_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Draw_Right_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Draw_Right_Output_ReadOnly.destroy();
            this.texture_Draw_Right_Output_ReadOnly = null;
        }

        if (this.texture_Draw_Right_Output_ReadOnly === null)
        {
            this.texture_Draw_Right_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Draw_Right_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Draw_Right_Output_ReadOnly_format))
                viewFormats.push(this.texture_Draw_Right_Output_ReadOnly_format);

            this.texture_Draw_Right_Output_ReadOnly = device.createTexture({
                label: "texture BarrierTest.Draw_Right_Output_ReadOnly",
                size: this.texture_Draw_Right_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Draw_Right_Output_ReadOnly_format),
                usage: this.texture_Draw_Right_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Draw_Right
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
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Draw_Right_Output_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Draw_Right === null || newHash !== this.Hash_Compute_Draw_Right)
        {
            this.Hash_Compute_Draw_Right = newHash;

            let shaderCode = class_BarrierTest.ShaderCode_Draw_Right_Right;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Draw_Right_Output_ReadOnly_format));

            this.ShaderModule_Compute_Draw_Right = device.createShaderModule({ code: shaderCode, label: "Compute Shader Draw_Right"});
            this.BindGroupLayout_Compute_Draw_Right = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Draw_Right",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Draw_Right = device.createPipelineLayout({
                label: "Compute Pipeline Layout Draw_Right",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Draw_Right],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Draw_Right = device.createComputePipeline({
                    label: "Compute Pipeline Draw_Right",
                    layout: this.PipelineLayout_Compute_Draw_Right,
                    compute: {
                        module: this.ShaderModule_Compute_Draw_Right,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Draw_Right");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Draw_Right",
                    layout: this.PipelineLayout_Compute_Draw_Right,
                    compute: {
                        module: this.ShaderModule_Compute_Draw_Right,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Draw_Right = handle; this.loadingPromises.delete("Draw_Right"); } );
            }
        }
    }

    // Handle (re)creation of texture After_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_After_Output_ReadOnly !== null && (this.texture_After_Output_ReadOnly_format != desiredFormat || this.texture_After_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_After_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_After_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_After_Output_ReadOnly.destroy();
            this.texture_After_Output_ReadOnly = null;
        }

        if (this.texture_After_Output_ReadOnly === null)
        {
            this.texture_After_Output_ReadOnly_size = desiredSize.slice();
            this.texture_After_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_After_Output_ReadOnly_format))
                viewFormats.push(this.texture_After_Output_ReadOnly_format);

            this.texture_After_Output_ReadOnly = device.createTexture({
                label: "texture BarrierTest.After_Output_ReadOnly",
                size: this.texture_After_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_After_Output_ReadOnly_format),
                usage: this.texture_After_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader After
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
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_After_Output_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_After === null || newHash !== this.Hash_Compute_After)
        {
            this.Hash_Compute_After = newHash;

            let shaderCode = class_BarrierTest.ShaderCode_After_After;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_After_Output_ReadOnly_format));

            this.ShaderModule_Compute_After = device.createShaderModule({ code: shaderCode, label: "Compute Shader After"});
            this.BindGroupLayout_Compute_After = device.createBindGroupLayout({
                label: "Compute Bind Group Layout After",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_After = device.createPipelineLayout({
                label: "Compute Pipeline Layout After",
                bindGroupLayouts: [this.BindGroupLayout_Compute_After],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_After = device.createComputePipeline({
                    label: "Compute Pipeline After",
                    layout: this.PipelineLayout_Compute_After,
                    compute: {
                        module: this.ShaderModule_Compute_After,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("After");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline After",
                    layout: this.PipelineLayout_Compute_After,
                    compute: {
                        module: this.ShaderModule_Compute_After,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_After = handle; this.loadingPromises.delete("After"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("BarrierTest.Output");

    encoder.popDebugGroup(); // "BarrierTest.Output"

    encoder.pushDebugGroup("BarrierTest.Draw_Left_Output_ReadOnly");

    encoder.popDebugGroup(); // "BarrierTest.Draw_Left_Output_ReadOnly"

    encoder.pushDebugGroup("BarrierTest.Copy_Draw_Left_Output");

    // Copy texture Output to texture Draw_Left_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Draw_Left_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Draw_Left_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "BarrierTest.Copy_Draw_Left_Output"

    encoder.pushDebugGroup("BarrierTest.Draw_Left");

    // Run compute shader Draw_Left
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Draw_Left",
            layout: this.BindGroupLayout_Compute_Draw_Left,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Draw_Left_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 2) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Draw_Left !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Draw_Left);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "BarrierTest.Draw_Left"

    encoder.pushDebugGroup("BarrierTest.Draw_Right_Output_ReadOnly");

    encoder.popDebugGroup(); // "BarrierTest.Draw_Right_Output_ReadOnly"

    encoder.pushDebugGroup("BarrierTest.Copy_Draw_Right_Output");

    // Copy texture Output to texture Draw_Right_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Draw_Right_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Draw_Right_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "BarrierTest.Copy_Draw_Right_Output"

    encoder.pushDebugGroup("BarrierTest.Draw_Right");

    // Run compute shader Draw_Right
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Draw_Right",
            layout: this.BindGroupLayout_Compute_Draw_Right,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Draw_Right_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 2) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Draw_Right !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Draw_Right);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "BarrierTest.Draw_Right"

    encoder.pushDebugGroup("BarrierTest.After_Output_ReadOnly");

    encoder.popDebugGroup(); // "BarrierTest.After_Output_ReadOnly"

    encoder.pushDebugGroup("BarrierTest.Copy_After_Output");

    // Copy texture Output to texture After_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_After_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_After_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "BarrierTest.Copy_After_Output"

    encoder.pushDebugGroup("BarrierTest.After");

    // Run compute shader After
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group After",
            layout: this.BindGroupLayout_Compute_After,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_After_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
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

        if (this.Pipeline_Compute_After !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_After);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "BarrierTest.After"

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

var BarrierTest = new class_BarrierTest;

export default BarrierTest;
