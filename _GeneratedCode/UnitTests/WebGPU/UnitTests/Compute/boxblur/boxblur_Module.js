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

class class_boxblur
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "BoxBlur_0", node "BlurH"
static ShaderCode_BlurH_BoxBlur_0 = `
struct Struct_BoxBlur_0CB_std140_0
{
    @align(16) radius_0 : i32,
    @align(4) sRGB_0 : u32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _BoxBlur_0CB : Struct_BoxBlur_0CB_std140_0;
@binding(0) @group(0) var Input : texture_2d<f32>;

@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_1 : vec3<f32>;
    var _S1 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.x;
    }
    else
    {
        _S1 = sRGBHi_0.x;
    }
    sRGB_1[i32(0)] = _S1;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.y;
    }
    else
    {
        _S1 = sRGBHi_0.y;
    }
    sRGB_1[i32(1)] = _S1;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.z;
    }
    else
    {
        _S1 = sRGBHi_0.z;
    }
    sRGB_1[i32(2)] = _S1;
    return sRGB_1;
}

@compute
@workgroup_size(8, 8, 1)
fn BlurH(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var radius_1 : i32 = _BoxBlur_0CB.radius_0;
    var w_0 : u32;
    var h_0 : u32;
    {var dim = textureDimensions((Input));((w_0)) = dim.x;((h_0)) = dim.y;};
    const _S2 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var i_0 : i32 = - radius_1;
    var result_0 : vec4<f32> = _S2;
    for(;;)
    {
        if(i_0 <= radius_1)
        {
        }
        else
        {
            break;
        }
        var _S3 : vec2<u32> = (DTid_0.xy + vec2<u32>(vec2<i32>(i_0, i32(0)))) % vec2<u32>(vec2<i32>(i32(w_0), i32(h_0)));
        var _S4 : vec3<i32> = vec3<i32>(vec2<i32>(_S3), i32(0));
        var result_1 : vec4<f32> = result_0 + (textureLoad((Input), ((_S4)).xy, ((_S4)).z)) / vec4<f32>((f32(radius_1) * 2.0f + 1.0f));
        i_0 = i_0 + i32(1);
        result_0 = result_1;
    }
    if(bool(_BoxBlur_0CB.sRGB_0))
    {
        textureStore((Output), (DTid_0.xy), (vec4<f32>(LinearToSRGB_0(result_0.xyz), result_0.w)));
    }
    else
    {
        textureStore((Output), (DTid_0.xy), (result_0));
    }
    return;
}

`;

// Shader code for Compute shader "BoxBlur_1", node "BlurV"
static ShaderCode_BlurV_BoxBlur_1 = `
struct Struct_BoxBlur_1CB_std140_0
{
    @align(16) radius_0 : i32,
    @align(4) sRGB_0 : u32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _BoxBlur_1CB : Struct_BoxBlur_1CB_std140_0;
@binding(0) @group(0) var Input : texture_2d<f32>;

@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_1 : vec3<f32>;
    var _S1 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.x;
    }
    else
    {
        _S1 = sRGBHi_0.x;
    }
    sRGB_1[i32(0)] = _S1;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.y;
    }
    else
    {
        _S1 = sRGBHi_0.y;
    }
    sRGB_1[i32(1)] = _S1;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.z;
    }
    else
    {
        _S1 = sRGBHi_0.z;
    }
    sRGB_1[i32(2)] = _S1;
    return sRGB_1;
}

@compute
@workgroup_size(8, 8, 1)
fn BlurV(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var radius_1 : i32 = _BoxBlur_1CB.radius_0;
    var w_0 : u32;
    var h_0 : u32;
    {var dim = textureDimensions((Input));((w_0)) = dim.x;((h_0)) = dim.y;};
    const _S2 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var i_0 : i32 = - radius_1;
    var result_0 : vec4<f32> = _S2;
    for(;;)
    {
        if(i_0 <= radius_1)
        {
        }
        else
        {
            break;
        }
        var _S3 : vec2<u32> = (DTid_0.xy + vec2<u32>(vec2<i32>(i32(0), i_0))) % vec2<u32>(vec2<i32>(i32(w_0), i32(h_0)));
        var _S4 : vec3<i32> = vec3<i32>(vec2<i32>(_S3), i32(0));
        var result_1 : vec4<f32> = result_0 + (textureLoad((Input), ((_S4)).xy, ((_S4)).z)) / vec4<f32>((f32(radius_1) * 2.0f + 1.0f));
        i_0 = i_0 + i32(1);
        result_0 = result_1;
    }
    if(bool(_BoxBlur_1CB.sRGB_0))
    {
        textureStore((Output), (DTid_0.xy), (vec4<f32>(LinearToSRGB_0(result_0.xyz), result_0.w)));
    }
    else
    {
        textureStore((Output), (DTid_0.xy), (result_0));
    }
    return;
}

`;

