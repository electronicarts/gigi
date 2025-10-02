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

class class_VariableAliases
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "SetChannel_0", node "Set_Red"
static ShaderCode_Set_Red_SetChannel_0 = `
@binding(1) @group(0) var OutputReadOnly : texture_storage_2d</*(OutputReadOnly_format)*/, read>;

struct Struct_SetChannel_0CB_std140_0
{
    @align(16) _alias_Channel_0 : i32,
    @align(4) _alias_Value_0 : f32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(2) @group(0) var<uniform> _SetChannel_0CB : Struct_SetChannel_0CB_std140_0;
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

fn _GetVariableAliasValue_Channel_0() -> i32
{
    return _SetChannel_0CB._alias_Channel_0;
}

fn _GetVariableAliasValue_Value_0() -> f32
{
    return _SetChannel_0CB._alias_Value_0;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S1 : vec4<f32> = (textureLoad((OutputReadOnly), (vec2<i32>(px_0))));
    var pixelValue_0 : vec3<f32> = _S1.xyz;
    pixelValue_0[_GetVariableAliasValue_Channel_0()] = _GetVariableAliasValue_Value_0();
    textureStore((Output), (px_0), (vec4<f32>(pixelValue_0, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "SetChannel_1", node "Set_Green"
static ShaderCode_Set_Green_SetChannel_1 = `
@binding(1) @group(0) var OutputReadOnly : texture_storage_2d</*(OutputReadOnly_format)*/, read>;

