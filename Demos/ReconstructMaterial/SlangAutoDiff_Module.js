import * as Shared from './Shared.js';

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
// Shader code for Compute shader "ComputeGradient", node "ComputeGradient"
static ShaderCode_ComputeGradient_ComputeGradient = `
@binding(4) @group(0) var StateAReadOnly : texture_storage_2d</*(StateAReadOnly_format)*/, read>;

@binding(5) @group(0) var StateBReadOnly : texture_storage_2d</*(StateBReadOnly_format)*/, read>;

struct Struct_ComputeGradientCB_std140_0
{
    @align(16) FrameIndex_0 : i32,
    @align(4) Reset_0 : u32,
    @align(8) UseBackwardAD_0 : u32,
    @align(4) _padding0_0 : f32,
};

@binding(6) @group(0) var<uniform> _ComputeGradientCB : Struct_ComputeGradientCB_std140_0;
@binding(0) @group(0) var OutputGradient : texture_storage_2d</*(OutputGradient_format)*/, write>;

@binding(1) @group(0) var StateA : texture_storage_2d</*(StateA_format)*/, write>;

@binding(2) @group(0) var StateB : texture_storage_2d</*(StateB_format)*/, write>;

fn getSmoothStateA_0( DTid_0 : vec2<u32>) -> vec4<f32>
{
    var _S1 : vec2<u32> = DTid_0.xy;
    var _S2 : vec4<f32> = (textureLoad((StateAReadOnly), (vec2<i32>(_S1 + vec2<u32>(vec2<i32>(i32(-1), i32(0)))))));
    var a_0 : vec4<f32> = saturate(_S2);
    var _S3 : vec4<f32> = (textureLoad((StateAReadOnly), (vec2<i32>(_S1 + vec2<u32>(vec2<i32>(i32(1), i32(0)))))));
    var b_0 : vec4<f32> = saturate(_S3);
    var _S4 : vec4<f32> = (textureLoad((StateAReadOnly), (vec2<i32>(_S1 + vec2<u32>(vec2<i32>(i32(0), i32(-1)))))));
    var c_0 : vec4<f32> = saturate(_S4);
    var _S5 : vec4<f32> = (textureLoad((StateAReadOnly), (vec2<i32>(_S1 + vec2<u32>(vec2<i32>(i32(0), i32(1)))))));
    return (a_0 + b_0 + c_0 + saturate(_S5)) * vec4<f32>(0.25f);
}

fn getSmoothStateB_0( DTid_1 : vec2<u32>) -> vec4<f32>
{
    var _S6 : vec2<u32> = DTid_1.xy;
    var _S7 : vec4<f32> = (textureLoad((StateBReadOnly), (vec2<i32>(_S6 + vec2<u32>(vec2<i32>(i32(-1), i32(0)))))));
    var a_1 : vec4<f32> = saturate(_S7);
    var _S8 : vec4<f32> = (textureLoad((StateBReadOnly), (vec2<i32>(_S6 + vec2<u32>(vec2<i32>(i32(1), i32(0)))))));
    var b_1 : vec4<f32> = saturate(_S8);
    var _S9 : vec4<f32> = (textureLoad((StateBReadOnly), (vec2<i32>(_S6 + vec2<u32>(vec2<i32>(i32(0), i32(-1)))))));
    var c_1 : vec4<f32> = saturate(_S9);
    var _S10 : vec4<f32> = (textureLoad((StateBReadOnly), (vec2<i32>(_S6 + vec2<u32>(vec2<i32>(i32(0), i32(1)))))));
    return (a_1 + b_1 + c_1 + saturate(_S10)) * vec4<f32>(0.25f);
}

struct DiffPair_float_0
{
     primal_0 : f32,
     differential_0 : f32,
};

fn _d_sqrt_0( dpx_0 : ptr<function, DiffPair_float_0>,  dOut_0 : f32)
{
    var _S11 : f32 = 0.5f / sqrt(max(1.00000001168609742e-07f, (*dpx_0).primal_0)) * dOut_0;
    (*dpx_0).primal_0 = (*dpx_0).primal_0;
    (*dpx_0).differential_0 = _S11;
    return;
}

fn _d_sqrt_1( dpx_1 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S12 : DiffPair_float_0 = DiffPair_float_0( sqrt(dpx_1.primal_0), 0.5f / sqrt(max(1.00000001168609742e-07f, dpx_1.primal_0)) * dpx_1.differential_0 );
    return _S12;
}

struct DiffPair_vectorx3Cfloatx2C3x3E_0
{
     primal_0 : vec3<f32>,
     differential_0 : vec3<f32>,
};

fn _d_dot_0( dpx_2 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  dpy_0 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  dOut_1 : f32)
{
    var x_d_result_0 : vec3<f32>;
    x_d_result_0[i32(0)] = (*dpy_0).primal_0[i32(0)] * dOut_1;
    var y_d_result_0 : vec3<f32>;
    y_d_result_0[i32(0)] = (*dpx_2).primal_0[i32(0)] * dOut_1;
    x_d_result_0[i32(1)] = (*dpy_0).primal_0[i32(1)] * dOut_1;
    y_d_result_0[i32(1)] = (*dpx_2).primal_0[i32(1)] * dOut_1;
    x_d_result_0[i32(2)] = (*dpy_0).primal_0[i32(2)] * dOut_1;
    y_d_result_0[i32(2)] = (*dpx_2).primal_0[i32(2)] * dOut_1;
    (*dpx_2).primal_0 = (*dpx_2).primal_0;
    (*dpx_2).differential_0 = x_d_result_0;
    (*dpy_0).primal_0 = (*dpy_0).primal_0;
    (*dpy_0).differential_0 = y_d_result_0;
    return;
}

fn _d_dot_1( dpx_3 : DiffPair_vectorx3Cfloatx2C3x3E_0,  dpy_1 : DiffPair_vectorx3Cfloatx2C3x3E_0) -> DiffPair_float_0
{
    var _S13 : DiffPair_float_0 = DiffPair_float_0( dpx_3.primal_0[i32(0)] * dpy_1.primal_0[i32(0)] + dpx_3.primal_0[i32(1)] * dpy_1.primal_0[i32(1)] + dpx_3.primal_0[i32(2)] * dpy_1.primal_0[i32(2)], dpx_3.primal_0[i32(0)] * dpy_1.differential_0[i32(0)] + dpy_1.primal_0[i32(0)] * dpx_3.differential_0[i32(0)] + dpx_3.primal_0[i32(1)] * dpy_1.differential_0[i32(1)] + dpy_1.primal_0[i32(1)] * dpx_3.differential_0[i32(1)] + dpx_3.primal_0[i32(2)] * dpy_1.differential_0[i32(2)] + dpy_1.primal_0[i32(2)] * dpx_3.differential_0[i32(2)] );
    return _S13;
}

fn base_hash_0( p_0 : vec2<u32>) -> u32
{
    var _S14 : vec2<u32> = vec2<u32>(u32(1103515245)) * ((((p_0 >> (vec2<u32>(u32(1))))) ^ ((p_0.yx))));
    var h32_0 : u32 = u32(1103515245) * (((_S14.x) ^ ((((_S14.y) >> (u32(3)))))));
    return (h32_0 ^ (((h32_0 >> (u32(16))))));
}

fn hash3_0( seed_0 : ptr<function, f32>) -> vec3<f32>
{
    var _S15 : f32 = (*seed_0) + 0.10000000149011612f;
    var _S16 : f32 = _S15 + 0.10000000149011612f;
    (*seed_0) = _S16;
    var n_0 : u32 = base_hash_0((bitcast<vec2<u32>>((vec2<f32>(_S15, _S16)))));
    return vec3<f32>((vec3<u32>(n_0, n_0 * u32(16807), n_0 * u32(48271)) & (vec3<u32>(u32(2147483647))))) / vec3<f32>(2.147483648e+09f);
}

fn random_in_unit_sphere_0( seed_1 : ptr<function, f32>) -> vec3<f32>
{
    var _S17 : vec3<f32> = hash3_0(&((*seed_1)));
    var h_0 : vec3<f32> = _S17 * vec3<f32>(2.0f, 6.28318548202514648f, 1.0f) - vec3<f32>(1.0f, 0.0f, 0.0f);
    var phi_0 : f32 = h_0.y;
    var _S18 : f32 = h_0.x;
    return vec3<f32>(pow(h_0.z, 0.3333333432674408f)) * vec3<f32>(vec2<f32>(sqrt(1.0f - _S18 * _S18)) * vec2<f32>(sin(phi_0), cos(phi_0)), _S18);
}

struct GBuffer_0
{
     depth_0 : f32,
     normal_0 : vec3<f32>,
     baseColor_0 : vec3<f32>,
     metallic_0 : f32,
     linearRoughness_0 : f32,
};

fn sphere_0( gbuffer_0 : ptr<function, GBuffer_0>,  pos_0 : vec2<f32>,  center_0 : vec3<f32>,  radius_0 : f32,  baseColor_1 : vec3<f32>,  linearRoughness_1 : f32,  metallic_1 : f32)
{
    var norm_0 : vec2<f32> = (pos_0 - center_0.xy) / vec2<f32>(radius_0);
    var dist2_0 : f32 = dot(norm_0, norm_0);
    if(dist2_0 < 1.0f)
    {
        var h_1 : f32 = sqrt(1.0f - dist2_0);
        var srcDepth_0 : f32 = center_0.z - h_1 * radius_0;
        if(srcDepth_0 < ((*gbuffer_0).depth_0))
        {
            (*gbuffer_0).depth_0 = srcDepth_0;
            (*gbuffer_0).normal_0 = normalize(vec3<f32>(norm_0, h_1));
            (*gbuffer_0).baseColor_0 = baseColor_1;
            (*gbuffer_0).metallic_0 = metallic_1;
            (*gbuffer_0).linearRoughness_0 = linearRoughness_1;
        }
    }
    return;
}

fn scene_0( uv_0 : vec2<f32>) -> GBuffer_0
{
    var gbuffer_1 : GBuffer_0;
    gbuffer_1.depth_0 = 1.0f;
    var _S19 : vec3<f32> = vec3<f32>(vec3<i32>(i32(0)));
    gbuffer_1.normal_0 = _S19;
    gbuffer_1.baseColor_0 = _S19;
    gbuffer_1.metallic_0 = 0.0f;
    gbuffer_1.linearRoughness_0 = 0.0f;
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.40000000596046448f, 0.34999999403953552f, 0.63999998569488525f), 0.25f, vec3<f32>(1.0f, 0.30000001192092896f, 0.30000001192092896f), 0.20000000298023224f, 0.0f);
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.60000002384185791f, 0.44999998807907104f, 0.5f), 0.17000000178813934f, vec3<f32>(1.0f, 1.0f, 0.20000000298023224f), 0.44999998807907104f, 1.0f);
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.64999997615814209f, 0.75f, 0.40000000596046448f), 0.30000001192092896f, vec3<f32>(0.30000001192092896f, 1.0f, 0.30000001192092896f), 0.00999999977648258f, 0.0f);
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.34999999403953552f, 0.64999997615814209f, 0.40000000596046448f), 0.20000000298023224f, vec3<f32>(1.0f, 1.0f, 1.0f), 0.10000000149011612f, 0.0f);
    return gbuffer_1;
}

fn _d_abs_0( dpx_4 : ptr<function, DiffPair_float_0>,  dOut_2 : f32)
{
    var _S20 : f32 = select(select(-1.0f, 0.0f, ((*dpx_4).primal_0) == 0.0f), 1.0f, ((*dpx_4).primal_0) > 0.0f) * dOut_2;
    (*dpx_4).primal_0 = (*dpx_4).primal_0;
    (*dpx_4).differential_0 = _S20;
    return;
}

fn _d_abs_1( dpx_5 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S21 : DiffPair_float_0 = DiffPair_float_0( abs(dpx_5.primal_0), select(select(-1.0f, 0.0f, (dpx_5.primal_0) == 0.0f), 1.0f, (dpx_5.primal_0) > 0.0f) * dpx_5.differential_0 );
    return _S21;
}

fn _d_saturate_0( dpx_6 : ptr<function, DiffPair_float_0>,  dOut_3 : f32)
{
    var _S22 : DiffPair_float_0 = (*dpx_6);
    var _S23 : bool;
    if(((*dpx_6).primal_0) < 0.0f)
    {
        _S23 = true;
    }
    else
    {
        _S23 = ((*dpx_6).primal_0) > 1.0f;
    }
    var _S24 : f32 = select(dOut_3, 0.0f, _S23);
    (*dpx_6).primal_0 = _S22.primal_0;
    (*dpx_6).differential_0 = _S24;
    return;
}

fn _d_saturate_1( dpx_7 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S25 : f32 = saturate(dpx_7.primal_0);
    var _S26 : bool;
    if((dpx_7.primal_0) < 0.0f)
    {
        _S26 = true;
    }
    else
    {
        _S26 = (dpx_7.primal_0) > 1.0f;
    }
    var _S27 : DiffPair_float_0 = DiffPair_float_0( _S25, select(dpx_7.differential_0, 0.0f, _S26) );
    return _S27;
}

fn D_GGX_0( linearRoughness_2 : f32,  NoH_0 : f32,  h_2 : vec3<f32>) -> f32
{
    var a_2 : f32 = NoH_0 * linearRoughness_2;
    var k_0 : f32 = linearRoughness_2 / (1.0f - NoH_0 * NoH_0 + a_2 * a_2 + 0.00999999977648258f);
    return k_0 * k_0 * 0.31830987334251404f;
}

fn V_SmithGGXCorrelated_0( linearRoughness_3 : f32,  NoV_0 : f32,  NoL_0 : f32) -> f32
{
    var a2_0 : f32 = linearRoughness_3 * linearRoughness_3;
    return 0.5f / (NoL_0 * sqrt((NoV_0 - a2_0 * NoV_0) * NoV_0 + a2_0) + NoV_0 * sqrt((NoL_0 - a2_0 * NoL_0) * NoL_0 + a2_0));
}

fn pow5_0( x_0 : f32) -> f32
{
    var x2_0 : f32 = x_0 * x_0;
    return x2_0 * x2_0 * x_0;
}

fn F_Schlick_0( f0_0 : vec3<f32>,  VoH_0 : f32) -> vec3<f32>
{
    return f0_0 + (vec3<f32>(1.0f) - f0_0) * vec3<f32>(pow5_0(1.0f - VoH_0));
}

fn F_Schlick_1( f0_1 : f32,  f90_0 : f32,  VoH_1 : f32) -> f32
{
    return f0_1 + (f90_0 - f0_1) * pow5_0(1.0f - VoH_1);
}

fn Fd_Burley_0( linearRoughness_4 : f32,  NoV_1 : f32,  NoL_1 : f32,  LoH_0 : f32) -> f32
{
    var f90_1 : f32 = 0.5f + 2.0f * linearRoughness_4 * LoH_0 * LoH_0;
    return F_Schlick_1(1.0f, f90_1, NoL_1) * F_Schlick_1(1.0f, f90_1, NoV_1) * 0.31830987334251404f;
}

fn BRDF_0( uv_1 : vec2<f32>,  gbuffer_2 : GBuffer_0,  lightDirection_0 : vec3<f32>,  viewDirection_0 : vec3<f32>) -> vec3<f32>
{
    var _S28 : vec3<f32> = normalize(lightDirection_0);
    var _S29 : f32 = 1.0f - gbuffer_2.metallic_0;
    var _S30 : vec3<f32> = gbuffer_2.baseColor_0.xyz;
    var v_0 : vec3<f32> = normalize(- viewDirection_0);
    var n_1 : vec3<f32> = normalize(gbuffer_2.normal_0);
    var h_3 : vec3<f32> = normalize(v_0 + _S28);
    var NoV_2 : f32 = abs(dot(n_1, v_0)) + 0.00000999999974738f;
    var NoL_2 : f32 = saturate(dot(n_1, _S28));
    var LoH_1 : f32 = saturate(dot(_S28, h_3));
    return (vec3<f32>(_S29) * _S30 * vec3<f32>(Fd_Burley_0(gbuffer_2.linearRoughness_0, NoV_2, NoL_2, LoH_1)) + vec3<f32>((D_GGX_0(gbuffer_2.linearRoughness_0, saturate(dot(n_1, h_3)), h_3) * V_SmithGGXCorrelated_0(gbuffer_2.linearRoughness_0, NoV_2, NoL_2))) * F_Schlick_0(vec3<f32>((0.03999999910593033f * _S29)) + _S30 * vec3<f32>(gbuffer_2.metallic_0), LoH_1)) * (vec3<f32>(NoL_2) * (vec3<f32>(0.98000001907348633f, 0.92000001668930054f, 0.88999998569488525f) * vec3<f32>(2.0f)));
}

fn ComputeColorError_0( a_3 : vec3<f32>,  b_2 : vec3<f32>) -> f32
{
    var delta_0 : vec3<f32> = a_3 - b_2;
    return dot(delta_0, delta_0);
}

fn ComputeImageError_0( uv_2 : vec2<f32>,  lightDirection_1 : vec3<f32>,  viewDirection_1 : vec3<f32>,  stateA_0 : vec4<f32>,  stateB_0 : vec4<f32>,  targetColor_0 : vec3<f32>,  smoothStateA_0 : vec4<f32>,  smoothStateB_0 : vec4<f32>) -> f32
{
    var gbuffer_3 : GBuffer_0;
    gbuffer_3.depth_0 = 0.5f;
    gbuffer_3.normal_0 = normalize(stateB_0.xyz * vec3<f32>(2.0f) - vec3<f32>(1.0f));
    gbuffer_3.baseColor_0 = stateA_0.xyz;
    gbuffer_3.metallic_0 = stateB_0.w;
    gbuffer_3.linearRoughness_0 = stateA_0.w;
    return ComputeColorError_0(BRDF_0(uv_2, gbuffer_3, lightDirection_1, viewDirection_1), targetColor_0);
}

fn indexToMask_0( i_0 : i32) -> vec4<f32>
{
    var ret_0 : vec4<f32> = vec4<f32>(vec4<i32>(i32(0)));
    ret_0[i_0] = 0.10000000149011612f;
    return ret_0;
}

struct DiffPair_vectorx3Cfloatx2C4x3E_0
{
     primal_0 : vec4<f32>,
     differential_0 : vec4<f32>,
};

fn s_primal_ctx_dot_0( _S31 : vec3<f32>,  _S32 : vec3<f32>) -> f32
{
    return dot(_S31, _S32);
}

fn s_primal_ctx_abs_0( _S33 : f32) -> f32
{
    return abs(_S33);
}

fn s_primal_ctx_saturate_0( _S34 : f32) -> f32
{
    return saturate(_S34);
}

fn s_primal_ctx_D_GGX_0( dplinearRoughness_0 : f32,  dpNoH_0 : f32,  dph_0 : vec3<f32>) -> f32
{
    var a_4 : f32 = dpNoH_0 * dplinearRoughness_0;
    var k_1 : f32 = dplinearRoughness_0 / (1.0f - dpNoH_0 * dpNoH_0 + a_4 * a_4 + 0.00999999977648258f);
    return k_1 * k_1 * 0.31830987334251404f;
}

fn s_primal_ctx_sqrt_0( _S35 : f32) -> f32
{
    return sqrt(_S35);
}

fn s_primal_ctx_V_SmithGGXCorrelated_0( dplinearRoughness_1 : f32,  dpNoV_0 : f32,  dpNoL_0 : f32) -> f32
{
    var a2_1 : f32 = dplinearRoughness_1 * dplinearRoughness_1;
    return 0.5f / (dpNoL_0 * s_primal_ctx_sqrt_0((dpNoV_0 - a2_1 * dpNoV_0) * dpNoV_0 + a2_1) + dpNoV_0 * s_primal_ctx_sqrt_0((dpNoL_0 - a2_1 * dpNoL_0) * dpNoL_0 + a2_1));
}

fn s_primal_ctx_pow5_0( dpx_8 : f32) -> f32
{
    var x2_1 : f32 = dpx_8 * dpx_8;
    return x2_1 * x2_1 * dpx_8;
}

fn s_primal_ctx_F_Schlick_0( dpf0_0 : vec3<f32>,  dpVoH_0 : f32) -> vec3<f32>
{
    return dpf0_0 + (vec3<f32>(1.0f) - dpf0_0) * vec3<f32>(s_primal_ctx_pow5_0(1.0f - dpVoH_0));
}

fn s_primal_ctx_F_Schlick_1( dpf0_1 : f32,  dpf90_0 : f32,  dpVoH_1 : f32) -> f32
{
    return dpf0_1 + (dpf90_0 - dpf0_1) * s_primal_ctx_pow5_0(1.0f - dpVoH_1);
}

fn s_primal_ctx_Fd_Burley_0( dplinearRoughness_2 : f32,  dpNoV_1 : f32,  dpNoL_1 : f32,  dpLoH_0 : f32) -> f32
{
    var f90_2 : f32 = 0.5f + 2.0f * dplinearRoughness_2 * dpLoH_0 * dpLoH_0;
    return s_primal_ctx_F_Schlick_1(1.0f, f90_2, dpNoL_1) * s_primal_ctx_F_Schlick_1(1.0f, f90_2, dpNoV_1) * 0.31830987334251404f;
}

fn s_primal_ctx_BRDF_0( dpuv_0 : vec2<f32>,  dpgbuffer_0 : GBuffer_0,  lightDirection_2 : vec3<f32>,  viewDirection_2 : vec3<f32>) -> vec3<f32>
{
    var _S36 : vec3<f32> = normalize(lightDirection_2);
    var _S37 : f32 = 1.0f - dpgbuffer_0.metallic_0;
    var _S38 : vec3<f32> = dpgbuffer_0.baseColor_0.xyz;
    var _S39 : vec3<f32> = normalize(- viewDirection_2);
    var _S40 : vec3<f32> = normalize(dpgbuffer_0.normal_0);
    var _S41 : vec3<f32> = normalize(_S39 + _S36);
    var NoV_3 : f32 = s_primal_ctx_abs_0(s_primal_ctx_dot_0(_S40, _S39)) + 0.00000999999974738f;
    var _S42 : f32 = s_primal_ctx_saturate_0(s_primal_ctx_dot_0(_S40, _S36));
    var _S43 : f32 = s_primal_ctx_saturate_0(s_primal_ctx_dot_0(_S36, _S41));
    return (vec3<f32>(_S37) * _S38 * vec3<f32>(s_primal_ctx_Fd_Burley_0(dpgbuffer_0.linearRoughness_0, NoV_3, _S42, _S43)) + vec3<f32>((s_primal_ctx_D_GGX_0(dpgbuffer_0.linearRoughness_0, s_primal_ctx_saturate_0(s_primal_ctx_dot_0(_S40, _S41)), _S41) * s_primal_ctx_V_SmithGGXCorrelated_0(dpgbuffer_0.linearRoughness_0, NoV_3, _S42))) * s_primal_ctx_F_Schlick_0(vec3<f32>((0.03999999910593033f * _S37)) + _S38 * vec3<f32>(dpgbuffer_0.metallic_0), _S43)) * (vec3<f32>(_S42) * (vec3<f32>(0.98000001907348633f, 0.92000001668930054f, 0.88999998569488525f) * vec3<f32>(2.0f)));
}

fn s_bwd_prop_dot_0( _S44 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  _S45 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  _S46 : f32)
{
    _d_dot_0(&((*_S44)), &((*_S45)), _S46);
    return;
}

fn s_bwd_prop_ComputeColorError_0( dpa_0 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  dpb_0 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  _s_dOut_0 : f32)
{
    var delta_1 : vec3<f32> = (*dpa_0).primal_0 - (*dpb_0).primal_0;
    var _S47 : vec3<f32> = vec3<f32>(0.0f);
    var _S48 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S48.primal_0 = delta_1;
    _S48.differential_0 = _S47;
    var _S49 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S49.primal_0 = delta_1;
    _S49.differential_0 = _S47;
    s_bwd_prop_dot_0(&(_S48), &(_S49), _s_dOut_0);
    var _S50 : vec3<f32> = _S49.differential_0 + _S48.differential_0;
    var _S51 : vec3<f32> = - _S50;
    (*dpb_0).primal_0 = (*dpb_0).primal_0;
    (*dpb_0).differential_0 = _S51;
    (*dpa_0).primal_0 = (*dpa_0).primal_0;
    (*dpa_0).differential_0 = _S50;
    return;
}

struct DiffPair_vectorx3Cfloatx2C2x3E_0
{
     primal_0 : vec2<f32>,
     differential_0 : vec2<f32>,
};

fn GBuffer_x24_syn_dzero_0() -> GBuffer_0
{
    var result_0 : GBuffer_0;
    result_0.depth_0 = 0.0f;
    var _S52 : vec3<f32> = vec3<f32>(0.0f);
    result_0.normal_0 = _S52;
    result_0.baseColor_0 = _S52;
    result_0.metallic_0 = 0.0f;
    result_0.linearRoughness_0 = 0.0f;
    return result_0;
}

struct DiffPair_GBuffer_0
{
     primal_0 : GBuffer_0,
     differential_0 : GBuffer_0,
};

fn s_bwd_prop_pow5_0( dpx_9 : ptr<function, DiffPair_float_0>,  _s_dOut_1 : f32)
{
    var _S53 : f32 = (*dpx_9).primal_0;
    var x2_2 : f32 = _S53 * _S53;
    var _S54 : f32 = x2_2 * ((*dpx_9).primal_0 * _s_dOut_1);
    var _S55 : f32 = (*dpx_9).primal_0 * (_S54 + _S54);
    var _S56 : f32 = x2_2 * x2_2 * _s_dOut_1 + _S55 + _S55;
    (*dpx_9).primal_0 = (*dpx_9).primal_0;
    (*dpx_9).differential_0 = _S56;
    return;
}

fn s_bwd_prop_F_Schlick_0( dpf0_2 : ptr<function, DiffPair_float_0>,  dpf90_1 : ptr<function, DiffPair_float_0>,  dpVoH_2 : ptr<function, DiffPair_float_0>,  _s_dOut_2 : f32)
{
    var _S57 : f32 = 1.0f - (*dpVoH_2).primal_0;
    var _S58 : f32 = ((*dpf90_1).primal_0 - (*dpf0_2).primal_0) * _s_dOut_2;
    var _S59 : f32 = s_primal_ctx_pow5_0(_S57) * _s_dOut_2;
    var _S60 : DiffPair_float_0;
    _S60.primal_0 = _S57;
    _S60.differential_0 = 0.0f;
    s_bwd_prop_pow5_0(&(_S60), _S58);
    var _S61 : f32 = - _S60.differential_0;
    var _S62 : f32 = - _S59;
    (*dpVoH_2).primal_0 = (*dpVoH_2).primal_0;
    (*dpVoH_2).differential_0 = _S61;
    (*dpf90_1).primal_0 = (*dpf90_1).primal_0;
    (*dpf90_1).differential_0 = _S59;
    var _S63 : f32 = _s_dOut_2 + _S62;
    (*dpf0_2).primal_0 = (*dpf0_2).primal_0;
    (*dpf0_2).differential_0 = _S63;
    return;
}

fn s_bwd_prop_Fd_Burley_0( dplinearRoughness_3 : ptr<function, DiffPair_float_0>,  dpNoV_2 : ptr<function, DiffPair_float_0>,  dpNoL_2 : ptr<function, DiffPair_float_0>,  dpLoH_1 : ptr<function, DiffPair_float_0>,  _s_dOut_3 : f32)
{
    var _S64 : f32 = 2.0f * (*dplinearRoughness_3).primal_0;
    var _S65 : f32 = _S64 * (*dpLoH_1).primal_0;
    var f90_3 : f32 = 0.5f + _S65 * (*dpLoH_1).primal_0;
    var _S66 : f32 = 0.31830987334251404f * _s_dOut_3;
    var _S67 : f32 = s_primal_ctx_F_Schlick_1(1.0f, f90_3, (*dpNoL_2).primal_0) * _S66;
    var _S68 : f32 = s_primal_ctx_F_Schlick_1(1.0f, f90_3, (*dpNoV_2).primal_0) * _S66;
    var _S69 : DiffPair_float_0;
    _S69.primal_0 = 1.0f;
    _S69.differential_0 = 0.0f;
    var _S70 : DiffPair_float_0;
    _S70.primal_0 = f90_3;
    _S70.differential_0 = 0.0f;
    var _S71 : DiffPair_float_0;
    _S71.primal_0 = (*dpNoV_2).primal_0;
    _S71.differential_0 = 0.0f;
    s_bwd_prop_F_Schlick_0(&(_S69), &(_S70), &(_S71), _S67);
    var _S72 : DiffPair_float_0;
    _S72.primal_0 = 1.0f;
    _S72.differential_0 = 0.0f;
    var _S73 : DiffPair_float_0;
    _S73.primal_0 = f90_3;
    _S73.differential_0 = 0.0f;
    var _S74 : DiffPair_float_0;
    _S74.primal_0 = (*dpNoL_2).primal_0;
    _S74.differential_0 = 0.0f;
    s_bwd_prop_F_Schlick_0(&(_S72), &(_S73), &(_S74), _S68);
    var _S75 : f32 = _S70.differential_0 + _S73.differential_0;
    var _S76 : f32 = (*dpLoH_1).primal_0 * _S75;
    var _S77 : f32 = 2.0f * ((*dpLoH_1).primal_0 * _S76);
    var _S78 : f32 = _S65 * _S75 + _S64 * _S76;
    (*dpLoH_1).primal_0 = (*dpLoH_1).primal_0;
    (*dpLoH_1).differential_0 = _S78;
    (*dpNoL_2).primal_0 = (*dpNoL_2).primal_0;
    (*dpNoL_2).differential_0 = _S74.differential_0;
    (*dpNoV_2).primal_0 = (*dpNoV_2).primal_0;
    (*dpNoV_2).differential_0 = _S71.differential_0;
    (*dplinearRoughness_3).primal_0 = (*dplinearRoughness_3).primal_0;
    (*dplinearRoughness_3).differential_0 = _S77;
    return;
}

fn s_bwd_prop_F_Schlick_1( dpf0_3 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  dpVoH_3 : ptr<function, DiffPair_float_0>,  _s_dOut_4 : vec3<f32>)
{
    var _S79 : f32 = 1.0f - (*dpVoH_3).primal_0;
    var _S80 : vec3<f32> = (vec3<f32>(1.0f) - (*dpf0_3).primal_0) * _s_dOut_4;
    var _S81 : vec3<f32> = vec3<f32>(s_primal_ctx_pow5_0(_S79)) * _s_dOut_4;
    var _S82 : f32 = _S80[i32(0)] + _S80[i32(1)] + _S80[i32(2)];
    var _S83 : DiffPair_float_0;
    _S83.primal_0 = _S79;
    _S83.differential_0 = 0.0f;
    s_bwd_prop_pow5_0(&(_S83), _S82);
    var _S84 : f32 = - _S83.differential_0;
    var _S85 : vec3<f32> = - _S81;
    (*dpVoH_3).primal_0 = (*dpVoH_3).primal_0;
    (*dpVoH_3).differential_0 = _S84;
    var _S86 : vec3<f32> = _s_dOut_4 + _S85;
    (*dpf0_3).primal_0 = (*dpf0_3).primal_0;
    (*dpf0_3).differential_0 = _S86;
    return;
}

fn s_bwd_prop_sqrt_0( _S87 : ptr<function, DiffPair_float_0>,  _S88 : f32)
{
    _d_sqrt_0(&((*_S87)), _S88);
    return;
}

fn s_bwd_prop_V_SmithGGXCorrelated_0( dplinearRoughness_4 : ptr<function, DiffPair_float_0>,  dpNoV_3 : ptr<function, DiffPair_float_0>,  dpNoL_3 : ptr<function, DiffPair_float_0>,  _s_dOut_5 : f32)
{
    var a2_2 : f32 = (*dplinearRoughness_4).primal_0 * (*dplinearRoughness_4).primal_0;
    var _S89 : f32 = (*dpNoV_3).primal_0 - a2_2 * (*dpNoV_3).primal_0;
    var _S90 : f32 = _S89 * (*dpNoV_3).primal_0 + a2_2;
    var _S91 : f32 = s_primal_ctx_sqrt_0(_S90);
    var _S92 : f32 = (*dpNoL_3).primal_0 - a2_2 * (*dpNoL_3).primal_0;
    var _S93 : f32 = _S92 * (*dpNoL_3).primal_0 + a2_2;
    var _S94 : f32 = s_primal_ctx_sqrt_0(_S93);
    var _S95 : f32 = (*dpNoL_3).primal_0 * _S91 + (*dpNoV_3).primal_0 * _S94;
    var s_diff_GGXV_T_0 : f32 = 0.5f * - (_s_dOut_5 / (_S95 * _S95));
    var _S96 : f32 = (*dpNoV_3).primal_0 * s_diff_GGXV_T_0;
    var _S97 : f32 = _S94 * s_diff_GGXV_T_0;
    var _S98 : DiffPair_float_0;
    _S98.primal_0 = _S93;
    _S98.differential_0 = 0.0f;
    s_bwd_prop_sqrt_0(&(_S98), _S96);
    var _S99 : f32 = _S92 * _S98.differential_0;
    var _S100 : f32 = (*dpNoL_3).primal_0 * _S98.differential_0;
    var _S101 : f32 = - _S100;
    var _S102 : f32 = a2_2 * _S101;
    var _S103 : f32 = (*dpNoL_3).primal_0 * _S101;
    var _S104 : f32 = (*dpNoL_3).primal_0 * s_diff_GGXV_T_0;
    var _S105 : f32 = _S91 * s_diff_GGXV_T_0;
    var _S106 : DiffPair_float_0;
    _S106.primal_0 = _S90;
    _S106.differential_0 = 0.0f;
    s_bwd_prop_sqrt_0(&(_S106), _S104);
    var _S107 : f32 = _S89 * _S106.differential_0;
    var _S108 : f32 = (*dpNoV_3).primal_0 * _S106.differential_0;
    var _S109 : f32 = - _S108;
    var _S110 : f32 = a2_2 * _S109;
    var _S111 : f32 = (*dplinearRoughness_4).primal_0 * (_S98.differential_0 + _S103 + _S106.differential_0 + (*dpNoV_3).primal_0 * _S109);
    var _S112 : f32 = _S99 + _S100 + _S102 + _S105;
    (*dpNoL_3).primal_0 = (*dpNoL_3).primal_0;
    (*dpNoL_3).differential_0 = _S112;
    var _S113 : f32 = _S97 + _S107 + _S108 + _S110;
    (*dpNoV_3).primal_0 = (*dpNoV_3).primal_0;
    (*dpNoV_3).differential_0 = _S113;
    var _S114 : f32 = _S111 + _S111;
    (*dplinearRoughness_4).primal_0 = (*dplinearRoughness_4).primal_0;
    (*dplinearRoughness_4).differential_0 = _S114;
    return;
}

fn s_bwd_prop_D_GGX_0( dplinearRoughness_5 : ptr<function, DiffPair_float_0>,  dpNoH_1 : ptr<function, DiffPair_float_0>,  dph_1 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  s_diff_d_T_0 : f32)
{
    var _S115 : f32 = (*dpNoH_1).primal_0;
    var a_5 : f32 = (*dpNoH_1).primal_0 * (*dplinearRoughness_5).primal_0;
    var _S116 : f32 = 1.0f - _S115 * _S115 + a_5 * a_5 + 0.00999999977648258f;
    var _S117 : f32 = (*dplinearRoughness_5).primal_0 / _S116 * (0.31830987334251404f * s_diff_d_T_0);
    var _S118 : f32 = (_S117 + _S117) / (_S116 * _S116);
    var s_diff_oneMinusNoHSquared_T_0 : f32 = (*dplinearRoughness_5).primal_0 * - _S118;
    var _S119 : f32 = _S116 * _S118;
    var _S120 : f32 = a_5 * s_diff_oneMinusNoHSquared_T_0;
    var _S121 : f32 = _S120 + _S120;
    var _S122 : f32 = (*dpNoH_1).primal_0 * _S121;
    var _S123 : f32 = (*dplinearRoughness_5).primal_0 * _S121;
    var _S124 : f32 = (*dpNoH_1).primal_0 * - s_diff_oneMinusNoHSquared_T_0;
    var _S125 : vec3<f32> = vec3<f32>(0.0f);
    (*dph_1).primal_0 = (*dph_1).primal_0;
    (*dph_1).differential_0 = _S125;
    var _S126 : f32 = _S123 + _S124 + _S124;
    (*dpNoH_1).primal_0 = (*dpNoH_1).primal_0;
    (*dpNoH_1).differential_0 = _S126;
    var _S127 : f32 = _S119 + _S122;
    (*dplinearRoughness_5).primal_0 = (*dplinearRoughness_5).primal_0;
    (*dplinearRoughness_5).differential_0 = _S127;
    return;
}

fn s_bwd_prop_saturate_0( _S128 : ptr<function, DiffPair_float_0>,  _S129 : f32)
{
    _d_saturate_0(&((*_S128)), _S129);
    return;
}

fn s_bwd_prop_abs_0( _S130 : ptr<function, DiffPair_float_0>,  _S131 : f32)
{
    _d_abs_0(&((*_S130)), _S131);
    return;
}

fn s_bwd_prop_length_impl_0( dpx_10 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  _s_dOut_6 : f32)
{
    var _S132 : f32 = (*dpx_10).primal_0[i32(0)];
    var _S133 : f32 = (*dpx_10).primal_0[i32(1)];
    var _S134 : f32 = (*dpx_10).primal_0[i32(2)];
    var _S135 : DiffPair_float_0;
    _S135.primal_0 = _S132 * _S132 + _S133 * _S133 + _S134 * _S134;
    _S135.differential_0 = 0.0f;
    s_bwd_prop_sqrt_0(&(_S135), _s_dOut_6);
    var _S136 : f32 = (*dpx_10).primal_0[i32(2)] * _S135.differential_0;
    var _S137 : f32 = _S136 + _S136;
    var _S138 : f32 = (*dpx_10).primal_0[i32(1)] * _S135.differential_0;
    var _S139 : f32 = _S138 + _S138;
    var _S140 : f32 = (*dpx_10).primal_0[i32(0)] * _S135.differential_0;
    var _S141 : f32 = _S140 + _S140;
    var _S142 : vec3<f32> = vec3<f32>(0.0f);
    _S142[i32(2)] = _S137;
    _S142[i32(1)] = _S139;
    _S142[i32(0)] = _S141;
    (*dpx_10).primal_0 = (*dpx_10).primal_0;
    (*dpx_10).differential_0 = _S142;
    return;
}

fn s_bwd_length_impl_0( _S143 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  _S144 : f32)
{
    s_bwd_prop_length_impl_0(&((*_S143)), _S144);
    return;
}

fn s_bwd_prop_normalize_impl_0( dpx_11 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  _s_dOut_7 : vec3<f32>)
{
    var _S145 : f32 = length((*dpx_11).primal_0);
    var _S146 : vec3<f32> = (*dpx_11).primal_0 * _s_dOut_7;
    var _S147 : vec3<f32> = vec3<f32>((1.0f / _S145)) * _s_dOut_7;
    var _S148 : f32 = - ((_S146[i32(0)] + _S146[i32(1)] + _S146[i32(2)]) / (_S145 * _S145));
    var _S149 : vec3<f32> = vec3<f32>(0.0f);
    var _S150 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S150.primal_0 = (*dpx_11).primal_0;
    _S150.differential_0 = _S149;
    s_bwd_length_impl_0(&(_S150), _S148);
    var _S151 : vec3<f32> = _S147 + _S150.differential_0;
    (*dpx_11).primal_0 = (*dpx_11).primal_0;
    (*dpx_11).differential_0 = _S151;
    return;
}

fn s_bwd_normalize_impl_0( _S152 : ptr<function, DiffPair_vectorx3Cfloatx2C3x3E_0>,  _S153 : vec3<f32>)
{
    s_bwd_prop_normalize_impl_0(&((*_S152)), _S153);
    return;
}

fn s_bwd_prop_BRDF_0( dpuv_1 : ptr<function, DiffPair_vectorx3Cfloatx2C2x3E_0>,  dpgbuffer_1 : ptr<function, DiffPair_GBuffer_0>,  lightDirection_3 : vec3<f32>,  viewDirection_3 : vec3<f32>,  s_diff_color_T_0 : vec3<f32>)
{
    var _S154 : vec3<f32> = normalize(lightDirection_3);
    var _S155 : vec3<f32> = vec3<f32>((*dpgbuffer_1).primal_0.metallic_0);
    var lightColor_0 : vec3<f32> = vec3<f32>(0.98000001907348633f, 0.92000001668930054f, 0.88999998569488525f) * vec3<f32>(2.0f);
    var _S156 : f32 = 1.0f - (*dpgbuffer_1).primal_0.metallic_0;
    var _S157 : vec3<f32> = vec3<f32>(_S156);
    var _S158 : vec3<f32> = (*dpgbuffer_1).primal_0.baseColor_0.xyz;
    var diffuseColor_0 : vec3<f32> = _S157 * _S158;
    var f0_2 : vec3<f32> = vec3<f32>((0.03999999910593033f * _S156)) + _S158 * _S155;
    var _S159 : vec3<f32> = normalize(- viewDirection_3);
    var _S160 : vec3<f32> = normalize((*dpgbuffer_1).primal_0.normal_0);
    var _S161 : vec3<f32> = normalize(_S159 + _S154);
    var _S162 : f32 = s_primal_ctx_dot_0(_S160, _S159);
    var NoV_4 : f32 = s_primal_ctx_abs_0(_S162) + 0.00000999999974738f;
    var _S163 : f32 = s_primal_ctx_dot_0(_S160, _S154);
    var _S164 : f32 = s_primal_ctx_saturate_0(_S163);
    var _S165 : f32 = s_primal_ctx_dot_0(_S160, _S161);
    var _S166 : f32 = s_primal_ctx_saturate_0(_S165);
    var _S167 : f32 = s_primal_ctx_saturate_0(s_primal_ctx_dot_0(_S154, _S161));
    var _S168 : f32 = s_primal_ctx_D_GGX_0((*dpgbuffer_1).primal_0.linearRoughness_0, _S166, _S161);
    var _S169 : f32 = s_primal_ctx_V_SmithGGXCorrelated_0((*dpgbuffer_1).primal_0.linearRoughness_0, NoV_4, _S164);
    var _S170 : vec3<f32> = s_primal_ctx_F_Schlick_0(f0_2, _S167);
    var _S171 : vec3<f32> = vec3<f32>((_S168 * _S169));
    var _S172 : vec3<f32> = vec3<f32>(s_primal_ctx_Fd_Burley_0((*dpgbuffer_1).primal_0.linearRoughness_0, NoV_4, _S164, _S167));
    var s_diff_Fr_T_0 : vec3<f32> = vec3<f32>(_S164) * lightColor_0 * s_diff_color_T_0;
    var _S173 : vec3<f32> = lightColor_0 * ((diffuseColor_0 * _S172 + _S171 * _S170) * s_diff_color_T_0);
    var _S174 : vec3<f32> = diffuseColor_0 * s_diff_Fr_T_0;
    var s_diff_diffuseColor_T_0 : vec3<f32> = _S172 * s_diff_Fr_T_0;
    var _S175 : f32 = _S174[i32(0)] + _S174[i32(1)] + _S174[i32(2)];
    var _S176 : DiffPair_float_0;
    _S176.primal_0 = (*dpgbuffer_1).primal_0.linearRoughness_0;
    _S176.differential_0 = 0.0f;
    var _S177 : DiffPair_float_0;
    _S177.primal_0 = NoV_4;
    _S177.differential_0 = 0.0f;
    var _S178 : DiffPair_float_0;
    _S178.primal_0 = _S164;
    _S178.differential_0 = 0.0f;
    var _S179 : DiffPair_float_0;
    _S179.primal_0 = _S167;
    _S179.differential_0 = 0.0f;
    s_bwd_prop_Fd_Burley_0(&(_S176), &(_S177), &(_S178), &(_S179), _S175);
    var _S180 : vec3<f32> = _S171 * s_diff_Fr_T_0;
    var _S181 : vec3<f32> = _S170 * s_diff_Fr_T_0;
    var _S182 : f32 = _S181[i32(0)] + _S181[i32(1)] + _S181[i32(2)];
    var _S183 : f32 = _S168 * _S182;
    var _S184 : f32 = _S169 * _S182;
    var _S185 : vec3<f32> = vec3<f32>(0.0f);
    var _S186 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S186.primal_0 = f0_2;
    _S186.differential_0 = _S185;
    var _S187 : DiffPair_float_0;
    _S187.primal_0 = _S167;
    _S187.differential_0 = 0.0f;
    s_bwd_prop_F_Schlick_1(&(_S186), &(_S187), _S180);
    var _S188 : DiffPair_float_0;
    _S188.primal_0 = (*dpgbuffer_1).primal_0.linearRoughness_0;
    _S188.differential_0 = 0.0f;
    var _S189 : DiffPair_float_0;
    _S189.primal_0 = NoV_4;
    _S189.differential_0 = 0.0f;
    var _S190 : DiffPair_float_0;
    _S190.primal_0 = _S164;
    _S190.differential_0 = 0.0f;
    s_bwd_prop_V_SmithGGXCorrelated_0(&(_S188), &(_S189), &(_S190), _S183);
    var _S191 : DiffPair_float_0;
    _S191.primal_0 = (*dpgbuffer_1).primal_0.linearRoughness_0;
    _S191.differential_0 = 0.0f;
    var _S192 : DiffPair_float_0;
    _S192.primal_0 = _S166;
    _S192.differential_0 = 0.0f;
    var _S193 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S193.primal_0 = _S161;
    _S193.differential_0 = _S185;
    s_bwd_prop_D_GGX_0(&(_S191), &(_S192), &(_S193), _S184);
    var _S194 : DiffPair_float_0;
    _S194.primal_0 = _S165;
    _S194.differential_0 = 0.0f;
    s_bwd_prop_saturate_0(&(_S194), _S192.differential_0);
    var _S195 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S195.primal_0 = _S160;
    _S195.differential_0 = _S185;
    var _S196 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S196.primal_0 = _S161;
    _S196.differential_0 = _S185;
    s_bwd_prop_dot_0(&(_S195), &(_S196), _S194.differential_0);
    var _S197 : f32 = _S178.differential_0 + _S190.differential_0 + _S173[i32(0)] + _S173[i32(1)] + _S173[i32(2)];
    var _S198 : DiffPair_float_0;
    _S198.primal_0 = _S163;
    _S198.differential_0 = 0.0f;
    s_bwd_prop_saturate_0(&(_S198), _S197);
    var _S199 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S199.primal_0 = _S160;
    _S199.differential_0 = _S185;
    var _S200 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S200.primal_0 = _S154;
    _S200.differential_0 = _S185;
    s_bwd_prop_dot_0(&(_S199), &(_S200), _S198.differential_0);
    var _S201 : f32 = _S177.differential_0 + _S189.differential_0;
    var _S202 : DiffPair_float_0;
    _S202.primal_0 = _S162;
    _S202.differential_0 = 0.0f;
    s_bwd_prop_abs_0(&(_S202), _S201);
    var _S203 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S203.primal_0 = _S160;
    _S203.differential_0 = _S185;
    var _S204 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S204.primal_0 = _S159;
    _S204.differential_0 = _S185;
    s_bwd_prop_dot_0(&(_S203), &(_S204), _S202.differential_0);
    var _S205 : vec3<f32> = _S195.differential_0 + _S199.differential_0 + _S203.differential_0;
    var _S206 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S206.primal_0 = (*dpgbuffer_1).primal_0.normal_0;
    _S206.differential_0 = _S185;
    s_bwd_normalize_impl_0(&(_S206), _S205);
    var _S207 : vec3<f32> = _S158 * _S186.differential_0;
    var _S208 : vec3<f32> = _S158 * s_diff_diffuseColor_T_0;
    var _S209 : vec3<f32> = _S155 * _S186.differential_0 + _S157 * s_diff_diffuseColor_T_0;
    var _S210 : f32 = _S176.differential_0 + _S188.differential_0 + _S191.differential_0;
    var _S211 : f32 = - (0.03999999910593033f * (_S186.differential_0[i32(0)] + _S186.differential_0[i32(1)] + _S186.differential_0[i32(2)]) + _S208[i32(0)] + _S208[i32(1)] + _S208[i32(2)]) + _S207[i32(0)] + _S207[i32(1)] + _S207[i32(2)];
    var _S212 : vec3<f32> = vec3<f32>(_S209[i32(0)], _S209[i32(1)], _S209[i32(2)]);
    var _S213 : GBuffer_0 = GBuffer_x24_syn_dzero_0();
    _S213.normal_0 = _S206.differential_0;
    _S213.linearRoughness_0 = _S210;
    _S213.metallic_0 = _S211;
    _S213.baseColor_0 = _S212;
    (*dpgbuffer_1).primal_0 = (*dpgbuffer_1).primal_0;
    (*dpgbuffer_1).differential_0 = _S213;
    var _S214 : vec2<f32> = vec2<f32>(0.0f);
    (*dpuv_1).primal_0 = (*dpuv_1).primal_0;
    (*dpuv_1).differential_0 = _S214;
    return;
}

fn s_bwd_prop_ComputeImageError_0( uv_3 : vec2<f32>,  lightDirection_4 : vec3<f32>,  viewDirection_4 : vec3<f32>,  dpstateA_0 : ptr<function, DiffPair_vectorx3Cfloatx2C4x3E_0>,  dpstateB_0 : ptr<function, DiffPair_vectorx3Cfloatx2C4x3E_0>,  targetColor_1 : vec3<f32>,  smoothStateA_1 : vec4<f32>,  smoothStateB_1 : vec4<f32>,  _s_dOut_8 : f32)
{
    var _S215 : vec3<f32> = (*dpstateB_0).primal_0.xyz * vec3<f32>(2.0f) - vec3<f32>(1.0f);
    var _S216 : GBuffer_0 = GBuffer_0( 0.5f, normalize(_S215), (*dpstateA_0).primal_0.xyz, (*dpstateB_0).primal_0.w, (*dpstateA_0).primal_0.w );
    var _S217 : vec3<f32> = vec3<f32>(0.0f);
    var _S218 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S218.primal_0 = s_primal_ctx_BRDF_0(uv_3, _S216, lightDirection_4, viewDirection_4);
    _S218.differential_0 = _S217;
    var _S219 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S219.primal_0 = targetColor_1;
    _S219.differential_0 = _S217;
    s_bwd_prop_ComputeColorError_0(&(_S218), &(_S219), _s_dOut_8);
    var _S220 : vec2<f32> = vec2<f32>(0.0f);
    var _S221 : DiffPair_vectorx3Cfloatx2C2x3E_0;
    _S221.primal_0 = uv_3;
    _S221.differential_0 = _S220;
    var _S222 : GBuffer_0 = GBuffer_x24_syn_dzero_0();
    var _S223 : DiffPair_GBuffer_0;
    _S223.primal_0 = _S216;
    _S223.differential_0 = _S222;
    s_bwd_prop_BRDF_0(&(_S221), &(_S223), lightDirection_4, viewDirection_4, _S218.differential_0);
    var _S224 : DiffPair_vectorx3Cfloatx2C3x3E_0;
    _S224.primal_0 = _S215;
    _S224.differential_0 = _S217;
    s_bwd_normalize_impl_0(&(_S224), _S223.differential_0.normal_0);
    var _S225 : vec3<f32> = vec3<f32>(2.0f) * _S224.differential_0;
    var _S226 : vec4<f32> = vec4<f32>(_S225[i32(0)], _S225[i32(1)], _S225[i32(2)], _S223.differential_0.metallic_0);
    (*dpstateB_0).primal_0 = (*dpstateB_0).primal_0;
    (*dpstateB_0).differential_0 = _S226;
    var _S227 : vec4<f32> = vec4<f32>(_S223.differential_0.baseColor_0[i32(0)], _S223.differential_0.baseColor_0[i32(1)], _S223.differential_0.baseColor_0[i32(2)], _S223.differential_0.linearRoughness_0);
    (*dpstateA_0).primal_0 = (*dpstateA_0).primal_0;
    (*dpstateA_0).differential_0 = _S227;
    return;
}

fn s_bwd_ComputeImageError_0( _S228 : vec2<f32>,  _S229 : vec3<f32>,  _S230 : vec3<f32>,  _S231 : ptr<function, DiffPair_vectorx3Cfloatx2C4x3E_0>,  _S232 : ptr<function, DiffPair_vectorx3Cfloatx2C4x3E_0>,  _S233 : vec3<f32>,  _S234 : vec4<f32>,  _S235 : vec4<f32>,  _S236 : f32)
{
    s_bwd_prop_ComputeImageError_0(_S228, _S229, _S230, &((*_S231)), &((*_S232)), _S233, _S234, _S235, _S236);
    return;
}

fn s_fwd_length_impl_0( dpx_12 : DiffPair_vectorx3Cfloatx2C3x3E_0) -> DiffPair_float_0
{
    var _S237 : f32 = dpx_12.primal_0[i32(0)];
    var _S238 : f32 = dpx_12.differential_0[i32(0)] * dpx_12.primal_0[i32(0)];
    var _S239 : f32 = dpx_12.primal_0[i32(1)];
    var _S240 : f32 = dpx_12.differential_0[i32(1)] * dpx_12.primal_0[i32(1)];
    var _S241 : f32 = dpx_12.primal_0[i32(2)];
    var _S242 : f32 = dpx_12.differential_0[i32(2)] * dpx_12.primal_0[i32(2)];
    var _S243 : DiffPair_float_0 = DiffPair_float_0( _S237 * _S237 + _S239 * _S239 + _S241 * _S241, _S238 + _S238 + (_S240 + _S240) + (_S242 + _S242) );
    var _S244 : DiffPair_float_0 = _d_sqrt_1(_S243);
    var _S245 : DiffPair_float_0 = DiffPair_float_0( _S244.primal_0, _S244.differential_0 );
    return _S245;
}

fn s_fwd_normalize_impl_0( dpx_13 : DiffPair_vectorx3Cfloatx2C3x3E_0) -> DiffPair_vectorx3Cfloatx2C3x3E_0
{
    var _S246 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( dpx_13.primal_0, dpx_13.differential_0 );
    var _S247 : DiffPair_float_0 = s_fwd_length_impl_0(_S246);
    var _S248 : vec3<f32> = vec3<f32>((1.0f / _S247.primal_0));
    var _S249 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( dpx_13.primal_0 * _S248, dpx_13.differential_0 * _S248 + vec3<f32>(((0.0f - _S247.differential_0) / (_S247.primal_0 * _S247.primal_0))) * dpx_13.primal_0 );
    return _S249;
}

fn s_fwd_D_GGX_0( dplinearRoughness_6 : DiffPair_float_0,  dpNoH_2 : DiffPair_float_0,  dph_2 : DiffPair_vectorx3Cfloatx2C3x3E_0) -> DiffPair_float_0
{
    var _S250 : f32 = dpNoH_2.primal_0;
    var _S251 : f32 = dpNoH_2.differential_0 * dpNoH_2.primal_0;
    var a_6 : f32 = dpNoH_2.primal_0 * dplinearRoughness_6.primal_0;
    var _S252 : f32 = (dpNoH_2.differential_0 * dplinearRoughness_6.primal_0 + dplinearRoughness_6.differential_0 * dpNoH_2.primal_0) * a_6;
    var _S253 : f32 = 1.0f - _S250 * _S250 + a_6 * a_6 + 0.00999999977648258f;
    var k_2 : f32 = dplinearRoughness_6.primal_0 / _S253;
    var _S254 : f32 = (dplinearRoughness_6.differential_0 * _S253 - dplinearRoughness_6.primal_0 * (0.0f - (_S251 + _S251) + (_S252 + _S252))) / (_S253 * _S253) * k_2;
    var _S255 : DiffPair_float_0 = DiffPair_float_0( k_2 * k_2 * 0.31830987334251404f, (_S254 + _S254) * 0.31830987334251404f );
    return _S255;
}

fn s_fwd_V_SmithGGXCorrelated_0( dplinearRoughness_7 : DiffPair_float_0,  dpNoV_4 : DiffPair_float_0,  dpNoL_4 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S256 : f32 = dplinearRoughness_7.primal_0;
    var a2_3 : f32 = _S256 * _S256;
    var _S257 : f32 = dplinearRoughness_7.differential_0 * dplinearRoughness_7.primal_0;
    var s_diff_a2_0 : f32 = _S257 + _S257;
    var _S258 : f32 = dpNoV_4.primal_0 - a2_3 * dpNoV_4.primal_0;
    var _S259 : DiffPair_float_0 = DiffPair_float_0( _S258 * dpNoV_4.primal_0 + a2_3, (dpNoV_4.differential_0 - (s_diff_a2_0 * dpNoV_4.primal_0 + dpNoV_4.differential_0 * a2_3)) * dpNoV_4.primal_0 + dpNoV_4.differential_0 * _S258 + s_diff_a2_0 );
    var _S260 : DiffPair_float_0 = _d_sqrt_1(_S259);
    var _S261 : f32 = dpNoL_4.primal_0 - a2_3 * dpNoL_4.primal_0;
    var _S262 : DiffPair_float_0 = DiffPair_float_0( _S261 * dpNoL_4.primal_0 + a2_3, (dpNoL_4.differential_0 - (s_diff_a2_0 * dpNoL_4.primal_0 + dpNoL_4.differential_0 * a2_3)) * dpNoL_4.primal_0 + dpNoL_4.differential_0 * _S261 + s_diff_a2_0 );
    var _S263 : DiffPair_float_0 = _d_sqrt_1(_S262);
    var _S264 : f32 = dpNoL_4.primal_0 * _S260.primal_0 + dpNoV_4.primal_0 * _S263.primal_0;
    var _S265 : DiffPair_float_0 = DiffPair_float_0( 0.5f / _S264, (0.0f - 0.5f * (dpNoL_4.differential_0 * _S260.primal_0 + _S260.differential_0 * dpNoL_4.primal_0 + (dpNoV_4.differential_0 * _S263.primal_0 + _S263.differential_0 * dpNoV_4.primal_0))) / (_S264 * _S264) );
    return _S265;
}

fn s_fwd_pow5_0( dpx_14 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S266 : f32 = dpx_14.primal_0;
    var x2_3 : f32 = _S266 * _S266;
    var _S267 : f32 = dpx_14.differential_0 * dpx_14.primal_0;
    var _S268 : f32 = x2_3 * x2_3;
    var _S269 : f32 = (_S267 + _S267) * x2_3;
    var _S270 : DiffPair_float_0 = DiffPair_float_0( _S268 * dpx_14.primal_0, (_S269 + _S269) * dpx_14.primal_0 + dpx_14.differential_0 * _S268 );
    return _S270;
}

fn s_fwd_F_Schlick_0( dpf0_4 : DiffPair_vectorx3Cfloatx2C3x3E_0,  dpVoH_4 : DiffPair_float_0) -> DiffPair_vectorx3Cfloatx2C3x3E_0
{
    var _S271 : vec3<f32> = vec3<f32>(1.0f) - dpf0_4.primal_0;
    var _S272 : DiffPair_float_0 = DiffPair_float_0( 1.0f - dpVoH_4.primal_0, 0.0f - dpVoH_4.differential_0 );
    var _S273 : DiffPair_float_0 = s_fwd_pow5_0(_S272);
    var _S274 : vec3<f32> = vec3<f32>(_S273.primal_0);
    var _S275 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( dpf0_4.primal_0 + _S271 * _S274, dpf0_4.differential_0 + ((vec3<f32>(0.0f) - dpf0_4.differential_0) * _S274 + vec3<f32>(_S273.differential_0) * _S271) );
    return _S275;
}

fn s_fwd_F_Schlick_1( dpf0_5 : DiffPair_float_0,  dpf90_2 : DiffPair_float_0,  dpVoH_5 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S276 : f32 = dpf90_2.primal_0 - dpf0_5.primal_0;
    var _S277 : DiffPair_float_0 = DiffPair_float_0( 1.0f - dpVoH_5.primal_0, 0.0f - dpVoH_5.differential_0 );
    var _S278 : DiffPair_float_0 = s_fwd_pow5_0(_S277);
    var _S279 : DiffPair_float_0 = DiffPair_float_0( dpf0_5.primal_0 + _S276 * _S278.primal_0, dpf0_5.differential_0 + ((dpf90_2.differential_0 - dpf0_5.differential_0) * _S278.primal_0 + _S278.differential_0 * _S276) );
    return _S279;
}

fn s_fwd_Fd_Burley_0( dplinearRoughness_8 : DiffPair_float_0,  dpNoV_5 : DiffPair_float_0,  dpNoL_5 : DiffPair_float_0,  dpLoH_2 : DiffPair_float_0) -> DiffPair_float_0
{
    var _S280 : f32 = 2.0f * dplinearRoughness_8.primal_0;
    var _S281 : f32 = _S280 * dpLoH_2.primal_0;
    var _S282 : DiffPair_float_0 = DiffPair_float_0( 1.0f, 0.0f );
    var _S283 : DiffPair_float_0 = DiffPair_float_0( 0.5f + _S281 * dpLoH_2.primal_0, (dplinearRoughness_8.differential_0 * 2.0f * dpLoH_2.primal_0 + dpLoH_2.differential_0 * _S280) * dpLoH_2.primal_0 + dpLoH_2.differential_0 * _S281 );
    var _S284 : DiffPair_float_0 = DiffPair_float_0( dpNoL_5.primal_0, dpNoL_5.differential_0 );
    var _S285 : DiffPair_float_0 = s_fwd_F_Schlick_1(_S282, _S283, _S284);
    var _S286 : DiffPair_float_0 = DiffPair_float_0( dpNoV_5.primal_0, dpNoV_5.differential_0 );
    var _S287 : DiffPair_float_0 = s_fwd_F_Schlick_1(_S282, _S283, _S286);
    var _S288 : DiffPair_float_0 = DiffPair_float_0( _S285.primal_0 * _S287.primal_0 * 0.31830987334251404f, (_S285.differential_0 * _S287.primal_0 + _S287.differential_0 * _S285.primal_0) * 0.31830987334251404f );
    return _S288;
}

fn s_fwd_BRDF_0( dpuv_2 : DiffPair_vectorx3Cfloatx2C2x3E_0,  dpgbuffer_2 : DiffPair_GBuffer_0,  lightDirection_5 : vec3<f32>,  viewDirection_5 : vec3<f32>) -> DiffPair_vectorx3Cfloatx2C3x3E_0
{
    var _S289 : vec3<f32> = vec3<f32>(0.0f);
    var _S290 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( lightDirection_5, _S289 );
    var _S291 : DiffPair_vectorx3Cfloatx2C3x3E_0 = s_fwd_normalize_impl_0(_S290);
    var lightColor_1 : vec3<f32> = vec3<f32>(0.98000001907348633f, 0.92000001668930054f, 0.88999998569488525f) * vec3<f32>(2.0f);
    var _S292 : f32 = 1.0f - dpgbuffer_2.primal_0.metallic_0;
    var _S293 : f32 = 0.0f - dpgbuffer_2.differential_0.metallic_0;
    var _S294 : vec3<f32> = dpgbuffer_2.primal_0.baseColor_0.xyz;
    var _S295 : vec3<f32> = dpgbuffer_2.differential_0.baseColor_0.xyz;
    var _S296 : vec3<f32> = vec3<f32>(_S292);
    var diffuseColor_1 : vec3<f32> = _S296 * _S294;
    var _S297 : vec3<f32> = vec3<f32>(dpgbuffer_2.primal_0.metallic_0);
    var _S298 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( - viewDirection_5, _S289 );
    var _S299 : DiffPair_vectorx3Cfloatx2C3x3E_0 = s_fwd_normalize_impl_0(_S298);
    var _S300 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( dpgbuffer_2.primal_0.normal_0, dpgbuffer_2.differential_0.normal_0 );
    var _S301 : DiffPair_vectorx3Cfloatx2C3x3E_0 = s_fwd_normalize_impl_0(_S300);
    var _S302 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( _S299.primal_0 + _S291.primal_0, _S299.differential_0 + _S291.differential_0 );
    var _S303 : DiffPair_vectorx3Cfloatx2C3x3E_0 = s_fwd_normalize_impl_0(_S302);
    var _S304 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( _S301.primal_0, _S301.differential_0 );
    var _S305 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( _S299.primal_0, _S299.differential_0 );
    var _S306 : DiffPair_float_0 = _d_dot_1(_S304, _S305);
    var _S307 : DiffPair_float_0 = DiffPair_float_0( _S306.primal_0, _S306.differential_0 );
    var _S308 : DiffPair_float_0 = _d_abs_1(_S307);
    var _S309 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( _S291.primal_0, _S291.differential_0 );
    var _S310 : DiffPair_float_0 = _d_dot_1(_S304, _S309);
    var _S311 : DiffPair_float_0 = DiffPair_float_0( _S310.primal_0, _S310.differential_0 );
    var _S312 : DiffPair_float_0 = _d_saturate_1(_S311);
    var _S313 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( _S303.primal_0, _S303.differential_0 );
    var _S314 : DiffPair_float_0 = _d_dot_1(_S304, _S313);
    var _S315 : DiffPair_float_0 = DiffPair_float_0( _S314.primal_0, _S314.differential_0 );
    var _S316 : DiffPair_float_0 = _d_saturate_1(_S315);
    var _S317 : DiffPair_float_0 = _d_dot_1(_S309, _S313);
    var _S318 : DiffPair_float_0 = DiffPair_float_0( _S317.primal_0, _S317.differential_0 );
    var _S319 : DiffPair_float_0 = _d_saturate_1(_S318);
    var _S320 : DiffPair_float_0 = DiffPair_float_0( dpgbuffer_2.primal_0.linearRoughness_0, dpgbuffer_2.differential_0.linearRoughness_0 );
    var _S321 : DiffPair_float_0 = DiffPair_float_0( _S316.primal_0, _S316.differential_0 );
    var _S322 : DiffPair_float_0 = s_fwd_D_GGX_0(_S320, _S321, _S313);
    var _S323 : DiffPair_float_0 = DiffPair_float_0( _S308.primal_0 + 0.00000999999974738f, _S308.differential_0 );
    var _S324 : DiffPair_float_0 = DiffPair_float_0( _S312.primal_0, _S312.differential_0 );
    var _S325 : DiffPair_float_0 = s_fwd_V_SmithGGXCorrelated_0(_S320, _S323, _S324);
    var _S326 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( vec3<f32>((0.03999999910593033f * _S292)) + _S294 * _S297, vec3<f32>((_S293 * 0.03999999910593033f)) + (_S295 * _S297 + vec3<f32>(dpgbuffer_2.differential_0.metallic_0) * _S294) );
    var _S327 : DiffPair_float_0 = DiffPair_float_0( _S319.primal_0, _S319.differential_0 );
    var _S328 : DiffPair_vectorx3Cfloatx2C3x3E_0 = s_fwd_F_Schlick_0(_S326, _S327);
    var _S329 : vec3<f32> = vec3<f32>((_S322.primal_0 * _S325.primal_0));
    var _S330 : DiffPair_float_0 = s_fwd_Fd_Burley_0(_S320, _S323, _S324, _S327);
    var _S331 : vec3<f32> = vec3<f32>(_S330.primal_0);
    var color_0 : vec3<f32> = diffuseColor_1 * _S331 + _S329 * _S328.primal_0;
    var _S332 : vec3<f32> = vec3<f32>(_S312.primal_0) * lightColor_1;
    var _S333 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( color_0 * _S332, ((vec3<f32>(_S293) * _S294 + _S295 * _S296) * _S331 + vec3<f32>(_S330.differential_0) * diffuseColor_1 + (vec3<f32>((_S322.differential_0 * _S325.primal_0 + _S325.differential_0 * _S322.primal_0)) * _S328.primal_0 + _S328.differential_0 * _S329)) * _S332 + vec3<f32>(_S312.differential_0) * lightColor_1 * color_0 );
    return _S333;
}

fn s_fwd_ComputeColorError_0( dpa_1 : DiffPair_vectorx3Cfloatx2C3x3E_0,  dpb_1 : DiffPair_vectorx3Cfloatx2C3x3E_0) -> DiffPair_float_0
{
    var _S334 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( dpa_1.primal_0 - dpb_1.primal_0, dpa_1.differential_0 - dpb_1.differential_0 );
    var _S335 : DiffPair_float_0 = _d_dot_1(_S334, _S334);
    var _S336 : DiffPair_float_0 = DiffPair_float_0( _S335.primal_0, _S335.differential_0 );
    return _S336;
}

fn s_fwd_ComputeImageError_0( uv_4 : vec2<f32>,  lightDirection_6 : vec3<f32>,  viewDirection_6 : vec3<f32>,  dpstateA_1 : DiffPair_vectorx3Cfloatx2C4x3E_0,  dpstateB_1 : DiffPair_vectorx3Cfloatx2C4x3E_0,  targetColor_2 : vec3<f32>,  smoothStateA_2 : vec4<f32>,  smoothStateB_2 : vec4<f32>) -> DiffPair_float_0
{
    var _S337 : vec3<f32> = vec3<f32>(2.0f);
    var _S338 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( dpstateB_1.primal_0.xyz * _S337 - vec3<f32>(1.0f), dpstateB_1.differential_0.xyz * _S337 );
    var _S339 : DiffPair_vectorx3Cfloatx2C3x3E_0 = s_fwd_normalize_impl_0(_S338);
    var _S340 : GBuffer_0 = GBuffer_0( 0.5f, _S339.primal_0, dpstateA_1.primal_0.xyz, dpstateB_1.primal_0.w, dpstateA_1.primal_0.w );
    var _S341 : GBuffer_0 = GBuffer_0( 0.0f, _S339.differential_0, dpstateA_1.differential_0.xyz, dpstateB_1.differential_0.w, dpstateA_1.differential_0.w );
    var _S342 : DiffPair_vectorx3Cfloatx2C2x3E_0 = DiffPair_vectorx3Cfloatx2C2x3E_0( uv_4, vec2<f32>(0.0f) );
    var _S343 : DiffPair_GBuffer_0 = DiffPair_GBuffer_0( _S340, _S341 );
    var _S344 : DiffPair_vectorx3Cfloatx2C3x3E_0 = s_fwd_BRDF_0(_S342, _S343, lightDirection_6, viewDirection_6);
    var _S345 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( _S344.primal_0, _S344.differential_0 );
    var _S346 : DiffPair_vectorx3Cfloatx2C3x3E_0 = DiffPair_vectorx3Cfloatx2C3x3E_0( targetColor_2, vec3<f32>(0.0f) );
    var _S347 : DiffPair_float_0 = s_fwd_ComputeColorError_0(_S345, _S346);
    var _S348 : DiffPair_float_0 = DiffPair_float_0( _S347.primal_0, _S347.differential_0 );
    return _S348;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_2 : vec3<u32>)
{
    var _S349 : vec2<u32> = DTid_2.xy;
    var _S350 : vec2<f32> = vec2<f32>(_S349);
    var uv_5 : vec2<f32> = _S350 / vec2<f32>(vec2<u32>(u32(256), u32(256)));
    var _S351 : vec4<f32> = vec4<f32>(vec4<i32>(i32(0)));
    var dFLocalA_0 : vec4<f32> = _S351;
    var dFLocalB_0 : vec4<f32> = _S351;
    var _S352 : vec2<i32> = vec2<i32>(_S349);
    var _S353 : vec4<f32> = (textureLoad((StateAReadOnly), (_S352)));
    var stateA_1 : vec4<f32> = saturate(_S353);
    var _S354 : vec4<f32> = (textureLoad((StateBReadOnly), (_S352)));
    var _S355 : vec4<f32> = saturate(_S354);
    var stateB_1 : vec4<f32> = _S355;
    var _S356 : vec4<f32> = getSmoothStateA_0(_S349);
    var _S357 : vec4<f32> = getSmoothStateB_0(_S349);
    var _S358 : vec3<f32> = vec3<f32>(2.0f);
    var _S359 : vec3<f32> = vec3<f32>(1.0f);
    var _S360 : vec3<f32> = vec3<f32>(0.5f);
    var _S361 : vec3<f32> = normalize(_S355.xyz * _S358 - _S359) * _S360 + _S360;
    stateB_1.x = _S361.x;
    stateB_1.y = _S361.y;
    stateB_1.z = _S361.z;
    var seed_2 : f32 = f32(base_hash_0((bitcast<vec2<u32>>((_S350))))) / 4.294967296e+09f + f32(_ComputeGradientCB.FrameIndex_0);
    var directionId_0 : i32 = i32(0);
    for(;;)
    {
        if(directionId_0 < i32(12))
        {
        }
        else
        {
            break;
        }
        var lightDirection_7 : vec3<f32> = random_in_unit_sphere_0(&(seed_2));
        var viewDirection_7 : vec3<f32> = random_in_unit_sphere_0(&(seed_2));
        var targetColor_3 : vec3<f32> = BRDF_0(uv_5, scene_0(uv_5), lightDirection_7, viewDirection_7);
        var i_1 : i32;
        var _S362 : bool;
        if((_ComputeGradientCB.UseBackwardAD_0) != u32(0))
        {
            var loss_0 : f32 = ComputeImageError_0(uv_5, lightDirection_7, viewDirection_7, stateA_1, stateB_1, targetColor_3, _S356, _S357);
            var _S363 : vec4<f32> = vec4<f32>(0.0f);
            var stateAD_0 : DiffPair_vectorx3Cfloatx2C4x3E_0;
            stateAD_0.primal_0 = stateA_1;
            stateAD_0.differential_0 = _S363;
            var stateBD_0 : DiffPair_vectorx3Cfloatx2C4x3E_0;
            stateBD_0.primal_0 = stateB_1;
            stateBD_0.differential_0 = _S363;
            s_bwd_ComputeImageError_0(uv_5, lightDirection_7, viewDirection_7, &(stateAD_0), &(stateBD_0), targetColor_3, _S356, _S357, loss_0);
            i_1 = i32(0);
            for(;;)
            {
                if(i_1 < i32(4))
                {
                }
                else
                {
                    break;
                }
                if((stateAD_0.differential_0[i_1]) > -10.0f)
                {
                    _S362 = (stateAD_0.differential_0[i_1]) < 10.0f;
                }
                else
                {
                    _S362 = false;
                }
                if(_S362)
                {
                    dFLocalA_0[i_1] = dFLocalA_0[i_1] + stateAD_0.differential_0[i_1];
                }
                var _S364 : bool;
                if((stateBD_0.differential_0[i_1]) > -10.0f)
                {
                    _S364 = (stateBD_0.differential_0[i_1]) < 10.0f;
                }
                else
                {
                    _S364 = false;
                }
                if(_S364)
                {
                    dFLocalB_0[i_1] = dFLocalB_0[i_1] + stateBD_0.differential_0[i_1];
                }
                i_1 = i_1 + i32(1);
            }
        }
        else
        {
            i_1 = i32(0);
            for(;;)
            {
                if(i_1 < i32(4))
                {
                }
                else
                {
                    break;
                }
                var _S365 : DiffPair_vectorx3Cfloatx2C4x3E_0 = DiffPair_vectorx3Cfloatx2C4x3E_0( stateA_1, indexToMask_0(i_1) );
                var _S366 : DiffPair_vectorx3Cfloatx2C4x3E_0 = DiffPair_vectorx3Cfloatx2C4x3E_0( stateB_1, vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                var _S367 : DiffPair_float_0 = s_fwd_ComputeImageError_0(uv_5, lightDirection_7, viewDirection_7, _S365, _S366, targetColor_3, _S356, _S357);
                if((_S367.differential_0) > -1.0f)
                {
                    _S362 = (_S367.differential_0) < 1.0f;
                }
                else
                {
                    _S362 = false;
                }
                if(_S362)
                {
                    dFLocalA_0[i_1] = dFLocalA_0[i_1] + _S367.differential_0;
                }
                i_1 = i_1 + i32(1);
            }
            var i_2 : i32 = i32(0);
            for(;;)
            {
                if(i_2 < i32(4))
                {
                }
                else
                {
                    break;
                }
                var _S368 : DiffPair_vectorx3Cfloatx2C4x3E_0 = DiffPair_vectorx3Cfloatx2C4x3E_0( stateA_1, vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                var _S369 : DiffPair_vectorx3Cfloatx2C4x3E_0 = DiffPair_vectorx3Cfloatx2C4x3E_0( stateB_1, indexToMask_0(i_2) );
                var _S370 : DiffPair_float_0 = s_fwd_ComputeImageError_0(uv_5, lightDirection_7, viewDirection_7, _S368, _S369, targetColor_3, _S356, _S357);
                if((_S370.differential_0) > -1.0f)
                {
                    _S362 = (_S370.differential_0) < 1.0f;
                }
                else
                {
                    _S362 = false;
                }
                if(_S362)
                {
                    dFLocalB_0[i_2] = dFLocalB_0[i_2] + _S370.differential_0;
                }
                i_2 = i_2 + i32(1);
            }
        }
        directionId_0 = directionId_0 + i32(1);
    }
    var gbuffer_4 : GBuffer_0;
    gbuffer_4.depth_0 = 0.5f;
    gbuffer_4.normal_0 = normalize(stateB_1.xyz * _S358 - _S359);
    gbuffer_4.baseColor_0 = stateA_1.xyz;
    gbuffer_4.metallic_0 = stateB_1.w;
    gbuffer_4.linearRoughness_0 = stateA_1.w;
    textureStore((OutputGradient), (_S349), (vec4<f32>(BRDF_0(uv_5, gbuffer_4, vec3<f32>(-0.60000002384185791f, -0.69999998807907104f, 0.69999998807907104f), vec3<f32>(0.0f, 0.0f, -1.0f)), 1.0f)));
    var _S371 : bool = (_ComputeGradientCB.Reset_0) != u32(0);
    if(_S371)
    {
        seed_2 = f32(base_hash_0(_S349 + vec2<u32>(u32(_ComputeGradientCB.FrameIndex_0 * i32(123)), u32(_ComputeGradientCB.FrameIndex_0 * i32(137)))));
        textureStore((StateA), (_S349), (vec4<f32>(0.5f, 0.5f, 0.5f, 1.0f)));
        textureStore((StateB), (_S349), (vec4<f32>(0.5f, 0.5f, 1.0f, 0.0f)));
    }
    else
    {
        var _S372 : vec4<f32> = vec4<f32>(0.00343750021420419f);
        textureStore((StateA), (_S349), (stateA_1 - _S372 * dFLocalA_0));
        textureStore((StateB), (_S349), (stateB_1 - _S372 * dFLocalB_0));
    }
    return;
}

`;

