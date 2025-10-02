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

class class_SlangAutoDiff
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Init_0", node "Initialize"
static ShaderCode_Initialize_Init_0 = `
struct Struct_Init_0CB_std140_0
{
    @align(16) FrameIndex_0 : i32,
    @align(4) _padding0_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
    @align(16) MouseState_0 : vec4<f32>,
    @align(16) MouseStateLastFrame_0 : vec4<f32>,
    @align(16) NumGaussians_0 : i32,
    @align(4) _padding3_0 : f32,
    @align(8) _padding4_0 : f32,
    @align(4) _padding5_0 : f32,
    @align(16) iResolution_0 : vec3<f32>,
    @align(4) initialized_0 : u32,
};

@binding(1) @group(0) var<uniform> _Init_0CB : Struct_Init_0CB_std140_0;
@binding(0) @group(0) var<storage, read_write> Data : array<f32>;

fn wang_hash_init_0( seed_0 : vec3<u32>) -> u32
{
    return ((seed_0.x * u32(1973) + seed_0.y * u32(9277) + seed_0.z * u32(26699)) | (u32(1)));
}

fn wang_hash_uint_0( seed_1 : ptr<function, u32>) -> u32
{
    var _S1 : u32 = (((((*seed_1) ^ (u32(61)))) ^ ((((*seed_1) >> (u32(16))))))) * u32(9);
    var _S2 : u32 = ((_S1 ^ (((_S1 >> (u32(4))))))) * u32(668265261);
    var _S3 : u32 = (_S2 ^ (((_S2 >> (u32(15))))));
    (*seed_1) = _S3;
    return _S3;
}

fn wang_hash_float01_0( state_0 : ptr<function, u32>) -> f32
{
    var _S4 : u32 = wang_hash_uint_0(&((*state_0)));
    return f32((_S4 & (u32(16777215)))) / 1.6777216e+07f;
}

@compute
@workgroup_size(1, 1, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var mouseState_0 : vec4<f32> = _Init_0CB.MouseState_0;
    var mouseStateLastFrame_0 : vec4<f32> = _Init_0CB.MouseStateLastFrame_0;
    var _S5 : bool;
    if(bool(_Init_0CB.MouseState_0.z))
    {
        _S5 = !bool(mouseStateLastFrame_0.z);
    }
    else
    {
        _S5 = false;
    }
    if(_S5)
    {
        var uv_0 : vec2<f32> = mouseState_0.xy / _Init_0CB.iResolution_0.xy;
        Data[i32(0)] = uv_0.x;
        Data[i32(1)] = uv_0.y;
    }
    if(bool(_Init_0CB.initialized_0))
    {
        return;
    }
    var rng_0 : u32 = wang_hash_init_0(vec3<u32>(u32(322417843), u32(3405705229), u32(_Init_0CB.FrameIndex_0)));
    Data[i32(0)] = 0.5f;
    Data[i32(1)] = 0.5f;
    Data[i32(2)] = 0.0f;
    Data[i32(3)] = 0.0f;
    var i_0 : i32 = i32(0);
    for(;;)
    {
        if(i_0 < (_Init_0CB.NumGaussians_0))
        {
        }
        else
        {
            break;
        }
        var _S6 : i32 = i32(4) + i_0 * i32(5);
        var _S7 : f32 = wang_hash_float01_0(&(rng_0));
        Data[_S6] = _S7;
        var _S8 : f32 = wang_hash_float01_0(&(rng_0));
        Data[_S6 + i32(1)] = _S8;
        var _S9 : f32 = wang_hash_float01_0(&(rng_0));
        Data[_S6 + i32(2)] = _S9 * 2.0f * 3.14159274101257324f;
        var _S10 : f32 = wang_hash_float01_0(&(rng_0));
        Data[_S6 + i32(3)] = mix(0.05000000074505806f, 0.30000001192092896f, _S10);
        var _S11 : f32 = wang_hash_float01_0(&(rng_0));
        Data[_S6 + i32(4)] = mix(0.05000000074505806f, 0.30000001192092896f, _S11);
        i_0 = i_0 + i32(1);
    }
    return;
}

`;

