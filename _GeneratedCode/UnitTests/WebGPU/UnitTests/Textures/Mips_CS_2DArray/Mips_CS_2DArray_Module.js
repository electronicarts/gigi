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

class class_Mips_CS_2DArray
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Mip0CS", node "Mip0"
static ShaderCode_Mip0_Mip0CS = `
@binding(2) @group(0) var _loadedTexture_0 : texture_2d<f32>;

@binding(0) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S1 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.x;
    }
    else
    {
        _S1 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S1;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.y;
    }
    else
    {
        _S1 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S1;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S1 = sRGBLo_0.z;
    }
    else
    {
        _S1 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S1;
    return sRGB_0;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var dims_0 : vec2<u32>;
    var _S2 : u32 = dims_0[i32(0)];
    var _S3 : u32 = dims_0[i32(1)];
    {var dim = textureDimensions((_loadedTexture_0));((_S2)) = dim.x;((_S3)) = dim.y;};
    dims_0[i32(0)] = _S2;
    dims_0[i32(1)] = _S3;
    var readPos_0 : vec2<u32> = px_0 % dims_0;
    var _S4 : vec3<u32> = vec3<u32>(px_0, u32(0));
    var _S5 : vec3<i32> = vec3<i32>(vec3<u32>(readPos_0, u32(0)));
    textureStore((Output), ((_S4)).xy, i32(((_S4)).z), (vec4<f32>(LinearToSRGB_0((textureLoad((_loadedTexture_0), ((_S5)).xy, ((_S5)).z)).xyz), 1.0f)));
    var _S6 : vec2<u32> = dims_0 / vec2<u32>(u32(2));
    var readPos_1 : vec2<u32> = (_S6 + px_0) % dims_0;
    var _S7 : vec3<u32> = vec3<u32>(px_0, u32(1));
    var _S8 : vec3<i32> = vec3<i32>(vec3<u32>(readPos_1, u32(0)));
    textureStore((Output), ((_S7)).xy, i32(((_S7)).z), (vec4<f32>(LinearToSRGB_0((textureLoad((_loadedTexture_0), ((_S8)).xy, ((_S8)).z)).xyz), 1.0f)));
    return;
}

`;

