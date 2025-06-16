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

class class_SubGraphLoops
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "FilterSub_Iteration_0_Blur", node "FilterSub_Iteration_0_DoBlur"
static ShaderCode_FilterSub_Iteration_0_DoBlur_FilterSub_Iteration_0_Blur = `
@binding(0) @group(0) var Input : texture_2d<f32>;

struct Struct_FilterSub_Iteration_0_BlurCB_std140_0
{
    @align(16) FilterSub_Iteration_0_sRGB_0 : u32,
    @align(4) _loopIndexValue_0_0 : i32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _FilterSub_Iteration_0_BlurCB : Struct_FilterSub_Iteration_0_BlurCB_std140_0;
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

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
    var _S2 : vec2<i32> = vec2<i32>(DTid_0.xy);
    var dims_0 : vec2<i32>;
    var _S3 : i32 = dims_0[i32(0)];
    var _S4 : i32 = dims_0[i32(1)];
    {var dim = textureDimensions((Input));((_S3)) = bitcast<i32>(dim.x);((_S4)) = bitcast<i32>(dim.y);};
    dims_0[i32(0)] = _S3;
    dims_0[i32(1)] = _S4;
    var _S5 : i32 = _FilterSub_Iteration_0_BlurCB._loopIndexValue_0_0 + i32(1);
    const _S6 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var iy_0 : i32 = i32(-1);
    var ret_0 : vec3<f32> = _S6;
    for(;;)
    {
        if(iy_0 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var ix_0 : i32 = i32(-1);
        for(;;)
        {
            if(ix_0 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var readpx_0 : vec2<i32> = (_S2 + vec2<i32>(ix_0, iy_0) * vec2<i32>(_S5) + dims_0) % dims_0;
            var _S7 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(readpx_0), u32(0)));
            var ret_1 : vec3<f32> = ret_0 + (textureLoad((Input), ((_S7)).xy, ((_S7)).z)).xyz;
            ix_0 = ix_0 + i32(1);
            ret_0 = ret_1;
        }
        iy_0 = iy_0 + i32(1);
    }
    var ret_2 : vec3<f32> = ret_0 / vec3<f32>(9.0f);
    if(bool(_FilterSub_Iteration_0_BlurCB.FilterSub_Iteration_0_sRGB_0))
    {
        ret_0 = LinearToSRGB_0(ret_2);
    }
    else
    {
        ret_0 = ret_2;
    }
    textureStore((Output), (vec2<u32>(_S2)), (vec4<f32>(ret_0, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "FilterSub_Iteration_1_Blur", node "FilterSub_Iteration_1_DoBlur"
static ShaderCode_FilterSub_Iteration_1_DoBlur_FilterSub_Iteration_1_Blur = `
@binding(0) @group(0) var Input : texture_2d<f32>;