// Shader code for Compute shader "Render_0", node "Render"
static ShaderCode_Render_Render_0 = `
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

struct Struct_Render_0CB_std140_0
{
    @align(16) NumGaussians_0 : i32,
    @align(4) QuantizeDisplay_0 : u32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _Render_0CB : Struct_Render_0CB_std140_0;
@binding(0) @group(0) var<storage, read> Data : array<f32>;

fn GetHeightAtPos_0( x_0 : f32,  y_0 : f32,  gaussPos_0 : vec2<f32>,  gaussSigma_0 : vec2<f32>) -> f32
{
    var _S1 : f32 = gaussSigma_0.x;
    var XOverSigma_0 : f32 = x_0 / _S1;
    var _S2 : f32 = sqrt(6.28318548202514648f);
    var _S3 : f32 = gaussSigma_0.y;
    var XOverSigma_1 : f32 = y_0 / _S3;
    return exp(-0.5f * XOverSigma_0 * XOverSigma_0) / (_S1 * _S2) * (exp(-0.5f * XOverSigma_1 * XOverSigma_1) / (_S3 * _S2));
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S4 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S4 = sRGBLo_0.x;
    }
    else
    {
        _S4 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S4;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S4 = sRGBLo_0.y;
    }
    else
    {
        _S4 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S4;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S4 = sRGBLo_0.z;
    }
    else
    {
        _S4 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S4;
    return sRGB_0;
}

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var dims_0 : vec2<u32>;
    var _S5 : u32 = dims_0[i32(0)];
    var _S6 : u32 = dims_0[i32(1)];
    {var dim = textureDimensions((Output));((_S5)) = dim.x;((_S6)) = dim.y;};
    dims_0[i32(0)] = _S5;
    dims_0[i32(1)] = _S6;
    var px_0 : vec2<u32> = DTid_0.xy;
    var uv_0 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(dims_0);
    var i_0 : i32 = i32(0);
    var height_0 : f32 = 0.0f;
    var maxHeight_0 : f32 = 0.0f;
    for(;;)
    {
        if(i_0 < (_Render_0CB.NumGaussians_0))
        {
        }
        else
        {
            break;
        }
        var _S7 : i32 = i32(4) + i_0 * i32(5);
        var gaussPos_1 : vec2<f32> = vec2<f32>(Data[_S7], Data[_S7 + i32(1)]);
        var gaussSigma_1 : vec2<f32> = vec2<f32>(Data[_S7 + i32(3)], Data[_S7 + i32(4)]);
        var pxRelativePos_0 : vec2<f32> = uv_0 - gaussPos_1;
        var _S8 : f32 = - Data[_S7 + i32(2)];
        var cosTheta_0 : f32 = cos(_S8);
        var sinTheta_0 : f32 = sin(_S8);
        var _S9 : f32 = pxRelativePos_0.x;
        var _S10 : f32 = pxRelativePos_0.y;
        var height_1 : f32 = height_0 + GetHeightAtPos_0(_S9 * cosTheta_0 - _S10 * sinTheta_0, _S9 * sinTheta_0 + _S10 * cosTheta_0, gaussPos_1, gaussSigma_1);
        var maxHeight_1 : f32 = maxHeight_0 + GetHeightAtPos_0(0.0f, 0.0f, gaussPos_1, gaussSigma_1);
        i_0 = i_0 + i32(1);
        height_0 = height_1;
        maxHeight_0 = maxHeight_1;
    }
    var ball_0 : f32 = smoothstep(5.0f / f32(dims_0.x), 0.0f, length(vec2<f32>(Data[i32(0)], Data[i32(1)]) - uv_0));
    var color_0 : f32 = height_0 / maxHeight_0;
    var color_1 : f32;
    if(bool(_Render_0CB.QuantizeDisplay_0))
    {
        color_1 = floor(color_0 * 64.0f + 0.5f) / 64.0f;
    }
    else
    {
        color_1 = color_0;
    }
    textureStore((Output), (px_0), (vec4<f32>(LinearToSRGB_0(vec3<f32>(color_1, ball_0, 0.0f)), 1.0f)));
    return;
}

`;