// Shader code for Compute shader "MipNCS", node "Mip1"
static ShaderCode_Mip1_MipNCS = `
@binding(2) @group(0) var InputReadOnly : texture_storage_2d_array</*(InputReadOnly_format)*/, read>;

@binding(1) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

fn SRGBToLinear_0( sRGBCol_0 : vec3<f32>) -> vec3<f32>
{
    var linearRGBLo_0 : vec3<f32> = sRGBCol_0 / vec3<f32>(12.92000007629394531f);
    var linearRGBHi_0 : vec3<f32> = pow((sRGBCol_0 + vec3<f32>(0.05499999970197678f)) / vec3<f32>(1.0549999475479126f), vec3<f32>(2.40000009536743164f, 2.40000009536743164f, 2.40000009536743164f));
    var linearRGB_0 : vec3<f32>;
    var _S1 : f32;
    if((sRGBCol_0.x) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.x;
    }
    else
    {
        _S1 = linearRGBHi_0.x;
    }
    linearRGB_0[i32(0)] = _S1;
    if((sRGBCol_0.y) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.y;
    }
    else
    {
        _S1 = linearRGBHi_0.y;
    }
    linearRGB_0[i32(1)] = _S1;
    if((sRGBCol_0.z) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.z;
    }
    else
    {
        _S1 = linearRGBHi_0.z;
    }
    linearRGB_0[i32(2)] = _S1;
    return linearRGB_0;
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S2 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.x;
    }
    else
    {
        _S2 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S2;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.y;
    }
    else
    {
        _S2 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S2;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.z;
    }
    else
    {
        _S2 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S2;
    return sRGB_0;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S3 : vec2<u32> = px_0 * vec2<u32>(u32(2));
    var _S4 : vec3<i32> = vec3<i32>(vec3<u32>(_S3, u32(0)));
    var _S5 : vec4<f32> = (textureLoad((InputReadOnly), ((_S4)).xy, i32(((_S4)).z)));
    var _S6 : vec3<f32> = SRGBToLinear_0(_S5.xyz);
    var _S7 : vec2<u32> = _S3 + vec2<u32>(u32(1), u32(0));
    var _S8 : vec3<i32> = vec3<i32>(vec3<u32>(_S7, u32(0)));
    var _S9 : vec4<f32> = (textureLoad((InputReadOnly), ((_S8)).xy, i32(((_S8)).z)));
    var _S10 : vec3<f32> = _S6 + SRGBToLinear_0(_S9.xyz);
    var _S11 : vec2<u32> = _S3 + vec2<u32>(u32(1), u32(1));
    var _S12 : vec3<i32> = vec3<i32>(vec3<u32>(_S11, u32(0)));
    var _S13 : vec4<f32> = (textureLoad((InputReadOnly), ((_S12)).xy, i32(((_S12)).z)));
    var _S14 : vec3<f32> = _S10 + SRGBToLinear_0(_S13.xyz);
    var _S15 : vec2<u32> = _S3 + vec2<u32>(u32(0), u32(1));
    var _S16 : vec3<i32> = vec3<i32>(vec3<u32>(_S15, u32(0)));
    var _S17 : vec4<f32> = (textureLoad((InputReadOnly), ((_S16)).xy, i32(((_S16)).z)));
    var _S18 : vec3<f32> = vec3<f32>(4.0f);
    var _S19 : vec3<u32> = vec3<u32>(px_0, u32(0));
    textureStore((Output), ((_S19)).xy, i32(((_S19)).z), (vec4<f32>(LinearToSRGB_0((_S14 + SRGBToLinear_0(_S17.xyz)) / _S18), 1.0f)));
    var _S20 : vec3<i32> = vec3<i32>(vec3<u32>(_S3, u32(1)));
    var _S21 : vec4<f32> = (textureLoad((InputReadOnly), ((_S20)).xy, i32(((_S20)).z)));
    var _S22 : vec3<f32> = SRGBToLinear_0(_S21.xyz);
    var _S23 : vec3<i32> = vec3<i32>(vec3<u32>(_S7, u32(1)));
    var _S24 : vec4<f32> = (textureLoad((InputReadOnly), ((_S23)).xy, i32(((_S23)).z)));
    var _S25 : vec3<f32> = _S22 + SRGBToLinear_0(_S24.xyz);
    var _S26 : vec3<i32> = vec3<i32>(vec3<u32>(_S11, u32(1)));
    var _S27 : vec4<f32> = (textureLoad((InputReadOnly), ((_S26)).xy, i32(((_S26)).z)));
    var _S28 : vec3<f32> = _S25 + SRGBToLinear_0(_S27.xyz);
    var _S29 : vec3<i32> = vec3<i32>(vec3<u32>(_S15, u32(1)));
    var _S30 : vec4<f32> = (textureLoad((InputReadOnly), ((_S29)).xy, i32(((_S29)).z)));
    var _S31 : vec3<u32> = vec3<u32>(px_0, u32(1));
    textureStore((Output), ((_S31)).xy, i32(((_S31)).z), (vec4<f32>(LinearToSRGB_0((_S28 + SRGBToLinear_0(_S30.xyz)) / _S18), 1.0f)));
    return;
}

`;

