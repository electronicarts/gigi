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

class class_ConstOverride
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "ClearCS", node "Clear"
static ShaderCode_Clear_ClearCS = `
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    textureStore((Output), (DTid_0.xy), (vec4<f32>(0.5f, 0.5f, 0.5f, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "Left_WriteColorCS", node "Left_WriteColor"
static ShaderCode_Left_WriteColor_Left_WriteColorCS = `
@binding(0) @group(0) var Color : texture_storage_2d</*(Color_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : u32 = DTid_0.x;
    var _S2 : bool;
    if(_S1 >= u32(50))
    {
        _S2 = _S1 <= u32(100);
    }
    else
    {
        _S2 = false;
    }
    if(_S2)
    {
        textureStore((Color), (DTid_0.xy), (vec4<f32>(0.20000000298023224f, 0.80000001192092896f, 0.20000000298023224f, 1.0f)));
    }
    return;
}

`;

// Shader code for Compute shader "Right_WriteColorCS", node "Right_WriteColor"
static ShaderCode_Right_WriteColor_Right_WriteColorCS = `
@binding(0) @group(0) var Color : texture_storage_2d</*(Color_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : u32 = DTid_0.x;
    var _S2 : bool;
    if(_S1 >= u32(150))
    {
        _S2 = _S1 <= u32(200);
    }
    else
    {
        _S2 = false;
    }
    if(_S2)
    {
        textureStore((Color), (DTid_0.xy), (vec4<f32>(0.20000000298023224f, 0.80000001192092896f, 0.20000000298023224f, 1.0f)));
    }
    return;
}

`;

// -------------------- Private Members

// Texture Clear_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Clear_Output_ReadOnly = null;
texture_Clear_Output_ReadOnly_size = [0, 0, 0];
texture_Clear_Output_ReadOnly_format = "";
texture_Clear_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader Clear
Hash_Compute_Clear = 0;
ShaderModule_Compute_Clear = null;
BindGroupLayout_Compute_Clear = null;
PipelineLayout_Compute_Clear = null;
Pipeline_Compute_Clear = null;

// Texture Left_WriteColor_Color_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Left_WriteColor_Color_ReadOnly = null;
texture_Left_WriteColor_Color_ReadOnly_size = [0, 0, 0];
texture_Left_WriteColor_Color_ReadOnly_format = "";
texture_Left_WriteColor_Color_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader Left_WriteColor
Hash_Compute_Left_WriteColor = 0;
ShaderModule_Compute_Left_WriteColor = null;
BindGroupLayout_Compute_Left_WriteColor = null;
PipelineLayout_Compute_Left_WriteColor = null;
Pipeline_Compute_Left_WriteColor = null;

// Texture Right_WriteColor_Color_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Right_WriteColor_Color_ReadOnly = null;
texture_Right_WriteColor_Color_ReadOnly_size = [0, 0, 0];
texture_Right_WriteColor_Color_ReadOnly_format = "";
texture_Right_WriteColor_Color_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader Right_WriteColor
Hash_Compute_Right_WriteColor = 0;
ShaderModule_Compute_Right_WriteColor = null;
BindGroupLayout_Compute_Right_WriteColor = null;
PipelineLayout_Compute_Right_WriteColor = null;
Pipeline_Compute_Right_WriteColor = null;

// -------------------- Exported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Private Variables

variable___literal_0 = 50;  // Made to replace variable "MinX" with a constant value in subgraph node "Left"
variableDefault___literal_0 = 50;  // Made to replace variable "MinX" with a constant value in subgraph node "Left"
variableChanged___literal_0 = false;
variable___literal_1 = 100;  // Made to replace variable "MaxX" with a constant value in subgraph node "Left"
variableDefault___literal_1 = 100;  // Made to replace variable "MaxX" with a constant value in subgraph node "Left"
variableChanged___literal_1 = false;
variable___literal_2 = 150;  // Made to replace variable "MinX" with a constant value in subgraph node "Right"
variableDefault___literal_2 = 150;  // Made to replace variable "MinX" with a constant value in subgraph node "Right"
variableChanged___literal_2 = false;
variable___literal_3 = 200;  // Made to replace variable "MaxX" with a constant value in subgraph node "Right"
variableDefault___literal_3 = 200;  // Made to replace variable "MaxX" with a constant value in subgraph node "Right"
variableChanged___literal_3 = false;

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
        const desiredFormat = "rgba8unorm-srgb";
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
                label: "texture ConstOverride.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Clear_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Clear_Output_ReadOnly !== null && (this.texture_Clear_Output_ReadOnly_format != desiredFormat || this.texture_Clear_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Clear_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Clear_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Clear_Output_ReadOnly.destroy();
            this.texture_Clear_Output_ReadOnly = null;
        }

        if (this.texture_Clear_Output_ReadOnly === null)
        {
            this.texture_Clear_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Clear_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Clear_Output_ReadOnly_format))
                viewFormats.push(this.texture_Clear_Output_ReadOnly_format);

            this.texture_Clear_Output_ReadOnly = device.createTexture({
                label: "texture ConstOverride.Clear_Output_ReadOnly",
                size: this.texture_Clear_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Clear_Output_ReadOnly_format),
                usage: this.texture_Clear_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Clear
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
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Clear_Output_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Clear === null || newHash !== this.Hash_Compute_Clear)
        {
            this.Hash_Compute_Clear = newHash;

            let shaderCode = class_ConstOverride.ShaderCode_Clear_ClearCS;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Clear_Output_ReadOnly_format));

            this.ShaderModule_Compute_Clear = device.createShaderModule({ code: shaderCode, label: "Compute Shader Clear"});
            this.BindGroupLayout_Compute_Clear = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Clear",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Clear = device.createPipelineLayout({
                label: "Compute Pipeline Layout Clear",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Clear],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Clear = device.createComputePipeline({
                    label: "Compute Pipeline Clear",
                    layout: this.PipelineLayout_Compute_Clear,
                    compute: {
                        module: this.ShaderModule_Compute_Clear,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Clear");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Clear",
                    layout: this.PipelineLayout_Compute_Clear,
                    compute: {
                        module: this.ShaderModule_Compute_Clear,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Clear = handle; this.loadingPromises.delete("Clear"); } );
            }
        }
    }

    // Handle (re)creation of texture Left_WriteColor_Color_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Left_WriteColor_Color_ReadOnly !== null && (this.texture_Left_WriteColor_Color_ReadOnly_format != desiredFormat || this.texture_Left_WriteColor_Color_ReadOnly_size[0] != desiredSize[0] || this.texture_Left_WriteColor_Color_ReadOnly_size[1] != desiredSize[1] || this.texture_Left_WriteColor_Color_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Left_WriteColor_Color_ReadOnly.destroy();
            this.texture_Left_WriteColor_Color_ReadOnly = null;
        }

        if (this.texture_Left_WriteColor_Color_ReadOnly === null)
        {
            this.texture_Left_WriteColor_Color_ReadOnly_size = desiredSize.slice();
            this.texture_Left_WriteColor_Color_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Left_WriteColor_Color_ReadOnly_format))
                viewFormats.push(this.texture_Left_WriteColor_Color_ReadOnly_format);

            this.texture_Left_WriteColor_Color_ReadOnly = device.createTexture({
                label: "texture ConstOverride.Left_WriteColor_Color_ReadOnly",
                size: this.texture_Left_WriteColor_Color_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Left_WriteColor_Color_ReadOnly_format),
                usage: this.texture_Left_WriteColor_Color_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Left_WriteColor
    {
        const bindGroupEntries =
        [
            {
                // Color
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // ColorReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Left_WriteColor_Color_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Left_WriteColor === null || newHash !== this.Hash_Compute_Left_WriteColor)
        {
            this.Hash_Compute_Left_WriteColor = newHash;

            let shaderCode = class_ConstOverride.ShaderCode_Left_WriteColor_Left_WriteColorCS;
            shaderCode = shaderCode.replace("/*(Color_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(ColorReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Left_WriteColor_Color_ReadOnly_format));

            this.ShaderModule_Compute_Left_WriteColor = device.createShaderModule({ code: shaderCode, label: "Compute Shader Left_WriteColor"});
            this.BindGroupLayout_Compute_Left_WriteColor = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Left_WriteColor",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Left_WriteColor = device.createPipelineLayout({
                label: "Compute Pipeline Layout Left_WriteColor",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Left_WriteColor],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Left_WriteColor = device.createComputePipeline({
                    label: "Compute Pipeline Left_WriteColor",
                    layout: this.PipelineLayout_Compute_Left_WriteColor,
                    compute: {
                        module: this.ShaderModule_Compute_Left_WriteColor,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Left_WriteColor");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Left_WriteColor",
                    layout: this.PipelineLayout_Compute_Left_WriteColor,
                    compute: {
                        module: this.ShaderModule_Compute_Left_WriteColor,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Left_WriteColor = handle; this.loadingPromises.delete("Left_WriteColor"); } );
            }
        }
    }

    // Handle (re)creation of texture Right_WriteColor_Color_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Right_WriteColor_Color_ReadOnly !== null && (this.texture_Right_WriteColor_Color_ReadOnly_format != desiredFormat || this.texture_Right_WriteColor_Color_ReadOnly_size[0] != desiredSize[0] || this.texture_Right_WriteColor_Color_ReadOnly_size[1] != desiredSize[1] || this.texture_Right_WriteColor_Color_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Right_WriteColor_Color_ReadOnly.destroy();
            this.texture_Right_WriteColor_Color_ReadOnly = null;
        }

        if (this.texture_Right_WriteColor_Color_ReadOnly === null)
        {
            this.texture_Right_WriteColor_Color_ReadOnly_size = desiredSize.slice();
            this.texture_Right_WriteColor_Color_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Right_WriteColor_Color_ReadOnly_format))
                viewFormats.push(this.texture_Right_WriteColor_Color_ReadOnly_format);

            this.texture_Right_WriteColor_Color_ReadOnly = device.createTexture({
                label: "texture ConstOverride.Right_WriteColor_Color_ReadOnly",
                size: this.texture_Right_WriteColor_Color_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Right_WriteColor_Color_ReadOnly_format),
                usage: this.texture_Right_WriteColor_Color_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Right_WriteColor
    {
        const bindGroupEntries =
        [
            {
                // Color
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // ColorReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Right_WriteColor_Color_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Right_WriteColor === null || newHash !== this.Hash_Compute_Right_WriteColor)
        {
            this.Hash_Compute_Right_WriteColor = newHash;

            let shaderCode = class_ConstOverride.ShaderCode_Right_WriteColor_Right_WriteColorCS;
            shaderCode = shaderCode.replace("/*(Color_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(ColorReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Right_WriteColor_Color_ReadOnly_format));

            this.ShaderModule_Compute_Right_WriteColor = device.createShaderModule({ code: shaderCode, label: "Compute Shader Right_WriteColor"});
            this.BindGroupLayout_Compute_Right_WriteColor = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Right_WriteColor",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Right_WriteColor = device.createPipelineLayout({
                label: "Compute Pipeline Layout Right_WriteColor",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Right_WriteColor],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Right_WriteColor = device.createComputePipeline({
                    label: "Compute Pipeline Right_WriteColor",
                    layout: this.PipelineLayout_Compute_Right_WriteColor,
                    compute: {
                        module: this.ShaderModule_Compute_Right_WriteColor,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Right_WriteColor");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Right_WriteColor",
                    layout: this.PipelineLayout_Compute_Right_WriteColor,
                    compute: {
                        module: this.ShaderModule_Compute_Right_WriteColor,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Right_WriteColor = handle; this.loadingPromises.delete("Right_WriteColor"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("ConstOverride.Output");

    encoder.popDebugGroup(); // "ConstOverride.Output"

    encoder.pushDebugGroup("ConstOverride.Clear_Output_ReadOnly");

    encoder.popDebugGroup(); // "ConstOverride.Clear_Output_ReadOnly"

    encoder.pushDebugGroup("ConstOverride.Copy_Clear_Output");

    // Copy texture Output to texture Clear_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Clear_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Clear_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "ConstOverride.Copy_Clear_Output"

    encoder.pushDebugGroup("ConstOverride.Clear");

    // Run compute shader Clear
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Clear",
            layout: this.BindGroupLayout_Compute_Clear,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Clear_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
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

        if (this.Pipeline_Compute_Clear !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Clear);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "ConstOverride.Clear"

    encoder.pushDebugGroup("ConstOverride.Left_WriteColor_Color_ReadOnly");

    encoder.popDebugGroup(); // "ConstOverride.Left_WriteColor_Color_ReadOnly"

    encoder.pushDebugGroup("ConstOverride.Copy_Left_WriteColor_Color");

    // Copy texture Output to texture Left_WriteColor_Color_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Left_WriteColor_Color_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Left_WriteColor_Color_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "ConstOverride.Copy_Left_WriteColor_Color"

    encoder.pushDebugGroup("ConstOverride.Left_WriteColor");

    // Run compute shader Left_WriteColor
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Left_WriteColor",
            layout: this.BindGroupLayout_Compute_Left_WriteColor,
            entries: [
                {
                    // Color
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // ColorReadOnly
                    binding: 1,
                    resource: this.texture_Left_WriteColor_Color_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
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

        if (this.Pipeline_Compute_Left_WriteColor !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Left_WriteColor);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "ConstOverride.Left_WriteColor"

    encoder.pushDebugGroup("ConstOverride.Right_WriteColor_Color_ReadOnly");

    encoder.popDebugGroup(); // "ConstOverride.Right_WriteColor_Color_ReadOnly"

    encoder.pushDebugGroup("ConstOverride.Copy_Right_WriteColor_Color");

    // Copy texture Output to texture Right_WriteColor_Color_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Right_WriteColor_Color_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Right_WriteColor_Color_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "ConstOverride.Copy_Right_WriteColor_Color"

    encoder.pushDebugGroup("ConstOverride.Right_WriteColor");

    // Run compute shader Right_WriteColor
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Right_WriteColor",
            layout: this.BindGroupLayout_Compute_Right_WriteColor,
            entries: [
                {
                    // Color
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // ColorReadOnly
                    binding: 1,
                    resource: this.texture_Right_WriteColor_Color_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
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

        if (this.Pipeline_Compute_Right_WriteColor !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Right_WriteColor);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "ConstOverride.Right_WriteColor"

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

var ConstOverride = new class_ConstOverride;

export default ConstOverride;