struct Struct_FilterSub_Iteration_1_BlurCB_std140_0
{
    @align(16) FilterSub_Iteration_1_sRGB_0 : u32,
    @align(4) _loopIndexValue_1_0 : i32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _FilterSub_Iteration_1_BlurCB : Struct_FilterSub_Iteration_1_BlurCB_std140_0;
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

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
    var _S2 : vec2<i32> = vec2<i32>(DTid_0.xy);
    var dims_0 : vec2<i32>;
    var _S3 : i32 = dims_0[i32(0)];
    var _S4 : i32 = dims_0[i32(1)];
    {var dim = textureDimensions((Input));((_S3)) = bitcast<i32>(dim.x);((_S4)) = bitcast<i32>(dim.y);};
    dims_0[i32(0)] = _S3;
    dims_0[i32(1)] = _S4;
    var _S5 : i32 = _FilterSub_Iteration_1_BlurCB._loopIndexValue_1_0 + i32(1);
    const _S6 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var iy_0 : i32 = i32(-1);
    var ret_0 : vec3<f32> = _S6;
    for(;;)
    {
        if(iy_0 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var ix_0 : i32 = i32(-1);
        for(;;)
        {
            if(ix_0 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var readpx_0 : vec2<i32> = (_S2 + vec2<i32>(ix_0, iy_0) * vec2<i32>(_S5) + dims_0) % dims_0;
            var _S7 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(readpx_0), u32(0)));
            var ret_1 : vec3<f32> = ret_0 + (textureLoad((Input), ((_S7)).xy, ((_S7)).z)).xyz;
            ix_0 = ix_0 + i32(1);
            ret_0 = ret_1;
        }
        iy_0 = iy_0 + i32(1);
    }
    var ret_2 : vec3<f32> = ret_0 / vec3<f32>(9.0f);
    if(bool(_FilterSub_Iteration_1_BlurCB.FilterSub_Iteration_1_sRGB_0))
    {
        ret_0 = LinearToSRGB_0(ret_2);
    }
    else
    {
        ret_0 = ret_2;
    }
    textureStore((Output), (vec2<u32>(_S2)), (vec4<f32>(ret_0, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "FilterSub_Iteration_2_Blur", node "FilterSub_Iteration_2_DoBlur"
static ShaderCode_FilterSub_Iteration_2_DoBlur_FilterSub_Iteration_2_Blur = `
@binding(0) @group(0) var Input : texture_2d<f32>;

struct Struct_FilterSub_Iteration_2_BlurCB_std140_0
{
    @align(16) FilterSub_Iteration_2_sRGB_0 : u32,
    @align(4) _loopIndexValue_2_0 : i32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _FilterSub_Iteration_2_BlurCB : Struct_FilterSub_Iteration_2_BlurCB_std140_0;
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

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
    var _S2 : vec2<i32> = vec2<i32>(DTid_0.xy);
    var dims_0 : vec2<i32>;
    var _S3 : i32 = dims_0[i32(0)];
    var _S4 : i32 = dims_0[i32(1)];
    {var dim = textureDimensions((Input));((_S3)) = bitcast<i32>(dim.x);((_S4)) = bitcast<i32>(dim.y);};
    dims_0[i32(0)] = _S3;
    dims_0[i32(1)] = _S4;
    var _S5 : i32 = _FilterSub_Iteration_2_BlurCB._loopIndexValue_2_0 + i32(1);
    const _S6 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var iy_0 : i32 = i32(-1);
    var ret_0 : vec3<f32> = _S6;
    for(;;)
    {
        if(iy_0 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var ix_0 : i32 = i32(-1);
        for(;;)
        {
            if(ix_0 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var readpx_0 : vec2<i32> = (_S2 + vec2<i32>(ix_0, iy_0) * vec2<i32>(_S5) + dims_0) % dims_0;
            var _S7 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(readpx_0), u32(0)));
            var ret_1 : vec3<f32> = ret_0 + (textureLoad((Input), ((_S7)).xy, ((_S7)).z)).xyz;
            ix_0 = ix_0 + i32(1);
            ret_0 = ret_1;
        }
        iy_0 = iy_0 + i32(1);
    }
    var ret_2 : vec3<f32> = ret_0 / vec3<f32>(9.0f);
    if(bool(_FilterSub_Iteration_2_BlurCB.FilterSub_Iteration_2_sRGB_0))
    {
        ret_0 = LinearToSRGB_0(ret_2);
    }
    else
    {
        ret_0 = ret_2;
    }
    textureStore((Output), (vec2<u32>(_S2)), (vec4<f32>(ret_0, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "FilterSub_Iteration_3_Blur", node "FilterSub_Iteration_3_DoBlur"
static ShaderCode_FilterSub_Iteration_3_DoBlur_FilterSub_Iteration_3_Blur = `
@binding(0) @group(0) var Input : texture_2d<f32>;

struct Struct_FilterSub_Iteration_3_BlurCB_std140_0
{
    @align(16) FilterSub_Iteration_3_sRGB_0 : u32,
    @align(4) _loopIndexValue_3_0 : i32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _FilterSub_Iteration_3_BlurCB : Struct_FilterSub_Iteration_3_BlurCB_std140_0;
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

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
    var _S2 : vec2<i32> = vec2<i32>(DTid_0.xy);
    var dims_0 : vec2<i32>;
    var _S3 : i32 = dims_0[i32(0)];
    var _S4 : i32 = dims_0[i32(1)];
    {var dim = textureDimensions((Input));((_S3)) = bitcast<i32>(dim.x);((_S4)) = bitcast<i32>(dim.y);};
    dims_0[i32(0)] = _S3;
    dims_0[i32(1)] = _S4;
    var _S5 : i32 = _FilterSub_Iteration_3_BlurCB._loopIndexValue_3_0 + i32(1);
    const _S6 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var iy_0 : i32 = i32(-1);
    var ret_0 : vec3<f32> = _S6;
    for(;;)
    {
        if(iy_0 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var ix_0 : i32 = i32(-1);
        for(;;)
        {
            if(ix_0 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var readpx_0 : vec2<i32> = (_S2 + vec2<i32>(ix_0, iy_0) * vec2<i32>(_S5) + dims_0) % dims_0;
            var _S7 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(readpx_0), u32(0)));
            var ret_1 : vec3<f32> = ret_0 + (textureLoad((Input), ((_S7)).xy, ((_S7)).z)).xyz;
            ix_0 = ix_0 + i32(1);
            ret_0 = ret_1;
        }
        iy_0 = iy_0 + i32(1);
    }
    var ret_2 : vec3<f32> = ret_0 / vec3<f32>(9.0f);
    if(bool(_FilterSub_Iteration_3_BlurCB.FilterSub_Iteration_3_sRGB_0))
    {
        ret_0 = LinearToSRGB_0(ret_2);
    }
    else
    {
        ret_0 = ret_2;
    }
    textureStore((Output), (vec2<u32>(_S2)), (vec4<f32>(ret_0, 1.0f)));
    return;
}

`;

// Shader code for Compute shader "FilterSub_Iteration_4_Blur", node "FilterSub_Iteration_4_DoBlur"
static ShaderCode_FilterSub_Iteration_4_DoBlur_FilterSub_Iteration_4_Blur = `
@binding(0) @group(0) var Input : texture_2d<f32>;

struct Struct_FilterSub_Iteration_4_BlurCB_std140_0
{
    @align(16) FilterSub_Iteration_4_sRGB_0 : u32,
    @align(4) _loopIndexValue_4_0 : i32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _FilterSub_Iteration_4_BlurCB : Struct_FilterSub_Iteration_4_BlurCB_std140_0;
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

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
    var _S2 : vec2<i32> = vec2<i32>(DTid_0.xy);
    var dims_0 : vec2<i32>;
    var _S3 : i32 = dims_0[i32(0)];
    var _S4 : i32 = dims_0[i32(1)];
    {var dim = textureDimensions((Input));((_S3)) = bitcast<i32>(dim.x);((_S4)) = bitcast<i32>(dim.y);};
    dims_0[i32(0)] = _S3;
    dims_0[i32(1)] = _S4;
    var _S5 : i32 = _FilterSub_Iteration_4_BlurCB._loopIndexValue_4_0 + i32(1);
    const _S6 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var iy_0 : i32 = i32(-1);
    var ret_0 : vec3<f32> = _S6;
    for(;;)
    {
        if(iy_0 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var ix_0 : i32 = i32(-1);
        for(;;)
        {
            if(ix_0 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var readpx_0 : vec2<i32> = (_S2 + vec2<i32>(ix_0, iy_0) * vec2<i32>(_S5) + dims_0) % dims_0;
            var _S7 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(readpx_0), u32(0)));
            var ret_1 : vec3<f32> = ret_0 + (textureLoad((Input), ((_S7)).xy, ((_S7)).z)).xyz;
            ix_0 = ix_0 + i32(1);
            ret_0 = ret_1;
        }
        iy_0 = iy_0 + i32(1);
    }
    var ret_2 : vec3<f32> = ret_0 / vec3<f32>(9.0f);
    if(bool(_FilterSub_Iteration_4_BlurCB.FilterSub_Iteration_4_sRGB_0))
    {
        ret_0 = LinearToSRGB_0(ret_2);
    }
    else
    {
        ret_0 = ret_2;
    }
    textureStore((Output), (vec2<u32>(_S2)), (vec4<f32>(ret_0, 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture FilterSub_Iteration_0_DoBlur_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly = null;
texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_size = [0, 0, 0];
texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format = "";
texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_1_DoBlur_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly = null;
texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_size = [0, 0, 0];
texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format = "";
texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_2_DoBlur_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly = null;
texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_size = [0, 0, 0];
texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format = "";
texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_3_DoBlur_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly = null;
texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_size = [0, 0, 0];
texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format = "";
texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_4_DoBlur_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly = null;
texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_size = [0, 0, 0];
texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format = "";
texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _FilterSub_Iteration_0_BlurCB
constantBuffer__FilterSub_Iteration_0_BlurCB = null;
constantBuffer__FilterSub_Iteration_0_BlurCB_size = 16;
constantBuffer__FilterSub_Iteration_0_BlurCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader FilterSub_Iteration_0_DoBlur
Hash_Compute_FilterSub_Iteration_0_DoBlur = 0;
ShaderModule_Compute_FilterSub_Iteration_0_DoBlur = null;
BindGroupLayout_Compute_FilterSub_Iteration_0_DoBlur = null;
PipelineLayout_Compute_FilterSub_Iteration_0_DoBlur = null;
Pipeline_Compute_FilterSub_Iteration_0_DoBlur = null;

// Constant buffer _FilterSub_Iteration_1_BlurCB
constantBuffer__FilterSub_Iteration_1_BlurCB = null;
constantBuffer__FilterSub_Iteration_1_BlurCB_size = 16;
constantBuffer__FilterSub_Iteration_1_BlurCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader FilterSub_Iteration_1_DoBlur
Hash_Compute_FilterSub_Iteration_1_DoBlur = 0;
ShaderModule_Compute_FilterSub_Iteration_1_DoBlur = null;
BindGroupLayout_Compute_FilterSub_Iteration_1_DoBlur = null;
PipelineLayout_Compute_FilterSub_Iteration_1_DoBlur = null;
Pipeline_Compute_FilterSub_Iteration_1_DoBlur = null;

// Constant buffer _FilterSub_Iteration_2_BlurCB
constantBuffer__FilterSub_Iteration_2_BlurCB = null;
constantBuffer__FilterSub_Iteration_2_BlurCB_size = 16;
constantBuffer__FilterSub_Iteration_2_BlurCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader FilterSub_Iteration_2_DoBlur
Hash_Compute_FilterSub_Iteration_2_DoBlur = 0;
ShaderModule_Compute_FilterSub_Iteration_2_DoBlur = null;
BindGroupLayout_Compute_FilterSub_Iteration_2_DoBlur = null;
PipelineLayout_Compute_FilterSub_Iteration_2_DoBlur = null;
Pipeline_Compute_FilterSub_Iteration_2_DoBlur = null;

// Constant buffer _FilterSub_Iteration_3_BlurCB
constantBuffer__FilterSub_Iteration_3_BlurCB = null;
constantBuffer__FilterSub_Iteration_3_BlurCB_size = 16;
constantBuffer__FilterSub_Iteration_3_BlurCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader FilterSub_Iteration_3_DoBlur
Hash_Compute_FilterSub_Iteration_3_DoBlur = 0;
ShaderModule_Compute_FilterSub_Iteration_3_DoBlur = null;
BindGroupLayout_Compute_FilterSub_Iteration_3_DoBlur = null;
PipelineLayout_Compute_FilterSub_Iteration_3_DoBlur = null;
Pipeline_Compute_FilterSub_Iteration_3_DoBlur = null;

// Constant buffer _FilterSub_Iteration_4_BlurCB
constantBuffer__FilterSub_Iteration_4_BlurCB = null;
constantBuffer__FilterSub_Iteration_4_BlurCB_size = 16;
constantBuffer__FilterSub_Iteration_4_BlurCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader FilterSub_Iteration_4_DoBlur
Hash_Compute_FilterSub_Iteration_4_DoBlur = 0;
ShaderModule_Compute_FilterSub_Iteration_4_DoBlur = null;
BindGroupLayout_Compute_FilterSub_Iteration_4_DoBlur = null;
PipelineLayout_Compute_FilterSub_Iteration_4_DoBlur = null;
Pipeline_Compute_FilterSub_Iteration_4_DoBlur = null;

// -------------------- Imported Members

// Texture Input
texture_Input = null;
texture_Input_size = [0, 0, 0];
texture_Input_format = "";
texture_Input_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// -------------------- Exported Members

// Texture FilterSub_Iteration_0_Output
texture_FilterSub_Iteration_0_Output = null;
texture_FilterSub_Iteration_0_Output_size = [0, 0, 0];
texture_FilterSub_Iteration_0_Output_format = "";
texture_FilterSub_Iteration_0_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_1_Output
texture_FilterSub_Iteration_1_Output = null;
texture_FilterSub_Iteration_1_Output_size = [0, 0, 0];
texture_FilterSub_Iteration_1_Output_format = "";
texture_FilterSub_Iteration_1_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_2_Output
texture_FilterSub_Iteration_2_Output = null;
texture_FilterSub_Iteration_2_Output_size = [0, 0, 0];
texture_FilterSub_Iteration_2_Output_format = "";
texture_FilterSub_Iteration_2_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_3_Output
texture_FilterSub_Iteration_3_Output = null;
texture_FilterSub_Iteration_3_Output_size = [0, 0, 0];
texture_FilterSub_Iteration_3_Output_format = "";
texture_FilterSub_Iteration_3_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture FilterSub_Iteration_4_Output
texture_FilterSub_Iteration_4_Output = null;
texture_FilterSub_Iteration_4_Output_size = [0, 0, 0];
texture_FilterSub_Iteration_4_Output_format = "";
texture_FilterSub_Iteration_4_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_FilterSub_Iteration_0_LoopIndex = 0;
variableDefault_FilterSub_Iteration_0_LoopIndex = 0;
variableChanged_FilterSub_Iteration_0_LoopIndex = false;
variable_FilterSub_Iteration_0_sRGB = true;
variableDefault_FilterSub_Iteration_0_sRGB = true;
variableChanged_FilterSub_Iteration_0_sRGB = false;
variable_FilterSub_Iteration_1_LoopIndex = 0;
variableDefault_FilterSub_Iteration_1_LoopIndex = 0;
variableChanged_FilterSub_Iteration_1_LoopIndex = false;
variable_FilterSub_Iteration_1_sRGB = true;
variableDefault_FilterSub_Iteration_1_sRGB = true;
variableChanged_FilterSub_Iteration_1_sRGB = false;
variable_FilterSub_Iteration_2_LoopIndex = 0;
variableDefault_FilterSub_Iteration_2_LoopIndex = 0;
variableChanged_FilterSub_Iteration_2_LoopIndex = false;
variable_FilterSub_Iteration_2_sRGB = true;
variableDefault_FilterSub_Iteration_2_sRGB = true;
variableChanged_FilterSub_Iteration_2_sRGB = false;
variable_FilterSub_Iteration_3_LoopIndex = 0;
variableDefault_FilterSub_Iteration_3_LoopIndex = 0;
variableChanged_FilterSub_Iteration_3_LoopIndex = false;
variable_FilterSub_Iteration_3_sRGB = true;
variableDefault_FilterSub_Iteration_3_sRGB = true;
variableChanged_FilterSub_Iteration_3_sRGB = false;
variable_FilterSub_Iteration_4_LoopIndex = 0;
variableDefault_FilterSub_Iteration_4_LoopIndex = 0;
variableChanged_FilterSub_Iteration_4_LoopIndex = false;
variable_FilterSub_Iteration_4_sRGB = true;
variableDefault_FilterSub_Iteration_4_sRGB = true;
variableChanged_FilterSub_Iteration_4_sRGB = false;

// -------------------- Private Variables

variable___loopIndexValue_0 = 0;
variableDefault___loopIndexValue_0 = 0;
variableChanged___loopIndexValue_0 = false;
variable___loopIndexValue_1 = 1;
variableDefault___loopIndexValue_1 = 1;
variableChanged___loopIndexValue_1 = false;
variable___loopIndexValue_2 = 2;
variableDefault___loopIndexValue_2 = 2;
variableChanged___loopIndexValue_2 = false;
variable___loopIndexValue_3 = 3;
variableDefault___loopIndexValue_3 = 3;
variableChanged___loopIndexValue_3 = false;
variable___loopIndexValue_4 = 4;
variableDefault___loopIndexValue_4 = 4;
variableChanged___loopIndexValue_4 = false;
// -------------------- Structs

static StructOffsets__FilterSub_Iteration_0_BlurCB =
{
    FilterSub_Iteration_0_sRGB: 0,
    __loopIndexValue_0: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__FilterSub_Iteration_1_BlurCB =
{
    FilterSub_Iteration_1_sRGB: 0,
    __loopIndexValue_1: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__FilterSub_Iteration_2_BlurCB =
{
    FilterSub_Iteration_2_sRGB: 0,
    __loopIndexValue_2: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__FilterSub_Iteration_3_BlurCB =
{
    FilterSub_Iteration_3_sRGB: 0,
    __loopIndexValue_3: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__FilterSub_Iteration_4_BlurCB =
{
    FilterSub_Iteration_4_sRGB: 0,
    __loopIndexValue_4: 4,
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
    // Validate texture Input
    if (this.texture_Input === null)
    {
        Shared.LogError("Imported resource texture_Input was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture FilterSub_Iteration_0_Output
    {
        const baseSize = this.texture_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Input_format;
        if (this.texture_FilterSub_Iteration_0_Output !== null && (this.texture_FilterSub_Iteration_0_Output_format != desiredFormat || this.texture_FilterSub_Iteration_0_Output_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_0_Output_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_0_Output_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_0_Output.destroy();
            this.texture_FilterSub_Iteration_0_Output = null;
        }

        if (this.texture_FilterSub_Iteration_0_Output === null)
        {
            this.texture_FilterSub_Iteration_0_Output_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_0_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_0_Output_format))
                viewFormats.push(this.texture_FilterSub_Iteration_0_Output_format);

            this.texture_FilterSub_Iteration_0_Output = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_0_Output",
                size: this.texture_FilterSub_Iteration_0_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_0_Output_format),
                usage: this.texture_FilterSub_Iteration_0_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_1_Output
    {
        const baseSize = this.texture_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Input_format;
        if (this.texture_FilterSub_Iteration_1_Output !== null && (this.texture_FilterSub_Iteration_1_Output_format != desiredFormat || this.texture_FilterSub_Iteration_1_Output_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_1_Output_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_1_Output_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_1_Output.destroy();
            this.texture_FilterSub_Iteration_1_Output = null;
        }

        if (this.texture_FilterSub_Iteration_1_Output === null)
        {
            this.texture_FilterSub_Iteration_1_Output_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_1_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_1_Output_format))
                viewFormats.push(this.texture_FilterSub_Iteration_1_Output_format);

            this.texture_FilterSub_Iteration_1_Output = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_1_Output",
                size: this.texture_FilterSub_Iteration_1_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_1_Output_format),
                usage: this.texture_FilterSub_Iteration_1_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_2_Output
    {
        const baseSize = this.texture_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Input_format;
        if (this.texture_FilterSub_Iteration_2_Output !== null && (this.texture_FilterSub_Iteration_2_Output_format != desiredFormat || this.texture_FilterSub_Iteration_2_Output_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_2_Output_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_2_Output_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_2_Output.destroy();
            this.texture_FilterSub_Iteration_2_Output = null;
        }

        if (this.texture_FilterSub_Iteration_2_Output === null)
        {
            this.texture_FilterSub_Iteration_2_Output_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_2_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_2_Output_format))
                viewFormats.push(this.texture_FilterSub_Iteration_2_Output_format);

            this.texture_FilterSub_Iteration_2_Output = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_2_Output",
                size: this.texture_FilterSub_Iteration_2_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_2_Output_format),
                usage: this.texture_FilterSub_Iteration_2_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_3_Output
    {
        const baseSize = this.texture_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Input_format;
        if (this.texture_FilterSub_Iteration_3_Output !== null && (this.texture_FilterSub_Iteration_3_Output_format != desiredFormat || this.texture_FilterSub_Iteration_3_Output_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_3_Output_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_3_Output_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_3_Output.destroy();
            this.texture_FilterSub_Iteration_3_Output = null;
        }

        if (this.texture_FilterSub_Iteration_3_Output === null)
        {
            this.texture_FilterSub_Iteration_3_Output_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_3_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_3_Output_format))
                viewFormats.push(this.texture_FilterSub_Iteration_3_Output_format);

            this.texture_FilterSub_Iteration_3_Output = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_3_Output",
                size: this.texture_FilterSub_Iteration_3_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_3_Output_format),
                usage: this.texture_FilterSub_Iteration_3_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_4_Output
    {
        const baseSize = this.texture_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Input_format;
        if (this.texture_FilterSub_Iteration_4_Output !== null && (this.texture_FilterSub_Iteration_4_Output_format != desiredFormat || this.texture_FilterSub_Iteration_4_Output_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_4_Output_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_4_Output_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_4_Output.destroy();
            this.texture_FilterSub_Iteration_4_Output = null;
        }

        if (this.texture_FilterSub_Iteration_4_Output === null)
        {
            this.texture_FilterSub_Iteration_4_Output_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_4_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_4_Output_format))
                viewFormats.push(this.texture_FilterSub_Iteration_4_Output_format);

            this.texture_FilterSub_Iteration_4_Output = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_4_Output",
                size: this.texture_FilterSub_Iteration_4_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_4_Output_format),
                usage: this.texture_FilterSub_Iteration_4_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_0_DoBlur_Output_ReadOnly
    {
        const baseSize = this.texture_FilterSub_Iteration_0_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_FilterSub_Iteration_0_Output_format;
        if (this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly !== null && (this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format != desiredFormat || this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly.destroy();
            this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly = null;
        }

        if (this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly === null)
        {
            this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format))
                viewFormats.push(this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format);

            this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_0_DoBlur_Output_ReadOnly",
                size: this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format),
                usage: this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_1_DoBlur_Output_ReadOnly
    {
        const baseSize = this.texture_FilterSub_Iteration_1_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_FilterSub_Iteration_1_Output_format;
        if (this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly !== null && (this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format != desiredFormat || this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly.destroy();
            this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly = null;
        }

        if (this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly === null)
        {
            this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format))
                viewFormats.push(this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format);

            this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_1_DoBlur_Output_ReadOnly",
                size: this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format),
                usage: this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_2_DoBlur_Output_ReadOnly
    {
        const baseSize = this.texture_FilterSub_Iteration_2_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_FilterSub_Iteration_2_Output_format;
        if (this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly !== null && (this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format != desiredFormat || this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly.destroy();
            this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly = null;
        }

        if (this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly === null)
        {
            this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format))
                viewFormats.push(this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format);

            this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_2_DoBlur_Output_ReadOnly",
                size: this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format),
                usage: this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_3_DoBlur_Output_ReadOnly
    {
        const baseSize = this.texture_FilterSub_Iteration_3_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_FilterSub_Iteration_3_Output_format;
        if (this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly !== null && (this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format != desiredFormat || this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly.destroy();
            this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly = null;
        }

        if (this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly === null)
        {
            this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format))
                viewFormats.push(this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format);

            this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_3_DoBlur_Output_ReadOnly",
                size: this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format),
                usage: this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture FilterSub_Iteration_4_DoBlur_Output_ReadOnly
    {
        const baseSize = this.texture_FilterSub_Iteration_4_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_FilterSub_Iteration_4_Output_format;
        if (this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly !== null && (this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format != desiredFormat || this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly.destroy();
            this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly = null;
        }

        if (this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly === null)
        {
            this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_size = desiredSize.slice();
            this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format))
                viewFormats.push(this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format);

            this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly = device.createTexture({
                label: "texture SubGraphLoops.FilterSub_Iteration_4_DoBlur_Output_ReadOnly",
                size: this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format),
                usage: this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader FilterSub_Iteration_0_DoBlur
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Input_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_0_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _FilterSub_Iteration_0_BlurCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_FilterSub_Iteration_0_DoBlur === null || newHash !== this.Hash_Compute_FilterSub_Iteration_0_DoBlur)
        {
            this.Hash_Compute_FilterSub_Iteration_0_DoBlur = newHash;

            let shaderCode = class_SubGraphLoops.ShaderCode_FilterSub_Iteration_0_DoBlur_FilterSub_Iteration_0_Blur;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_0_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly_format));

            this.ShaderModule_Compute_FilterSub_Iteration_0_DoBlur = device.createShaderModule({ code: shaderCode, label: "Compute Shader FilterSub_Iteration_0_DoBlur"});
            this.BindGroupLayout_Compute_FilterSub_Iteration_0_DoBlur = device.createBindGroupLayout({
                label: "Compute Bind Group Layout FilterSub_Iteration_0_DoBlur",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_FilterSub_Iteration_0_DoBlur = device.createPipelineLayout({
                label: "Compute Pipeline Layout FilterSub_Iteration_0_DoBlur",
                bindGroupLayouts: [this.BindGroupLayout_Compute_FilterSub_Iteration_0_DoBlur],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_FilterSub_Iteration_0_DoBlur = device.createComputePipeline({
                    label: "Compute Pipeline FilterSub_Iteration_0_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_0_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_0_DoBlur,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("FilterSub_Iteration_0_DoBlur");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline FilterSub_Iteration_0_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_0_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_0_DoBlur,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_FilterSub_Iteration_0_DoBlur = handle; this.loadingPromises.delete("FilterSub_Iteration_0_DoBlur"); } );
            }
        }
    }

    // (Re)create compute shader FilterSub_Iteration_1_DoBlur
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Input_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_1_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _FilterSub_Iteration_1_BlurCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_FilterSub_Iteration_1_DoBlur === null || newHash !== this.Hash_Compute_FilterSub_Iteration_1_DoBlur)
        {
            this.Hash_Compute_FilterSub_Iteration_1_DoBlur = newHash;

            let shaderCode = class_SubGraphLoops.ShaderCode_FilterSub_Iteration_1_DoBlur_FilterSub_Iteration_1_Blur;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_1_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly_format));

            this.ShaderModule_Compute_FilterSub_Iteration_1_DoBlur = device.createShaderModule({ code: shaderCode, label: "Compute Shader FilterSub_Iteration_1_DoBlur"});
            this.BindGroupLayout_Compute_FilterSub_Iteration_1_DoBlur = device.createBindGroupLayout({
                label: "Compute Bind Group Layout FilterSub_Iteration_1_DoBlur",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_FilterSub_Iteration_1_DoBlur = device.createPipelineLayout({
                label: "Compute Pipeline Layout FilterSub_Iteration_1_DoBlur",
                bindGroupLayouts: [this.BindGroupLayout_Compute_FilterSub_Iteration_1_DoBlur],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_FilterSub_Iteration_1_DoBlur = device.createComputePipeline({
                    label: "Compute Pipeline FilterSub_Iteration_1_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_1_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_1_DoBlur,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("FilterSub_Iteration_1_DoBlur");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline FilterSub_Iteration_1_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_1_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_1_DoBlur,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_FilterSub_Iteration_1_DoBlur = handle; this.loadingPromises.delete("FilterSub_Iteration_1_DoBlur"); } );
            }
        }
    }

    // (Re)create compute shader FilterSub_Iteration_2_DoBlur
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Input_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_2_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _FilterSub_Iteration_2_BlurCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_FilterSub_Iteration_2_DoBlur === null || newHash !== this.Hash_Compute_FilterSub_Iteration_2_DoBlur)
        {
            this.Hash_Compute_FilterSub_Iteration_2_DoBlur = newHash;

            let shaderCode = class_SubGraphLoops.ShaderCode_FilterSub_Iteration_2_DoBlur_FilterSub_Iteration_2_Blur;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_2_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly_format));

            this.ShaderModule_Compute_FilterSub_Iteration_2_DoBlur = device.createShaderModule({ code: shaderCode, label: "Compute Shader FilterSub_Iteration_2_DoBlur"});
            this.BindGroupLayout_Compute_FilterSub_Iteration_2_DoBlur = device.createBindGroupLayout({
                label: "Compute Bind Group Layout FilterSub_Iteration_2_DoBlur",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_FilterSub_Iteration_2_DoBlur = device.createPipelineLayout({
                label: "Compute Pipeline Layout FilterSub_Iteration_2_DoBlur",
                bindGroupLayouts: [this.BindGroupLayout_Compute_FilterSub_Iteration_2_DoBlur],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_FilterSub_Iteration_2_DoBlur = device.createComputePipeline({
                    label: "Compute Pipeline FilterSub_Iteration_2_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_2_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_2_DoBlur,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("FilterSub_Iteration_2_DoBlur");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline FilterSub_Iteration_2_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_2_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_2_DoBlur,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_FilterSub_Iteration_2_DoBlur = handle; this.loadingPromises.delete("FilterSub_Iteration_2_DoBlur"); } );
            }
        }
    }

    // (Re)create compute shader FilterSub_Iteration_3_DoBlur
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Input_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_3_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _FilterSub_Iteration_3_BlurCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_FilterSub_Iteration_3_DoBlur === null || newHash !== this.Hash_Compute_FilterSub_Iteration_3_DoBlur)
        {
            this.Hash_Compute_FilterSub_Iteration_3_DoBlur = newHash;

            let shaderCode = class_SubGraphLoops.ShaderCode_FilterSub_Iteration_3_DoBlur_FilterSub_Iteration_3_Blur;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_3_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly_format));

            this.ShaderModule_Compute_FilterSub_Iteration_3_DoBlur = device.createShaderModule({ code: shaderCode, label: "Compute Shader FilterSub_Iteration_3_DoBlur"});
            this.BindGroupLayout_Compute_FilterSub_Iteration_3_DoBlur = device.createBindGroupLayout({
                label: "Compute Bind Group Layout FilterSub_Iteration_3_DoBlur",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_FilterSub_Iteration_3_DoBlur = device.createPipelineLayout({
                label: "Compute Pipeline Layout FilterSub_Iteration_3_DoBlur",
                bindGroupLayouts: [this.BindGroupLayout_Compute_FilterSub_Iteration_3_DoBlur],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_FilterSub_Iteration_3_DoBlur = device.createComputePipeline({
                    label: "Compute Pipeline FilterSub_Iteration_3_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_3_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_3_DoBlur,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("FilterSub_Iteration_3_DoBlur");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline FilterSub_Iteration_3_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_3_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_3_DoBlur,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_FilterSub_Iteration_3_DoBlur = handle; this.loadingPromises.delete("FilterSub_Iteration_3_DoBlur"); } );
            }
        }
    }

    // (Re)create compute shader FilterSub_Iteration_4_DoBlur
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Input_format).sampleType }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_4_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _FilterSub_Iteration_4_BlurCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_FilterSub_Iteration_4_DoBlur === null || newHash !== this.Hash_Compute_FilterSub_Iteration_4_DoBlur)
        {
            this.Hash_Compute_FilterSub_Iteration_4_DoBlur = newHash;

            let shaderCode = class_SubGraphLoops.ShaderCode_FilterSub_Iteration_4_DoBlur_FilterSub_Iteration_4_Blur;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_4_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly_format));

            this.ShaderModule_Compute_FilterSub_Iteration_4_DoBlur = device.createShaderModule({ code: shaderCode, label: "Compute Shader FilterSub_Iteration_4_DoBlur"});
            this.BindGroupLayout_Compute_FilterSub_Iteration_4_DoBlur = device.createBindGroupLayout({
                label: "Compute Bind Group Layout FilterSub_Iteration_4_DoBlur",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_FilterSub_Iteration_4_DoBlur = device.createPipelineLayout({
                label: "Compute Pipeline Layout FilterSub_Iteration_4_DoBlur",
                bindGroupLayouts: [this.BindGroupLayout_Compute_FilterSub_Iteration_4_DoBlur],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_FilterSub_Iteration_4_DoBlur = device.createComputePipeline({
                    label: "Compute Pipeline FilterSub_Iteration_4_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_4_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_4_DoBlur,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("FilterSub_Iteration_4_DoBlur");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline FilterSub_Iteration_4_DoBlur",
                    layout: this.PipelineLayout_Compute_FilterSub_Iteration_4_DoBlur,
                    compute: {
                        module: this.ShaderModule_Compute_FilterSub_Iteration_4_DoBlur,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_FilterSub_Iteration_4_DoBlur = handle; this.loadingPromises.delete("FilterSub_Iteration_4_DoBlur"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("SubGraphLoops.Input");

    encoder.popDebugGroup(); // "SubGraphLoops.Input"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_0_Output");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_0_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_1_Output");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_1_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_2_Output");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_2_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_3_Output");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_3_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_4_Output");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_4_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_0_DoBlur_Output_ReadOnly");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_0_DoBlur_Output_ReadOnly"

    encoder.pushDebugGroup("SubGraphLoops.Copy_FilterSub_Iteration_0_DoBlur_Output");

    // Copy texture FilterSub_Iteration_0_Output to texture FilterSub_Iteration_0_DoBlur_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_0_Output.mipLevelCount, this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_0_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_0_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_0_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_0_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_0_Output, mipLevel: mipIndex },
                { texture: this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.Copy_FilterSub_Iteration_0_DoBlur_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_1_DoBlur_Output_ReadOnly");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_1_DoBlur_Output_ReadOnly"

    encoder.pushDebugGroup("SubGraphLoops.Copy_FilterSub_Iteration_1_DoBlur_Output");

    // Copy texture FilterSub_Iteration_1_Output to texture FilterSub_Iteration_1_DoBlur_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_1_Output.mipLevelCount, this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_1_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_1_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_1_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_1_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_1_Output, mipLevel: mipIndex },
                { texture: this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.Copy_FilterSub_Iteration_1_DoBlur_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_2_DoBlur_Output_ReadOnly");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_2_DoBlur_Output_ReadOnly"

    encoder.pushDebugGroup("SubGraphLoops.Copy_FilterSub_Iteration_2_DoBlur_Output");

    // Copy texture FilterSub_Iteration_2_Output to texture FilterSub_Iteration_2_DoBlur_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_2_Output.mipLevelCount, this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_2_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_2_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_2_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_2_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_2_Output, mipLevel: mipIndex },
                { texture: this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.Copy_FilterSub_Iteration_2_DoBlur_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_3_DoBlur_Output_ReadOnly");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_3_DoBlur_Output_ReadOnly"

    encoder.pushDebugGroup("SubGraphLoops.Copy_FilterSub_Iteration_3_DoBlur_Output");

    // Copy texture FilterSub_Iteration_3_Output to texture FilterSub_Iteration_3_DoBlur_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_3_Output.mipLevelCount, this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_3_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_3_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_3_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_3_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_3_Output, mipLevel: mipIndex },
                { texture: this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.Copy_FilterSub_Iteration_3_DoBlur_Output"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_4_DoBlur_Output_ReadOnly");

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_4_DoBlur_Output_ReadOnly"

    encoder.pushDebugGroup("SubGraphLoops.Copy_FilterSub_Iteration_4_DoBlur_Output");

    // Copy texture FilterSub_Iteration_4_Output to texture FilterSub_Iteration_4_DoBlur_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_4_Output.mipLevelCount, this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_4_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_4_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_4_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_4_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_4_Output, mipLevel: mipIndex },
                { texture: this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.Copy_FilterSub_Iteration_4_DoBlur_Output"

    encoder.pushDebugGroup("SubGraphLoops._FilterSub_Iteration_0_BlurCB");

    // Create constant buffer _FilterSub_Iteration_0_BlurCB
    if (this.constantBuffer__FilterSub_Iteration_0_BlurCB === null)
    {
        this.constantBuffer__FilterSub_Iteration_0_BlurCB = device.createBuffer({
            label: "SubGraphLoops._FilterSub_Iteration_0_BlurCB",
            size: Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_0_BlurCB._size),
            usage: this.constantBuffer__FilterSub_Iteration_0_BlurCB_usageFlags,
        });
    }

    // Upload values to constant buffer _FilterSub_Iteration_0_BlurCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_0_BlurCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__FilterSub_Iteration_0_BlurCB.FilterSub_Iteration_0_sRGB, (this.variable_FilterSub_Iteration_0_sRGB === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__FilterSub_Iteration_0_BlurCB.__loopIndexValue_0, this.variable___loopIndexValue_0, true);
        device.queue.writeBuffer(this.constantBuffer__FilterSub_Iteration_0_BlurCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SubGraphLoops._FilterSub_Iteration_0_BlurCB"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_0_DoBlur");

    // Run compute shader FilterSub_Iteration_0_DoBlur
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group FilterSub_Iteration_0_DoBlur",
            layout: this.BindGroupLayout_Compute_FilterSub_Iteration_0_DoBlur,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Input.createView({ dimension: "2d", format: this.texture_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_FilterSub_Iteration_0_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_FilterSub_Iteration_0_DoBlur_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _FilterSub_Iteration_0_BlurCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__FilterSub_Iteration_0_BlurCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Input_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_FilterSub_Iteration_0_DoBlur !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_FilterSub_Iteration_0_DoBlur);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_0_DoBlur"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_0_Copy_back");

    // Copy texture FilterSub_Iteration_0_Output to texture Input
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_0_Output.mipLevelCount, this.texture_Input.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_0_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_0_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_0_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_0_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_0_Output, mipLevel: mipIndex },
                { texture: this.texture_Input, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_0_Copy_back"

    encoder.pushDebugGroup("SubGraphLoops._FilterSub_Iteration_1_BlurCB");

    // Create constant buffer _FilterSub_Iteration_1_BlurCB
    if (this.constantBuffer__FilterSub_Iteration_1_BlurCB === null)
    {
        this.constantBuffer__FilterSub_Iteration_1_BlurCB = device.createBuffer({
            label: "SubGraphLoops._FilterSub_Iteration_1_BlurCB",
            size: Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_1_BlurCB._size),
            usage: this.constantBuffer__FilterSub_Iteration_1_BlurCB_usageFlags,
        });
    }

    // Upload values to constant buffer _FilterSub_Iteration_1_BlurCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_1_BlurCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__FilterSub_Iteration_1_BlurCB.FilterSub_Iteration_1_sRGB, (this.variable_FilterSub_Iteration_1_sRGB === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__FilterSub_Iteration_1_BlurCB.__loopIndexValue_1, this.variable___loopIndexValue_1, true);
        device.queue.writeBuffer(this.constantBuffer__FilterSub_Iteration_1_BlurCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SubGraphLoops._FilterSub_Iteration_1_BlurCB"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_1_DoBlur");

    // Run compute shader FilterSub_Iteration_1_DoBlur
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group FilterSub_Iteration_1_DoBlur",
            layout: this.BindGroupLayout_Compute_FilterSub_Iteration_1_DoBlur,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Input.createView({ dimension: "2d", format: this.texture_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_FilterSub_Iteration_1_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_FilterSub_Iteration_1_DoBlur_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _FilterSub_Iteration_1_BlurCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__FilterSub_Iteration_1_BlurCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Input_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_FilterSub_Iteration_1_DoBlur !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_FilterSub_Iteration_1_DoBlur);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_1_DoBlur"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_1_Copy_back");

    // Copy texture FilterSub_Iteration_1_Output to texture Input
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_1_Output.mipLevelCount, this.texture_Input.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_1_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_1_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_1_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_1_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_1_Output, mipLevel: mipIndex },
                { texture: this.texture_Input, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_1_Copy_back"

    encoder.pushDebugGroup("SubGraphLoops._FilterSub_Iteration_2_BlurCB");

    // Create constant buffer _FilterSub_Iteration_2_BlurCB
    if (this.constantBuffer__FilterSub_Iteration_2_BlurCB === null)
    {
        this.constantBuffer__FilterSub_Iteration_2_BlurCB = device.createBuffer({
            label: "SubGraphLoops._FilterSub_Iteration_2_BlurCB",
            size: Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_2_BlurCB._size),
            usage: this.constantBuffer__FilterSub_Iteration_2_BlurCB_usageFlags,
        });
    }

    // Upload values to constant buffer _FilterSub_Iteration_2_BlurCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_2_BlurCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__FilterSub_Iteration_2_BlurCB.FilterSub_Iteration_2_sRGB, (this.variable_FilterSub_Iteration_2_sRGB === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__FilterSub_Iteration_2_BlurCB.__loopIndexValue_2, this.variable___loopIndexValue_2, true);
        device.queue.writeBuffer(this.constantBuffer__FilterSub_Iteration_2_BlurCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SubGraphLoops._FilterSub_Iteration_2_BlurCB"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_2_DoBlur");

    // Run compute shader FilterSub_Iteration_2_DoBlur
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group FilterSub_Iteration_2_DoBlur",
            layout: this.BindGroupLayout_Compute_FilterSub_Iteration_2_DoBlur,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Input.createView({ dimension: "2d", format: this.texture_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_FilterSub_Iteration_2_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_FilterSub_Iteration_2_DoBlur_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _FilterSub_Iteration_2_BlurCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__FilterSub_Iteration_2_BlurCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Input_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_FilterSub_Iteration_2_DoBlur !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_FilterSub_Iteration_2_DoBlur);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_2_DoBlur"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_2_Copy_back");

    // Copy texture FilterSub_Iteration_2_Output to texture Input
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_2_Output.mipLevelCount, this.texture_Input.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_2_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_2_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_2_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_2_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_2_Output, mipLevel: mipIndex },
                { texture: this.texture_Input, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_2_Copy_back"

    encoder.pushDebugGroup("SubGraphLoops._FilterSub_Iteration_3_BlurCB");

    // Create constant buffer _FilterSub_Iteration_3_BlurCB
    if (this.constantBuffer__FilterSub_Iteration_3_BlurCB === null)
    {
        this.constantBuffer__FilterSub_Iteration_3_BlurCB = device.createBuffer({
            label: "SubGraphLoops._FilterSub_Iteration_3_BlurCB",
            size: Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_3_BlurCB._size),
            usage: this.constantBuffer__FilterSub_Iteration_3_BlurCB_usageFlags,
        });
    }

    // Upload values to constant buffer _FilterSub_Iteration_3_BlurCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_3_BlurCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__FilterSub_Iteration_3_BlurCB.FilterSub_Iteration_3_sRGB, (this.variable_FilterSub_Iteration_3_sRGB === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__FilterSub_Iteration_3_BlurCB.__loopIndexValue_3, this.variable___loopIndexValue_3, true);
        device.queue.writeBuffer(this.constantBuffer__FilterSub_Iteration_3_BlurCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SubGraphLoops._FilterSub_Iteration_3_BlurCB"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_3_DoBlur");

    // Run compute shader FilterSub_Iteration_3_DoBlur
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group FilterSub_Iteration_3_DoBlur",
            layout: this.BindGroupLayout_Compute_FilterSub_Iteration_3_DoBlur,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Input.createView({ dimension: "2d", format: this.texture_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_FilterSub_Iteration_3_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_FilterSub_Iteration_3_DoBlur_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _FilterSub_Iteration_3_BlurCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__FilterSub_Iteration_3_BlurCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Input_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_FilterSub_Iteration_3_DoBlur !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_FilterSub_Iteration_3_DoBlur);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_3_DoBlur"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_3_Copy_back");

    // Copy texture FilterSub_Iteration_3_Output to texture Input
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_3_Output.mipLevelCount, this.texture_Input.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_3_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_3_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_3_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_3_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_3_Output, mipLevel: mipIndex },
                { texture: this.texture_Input, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_3_Copy_back"

    encoder.pushDebugGroup("SubGraphLoops._FilterSub_Iteration_4_BlurCB");

    // Create constant buffer _FilterSub_Iteration_4_BlurCB
    if (this.constantBuffer__FilterSub_Iteration_4_BlurCB === null)
    {
        this.constantBuffer__FilterSub_Iteration_4_BlurCB = device.createBuffer({
            label: "SubGraphLoops._FilterSub_Iteration_4_BlurCB",
            size: Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_4_BlurCB._size),
            usage: this.constantBuffer__FilterSub_Iteration_4_BlurCB_usageFlags,
        });
    }

    // Upload values to constant buffer _FilterSub_Iteration_4_BlurCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__FilterSub_Iteration_4_BlurCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__FilterSub_Iteration_4_BlurCB.FilterSub_Iteration_4_sRGB, (this.variable_FilterSub_Iteration_4_sRGB === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__FilterSub_Iteration_4_BlurCB.__loopIndexValue_4, this.variable___loopIndexValue_4, true);
        device.queue.writeBuffer(this.constantBuffer__FilterSub_Iteration_4_BlurCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SubGraphLoops._FilterSub_Iteration_4_BlurCB"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_4_DoBlur");

    // Run compute shader FilterSub_Iteration_4_DoBlur
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group FilterSub_Iteration_4_DoBlur",
            layout: this.BindGroupLayout_Compute_FilterSub_Iteration_4_DoBlur,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Input.createView({ dimension: "2d", format: this.texture_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_FilterSub_Iteration_4_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_FilterSub_Iteration_4_DoBlur_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _FilterSub_Iteration_4_BlurCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__FilterSub_Iteration_4_BlurCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Input_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_FilterSub_Iteration_4_DoBlur !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_FilterSub_Iteration_4_DoBlur);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_4_DoBlur"

    encoder.pushDebugGroup("SubGraphLoops.FilterSub_Iteration_4_Copy_back");

    // Copy texture FilterSub_Iteration_4_Output to texture Input
    {
        const numMips = Math.min(this.texture_FilterSub_Iteration_4_Output.mipLevelCount, this.texture_Input.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_FilterSub_Iteration_4_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_FilterSub_Iteration_4_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_FilterSub_Iteration_4_Output.depthOrArrayLayers;

            if (this.texture_FilterSub_Iteration_4_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_FilterSub_Iteration_4_Output, mipLevel: mipIndex },
                { texture: this.texture_Input, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SubGraphLoops.FilterSub_Iteration_4_Copy_back"

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

var SubGraphLoops = new class_SubGraphLoops;

export default SubGraphLoops;