// -------------------- Private Members

// Texture PingPongTexture : An internal texture used during the blurring process
texture_PingPongTexture = null;
texture_PingPongTexture_size = [0, 0, 0];
texture_PingPongTexture_format = "";
texture_PingPongTexture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture BlurH_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_BlurH_Output_ReadOnly = null;
texture_BlurH_Output_ReadOnly_size = [0, 0, 0];
texture_BlurH_Output_ReadOnly_format = "";
texture_BlurH_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture BlurV_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_BlurV_Output_ReadOnly = null;
texture_BlurV_Output_ReadOnly_size = [0, 0, 0];
texture_BlurV_Output_ReadOnly_format = "";
texture_BlurV_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _BoxBlur_0CB
constantBuffer__BoxBlur_0CB = null;
constantBuffer__BoxBlur_0CB_size = 16;
constantBuffer__BoxBlur_0CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader BlurH
Hash_Compute_BlurH = 0;
ShaderModule_Compute_BlurH = null;
BindGroupLayout_Compute_BlurH = null;
PipelineLayout_Compute_BlurH = null;
Pipeline_Compute_BlurH = null;

// Constant buffer _BoxBlur_1CB
constantBuffer__BoxBlur_1CB = null;
constantBuffer__BoxBlur_1CB_size = 16;
constantBuffer__BoxBlur_1CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader BlurV
Hash_Compute_BlurV = 0;
ShaderModule_Compute_BlurV = null;
BindGroupLayout_Compute_BlurV = null;
PipelineLayout_Compute_BlurV = null;
Pipeline_Compute_BlurV = null;

// -------------------- Imported Members

// Texture InputTexture : This is the texture to be blurred
texture_InputTexture = null;
texture_InputTexture_size = [0, 0, 0];
texture_InputTexture_format = "";
texture_InputTexture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// -------------------- Public Variables

variable_enabled = true;  // Enables or disables the blurring effect.
variableDefault_enabled = true;  // Enables or disables the blurring effect.
variableChanged_enabled = false;
variable_radius = 2;  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
variableDefault_radius = 2;  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
variableChanged_radius = false;
variable_sRGB = true;
variableDefault_sRGB = true;
variableChanged_sRGB = false;
variable_iResolution = [ 0.000000, 0.000000, 0.000000 ];
variableDefault_iResolution = [ 0.000000, 0.000000, 0.000000 ];
variableChanged_iResolution = [ false, false, false ];
variable_iTime = 0.000000;
variableDefault_iTime = 0.000000;
variableChanged_iTime = false;
variable_iTimeDelta = 0.000000;
variableDefault_iTimeDelta = 0.000000;
variableChanged_iTimeDelta = false;
variable_iFrameRate = 0.000000;
variableDefault_iFrameRate = 0.000000;
variableChanged_iFrameRate = false;
variable_iFrame = 0;
variableDefault_iFrame = 0;
variableChanged_iFrame = false;
variable_iMouse = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_iMouse = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_iMouse = [ false, false, false, false ];
variable_MouseState = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_MouseState = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_MouseState = [ false, false, false, false ];
variable_MouseStateLastFrame = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_MouseStateLastFrame = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_MouseStateLastFrame = [ false, false, false, false ];

// -------------------- Structs

