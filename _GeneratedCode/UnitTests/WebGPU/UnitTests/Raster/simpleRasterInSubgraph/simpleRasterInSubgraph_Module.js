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

class class_simpleRasterInSubgraph
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Vertex shader "DoSimpleRaster_VertexShader", node "DoSimpleRaster_Rasterize"
static ShaderCode_DoSimpleRaster_Rasterize_DoSimpleRaster_VertexShader = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_DoSimpleRaster_VertexShaderCB_std140_0
{
    @align(16) DoSimpleRaster_ViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
};

@binding(0) @group(0) var<uniform> _DoSimpleRaster_VertexShaderCB : Struct_DoSimpleRaster_VertexShaderCB_std140_0;
struct VSOutput_0
{
    @builtin(position) position_0 : vec4<f32>,
    @location(1) normal_0 : vec3<f32>,
};

struct vertexInput_0
{
    @location(0) position_1 : vec3<f32>,
    @location(1) normal_1 : vec3<f32>,
};

@vertex
fn VSMain( _S1 : vertexInput_0) -> VSOutput_0
{
    const _S2 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var ret_0 : VSOutput_0;
    ret_0.position_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.normal_0 = _S2;
    ret_0.position_0 = (((mat4x4<f32>(_DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(0)][i32(0)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(0)][i32(1)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(0)][i32(2)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(0)][i32(3)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(1)][i32(0)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(1)][i32(1)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(1)][i32(2)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(1)][i32(3)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(2)][i32(0)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(2)][i32(1)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(2)][i32(2)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(2)][i32(3)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(3)][i32(0)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(3)][i32(1)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(3)][i32(2)], _DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "DoSimpleRaster_PixelShader", node "DoSimpleRaster_Rasterize"
static ShaderCode_DoSimpleRaster_Rasterize_DoSimpleRaster_PixelShader = `
struct PSOutput_0
{
    @location(0) colorTarget_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(1) normal_0 : vec3<f32>,
};

@fragment
fn PSMain( _S1 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> PSOutput_0
{
    var ret_0 : PSOutput_0;
    ret_0.colorTarget_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var _S2 : vec3<f32> = vec3<f32>(0.5f);
    ret_0.colorTarget_0 = vec4<f32>(_S1.normal_0 * _S2 + _S2, 1.0f);
    return ret_0;
}

`;

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct DoSimpleRaster_VertexFormat
static StructVertexBufferAttributes_DoSimpleRaster_VertexFormat =
[
    // Position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // Normal
    {
        format: "float32x3",
        offset: 16,
        shaderLocation: 1,
    },
];

// Vertex buffer attributes for struct DoSimpleRaster_VertexFormat_Unpadded
static StructVertexBufferAttributes_DoSimpleRaster_VertexFormat_Unpadded =
[
    // Position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // Normal
    {
        format: "float32x3",
        offset: 12,
        shaderLocation: 1,
    },
];

// -------------------- Private Members

// Buffer DoSimpleRaster_VBCopy : This is here for the benefit of simpleRasterInSubgraph. Need a reference to a struct type in a subgraph.
buffer_DoSimpleRaster_VBCopy = null;
buffer_DoSimpleRaster_VBCopy_count = 0;
buffer_DoSimpleRaster_VBCopy_stride = 0;
buffer_DoSimpleRaster_VBCopy_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.VERTEX;
buffer_DoSimpleRaster_VBCopy_vertexBufferAttributes = class_simpleRasterInSubgraph.StructVertexBufferAttributes_DoSimpleRaster_VertexFormat;

// Constant buffer _DoSimpleRaster_VertexShaderCB
constantBuffer__DoSimpleRaster_VertexShaderCB = null;
constantBuffer__DoSimpleRaster_VertexShaderCB_size = 64;
constantBuffer__DoSimpleRaster_VertexShaderCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Draw call Shader DoSimpleRaster_Rasterize
Hash_DrawCall_DoSimpleRaster_Rasterize = 0;
ShaderModule_DrawCallVS_DoSimpleRaster_Rasterize = null;
ShaderModule_DrawCallPS_DoSimpleRaster_Rasterize = null;
BindGroupLayout_DrawCall_DoSimpleRaster_Rasterize = null;
PipelineLayout_DrawCall_DoSimpleRaster_Rasterize = null;
Pipeline_DrawCall_DoSimpleRaster_Rasterize = null;

// -------------------- Imported Members

// Buffer VB
buffer_VB = null;
buffer_VB_count = 0;
buffer_VB_stride = 0;
buffer_VB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC;

// -------------------- Exported Members

// Texture DoSimpleRaster_Color_Buffer
texture_DoSimpleRaster_Color_Buffer = null;
texture_DoSimpleRaster_Color_Buffer_size = [0, 0, 0];
texture_DoSimpleRaster_Color_Buffer_format = "";
texture_DoSimpleRaster_Color_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// Texture DoSimpleRaster_Depth_Buffer
texture_DoSimpleRaster_Depth_Buffer = null;
texture_DoSimpleRaster_Depth_Buffer_size = [0, 0, 0];
texture_DoSimpleRaster_Depth_Buffer_format = "";
texture_DoSimpleRaster_Depth_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// -------------------- Public Variables

variable_DoSimpleRaster_MouseState = [ 0., 0., 0., 0. ];
variableDefault_DoSimpleRaster_MouseState = [ 0., 0., 0., 0. ];
variableChanged_DoSimpleRaster_MouseState = [ false, false, false, false ];
variable_DoSimpleRaster_MouseStateLastFrame = [ 0., 0., 0., 0. ];
variableDefault_DoSimpleRaster_MouseStateLastFrame = [ 0., 0., 0., 0. ];
variableChanged_DoSimpleRaster_MouseStateLastFrame = [ false, false, false, false ];
variable_DoSimpleRaster_iResolution = [ 0., 0., 0. ];
variableDefault_DoSimpleRaster_iResolution = [ 0., 0., 0. ];
variableChanged_DoSimpleRaster_iResolution = [ false, false, false ];
variable_DoSimpleRaster_iTime = 0.;
variableDefault_DoSimpleRaster_iTime = 0.;
variableChanged_DoSimpleRaster_iTime = false;
variable_DoSimpleRaster_iTimeDelta = 0.;
variableDefault_DoSimpleRaster_iTimeDelta = 0.;
variableChanged_DoSimpleRaster_iTimeDelta = false;
variable_DoSimpleRaster_iFrameRate = 0.;
variableDefault_DoSimpleRaster_iFrameRate = 0.;
variableChanged_DoSimpleRaster_iFrameRate = false;
variable_DoSimpleRaster_iFrame = 0;
variableDefault_DoSimpleRaster_iFrame = 0;
variableChanged_DoSimpleRaster_iFrame = false;
variable_DoSimpleRaster_iMouse = [ 0., 0., 0., 0. ];
variableDefault_DoSimpleRaster_iMouse = [ 0., 0., 0., 0. ];
variableChanged_DoSimpleRaster_iMouse = [ false, false, false, false ];
variable_DoSimpleRaster_ViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_DoSimpleRaster_ViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_DoSimpleRaster_ViewMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_DoSimpleRaster_InvViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_DoSimpleRaster_InvViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_DoSimpleRaster_InvViewMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_DoSimpleRaster_ProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_DoSimpleRaster_ProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_DoSimpleRaster_ProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_DoSimpleRaster_InvProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_DoSimpleRaster_InvProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_DoSimpleRaster_InvProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_DoSimpleRaster_ViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_DoSimpleRaster_ViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_DoSimpleRaster_ViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_DoSimpleRaster_InvViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_DoSimpleRaster_InvViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_DoSimpleRaster_InvViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_DoSimpleRaster_CameraPos = [ 0., 0., 0. ];
variableDefault_DoSimpleRaster_CameraPos = [ 0., 0., 0. ];
variableChanged_DoSimpleRaster_CameraPos = [ false, false, false ];

// -------------------- Structs

static StructOffsets_DoSimpleRaster_VertexFormat =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    _padding0: 12,
    Normal_0: 16,
    Normal_1: 20,
    Normal_2: 24,
    _padding1: 28,
    _size: 32,
}

static StructOffsets__DoSimpleRaster_VertexShaderCB =
{
    DoSimpleRaster_ViewProjMtx_0: 0,
    DoSimpleRaster_ViewProjMtx_1: 4,
    DoSimpleRaster_ViewProjMtx_2: 8,
    DoSimpleRaster_ViewProjMtx_3: 12,
    DoSimpleRaster_ViewProjMtx_4: 16,
    DoSimpleRaster_ViewProjMtx_5: 20,
    DoSimpleRaster_ViewProjMtx_6: 24,
    DoSimpleRaster_ViewProjMtx_7: 28,
    DoSimpleRaster_ViewProjMtx_8: 32,
    DoSimpleRaster_ViewProjMtx_9: 36,
    DoSimpleRaster_ViewProjMtx_10: 40,
    DoSimpleRaster_ViewProjMtx_11: 44,
    DoSimpleRaster_ViewProjMtx_12: 48,
    DoSimpleRaster_ViewProjMtx_13: 52,
    DoSimpleRaster_ViewProjMtx_14: 56,
    DoSimpleRaster_ViewProjMtx_15: 60,
    _size: 64,
}

static StructOffsets_DoSimpleRaster_VertexFormat_Unpadded =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    Normal_0: 12,
    Normal_1: 16,
    Normal_2: 20,
    _size: 24,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate buffer VB
    if (this.buffer_VB === null)
    {
        Shared.LogError("Imported resource buffer_VB was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture DoSimpleRaster_Color_Buffer
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_DoSimpleRaster_Color_Buffer !== null && (this.texture_DoSimpleRaster_Color_Buffer_format != desiredFormat || this.texture_DoSimpleRaster_Color_Buffer_size[0] != desiredSize[0] || this.texture_DoSimpleRaster_Color_Buffer_size[1] != desiredSize[1] || this.texture_DoSimpleRaster_Color_Buffer_size[2] != desiredSize[2]))
        {
            this.texture_DoSimpleRaster_Color_Buffer.destroy();
            this.texture_DoSimpleRaster_Color_Buffer = null;
        }

        if (this.texture_DoSimpleRaster_Color_Buffer === null)
        {
            this.texture_DoSimpleRaster_Color_Buffer_size = desiredSize.slice();
            this.texture_DoSimpleRaster_Color_Buffer_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_DoSimpleRaster_Color_Buffer_format))
                viewFormats.push(this.texture_DoSimpleRaster_Color_Buffer_format);

            this.texture_DoSimpleRaster_Color_Buffer = device.createTexture({
                label: "texture simpleRasterInSubgraph.DoSimpleRaster_Color_Buffer",
                size: this.texture_DoSimpleRaster_Color_Buffer_size,
                format: Shared.GetNonSRGBFormat(this.texture_DoSimpleRaster_Color_Buffer_format),
                usage: this.texture_DoSimpleRaster_Color_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture DoSimpleRaster_Depth_Buffer
    {
        const baseSize = this.texture_DoSimpleRaster_Color_Buffer_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "depth32float";
        if (this.texture_DoSimpleRaster_Depth_Buffer !== null && (this.texture_DoSimpleRaster_Depth_Buffer_format != desiredFormat || this.texture_DoSimpleRaster_Depth_Buffer_size[0] != desiredSize[0] || this.texture_DoSimpleRaster_Depth_Buffer_size[1] != desiredSize[1] || this.texture_DoSimpleRaster_Depth_Buffer_size[2] != desiredSize[2]))
        {
            this.texture_DoSimpleRaster_Depth_Buffer.destroy();
            this.texture_DoSimpleRaster_Depth_Buffer = null;
        }

        if (this.texture_DoSimpleRaster_Depth_Buffer === null)
        {
            this.texture_DoSimpleRaster_Depth_Buffer_size = desiredSize.slice();
            this.texture_DoSimpleRaster_Depth_Buffer_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_DoSimpleRaster_Depth_Buffer_format))
                viewFormats.push(this.texture_DoSimpleRaster_Depth_Buffer_format);

            this.texture_DoSimpleRaster_Depth_Buffer = device.createTexture({
                label: "texture simpleRasterInSubgraph.DoSimpleRaster_Depth_Buffer",
                size: this.texture_DoSimpleRaster_Depth_Buffer_size,
                format: Shared.GetNonSRGBFormat(this.texture_DoSimpleRaster_Depth_Buffer_format),
                usage: this.texture_DoSimpleRaster_Depth_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of buffer DoSimpleRaster_VBCopy
    {
        const baseCount = this.buffer_VB_count;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = 32;
        if (this.buffer_DoSimpleRaster_VBCopy !== null && (this.buffer_DoSimpleRaster_VBCopy_count != desiredCount || this.buffer_DoSimpleRaster_VBCopy_stride != desiredStride))
        {
            this.buffer_DoSimpleRaster_VBCopy.destroy();
            this.buffer_DoSimpleRaster_VBCopy = null;
        }

        if (this.buffer_DoSimpleRaster_VBCopy === null)
        {
            this.buffer_DoSimpleRaster_VBCopy_count = desiredCount;
            this.buffer_DoSimpleRaster_VBCopy_stride = desiredStride;
            this.buffer_DoSimpleRaster_VBCopy = device.createBuffer({
                label: "buffer simpleRasterInSubgraph.DoSimpleRaster_VBCopy",
                size: Shared.Align(16, this.buffer_DoSimpleRaster_VBCopy_count * this.buffer_DoSimpleRaster_VBCopy_stride),
                usage: this.buffer_DoSimpleRaster_VBCopy_usageFlags,
            });
        }
    }

    // (Re)create draw call shader DoSimpleRaster_Rasterize
    {
        const bindGroupEntries =
        [
            {
                // _DoSimpleRaster_VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_DoSimpleRaster_Rasterize === null || newHash !== this.Hash_DrawCall_DoSimpleRaster_Rasterize)
        {
            this.Hash_DrawCall_DoSimpleRaster_Rasterize = newHash;

            let shaderCodeVS = class_simpleRasterInSubgraph.ShaderCode_DoSimpleRaster_Rasterize_DoSimpleRaster_VertexShader;
            let shaderCodePS = class_simpleRasterInSubgraph.ShaderCode_DoSimpleRaster_Rasterize_DoSimpleRaster_PixelShader;

            this.ShaderModule_DrawCallVS_DoSimpleRaster_Rasterize = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS DoSimpleRaster_Rasterize"});
            this.ShaderModule_DrawCallPS_DoSimpleRaster_Rasterize = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS DoSimpleRaster_Rasterize"});

            this.BindGroupLayout_DrawCall_DoSimpleRaster_Rasterize = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout DoSimpleRaster_Rasterize",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_DoSimpleRaster_Rasterize = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout DoSimpleRaster_Rasterize",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_DoSimpleRaster_Rasterize],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline DoSimpleRaster_Rasterize",
                layout: this.PipelineLayout_DrawCall_DoSimpleRaster_Rasterize,
                primitive:
                {
                    cullMode: "none",
                    frontFace: "cw",
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
                    format: this.texture_DoSimpleRaster_Depth_Buffer_format,
                    stencilBack:
                    {
                        compare: "always",
                        depthFailOp: "keep",
                        failOp: "keep",
                        passOp: "keep",
                    },
                    stencilFront:
                    {
                        compare: "always",
                        depthFailOp: "keep",
                        failOp: "keep",
                        passOp: "keep",
                    },
                    stencilReadMask: 0xFF,
                    stencilWriteMask: 0xFF,
                },
                vertex:
                {
                    module: this.ShaderModule_DrawCallVS_DoSimpleRaster_Rasterize,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_DoSimpleRaster_VBCopy_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_DoSimpleRaster_VBCopy_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_DoSimpleRaster_Rasterize,
                    targets:
                    [
                        {
                            blend:
                            {
                                color:
                                {
                                    srcFactor: "src-alpha",
                                    dstFactor: "one-minus-src-alpha",
                                },
                                alpha:
                                {
                                    srcFactor: "one",
                                    dstFactor: "zero",
                                },
                            },
                            format: this.texture_DoSimpleRaster_Color_Buffer_format,
                            writeMask: GPUColorWrite.RED | GPUColorWrite.GREEN | GPUColorWrite.BLUE,
                        },
                    ],
                },
            };

            if (useBlockingAPIs)
            {
                this.Pipeline_DrawCall_DoSimpleRaster_Rasterize = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("DoSimpleRaster_Rasterize");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_DoSimpleRaster_Rasterize = handle; this.loadingPromises.delete("DoSimpleRaster_Rasterize"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("simpleRasterInSubgraph.VB");

    encoder.popDebugGroup(); // "simpleRasterInSubgraph.VB"

    encoder.pushDebugGroup("simpleRasterInSubgraph.DoSimpleRaster_Color_Buffer");

    encoder.popDebugGroup(); // "simpleRasterInSubgraph.DoSimpleRaster_Color_Buffer"

    encoder.pushDebugGroup("simpleRasterInSubgraph.DoSimpleRaster_Depth_Buffer");

    encoder.popDebugGroup(); // "simpleRasterInSubgraph.DoSimpleRaster_Depth_Buffer"

    encoder.pushDebugGroup("simpleRasterInSubgraph.DoSimpleRaster_VBCopy");

    encoder.popDebugGroup(); // "simpleRasterInSubgraph.DoSimpleRaster_VBCopy"

    encoder.pushDebugGroup("simpleRasterInSubgraph.DoSimpleRaster_CopyVBs");

    // Copy buffer VB to buffer DoSimpleRaster_VBCopy
    // This is here for the benefit of simpleRasterInSubgraph. Need a reference to a struct type in a subgraph.
    {
        encoder.copyBufferToBuffer(
            this.buffer_VB,
            0,
            this.buffer_DoSimpleRaster_VBCopy,
            0,
            Math.min(this.buffer_VB.size, this.buffer_DoSimpleRaster_VBCopy.size)
        );
    }

    encoder.popDebugGroup(); // "simpleRasterInSubgraph.DoSimpleRaster_CopyVBs"

    encoder.pushDebugGroup("simpleRasterInSubgraph._DoSimpleRaster_VertexShaderCB");

    // Create constant buffer _DoSimpleRaster_VertexShaderCB
    if (this.constantBuffer__DoSimpleRaster_VertexShaderCB === null)
    {
        this.constantBuffer__DoSimpleRaster_VertexShaderCB = device.createBuffer({
            label: "simpleRasterInSubgraph._DoSimpleRaster_VertexShaderCB",
            size: Shared.Align(16, this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB._size),
            usage: this.constantBuffer__DoSimpleRaster_VertexShaderCB_usageFlags,
        });
    }

    // Upload values to constant buffer _DoSimpleRaster_VertexShaderCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_0, this.variable_DoSimpleRaster_ViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_1, this.variable_DoSimpleRaster_ViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_2, this.variable_DoSimpleRaster_ViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_3, this.variable_DoSimpleRaster_ViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_4, this.variable_DoSimpleRaster_ViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_5, this.variable_DoSimpleRaster_ViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_6, this.variable_DoSimpleRaster_ViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_7, this.variable_DoSimpleRaster_ViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_8, this.variable_DoSimpleRaster_ViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_9, this.variable_DoSimpleRaster_ViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_10, this.variable_DoSimpleRaster_ViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_11, this.variable_DoSimpleRaster_ViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_12, this.variable_DoSimpleRaster_ViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_13, this.variable_DoSimpleRaster_ViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_14, this.variable_DoSimpleRaster_ViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__DoSimpleRaster_VertexShaderCB.DoSimpleRaster_ViewProjMtx_15, this.variable_DoSimpleRaster_ViewProjMtx[15], true);
        device.queue.writeBuffer(this.constantBuffer__DoSimpleRaster_VertexShaderCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "simpleRasterInSubgraph._DoSimpleRaster_VertexShaderCB"

    encoder.pushDebugGroup("simpleRasterInSubgraph.DoSimpleRaster_Rasterize");

    // Run draw call shader DoSimpleRaster_Rasterize
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group DoSimpleRaster_Rasterize",
            layout: this.BindGroupLayout_DrawCall_DoSimpleRaster_Rasterize,
            entries:
            [
                {
                    // _DoSimpleRaster_VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__DoSimpleRaster_VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor DoSimpleRaster_Rasterize",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_DoSimpleRaster_Color_Buffer.createView({ dimension: "2d", format: this.texture_DoSimpleRaster_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_DoSimpleRaster_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_DoSimpleRaster_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_DoSimpleRaster_VBCopy_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_DoSimpleRaster_Rasterize !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_DoSimpleRaster_Rasterize);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_DoSimpleRaster_VBCopy);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "simpleRasterInSubgraph.DoSimpleRaster_Rasterize"

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

var simpleRasterInSubgraph = new class_simpleRasterInSubgraph;

export default simpleRasterInSubgraph;