// Shader code for Compute shader "MipNCS", node "Mip2"
static ShaderCode_Mip2_MipNCS = `
@binding(2) @group(0) var InputReadOnly : texture_storage_2d_array</*(InputReadOnly_format)*/, read>;

@binding(1) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

fn SRGBToLinear_0( sRGBCol_0 : vec3<f32>) -> vec3<f32>
{
    var linearRGBLo_0 : vec3<f32> = sRGBCol_0 / vec3<f32>(12.92000007629394531f);
    var linearRGBHi_0 : vec3<f32> = pow((sRGBCol_0 + vec3<f32>(0.05499999970197678f)) / vec3<f32>(1.0549999475479126f), vec3<f32>(2.40000009536743164f, 2.40000009536743164f, 2.40000009536743164f));
    var linearRGB_0 : vec3<f32>;
    var _S1 : f32;
    if((sRGBCol_0.x) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.x;
    }
    else
    {
        _S1 = linearRGBHi_0.x;
    }
    linearRGB_0[i32(0)] = _S1;
    if((sRGBCol_0.y) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.y;
    }
    else
    {
        _S1 = linearRGBHi_0.y;
    }
    linearRGB_0[i32(1)] = _S1;
    if((sRGBCol_0.z) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.z;
    }
    else
    {
        _S1 = linearRGBHi_0.z;
    }
    linearRGB_0[i32(2)] = _S1;
    return linearRGB_0;
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S2 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.x;
    }
    else
    {
        _S2 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S2;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.y;
    }
    else
    {
        _S2 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S2;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.z;
    }
    else
    {
        _S2 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S2;
    return sRGB_0;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S3 : vec2<u32> = px_0 * vec2<u32>(u32(2));
    var _S4 : vec3<i32> = vec3<i32>(vec3<u32>(_S3, u32(0)));
    var _S5 : vec4<f32> = (textureLoad((InputReadOnly), ((_S4)).xy, i32(((_S4)).z)));
    var _S6 : vec3<f32> = SRGBToLinear_0(_S5.xyz);
    var _S7 : vec2<u32> = _S3 + vec2<u32>(u32(1), u32(0));
    var _S8 : vec3<i32> = vec3<i32>(vec3<u32>(_S7, u32(0)));
    var _S9 : vec4<f32> = (textureLoad((InputReadOnly), ((_S8)).xy, i32(((_S8)).z)));
    var _S10 : vec3<f32> = _S6 + SRGBToLinear_0(_S9.xyz);
    var _S11 : vec2<u32> = _S3 + vec2<u32>(u32(1), u32(1));
    var _S12 : vec3<i32> = vec3<i32>(vec3<u32>(_S11, u32(0)));
    var _S13 : vec4<f32> = (textureLoad((InputReadOnly), ((_S12)).xy, i32(((_S12)).z)));
    var _S14 : vec3<f32> = _S10 + SRGBToLinear_0(_S13.xyz);
    var _S15 : vec2<u32> = _S3 + vec2<u32>(u32(0), u32(1));
    var _S16 : vec3<i32> = vec3<i32>(vec3<u32>(_S15, u32(0)));
    var _S17 : vec4<f32> = (textureLoad((InputReadOnly), ((_S16)).xy, i32(((_S16)).z)));
    var _S18 : vec3<f32> = vec3<f32>(4.0f);
    var _S19 : vec3<u32> = vec3<u32>(px_0, u32(0));
    textureStore((Output), ((_S19)).xy, i32(((_S19)).z), (vec4<f32>(LinearToSRGB_0((_S14 + SRGBToLinear_0(_S17.xyz)) / _S18), 1.0f)));
    var _S20 : vec3<i32> = vec3<i32>(vec3<u32>(_S3, u32(1)));
    var _S21 : vec4<f32> = (textureLoad((InputReadOnly), ((_S20)).xy, i32(((_S20)).z)));
    var _S22 : vec3<f32> = SRGBToLinear_0(_S21.xyz);
    var _S23 : vec3<i32> = vec3<i32>(vec3<u32>(_S7, u32(1)));
    var _S24 : vec4<f32> = (textureLoad((InputReadOnly), ((_S23)).xy, i32(((_S23)).z)));
    var _S25 : vec3<f32> = _S22 + SRGBToLinear_0(_S24.xyz);
    var _S26 : vec3<i32> = vec3<i32>(vec3<u32>(_S11, u32(1)));
    var _S27 : vec4<f32> = (textureLoad((InputReadOnly), ((_S26)).xy, i32(((_S26)).z)));
    var _S28 : vec3<f32> = _S25 + SRGBToLinear_0(_S27.xyz);
    var _S29 : vec3<i32> = vec3<i32>(vec3<u32>(_S15, u32(1)));
    var _S30 : vec4<f32> = (textureLoad((InputReadOnly), ((_S29)).xy, i32(((_S29)).z)));
    var _S31 : vec3<u32> = vec3<u32>(px_0, u32(1));
    textureStore((Output), ((_S31)).xy, i32(((_S31)).z), (vec4<f32>(LinearToSRGB_0((_S28 + SRGBToLinear_0(_S30.xyz)) / _S18), 1.0f)));
    return;
}

`;