struct Struct_SetChannel_1CB_std140_0
{
    @align(16) _alias_Channel_0 : i32,
    @align(4) _alias_Value_0 : f32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(2) @group(0) var<uniform> _SetChannel_1CB : Struct_SetChannel_1CB_std140_0;
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

fn _GetVariableAliasValue_Channel_0() -> i32
{
    return _SetChannel_1CB._alias_Channel_0;
}

fn _GetVariableAliasValue_Value_0() -> f32
{
    return _SetChannel_1CB._alias_Value_0;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S1 : vec4<f32> = (textureLoad((OutputReadOnly), (vec2<i32>(px_0))));
    var pixelValue_0 : vec3<f32> = _S1.xyz;
    pixelValue_0[_GetVariableAliasValue_Channel_0()] = _GetVariableAliasValue_Value_0();
    textureStore((Output), (px_0), (vec4<f32>(pixelValue_0, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "SetChannel_2", node "Set_Blue"
static ShaderCode_Set_Blue_SetChannel_2 = `
@binding(1) @group(0) var OutputReadOnly : texture_storage_2d</*(OutputReadOnly_format)*/, read>;

struct Struct_SetChannel_2CB_std140_0
{
    @align(16) _alias_Channel_0 : i32,
    @align(4) _alias_Value_0 : f32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(2) @group(0) var<uniform> _SetChannel_2CB : Struct_SetChannel_2CB_std140_0;
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

fn _GetVariableAliasValue_Channel_0() -> i32
{
    return _SetChannel_2CB._alias_Channel_0;
}

fn _GetVariableAliasValue_Value_0() -> f32
{
    return 0.75f;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S1 : vec4<f32> = (textureLoad((OutputReadOnly), (vec2<i32>(px_0))));
    var pixelValue_0 : vec3<f32> = _S1.xyz;
    pixelValue_0[_GetVariableAliasValue_Channel_0()] = _GetVariableAliasValue_Value_0();
    textureStore((Output), (px_0), (vec4<f32>(pixelValue_0, 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture Set_Red_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Set_Red_Output_ReadOnly = null;
texture_Set_Red_Output_ReadOnly_size = [0, 0, 0];
texture_Set_Red_Output_ReadOnly_format = "";
texture_Set_Red_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Set_Green_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Set_Green_Output_ReadOnly = null;
texture_Set_Green_Output_ReadOnly_size = [0, 0, 0];
texture_Set_Green_Output_ReadOnly_format = "";
texture_Set_Green_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Set_Blue_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Set_Blue_Output_ReadOnly = null;
texture_Set_Blue_Output_ReadOnly_size = [0, 0, 0];
texture_Set_Blue_Output_ReadOnly_format = "";
texture_Set_Blue_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _SetChannel_0CB_0
constantBuffer__SetChannel_0CB_0 = null;
constantBuffer__SetChannel_0CB_0_size = 16;
constantBuffer__SetChannel_0CB_0_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Set_Red
Hash_Compute_Set_Red = 0;
ShaderModule_Compute_Set_Red = null;
BindGroupLayout_Compute_Set_Red = null;
PipelineLayout_Compute_Set_Red = null;
Pipeline_Compute_Set_Red = null;

// Constant buffer _SetChannel_1CB_0
constantBuffer__SetChannel_1CB_0 = null;
constantBuffer__SetChannel_1CB_0_size = 16;
constantBuffer__SetChannel_1CB_0_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Set_Green
Hash_Compute_Set_Green = 0;
ShaderModule_Compute_Set_Green = null;
BindGroupLayout_Compute_Set_Green = null;
PipelineLayout_Compute_Set_Green = null;
Pipeline_Compute_Set_Green = null;

// Constant buffer _SetChannel_2CB_0
constantBuffer__SetChannel_2CB_0 = null;
constantBuffer__SetChannel_2CB_0_size = 16;
constantBuffer__SetChannel_2CB_0_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Set_Blue
Hash_Compute_Set_Blue = 0;
ShaderModule_Compute_Set_Blue = null;
BindGroupLayout_Compute_Set_Blue = null;
PipelineLayout_Compute_Set_Blue = null;
Pipeline_Compute_Set_Blue = null;

// -------------------- Exported Members

// Texture Color
texture_Color = null;
texture_Color_size = [0, 0, 0];
texture_Color_format = "";
texture_Color_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_RenderSize = [ 32, 32 ];
variableDefault_RenderSize = [ 32, 32 ];
variableChanged_RenderSize = [ false, false ];
variable_Node1Channel = 0;
variableDefault_Node1Channel = 0;
variableChanged_Node1Channel = false;
variable_Node1Value = 0.25;
variableDefault_Node1Value = 0.25;
variableChanged_Node1Value = false;
variable_Node2Channel = 1;
variableDefault_Node2Channel = 1;
variableChanged_Node2Channel = false;
variable_Node2Value = 0.5;
variableDefault_Node2Value = 0.5;
variableChanged_Node2Value = false;
variable_Node3Channel = 2;
variableDefault_Node3Channel = 2;
variableChanged_Node3Channel = false;
variable_Node3Value = 0.75;
variableDefault_Node3Value = 0.75;
variableChanged_Node3Value = false;
variable_UnusedFloat = 0.;  // This is for the unused alias in the shader
variableDefault_UnusedFloat = 0.;  // This is for the unused alias in the shader
variableChanged_UnusedFloat = false;

// -------------------- Structs

static StructOffsets__SetChannel_0CB =
{
    _alias_Channel: 0,
    _alias_Value: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__SetChannel_1CB =
{
    _alias_Channel: 0,
    _alias_Value: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__SetChannel_2CB =
{
    _alias_Channel: 0,
    _alias_Value: 4,
    _padding0: 8,
    _padding1: 12,
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
    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Color
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm";
        if (this.texture_Color !== null && (this.texture_Color_format != desiredFormat || this.texture_Color_size[0] != desiredSize[0] || this.texture_Color_size[1] != desiredSize[1] || this.texture_Color_size[2] != desiredSize[2]))
        {
            this.texture_Color.destroy();
            this.texture_Color = null;
        }

        if (this.texture_Color === null)
        {
            this.texture_Color_size = desiredSize.slice();
            this.texture_Color_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Color_format))
                viewFormats.push(this.texture_Color_format);

            this.texture_Color = device.createTexture({
                label: "texture VariableAliases.Color",
                size: this.texture_Color_size,
                format: Shared.GetNonSRGBFormat(this.texture_Color_format),
                usage: this.texture_Color_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Set_Red_Output_ReadOnly
    {
        const baseSize = this.texture_Color_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Color_format;
        if (this.texture_Set_Red_Output_ReadOnly !== null && (this.texture_Set_Red_Output_ReadOnly_format != desiredFormat || this.texture_Set_Red_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Set_Red_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Set_Red_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Set_Red_Output_ReadOnly.destroy();
            this.texture_Set_Red_Output_ReadOnly = null;
        }

        if (this.texture_Set_Red_Output_ReadOnly === null)
        {
            this.texture_Set_Red_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Set_Red_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Set_Red_Output_ReadOnly_format))
                viewFormats.push(this.texture_Set_Red_Output_ReadOnly_format);

            this.texture_Set_Red_Output_ReadOnly = device.createTexture({
                label: "texture VariableAliases.Set_Red_Output_ReadOnly",
                size: this.texture_Set_Red_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Set_Red_Output_ReadOnly_format),
                usage: this.texture_Set_Red_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Set_Green_Output_ReadOnly
    {
        const baseSize = this.texture_Color_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Color_format;
        if (this.texture_Set_Green_Output_ReadOnly !== null && (this.texture_Set_Green_Output_ReadOnly_format != desiredFormat || this.texture_Set_Green_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Set_Green_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Set_Green_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Set_Green_Output_ReadOnly.destroy();
            this.texture_Set_Green_Output_ReadOnly = null;
        }

        if (this.texture_Set_Green_Output_ReadOnly === null)
        {
            this.texture_Set_Green_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Set_Green_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Set_Green_Output_ReadOnly_format))
                viewFormats.push(this.texture_Set_Green_Output_ReadOnly_format);

            this.texture_Set_Green_Output_ReadOnly = device.createTexture({
                label: "texture VariableAliases.Set_Green_Output_ReadOnly",
                size: this.texture_Set_Green_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Set_Green_Output_ReadOnly_format),
                usage: this.texture_Set_Green_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Set_Blue_Output_ReadOnly
    {
        const baseSize = this.texture_Color_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Color_format;
        if (this.texture_Set_Blue_Output_ReadOnly !== null && (this.texture_Set_Blue_Output_ReadOnly_format != desiredFormat || this.texture_Set_Blue_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Set_Blue_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Set_Blue_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Set_Blue_Output_ReadOnly.destroy();
            this.texture_Set_Blue_Output_ReadOnly = null;
        }

        if (this.texture_Set_Blue_Output_ReadOnly === null)
        {
            this.texture_Set_Blue_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Set_Blue_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Set_Blue_Output_ReadOnly_format))
                viewFormats.push(this.texture_Set_Blue_Output_ReadOnly_format);

            this.texture_Set_Blue_Output_ReadOnly = device.createTexture({
                label: "texture VariableAliases.Set_Blue_Output_ReadOnly",
                size: this.texture_Set_Blue_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Set_Blue_Output_ReadOnly_format),
                usage: this.texture_Set_Blue_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Set_Red
    {
        const bindGroupEntries =
        [
            {
                // Output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Color_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Set_Red_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _SetChannel_0CB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Set_Red === null || newHash !== this.Hash_Compute_Set_Red)
        {
            this.Hash_Compute_Set_Red = newHash;

            let shaderCode = class_VariableAliases.ShaderCode_Set_Red_SetChannel_0;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Color_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Set_Red_Output_ReadOnly_format));

            this.ShaderModule_Compute_Set_Red = device.createShaderModule({ code: shaderCode, label: "Compute Shader Set_Red"});
            this.BindGroupLayout_Compute_Set_Red = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Set_Red",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Set_Red = device.createPipelineLayout({
                label: "Compute Pipeline Layout Set_Red",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Set_Red],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Set_Red = device.createComputePipeline({
                    label: "Compute Pipeline Set_Red",
                    layout: this.PipelineLayout_Compute_Set_Red,
                    compute: {
                        module: this.ShaderModule_Compute_Set_Red,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Set_Red");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Set_Red",
                    layout: this.PipelineLayout_Compute_Set_Red,
                    compute: {
                        module: this.ShaderModule_Compute_Set_Red,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Set_Red = handle; this.loadingPromises.delete("Set_Red"); } );
            }
        }
    }

    // (Re)create compute shader Set_Green
    {
        const bindGroupEntries =
        [
            {
                // Output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Color_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Set_Green_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _SetChannel_1CB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Set_Green === null || newHash !== this.Hash_Compute_Set_Green)
        {
            this.Hash_Compute_Set_Green = newHash;

            let shaderCode = class_VariableAliases.ShaderCode_Set_Green_SetChannel_1;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Color_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Set_Green_Output_ReadOnly_format));

            this.ShaderModule_Compute_Set_Green = device.createShaderModule({ code: shaderCode, label: "Compute Shader Set_Green"});
            this.BindGroupLayout_Compute_Set_Green = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Set_Green",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Set_Green = device.createPipelineLayout({
                label: "Compute Pipeline Layout Set_Green",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Set_Green],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Set_Green = device.createComputePipeline({
                    label: "Compute Pipeline Set_Green",
                    layout: this.PipelineLayout_Compute_Set_Green,
                    compute: {
                        module: this.ShaderModule_Compute_Set_Green,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Set_Green");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Set_Green",
                    layout: this.PipelineLayout_Compute_Set_Green,
                    compute: {
                        module: this.ShaderModule_Compute_Set_Green,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Set_Green = handle; this.loadingPromises.delete("Set_Green"); } );
            }
        }
    }

    // (Re)create compute shader Set_Blue
    {
        const bindGroupEntries =
        [
            {
                // Output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Color_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Set_Blue_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _SetChannel_2CB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Set_Blue === null || newHash !== this.Hash_Compute_Set_Blue)
        {
            this.Hash_Compute_Set_Blue = newHash;

            let shaderCode = class_VariableAliases.ShaderCode_Set_Blue_SetChannel_2;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Color_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Set_Blue_Output_ReadOnly_format));

            this.ShaderModule_Compute_Set_Blue = device.createShaderModule({ code: shaderCode, label: "Compute Shader Set_Blue"});
            this.BindGroupLayout_Compute_Set_Blue = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Set_Blue",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Set_Blue = device.createPipelineLayout({
                label: "Compute Pipeline Layout Set_Blue",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Set_Blue],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Set_Blue = device.createComputePipeline({
                    label: "Compute Pipeline Set_Blue",
                    layout: this.PipelineLayout_Compute_Set_Blue,
                    compute: {
                        module: this.ShaderModule_Compute_Set_Blue,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Set_Blue");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Set_Blue",
                    layout: this.PipelineLayout_Compute_Set_Blue,
                    compute: {
                        module: this.ShaderModule_Compute_Set_Blue,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Set_Blue = handle; this.loadingPromises.delete("Set_Blue"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("VariableAliases.Color");

    encoder.popDebugGroup(); // "VariableAliases.Color"

    encoder.pushDebugGroup("VariableAliases.Set_Red_Output_ReadOnly");

    encoder.popDebugGroup(); // "VariableAliases.Set_Red_Output_ReadOnly"

    encoder.pushDebugGroup("VariableAliases.Copy_Set_Red_Output");

    // Copy texture Color to texture Set_Red_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Color.mipLevelCount, this.texture_Set_Red_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Color.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Color.height >> mipIndex, 1);
            let mipDepth = this.texture_Color.depthOrArrayLayers;

            if (this.texture_Color.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Color, mipLevel: mipIndex },
                { texture: this.texture_Set_Red_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "VariableAliases.Copy_Set_Red_Output"

    encoder.pushDebugGroup("VariableAliases.Set_Green_Output_ReadOnly");

    encoder.popDebugGroup(); // "VariableAliases.Set_Green_Output_ReadOnly"

    encoder.pushDebugGroup("VariableAliases.Set_Blue_Output_ReadOnly");

    encoder.popDebugGroup(); // "VariableAliases.Set_Blue_Output_ReadOnly"

    encoder.pushDebugGroup("VariableAliases._SetChannel_0CB_0");

    // Create constant buffer _SetChannel_0CB_0
    if (this.constantBuffer__SetChannel_0CB_0 === null)
    {
        this.constantBuffer__SetChannel_0CB_0 = device.createBuffer({
            label: "VariableAliases._SetChannel_0CB_0",
            size: Shared.Align(16, this.constructor.StructOffsets__SetChannel_0CB._size),
            usage: this.constantBuffer__SetChannel_0CB_0_usageFlags,
        });
    }

    // Upload values to constant buffer _SetChannel_0CB_0
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__SetChannel_0CB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__SetChannel_0CB._alias_Channel, this.variable_Node1Channel, true);
        view.setFloat32(this.constructor.StructOffsets__SetChannel_0CB._alias_Value, this.variable_Node1Value, true);
        device.queue.writeBuffer(this.constantBuffer__SetChannel_0CB_0, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "VariableAliases._SetChannel_0CB_0"

    encoder.pushDebugGroup("VariableAliases.Set_Red");

    // Run compute shader Set_Red
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Set_Red",
            layout: this.BindGroupLayout_Compute_Set_Red,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Color.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Set_Red_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _SetChannel_0CB
                    binding: 2,
                    resource: { buffer: this.constantBuffer__SetChannel_0CB_0 }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Color_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Set_Red !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Set_Red);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "VariableAliases.Set_Red"

    encoder.pushDebugGroup("VariableAliases.Copy_Set_Green_Output");

    // Copy texture Color to texture Set_Green_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Color.mipLevelCount, this.texture_Set_Green_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Color.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Color.height >> mipIndex, 1);
            let mipDepth = this.texture_Color.depthOrArrayLayers;

            if (this.texture_Color.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Color, mipLevel: mipIndex },
                { texture: this.texture_Set_Green_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "VariableAliases.Copy_Set_Green_Output"

    encoder.pushDebugGroup("VariableAliases._SetChannel_1CB_0");

    // Create constant buffer _SetChannel_1CB_0
    if (this.constantBuffer__SetChannel_1CB_0 === null)
    {
        this.constantBuffer__SetChannel_1CB_0 = device.createBuffer({
            label: "VariableAliases._SetChannel_1CB_0",
            size: Shared.Align(16, this.constructor.StructOffsets__SetChannel_1CB._size),
            usage: this.constantBuffer__SetChannel_1CB_0_usageFlags,
        });
    }

    // Upload values to constant buffer _SetChannel_1CB_0
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__SetChannel_1CB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__SetChannel_1CB._alias_Channel, this.variable_Node2Channel, true);
        view.setFloat32(this.constructor.StructOffsets__SetChannel_1CB._alias_Value, this.variable_Node2Value, true);
        device.queue.writeBuffer(this.constantBuffer__SetChannel_1CB_0, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "VariableAliases._SetChannel_1CB_0"

    encoder.pushDebugGroup("VariableAliases.Set_Green");

    // Run compute shader Set_Green
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Set_Green",
            layout: this.BindGroupLayout_Compute_Set_Green,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Color.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Set_Green_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _SetChannel_1CB
                    binding: 2,
                    resource: { buffer: this.constantBuffer__SetChannel_1CB_0 }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Color_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Set_Green !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Set_Green);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "VariableAliases.Set_Green"

    encoder.pushDebugGroup("VariableAliases.Copy_Set_Blue_Output");

    // Copy texture Color to texture Set_Blue_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Color.mipLevelCount, this.texture_Set_Blue_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Color.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Color.height >> mipIndex, 1);
            let mipDepth = this.texture_Color.depthOrArrayLayers;

            if (this.texture_Color.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Color, mipLevel: mipIndex },
                { texture: this.texture_Set_Blue_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "VariableAliases.Copy_Set_Blue_Output"

    encoder.pushDebugGroup("VariableAliases._SetChannel_2CB_0");

    // Create constant buffer _SetChannel_2CB_0
    if (this.constantBuffer__SetChannel_2CB_0 === null)
    {
        this.constantBuffer__SetChannel_2CB_0 = device.createBuffer({
            label: "VariableAliases._SetChannel_2CB_0",
            size: Shared.Align(16, this.constructor.StructOffsets__SetChannel_2CB._size),
            usage: this.constantBuffer__SetChannel_2CB_0_usageFlags,
        });
    }

    // Upload values to constant buffer _SetChannel_2CB_0
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__SetChannel_2CB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__SetChannel_2CB._alias_Channel, this.variable_Node3Channel, true);
        view.setFloat32(this.constructor.StructOffsets__SetChannel_2CB._alias_Value, this.variable_Node3Value, true);
        device.queue.writeBuffer(this.constantBuffer__SetChannel_2CB_0, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "VariableAliases._SetChannel_2CB_0"

    encoder.pushDebugGroup("VariableAliases.Set_Blue");

    // Run compute shader Set_Blue
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Set_Blue",
            layout: this.BindGroupLayout_Compute_Set_Blue,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Color.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Set_Blue_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _SetChannel_2CB
                    binding: 2,
                    resource: { buffer: this.constantBuffer__SetChannel_2CB_0 }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Color_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Set_Blue !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Set_Blue);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "VariableAliases.Set_Blue"

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

var VariableAliases = new class_VariableAliases;

export default VariableAliases;