// Shader code for Compute shader "Descend_0", node "GradientDescend"
static ShaderCode_GradientDescend_Descend_0 = `
@binding(0) @group(0) var<storage, read_write> Data : array<f32>;

struct Struct_Descend_0CB_std140_0
{
    @align(16) LearningRate_0 : f32,
    @align(4) MaximumStepSize_0 : f32,
    @align(8) NumGaussians_0 : i32,
    @align(4) UseBackwardAD_0 : u32,
};

@binding(1) @group(0) var<uniform> _Descend_0CB : Struct_Descend_0CB_std140_0;
struct DiffPair_float_0
{
     primal_0 : f32,
     differential_0 : f32,
};

fn _d_exp_0( dpx_0 : ptr<function, DiffPair_float_0>,  dOut_0 : f32)
{
    var _S1 : f32 = exp((*dpx_0).primal_0) * dOut_0;
    (*dpx_0).primal_0 = (*dpx_0).primal_0;
    (*dpx_0).differential_0 = _S1;
    return;
}

fn _d_exp_1( dpx_1 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S2 : f32 = exp(dpx_1.primal_0);
    var _S3 : DiffPair_float_0 = DiffPair_float_0( _S2, _S2 * dpx_1.differential_0 );
    return _S3;
}

fn _d_sqrt_0( dpx_2 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S4 : DiffPair_float_0 = DiffPair_float_0( sqrt(dpx_2.primal_0), 0.5f / sqrt(max(1.00000001168609742e-07f, dpx_2.primal_0)) * dpx_2.differential_0 );
    return _S4;
}

fn GetHeightAtPos_0( x_0 : f32,  y_0 : f32,  gaussPos_0 : vec2<f32>,  gaussSigma_0 : vec2<f32>) -> f32
{
    var _S5 : f32 = gaussSigma_0.x;
    var XOverSigma_0 : f32 = x_0 / _S5;
    var _S6 : f32 = sqrt(6.28318548202514648f);
    var _S7 : f32 = gaussSigma_0.y;
    var XOverSigma_1 : f32 = y_0 / _S7;
    return exp(-0.5f * XOverSigma_0 * XOverSigma_0) / (_S5 * _S6) * (exp(-0.5f * XOverSigma_1 * XOverSigma_1) / (_S7 * _S6));
}

fn s_primal_ctx_exp_0( _S8 : f32) -> f32
{
    return exp(_S8);
}

fn s_primal_ctx_sqrt_0( _S9 : f32) -> f32
{
    return sqrt(_S9);
}

fn s_bwd_prop_exp_0( _S10 : ptr<function, DiffPair_float_0>,  _S11 : f32)
{
    _d_exp_0(&((*_S10)), _S11);
    return;
}

fn s_bwd_prop_GetHeightAtPos_0( dpx_3 : ptr<function, DiffPair_float_0>,  dpy_0 : ptr<function, DiffPair_float_0>,  gaussPos_1 : vec2<f32>,  gaussSigma_1 : vec2<f32>,  _s_dOut_0 : f32)
{
    var _S12 : f32 = gaussSigma_1.x;
    var XOverSigma_2 : f32 = (*dpx_3).primal_0 / _S12;
    var _S13 : f32 = -0.5f * XOverSigma_2;
    var _S14 : f32 = _S13 * XOverSigma_2;
    var _S15 : f32 = s_primal_ctx_sqrt_0(6.28318548202514648f);
    var _S16 : f32 = _S12 * _S15;
    var _S17 : f32 = _S16 * _S16;
    var _S18 : f32 = gaussSigma_1.y;
    var XOverSigma_3 : f32 = (*dpy_0).primal_0 / _S18;
    var _S19 : f32 = -0.5f * XOverSigma_3;
    var _S20 : f32 = _S19 * XOverSigma_3;
    var _S21 : f32 = _S18 * _S15;
    var s_diff_gaussX_T_0 : f32 = s_primal_ctx_exp_0(_S20) / _S21 * _s_dOut_0;
    var _S22 : f32 = _S21 * (s_primal_ctx_exp_0(_S14) / _S16 * _s_dOut_0 / (_S21 * _S21));
    var _S23 : DiffPair_float_0;
    _S23.primal_0 = _S20;
    _S23.differential_0 = 0.0f;
    s_bwd_prop_exp_0(&(_S23), _S22);
    var _S24 : f32 = (_S19 * _S23.differential_0 + -0.5f * (XOverSigma_3 * _S23.differential_0)) / _S18;
    var _S25 : f32 = _S16 * (s_diff_gaussX_T_0 / _S17);
    var _S26 : DiffPair_float_0;
    _S26.primal_0 = _S14;
    _S26.differential_0 = 0.0f;
    s_bwd_prop_exp_0(&(_S26), _S25);
    var _S27 : f32 = (_S13 * _S26.differential_0 + -0.5f * (XOverSigma_2 * _S26.differential_0)) / _S12;
    (*dpy_0).primal_0 = (*dpy_0).primal_0;
    (*dpy_0).differential_0 = _S24;
    (*dpx_3).primal_0 = (*dpx_3).primal_0;
    (*dpx_3).differential_0 = _S27;
    return;
}

fn s_bwd_GetHeightAtPos_0( _S28 : ptr<function, DiffPair_float_0>,  _S29 : ptr<function, DiffPair_float_0>,  _S30 : vec2<f32>,  _S31 : vec2<f32>,  _S32 : f32)
{
    s_bwd_prop_GetHeightAtPos_0(&((*_S28)), &((*_S29)), _S30, _S31, _S32);
    return;
}

fn s_fwd_GetHeightAtPos_0( dpx_4 : DiffPair_float_0,  dpy_1 : DiffPair_float_0,  gaussPos_2 : vec2<f32>,  gaussSigma_2 : vec2<f32>) -> DiffPair_float_0
{
    var _S33 : f32 = gaussSigma_2.x;
    var XOverSigma_4 : f32 = dpx_4.primal_0 / _S33;
    var s_diff_XOverSigma_0 : f32 = dpx_4.differential_0 / _S33;
    var _S34 : f32 = -0.5f * XOverSigma_4;
    var _S35 : DiffPair_float_0 = DiffPair_float_0( _S34 * XOverSigma_4, s_diff_XOverSigma_0 * -0.5f * XOverSigma_4 + s_diff_XOverSigma_0 * _S34 );
    var _S36 : DiffPair_float_0 = _d_exp_1(_S35);
    var _S37 : DiffPair_float_0 = DiffPair_float_0( 6.28318548202514648f, 0.0f );
    var _S38 : DiffPair_float_0 = _d_sqrt_0(_S37);
    var _S39 : f32 = _S33 * _S38.primal_0;
    var gaussX_0 : f32 = _S36.primal_0 / _S39;
    var _S40 : f32 = gaussSigma_2.y;
    var XOverSigma_5 : f32 = dpy_1.primal_0 / _S40;
    var s_diff_XOverSigma_1 : f32 = dpy_1.differential_0 / _S40;
    var _S41 : f32 = -0.5f * XOverSigma_5;
    var _S42 : DiffPair_float_0 = DiffPair_float_0( _S41 * XOverSigma_5, s_diff_XOverSigma_1 * -0.5f * XOverSigma_5 + s_diff_XOverSigma_1 * _S41 );
    var _S43 : DiffPair_float_0 = _d_exp_1(_S42);
    var _S44 : f32 = _S40 * _S38.primal_0;
    var gaussY_0 : f32 = _S43.primal_0 / _S44;
    var _S45 : DiffPair_float_0 = DiffPair_float_0( gaussX_0 * gaussY_0, (_S36.differential_0 * _S39 - _S36.primal_0 * (_S38.differential_0 * _S33)) / (_S39 * _S39) * gaussY_0 + (_S43.differential_0 * _S44 - _S43.primal_0 * (_S38.differential_0 * _S40)) / (_S44 * _S44) * gaussX_0 );
    return _S45;
}

@compute
@workgroup_size(1, 1, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var ballPos_0 : vec2<f32> = vec2<f32>(Data[i32(0)], Data[i32(1)]);
    const _S46 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
    var i_0 : i32 = i32(0);
    var ballPosGradient_0 : vec2<f32> = _S46;
    for(;;)
    {
        if(i_0 < (_Descend_0CB.NumGaussians_0))
        {
        }
        else
        {
            break;
        }
        var _S47 : i32 = i32(4) + i_0 * i32(5);
        var gaussPos_3 : vec2<f32> = vec2<f32>(Data[_S47], Data[_S47 + i32(1)]);
        var gaussAngle_0 : f32 = Data[_S47 + i32(2)];
        var gaussSigma_3 : vec2<f32> = vec2<f32>(Data[_S47 + i32(3)], Data[_S47 + i32(4)]);
        var relativePos_0 : vec2<f32> = ballPos_0 - gaussPos_3;
        var _S48 : f32 = - Data[_S47 + i32(2)];
        var cosTheta_0 : f32 = cos(_S48);
        var sinTheta_0 : f32 = sin(_S48);
        var _S49 : f32 = relativePos_0.x;
        var _S50 : f32 = relativePos_0.y;
        var _S51 : f32 = _S49 * cosTheta_0 - _S50 * sinTheta_0;
        var _S52 : f32 = _S49 * sinTheta_0 + _S50 * cosTheta_0;
        var dFLocal_0 : vec2<f32> = _S46;
        if(bool(_Descend_0CB.UseBackwardAD_0))
        {
            var height_0 : f32 = GetHeightAtPos_0(_S51, _S52, gaussPos_3, gaussSigma_3);
            var ballPosX_0 : DiffPair_float_0;
            ballPosX_0.primal_0 = _S51;
            ballPosX_0.differential_0 = 0.0f;
            var ballPosY_0 : DiffPair_float_0;
            ballPosY_0.primal_0 = _S52;
            ballPosY_0.differential_0 = 0.0f;
            s_bwd_GetHeightAtPos_0(&(ballPosX_0), &(ballPosY_0), gaussPos_3, gaussSigma_3, height_0);
            dFLocal_0 = vec2<f32>(ballPosX_0.differential_0, ballPosY_0.differential_0);
        }
        else
        {
            var _S53 : DiffPair_float_0 = DiffPair_float_0( _S51, 1.0f );
            var _S54 : DiffPair_float_0 = DiffPair_float_0( _S52, 0.0f );
            dFLocal_0[i32(0)] = s_fwd_GetHeightAtPos_0(_S53, _S54, gaussPos_3, gaussSigma_3).differential_0;
            var _S55 : DiffPair_float_0 = DiffPair_float_0( _S51, 0.0f );
            var _S56 : DiffPair_float_0 = DiffPair_float_0( _S52, 1.0f );
            dFLocal_0[i32(1)] = dFLocal_0[i32(1)] + s_fwd_GetHeightAtPos_0(_S55, _S56, gaussPos_3, gaussSigma_3).differential_0;
        }
        var cosNegTheta_0 : f32 = cos(gaussAngle_0);
        var sinNegTheta_0 : f32 = sin(gaussAngle_0);
        var ballPosGradient_1 : vec2<f32> = ballPosGradient_0 + vec2<f32>(dFLocal_0.x * cosNegTheta_0 - dFLocal_0.y * sinNegTheta_0, dFLocal_0.x * sinNegTheta_0 + dFLocal_0.y * cosNegTheta_0);
        i_0 = i_0 + i32(1);
        ballPosGradient_0 = ballPosGradient_1;
    }
    var adjust_0 : vec2<f32> = - ballPosGradient_0 * vec2<f32>(_Descend_0CB.LearningRate_0);
    var adjust_1 : vec2<f32>;
    if((length(adjust_0)) > (_Descend_0CB.MaximumStepSize_0))
    {
        adjust_1 = normalize(adjust_0) * vec2<f32>(_Descend_0CB.MaximumStepSize_0);
    }
    else
    {
        adjust_1 = adjust_0;
    }
    var ballPos_1 : vec2<f32> = clamp(ballPos_0 + adjust_1, vec2<f32>(0.00100000004749745f, 0.00100000004749745f), vec2<f32>(0.99000000953674316f, 0.99000000953674316f));
    Data[i32(0)] = ballPos_1.x;
    Data[i32(1)] = ballPos_1.y;
    Data[i32(2)] = ballPosGradient_0.x;
    Data[i32(3)] = ballPosGradient_0.y;
    return;
}

`;

