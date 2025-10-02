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

class class_IndirectDispatch
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Fill_Indirect_Dispatch_Count_0", node "Fill_Indirect_Dispatch_Count"
static ShaderCode_Fill_Indirect_Dispatch_Count_Fill_Indirect_Dispatch_Count_0 = `
@binding(0) @group(0) var<storage, read_write> IndirectDispatchCount : array<vec4<u32>>;

struct Struct_Fill_Indirect_Dispatch_Count_0CB_std140_0
{
    @align(16) Dispatch_Count_1_0 : vec3<u32>,
    @align(4) _padding0_0 : f32,
    @align(16) Dispatch_Count_2_0 : vec3<u32>,
    @align(4) _padding1_0 : f32,
};

@binding(1) @group(0) var<uniform> _Fill_Indirect_Dispatch_Count_0CB : Struct_Fill_Indirect_Dispatch_Count_0CB_std140_0;
@compute
@workgroup_size(1, 1, 1)
fn FillIndirectDispatchCount(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    IndirectDispatchCount[i32(0)][i32(0)] = u32(5);
    IndirectDispatchCount[i32(0)][i32(1)] = u32(4);
    IndirectDispatchCount[i32(0)][i32(2)] = u32(3);
    IndirectDispatchCount[i32(0)][i32(3)] = u32(2);
    IndirectDispatchCount[i32(1)][i32(0)] = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1_0.x;
    IndirectDispatchCount[i32(1)][i32(1)] = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1_0.y;
    IndirectDispatchCount[i32(1)][i32(2)] = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1_0.z;
    IndirectDispatchCount[i32(1)][i32(3)] = u32(42);
    IndirectDispatchCount[i32(2)][i32(0)] = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2_0.x;
    IndirectDispatchCount[i32(2)][i32(1)] = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2_0.y;
    IndirectDispatchCount[i32(2)][i32(2)] = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2_0.z;
    IndirectDispatchCount[i32(2)][i32(3)] = u32(435);
    return;
}

`;

// Shader code for Compute shader "Do_Indirect_Dispatch_0", node "Do_Indirect_Dispatch_1"
static ShaderCode_Do_Indirect_Dispatch_1_Do_Indirect_Dispatch_0 = `
@binding(1) @group(0) var Render_TargetReadOnly : texture_storage_2d</*(Render_TargetReadOnly_format)*/, read>;

@binding(0) @group(0) var Render_Target : texture_storage_2d</*(Render_Target_format)*/, write>;

@compute
@workgroup_size(1, 1, 1)
fn DoIndirectDispatch(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S1 : vec4<f32> = (textureLoad((Render_TargetReadOnly), (vec2<i32>(px_0))));
    textureStore((Render_Target), (px_0), (vec4<f32>(_S1.xyz + vec3<f32>(0.80000001192092896f, 0.5f, 0.20000000298023224f), 1.0f)));
    return;
}

`;

// Shader code for Compute shader "Do_Indirect_Dispatch_1", node "Do_Indirect_Dispatch_2"
static ShaderCode_Do_Indirect_Dispatch_2_Do_Indirect_Dispatch_1 = `
@binding(1) @group(0) var Render_TargetReadOnly : texture_storage_2d</*(Render_TargetReadOnly_format)*/, read>;

@binding(0) @group(0) var Render_Target : texture_storage_2d</*(Render_Target_format)*/, write>;

@compute
@workgroup_size(1, 1, 1)
fn DoIndirectDispatch(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S1 : vec4<f32> = (textureLoad((Render_TargetReadOnly), (vec2<i32>(px_0))));
    textureStore((Render_Target), (px_0), (vec4<f32>(_S1.xyz + vec3<f32>(0.80000001192092896f, 0.5f, 0.20000000298023224f), 1.0f)));
    return;
}

`;

// Shader code for Compute shader "Clear_Render_Target_0", node "Clear_Render_Target"
static ShaderCode_Clear_Render_Target_Clear_Render_Target_0 = `
@binding(0) @group(0) var Render_Target : texture_storage_2d</*(Render_Target_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    textureStore((Render_Target), (DTid_0.xy), (vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f)));
    return;
}

`;

// -------------------- Private Members