// Shader code for Compute shader "MipNCS", node "Mip3"
static ShaderCode_Mip3_MipNCS = `
@binding(2) @group(0) var InputReadOnly : texture_storage_2d_array</*(InputReadOnly_format)*/, read>;

@binding(1) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

fn SRGBToLinear_0( sRGBCol_0 : vec3<f32>) -> vec3<f32>
{
    var linearRGBLo_0 : vec3<f32> = sRGBCol_0 / vec3<f32>(12.92000007629394531f);
    var linearRGBHi_0 : vec3<f32> = pow((sRGBCol_0 + vec3<f32>(0.05499999970197678f)) / vec3<f32>(1.0549999475479126f), vec3<f32>(2.40000009536743164f, 2.40000009536743164f, 2.40000009536743164f));
    var linearRGB_0 : vec3<f32>;
    var _S1 : f32;
    if((sRGBCol_0.x) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.x;
    }
    else
    {
        _S1 = linearRGBHi_0.x;
    }
    linearRGB_0[i32(0)] = _S1;
    if((sRGBCol_0.y) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.y;
    }
    else
    {
        _S1 = linearRGBHi_0.y;
    }
    linearRGB_0[i32(1)] = _S1;
    if((sRGBCol_0.z) <= 0.04044999927282333f)
    {
        _S1 = linearRGBLo_0.z;
    }
    else
    {
        _S1 = linearRGBHi_0.z;
    }
    linearRGB_0[i32(2)] = _S1;
    return linearRGB_0;
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S2 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.x;
    }
    else
    {
        _S2 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S2;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.y;
    }
    else
    {
        _S2 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S2;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S2 = sRGBLo_0.z;
    }
    else
    {
        _S2 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S2;
    return sRGB_0;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S3 : vec2<u32> = px_0 * vec2<u32>(u32(2));
    var _S4 : vec3<i32> = vec3<i32>(vec3<u32>(_S3, u32(0)));
    var _S5 : vec4<f32> = (textureLoad((InputReadOnly), ((_S4)).xy, i32(((_S4)).z)));
    var _S6 : vec3<f32> = SRGBToLinear_0(_S5.xyz);
    var _S7 : vec2<u32> = _S3 + vec2<u32>(u32(1), u32(0));
    var _S8 : vec3<i32> = vec3<i32>(vec3<u32>(_S7, u32(0)));
    var _S9 : vec4<f32> = (textureLoad((InputReadOnly), ((_S8)).xy, i32(((_S8)).z)));
    var _S10 : vec3<f32> = _S6 + SRGBToLinear_0(_S9.xyz);
    var _S11 : vec2<u32> = _S3 + vec2<u32>(u32(1), u32(1));
    var _S12 : vec3<i32> = vec3<i32>(vec3<u32>(_S11, u32(0)));
    var _S13 : vec4<f32> = (textureLoad((InputReadOnly), ((_S12)).xy, i32(((_S12)).z)));
    var _S14 : vec3<f32> = _S10 + SRGBToLinear_0(_S13.xyz);
    var _S15 : vec2<u32> = _S3 + vec2<u32>(u32(0), u32(1));
    var _S16 : vec3<i32> = vec3<i32>(vec3<u32>(_S15, u32(0)));
    var _S17 : vec4<f32> = (textureLoad((InputReadOnly), ((_S16)).xy, i32(((_S16)).z)));
    var _S18 : vec3<f32> = vec3<f32>(4.0f);
    var _S19 : vec3<u32> = vec3<u32>(px_0, u32(0));
    textureStore((Output), ((_S19)).xy, i32(((_S19)).z), (vec4<f32>(LinearToSRGB_0((_S14 + SRGBToLinear_0(_S17.xyz)) / _S18), 1.0f)));
    var _S20 : vec3<i32> = vec3<i32>(vec3<u32>(_S3, u32(1)));
    var _S21 : vec4<f32> = (textureLoad((InputReadOnly), ((_S20)).xy, i32(((_S20)).z)));
    var _S22 : vec3<f32> = SRGBToLinear_0(_S21.xyz);
    var _S23 : vec3<i32> = vec3<i32>(vec3<u32>(_S7, u32(1)));
    var _S24 : vec4<f32> = (textureLoad((InputReadOnly), ((_S23)).xy, i32(((_S23)).z)));
    var _S25 : vec3<f32> = _S22 + SRGBToLinear_0(_S24.xyz);
    var _S26 : vec3<i32> = vec3<i32>(vec3<u32>(_S11, u32(1)));
    var _S27 : vec4<f32> = (textureLoad((InputReadOnly), ((_S26)).xy, i32(((_S26)).z)));
    var _S28 : vec3<f32> = _S25 + SRGBToLinear_0(_S27.xyz);
    var _S29 : vec3<i32> = vec3<i32>(vec3<u32>(_S15, u32(1)));
    var _S30 : vec4<f32> = (textureLoad((InputReadOnly), ((_S29)).xy, i32(((_S29)).z)));
    var _S31 : vec3<u32> = vec3<u32>(px_0, u32(1));
    textureStore((Output), ((_S31)).xy, i32(((_S31)).z), (vec4<f32>(LinearToSRGB_0((_S28 + SRGBToLinear_0(_S30.xyz)) / _S18), 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture Mip0_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Mip0_Output_ReadOnly = null;
texture_Mip0_Output_ReadOnly_size = [0, 0, 0];
texture_Mip0_Output_ReadOnly_format = "";
texture_Mip0_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Mip1_Input_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Mip1_Input_ReadOnly = null;
texture_Mip1_Input_ReadOnly_size = [0, 0, 0];
texture_Mip1_Input_ReadOnly_format = "";
texture_Mip1_Input_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Mip1_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Mip1_Output_ReadOnly = null;
texture_Mip1_Output_ReadOnly_size = [0, 0, 0];
texture_Mip1_Output_ReadOnly_format = "";
texture_Mip1_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Mip2_Input_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Mip2_Input_ReadOnly = null;
texture_Mip2_Input_ReadOnly_size = [0, 0, 0];
texture_Mip2_Input_ReadOnly_format = "";
texture_Mip2_Input_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Mip2_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Mip2_Output_ReadOnly = null;
texture_Mip2_Output_ReadOnly_size = [0, 0, 0];
texture_Mip2_Output_ReadOnly_format = "";
texture_Mip2_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Mip3_Input_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Mip3_Input_ReadOnly = null;
texture_Mip3_Input_ReadOnly_size = [0, 0, 0];
texture_Mip3_Input_ReadOnly_format = "";
texture_Mip3_Input_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Mip3_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Mip3_Output_ReadOnly = null;
texture_Mip3_Output_ReadOnly_size = [0, 0, 0];
texture_Mip3_Output_ReadOnly_format = "";
texture_Mip3_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Compute Shader Mip0
Hash_Compute_Mip0 = 0;
ShaderModule_Compute_Mip0 = null;
BindGroupLayout_Compute_Mip0 = null;
PipelineLayout_Compute_Mip0 = null;
Pipeline_Compute_Mip0 = null;

// Compute Shader Mip1
Hash_Compute_Mip1 = 0;
ShaderModule_Compute_Mip1 = null;
BindGroupLayout_Compute_Mip1 = null;
PipelineLayout_Compute_Mip1 = null;
Pipeline_Compute_Mip1 = null;

// Compute Shader Mip2
Hash_Compute_Mip2 = 0;
ShaderModule_Compute_Mip2 = null;
BindGroupLayout_Compute_Mip2 = null;
PipelineLayout_Compute_Mip2 = null;
Pipeline_Compute_Mip2 = null;

// Compute Shader Mip3
Hash_Compute_Mip3 = 0;
ShaderModule_Compute_Mip3 = null;
BindGroupLayout_Compute_Mip3 = null;
PipelineLayout_Compute_Mip3 = null;
Pipeline_Compute_Mip3 = null;

// -------------------- Exported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_RenderSize = [ 256, 256 ];
variableDefault_RenderSize = [ 256, 256 ];
variableChanged_RenderSize = [ false, false ];


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
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 2) / 1) + 0
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
                label: "texture Mips_CS_2DArray.Output",
                size: this.texture_Output_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Mip0_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Mip0_Output_ReadOnly !== null && (this.texture_Mip0_Output_ReadOnly_format != desiredFormat || this.texture_Mip0_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Mip0_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Mip0_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Mip0_Output_ReadOnly.destroy();
            this.texture_Mip0_Output_ReadOnly = null;
        }

        if (this.texture_Mip0_Output_ReadOnly === null)
        {
            this.texture_Mip0_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Mip0_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Mip0_Output_ReadOnly_format))
                viewFormats.push(this.texture_Mip0_Output_ReadOnly_format);

            this.texture_Mip0_Output_ReadOnly = device.createTexture({
                label: "texture Mips_CS_2DArray.Mip0_Output_ReadOnly",
                size: this.texture_Mip0_Output_ReadOnly_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Mip0_Output_ReadOnly_format),
                usage: this.texture_Mip0_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Mip1_Input_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Mip1_Input_ReadOnly !== null && (this.texture_Mip1_Input_ReadOnly_format != desiredFormat || this.texture_Mip1_Input_ReadOnly_size[0] != desiredSize[0] || this.texture_Mip1_Input_ReadOnly_size[1] != desiredSize[1] || this.texture_Mip1_Input_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Mip1_Input_ReadOnly.destroy();
            this.texture_Mip1_Input_ReadOnly = null;
        }

        if (this.texture_Mip1_Input_ReadOnly === null)
        {
            this.texture_Mip1_Input_ReadOnly_size = desiredSize.slice();
            this.texture_Mip1_Input_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Mip1_Input_ReadOnly_format))
                viewFormats.push(this.texture_Mip1_Input_ReadOnly_format);

            this.texture_Mip1_Input_ReadOnly = device.createTexture({
                label: "texture Mips_CS_2DArray.Mip1_Input_ReadOnly",
                size: this.texture_Mip1_Input_ReadOnly_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Mip1_Input_ReadOnly_format),
                usage: this.texture_Mip1_Input_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Mip1_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Mip1_Output_ReadOnly !== null && (this.texture_Mip1_Output_ReadOnly_format != desiredFormat || this.texture_Mip1_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Mip1_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Mip1_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Mip1_Output_ReadOnly.destroy();
            this.texture_Mip1_Output_ReadOnly = null;
        }

        if (this.texture_Mip1_Output_ReadOnly === null)
        {
            this.texture_Mip1_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Mip1_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Mip1_Output_ReadOnly_format))
                viewFormats.push(this.texture_Mip1_Output_ReadOnly_format);

            this.texture_Mip1_Output_ReadOnly = device.createTexture({
                label: "texture Mips_CS_2DArray.Mip1_Output_ReadOnly",
                size: this.texture_Mip1_Output_ReadOnly_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Mip1_Output_ReadOnly_format),
                usage: this.texture_Mip1_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Mip2_Input_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Mip2_Input_ReadOnly !== null && (this.texture_Mip2_Input_ReadOnly_format != desiredFormat || this.texture_Mip2_Input_ReadOnly_size[0] != desiredSize[0] || this.texture_Mip2_Input_ReadOnly_size[1] != desiredSize[1] || this.texture_Mip2_Input_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Mip2_Input_ReadOnly.destroy();
            this.texture_Mip2_Input_ReadOnly = null;
        }

        if (this.texture_Mip2_Input_ReadOnly === null)
        {
            this.texture_Mip2_Input_ReadOnly_size = desiredSize.slice();
            this.texture_Mip2_Input_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Mip2_Input_ReadOnly_format))
                viewFormats.push(this.texture_Mip2_Input_ReadOnly_format);

            this.texture_Mip2_Input_ReadOnly = device.createTexture({
                label: "texture Mips_CS_2DArray.Mip2_Input_ReadOnly",
                size: this.texture_Mip2_Input_ReadOnly_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Mip2_Input_ReadOnly_format),
                usage: this.texture_Mip2_Input_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Mip2_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Mip2_Output_ReadOnly !== null && (this.texture_Mip2_Output_ReadOnly_format != desiredFormat || this.texture_Mip2_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Mip2_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Mip2_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Mip2_Output_ReadOnly.destroy();
            this.texture_Mip2_Output_ReadOnly = null;
        }

        if (this.texture_Mip2_Output_ReadOnly === null)
        {
            this.texture_Mip2_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Mip2_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Mip2_Output_ReadOnly_format))
                viewFormats.push(this.texture_Mip2_Output_ReadOnly_format);

            this.texture_Mip2_Output_ReadOnly = device.createTexture({
                label: "texture Mips_CS_2DArray.Mip2_Output_ReadOnly",
                size: this.texture_Mip2_Output_ReadOnly_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Mip2_Output_ReadOnly_format),
                usage: this.texture_Mip2_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Mip3_Input_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Mip3_Input_ReadOnly !== null && (this.texture_Mip3_Input_ReadOnly_format != desiredFormat || this.texture_Mip3_Input_ReadOnly_size[0] != desiredSize[0] || this.texture_Mip3_Input_ReadOnly_size[1] != desiredSize[1] || this.texture_Mip3_Input_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Mip3_Input_ReadOnly.destroy();
            this.texture_Mip3_Input_ReadOnly = null;
        }

        if (this.texture_Mip3_Input_ReadOnly === null)
        {
            this.texture_Mip3_Input_ReadOnly_size = desiredSize.slice();
            this.texture_Mip3_Input_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Mip3_Input_ReadOnly_format))
                viewFormats.push(this.texture_Mip3_Input_ReadOnly_format);

            this.texture_Mip3_Input_ReadOnly = device.createTexture({
                label: "texture Mips_CS_2DArray.Mip3_Input_ReadOnly",
                size: this.texture_Mip3_Input_ReadOnly_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Mip3_Input_ReadOnly_format),
                usage: this.texture_Mip3_Input_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Mip3_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Mip3_Output_ReadOnly !== null && (this.texture_Mip3_Output_ReadOnly_format != desiredFormat || this.texture_Mip3_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Mip3_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Mip3_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Mip3_Output_ReadOnly.destroy();
            this.texture_Mip3_Output_ReadOnly = null;
        }

        if (this.texture_Mip3_Output_ReadOnly === null)
        {
            this.texture_Mip3_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Mip3_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Mip3_Output_ReadOnly_format))
                viewFormats.push(this.texture_Mip3_Output_ReadOnly_format);

            this.texture_Mip3_Output_ReadOnly = device.createTexture({
                label: "texture Mips_CS_2DArray.Mip3_Output_ReadOnly",
                size: this.texture_Mip3_Output_ReadOnly_size,
                mipLevelCount: 4,
                format: Shared.GetNonSRGBFormat(this.texture_Mip3_Output_ReadOnly_format),
                usage: this.texture_Mip3_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Load texture _loadedTexture_0 from "../cabinsmall.png"
    if (this.texture__loadedTexture_0 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/../cabinsmall.png", this.texture__loadedTexture_0_usageFlags, "2d");
        this.texture__loadedTexture_0 = loadedTex.texture;
        this.texture__loadedTexture_0_size = loadedTex.size;
        this.texture__loadedTexture_0_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_0_usageFlags = loadedTex.usageFlags;
    }
    // (Re)create compute shader Mip0
    {
        const bindGroupEntries =
        [
            {
                // Output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d-array" }
            },
            {
                // OutputReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Mip0_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
            {
                // _loadedTexture_0
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Mip0 === null || newHash !== this.Hash_Compute_Mip0)
        {
            this.Hash_Compute_Mip0 = newHash;

            let shaderCode = class_Mips_CS_2DArray.ShaderCode_Mip0_Mip0CS;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Mip0_Output_ReadOnly_format));

            this.ShaderModule_Compute_Mip0 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Mip0"});
            this.BindGroupLayout_Compute_Mip0 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Mip0",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Mip0 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Mip0",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Mip0],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Mip0 = device.createComputePipeline({
                    label: "Compute Pipeline Mip0",
                    layout: this.PipelineLayout_Compute_Mip0,
                    compute: {
                        module: this.ShaderModule_Compute_Mip0,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Mip0");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Mip0",
                    layout: this.PipelineLayout_Compute_Mip0,
                    compute: {
                        module: this.ShaderModule_Compute_Mip0,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Mip0 = handle; this.loadingPromises.delete("Mip0"); } );
            }
        }
    }

    // (Re)create compute shader Mip1
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d-array" }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d-array" }
            },
            {
                // InputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Mip1_Input_ReadOnly_format), viewDimension: "2d-array" }
            },
            {
                // OutputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Mip1_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Mip1 === null || newHash !== this.Hash_Compute_Mip1)
        {
            this.Hash_Compute_Mip1 = newHash;

            let shaderCode = class_Mips_CS_2DArray.ShaderCode_Mip1_MipNCS;
            shaderCode = shaderCode.replace("/*(Input_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(InputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Mip1_Input_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Mip1_Output_ReadOnly_format));

            this.ShaderModule_Compute_Mip1 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Mip1"});
            this.BindGroupLayout_Compute_Mip1 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Mip1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Mip1 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Mip1",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Mip1],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Mip1 = device.createComputePipeline({
                    label: "Compute Pipeline Mip1",
                    layout: this.PipelineLayout_Compute_Mip1,
                    compute: {
                        module: this.ShaderModule_Compute_Mip1,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Mip1");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Mip1",
                    layout: this.PipelineLayout_Compute_Mip1,
                    compute: {
                        module: this.ShaderModule_Compute_Mip1,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Mip1 = handle; this.loadingPromises.delete("Mip1"); } );
            }
        }
    }

    // (Re)create compute shader Mip2
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d-array" }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d-array" }
            },
            {
                // InputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Mip2_Input_ReadOnly_format), viewDimension: "2d-array" }
            },
            {
                // OutputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Mip2_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Mip2 === null || newHash !== this.Hash_Compute_Mip2)
        {
            this.Hash_Compute_Mip2 = newHash;

            let shaderCode = class_Mips_CS_2DArray.ShaderCode_Mip2_MipNCS;
            shaderCode = shaderCode.replace("/*(Input_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(InputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Mip2_Input_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Mip2_Output_ReadOnly_format));

            this.ShaderModule_Compute_Mip2 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Mip2"});
            this.BindGroupLayout_Compute_Mip2 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Mip2",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Mip2 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Mip2",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Mip2],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Mip2 = device.createComputePipeline({
                    label: "Compute Pipeline Mip2",
                    layout: this.PipelineLayout_Compute_Mip2,
                    compute: {
                        module: this.ShaderModule_Compute_Mip2,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Mip2");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Mip2",
                    layout: this.PipelineLayout_Compute_Mip2,
                    compute: {
                        module: this.ShaderModule_Compute_Mip2,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Mip2 = handle; this.loadingPromises.delete("Mip2"); } );
            }
        }
    }

    // (Re)create compute shader Mip3
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d-array" }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d-array" }
            },
            {
                // InputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Mip3_Input_ReadOnly_format), viewDimension: "2d-array" }
            },
            {
                // OutputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Mip3_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Mip3 === null || newHash !== this.Hash_Compute_Mip3)
        {
            this.Hash_Compute_Mip3 = newHash;

            let shaderCode = class_Mips_CS_2DArray.ShaderCode_Mip3_MipNCS;
            shaderCode = shaderCode.replace("/*(Input_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(InputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Mip3_Input_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Mip3_Output_ReadOnly_format));

            this.ShaderModule_Compute_Mip3 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Mip3"});
            this.BindGroupLayout_Compute_Mip3 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Mip3",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Mip3 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Mip3",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Mip3],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Mip3 = device.createComputePipeline({
                    label: "Compute Pipeline Mip3",
                    layout: this.PipelineLayout_Compute_Mip3,
                    compute: {
                        module: this.ShaderModule_Compute_Mip3,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Mip3");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Mip3",
                    layout: this.PipelineLayout_Compute_Mip3,
                    compute: {
                        module: this.ShaderModule_Compute_Mip3,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Mip3 = handle; this.loadingPromises.delete("Mip3"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("Mips_CS_2DArray.Output");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Output"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip0_Output_ReadOnly");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip0_Output_ReadOnly"

    encoder.pushDebugGroup("Mips_CS_2DArray.Copy_Mip0_Output");

    // Copy texture Output to texture Mip0_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Mip0_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Mip0_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Copy_Mip0_Output"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip1_Input_ReadOnly");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip1_Input_ReadOnly"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip1_Output_ReadOnly");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip1_Output_ReadOnly"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip2_Input_ReadOnly");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip2_Input_ReadOnly"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip2_Output_ReadOnly");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip2_Output_ReadOnly"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip3_Input_ReadOnly");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip3_Input_ReadOnly"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip3_Output_ReadOnly");

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip3_Output_ReadOnly"

    encoder.pushDebugGroup("Mips_CS_2DArray._loadedTexture_0");

    encoder.popDebugGroup(); // "Mips_CS_2DArray._loadedTexture_0"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip0");

    // Run compute shader Mip0
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Mip0",
            layout: this.BindGroupLayout_Compute_Mip0,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 1,
                    resource: this.texture_Mip0_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 2,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
            ]
        });

        // Calculate dispatch size

        const baseDispatchSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1 ];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Mip0 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Mip0);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip0"

    encoder.pushDebugGroup("Mips_CS_2DArray.Copy_Mip1_Input");

    // Copy texture Output to texture Mip1_Input_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Mip1_Input_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Mip1_Input_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Copy_Mip1_Input"

    encoder.pushDebugGroup("Mips_CS_2DArray.Copy_Mip1_Output");

    // Copy texture Output to texture Mip1_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Mip1_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Mip1_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Copy_Mip1_Output"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip1");

    // Run compute shader Mip1
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Mip1",
            layout: this.BindGroupLayout_Compute_Mip1,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 1 })
                },
                {
                    // InputReadOnly
                    binding: 2,
                    resource: this.texture_Mip1_Input_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 3,
                    resource: this.texture_Mip1_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 1 })
                },
            ]
        });

        // Calculate dispatch size

        const baseDispatchSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1 ];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 2) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 2) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Mip1 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Mip1);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip1"

    encoder.pushDebugGroup("Mips_CS_2DArray.Copy_Mip2_Input");

    // Copy texture Output to texture Mip2_Input_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Mip2_Input_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Mip2_Input_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Copy_Mip2_Input"

    encoder.pushDebugGroup("Mips_CS_2DArray.Copy_Mip2_Output");

    // Copy texture Output to texture Mip2_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Mip2_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Mip2_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Copy_Mip2_Output"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip2");

    // Run compute shader Mip2
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Mip2",
            layout: this.BindGroupLayout_Compute_Mip2,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 1 })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 2 })
                },
                {
                    // InputReadOnly
                    binding: 2,
                    resource: this.texture_Mip2_Input_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 1 })
                },
                {
                    // OutputReadOnly
                    binding: 3,
                    resource: this.texture_Mip2_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 2 })
                },
            ]
        });

        // Calculate dispatch size

        const baseDispatchSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1 ];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 4) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 4) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Mip2 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Mip2);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip2"

    encoder.pushDebugGroup("Mips_CS_2DArray.Copy_Mip3_Input");

    // Copy texture Output to texture Mip3_Input_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Mip3_Input_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Mip3_Input_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Copy_Mip3_Input"

    encoder.pushDebugGroup("Mips_CS_2DArray.Copy_Mip3_Output");

    // Copy texture Output to texture Mip3_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Mip3_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Mip3_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Copy_Mip3_Output"

    encoder.pushDebugGroup("Mips_CS_2DArray.Mip3");

    // Run compute shader Mip3
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Mip3",
            layout: this.BindGroupLayout_Compute_Mip3,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 2 })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 3 })
                },
                {
                    // InputReadOnly
                    binding: 2,
                    resource: this.texture_Mip3_Input_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 2 })
                },
                {
                    // OutputReadOnly
                    binding: 3,
                    resource: this.texture_Mip3_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 3 })
                },
            ]
        });

        // Calculate dispatch size

        const baseDispatchSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1 ];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 8) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 8) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Mip3 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Mip3);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "Mips_CS_2DArray.Mip3"

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

var Mips_CS_2DArray = new class_Mips_CS_2DArray;

export default Mips_CS_2DArray;