// -------------------- Private Members

// Buffer Data : first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
buffer_Data = null;
buffer_Data_count = 0;
buffer_Data_stride = 0;
buffer_Data_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Texture Render_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Render_Output_ReadOnly = null;
texture_Render_Output_ReadOnly_size = [0, 0, 0];
texture_Render_Output_ReadOnly_format = "";
texture_Render_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _Init_0CB
constantBuffer__Init_0CB = null;
constantBuffer__Init_0CB_size = 80;
constantBuffer__Init_0CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Initialize
Hash_Compute_Initialize = 0;
ShaderModule_Compute_Initialize = null;
BindGroupLayout_Compute_Initialize = null;
PipelineLayout_Compute_Initialize = null;
Pipeline_Compute_Initialize = null;

// Constant buffer _Render_0CB
constantBuffer__Render_0CB = null;
constantBuffer__Render_0CB_size = 16;
constantBuffer__Render_0CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Constant buffer _Descend_0CB
constantBuffer__Descend_0CB = null;
constantBuffer__Descend_0CB_size = 16;
constantBuffer__Descend_0CB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader GradientDescend
Hash_Compute_GradientDescend = 0;
ShaderModule_Compute_GradientDescend = null;
BindGroupLayout_Compute_GradientDescend = null;
PipelineLayout_Compute_GradientDescend = null;
Pipeline_Compute_GradientDescend = null;