// Buffer Indirect_Dispatch_Count
buffer_Indirect_Dispatch_Count = null;
buffer_Indirect_Dispatch_Count_count = 0;
buffer_Indirect_Dispatch_Count_stride = 0;
buffer_Indirect_Dispatch_Count_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.INDIRECT;

// Texture Do_Indirect_Dispatch_1_Render_Target_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly = null;
texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_size = [0, 0, 0];
texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format = "";
texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Do_Indirect_Dispatch_2_Render_Target_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly = null;
texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_size = [0, 0, 0];
texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format = "";
texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Clear_Render_Target_Render_Target_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Clear_Render_Target_Render_Target_ReadOnly = null;
texture_Clear_Render_Target_Render_Target_ReadOnly_size = [0, 0, 0];
texture_Clear_Render_Target_Render_Target_ReadOnly_format = "";
texture_Clear_Render_Target_Render_Target_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Compute Shader Clear_Render_Target
Hash_Compute_Clear_Render_Target = 0;
ShaderModule_Compute_Clear_Render_Target = null;
BindGroupLayout_Compute_Clear_Render_Target = null;
PipelineLayout_Compute_Clear_Render_Target = null;
Pipeline_Compute_Clear_Render_Target = null;

// Constant buffer _Fill_Indirect_Dispatch_Count_0CB
constantBuffer__Fill_Indirect_Dispatch_Count_0CB = null;
constantBuffer__Fill_Indirect_Dispatch_Count_0CB_size = 32;
constantBuffer__Fill_Indirect_Dispatch_Count_0CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Fill_Indirect_Dispatch_Count
Hash_Compute_Fill_Indirect_Dispatch_Count = 0;
ShaderModule_Compute_Fill_Indirect_Dispatch_Count = null;
BindGroupLayout_Compute_Fill_Indirect_Dispatch_Count = null;
PipelineLayout_Compute_Fill_Indirect_Dispatch_Count = null;
Pipeline_Compute_Fill_Indirect_Dispatch_Count = null;

// Compute Shader Do_Indirect_Dispatch_1
Hash_Compute_Do_Indirect_Dispatch_1 = 0;
ShaderModule_Compute_Do_Indirect_Dispatch_1 = null;
BindGroupLayout_Compute_Do_Indirect_Dispatch_1 = null;
PipelineLayout_Compute_Do_Indirect_Dispatch_1 = null;
Pipeline_Compute_Do_Indirect_Dispatch_1 = null;

// Compute Shader Do_Indirect_Dispatch_2
Hash_Compute_Do_Indirect_Dispatch_2 = 0;
ShaderModule_Compute_Do_Indirect_Dispatch_2 = null;
BindGroupLayout_Compute_Do_Indirect_Dispatch_2 = null;
PipelineLayout_Compute_Do_Indirect_Dispatch_2 = null;
Pipeline_Compute_Do_Indirect_Dispatch_2 = null;

// -------------------- Exported Members

// Texture Render_Target
texture_Render_Target = null;
texture_Render_Target_size = [0, 0, 0];
texture_Render_Target_format = "";
texture_Render_Target_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_Dispatch_Count_1 = [ 50, 50, 1 ];
variableDefault_Dispatch_Count_1 = [ 50, 50, 1 ];
variableChanged_Dispatch_Count_1 = [ false, false, false ];
variable_Dispatch_Count_2 = [ 100, 100, 1 ];
variableDefault_Dispatch_Count_2 = [ 100, 100, 1 ];
variableChanged_Dispatch_Count_2 = [ false, false, false ];
variable_Second_Dispatch_Offset = 2;
variableDefault_Second_Dispatch_Offset = 2;
variableChanged_Second_Dispatch_Offset = false;

// -------------------- Structs

