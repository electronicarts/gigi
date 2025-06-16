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

class class_Mips_DrawCall
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_0"
static ShaderCode_Rasterize_Mip_0_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_0"
static ShaderCode_Rasterize_Mip_0_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_1"
static ShaderCode_Rasterize_Mip_1_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_1"
static ShaderCode_Rasterize_Mip_1_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_2"
static ShaderCode_Rasterize_Mip_2_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_2"
static ShaderCode_Rasterize_Mip_2_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_3"
static ShaderCode_Rasterize_Mip_3_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_3"
static ShaderCode_Rasterize_Mip_3_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_4"
static ShaderCode_Rasterize_Mip_4_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_4"
static ShaderCode_Rasterize_Mip_4_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_5"
static ShaderCode_Rasterize_Mip_5_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_5"
static ShaderCode_Rasterize_Mip_5_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_6"
static ShaderCode_Rasterize_Mip_6_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_6"
static ShaderCode_Rasterize_Mip_6_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_7"
static ShaderCode_Rasterize_Mip_7_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_7"
static ShaderCode_Rasterize_Mip_7_PixelShader = `
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

// Shader code for Vertex shader "VertexShader", node "Rasterize_Mip_8"
static ShaderCode_Rasterize_Mip_8_VertexShader = `
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
    ret_0.position_0 = (((transpose(mat4x4<f32>(vec4<f32>(-2.41226291656494141f, 0.0f, -0.09703599661588669f, -0.54421001672744751f), vec4<f32>(-0.00887900032103062f, 2.40408492088317871f, 0.22073699533939362f, -1.63925600051879883f), vec4<f32>(-3.99999998990097083e-06f, -0.00000900000031834f, 0.00009999999747379f, 0.09968700259923935f), vec4<f32>(0.04002499952912331f, 0.0915059968829155f, -0.99500000476837158f, 3.22507596015930176f)))) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.normal_0 = _S1.normal_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShader", node "Rasterize_Mip_8"
static ShaderCode_Rasterize_Mip_8_PixelShader = `
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

// Shader code for Vertex shader "VertexShaderSphere", node "Draw_Sphere"
static ShaderCode_Draw_Sphere_VertexShaderSphere = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_VertexShaderSphereCB_std140_0
{
    @align(16) ViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
};

@binding(0) @group(0) var<uniform> _VertexShaderSphereCB : Struct_VertexShaderSphereCB_std140_0;
struct VSOutput_0
{
    @builtin(position) position_0 : vec4<f32>,
    @location(0) UV_0 : vec2<f32>,
};

struct vertexInput_0
{
    @location(0) position_1 : vec3<f32>,
    @location(1) normal_0 : vec3<f32>,
    @location(2) UV_1 : vec2<f32>,
};

@vertex
fn VSMain( _S1 : vertexInput_0) -> VSOutput_0
{
    const _S2 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
    var ret_0 : VSOutput_0;
    ret_0.position_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.UV_0 = _S2;
    ret_0.position_0 = (((mat4x4<f32>(_VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(0)][i32(0)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(0)][i32(1)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(0)][i32(2)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(0)][i32(3)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(1)][i32(0)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(1)][i32(1)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(1)][i32(2)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(1)][i32(3)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(2)][i32(0)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(2)][i32(1)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(2)][i32(2)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(2)][i32(3)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(3)][i32(0)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(3)][i32(1)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(3)][i32(2)], _VertexShaderSphereCB.ViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(_S1.position_1, 1.0f))));
    ret_0.UV_0 = _S1.UV_1;
    return ret_0;
}

`;

// Shader code for Pixel shader "PixelShaderSphere", node "Draw_Sphere"
static ShaderCode_Draw_Sphere_PixelShaderSphere = `
@binding(100) @group(0) var MipTex : texture_2d_array<f32>;

@binding(102) @group(0) var LinearWrap : sampler;

@binding(103) @group(0) var PointWrap : sampler;

struct Struct_PixelShaderSphereCB_std140_0
{
    @align(16) UseMips_0 : u32,
    @align(4) _padding0_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
};

@binding(101) @group(0) var<uniform> _PixelShaderSphereCB : Struct_PixelShaderSphereCB_std140_0;
struct PSOutput_0
{
    @location(0) colorTarget_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) UV_0 : vec2<f32>,
};

@fragment
fn PSMain( _S1 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> PSOutput_0
{
    var ret_0 : PSOutput_0;
    ret_0.colorTarget_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var _S2 : vec3<f32> = vec3<f32>(_S1.UV_0 * vec2<f32>(2.0f), 0.0f);
    var texLinear_0 : vec4<f32> = (textureSample((MipTex), (LinearWrap), ((_S2)).xy, i32(((_S2)).z)));
    var texPoint_0 : vec4<f32> = (textureSample((MipTex), (PointWrap), ((_S2)).xy, i32(((_S2)).z)));
    var _S3 : vec4<f32>;
    if(bool(_PixelShaderSphereCB.UseMips_0))
    {
        _S3 = vec4<f32>(texLinear_0.xyz, 1.0f);
    }
    else
    {
        _S3 = vec4<f32>(texPoint_0.xyz, 1.0f);
    }
    ret_0.colorTarget_0 = _S3;
    return ret_0;
}

`;

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

// Texture Color_Buffer
texture_Color_Buffer = null;
texture_Color_Buffer_size = [0, 0, 0];
texture_Color_Buffer_format = "";
texture_Color_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING;

// Texture Depth_Buffer
texture_Depth_Buffer = null;
texture_Depth_Buffer_size = [0, 0, 0];
texture_Depth_Buffer_format = "";
texture_Depth_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// Constant buffer _VertexShaderCB
constantBuffer__VertexShaderCB = null;
constantBuffer__VertexShaderCB_size = 64;
constantBuffer__VertexShaderCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Draw call Shader Rasterize_Mip_0
Hash_DrawCall_Rasterize_Mip_0 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_0 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_0 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_0 = null;
PipelineLayout_DrawCall_Rasterize_Mip_0 = null;
Pipeline_DrawCall_Rasterize_Mip_0 = null;

// Draw call Shader Rasterize_Mip_1
Hash_DrawCall_Rasterize_Mip_1 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_1 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_1 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_1 = null;
PipelineLayout_DrawCall_Rasterize_Mip_1 = null;
Pipeline_DrawCall_Rasterize_Mip_1 = null;

// Draw call Shader Rasterize_Mip_2
Hash_DrawCall_Rasterize_Mip_2 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_2 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_2 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_2 = null;
PipelineLayout_DrawCall_Rasterize_Mip_2 = null;
Pipeline_DrawCall_Rasterize_Mip_2 = null;

// Draw call Shader Rasterize_Mip_3
Hash_DrawCall_Rasterize_Mip_3 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_3 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_3 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_3 = null;
PipelineLayout_DrawCall_Rasterize_Mip_3 = null;
Pipeline_DrawCall_Rasterize_Mip_3 = null;

// Draw call Shader Rasterize_Mip_4
Hash_DrawCall_Rasterize_Mip_4 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_4 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_4 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_4 = null;
PipelineLayout_DrawCall_Rasterize_Mip_4 = null;
Pipeline_DrawCall_Rasterize_Mip_4 = null;

// Draw call Shader Rasterize_Mip_5
Hash_DrawCall_Rasterize_Mip_5 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_5 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_5 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_5 = null;
PipelineLayout_DrawCall_Rasterize_Mip_5 = null;
Pipeline_DrawCall_Rasterize_Mip_5 = null;

// Draw call Shader Rasterize_Mip_6
Hash_DrawCall_Rasterize_Mip_6 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_6 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_6 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_6 = null;
PipelineLayout_DrawCall_Rasterize_Mip_6 = null;
Pipeline_DrawCall_Rasterize_Mip_6 = null;

// Draw call Shader Rasterize_Mip_7
Hash_DrawCall_Rasterize_Mip_7 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_7 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_7 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_7 = null;
PipelineLayout_DrawCall_Rasterize_Mip_7 = null;
Pipeline_DrawCall_Rasterize_Mip_7 = null;

// Draw call Shader Rasterize_Mip_8
Hash_DrawCall_Rasterize_Mip_8 = 0;
ShaderModule_DrawCallVS_Rasterize_Mip_8 = null;
ShaderModule_DrawCallPS_Rasterize_Mip_8 = null;
BindGroupLayout_DrawCall_Rasterize_Mip_8 = null;
PipelineLayout_DrawCall_Rasterize_Mip_8 = null;
Pipeline_DrawCall_Rasterize_Mip_8 = null;

// Constant buffer _VertexShaderSphereCB
constantBuffer__VertexShaderSphereCB = null;
constantBuffer__VertexShaderSphereCB_size = 64;
constantBuffer__VertexShaderSphereCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Constant buffer _PixelShaderSphereCB
constantBuffer__PixelShaderSphereCB = null;
constantBuffer__PixelShaderSphereCB_size = 16;
constantBuffer__PixelShaderSphereCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Draw call Shader Draw_Sphere
Hash_DrawCall_Draw_Sphere = 0;
ShaderModule_DrawCallVS_Draw_Sphere = null;
ShaderModule_DrawCallPS_Draw_Sphere = null;
BindGroupLayout_DrawCall_Draw_Sphere = null;
PipelineLayout_DrawCall_Draw_Sphere = null;
Pipeline_DrawCall_Draw_Sphere = null;

// -------------------- Imported Members

// Buffer VertexBuffer
buffer_VertexBuffer = null;
buffer_VertexBuffer_count = 0;
buffer_VertexBuffer_stride = 0;
buffer_VertexBuffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.VERTEX;
buffer_VertexBuffer_vertexBufferAttributes = null;

// Buffer SphereVB
buffer_SphereVB = null;
buffer_SphereVB_count = 0;
buffer_SphereVB_stride = 0;
buffer_SphereVB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.VERTEX;
buffer_SphereVB_vertexBufferAttributes = null;

// -------------------- Exported Members

// Texture Final_Color_Buffer
texture_Final_Color_Buffer = null;
texture_Final_Color_Buffer_size = [0, 0, 0];
texture_Final_Color_Buffer_format = "";
texture_Final_Color_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// Texture Final_Depth_Buffer
texture_Final_Depth_Buffer = null;
texture_Final_Depth_Buffer_size = [0, 0, 0];
texture_Final_Depth_Buffer_format = "";
texture_Final_Depth_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

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
variable_UseMips = true;
variableDefault_UseMips = true;
variableChanged_UseMips = false;
variable_RenderSize = [ 512, 512 ];
variableDefault_RenderSize = [ 512, 512 ];
variableChanged_RenderSize = [ false, false ];

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

static StructOffsets__VertexShaderSphereCB =
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

static StructOffsets__PixelShaderSphereCB =
{
    UseMips: 0,
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
    // Validate buffer VertexBuffer
    if (this.buffer_VertexBuffer === null)
    {
        Shared.LogError("Imported resource buffer_VertexBuffer was not provided");
        return false;
    }

    // Validate buffer SphereVB
    if (this.buffer_SphereVB === null)
    {
        Shared.LogError("Imported resource buffer_SphereVB was not provided");
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
            Math.floor(((parseInt(baseSize[0]) + 0) * 400) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 400) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 2) / 1) + 0
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

            let numMips = 1;
            {
                let maxSize = Math.max(this.texture_Color_Buffer_size[0], this.texture_Color_Buffer_size[1]);
                while (maxSize > 1)
                {
                    maxSize = Math.floor(maxSize / 2);
                    numMips = numMips + 1;
                }
            }
            this.texture_Color_Buffer = device.createTexture({
                label: "texture Mips_DrawCall.Color_Buffer",
                size: this.texture_Color_Buffer_size,
                mipLevelCount: numMips,
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

            let numMips = 1;
            {
                let maxSize = Math.max(this.texture_Depth_Buffer_size[0], this.texture_Depth_Buffer_size[1]);
                while (maxSize > 1)
                {
                    maxSize = Math.floor(maxSize / 2);
                    numMips = numMips + 1;
                }
            }
            this.texture_Depth_Buffer = device.createTexture({
                label: "texture Mips_DrawCall.Depth_Buffer",
                size: this.texture_Depth_Buffer_size,
                mipLevelCount: numMips,
                format: Shared.GetNonSRGBFormat(this.texture_Depth_Buffer_format),
                usage: this.texture_Depth_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Final_Color_Buffer
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_Final_Color_Buffer !== null && (this.texture_Final_Color_Buffer_format != desiredFormat || this.texture_Final_Color_Buffer_size[0] != desiredSize[0] || this.texture_Final_Color_Buffer_size[1] != desiredSize[1] || this.texture_Final_Color_Buffer_size[2] != desiredSize[2]))
        {
            this.texture_Final_Color_Buffer.destroy();
            this.texture_Final_Color_Buffer = null;
        }

        if (this.texture_Final_Color_Buffer === null)
        {
            this.texture_Final_Color_Buffer_size = desiredSize.slice();
            this.texture_Final_Color_Buffer_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Final_Color_Buffer_format))
                viewFormats.push(this.texture_Final_Color_Buffer_format);

            let numMips = 1;
            {
                let maxSize = Math.max(this.texture_Final_Color_Buffer_size[0], this.texture_Final_Color_Buffer_size[1]);
                while (maxSize > 1)
                {
                    maxSize = Math.floor(maxSize / 2);
                    numMips = numMips + 1;
                }
            }
            this.texture_Final_Color_Buffer = device.createTexture({
                label: "texture Mips_DrawCall.Final_Color_Buffer",
                size: this.texture_Final_Color_Buffer_size,
                mipLevelCount: numMips,
                format: Shared.GetNonSRGBFormat(this.texture_Final_Color_Buffer_format),
                usage: this.texture_Final_Color_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Final_Depth_Buffer
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "depth32float";
        if (this.texture_Final_Depth_Buffer !== null && (this.texture_Final_Depth_Buffer_format != desiredFormat || this.texture_Final_Depth_Buffer_size[0] != desiredSize[0] || this.texture_Final_Depth_Buffer_size[1] != desiredSize[1] || this.texture_Final_Depth_Buffer_size[2] != desiredSize[2]))
        {
            this.texture_Final_Depth_Buffer.destroy();
            this.texture_Final_Depth_Buffer = null;
        }

        if (this.texture_Final_Depth_Buffer === null)
        {
            this.texture_Final_Depth_Buffer_size = desiredSize.slice();
            this.texture_Final_Depth_Buffer_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Final_Depth_Buffer_format))
                viewFormats.push(this.texture_Final_Depth_Buffer_format);

            let numMips = 1;
            {
                let maxSize = Math.max(this.texture_Final_Depth_Buffer_size[0], this.texture_Final_Depth_Buffer_size[1]);
                while (maxSize > 1)
                {
                    maxSize = Math.floor(maxSize / 2);
                    numMips = numMips + 1;
                }
            }
            this.texture_Final_Depth_Buffer = device.createTexture({
                label: "texture Mips_DrawCall.Final_Depth_Buffer",
                size: this.texture_Final_Depth_Buffer_size,
                mipLevelCount: numMips,
                format: Shared.GetNonSRGBFormat(this.texture_Final_Depth_Buffer_format),
                usage: this.texture_Final_Depth_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create draw call shader Rasterize_Mip_0
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_0 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_0)
        {
            this.Hash_DrawCall_Rasterize_Mip_0 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_0_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_0_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_0 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_0"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_0 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_0"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_0 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_0",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_0 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_0",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_0],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_0",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_0,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_0,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_0,
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
                this.Pipeline_DrawCall_Rasterize_Mip_0 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_0");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_0 = handle; this.loadingPromises.delete("Rasterize_Mip_0"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_1
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_1 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_1)
        {
            this.Hash_DrawCall_Rasterize_Mip_1 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_1_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_1_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_1 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_1"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_1 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_1"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_1 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_1 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_1",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_1],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_1",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_1,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_1,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_1,
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
                this.Pipeline_DrawCall_Rasterize_Mip_1 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_1");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_1 = handle; this.loadingPromises.delete("Rasterize_Mip_1"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_2
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_2 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_2)
        {
            this.Hash_DrawCall_Rasterize_Mip_2 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_2_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_2_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_2 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_2"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_2 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_2"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_2 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_2",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_2 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_2",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_2],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_2",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_2,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_2,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_2,
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
                this.Pipeline_DrawCall_Rasterize_Mip_2 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_2");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_2 = handle; this.loadingPromises.delete("Rasterize_Mip_2"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_3
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_3 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_3)
        {
            this.Hash_DrawCall_Rasterize_Mip_3 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_3_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_3_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_3 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_3"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_3 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_3"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_3 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_3",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_3 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_3",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_3],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_3",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_3,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_3,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_3,
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
                this.Pipeline_DrawCall_Rasterize_Mip_3 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_3");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_3 = handle; this.loadingPromises.delete("Rasterize_Mip_3"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_4
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_4 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_4)
        {
            this.Hash_DrawCall_Rasterize_Mip_4 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_4_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_4_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_4 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_4"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_4 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_4"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_4 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_4",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_4 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_4",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_4],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_4",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_4,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_4,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_4,
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
                this.Pipeline_DrawCall_Rasterize_Mip_4 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_4");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_4 = handle; this.loadingPromises.delete("Rasterize_Mip_4"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_5
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_5 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_5)
        {
            this.Hash_DrawCall_Rasterize_Mip_5 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_5_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_5_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_5 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_5"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_5 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_5"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_5 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_5",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_5 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_5",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_5],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_5",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_5,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_5,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_5,
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
                this.Pipeline_DrawCall_Rasterize_Mip_5 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_5");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_5 = handle; this.loadingPromises.delete("Rasterize_Mip_5"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_6
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_6 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_6)
        {
            this.Hash_DrawCall_Rasterize_Mip_6 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_6_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_6_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_6 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_6"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_6 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_6"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_6 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_6",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_6 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_6",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_6],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_6",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_6,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_6,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_6,
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
                this.Pipeline_DrawCall_Rasterize_Mip_6 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_6");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_6 = handle; this.loadingPromises.delete("Rasterize_Mip_6"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_7
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_7 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_7)
        {
            this.Hash_DrawCall_Rasterize_Mip_7 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_7_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_7_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_7 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_7"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_7 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_7"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_7 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_7",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_7 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_7",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_7],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_7",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_7,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_7,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_7,
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
                this.Pipeline_DrawCall_Rasterize_Mip_7 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_7");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_7 = handle; this.loadingPromises.delete("Rasterize_Mip_7"); } );
            }
        }
    }

    // (Re)create draw call shader Rasterize_Mip_8
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Rasterize_Mip_8 === null || newHash !== this.Hash_DrawCall_Rasterize_Mip_8)
        {
            this.Hash_DrawCall_Rasterize_Mip_8 = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_8_VertexShader;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Rasterize_Mip_8_PixelShader;

            this.ShaderModule_DrawCallVS_Rasterize_Mip_8 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Rasterize_Mip_8"});
            this.ShaderModule_DrawCallPS_Rasterize_Mip_8 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Rasterize_Mip_8"});

            this.BindGroupLayout_DrawCall_Rasterize_Mip_8 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Rasterize_Mip_8",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Rasterize_Mip_8 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Rasterize_Mip_8",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Rasterize_Mip_8],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Rasterize_Mip_8",
                layout: this.PipelineLayout_DrawCall_Rasterize_Mip_8,
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
                    module: this.ShaderModule_DrawCallVS_Rasterize_Mip_8,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_VertexBuffer_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_VertexBuffer_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Rasterize_Mip_8,
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
                this.Pipeline_DrawCall_Rasterize_Mip_8 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Rasterize_Mip_8");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Rasterize_Mip_8 = handle; this.loadingPromises.delete("Rasterize_Mip_8"); } );
            }
        }
    }

    // (Re)create draw call shader Draw_Sphere
    {
        const bindGroupEntries =
        [
            {
                // _VertexShaderSphereCB
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: "uniform" }
            },
            {
                // MipTex
                binding: 100,
                visibility: GPUShaderStage.FRAGMENT,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_Color_Buffer_format).sampleType }
            },
            {
                // _PixelShaderSphereCB
                binding: 101,
                visibility: GPUShaderStage.FRAGMENT,
                buffer: { type: "uniform" }
            },
            {
                // LinearWrap
                binding: 102,
                visibility: GPUShaderStage.FRAGMENT,
                sampler: { },
            },
            {
                // PointWrap
                binding: 103,
                visibility: GPUShaderStage.FRAGMENT,
                sampler: { },
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Draw_Sphere === null || newHash !== this.Hash_DrawCall_Draw_Sphere)
        {
            this.Hash_DrawCall_Draw_Sphere = newHash;

            let shaderCodeVS = class_Mips_DrawCall.ShaderCode_Draw_Sphere_VertexShaderSphere;
            let shaderCodePS = class_Mips_DrawCall.ShaderCode_Draw_Sphere_PixelShaderSphere;

            this.ShaderModule_DrawCallVS_Draw_Sphere = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Draw_Sphere"});
            this.ShaderModule_DrawCallPS_Draw_Sphere = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Draw_Sphere"});

            this.BindGroupLayout_DrawCall_Draw_Sphere = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Draw_Sphere",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Draw_Sphere = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Draw_Sphere",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Draw_Sphere],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Draw_Sphere",
                layout: this.PipelineLayout_DrawCall_Draw_Sphere,
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
                    format: this.texture_Final_Depth_Buffer_format,
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
                    module: this.ShaderModule_DrawCallVS_Draw_Sphere,
                    buffers:
                    [
                        {
                            arrayStride: this.buffer_SphereVB_stride,
                            stepMode: "vertex",
                            attributes: this.buffer_SphereVB_vertexBufferAttributes,
                        },
                    ],
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Draw_Sphere,
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
                            format: this.texture_Final_Color_Buffer_format,
                            writeMask: GPUColorWrite.RED | GPUColorWrite.GREEN | GPUColorWrite.BLUE,
                        },
                    ],
                },
            };

            if (useBlockingAPIs)
            {
                this.Pipeline_DrawCall_Draw_Sphere = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Draw_Sphere");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Draw_Sphere = handle; this.loadingPromises.delete("Draw_Sphere"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("Mips_DrawCall.VertexBuffer");

    encoder.popDebugGroup(); // "Mips_DrawCall.VertexBuffer"

    encoder.pushDebugGroup("Mips_DrawCall.Color_Buffer");

    encoder.popDebugGroup(); // "Mips_DrawCall.Color_Buffer"

    encoder.pushDebugGroup("Mips_DrawCall.Depth_Buffer");

    encoder.popDebugGroup(); // "Mips_DrawCall.Depth_Buffer"

    encoder.pushDebugGroup("Mips_DrawCall.Final_Color_Buffer");

    encoder.popDebugGroup(); // "Mips_DrawCall.Final_Color_Buffer"

    encoder.pushDebugGroup("Mips_DrawCall.Final_Depth_Buffer");

    encoder.popDebugGroup(); // "Mips_DrawCall.Final_Depth_Buffer"

    encoder.pushDebugGroup("Mips_DrawCall.SphereVB");

    encoder.popDebugGroup(); // "Mips_DrawCall.SphereVB"

    encoder.pushDebugGroup("Mips_DrawCall._VertexShaderCB");

    // Create constant buffer _VertexShaderCB
    if (this.constantBuffer__VertexShaderCB === null)
    {
        this.constantBuffer__VertexShaderCB = device.createBuffer({
            label: "Mips_DrawCall._VertexShaderCB",
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

    encoder.popDebugGroup(); // "Mips_DrawCall._VertexShaderCB"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_0");

    // Run draw call shader Rasterize_Mip_0
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_0",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_0,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_0",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
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

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_0 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_0);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_0"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_1");

    // Run draw call shader Rasterize_Mip_1
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_1",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_1,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_1",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 1, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 1, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_1 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_1);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_1"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_2");

    // Run draw call shader Rasterize_Mip_2
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_2",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_2,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_2",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 2, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 2, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_2 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_2);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_2"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_3");

    // Run draw call shader Rasterize_Mip_3
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_3",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_3,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_3",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 3, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 3, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_3 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_3);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_3"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_4");

    // Run draw call shader Rasterize_Mip_4
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_4",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_4,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_4",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 4, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 4, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_4 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_4);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_4"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_5");

    // Run draw call shader Rasterize_Mip_5
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_5",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_5,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_5",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 5, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 5, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_5 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_5);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_5"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_6");

    // Run draw call shader Rasterize_Mip_6
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_6",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_6,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_6",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 6, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 6, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_6 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_6);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_6"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_7");

    // Run draw call shader Rasterize_Mip_7
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_7",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_7,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_7",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 7, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 7, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_7 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_7);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_7"

    encoder.pushDebugGroup("Mips_DrawCall.Rasterize_Mip_8");

    // Run draw call shader Rasterize_Mip_8
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Rasterize_Mip_8",
            layout: this.BindGroupLayout_DrawCall_Rasterize_Mip_8,
            entries:
            [
                {
                    // _VertexShaderCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderCB }
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Rasterize_Mip_8",
            colorAttachments:
            [
                {
                    clearValue: [ 0.2, 0.2, 0.2, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 8, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 8, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_VertexBuffer_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Rasterize_Mip_8 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Rasterize_Mip_8);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_VertexBuffer);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Rasterize_Mip_8"

    encoder.pushDebugGroup("Mips_DrawCall._VertexShaderSphereCB");

    // Create constant buffer _VertexShaderSphereCB
    if (this.constantBuffer__VertexShaderSphereCB === null)
    {
        this.constantBuffer__VertexShaderSphereCB = device.createBuffer({
            label: "Mips_DrawCall._VertexShaderSphereCB",
            size: Shared.Align(16, this.constructor.StructOffsets__VertexShaderSphereCB._size),
            usage: this.constantBuffer__VertexShaderSphereCB_usageFlags,
        });
    }

    // Upload values to constant buffer _VertexShaderSphereCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__VertexShaderSphereCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_0, this.variable_ViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_1, this.variable_ViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_2, this.variable_ViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_3, this.variable_ViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_4, this.variable_ViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_5, this.variable_ViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_6, this.variable_ViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_7, this.variable_ViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_8, this.variable_ViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_9, this.variable_ViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_10, this.variable_ViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_11, this.variable_ViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_12, this.variable_ViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_13, this.variable_ViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_14, this.variable_ViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__VertexShaderSphereCB.ViewProjMtx_15, this.variable_ViewProjMtx[15], true);
        device.queue.writeBuffer(this.constantBuffer__VertexShaderSphereCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "Mips_DrawCall._VertexShaderSphereCB"

    encoder.pushDebugGroup("Mips_DrawCall._PixelShaderSphereCB");

    // Create constant buffer _PixelShaderSphereCB
    if (this.constantBuffer__PixelShaderSphereCB === null)
    {
        this.constantBuffer__PixelShaderSphereCB = device.createBuffer({
            label: "Mips_DrawCall._PixelShaderSphereCB",
            size: Shared.Align(16, this.constructor.StructOffsets__PixelShaderSphereCB._size),
            usage: this.constantBuffer__PixelShaderSphereCB_usageFlags,
        });
    }

    // Upload values to constant buffer _PixelShaderSphereCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__PixelShaderSphereCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__PixelShaderSphereCB.UseMips, (this.variable_UseMips === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__PixelShaderSphereCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "Mips_DrawCall._PixelShaderSphereCB"

    encoder.pushDebugGroup("Mips_DrawCall.Draw_Sphere");

    // Run draw call shader Draw_Sphere
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Draw_Sphere",
            layout: this.BindGroupLayout_DrawCall_Draw_Sphere,
            entries:
            [
                {
                    // _VertexShaderSphereCB
                    binding: 0,
                    resource: { buffer: this.constantBuffer__VertexShaderSphereCB }
                },
                {
                    // MipTex
                    binding: 100,
                    resource: this.texture_Color_Buffer.createView({ dimension: "2d-array", format: this.texture_Color_Buffer_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _PixelShaderSphereCB
                    binding: 101,
                    resource: { buffer: this.constantBuffer__PixelShaderSphereCB }
                },
                {
                    // LinearWrap
                    binding: 102,
                    resource: device.createSampler({
                        label: "Sampler LinearWrap",
                        addressModeU: "repeat",
                        addressModeV: "repeat",
                        addressModeW: "repeat",
                        magFilter: "linear",
                        minFilter: "linear",
                        mipmapFilter: "linear",
                    }),
                },
                {
                    // PointWrap
                    binding: 103,
                    resource: device.createSampler({
                        label: "Sampler PointWrap",
                        addressModeU: "repeat",
                        addressModeV: "repeat",
                        addressModeW: "repeat",
                        magFilter: "nearest",
                        minFilter: "nearest",
                        mipmapFilter: "nearest",
                    }),
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Draw_Sphere",
            colorAttachments:
            [
                {
                    clearValue: [ 0.5, 0.5, 0.5, 1 ],
                    loadOp: "clear",
                    storeOp: "store",
                    view: this.texture_Final_Color_Buffer.createView({ dimension: "2d", format: this.texture_Final_Color_Buffer_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
                },
            ],
            depthStencilAttachment:
            {
                depthClearValue: 0,
                depthReadOnly: false,
                depthLoadOp: "clear",
                depthStoreOp: "store",
                view: this.texture_Final_Depth_Buffer.createView({ baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1} ),
            },
        };

        if (Shared.GetTextureFormatInfo(this.texture_Final_Depth_Buffer_format).isDepthStencil)
        {
            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x00;
            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = "load";
            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = "store";
        }

        const vertexCountPerInstance = this.buffer_SphereVB_count;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Draw_Sphere !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Draw_Sphere);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.setVertexBuffer(0, this.buffer_SphereVB);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_DrawCall.Draw_Sphere"

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

var Mips_DrawCall = new class_Mips_DrawCall;

export default Mips_DrawCall;