// Compute Shader Render
Hash_Compute_Render = 0;
ShaderModule_Compute_Render = null;
BindGroupLayout_Compute_Render = null;
PipelineLayout_Compute_Render = null;
Pipeline_Compute_Render = null;

// -------------------- Imported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_NumGaussians = 10;
variableDefault_NumGaussians = 10;
variableChanged_NumGaussians = false;
variable_Reset = false;
variableDefault_Reset = false;
variableChanged_Reset = false;
variable_initialized = false;
variableDefault_initialized = false;
variableChanged_initialized = false;
variable_FrameIndex = 0;
variableDefault_FrameIndex = 0;
variableChanged_FrameIndex = false;
variable_LearningRate = 0.100000001;
variableDefault_LearningRate = 0.100000001;
variableChanged_LearningRate = false;
variable_MaximumStepSize = 0.00999999978;
variableDefault_MaximumStepSize = 0.00999999978;
variableChanged_MaximumStepSize = false;
variable_MouseState = [ 0., 0., 0., 0. ];
variableDefault_MouseState = [ 0., 0., 0., 0. ];
variableChanged_MouseState = [ false, false, false, false ];
variable_MouseStateLastFrame = [ 0., 0., 0., 0. ];
variableDefault_MouseStateLastFrame = [ 0., 0., 0., 0. ];
variableChanged_MouseStateLastFrame = [ false, false, false, false ];
variable_iResolution = [ 0., 0., 0. ];
variableDefault_iResolution = [ 0., 0., 0. ];
variableChanged_iResolution = [ false, false, false ];
variable_UseBackwardAD = true;
variableDefault_UseBackwardAD = true;
variableChanged_UseBackwardAD = false;
variable_QuantizeDisplay = false;
variableDefault_QuantizeDisplay = false;
variableChanged_QuantizeDisplay = false;

