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

class class_Stencil
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Vertex shader "Draw1VS", node "Draw_1"
static ShaderCode_Draw_1_Draw1VS = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_Draw1VSCB_std140_0
{
    @align(16) ViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
};

@binding(0) @group(0) var<uniform> _Draw1VSCB : Struct_Draw1VSCB_std140_0;
struct VSOutput_0
{
    @builtin(position) position_0 : vec4<f32>,
    @location(0) normal_0 : vec3<f32>,
    @location(1) uv_0 : vec2<f32>,
};

struct vertexInput_0
{
    @location(0) position_1 : vec3<f32>,
    @location(1) normal_1 : vec3<f32>,
    @location(2) uv_1 : vec2<f32>,
};

@vertex
fn Draw1VS( _S1 : vertexInput_0) -> VSOutput_0
{
    const _S2 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const _S3 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
    var ret_0 : VSOutput_0;
    ret_0.position_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.normal_0 = _S2;
    ret_0.uv_0 = _S3;
    ret_0.position_0 = (((mat4x4<f32>(_Draw1VSCB.ViewProjMtx_0.data_0[i32(0)][i32(0)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(0)][i32(1)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(0)][i32(2)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(0)][i32(3)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(1)][i32(0)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(1)][i32(1)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(1)][i32(2)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(1)][i32(3)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(2)][i32(0)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(2)][i32(1)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(2)][i32(2)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(2)][i32(3)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(3)][i32(0)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(3)][i32(1)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(3)][i32(2)], _Draw1VSCB.ViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    ret_0.uv_0 = _S1.uv_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "Draw1PS", node "Draw_1"
static ShaderCode_Draw_1_Draw1PS = `
struct PSOutput_0
{
    @location(0) colorTarget_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) normal_0 : vec3<f32>,
    @location(1) uv_0 : vec2<f32>,
};

@fragment
fn Draw1PS( _S1 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> PSOutput_0
{
    var ret_0 : PSOutput_0;
    ret_0.colorTarget_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var _S2 : vec3<f32> = vec3<f32>(0.5f);
    ret_0.colorTarget_0 = vec4<f32>(_S1.normal_0 * _S2 + _S2, 1.0f);
    return ret_0;
}

`;

// Shader code for Vertex shader "Draw2VS", node "Draw_2"
static ShaderCode_Draw_2_Draw2VS = `
struct VSOutput_0
{
    @builtin(position) position_0 : vec4<f32>,
};

@vertex
fn Draw2VS(@builtin(vertex_index) id_0 : u32) -> VSOutput_0
{
    var ret_0 : VSOutput_0;
    ret_0.position_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.position_0 = vec4<f32>(vec2<f32>(f32((((id_0 << (u32(1)))) & (u32(2)))), f32((id_0 & (u32(2))))) * vec2<f32>(2.0f, -2.0f) + vec2<f32>(-1.0f, 1.0f), 0.0f, 1.0f);
    return ret_0;
}

`;

// Shader code for Pixel shader "Draw2PS", node "Draw_2"
static ShaderCode_Draw_2_Draw2PS = `
@binding(100) @group(0) var _loadedTexture_0 : texture_2d<f32>;

struct PSOutput_0
{
    @location(0) colorTarget_0 : vec4<f32>,
};

@fragment
fn Draw2PS(@builtin(position) position_0 : vec4<f32>) -> PSOutput_0
{
    var ret_0 : PSOutput_0;
    ret_0.colorTarget_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var _S1 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(position_0.xy), u32(0)));
    ret_0.colorTarget_0 = vec4<f32>((textureLoad((_loadedTexture_0), ((_S1)).xy, ((_S1)).z)).xyz, 1.0f);
    return ret_0;
}

`;

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct VertexBuffer
static StructVertexBufferAttributes_VertexBuffer =
[
    // pos
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // normal
    {
        format: "float32x3",
        offset: 16,
        shaderLocation: 1,
    },
    // uv
    {
        format: "float32x2",
        offset: 32,
        shaderLocation: 2,
    },
];

// Vertex buffer attributes for struct VertexBuffer_Unpadded
static StructVertexBufferAttributes_VertexBuffer_Unpadded =
[
    // pos
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // normal
    {
        format: "float32x3",
        offset: 12,
        shaderLocation: 1,
    },
    // uv
    {
        format: "float32x2",
        offset: 24,
        shaderLocation: 2,
    },
];

// -------------------- Private Members

// Constant buffer _Draw1VSCB
constantBuffer__Draw1VSCB = null;
constantBuffer__Draw1VSCB_size = 64;
constantBuffer__Draw1VSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Draw call Shader Draw_1
Hash_DrawCall_Draw_1 = 0;
ShaderModule_DrawCallVS_Draw_1 = null;
ShaderModule_DrawCallPS_Draw_1 = null;
BindGroupLayout_DrawCall_Draw_1 = null;
PipelineLayout_DrawCall_Draw_1 = null;
Pipeline_DrawCall_Draw_1 = null;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Draw call Shader Draw_2
Hash_DrawCall_Draw_2 = 0;
ShaderModule_DrawCallVS_Draw_2 = null;
ShaderModule_DrawCallPS_Draw_2 = null;
BindGroupLayout_DrawCall_Draw_2 = null;
PipelineLayout_DrawCall_Draw_2 = null;
Pipeline_DrawCall_Draw_2 = null;

// -------------------- Imported Members

// Buffer Vertex_Buffer
buffer_Vertex_Buffer = null;
buffer_Vertex_Buffer_count = 0;
buffer_Vertex_Buffer_stride = 0;
buffer_Vertex_Buffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.VERTEX;
buffer_Vertex_Buffer_vertexBufferAttributes = null;

// -------------------- Exported Members

// Texture Color
texture_Color = null;
texture_Color_size = [0, 0, 0];
texture_Color_format = "";
texture_Color_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// Texture Depth_Stencil
texture_Depth_Stencil = null;
texture_Depth_Stencil_size = [0, 0, 0];
texture_Depth_Stencil_format = "";
texture_Depth_Stencil_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// -------------------- Public Variables

variable_MouseState = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_MouseState = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_MouseState = [ false, false, false, false ];
variable_MouseStateLastFrame = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_MouseStateLastFrame = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_MouseStateLastFrame = [ false, false, false, false ];
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
variable_ViewMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableDefault_ViewMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableChanged_ViewMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_InvViewMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableDefault_InvViewMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableChanged_InvViewMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_ProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableDefault_ProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableChanged_ProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_InvProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableDefault_InvProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableChanged_InvProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_ViewProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableDefault_ViewProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableChanged_ViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_InvViewProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableDefault_InvViewProjMtx = [ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000 ];
variableChanged_InvViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableDefault_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableChanged_CameraPos = [ false, false, false ];
variable_CameraChanged = false;
variableDefault_CameraChanged = false;
variableChanged_CameraChanged = false;

// -------------------- Structs

static StructOffsets_VertexBuffer =
{
    pos_0: 0,
    pos_1: 4,
    pos_2: 8,
    _padding0: 12,
    normal_0: 16,
    normal_1: 20,
    normal_2: 24,
    _padding1: 28,
    uv_0: 32,
    uv_1: 36,
    _padding2: 40,
    _padding3: 44,
    _size: 48,
}

static StructOffsets__Draw1VSCB =
{
    ViewProjMtx_0: 0,
    ViewProjMtx_1: 4,
    ViewProjMtx_2: 8,
    ViewProjMtx_3: 12,
    ViewProjMtx_4: 16,
    ViewProjMtx_5: 20,
    ViewProjMtx_6: 24,
    ViewProjMtx_7: 28,
    ViewProjMtx_8: 32,
    ViewProjMtx_9: 36,
    ViewProjMtx_10: 40,
    ViewProjMtx_11: 44,
    ViewProjMtx_12: 48,
    ViewProjMtx_13: 52,
    ViewProjMtx_14: 56,
    ViewProjMtx_15: 60,
    _size: 64,
}

static StructOffsets_VertexBuffer_Unpadded =
{
    pos_0: 0,
    pos_1: 4,
    pos_2: 8,
    normal_0: 12,
    normal_1: 16,
    normal_2: 20,
    uv_0: 24,
    uv_1: 28,
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
    // Validate buffer Vertex_Buffer
    if (this.buffer_Vertex_Buffer === null)
    {
        Shared.LogError("Imported resource buffer_Vertex_Buffer was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Color
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 128) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 128) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
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
                label: "texture Stencil.Color",
                size: this.texture_Color_size,
                format: Shared.GetNonSRGBFormat(this.texture_Color_format),
                usage: this.texture_Color_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Depth_Stencil
    {
        const baseSize = this.texture_Color_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "depth24plus-stencil8";
        if (this.texture_Depth_Stencil !== null && (this.texture_Depth_Stencil_format != desiredFormat || this.texture_Depth_Stencil_size[0] != desiredSize[0] || this.texture_Depth_Stencil_size[1] != desiredSize[1] || this.texture_Depth_Stencil_size[2] != desiredSize[2]))
        {
            this.texture_Depth_Stencil.destroy();
            this.texture_Depth_Stencil = null;
        }

        if (this.texture_Depth_Stencil === null)
        {
            this.texture_Depth_Stencil_size = desiredSize.slice();
            this.texture_Depth_Stencil_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Depth_Stencil_format))
                viewFormats.push(this.texture_Depth_Stencil_format);

            this.texture_Depth_Stencil = device.createTexture({
                label: "texture Stencil.Depth_Stencil",
                size: this.texture_Depth_Stencil_size,
                format: Shared.GetNonSRGBFormat(this.texture_Depth_Stencil_format),
                usage: this.texture_Depth_Stencil_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create draw call shader Draw_1
    {
        const bindGroupEntries =
        [
            {
                // _Draw1VSCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Draw_1 === null || newHash !== this.Hash_DrawCall_Draw_1)
        {
            this.Hash_DrawCall_Draw_1 = newHash;

            let shaderCodeVS = class_Stencil.ShaderCode_Draw_1_Draw1VS;
            let shaderCodePS = class_Stencil.ShaderCode_Draw_1_Draw1PS;

            this.ShaderModule_DrawCallVS_Draw_1 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Draw_1"});
            this.ShaderModule_DrawCallPS_Draw_1 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Draw_1"});

            this.BindGroupLayout_DrawCall_Draw_1 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Draw_1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Draw_1 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Draw_1",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Draw_1],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Draw_1",
                layout: this.PipelineLayout_DrawCall_Draw_1,
                primitive:
                {
                    cullMode: "none",
                    frontFace: "ccw",
                    topology: "triangle-list",
                },
                multisample:
                {
                    alphaToCoverageEnabled: false,
                },
                depthStencil:
                {
                    depthCompare: "greater",
                    depthWriteEnabled: true,
                    format: this.texture_Depth_Stencil_format,
                    stencilBack:
                    {
                        compare: "always",
                        depthFailOp: "keep",
                        failOp: "keep",
                        passOp: "replace",
                    },
                    stencilFront:
                    {
                        compare: "always",
                        depthFailOp: "keep",
                        failOp: "keep",
                        passOp: "replace",
                    },
                    stencilReadMask: 0xFF,
                    stencilWriteMask: 0xFF,
                },
                vertex:
                {
                    module: this.ShaderModule_DrawCallVS_Draw_1,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_Vertex_Buffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_Vertex_Buffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Draw_1,
                    targets:
                    [
                        {
                            blend:
                            {
                                color:
                                {
                                    srcFactor: "one",
                                    dstFactor: "zero",
                                },
                                alpha:
                                {
                                    srcFactor: "one",
                                    dstFactor: "zero",
                                },
                            },
                            format: this.texture_Color_format,
                            writeMask: GPUColorWrite.RED | GPUColorWrite.GREEN | GPUColorWrite.BLUE | GPUColorWrite.ALPHA,
                        },
                    ],
                },
            };

            if (useBlockingAPIs)
            {
                this.Pipeline_DrawCall_Draw_1 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Draw_1");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Draw_1 = handle; this.loadingPromises.delete("Draw_1"); } );
            }
        }
    }

    // Load texture _loadedTexture_0 from "cabinsmall.png"
    if (this.texture__loadedTexture_0 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/cabinsmall.png", this.texture__loadedTexture_0_usageFlags, "2d");
        this.texture__loadedTexture_0 = loadedTex.texture;
        this.texture__loadedTexture_0_size = loadedTex.size;
        this.texture__loadedTexture_0_format = "rgba8unorm";
        this.texture__loadedTexture_0_usageFlags = loadedTex.usageFlags;
    }
    // (Re)create draw call shader Draw_2
    {
        const bindGroupEntries =
        [
            {
                // _loadedTexture_0
                binding: 100,
                visibility: GPUShaderStage.FRAGMENT,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Draw_2 === null || newHash !== this.Hash_DrawCall_Draw_2)
        {
            this.Hash_DrawCall_Draw_2 = newHash;

            let shaderCodeVS = class_Stencil.ShaderCode_Draw_2_Draw2VS;
            let shaderCodePS = class_Stencil.ShaderCode_Draw_2_Draw2PS;

            this.ShaderModule_DrawCallVS_Draw_2 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Draw_2"});
            this.ShaderModule_DrawCallPS_Draw_2 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Draw_2"});

            this.BindGroupLayout_DrawCall_Draw_2 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Draw_2",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Draw_2 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Draw_2",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Draw_2],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Draw_2",
                layout: this.PipelineLayout_DrawCall_Draw_2,
                primitive:
                {
                    cullMode: "none",
                    frontFace: "ccw",
                    topology: "triangle-list",
                },
                multisample:
                {
                    alphaToCoverageEnabled: false,
                },
                depthStencil:
                {
                    depthCompare: "always",
                    depthWriteEnabled: false,
                    format: this.texture_Depth_Stencil_format,
                    stencilBack:
                    {
                        compare: "equal",
                        depthFailOp: "keep",
                        failOp: "keep",
                        passOp: "keep",
                    },
                    stencilFront:
                    {
                        compare: "equal",
                        depthFailOp: "keep",
                        failOp: "keep",
                        passOp: "keep",
                    },
                    stencilReadMask: 0xFF,
                    stencilWriteMask: 0xFF,
                },
                vertex:
                {
                    module: this.ShaderModule_DrawCallVS_Draw_2,
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Draw_2,
                    targets:
                    [
                        {
                            blend:
                            {
                                color:
                                {
                                    srcFactor: "one",
                                    dstFactor: "zero",
                                },
                                alpha:
                                {
                                    srcFactor: "one",
                                    dstFactor: "zero",
                                },
                            },
                            format: this.texture_Color_format,
                            writeMask: GPUColorWrite.RED | GPUColorWrite.GREEN | GPUColorWrite.BLUE | GPUColorWrite.ALPHA,
                        },
                    ],
                },
            };

            if (useBlockingAPIs)
            {
                this.Pipeline_DrawCall_Draw_2 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Draw_2");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Draw_2 = handle; this.loadingPromises.delete("Draw_2"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("Stencil.Vertex_Buffer");

    encoder.popDebugGroup(); // "Stencil.Vertex_Buffer"

    encoder.pushDebugGroup("Stencil.Color");

    encoder.popDebugGroup(); // "Stencil.Color"

    encoder.pushDebugGroup("Stencil.Depth_Stencil");

    encoder.popDebugGroup(); // "Stencil.Depth_Stencil"

    encoder.pushDebugGroup("Stencil._Draw1VSCB");

    // Create constant buffer _Draw1VSCB
    if (this.constantBuffer__Draw1VSCB === null)
    {
        this.constantBuffer__Draw1VSCB = device.createBuffer({
            label: "Stencil._Draw1VSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__Draw1VSCB._size),
            usage: this.constantBuffer__Draw1VSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _Draw1VSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Draw1VSCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_0, this.variable_ViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_1, this.variable_ViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_2, this.variable_ViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_3, this.variable_ViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_4, this.variable_ViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_5, this.variable_ViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_6, this.variable_ViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_7, this.variable_ViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_8, this.variable_ViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_9, this.variable_ViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_10, this.variable_ViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_11, this.variable_ViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_12, this.variable_ViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_13, this.variable_ViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_14, this.variable_ViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__Draw1VSCB.ViewProjMtx_15, this.variable_ViewProjMtx[15], true);
        device.queue.writeBuffer(this.constantBuffer__Draw1VSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "Stencil._Draw1VSCB"

    encoder.pushDebugGroup("Stencil.Draw_1");

    // Run draw call shader Draw_1
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Draw_1",
            layout: this.BindGroupLayout_DrawCall_Draw_1,
            entries:
            [
                {
                    // _Draw1VSCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__Draw1VSCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Draw_1",
            colorAttachments:
            [
                {
                    clearValue: [ 0, 0, 0, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color.createView({ dimension: "2d", format: this.texture_Color_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Stencil.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Stencil_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "clear";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_Vertex_Buffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Draw_1 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Draw_1);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(128);
                passEncoder.setVertexBuffer(0, this.buffer_Vertex_Buffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Stencil.Draw_1"

    encoder.pushDebugGroup("Stencil._loadedTexture_0");

    encoder.popDebugGroup(); // "Stencil._loadedTexture_0"

    encoder.pushDebugGroup("Stencil.Draw_2");

    // Run draw call shader Draw_2
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Draw_2",
            layout: this.BindGroupLayout_DrawCall_Draw_2,
            entries:
            [
                {
                    // _loadedTexture_0
                    binding: 100,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Draw_2",
            colorAttachments:
            [
                {
                    clearValue: [ 1, 1, 1, 1 ],
                    loadOp: "load",
                    storeOp: "store",
                    view: this.texture_Color.createView({ dimension: "2d", format: this.texture_Color_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: true,
                view: this.texture_Depth_Stencil.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Stencil_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = 4;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Draw_2 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Draw_2);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(128);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Stencil.Draw_2"

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

var Stencil = new class_Stencil;

export default Stencil;