// Shader code for Compute shader "Overview", node "Overview"
static ShaderCode_Overview_Overview = `
@binding(1) @group(0) var StateA : texture_2d<f32>;

@binding(2) @group(0) var StateB : texture_2d<f32>;

@binding(3) @group(0) var OverviewImage : texture_storage_2d</*(OverviewImage_format)*/, write>;

struct GBuffer_0
{
     depth_0 : f32,
     normal_0 : vec3<f32>,
     baseColor_0 : vec3<f32>,
     metallic_0 : f32,
     linearRoughness_0 : f32,
};

fn sphere_0( gbuffer_0 : ptr<function, GBuffer_0>,  pos_0 : vec2<f32>,  center_0 : vec3<f32>,  radius_0 : f32,  baseColor_1 : vec3<f32>,  linearRoughness_1 : f32,  metallic_1 : f32)
{
    var norm_0 : vec2<f32> = (pos_0 - center_0.xy) / vec2<f32>(radius_0);
    var dist2_0 : f32 = dot(norm_0, norm_0);
    if(dist2_0 < 1.0f)
    {
        var h_0 : f32 = sqrt(1.0f - dist2_0);
        var srcDepth_0 : f32 = center_0.z - h_0 * radius_0;
        if(srcDepth_0 < ((*gbuffer_0).depth_0))
        {
            (*gbuffer_0).depth_0 = srcDepth_0;
            (*gbuffer_0).normal_0 = normalize(vec3<f32>(norm_0, h_0));
            (*gbuffer_0).baseColor_0 = baseColor_1;
            (*gbuffer_0).metallic_0 = metallic_1;
            (*gbuffer_0).linearRoughness_0 = linearRoughness_1;
        }
    }
    return;
}

fn scene_0( uv_0 : vec2<f32>) -> GBuffer_0
{
    var gbuffer_1 : GBuffer_0;
    gbuffer_1.depth_0 = 1.0f;
    var _S1 : vec3<f32> = vec3<f32>(vec3<i32>(i32(0)));
    gbuffer_1.normal_0 = _S1;
    gbuffer_1.baseColor_0 = _S1;
    gbuffer_1.metallic_0 = 0.0f;
    gbuffer_1.linearRoughness_0 = 0.0f;
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.40000000596046448f, 0.34999999403953552f, 0.63999998569488525f), 0.25f, vec3<f32>(1.0f, 0.30000001192092896f, 0.30000001192092896f), 0.20000000298023224f, 0.0f);
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.60000002384185791f, 0.44999998807907104f, 0.5f), 0.17000000178813934f, vec3<f32>(1.0f, 1.0f, 0.20000000298023224f), 0.44999998807907104f, 1.0f);
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.64999997615814209f, 0.75f, 0.40000000596046448f), 0.30000001192092896f, vec3<f32>(0.30000001192092896f, 1.0f, 0.30000001192092896f), 0.00999999977648258f, 0.0f);
    sphere_0(&(gbuffer_1), uv_0, vec3<f32>(0.34999999403953552f, 0.64999997615814209f, 0.40000000596046448f), 0.20000000298023224f, vec3<f32>(1.0f, 1.0f, 1.0f), 0.10000000149011612f, 0.0f);
    return gbuffer_1;
}

fn D_GGX_0( linearRoughness_2 : f32,  NoH_0 : f32,  h_1 : vec3<f32>) -> f32
{
    var a_0 : f32 = NoH_0 * linearRoughness_2;
    var k_0 : f32 = linearRoughness_2 / (1.0f - NoH_0 * NoH_0 + a_0 * a_0 + 0.00999999977648258f);
    return k_0 * k_0 * 0.31830987334251404f;
}

fn V_SmithGGXCorrelated_0( linearRoughness_3 : f32,  NoV_0 : f32,  NoL_0 : f32) -> f32
{
    var a2_0 : f32 = linearRoughness_3 * linearRoughness_3;
    return 0.5f / (NoL_0 * sqrt((NoV_0 - a2_0 * NoV_0) * NoV_0 + a2_0) + NoV_0 * sqrt((NoL_0 - a2_0 * NoL_0) * NoL_0 + a2_0));
}

fn pow5_0( x_0 : f32) -> f32
{
    var x2_0 : f32 = x_0 * x_0;
    return x2_0 * x2_0 * x_0;
}

fn F_Schlick_0( f0_0 : vec3<f32>,  VoH_0 : f32) -> vec3<f32>
{
    return f0_0 + (vec3<f32>(1.0f) - f0_0) * vec3<f32>(pow5_0(1.0f - VoH_0));
}

fn F_Schlick_1( f0_1 : f32,  f90_0 : f32,  VoH_1 : f32) -> f32
{
    return f0_1 + (f90_0 - f0_1) * pow5_0(1.0f - VoH_1);
}

fn Fd_Burley_0( linearRoughness_4 : f32,  NoV_1 : f32,  NoL_1 : f32,  LoH_0 : f32) -> f32
{
    var f90_1 : f32 = 0.5f + 2.0f * linearRoughness_4 * LoH_0 * LoH_0;
    return F_Schlick_1(1.0f, f90_1, NoL_1) * F_Schlick_1(1.0f, f90_1, NoV_1) * 0.31830987334251404f;
}

fn BRDF_0( uv_1 : vec2<f32>,  gbuffer_2 : GBuffer_0,  lightDirection_0 : vec3<f32>,  viewDirection_0 : vec3<f32>) -> vec3<f32>
{
    var _S2 : vec3<f32> = normalize(lightDirection_0);
    var _S3 : f32 = 1.0f - gbuffer_2.metallic_0;
    var _S4 : vec3<f32> = gbuffer_2.baseColor_0.xyz;
    var v_0 : vec3<f32> = normalize(- viewDirection_0);
    var n_0 : vec3<f32> = normalize(gbuffer_2.normal_0);
    var h_2 : vec3<f32> = normalize(v_0 + _S2);
    var NoV_2 : f32 = abs(dot(n_0, v_0)) + 0.00000999999974738f;
    var NoL_2 : f32 = saturate(dot(n_0, _S2));
    var LoH_1 : f32 = saturate(dot(_S2, h_2));
    return (vec3<f32>(_S3) * _S4 * vec3<f32>(Fd_Burley_0(gbuffer_2.linearRoughness_0, NoV_2, NoL_2, LoH_1)) + vec3<f32>((D_GGX_0(gbuffer_2.linearRoughness_0, saturate(dot(n_0, h_2)), h_2) * V_SmithGGXCorrelated_0(gbuffer_2.linearRoughness_0, NoV_2, NoL_2))) * F_Schlick_0(vec3<f32>((0.03999999910593033f * _S3)) + _S4 * vec3<f32>(gbuffer_2.metallic_0), LoH_1)) * (vec3<f32>(NoL_2) * (vec3<f32>(0.98000001907348633f, 0.92000001668930054f, 0.88999998569488525f) * vec3<f32>(2.0f)));
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S5 : vec2<u32> = DTid_0.xy;
    var _S6 : vec2<u32> = vec2<u32>(u32(256));
    var _S7 : vec2<u32> = _S5 / _S6;
    var _S8 : vec2<i32> = vec2<i32>(_S7);
    var _S9 : vec2<u32> = _S5 % _S6;
    var _S10 : vec2<i32> = vec2<i32>(_S9);
    var uv_2 : vec2<f32> = vec2<f32>(_S10) / vec2<f32>(vec2<u32>(u32(256), u32(256)));
    var _S11 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(_S10.xy), u32(0)));
    var stateA_0 : vec4<f32> = saturate((textureLoad((StateA), ((_S11)).xy, ((_S11)).z)));
    var stateB_0 : vec4<f32> = saturate((textureLoad((StateB), ((_S11)).xy, ((_S11)).z)));
    var targetgbuffer_0 : GBuffer_0 = scene_0(uv_2);
    var targetStateA_0 : vec4<f32> = vec4<f32>(targetgbuffer_0.baseColor_0, targetgbuffer_0.linearRoughness_0);
    var _S12 : vec3<f32> = vec3<f32>(0.5f);
    var targetStateB_0 : vec4<f32> = vec4<f32>(targetgbuffer_0.normal_0 * _S12 + _S12, targetgbuffer_0.metallic_0);
    var gbuffer_3 : GBuffer_0;
    gbuffer_3.depth_0 = 0.5f;
    gbuffer_3.normal_0 = normalize(stateB_0.xyz * vec3<f32>(2.0f) - vec3<f32>(1.0f));
    gbuffer_3.baseColor_0 = stateA_0.xyz;
    gbuffer_3.metallic_0 = stateB_0.w;
    gbuffer_3.linearRoughness_0 = stateA_0.w;
    const _S13 : vec3<f32> = vec3<f32>(-0.60000002384185791f, -0.69999998807907104f, 0.69999998807907104f);
    const _S14 : vec3<f32> = vec3<f32>(0.0f, 0.0f, -1.0f);
    var shadedResult_0 : vec3<f32> = BRDF_0(uv_2, gbuffer_3, _S13, _S14);
    var targetColor_0 : vec3<f32> = BRDF_0(uv_2, targetgbuffer_0, _S13, _S14);
    const _S15 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var _S16 : i32 = _S8.x;
    var output_0 : vec4<f32>;
    if(_S16 == i32(0))
    {
        output_0 = targetStateA_0;
    }
    else
    {
        if(_S16 == i32(1))
        {
            output_0 = targetStateB_0;
        }
        else
        {
            output_0 = _S15;
        }
    }
    if(_S16 == i32(2))
    {
        output_0 = vec4<f32>(targetColor_0, 0.0f);
    }
    else
    {
        if(_S16 == i32(3))
        {
            output_0 = stateA_0;
        }
        else
        {
            if(_S16 == i32(4))
            {
                output_0 = stateB_0;
            }
            else
            {
                if(_S16 == i32(5))
                {
                    output_0 = vec4<f32>(shadedResult_0, 0.0f);
                }
            }
        }
    }
    var _S17 : i32 = _S8.y;
    if(_S17 == i32(0))
    {
        textureStore((OverviewImage), (_S5), (vec4<f32>(output_0.xyz, 1.0f)));
    }
    else
    {
        if(_S17 == i32(1))
        {
            textureStore((OverviewImage), (_S5), (vec4<f32>(output_0.www, 1.0f)));
        }
    }
    return;
}

`;