static StructOffsets__BoxBlur_0CB =
{
    radius: 0,
    sRGB: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__BoxBlur_1CB =
{
    radius: 0,
    sRGB: 4,
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
    // Validate texture InputTexture
    if (this.texture_InputTexture === null)
    {
        Shared.LogError("Imported resource texture_InputTexture was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture PingPongTexture
    {
        const baseSize = this.texture_InputTexture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_InputTexture_format;
        if (this.texture_PingPongTexture !== null && (this.texture_PingPongTexture_format != desiredFormat || this.texture_PingPongTexture_size[0] != desiredSize[0] || this.texture_PingPongTexture_size[1] != desiredSize[1] || this.texture_PingPongTexture_size[2] != desiredSize[2]))
        {
            this.texture_PingPongTexture.destroy();
            this.texture_PingPongTexture = null;
        }

        if (this.texture_PingPongTexture === null)
        {
            this.texture_PingPongTexture_size = desiredSize.slice();
            this.texture_PingPongTexture_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_PingPongTexture_format))
                viewFormats.push(this.texture_PingPongTexture_format);

            this.texture_PingPongTexture = device.createTexture({
                label: "texture boxblur.PingPongTexture",
                size: this.texture_PingPongTexture_size,
                format: Shared.GetNonSRGBFormat(this.texture_PingPongTexture_format),
                usage: this.texture_PingPongTexture_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture BlurH_Output_ReadOnly
    {
        const baseSize = this.texture_PingPongTexture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_PingPongTexture_format;
        if (this.texture_BlurH_Output_ReadOnly !== null && (this.texture_BlurH_Output_ReadOnly_format != desiredFormat || this.texture_BlurH_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_BlurH_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_BlurH_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_BlurH_Output_ReadOnly.destroy();
            this.texture_BlurH_Output_ReadOnly = null;
        }

        if (this.texture_BlurH_Output_ReadOnly === null)
        {
            this.texture_BlurH_Output_ReadOnly_size = desiredSize.slice();
            this.texture_BlurH_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_BlurH_Output_ReadOnly_format))
                viewFormats.push(this.texture_BlurH_Output_ReadOnly_format);

            this.texture_BlurH_Output_ReadOnly = device.createTexture({
                label: "texture boxblur.BlurH_Output_ReadOnly",
                size: this.texture_BlurH_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_BlurH_Output_ReadOnly_format),
                usage: this.texture_BlurH_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture BlurV_Output_ReadOnly
    {
        const baseSize = this.texture_InputTexture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_InputTexture_format;
        if (this.texture_BlurV_Output_ReadOnly !== null && (this.texture_BlurV_Output_ReadOnly_format != desiredFormat || this.texture_BlurV_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_BlurV_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_BlurV_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_BlurV_Output_ReadOnly.destroy();
            this.texture_BlurV_Output_ReadOnly = null;
        }

        if (this.texture_BlurV_Output_ReadOnly === null)
        {
            this.texture_BlurV_Output_ReadOnly_size = desiredSize.slice();
            this.texture_BlurV_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_BlurV_Output_ReadOnly_format))
                viewFormats.push(this.texture_BlurV_Output_ReadOnly_format);

            this.texture_BlurV_Output_ReadOnly = device.createTexture({
                label: "texture boxblur.BlurV_Output_ReadOnly",
                size: this.texture_BlurV_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_BlurV_Output_ReadOnly_format),
                usage: this.texture_BlurV_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader BlurH
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_InputTexture_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_PingPongTexture_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_BlurH_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _BoxBlur_0CB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_BlurH === null || newHash !== this.Hash_Compute_BlurH)
        {
            this.Hash_Compute_BlurH = newHash;

            let shaderCode = class_boxblur.ShaderCode_BlurH_BoxBlur_0;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_PingPongTexture_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_BlurH_Output_ReadOnly_format));

            this.ShaderModule_Compute_BlurH = device.createShaderModule({ code: shaderCode, label: "Compute Shader BlurH"});
            this.BindGroupLayout_Compute_BlurH = device.createBindGroupLayout({
                label: "Compute Bind Group Layout BlurH",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_BlurH = device.createPipelineLayout({
                label: "Compute Pipeline Layout BlurH",
                bindGroupLayouts: [this.BindGroupLayout_Compute_BlurH],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_BlurH = device.createComputePipeline({
                    label: "Compute Pipeline BlurH",
                    layout: this.PipelineLayout_Compute_BlurH,
                    compute: {
                        module: this.ShaderModule_Compute_BlurH,
                        entryPoint: "BlurH",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("BlurH");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline BlurH",
                    layout: this.PipelineLayout_Compute_BlurH,
                    compute: {
                        module: this.ShaderModule_Compute_BlurH,
                        entryPoint: "BlurH",
                    }
                }).then( handle => { this.Pipeline_Compute_BlurH = handle; this.loadingPromises.delete("BlurH"); } );
            }
        }
    }

    // (Re)create compute shader BlurV
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_PingPongTexture_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_InputTexture_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_BlurV_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _BoxBlur_1CB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_BlurV === null || newHash !== this.Hash_Compute_BlurV)
        {
            this.Hash_Compute_BlurV = newHash;

            let shaderCode = class_boxblur.ShaderCode_BlurV_BoxBlur_1;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_InputTexture_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_BlurV_Output_ReadOnly_format));

            this.ShaderModule_Compute_BlurV = device.createShaderModule({ code: shaderCode, label: "Compute Shader BlurV"});
            this.BindGroupLayout_Compute_BlurV = device.createBindGroupLayout({
                label: "Compute Bind Group Layout BlurV",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_BlurV = device.createPipelineLayout({
                label: "Compute Pipeline Layout BlurV",
                bindGroupLayouts: [this.BindGroupLayout_Compute_BlurV],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_BlurV = device.createComputePipeline({
                    label: "Compute Pipeline BlurV",
                    layout: this.PipelineLayout_Compute_BlurV,
                    compute: {
                        module: this.ShaderModule_Compute_BlurV,
                        entryPoint: "BlurV",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("BlurV");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline BlurV",
                    layout: this.PipelineLayout_Compute_BlurV,
                    compute: {
                        module: this.ShaderModule_Compute_BlurV,
                        entryPoint: "BlurV",
                    }
                }).then( handle => { this.Pipeline_Compute_BlurV = handle; this.loadingPromises.delete("BlurV"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("boxblur.InputTexture");

    encoder.popDebugGroup(); // "boxblur.InputTexture"

    encoder.pushDebugGroup("boxblur.PingPongTexture");

    encoder.popDebugGroup(); // "boxblur.PingPongTexture"

    encoder.pushDebugGroup("boxblur.BlurH_Output_ReadOnly");

    encoder.popDebugGroup(); // "boxblur.BlurH_Output_ReadOnly"

    encoder.pushDebugGroup("boxblur.Copy_BlurH_Output");

    // Copy texture PingPongTexture to texture BlurH_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_PingPongTexture.mipLevelCount, this.texture_BlurH_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_PingPongTexture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_PingPongTexture.height >> mipIndex, 1);
            let mipDepth = this.texture_PingPongTexture.depthOrArrayLayers;

            if (this.texture_PingPongTexture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_PingPongTexture, mipLevel: mipIndex },
                { texture: this.texture_BlurH_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "boxblur.Copy_BlurH_Output"

    encoder.pushDebugGroup("boxblur.BlurV_Output_ReadOnly");

    encoder.popDebugGroup(); // "boxblur.BlurV_Output_ReadOnly"

    encoder.pushDebugGroup("boxblur._BoxBlur_0CB");

    // Create constant buffer _BoxBlur_0CB
    if (this.constantBuffer__BoxBlur_0CB === null)
    {
        this.constantBuffer__BoxBlur_0CB = device.createBuffer({
            label: "boxblur._BoxBlur_0CB",
            size: Shared.Align(16, this.constructor.StructOffsets__BoxBlur_0CB._size),
            usage: this.constantBuffer__BoxBlur_0CB_usageFlags,
        });
    }

    // Upload values to constant buffer _BoxBlur_0CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__BoxBlur_0CB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__BoxBlur_0CB.radius, this.variable_radius, true);
        view.setUint32(this.constructor.StructOffsets__BoxBlur_0CB.sRGB, (this.variable_sRGB === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__BoxBlur_0CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "boxblur._BoxBlur_0CB"

    encoder.pushDebugGroup("boxblur.BlurH");

    // Run compute shader BlurH
    // Horizontal blur pass
    if (this.variable_enabled)
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group BlurH",
            layout: this.BindGroupLayout_Compute_BlurH,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_InputTexture.createView({ dimension: "2d", format: this.texture_InputTexture_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_PingPongTexture.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_BlurH_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _BoxBlur_0CB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__BoxBlur_0CB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_InputTexture_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_BlurH !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_BlurH);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "boxblur.BlurH"

    encoder.pushDebugGroup("boxblur.Copy_BlurV_Output");

    // Copy texture InputTexture to texture BlurV_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_InputTexture.mipLevelCount, this.texture_BlurV_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_InputTexture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_InputTexture.height >> mipIndex, 1);
            let mipDepth = this.texture_InputTexture.depthOrArrayLayers;

            if (this.texture_InputTexture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_InputTexture, mipLevel: mipIndex },
                { texture: this.texture_BlurV_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "boxblur.Copy_BlurV_Output"

    encoder.pushDebugGroup("boxblur._BoxBlur_1CB");

    // Create constant buffer _BoxBlur_1CB
    if (this.constantBuffer__BoxBlur_1CB === null)
    {
        this.constantBuffer__BoxBlur_1CB = device.createBuffer({
            label: "boxblur._BoxBlur_1CB",
            size: Shared.Align(16, this.constructor.StructOffsets__BoxBlur_1CB._size),
            usage: this.constantBuffer__BoxBlur_1CB_usageFlags,
        });
    }

    // Upload values to constant buffer _BoxBlur_1CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__BoxBlur_1CB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__BoxBlur_1CB.radius, this.variable_radius, true);
        view.setUint32(this.constructor.StructOffsets__BoxBlur_1CB.sRGB, (this.variable_sRGB === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__BoxBlur_1CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "boxblur._BoxBlur_1CB"

    encoder.pushDebugGroup("boxblur.BlurV");

    // Run compute shader BlurV
    // Vertical blur pass
    if (this.variable_enabled)
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group BlurV",
            layout: this.BindGroupLayout_Compute_BlurV,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_PingPongTexture.createView({ dimension: "2d", format: this.texture_PingPongTexture_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_InputTexture.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_BlurV_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _BoxBlur_1CB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__BoxBlur_1CB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_InputTexture_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_BlurV !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_BlurV);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "boxblur.BlurV"

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

var boxblur = new class_boxblur;

export default boxblur;
