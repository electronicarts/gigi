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

class class_YesVertexStruct_NoIndex_YesInstanceType
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Vertex shader "VertexShader", node "Rasterize"
static ShaderCode_Rasterize_VertexShader = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_VertexShaderCB_std140_0
{
    @align(16) ViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
};

@binding(0) @group(0) var<uniform> _VertexShaderCB : Struct_VertexShaderCB_std140_0;
struct VSOutput_0
{
    @builtin(position) position_0 : vec4<f32>,
    @location(0) normal_0 : vec3<f32>,
    @location(1) uv_0 : vec2<f32>,
};

struct vertexInput_0
{
    @location(0) position_1 : vec3<f32>,
    @location(2) normal_1 : vec3<f32>,
    @location(3) uv_1 : vec2<f32>,
    @location(1) offsetScale_0 : vec4<f32>,
};

@vertex
fn VSMain( _S1 : vertexInput_0) -> VSOutput_0
{
    const _S2 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const _S3 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
    var ret_0 : VSOutput_0;
    ret_0.position_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.normal_0 = _S2;
    ret_0.uv_0 = _S3;
    ret_0.position_0 = (((mat4x4<f32>(_VertexShaderCB.ViewProjMtx_0.data_0[i32(0)][i32(0)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(0)][i32(1)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(0)][i32(2)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(0)][i32(3)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(1)][i32(0)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(1)][i32(1)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(1)][i32(2)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(1)][i32(3)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(2)][i32(0)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(2)][i32(1)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(2)][i32(2)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(2)][i32(3)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(3)][i32(0)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(3)][i32(1)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(3)][i32(2)], _VertexShaderCB.ViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(_S1.position_1 * vec3<f32>(_S1.offsetScale_0.w) + _S1.offsetScale_0.xyz, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    ret_0.uv_0 = _S1.uv_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize"
static ShaderCode_Rasterize_PixelShader = `
struct Struct_PixelShaderCB_std140_0
{
    @align(16) viewMode_0 : i32,
    @align(4) _padding0_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
};

@binding(100) @group(0) var<uniform> _PixelShaderCB : Struct_PixelShaderCB_std140_0;
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
fn PSMain( _S1 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> PSOutput_0
{
    var ret_0 : PSOutput_0;
    ret_0.colorTarget_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    switch(_PixelShaderCB.viewMode_0)
    {
    case i32(0), :
        {
            var _S2 : vec3<f32> = vec3<f32>(0.5f);
            ret_0.colorTarget_0 = vec4<f32>(_S1.normal_0 * _S2 + _S2, 1.0f);
            break;
        }
    case i32(1), :
        {
            ret_0.colorTarget_0 = vec4<f32>(_S1.uv_0, 0.0f, 1.0f);
            break;
        }
    case i32(2), :
        {
            ret_0.colorTarget_0 = vec4<f32>(0.0f, 1.0f, 0.0f, 1.0f);
            break;
        }
    case default, :
        {
            break;
        }
    }
    return ret_0;
}

`;

// -------------------- Enums

static Enum_ViewMode =
{
    Normal: 0,
    UV: 1,
    Solid: 2,
    _count: 3,
}

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct VertexFormat
static StructVertexBufferAttributes_VertexFormat =
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
    // UV
    {
        format: "float32x2",
        offset: 32,
        shaderLocation: 2,
    },
];

// Vertex buffer attributes for struct VertexFormat_Unpadded
static StructVertexBufferAttributes_VertexFormat_Unpadded =
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
    // UV
    {
        format: "float32x2",
        offset: 24,
        shaderLocation: 2,
    },
];

// -------------------- Private Members

// Constant buffer _VertexShaderCB
constantBuffer__VertexShaderCB = null;
constantBuffer__VertexShaderCB_size = 64;
constantBuffer__VertexShaderCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Constant buffer _PixelShaderCB
constantBuffer__PixelShaderCB = null;
constantBuffer__PixelShaderCB_size = 16;
constantBuffer__PixelShaderCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Draw call Shader Rasterize
Hash_DrawCall_Rasterize = 0;
ShaderModule_DrawCallVS_Rasterize = null;
ShaderModule_DrawCallPS_Rasterize = null;
BindGroupLayout_DrawCall_Rasterize = null;
PipelineLayout_DrawCall_Rasterize = null;
Pipeline_DrawCall_Rasterize = null;

// -------------------- Imported Members

// Buffer Vertex_Buffer
buffer_Vertex_Buffer = null;
buffer_Vertex_Buffer_count = 0;
buffer_Vertex_Buffer_stride = 0;
buffer_Vertex_Buffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.VERTEX;
buffer_Vertex_Buffer_vertexBufferAttributes = null;

// Buffer Instance_Buffer
buffer_Instance_Buffer = null;
buffer_Instance_Buffer_count = 0;
buffer_Instance_Buffer_stride = 0;
buffer_Instance_Buffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.VERTEX;
buffer_Instance_Buffer_vertexBufferAttributes = null;

// -------------------- Exported Members

// Texture Color_Buffer
texture_Color_Buffer = null;
texture_Color_Buffer_size = [0, 0, 0];
texture_Color_Buffer_format = "";
texture_Color_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// Texture Depth_Buffer
texture_Depth_Buffer = null;
texture_Depth_Buffer_size = [0, 0, 0];
texture_Depth_Buffer_format = "";
texture_Depth_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// -------------------- Public Variables

variable_MouseState = [ 0., 0., 0., 0. ];
variableDefault_MouseState = [ 0., 0., 0., 0. ];
variableChanged_MouseState = [ false, false, false, false ];
variable_MouseStateLastFrame = [ 0., 0., 0., 0. ];
variableDefault_MouseStateLastFrame = [ 0., 0., 0., 0. ];
variableChanged_MouseStateLastFrame = [ false, false, false, false ];
variable_iResolution = [ 0., 0., 0. ];
variableDefault_iResolution = [ 0., 0., 0. ];
variableChanged_iResolution = [ false, false, false ];
variable_iTime = 0.;
variableDefault_iTime = 0.;
variableChanged_iTime = false;
variable_iTimeDelta = 0.;
variableDefault_iTimeDelta = 0.;
variableChanged_iTimeDelta = false;
variable_iFrameRate = 0.;
variableDefault_iFrameRate = 0.;
variableChanged_iFrameRate = false;
variable_iFrame = 0;
variableDefault_iFrame = 0;
variableChanged_iFrame = false;
variable_iMouse = [ 0., 0., 0., 0. ];
variableDefault_iMouse = [ 0., 0., 0., 0. ];
variableChanged_iMouse = [ false, false, false, false ];
variable_ViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_ViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_ViewMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_InvViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_InvViewMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_InvViewMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_ProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_ProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_ProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_InvProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_InvProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_InvProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_ViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_ViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_ViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_InvViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableDefault_InvViewProjMtx = [ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ];
variableChanged_InvViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_CameraPos = [ 0., 0., 0. ];
variableDefault_CameraPos = [ 0., 0., 0. ];
variableChanged_CameraPos = [ false, false, false ];
variable_viewMode = this.constructor.Enum_ViewMode.Normal;
variableDefault_viewMode = this.constructor.Enum_ViewMode.Normal;
variableChanged_viewMode = false;

// -------------------- Structs

static StructOffsets_VertexFormat =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    _padding0: 12,
    Normal_0: 16,
    Normal_1: 20,
    Normal_2: 24,
    _padding1: 28,
    UV_0: 32,
    UV_1: 36,
    _padding2: 40,
    _padding3: 44,
    _size: 48,
}

static StructOffsets__VertexShaderCB =
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

static StructOffsets__PixelShaderCB =
{
    viewMode: 0,
    _padding0: 4,
    _padding1: 8,
    _padding2: 12,
    _size: 16,
}

static StructOffsets_VertexFormat_Unpadded =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    Normal_0: 12,
    Normal_1: 16,
    Normal_2: 20,
    UV_0: 24,
    UV_1: 28,
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

    // Validate buffer Instance_Buffer
    if (this.buffer_Instance_Buffer === null)
    {
        Shared.LogError("Imported resource buffer_Instance_Buffer was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Color_Buffer
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_Color_Buffer !== null && (this.texture_Color_Buffer_format != desiredFormat || this.texture_Color_Buffer_size[0] != desiredSize[0] || this.texture_Color_Buffer_size[1] != desiredSize[1] || this.texture_Color_Buffer_size[2] != desiredSize[2]))
        {
            this.texture_Color_Buffer.destroy();
            this.texture_Color_Buffer = null;
        }

        if (this.texture_Color_Buffer === null)
        {
            this.texture_Color_Buffer_size = desiredSize.slice();
            this.texture_Color_Buffer_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Color_Buffer_format))
                viewFormats.push(this.texture_Color_Buffer_format);

            this.texture_Color_Buffer = device.createTexture({
                label: "texture YesVertexStruct_NoIndex_YesInstanceType.Color_Buffer",
                size: this.texture_Color_Buffer_size,
                format: Shared.GetNonSRGBFormat(this.texture_Color_Buffer_format),
                usage: this.texture_Color_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Depth_Buffer
    {
        const baseSize = this.texture_Color_Buffer_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "depth32float";
        if (this.texture_Depth_Buffer !== null && (this.texture_Depth_Buffer_format != desiredFormat || this.texture_Depth_Buffer_size[0] != desiredSize[0] || this.texture_Depth_Buffer_size[1] != desiredSize[1] || this.texture_Depth_Buffer_size[2] != desiredSize[2]))
        {
            this.texture_Depth_Buffer.destroy();
            this.texture_Depth_Buffer = null;
        }

        if (this.texture_Depth_Buffer === null)
        {
            this.texture_Depth_Buffer_size = desiredSize.slice();
            this.texture_Depth_Buffer_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Depth_Buffer_format))
                viewFormats.push(this.texture_Depth_Buffer_format);

            this.texture_Depth_Buffer = device.createTexture({
                label: "texture YesVertexStruct_NoIndex_YesInstanceType.Depth_Buffer",
                size: this.texture_Depth_Buffer_size,
                format: Shared.GetNonSRGBFormat(this.texture_Depth_Buffer_format),
                usage: this.texture_Depth_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create draw call shader Rasterize
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
            {
                // _PixelShaderCB
                binding: 100,
                visibility: GPUShaderStage.FRAGMENT,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize === null || newHash !== this.Hash_DrawCall_Rasterize)
        {
            this.Hash_DrawCall_Rasterize = newHash;

            let shaderCodeVS = class_YesVertexStruct_NoIndex_YesInstanceType.ShaderCode_Rasterize_VertexShader;
            let shaderCodePS = class_YesVertexStruct_NoIndex_YesInstanceType.ShaderCode_Rasterize_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize"});
            this.ShaderModule_DrawCallPS_Rasterize = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize"});

            this.BindGroupLayout_DrawCall_Rasterize = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize",
                layout: this.PipelineLayout_DrawCall_Rasterize,
                primitive:
                {
                    cullMode: "back",
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
                    format: this.texture_Depth_Buffer_format,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_Vertex_Buffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_Vertex_Buffer_vertexBufferAttributes,
                        },
                        {
                            arrayStride: this.buffer_Instance_Buffer_stride,
                            stepMode: "instance",
                            attributes: this.buffer_Instance_Buffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize,
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
                            format: this.texture_Color_Buffer_format,
                            writeMask: GPUColorWrite.RED | GPUColorWrite.GREEN | GPUColorWrite.BLUE,
                        },
                    ],
                },
            };

            if (useBlockingAPIs)
            {
                this.Pipeline_DrawCall_Rasterize = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize = handle; this.loadingPromises.delete("Rasterize"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("YesVertexStruct_NoIndex_YesInstanceType.Color_Buffer");

    encoder.popDebugGroup(); // "YesVertexStruct_NoIndex_YesInstanceType.Color_Buffer"

    encoder.pushDebugGroup("YesVertexStruct_NoIndex_YesInstanceType.Depth_Buffer");

    encoder.popDebugGroup(); // "YesVertexStruct_NoIndex_YesInstanceType.Depth_Buffer"

    encoder.pushDebugGroup("YesVertexStruct_NoIndex_YesInstanceType.Vertex_Buffer");

    encoder.popDebugGroup(); // "YesVertexStruct_NoIndex_YesInstanceType.Vertex_Buffer"

    encoder.pushDebugGroup("YesVertexStruct_NoIndex_YesInstanceType.Instance_Buffer");

    encoder.popDebugGroup(); // "YesVertexStruct_NoIndex_YesInstanceType.Instance_Buffer"

    encoder.pushDebugGroup("YesVertexStruct_NoIndex_YesInstanceType._VertexShaderCB");

    // Create constant buffer _VertexShaderCB
    if (this.constantBuffer__VertexShaderCB === null)
    {
        this.constantBuffer__VertexShaderCB = device.createBuffer({
            label: "YesVertexStruct_NoIndex_YesInstanceType._VertexShaderCB",
            size: Shared.Align(16, this.constructor.StructOffsets__VertexShaderCB._size),
            usage: this.constantBuffer__VertexShaderCB_usageFlags,
        });
    }

    // Upload values to constant buffer _VertexShaderCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__VertexShaderCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_0, this.variable_ViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_1, this.variable_ViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_2, this.variable_ViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_3, this.variable_ViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_4, this.variable_ViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_5, this.variable_ViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_6, this.variable_ViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_7, this.variable_ViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_8, this.variable_ViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_9, this.variable_ViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_10, this.variable_ViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_11, this.variable_ViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_12, this.variable_ViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_13, this.variable_ViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_14, this.variable_ViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderCB.ViewProjMtx_15, this.variable_ViewProjMtx[15], true);
        device.queue.writeBuffer(this.constantBuffer__VertexShaderCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "YesVertexStruct_NoIndex_YesInstanceType._VertexShaderCB"

    encoder.pushDebugGroup("YesVertexStruct_NoIndex_YesInstanceType._PixelShaderCB");

    // Create constant buffer _PixelShaderCB
    if (this.constantBuffer__PixelShaderCB === null)
    {
        this.constantBuffer__PixelShaderCB = device.createBuffer({
            label: "YesVertexStruct_NoIndex_YesInstanceType._PixelShaderCB",
            size: Shared.Align(16, this.constructor.StructOffsets__PixelShaderCB._size),
            usage: this.constantBuffer__PixelShaderCB_usageFlags,
        });
    }

    // Upload values to constant buffer _PixelShaderCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__PixelShaderCB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__PixelShaderCB.viewMode, this.variable_viewMode, true);
        device.queue.writeBuffer(this.constantBuffer__PixelShaderCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "YesVertexStruct_NoIndex_YesInstanceType._PixelShaderCB"

    encoder.pushDebugGroup("YesVertexStruct_NoIndex_YesInstanceType.Rasterize");

    // Run draw call shader Rasterize
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize",
            layout: this.BindGroupLayout_DrawCall_Rasterize,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
                {
                    // _PixelShaderCB
                    binding: 100,
                    resource: { buffer: this.constantBuffer__PixelShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_Vertex_Buffer_count;
        const instanceCount = this.buffer_Instance_Buffer_count;

        if (this.Pipeline_DrawCall_Rasterize !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_Vertex_Buffer);
                passEncoder.setVertexBuffer(1, this.buffer_Instance_Buffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "YesVertexStruct_NoIndex_YesInstanceType.Rasterize"

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

var YesVertexStruct_NoIndex_YesInstanceType = new class_YesVertexStruct_NoIndex_YesInstanceType;

export default YesVertexStruct_NoIndex_YesInstanceType;