// -------------------- Structs

static StructOffsets__Init_0CB =
{
    FrameIndex: 0,
    _padding0: 4,
    _padding1: 8,
    _padding2: 12,
    MouseState_0: 16,
    MouseState_1: 20,
    MouseState_2: 24,
    MouseState_3: 28,
    MouseStateLastFrame_0: 32,
    MouseStateLastFrame_1: 36,
    MouseStateLastFrame_2: 40,
    MouseStateLastFrame_3: 44,
    NumGaussians: 48,
    _padding3: 52,
    _padding4: 56,
    _padding5: 60,
    iResolution_0: 64,
    iResolution_1: 68,
    iResolution_2: 72,
    initialized: 76,
    _size: 80,
}

static StructOffsets__Render_0CB =
{
    NumGaussians: 0,
    QuantizeDisplay: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__Descend_0CB =
{
    LearningRate: 0,
    MaximumStepSize: 4,
    NumGaussians: 8,
    UseBackwardAD: 12,
    _size: 16,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{
    {
        this.variableChanged_initialized = true;
        this.variable_initialized = !this.variable_Reset;
    }

    if (!this.variable_initialized)
    {
        this.variableChanged_Reset = true;
        this.variable_Reset = false;
    }


}

async ValidateImports()
{
    // Validate texture Output
    if (this.texture_Output === null)
    {
        Shared.LogError("Imported resource texture_Output was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of buffer Data
    {
        const baseCount = this.variable_NumGaussians;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 5) / 1) + 4;
        const desiredStride = 4;
        if (this.buffer_Data !== null && (this.buffer_Data_count != desiredCount || this.buffer_Data_stride != desiredStride))
        {
            this.buffer_Data.destroy();
            this.buffer_Data = null;
        }

        if (this.buffer_Data === null)
        {
            this.buffer_Data_count = desiredCount;
            this.buffer_Data_stride = desiredStride;
            this.buffer_Data = device.createBuffer({
                label: "buffer SlangAutoDiff.Data",
                size: Shared.Align(16, this.buffer_Data_count * this.buffer_Data_stride),
                usage: this.buffer_Data_usageFlags,
            });
        }
    }

    // Handle (re)creation of texture Render_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Render_Output_ReadOnly !== null && (this.texture_Render_Output_ReadOnly_format != desiredFormat || this.texture_Render_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Render_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Render_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Render_Output_ReadOnly.destroy();
            this.texture_Render_Output_ReadOnly = null;
        }

        if (this.texture_Render_Output_ReadOnly === null)
        {
            this.texture_Render_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Render_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Render_Output_ReadOnly_format))
                viewFormats.push(this.texture_Render_Output_ReadOnly_format);

            this.texture_Render_Output_ReadOnly = device.createTexture({
                label: "texture SlangAutoDiff.Render_Output_ReadOnly",
                size: this.texture_Render_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Render_Output_ReadOnly_format),
                usage: this.texture_Render_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Initialize
    {
        const bindGroupEntries =
        [
            {
                // Data
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // _Init_0CB
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Initialize === null || newHash !== this.Hash_Compute_Initialize)
        {
            this.Hash_Compute_Initialize = newHash;

            let shaderCode = class_SlangAutoDiff.ShaderCode_Initialize_Init_0;

            this.ShaderModule_Compute_Initialize = device.createShaderModule({ code: shaderCode, label: "Compute Shader Initialize"});
            this.BindGroupLayout_Compute_Initialize = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Initialize",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Initialize = device.createPipelineLayout({
                label: "Compute Pipeline Layout Initialize",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Initialize],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Initialize = device.createComputePipeline({
                    label: "Compute Pipeline Initialize",
                    layout: this.PipelineLayout_Compute_Initialize,
                    compute: {
                        module: this.ShaderModule_Compute_Initialize,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Initialize");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Initialize",
                    layout: this.PipelineLayout_Compute_Initialize,
                    compute: {
                        module: this.ShaderModule_Compute_Initialize,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Initialize = handle; this.loadingPromises.delete("Initialize"); } );
            }
        }
    }

    // (Re)create compute shader GradientDescend
    {
        const bindGroupEntries =
        [
            {
                // Data
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // _Descend_0CB
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_GradientDescend === null || newHash !== this.Hash_Compute_GradientDescend)
        {
            this.Hash_Compute_GradientDescend = newHash;

            let shaderCode = class_SlangAutoDiff.ShaderCode_GradientDescend_Descend_0;

            this.ShaderModule_Compute_GradientDescend = device.createShaderModule({ code: shaderCode, label: "Compute Shader GradientDescend"});
            this.BindGroupLayout_Compute_GradientDescend = device.createBindGroupLayout({
                label: "Compute Bind Group Layout GradientDescend",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_GradientDescend = device.createPipelineLayout({
                label: "Compute Pipeline Layout GradientDescend",
                bindGroupLayouts: [this.BindGroupLayout_Compute_GradientDescend],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_GradientDescend = device.createComputePipeline({
                    label: "Compute Pipeline GradientDescend",
                    layout: this.PipelineLayout_Compute_GradientDescend,
                    compute: {
                        module: this.ShaderModule_Compute_GradientDescend,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("GradientDescend");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline GradientDescend",
                    layout: this.PipelineLayout_Compute_GradientDescend,
                    compute: {
                        module: this.ShaderModule_Compute_GradientDescend,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_GradientDescend = handle; this.loadingPromises.delete("GradientDescend"); } );
            }
        }
    }

    // (Re)create compute shader Render
    {
        const bindGroupEntries =
        [
            {
                // Data
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Render_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _Render_0CB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Render === null || newHash !== this.Hash_Compute_Render)
        {
            this.Hash_Compute_Render = newHash;

            let shaderCode = class_SlangAutoDiff.ShaderCode_Render_Render_0;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Render_Output_ReadOnly_format));

            this.ShaderModule_Compute_Render = device.createShaderModule({ code: shaderCode, label: "Compute Shader Render"});
            this.BindGroupLayout_Compute_Render = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Render",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Render = device.createPipelineLayout({
                label: "Compute Pipeline Layout Render",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Render],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Render = device.createComputePipeline({
                    label: "Compute Pipeline Render",
                    layout: this.PipelineLayout_Compute_Render,
                    compute: {
                        module: this.ShaderModule_Compute_Render,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Render");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Render",
                    layout: this.PipelineLayout_Compute_Render,
                    compute: {
                        module: this.ShaderModule_Compute_Render,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Render = handle; this.loadingPromises.delete("Render"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("SlangAutoDiff.Data");

    encoder.popDebugGroup(); // "SlangAutoDiff.Data"

    encoder.pushDebugGroup("SlangAutoDiff.Output");

    encoder.popDebugGroup(); // "SlangAutoDiff.Output"

    encoder.pushDebugGroup("SlangAutoDiff.Render_Output_ReadOnly");

    encoder.popDebugGroup(); // "SlangAutoDiff.Render_Output_ReadOnly"

    encoder.pushDebugGroup("SlangAutoDiff.Copy_Render_Output");

    // Copy texture Output to texture Render_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Render_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Render_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Copy_Render_Output"

    encoder.pushDebugGroup("SlangAutoDiff._Init_0CB");

    // Create constant buffer _Init_0CB
    if (this.constantBuffer__Init_0CB === null)
    {
        this.constantBuffer__Init_0CB = device.createBuffer({
            label: "SlangAutoDiff._Init_0CB",
            size: Shared.Align(16, this.constructor.StructOffsets__Init_0CB._size),
            usage: this.constantBuffer__Init_0CB_usageFlags,
        });
    }

    // Upload values to constant buffer _Init_0CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Init_0CB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__Init_0CB.FrameIndex, this.variable_FrameIndex, true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseState_0, this.variable_MouseState[0], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseState_1, this.variable_MouseState[1], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseState_2, this.variable_MouseState[2], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseState_3, this.variable_MouseState[3], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseStateLastFrame_0, this.variable_MouseStateLastFrame[0], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseStateLastFrame_1, this.variable_MouseStateLastFrame[1], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseStateLastFrame_2, this.variable_MouseStateLastFrame[2], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.MouseStateLastFrame_3, this.variable_MouseStateLastFrame[3], true);
        view.setInt32(this.constructor.StructOffsets__Init_0CB.NumGaussians, this.variable_NumGaussians, true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.iResolution_0, this.variable_iResolution[0], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.iResolution_1, this.variable_iResolution[1], true);
        view.setFloat32(this.constructor.StructOffsets__Init_0CB.iResolution_2, this.variable_iResolution[2], true);
        view.setUint32(this.constructor.StructOffsets__Init_0CB.initialized, (this.variable_initialized === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__Init_0CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SlangAutoDiff._Init_0CB"

    encoder.pushDebugGroup("SlangAutoDiff.Initialize");

    // Run compute shader Initialize
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Initialize",
            layout: this.BindGroupLayout_Compute_Initialize,
            entries: [
                {
                    // Data
                    binding: 0,
                    resource: { buffer: this.buffer_Data }
                },
                {
                    // _Init_0CB
                    binding: 1,
                    resource: { buffer: this.constantBuffer__Init_0CB }
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

        if (this.Pipeline_Compute_Initialize !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Initialize);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Initialize"

    encoder.pushDebugGroup("SlangAutoDiff._Render_0CB");

    // Create constant buffer _Render_0CB
    if (this.constantBuffer__Render_0CB === null)
    {
        this.constantBuffer__Render_0CB = device.createBuffer({
            label: "SlangAutoDiff._Render_0CB",
            size: Shared.Align(16, this.constructor.StructOffsets__Render_0CB._size),
            usage: this.constantBuffer__Render_0CB_usageFlags,
        });
    }

    // Upload values to constant buffer _Render_0CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Render_0CB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__Render_0CB.NumGaussians, this.variable_NumGaussians, true);
        view.setUint32(this.constructor.StructOffsets__Render_0CB.QuantizeDisplay, (this.variable_QuantizeDisplay === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__Render_0CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SlangAutoDiff._Render_0CB"

    encoder.pushDebugGroup("SlangAutoDiff._Descend_0CB");

    // Create constant buffer _Descend_0CB
    if (this.constantBuffer__Descend_0CB === null)
    {
        this.constantBuffer__Descend_0CB = device.createBuffer({
            label: "SlangAutoDiff._Descend_0CB",
            size: Shared.Align(16, this.constructor.StructOffsets__Descend_0CB._size),
            usage: this.constantBuffer__Descend_0CB_usageFlags,
        });
    }

    // Upload values to constant buffer _Descend_0CB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Descend_0CB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__Descend_0CB.LearningRate, this.variable_LearningRate, true);
        view.setFloat32(this.constructor.StructOffsets__Descend_0CB.MaximumStepSize, this.variable_MaximumStepSize, true);
        view.setInt32(this.constructor.StructOffsets__Descend_0CB.NumGaussians, this.variable_NumGaussians, true);
        view.setUint32(this.constructor.StructOffsets__Descend_0CB.UseBackwardAD, (this.variable_UseBackwardAD === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__Descend_0CB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SlangAutoDiff._Descend_0CB"

    encoder.pushDebugGroup("SlangAutoDiff.GradientDescend");

    // Run compute shader GradientDescend
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group GradientDescend",
            layout: this.BindGroupLayout_Compute_GradientDescend,
            entries: [
                {
                    // Data
                    binding: 0,
                    resource: { buffer: this.buffer_Data }
                },
                {
                    // _Descend_0CB
                    binding: 1,
                    resource: { buffer: this.constantBuffer__Descend_0CB }
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

        if (this.Pipeline_Compute_GradientDescend !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_GradientDescend);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.GradientDescend"

    encoder.pushDebugGroup("SlangAutoDiff.Render");

    // Run compute shader Render
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Render",
            layout: this.BindGroupLayout_Compute_Render,
            entries: [
                {
                    // Data
                    binding: 0,
                    resource: { buffer: this.buffer_Data }
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_Render_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _Render_0CB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__Render_0CB }
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

        if (this.Pipeline_Compute_Render !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Render);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Render"

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

var SlangAutoDiff = new class_SlangAutoDiff;

export default SlangAutoDiff;