static StructOffsets__Fill_Indirect_Dispatch_Count_0CB =
{
    Dispatch_Count_1_0: 0,
    Dispatch_Count_1_1: 4,
    Dispatch_Count_1_2: 8,
    _padding0: 12,
    Dispatch_Count_2_0: 16,
    Dispatch_Count_2_1: 20,
    Dispatch_Count_2_2: 24,
    _padding1: 28,
    _size: 32,
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
    // Handle (re)creation of texture Render_Target
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 256) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 256) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_Render_Target !== null && (this.texture_Render_Target_format != desiredFormat || this.texture_Render_Target_size[0] != desiredSize[0] || this.texture_Render_Target_size[1] != desiredSize[1] || this.texture_Render_Target_size[2] != desiredSize[2]))
        {
            this.texture_Render_Target.destroy();
            this.texture_Render_Target = null;
        }

        if (this.texture_Render_Target === null)
        {
            this.texture_Render_Target_size = desiredSize.slice();
            this.texture_Render_Target_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Render_Target_format))
                viewFormats.push(this.texture_Render_Target_format);

            this.texture_Render_Target = device.createTexture({
                label: "texture IndirectDispatch.Render_Target",
                size: this.texture_Render_Target_size,
                format: Shared.GetNonSRGBFormat(this.texture_Render_Target_format),
                usage: this.texture_Render_Target_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of buffer Indirect_Dispatch_Count
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 12) / 1) + 0;
        const desiredStride = 4;
        if (this.buffer_Indirect_Dispatch_Count !== null && (this.buffer_Indirect_Dispatch_Count_count != desiredCount || this.buffer_Indirect_Dispatch_Count_stride != desiredStride))
        {
            this.buffer_Indirect_Dispatch_Count.destroy();
            this.buffer_Indirect_Dispatch_Count = null;
        }

        if (this.buffer_Indirect_Dispatch_Count === null)
        {
            this.buffer_Indirect_Dispatch_Count_count = desiredCount;
            this.buffer_Indirect_Dispatch_Count_stride = desiredStride;
            this.buffer_Indirect_Dispatch_Count = device.createBuffer({
                label: "buffer IndirectDispatch.Indirect_Dispatch_Count",
                size: Shared.Align(16, this.buffer_Indirect_Dispatch_Count_count * this.buffer_Indirect_Dispatch_Count_stride),
                usage: this.buffer_Indirect_Dispatch_Count_usageFlags,
            });
        }
    }

    // Handle (re)creation of texture Do_Indirect_Dispatch_1_Render_Target_ReadOnly
    {
        const baseSize = this.texture_Render_Target_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Render_Target_format;
        if (this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly !== null && (this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format != desiredFormat || this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_size[0] != desiredSize[0] || this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_size[1] != desiredSize[1] || this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly.destroy();
            this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly = null;
        }

        if (this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly === null)
        {
            this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_size = desiredSize.slice();
            this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format))
                viewFormats.push(this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format);

            this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly = device.createTexture({
                label: "texture IndirectDispatch.Do_Indirect_Dispatch_1_Render_Target_ReadOnly",
                size: this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format),
                usage: this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Do_Indirect_Dispatch_2_Render_Target_ReadOnly
    {
        const baseSize = this.texture_Render_Target_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Render_Target_format;
        if (this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly !== null && (this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format != desiredFormat || this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_size[0] != desiredSize[0] || this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_size[1] != desiredSize[1] || this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly.destroy();
            this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly = null;
        }

        if (this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly === null)
        {
            this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_size = desiredSize.slice();
            this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format))
                viewFormats.push(this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format);

            this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly = device.createTexture({
                label: "texture IndirectDispatch.Do_Indirect_Dispatch_2_Render_Target_ReadOnly",
                size: this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format),
                usage: this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Clear_Render_Target_Render_Target_ReadOnly
    {
        const baseSize = this.texture_Render_Target_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Render_Target_format;
        if (this.texture_Clear_Render_Target_Render_Target_ReadOnly !== null && (this.texture_Clear_Render_Target_Render_Target_ReadOnly_format != desiredFormat || this.texture_Clear_Render_Target_Render_Target_ReadOnly_size[0] != desiredSize[0] || this.texture_Clear_Render_Target_Render_Target_ReadOnly_size[1] != desiredSize[1] || this.texture_Clear_Render_Target_Render_Target_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Clear_Render_Target_Render_Target_ReadOnly.destroy();
            this.texture_Clear_Render_Target_Render_Target_ReadOnly = null;
        }

        if (this.texture_Clear_Render_Target_Render_Target_ReadOnly === null)
        {
            this.texture_Clear_Render_Target_Render_Target_ReadOnly_size = desiredSize.slice();
            this.texture_Clear_Render_Target_Render_Target_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Clear_Render_Target_Render_Target_ReadOnly_format))
                viewFormats.push(this.texture_Clear_Render_Target_Render_Target_ReadOnly_format);

            this.texture_Clear_Render_Target_Render_Target_ReadOnly = device.createTexture({
                label: "texture IndirectDispatch.Clear_Render_Target_Render_Target_ReadOnly",
                size: this.texture_Clear_Render_Target_Render_Target_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Clear_Render_Target_Render_Target_ReadOnly_format),
                usage: this.texture_Clear_Render_Target_Render_Target_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Clear_Render_Target
    {
        const bindGroupEntries =
        [
            {
                // Render_Target
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Render_Target_format), viewDimension: "2d" }
            },
            {
                // Render_TargetReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Clear_Render_Target_Render_Target_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Clear_Render_Target === null || newHash !== this.Hash_Compute_Clear_Render_Target)
        {
            this.Hash_Compute_Clear_Render_Target = newHash;

            let shaderCode = class_IndirectDispatch.ShaderCode_Clear_Render_Target_Clear_Render_Target_0;
            shaderCode = shaderCode.replace("/*(Render_Target_format)*/", Shared.GetNonSRGBFormat(this.texture_Render_Target_format));
            shaderCode = shaderCode.replace("/*(Render_TargetReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Clear_Render_Target_Render_Target_ReadOnly_format));

            this.ShaderModule_Compute_Clear_Render_Target = device.createShaderModule({ code: shaderCode, label: "Compute Shader Clear_Render_Target"});
            this.BindGroupLayout_Compute_Clear_Render_Target = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Clear_Render_Target",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Clear_Render_Target = device.createPipelineLayout({
                label: "Compute Pipeline Layout Clear_Render_Target",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Clear_Render_Target],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Clear_Render_Target = device.createComputePipeline({
                    label: "Compute Pipeline Clear_Render_Target",
                    layout: this.PipelineLayout_Compute_Clear_Render_Target,
                    compute: {
                        module: this.ShaderModule_Compute_Clear_Render_Target,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Clear_Render_Target");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Clear_Render_Target",
                    layout: this.PipelineLayout_Compute_Clear_Render_Target,
                    compute: {
                        module: this.ShaderModule_Compute_Clear_Render_Target,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Clear_Render_Target = handle; this.loadingPromises.delete("Clear_Render_Target"); } );
            }
        }
    }

    // (Re)create compute shader Fill_Indirect_Dispatch_Count
    {
        const bindGroupEntries =
        [
            {
                // IndirectDispatchCount
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // _Fill_Indirect_Dispatch_Count_0CB
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Fill_Indirect_Dispatch_Count === null || newHash !== this.Hash_Compute_Fill_Indirect_Dispatch_Count)
        {
            this.Hash_Compute_Fill_Indirect_Dispatch_Count = newHash;

            let shaderCode = class_IndirectDispatch.ShaderCode_Fill_Indirect_Dispatch_Count_Fill_Indirect_Dispatch_Count_0;

            this.ShaderModule_Compute_Fill_Indirect_Dispatch_Count = device.createShaderModule({ code: shaderCode, label: "Compute Shader Fill_Indirect_Dispatch_Count"});
            this.BindGroupLayout_Compute_Fill_Indirect_Dispatch_Count = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Fill_Indirect_Dispatch_Count",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Fill_Indirect_Dispatch_Count = device.createPipelineLayout({
                label: "Compute Pipeline Layout Fill_Indirect_Dispatch_Count",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Fill_Indirect_Dispatch_Count],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Fill_Indirect_Dispatch_Count = device.createComputePipeline({
                    label: "Compute Pipeline Fill_Indirect_Dispatch_Count",
                    layout: this.PipelineLayout_Compute_Fill_Indirect_Dispatch_Count,
                    compute: {
                        module: this.ShaderModule_Compute_Fill_Indirect_Dispatch_Count,
                        entryPoint: "FillIndirectDispatchCount",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Fill_Indirect_Dispatch_Count");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Fill_Indirect_Dispatch_Count",
                    layout: this.PipelineLayout_Compute_Fill_Indirect_Dispatch_Count,
                    compute: {
                        module: this.ShaderModule_Compute_Fill_Indirect_Dispatch_Count,
                        entryPoint: "FillIndirectDispatchCount",
                    }
                }).then( handle => { this.Pipeline_Compute_Fill_Indirect_Dispatch_Count = handle; this.loadingPromises.delete("Fill_Indirect_Dispatch_Count"); } );
            }
        }
    }

    // (Re)create compute shader Do_Indirect_Dispatch_1
    {
        const bindGroupEntries =
        [
            {
                // Render_Target
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Render_Target_format), viewDimension: "2d" }
            },
            {
                // Render_TargetReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Do_Indirect_Dispatch_1 === null || newHash !== this.Hash_Compute_Do_Indirect_Dispatch_1)
        {
            this.Hash_Compute_Do_Indirect_Dispatch_1 = newHash;

            let shaderCode = class_IndirectDispatch.ShaderCode_Do_Indirect_Dispatch_1_Do_Indirect_Dispatch_0;
            shaderCode = shaderCode.replace("/*(Render_Target_format)*/", Shared.GetNonSRGBFormat(this.texture_Render_Target_format));
            shaderCode = shaderCode.replace("/*(Render_TargetReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly_format));

            this.ShaderModule_Compute_Do_Indirect_Dispatch_1 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Do_Indirect_Dispatch_1"});
            this.BindGroupLayout_Compute_Do_Indirect_Dispatch_1 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Do_Indirect_Dispatch_1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Do_Indirect_Dispatch_1 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Do_Indirect_Dispatch_1",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Do_Indirect_Dispatch_1],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Do_Indirect_Dispatch_1 = device.createComputePipeline({
                    label: "Compute Pipeline Do_Indirect_Dispatch_1",
                    layout: this.PipelineLayout_Compute_Do_Indirect_Dispatch_1,
                    compute: {
                        module: this.ShaderModule_Compute_Do_Indirect_Dispatch_1,
                        entryPoint: "DoIndirectDispatch",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Do_Indirect_Dispatch_1");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Do_Indirect_Dispatch_1",
                    layout: this.PipelineLayout_Compute_Do_Indirect_Dispatch_1,
                    compute: {
                        module: this.ShaderModule_Compute_Do_Indirect_Dispatch_1,
                        entryPoint: "DoIndirectDispatch",
                    }
                }).then( handle => { this.Pipeline_Compute_Do_Indirect_Dispatch_1 = handle; this.loadingPromises.delete("Do_Indirect_Dispatch_1"); } );
            }
        }
    }

    // (Re)create compute shader Do_Indirect_Dispatch_2
    {
        const bindGroupEntries =
        [
            {
                // Render_Target
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Render_Target_format), viewDimension: "2d" }
            },
            {
                // Render_TargetReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Do_Indirect_Dispatch_2 === null || newHash !== this.Hash_Compute_Do_Indirect_Dispatch_2)
        {
            this.Hash_Compute_Do_Indirect_Dispatch_2 = newHash;

            let shaderCode = class_IndirectDispatch.ShaderCode_Do_Indirect_Dispatch_2_Do_Indirect_Dispatch_1;
            shaderCode = shaderCode.replace("/*(Render_Target_format)*/", Shared.GetNonSRGBFormat(this.texture_Render_Target_format));
            shaderCode = shaderCode.replace("/*(Render_TargetReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly_format));

            this.ShaderModule_Compute_Do_Indirect_Dispatch_2 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Do_Indirect_Dispatch_2"});
            this.BindGroupLayout_Compute_Do_Indirect_Dispatch_2 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Do_Indirect_Dispatch_2",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Do_Indirect_Dispatch_2 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Do_Indirect_Dispatch_2",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Do_Indirect_Dispatch_2],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Do_Indirect_Dispatch_2 = device.createComputePipeline({
                    label: "Compute Pipeline Do_Indirect_Dispatch_2",
                    layout: this.PipelineLayout_Compute_Do_Indirect_Dispatch_2,
                    compute: {
                        module: this.ShaderModule_Compute_Do_Indirect_Dispatch_2,
                        entryPoint: "DoIndirectDispatch",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Do_Indirect_Dispatch_2");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Do_Indirect_Dispatch_2",
                    layout: this.PipelineLayout_Compute_Do_Indirect_Dispatch_2,
                    compute: {
                        module: this.ShaderModule_Compute_Do_Indirect_Dispatch_2,
                        entryPoint: "DoIndirectDispatch",
                    }
                }).then( handle => { this.Pipeline_Compute_Do_Indirect_Dispatch_2 = handle; this.loadingPromises.delete("Do_Indirect_Dispatch_2"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("IndirectDispatch.Render_Target");

    encoder.popDebugGroup(); // "IndirectDispatch.Render_Target"

    encoder.pushDebugGroup("IndirectDispatch.Indirect_Dispatch_Count");

    encoder.popDebugGroup(); // "IndirectDispatch.Indirect_Dispatch_Count"

    encoder.pushDebugGroup("IndirectDispatch.Do_Indirect_Dispatch_1_Render_Target_ReadOnly");

    encoder.popDebugGroup(); // "IndirectDispatch.Do_Indirect_Dispatch_1_Render_Target_ReadOnly"

    encoder.pushDebugGroup("IndirectDispatch.Do_Indirect_Dispatch_2_Render_Target_ReadOnly");

    encoder.popDebugGroup(); // "IndirectDispatch.Do_Indirect_Dispatch_2_Render_Target_ReadOnly"

    encoder.pushDebugGroup("IndirectDispatch.Clear_Render_Target_Render_Target_ReadOnly");

    encoder.popDebugGroup(); // "IndirectDispatch.Clear_Render_Target_Render_Target_ReadOnly"

    encoder.pushDebugGroup("IndirectDispatch.Copy_Clear_Render_Target_Render_Target");

    // Copy texture Render_Target to texture Clear_Render_Target_Render_Target_ReadOnly
    {
        const numMips = Math.min(this.texture_Render_Target.mipLevelCount, this.texture_Clear_Render_Target_Render_Target_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Render_Target.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Render_Target.height >> mipIndex, 1);
            let mipDepth = this.texture_Render_Target.depthOrArrayLayers;

            if (this.texture_Render_Target.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Render_Target, mipLevel: mipIndex },
                { texture: this.texture_Clear_Render_Target_Render_Target_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "IndirectDispatch.Copy_Clear_Render_Target_Render_Target"

    encoder.pushDebugGroup("IndirectDispatch.Clear_Render_Target");

    // Run compute shader Clear_Render_Target
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Clear_Render_Target",
            layout: this.BindGroupLayout_Compute_Clear_Render_Target,
            entries: [
                {
                    // Render_Target
                    binding: 0,
                    resource: this.texture_Render_Target.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // Render_TargetReadOnly
                    binding: 1,
                    resource: this.texture_Clear_Render_Target_Render_Target_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Render_Target_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Clear_Render_Target !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Clear_Render_Target);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "IndirectDispatch.Clear_Render_Target"

    encoder.pushDebugGroup("IndirectDispatch.Copy_Do_Indirect_Dispatch_1_Render_Target");

    // Copy texture Render_Target to texture Do_Indirect_Dispatch_1_Render_Target_ReadOnly
    {
        const numMips = Math.min(this.texture_Render_Target.mipLevelCount, this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Render_Target.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Render_Target.height >> mipIndex, 1);
            let mipDepth = this.texture_Render_Target.depthOrArrayLayers;

            if (this.texture_Render_Target.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Render_Target, mipLevel: mipIndex },
                { texture: this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "IndirectDispatch.Copy_Do_Indirect_Dispatch_1_Render_Target"

    encoder.pushDebugGroup("IndirectDispatch._Fill_Indirect_Dispatch_Count_0CB");

    // Create constant buffer _Fill_Indirect_Dispatch_Count_0CB
    if (this.constantBuffer__Fill_Indirect_Dispatch_Count_0CB === null)
    {
        this.constantBuffer__Fill_Indirect_Dispatch_Count_0CB = device.createBuffer({
            label: "IndirectDispatch._Fill_Indirect_Dispatch_Count_0CB",
            size: Shared.Align(16, this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB._size),
            usage: this.constantBuffer__Fill_Indirect_Dispatch_Count_0CB_usageFlags,
        });
    }

    // Upload values to constant buffer _Fill_Indirect_Dispatch_Count_0CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1_0, this.variable_Dispatch_Count_1[0], true);
        view.setUint32(this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1_1, this.variable_Dispatch_Count_1[1], true);
        view.setUint32(this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1_2, this.variable_Dispatch_Count_1[2], true);
        view.setUint32(this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2_0, this.variable_Dispatch_Count_2[0], true);
        view.setUint32(this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2_1, this.variable_Dispatch_Count_2[1], true);
        view.setUint32(this.constructor.StructOffsets__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2_2, this.variable_Dispatch_Count_2[2], true);
        device.queue.writeBuffer(this.constantBuffer__Fill_Indirect_Dispatch_Count_0CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "IndirectDispatch._Fill_Indirect_Dispatch_Count_0CB"

    encoder.pushDebugGroup("IndirectDispatch.Fill_Indirect_Dispatch_Count");

    // Run compute shader Fill_Indirect_Dispatch_Count
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Fill_Indirect_Dispatch_Count",
            layout: this.BindGroupLayout_Compute_Fill_Indirect_Dispatch_Count,
            entries: [
                {
                    // IndirectDispatchCount
                    binding: 0,
                    resource: { buffer: this.buffer_Indirect_Dispatch_Count }
                },
                {
                    // _Fill_Indirect_Dispatch_Count_0CB
                    binding: 1,
                    resource: { buffer: this.constantBuffer__Fill_Indirect_Dispatch_Count_0CB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = [1, 1, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Fill_Indirect_Dispatch_Count !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Fill_Indirect_Dispatch_Count);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "IndirectDispatch.Fill_Indirect_Dispatch_Count"

    encoder.pushDebugGroup("IndirectDispatch.Do_Indirect_Dispatch_1");

    // Run compute shader Do_Indirect_Dispatch_1
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Do_Indirect_Dispatch_1",
            layout: this.BindGroupLayout_Compute_Do_Indirect_Dispatch_1,
            entries: [
                {
                    // Render_Target
                    binding: 0,
                    resource: this.texture_Render_Target.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // Render_TargetReadOnly
                    binding: 1,
                    resource: this.texture_Do_Indirect_Dispatch_1_Render_Target_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        if (this.Pipeline_Compute_Do_Indirect_Dispatch_1 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Do_Indirect_Dispatch_1);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroupsIndirect(this.buffer_Indirect_Dispatch_Count, 1 * 4 * 4); // byte offset.  *4 because sizeof(UINT) is 4.  *4 again because of 4 items per dispatch.
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "IndirectDispatch.Do_Indirect_Dispatch_1"

    encoder.pushDebugGroup("IndirectDispatch.Copy_Do_Indirect_Dispatch_2_Render_Target");

    // Copy texture Render_Target to texture Do_Indirect_Dispatch_2_Render_Target_ReadOnly
    {
        const numMips = Math.min(this.texture_Render_Target.mipLevelCount, this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Render_Target.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Render_Target.height >> mipIndex, 1);
            let mipDepth = this.texture_Render_Target.depthOrArrayLayers;

            if (this.texture_Render_Target.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Render_Target, mipLevel: mipIndex },
                { texture: this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "IndirectDispatch.Copy_Do_Indirect_Dispatch_2_Render_Target"

    encoder.pushDebugGroup("IndirectDispatch.Do_Indirect_Dispatch_2");

    // Run compute shader Do_Indirect_Dispatch_2
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Do_Indirect_Dispatch_2",
            layout: this.BindGroupLayout_Compute_Do_Indirect_Dispatch_2,
            entries: [
                {
                    // Render_Target
                    binding: 0,
                    resource: this.texture_Render_Target.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // Render_TargetReadOnly
                    binding: 1,
                    resource: this.texture_Do_Indirect_Dispatch_2_Render_Target_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        if (this.Pipeline_Compute_Do_Indirect_Dispatch_2 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Do_Indirect_Dispatch_2);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroupsIndirect(this.buffer_Indirect_Dispatch_Count, this.variable_Second_Dispatch_Offset * 4 * 4); // byte offset.  *4 because sizeof(UINT) is 4.  *4 again because of 4 items per dispatch.
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "IndirectDispatch.Do_Indirect_Dispatch_2"

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

var IndirectDispatch = new class_IndirectDispatch;

export default IndirectDispatch;