// -------------------- Private Members

// Texture DebugGradient
texture_DebugGradient = null;
texture_DebugGradient_size = [0, 0, 0];
texture_DebugGradient_format = "";
texture_DebugGradient_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture StateB
texture_StateB = null;
texture_StateB_size = [0, 0, 0];
texture_StateB_format = "";
texture_StateB_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture StateA
texture_StateA = null;
texture_StateA_size = [0, 0, 0];
texture_StateA_format = "";
texture_StateA_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture OverviewImage
texture_OverviewImage = null;
texture_OverviewImage_size = [0, 0, 0];
texture_OverviewImage_format = "";
texture_OverviewImage_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture ComputeGradient_OutputGradient_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_ComputeGradient_OutputGradient_ReadOnly = null;
texture_ComputeGradient_OutputGradient_ReadOnly_size = [0, 0, 0];
texture_ComputeGradient_OutputGradient_ReadOnly_format = "";
texture_ComputeGradient_OutputGradient_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture ComputeGradient_StateA_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_ComputeGradient_StateA_ReadOnly = null;
texture_ComputeGradient_StateA_ReadOnly_size = [0, 0, 0];
texture_ComputeGradient_StateA_ReadOnly_format = "";
texture_ComputeGradient_StateA_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture ComputeGradient_StateB_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_ComputeGradient_StateB_ReadOnly = null;
texture_ComputeGradient_StateB_ReadOnly_size = [0, 0, 0];
texture_ComputeGradient_StateB_ReadOnly_format = "";
texture_ComputeGradient_StateB_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Overview_OverviewImage_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Overview_OverviewImage_ReadOnly = null;
texture_Overview_OverviewImage_ReadOnly_size = [0, 0, 0];
texture_Overview_OverviewImage_ReadOnly_format = "";
texture_Overview_OverviewImage_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _ComputeGradientCB
constantBuffer__ComputeGradientCB = null;
constantBuffer__ComputeGradientCB_size = 16;
constantBuffer__ComputeGradientCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader ComputeGradient
Hash_Compute_ComputeGradient = 0;
ShaderModule_Compute_ComputeGradient = null;
BindGroupLayout_Compute_ComputeGradient = null;
PipelineLayout_Compute_ComputeGradient = null;
Pipeline_Compute_ComputeGradient = null;

// Compute Shader Overview
Hash_Compute_Overview = 0;
ShaderModule_Compute_Overview = null;
BindGroupLayout_Compute_Overview = null;
PipelineLayout_Compute_Overview = null;
Pipeline_Compute_Overview = null;

// -------------------- Public Variables

variable_Reset = true;
variableDefault_Reset = true;
variableChanged_Reset = false;
variable_initialized = false;
variableDefault_initialized = false;
variableChanged_initialized = false;
variable_FrameIndex = 0;
variableDefault_FrameIndex = 0;
variableChanged_FrameIndex = false;
variable_UseBackwardAD = false;
variableDefault_UseBackwardAD = false;
variableChanged_UseBackwardAD = false;

// -------------------- Private Variables

variable_Size = [ 256,256 ];  // width and height of the image
variableDefault_Size = [ 256,256 ];  // width and height of the image
variableChanged_Size = [ false, false ];
// -------------------- Structs

static StructOffsets__ComputeGradientCB =
{
    FrameIndex: 0,
    Reset: 4,
    UseBackwardAD: 8,
    _padding0: 12,
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
    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture DebugGradient
    {
        const baseSize = [ this.variable_Size[0], this.variable_Size[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba32float";
        if (this.texture_DebugGradient !== null && (this.texture_DebugGradient_format != desiredFormat || this.texture_DebugGradient_size[0] != desiredSize[0] || this.texture_DebugGradient_size[1] != desiredSize[1] || this.texture_DebugGradient_size[2] != desiredSize[2]))
        {
            this.texture_DebugGradient.destroy();
            this.texture_DebugGradient = null;
        }

        if (this.texture_DebugGradient === null)
        {
            this.texture_DebugGradient_size = desiredSize.slice();
            this.texture_DebugGradient_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_DebugGradient_format))
                viewFormats.push(this.texture_DebugGradient_format);

            this.texture_DebugGradient = device.createTexture({
                label: "texture SlangAutoDiff.DebugGradient",
                size: this.texture_DebugGradient_size,
                format: Shared.GetNonSRGBFormat(this.texture_DebugGradient_format),
                usage: this.texture_DebugGradient_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture StateB
    {
        const baseSize = [ this.variable_Size[0], this.variable_Size[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba32float";
        if (this.texture_StateB !== null && (this.texture_StateB_format != desiredFormat || this.texture_StateB_size[0] != desiredSize[0] || this.texture_StateB_size[1] != desiredSize[1] || this.texture_StateB_size[2] != desiredSize[2]))
        {
            this.texture_StateB.destroy();
            this.texture_StateB = null;
        }

        if (this.texture_StateB === null)
        {
            this.texture_StateB_size = desiredSize.slice();
            this.texture_StateB_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_StateB_format))
                viewFormats.push(this.texture_StateB_format);

            this.texture_StateB = device.createTexture({
                label: "texture SlangAutoDiff.StateB",
                size: this.texture_StateB_size,
                format: Shared.GetNonSRGBFormat(this.texture_StateB_format),
                usage: this.texture_StateB_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture StateA
    {
        const baseSize = [ this.variable_Size[0], this.variable_Size[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba32float";
        if (this.texture_StateA !== null && (this.texture_StateA_format != desiredFormat || this.texture_StateA_size[0] != desiredSize[0] || this.texture_StateA_size[1] != desiredSize[1] || this.texture_StateA_size[2] != desiredSize[2]))
        {
            this.texture_StateA.destroy();
            this.texture_StateA = null;
        }

        if (this.texture_StateA === null)
        {
            this.texture_StateA_size = desiredSize.slice();
            this.texture_StateA_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_StateA_format))
                viewFormats.push(this.texture_StateA_format);

            this.texture_StateA = device.createTexture({
                label: "texture SlangAutoDiff.StateA",
                size: this.texture_StateA_size,
                format: Shared.GetNonSRGBFormat(this.texture_StateA_format),
                usage: this.texture_StateA_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture OverviewImage
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 0) / 1) + 2048,
            Math.floor(((parseInt(baseSize[1]) + 0) * 0) / 1) + 512,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_OverviewImage !== null && (this.texture_OverviewImage_format != desiredFormat || this.texture_OverviewImage_size[0] != desiredSize[0] || this.texture_OverviewImage_size[1] != desiredSize[1] || this.texture_OverviewImage_size[2] != desiredSize[2]))
        {
            this.texture_OverviewImage.destroy();
            this.texture_OverviewImage = null;
        }

        if (this.texture_OverviewImage === null)
        {
            this.texture_OverviewImage_size = desiredSize.slice();
            this.texture_OverviewImage_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_OverviewImage_format))
                viewFormats.push(this.texture_OverviewImage_format);

            this.texture_OverviewImage = device.createTexture({
                label: "texture SlangAutoDiff.OverviewImage",
                size: this.texture_OverviewImage_size,
                format: Shared.GetNonSRGBFormat(this.texture_OverviewImage_format),
                usage: this.texture_OverviewImage_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture ComputeGradient_OutputGradient_ReadOnly
    {
        const baseSize = this.texture_DebugGradient_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_DebugGradient_format;
        if (this.texture_ComputeGradient_OutputGradient_ReadOnly !== null && (this.texture_ComputeGradient_OutputGradient_ReadOnly_format != desiredFormat || this.texture_ComputeGradient_OutputGradient_ReadOnly_size[0] != desiredSize[0] || this.texture_ComputeGradient_OutputGradient_ReadOnly_size[1] != desiredSize[1] || this.texture_ComputeGradient_OutputGradient_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_ComputeGradient_OutputGradient_ReadOnly.destroy();
            this.texture_ComputeGradient_OutputGradient_ReadOnly = null;
        }

        if (this.texture_ComputeGradient_OutputGradient_ReadOnly === null)
        {
            this.texture_ComputeGradient_OutputGradient_ReadOnly_size = desiredSize.slice();
            this.texture_ComputeGradient_OutputGradient_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_ComputeGradient_OutputGradient_ReadOnly_format))
                viewFormats.push(this.texture_ComputeGradient_OutputGradient_ReadOnly_format);

            this.texture_ComputeGradient_OutputGradient_ReadOnly = device.createTexture({
                label: "texture SlangAutoDiff.ComputeGradient_OutputGradient_ReadOnly",
                size: this.texture_ComputeGradient_OutputGradient_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_ComputeGradient_OutputGradient_ReadOnly_format),
                usage: this.texture_ComputeGradient_OutputGradient_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture ComputeGradient_StateA_ReadOnly
    {
        const baseSize = this.texture_StateA_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_StateA_format;
        if (this.texture_ComputeGradient_StateA_ReadOnly !== null && (this.texture_ComputeGradient_StateA_ReadOnly_format != desiredFormat || this.texture_ComputeGradient_StateA_ReadOnly_size[0] != desiredSize[0] || this.texture_ComputeGradient_StateA_ReadOnly_size[1] != desiredSize[1] || this.texture_ComputeGradient_StateA_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_ComputeGradient_StateA_ReadOnly.destroy();
            this.texture_ComputeGradient_StateA_ReadOnly = null;
        }

        if (this.texture_ComputeGradient_StateA_ReadOnly === null)
        {
            this.texture_ComputeGradient_StateA_ReadOnly_size = desiredSize.slice();
            this.texture_ComputeGradient_StateA_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_ComputeGradient_StateA_ReadOnly_format))
                viewFormats.push(this.texture_ComputeGradient_StateA_ReadOnly_format);

            this.texture_ComputeGradient_StateA_ReadOnly = device.createTexture({
                label: "texture SlangAutoDiff.ComputeGradient_StateA_ReadOnly",
                size: this.texture_ComputeGradient_StateA_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_ComputeGradient_StateA_ReadOnly_format),
                usage: this.texture_ComputeGradient_StateA_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture ComputeGradient_StateB_ReadOnly
    {
        const baseSize = this.texture_StateB_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_StateB_format;
        if (this.texture_ComputeGradient_StateB_ReadOnly !== null && (this.texture_ComputeGradient_StateB_ReadOnly_format != desiredFormat || this.texture_ComputeGradient_StateB_ReadOnly_size[0] != desiredSize[0] || this.texture_ComputeGradient_StateB_ReadOnly_size[1] != desiredSize[1] || this.texture_ComputeGradient_StateB_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_ComputeGradient_StateB_ReadOnly.destroy();
            this.texture_ComputeGradient_StateB_ReadOnly = null;
        }

        if (this.texture_ComputeGradient_StateB_ReadOnly === null)
        {
            this.texture_ComputeGradient_StateB_ReadOnly_size = desiredSize.slice();
            this.texture_ComputeGradient_StateB_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_ComputeGradient_StateB_ReadOnly_format))
                viewFormats.push(this.texture_ComputeGradient_StateB_ReadOnly_format);

            this.texture_ComputeGradient_StateB_ReadOnly = device.createTexture({
                label: "texture SlangAutoDiff.ComputeGradient_StateB_ReadOnly",
                size: this.texture_ComputeGradient_StateB_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_ComputeGradient_StateB_ReadOnly_format),
                usage: this.texture_ComputeGradient_StateB_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Overview_OverviewImage_ReadOnly
    {
        const baseSize = this.texture_OverviewImage_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_OverviewImage_format;
        if (this.texture_Overview_OverviewImage_ReadOnly !== null && (this.texture_Overview_OverviewImage_ReadOnly_format != desiredFormat || this.texture_Overview_OverviewImage_ReadOnly_size[0] != desiredSize[0] || this.texture_Overview_OverviewImage_ReadOnly_size[1] != desiredSize[1] || this.texture_Overview_OverviewImage_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Overview_OverviewImage_ReadOnly.destroy();
            this.texture_Overview_OverviewImage_ReadOnly = null;
        }

        if (this.texture_Overview_OverviewImage_ReadOnly === null)
        {
            this.texture_Overview_OverviewImage_ReadOnly_size = desiredSize.slice();
            this.texture_Overview_OverviewImage_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Overview_OverviewImage_ReadOnly_format))
                viewFormats.push(this.texture_Overview_OverviewImage_ReadOnly_format);

            this.texture_Overview_OverviewImage_ReadOnly = device.createTexture({
                label: "texture SlangAutoDiff.Overview_OverviewImage_ReadOnly",
                size: this.texture_Overview_OverviewImage_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Overview_OverviewImage_ReadOnly_format),
                usage: this.texture_Overview_OverviewImage_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader ComputeGradient
    {
        const bindGroupEntries =
        [
            {
                // OutputGradient
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_DebugGradient_format), viewDimension: "2d" }
            },
            {
                // StateA
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_StateA_format), viewDimension: "2d" }
            },
            {
                // StateB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_StateB_format), viewDimension: "2d" }
            },
            {
                // OutputGradientReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_ComputeGradient_OutputGradient_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // StateAReadOnly
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_ComputeGradient_StateA_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // StateBReadOnly
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_ComputeGradient_StateB_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _ComputeGradientCB
                binding: 6,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_ComputeGradient === null || newHash !== this.Hash_Compute_ComputeGradient)
        {
            this.Hash_Compute_ComputeGradient = newHash;

            let shaderCode = class_SlangAutoDiff.ShaderCode_ComputeGradient_ComputeGradient;
            shaderCode = shaderCode.replace("/*(OutputGradient_format)*/", Shared.GetNonSRGBFormat(this.texture_DebugGradient_format));
            shaderCode = shaderCode.replace("/*(StateA_format)*/", Shared.GetNonSRGBFormat(this.texture_StateA_format));
            shaderCode = shaderCode.replace("/*(StateB_format)*/", Shared.GetNonSRGBFormat(this.texture_StateB_format));
            shaderCode = shaderCode.replace("/*(OutputGradientReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_ComputeGradient_OutputGradient_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(StateAReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_ComputeGradient_StateA_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(StateBReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_ComputeGradient_StateB_ReadOnly_format));

            this.ShaderModule_Compute_ComputeGradient = device.createShaderModule({ code: shaderCode, label: "Compute Shader ComputeGradient"});
            this.BindGroupLayout_Compute_ComputeGradient = device.createBindGroupLayout({
                label: "Compute Bind Group Layout ComputeGradient",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_ComputeGradient = device.createPipelineLayout({
                label: "Compute Pipeline Layout ComputeGradient",
                bindGroupLayouts: [this.BindGroupLayout_Compute_ComputeGradient],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_ComputeGradient = device.createComputePipeline({
                    label: "Compute Pipeline ComputeGradient",
                    layout: this.PipelineLayout_Compute_ComputeGradient,
                    compute: {
                        module: this.ShaderModule_Compute_ComputeGradient,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("ComputeGradient");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline ComputeGradient",
                    layout: this.PipelineLayout_Compute_ComputeGradient,
                    compute: {
                        module: this.ShaderModule_Compute_ComputeGradient,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_ComputeGradient = handle; this.loadingPromises.delete("ComputeGradient"); } );
            }
        }
    }

    // (Re)create compute shader Overview
    {
        const bindGroupEntries =
        [
            {
                // OutputGradient
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_DebugGradient_format).sampleType }
            },
            {
                // StateA
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_StateA_format).sampleType }
            },
            {
                // StateB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_StateB_format).sampleType }
            },
            {
                // OverviewImage
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_OverviewImage_format), viewDimension: "2d" }
            },
            {
                // OverviewImageReadOnly
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Overview_OverviewImage_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Overview === null || newHash !== this.Hash_Compute_Overview)
        {
            this.Hash_Compute_Overview = newHash;

            let shaderCode = class_SlangAutoDiff.ShaderCode_Overview_Overview;
            shaderCode = shaderCode.replace("/*(OverviewImage_format)*/", Shared.GetNonSRGBFormat(this.texture_OverviewImage_format));
            shaderCode = shaderCode.replace("/*(OverviewImageReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Overview_OverviewImage_ReadOnly_format));

            this.ShaderModule_Compute_Overview = device.createShaderModule({ code: shaderCode, label: "Compute Shader Overview"});
            this.BindGroupLayout_Compute_Overview = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Overview",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Overview = device.createPipelineLayout({
                label: "Compute Pipeline Layout Overview",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Overview],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Overview = device.createComputePipeline({
                    label: "Compute Pipeline Overview",
                    layout: this.PipelineLayout_Compute_Overview,
                    compute: {
                        module: this.ShaderModule_Compute_Overview,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Overview");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Overview",
                    layout: this.PipelineLayout_Compute_Overview,
                    compute: {
                        module: this.ShaderModule_Compute_Overview,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Overview = handle; this.loadingPromises.delete("Overview"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("SlangAutoDiff.DebugGradient");

    encoder.popDebugGroup(); // "SlangAutoDiff.DebugGradient"

    encoder.pushDebugGroup("SlangAutoDiff.StateB");

    encoder.popDebugGroup(); // "SlangAutoDiff.StateB"

    encoder.pushDebugGroup("SlangAutoDiff.StateA");

    encoder.popDebugGroup(); // "SlangAutoDiff.StateA"

    encoder.pushDebugGroup("SlangAutoDiff.OverviewImage");

    encoder.popDebugGroup(); // "SlangAutoDiff.OverviewImage"

    encoder.pushDebugGroup("SlangAutoDiff.ComputeGradient_OutputGradient_ReadOnly");

    encoder.popDebugGroup(); // "SlangAutoDiff.ComputeGradient_OutputGradient_ReadOnly"

    encoder.pushDebugGroup("SlangAutoDiff.Copy_ComputeGradient_OutputGradient");

    // Copy texture DebugGradient to texture ComputeGradient_OutputGradient_ReadOnly
    {
        const numMips = Math.min(this.texture_DebugGradient.mipLevelCount, this.texture_ComputeGradient_OutputGradient_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_DebugGradient.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_DebugGradient.height >> mipIndex, 1);
            let mipDepth = this.texture_DebugGradient.depthOrArrayLayers;

            if (this.texture_DebugGradient.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_DebugGradient, mipLevel: mipIndex },
                { texture: this.texture_ComputeGradient_OutputGradient_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Copy_ComputeGradient_OutputGradient"

    encoder.pushDebugGroup("SlangAutoDiff.ComputeGradient_StateA_ReadOnly");

    encoder.popDebugGroup(); // "SlangAutoDiff.ComputeGradient_StateA_ReadOnly"

    encoder.pushDebugGroup("SlangAutoDiff.Copy_ComputeGradient_StateA");

    // Copy texture StateA to texture ComputeGradient_StateA_ReadOnly
    {
        const numMips = Math.min(this.texture_StateA.mipLevelCount, this.texture_ComputeGradient_StateA_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_StateA.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_StateA.height >> mipIndex, 1);
            let mipDepth = this.texture_StateA.depthOrArrayLayers;

            if (this.texture_StateA.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_StateA, mipLevel: mipIndex },
                { texture: this.texture_ComputeGradient_StateA_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Copy_ComputeGradient_StateA"

    encoder.pushDebugGroup("SlangAutoDiff.ComputeGradient_StateB_ReadOnly");

    encoder.popDebugGroup(); // "SlangAutoDiff.ComputeGradient_StateB_ReadOnly"

    encoder.pushDebugGroup("SlangAutoDiff.Copy_ComputeGradient_StateB");

    // Copy texture StateB to texture ComputeGradient_StateB_ReadOnly
    {
        const numMips = Math.min(this.texture_StateB.mipLevelCount, this.texture_ComputeGradient_StateB_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_StateB.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_StateB.height >> mipIndex, 1);
            let mipDepth = this.texture_StateB.depthOrArrayLayers;

            if (this.texture_StateB.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_StateB, mipLevel: mipIndex },
                { texture: this.texture_ComputeGradient_StateB_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Copy_ComputeGradient_StateB"

    encoder.pushDebugGroup("SlangAutoDiff.Overview_OverviewImage_ReadOnly");

    encoder.popDebugGroup(); // "SlangAutoDiff.Overview_OverviewImage_ReadOnly"

    encoder.pushDebugGroup("SlangAutoDiff.Copy_Overview_OverviewImage");

    // Copy texture OverviewImage to texture Overview_OverviewImage_ReadOnly
    {
        const numMips = Math.min(this.texture_OverviewImage.mipLevelCount, this.texture_Overview_OverviewImage_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_OverviewImage.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_OverviewImage.height >> mipIndex, 1);
            let mipDepth = this.texture_OverviewImage.depthOrArrayLayers;

            if (this.texture_OverviewImage.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_OverviewImage, mipLevel: mipIndex },
                { texture: this.texture_Overview_OverviewImage_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Copy_Overview_OverviewImage"

    encoder.pushDebugGroup("SlangAutoDiff._ComputeGradientCB");

    // Create constant buffer _ComputeGradientCB
    if (this.constantBuffer__ComputeGradientCB === null)
    {
        this.constantBuffer__ComputeGradientCB = device.createBuffer({
            label: "SlangAutoDiff._ComputeGradientCB",
            size: Shared.Align(16, this.constructor.StructOffsets__ComputeGradientCB._size),
            usage: this.constantBuffer__ComputeGradientCB_usageFlags,
        });
    }

    // Upload values to constant buffer _ComputeGradientCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__ComputeGradientCB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__ComputeGradientCB.FrameIndex, this.variable_FrameIndex, true);
        view.setUint32(this.constructor.StructOffsets__ComputeGradientCB.Reset, (this.variable_Reset === true ? 1 : 0), true);
        view.setUint32(this.constructor.StructOffsets__ComputeGradientCB.UseBackwardAD, (this.variable_UseBackwardAD === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__ComputeGradientCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "SlangAutoDiff._ComputeGradientCB"

    encoder.pushDebugGroup("SlangAutoDiff.ComputeGradient");

    // Run compute shader ComputeGradient
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group ComputeGradient",
            layout: this.BindGroupLayout_Compute_ComputeGradient,
            entries: [
                {
                    // OutputGradient
                    binding: 0,
                    resource: this.texture_DebugGradient.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // StateA
                    binding: 1,
                    resource: this.texture_StateA.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // StateB
                    binding: 2,
                    resource: this.texture_StateB.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputGradientReadOnly
                    binding: 3,
                    resource: this.texture_ComputeGradient_OutputGradient_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // StateAReadOnly
                    binding: 4,
                    resource: this.texture_ComputeGradient_StateA_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // StateBReadOnly
                    binding: 5,
                    resource: this.texture_ComputeGradient_StateB_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _ComputeGradientCB
                    binding: 6,
                    resource: { buffer: this.constantBuffer__ComputeGradientCB }
                },
            ]
        });

        // Calculate dispatch size

        const baseDispatchSize = [ this.variable_Size[0], this.variable_Size[1], 1 ];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_ComputeGradient !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_ComputeGradient);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.ComputeGradient"

    encoder.pushDebugGroup("SlangAutoDiff.Overview");

    // Run compute shader Overview
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Overview",
            layout: this.BindGroupLayout_Compute_Overview,
            entries: [
                {
                    // OutputGradient
                    binding: 0,
                    resource: this.texture_DebugGradient.createView({ dimension: "2d", format: this.texture_DebugGradient_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // StateA
                    binding: 1,
                    resource: this.texture_StateA.createView({ dimension: "2d", format: this.texture_StateA_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // StateB
                    binding: 2,
                    resource: this.texture_StateB.createView({ dimension: "2d", format: this.texture_StateB_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // OverviewImage
                    binding: 3,
                    resource: this.texture_OverviewImage.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OverviewImageReadOnly
                    binding: 4,
                    resource: this.texture_Overview_OverviewImage_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_OverviewImage_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Overview !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Overview);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "SlangAutoDiff.Overview"

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
