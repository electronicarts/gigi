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

class class_BlueNoiseAreaLights
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "RayTraceCS", node "RayTrace"
static ShaderCode_RayTrace_RayTraceCS = `
@binding(0) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_RayTraceCSCB_std140_0
{
    @align(16) AmbientLight_0 : vec3<f32>,
    @align(4) CameraChanged_0 : u32,
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) DirLightBrightness_0 : f32,
    @align(16) DirLightColor_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) DirLightDirection_0 : vec3<f32>,
    @align(4) DirLightEnabled_0 : u32,
    @align(16) DirLightRadius_0 : f32,
    @align(4) DirLightSampleCount_0 : i32,
    @align(8) EMAAlpha_0 : f32,
    @align(4) ExposureFStops_0 : f32,
    @align(16) FrameIndex_0 : u32,
    @align(4) _padding1_0 : f32,
    @align(8) _padding2_0 : f32,
    @align(4) _padding3_0 : f32,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) NoiseType_0 : i32,
    @align(4) PosLightBrightness_0 : f32,
    @align(8) _padding4_0 : f32,
    @align(4) _padding5_0 : f32,
    @align(16) PosLightColor_0 : vec3<f32>,
    @align(4) PosLightEnabled_0 : u32,
    @align(16) PosLightPosition_0 : vec3<f32>,
    @align(4) PosLightRadius_0 : f32,
    @align(16) PosLightSampleCount_0 : i32,
    @align(4) PosLightShineCosThetaInner_0 : f32,
    @align(8) PosLightShineCosThetaOuter_0 : f32,
    @align(4) _padding6_0 : f32,
    @align(16) PosLightShineDir_0 : vec3<f32>,
    @align(4) ResetAccum_0 : u32,
    @align(16) TemporalFilter_0 : i32,
    @align(4) ToneMapping_0 : i32,
    @align(8) _padding7_0 : f32,
    @align(4) _padding8_0 : f32,
};

@binding(7) @group(0) var<uniform> _RayTraceCSCB : Struct_RayTraceCSCB_std140_0;
@binding(4) @group(0) var _loadedTexture_0 : texture_2d<f32>;

@binding(8) @group(0) var texSampler : sampler;

@binding(5) @group(0) var _loadedTexture_1 : texture_2d_array<f32>;

@binding(6) @group(0) var _loadedTexture_2 : texture_2d_array<f32>;

@binding(1) @group(0) var Accum : texture_storage_2d</*(Accum_format)*/, write>;

@binding(3) @group(0) var AccumReadOnly : texture_storage_2d</*(AccumReadOnly_format)*/, read>;

fn SampleSphericalMap_0( v_0 : vec3<f32>) -> vec2<f32>
{
    return vec2<f32>(atan2(v_0.z, v_0.x), asin(- v_0.y)) * vec2<f32>(0.1590999960899353f, 0.31830000877380371f) + vec2<f32>(0.5f);
}

fn HashPCG_0( val_0 : ptr<function, u32>) -> u32
{
    var state_0 : u32 = (*val_0) * u32(747796405) + u32(2891336453);
    var word_0 : u32 = ((((state_0 >> ((((state_0 >> (u32(28)))) + u32(4))))) ^ (state_0))) * u32(277803737);
    (*val_0) = state_0;
    return (((word_0 >> (u32(22)))) ^ (word_0));
}

fn HashPCGConst_0( val_1 : u32) -> u32
{
    var temp_0 : u32 = val_1;
    var _S1 : u32 = HashPCG_0(&(temp_0));
    return _S1;
}

fn HashInit_0( seed_0 : vec3<u32>) -> u32
{
    return HashPCGConst_0(seed_0.x + HashPCGConst_0(seed_0.y + HashPCGConst_0(seed_0.z)));
}

struct SRayHitInfo_0
{
     dist_0 : f32,
     normal_0 : vec3<f32>,
     albedo_0 : vec3<f32>,
     emissive_0 : vec3<f32>,
};

fn RayVsPlane_0( rayPos_0 : vec3<f32>,  rayDir_0 : vec3<f32>,  info_0 : ptr<function, SRayHitInfo_0>,  plane_0 : vec4<f32>,  albedo_1 : vec3<f32>) -> bool
{
    var _S2 : vec3<f32> = plane_0.xyz;
    var denom_0 : f32 = dot(_S2, rayDir_0);
    if((abs(denom_0)) > 0.00100000004749745f)
    {
        var dist_1 : f32 = (plane_0.w - dot(_S2, rayPos_0)) / denom_0;
        var _S3 : bool;
        if(dist_1 > 0.0f)
        {
            _S3 = dist_1 < ((*info_0).dist_0);
        }
        else
        {
            _S3 = false;
        }
        if(_S3)
        {
            (*info_0).dist_0 = dist_1;
            (*info_0).normal_0 = _S2;
            (*info_0).albedo_0 = albedo_1;
            return true;
        }
    }
    return false;
}

fn p_0( x_0 : vec2<f32>) -> vec2<f32>
{
    var _S4 : vec2<f32> = vec2<f32>(2.0f);
    var _S5 : vec2<f32> = vec2<f32>(0.5f);
    var h_0 : vec2<f32> = fract(x_0 / _S4) - _S5;
    return x_0 * _S5 + h_0 * (vec2<f32>(1.0f) - _S4 * abs(h_0));
}

fn checkersGradTriangle_0( uv_0 : vec2<f32>) -> f32
{
    var _S6 : vec2<f32> = abs(vec2<f32>(0.0f, 0.0f));
    var w_0 : vec2<f32> = max(_S6, _S6) + vec2<f32>(0.00999999977648258f);
    var i_0 : vec2<f32> = (p_0(uv_0 + w_0) - vec2<f32>(2.0f) * p_0(uv_0) + p_0(uv_0 - w_0)) / (w_0 * w_0);
    return 0.5f - 0.5f * i_0.x * i_0.y;
}

fn RayVsSphere_0( rayPos_1 : vec3<f32>,  rayDir_1 : vec3<f32>,  info_1 : ptr<function, SRayHitInfo_0>,  sphere_0 : vec4<f32>,  albedo_2 : vec3<f32>) -> bool
{
    var _S7 : vec3<f32> = sphere_0.xyz;
    var m_0 : vec3<f32> = rayPos_1 - _S7;
    var b_0 : f32 = dot(m_0, rayDir_1);
    var _S8 : f32 = sphere_0.w;
    var c_0 : f32 = dot(m_0, m_0) - _S8 * _S8;
    var _S9 : bool;
    if(c_0 > 0.0f)
    {
        _S9 = b_0 > 0.0f;
    }
    else
    {
        _S9 = false;
    }
    if(_S9)
    {
        return false;
    }
    var discr_0 : f32 = b_0 * b_0 - c_0;
    if(discr_0 < 0.0f)
    {
        return false;
    }
    var _S10 : f32 = - b_0;
    var _S11 : f32 = sqrt(discr_0);
    var dist_2 : f32 = _S10 - _S11;
    var _S12 : bool = dist_2 < 0.0f;
    var dist_3 : f32;
    if(_S12)
    {
        dist_3 = _S10 + _S11;
    }
    else
    {
        dist_3 = dist_2;
    }
    if(dist_3 > 0.0f)
    {
        _S9 = dist_3 < ((*info_1).dist_0);
    }
    else
    {
        _S9 = false;
    }
    if(_S9)
    {
        (*info_1).dist_0 = dist_3;
        var _S13 : vec3<f32> = normalize(rayPos_1 + rayDir_1 * vec3<f32>(dist_3) - _S7);
        if(_S12)
        {
            dist_3 = -1.0f;
        }
        else
        {
            dist_3 = 1.0f;
        }
        (*info_1).normal_0 = _S13 * vec3<f32>(dist_3);
        (*info_1).albedo_0 = albedo_2;
        return true;
    }
    return false;
}

fn RayVsScene_0( rayPos_2 : vec3<f32>,  rayDir_2 : vec3<f32>,  shadowRay_0 : bool) -> SRayHitInfo_0
{
    const _S14 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var hitInfo_0 : SRayHitInfo_0;
    hitInfo_0.dist_0 = 0.0f;
    hitInfo_0.normal_0 = _S14;
    hitInfo_0.albedo_0 = _S14;
    hitInfo_0.emissive_0 = _S14;
    hitInfo_0.dist_0 = 5000.0f;
    var _S15 : bool = RayVsPlane_0(rayPos_2, rayDir_2, &(hitInfo_0), vec4<f32>(0.0f, 1.0f, 0.0f, 0.0f), vec3<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f));
    if(_S15)
    {
        var shade_0 : f32 = mix(0.80000001192092896f, 0.40000000596046448f, checkersGradTriangle_0((rayPos_2 + rayDir_2 * vec3<f32>(hitInfo_0.dist_0)).xz / vec2<f32>(100.0f)));
        hitInfo_0.albedo_0 = vec3<f32>(shade_0, shade_0, shade_0);
    }
    var _S16 : bool = RayVsSphere_0(rayPos_2, rayDir_2, &(hitInfo_0), vec4<f32>(-60.0f, 10.0f, 0.0f, 10.0f), vec3<f32>(1.0f, 0.0f, 1.0f));
    var _S17 : bool = RayVsSphere_0(rayPos_2, rayDir_2, &(hitInfo_0), vec4<f32>(-30.0f, 20.0f, 0.0f, 10.0f), vec3<f32>(1.0f, 0.0f, 0.0f));
    var _S18 : bool = RayVsSphere_0(rayPos_2, rayDir_2, &(hitInfo_0), vec4<f32>(0.0f, 30.0f, 0.0f, 10.0f), vec3<f32>(0.0f, 1.0f, 0.0f));
    var _S19 : bool = RayVsSphere_0(rayPos_2, rayDir_2, &(hitInfo_0), vec4<f32>(30.0f, 40.0f, 0.0f, 10.0f), vec3<f32>(0.0f, 0.0f, 1.0f));
    var _S20 : bool = RayVsSphere_0(rayPos_2, rayDir_2, &(hitInfo_0), vec4<f32>(60.0f, 50.0f, 0.0f, 10.0f), vec3<f32>(1.0f, 1.0f, 0.0f));
    var _S21 : bool;
    if(bool(_RayTraceCSCB.PosLightEnabled_0))
    {
        _S21 = !shadowRay_0;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var c_lightPos_0 : vec3<f32> = _RayTraceCSCB.PosLightPosition_0;
        var c_lightDir_0 : vec3<f32> = _RayTraceCSCB.PosLightShineDir_0;
        var c_cosThetaOuter_0 : f32 = _RayTraceCSCB.PosLightShineCosThetaOuter_0;
        var c_cosThetaInner_0 : f32 = _RayTraceCSCB.PosLightShineCosThetaInner_0;
        var _S22 : bool = RayVsSphere_0(rayPos_2, rayDir_2, &(hitInfo_0), vec4<f32>(_RayTraceCSCB.PosLightPosition_0, _RayTraceCSCB.PosLightRadius_0), _S14);
        if(_S22)
        {
            hitInfo_0.emissive_0 = vec3<f32>(smoothstep(c_cosThetaOuter_0, c_cosThetaInner_0, dot(normalize(c_lightPos_0 - (rayPos_2 + rayDir_2 * vec3<f32>(hitInfo_0.dist_0))), - c_lightDir_0))) * _RayTraceCSCB.PosLightColor_0 * vec3<f32>(_RayTraceCSCB.PosLightBrightness_0) * vec3<f32>(10.0f);
        }
    }
    return hitInfo_0;
}

fn RandomFloat01_0( state_1 : ptr<function, u32>) -> f32
{
    var _S23 : u32 = HashPCG_0(&((*state_1)));
    return f32(_S23) / 4.294967296e+09f;
}

fn LDSShuffle1D_GetValueAtIndex_0( index_0 : u32,  numItems_0 : u32,  coprime_0 : u32,  seed_1 : u32) -> u32
{
    var _S24 : u32 = index_0 % numItems_0;
    var _S25 : u32 = (_S24 * coprime_0 + seed_1) % numItems_0;
    return _S25;
}

fn ReadVec2STTexture_0( px_0 : vec3<u32>,  tex_0 : texture_2d_array<f32>,  convertToNeg1Plus1_0 : bool) -> vec2<f32>
{
    var _S26 : vec3<u32> = px_0;
    var dims_0 : vec3<u32>;
    var _S27 : u32 = dims_0[i32(0)];
    var _S28 : u32 = dims_0[i32(1)];
    var _S29 : u32 = dims_0[i32(2)];
    {var dim = textureDimensions((tex_0));((_S27)) = dim.x;((_S28)) = dim.y;((_S29)) = textureNumLayers((tex_0));};
    dims_0[i32(0)] = _S27;
    dims_0[i32(1)] = _S28;
    dims_0[i32(2)] = _S29;
    var cycleCount_0 : u32 = px_0.z / dims_0.z;
    var shuffleIndex_0 : u32 = LDSShuffle1D_GetValueAtIndex_0(cycleCount_0, u32(16384), u32(10127), u32(435));
    var _S30 : u32 = shuffleIndex_0 % dims_0.x;
    _S26[i32(0)] = _S26[i32(0)] + _S30;
    var _S31 : u32 = shuffleIndex_0 / dims_0.x;
    _S26[i32(1)] = _S26[i32(1)] + _S31;
    var _S32 : vec3<u32> = _S26 % dims_0;
    var _S33 : vec4<i32> = vec4<i32>(vec4<u32>(_S32, u32(0)));
    var ret_0 : vec2<f32> = (textureLoad((tex_0), ((_S33)).xy, i32(((_S33)).z), ((_S33)).w).xy).xy;
    var ret_1 : vec2<f32>;
    if(convertToNeg1Plus1_0)
    {
        ret_1 = ret_0 * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    }
    else
    {
        ret_1 = ret_0;
    }
    return ret_1;
}

fn Reinhard_0( color_0 : vec3<f32>) -> vec3<f32>
{
    return color_0 / (vec3<f32>(1.0f) + color_0);
}

fn ACESFilm_0( x_1 : vec3<f32>) -> vec3<f32>
{
    var _S34 : vec3<f32> = x_1 * vec3<f32>(0.60000002384185791f);
    return saturate(_S34 * (vec3<f32>(2.50999999046325684f) * _S34 + vec3<f32>(0.02999999932944775f)) / (_S34 * (vec3<f32>(2.43000006675720215f) * _S34 + vec3<f32>(0.5899999737739563f)) + vec3<f32>(0.14000000059604645f)));
}

fn RRTAndODTFit_0( v_1 : vec3<f32>) -> vec3<f32>
{
    return (v_1 * (v_1 + vec3<f32>(0.02457859925925732f)) - vec3<f32>(0.0000905370034161f)) / (v_1 * (vec3<f32>(0.98372900485992432f) * v_1 + vec3<f32>(0.43295100331306458f)) + vec3<f32>(0.23808099329471588f));
}

fn ACESFitted_0( color_1 : vec3<f32>) -> vec3<f32>
{
    return saturate((((RRTAndODTFit_0((((color_1) * (mat3x3<f32>(vec3<f32>(0.59719002246856689f, 0.35457998514175415f, 0.04822999984025955f), vec3<f32>(0.07599999755620956f, 0.9083399772644043f, 0.01565999910235405f), vec3<f32>(0.0284000001847744f, 0.1338299959897995f, 0.83776998519897461f))))))) * (mat3x3<f32>(vec3<f32>(1.60475003719329834f, -0.53108000755310059f, -0.07366999983787537f), vec3<f32>(-0.10208000242710114f, 1.10812997817993164f, -0.00604999996721745f), vec3<f32>(-0.00326999998651445f, -0.07276000082492828f, 1.0760200023651123f))))));
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S35 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S35 = sRGBLo_0.x;
    }
    else
    {
        _S35 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S35;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S35 = sRGBLo_0.y;
    }
    else
    {
        _S35 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S35;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S35 = sRGBLo_0.z;
    }
    else
    {
        _S35 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S35;
    return sRGB_0;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_1 : vec2<u32> = DTid_0.xy;
    var renderSize_0 : vec2<u32>;
    var _S36 : u32 = renderSize_0[i32(0)];
    var _S37 : u32 = renderSize_0[i32(1)];
    {var dim = textureDimensions((Output));((_S36)) = dim.x;((_S37)) = dim.y;};
    renderSize_0[i32(0)] = _S36;
    renderSize_0[i32(1)] = _S37;
    var _S38 : vec2<f32> = (vec2<f32>(px_1) + vec2<f32>(0.5f)) / vec2<f32>(renderSize_0) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S38;
    screenPos_0[i32(1)] = - _S38.y;
    var _S39 : vec4<f32> = (((mat4x4<f32>(_RayTraceCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _RayTraceCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, 1.0f, 1.0f))));
    var world_0 : vec4<f32> = _S39;
    var _S40 : vec3<f32> = _S39.xyz / vec3<f32>(_S39.w);
    world_0.x = _S40.x;
    world_0.y = _S40.y;
    world_0.z = _S40.z;
    var rayPos_3 : vec3<f32> = _RayTraceCSCB.CameraPos_0;
    var rayDir_3 : vec3<f32> = normalize(world_0.xyz - _RayTraceCSCB.CameraPos_0);
    var _S41 : vec3<f32> = (textureSampleLevel((_loadedTexture_0), (texSampler), (SampleSphericalMap_0(rayDir_3)), (0.0f))).xyz;
    var rngState_0 : u32 = HashInit_0(vec3<u32>(px_1, _RayTraceCSCB.FrameIndex_0));
    var hitInfo_1 : SRayHitInfo_0 = RayVsScene_0(rayPos_3, rayDir_3, false);
    var lightTangent_0 : vec3<f32>;
    var color_2 : vec3<f32>;
    if((hitInfo_1.dist_0) < 5000.0f)
    {
        var color_3 : vec3<f32> = _RayTraceCSCB.AmbientLight_0 * hitInfo_1.albedo_0 + hitInfo_1.emissive_0;
        var i_1 : i32;
        var diskPoint_0 : vec2<f32>;
        if(bool(_RayTraceCSCB.DirLightEnabled_0))
        {
            var c_lightDir_1 : vec3<f32> = normalize(- _RayTraceCSCB.DirLightDirection_0);
            const _S42 : vec3<f32> = vec3<f32>(0.0f, 1.0f, 0.0f);
            if((abs(dot(c_lightDir_1, _S42))) < 0.5f)
            {
                lightTangent_0 = cross(c_lightDir_1, _S42);
            }
            else
            {
                lightTangent_0 = cross(c_lightDir_1, vec3<f32>(1.0f, 0.0f, 0.0f));
            }
            var lightTangent_1 : vec3<f32> = normalize(lightTangent_0);
            var _S43 : vec3<f32> = normalize(cross(lightTangent_1, c_lightDir_1));
            var _S44 : vec3<f32> = rayPos_3 + rayDir_3 * vec3<f32>(hitInfo_1.dist_0) + hitInfo_1.normal_0 * vec3<f32>(0.00999999977648258f);
            i_1 = i32(0);
            color_2 = color_3;
            for(;;)
            {
                if(i_1 < (_RayTraceCSCB.DirLightSampleCount_0))
                {
                }
                else
                {
                    break;
                }
                const _S45 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
                switch(_RayTraceCSCB.NoiseType_0)
                {
                case i32(0), :
                    {
                        var _S46 : f32 = RandomFloat01_0(&(rngState_0));
                        var _S47 : f32 = RandomFloat01_0(&(rngState_0));
                        var pointRadius_0 : f32 = _RayTraceCSCB.DirLightRadius_0 * sqrt(_S46);
                        var pointAngle_0 : f32 = _S47 * 2.0f * 3.14159274101257324f;
                        diskPoint_0 = vec2<f32>(pointRadius_0 * cos(pointAngle_0), pointRadius_0 * sin(pointAngle_0));
                        break;
                    }
                case i32(1), :
                    {
                        var uv_1 : vec2<f32> = ReadVec2STTexture_0(vec3<u32>(px_1, _RayTraceCSCB.FrameIndex_0 * u32(_RayTraceCSCB.DirLightSampleCount_0) + u32(i_1)), _loadedTexture_1, false);
                        var pointRadius_1 : f32 = _RayTraceCSCB.DirLightRadius_0 * sqrt(uv_1.x);
                        var pointAngle_1 : f32 = uv_1.y * 2.0f * 3.14159274101257324f;
                        diskPoint_0 = vec2<f32>(pointRadius_1 * cos(pointAngle_1), pointRadius_1 * sin(pointAngle_1));
                        break;
                    }
                case i32(2), :
                    {
                        var _S48 : f32 = _RayTraceCSCB.DirLightRadius_0;
                        var _S49 : vec2<f32> = ReadVec2STTexture_0(vec3<u32>(px_1, _RayTraceCSCB.FrameIndex_0 * u32(_RayTraceCSCB.DirLightSampleCount_0) + u32(i_1)), _loadedTexture_2, true);
                        diskPoint_0 = vec2<f32>(_S48) * _S49;
                        break;
                    }
                case default, :
                    {
                        diskPoint_0 = _S45;
                        break;
                    }
                }
                var shadowRayDir_0 : vec3<f32> = normalize(c_lightDir_1 + vec3<f32>(diskPoint_0.x) * lightTangent_1 + vec3<f32>(diskPoint_0.y) * _S43);
                if((RayVsScene_0(_S44, shadowRayDir_0, true).dist_0) >= 5000.0f)
                {
                    color_2 = color_2 + vec3<f32>(clamp(dot(shadowRayDir_0, hitInfo_1.normal_0), 0.0f, 1.0f)) * hitInfo_1.albedo_0 * _RayTraceCSCB.DirLightColor_0 * vec3<f32>(_RayTraceCSCB.DirLightBrightness_0) / vec3<f32>(f32(_RayTraceCSCB.DirLightSampleCount_0));
                }
                i_1 = i_1 + i32(1);
            }
        }
        else
        {
            color_2 = color_3;
        }
        if(bool(_RayTraceCSCB.PosLightEnabled_0))
        {
            var shadowRayPos_0 : vec3<f32> = rayPos_3 + rayDir_3 * vec3<f32>(hitInfo_1.dist_0) + hitInfo_1.normal_0 * vec3<f32>(0.00999999977648258f);
            var c_lightDir_2 : vec3<f32> = normalize(_RayTraceCSCB.PosLightPosition_0 - shadowRayPos_0);
            const _S50 : vec3<f32> = vec3<f32>(0.0f, 1.0f, 0.0f);
            if((abs(dot(c_lightDir_2, _S50))) < 0.5f)
            {
                lightTangent_0 = cross(c_lightDir_2, _S50);
            }
            else
            {
                lightTangent_0 = cross(c_lightDir_2, vec3<f32>(1.0f, 0.0f, 0.0f));
            }
            var lightTangent_2 : vec3<f32> = normalize(lightTangent_0);
            var _S51 : vec3<f32> = normalize(cross(lightTangent_2, c_lightDir_2));
            i_1 = i32(0);
            for(;;)
            {
                if(i_1 < (_RayTraceCSCB.PosLightSampleCount_0))
                {
                }
                else
                {
                    break;
                }
                const _S52 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
                switch(_RayTraceCSCB.NoiseType_0)
                {
                case i32(0), :
                    {
                        var _S53 : f32 = RandomFloat01_0(&(rngState_0));
                        var _S54 : f32 = RandomFloat01_0(&(rngState_0));
                        var pointRadius_2 : f32 = _RayTraceCSCB.PosLightRadius_0 * sqrt(_S53);
                        var pointAngle_2 : f32 = _S54 * 2.0f * 3.14159274101257324f;
                        diskPoint_0 = vec2<f32>(pointRadius_2 * cos(pointAngle_2), pointRadius_2 * sin(pointAngle_2));
                        break;
                    }
                case i32(1), :
                    {
                        var uv_2 : vec2<f32> = ReadVec2STTexture_0(vec3<u32>(px_1, _RayTraceCSCB.FrameIndex_0 * u32(_RayTraceCSCB.PosLightSampleCount_0) + u32(i_1)), _loadedTexture_1, false);
                        var pointRadius_3 : f32 = _RayTraceCSCB.PosLightRadius_0 * sqrt(uv_2.x);
                        var pointAngle_3 : f32 = uv_2.y * 2.0f * 3.14159274101257324f;
                        diskPoint_0 = vec2<f32>(pointRadius_3 * cos(pointAngle_3), pointRadius_3 * sin(pointAngle_3));
                        break;
                    }
                case i32(2), :
                    {
                        var _S55 : f32 = _RayTraceCSCB.PosLightRadius_0;
                        var _S56 : vec2<f32> = ReadVec2STTexture_0(vec3<u32>(px_1, _RayTraceCSCB.FrameIndex_0 * u32(_RayTraceCSCB.PosLightSampleCount_0) + u32(i_1)), _loadedTexture_2, true);
                        diskPoint_0 = vec2<f32>(_S55) * _S56;
                        break;
                    }
                case default, :
                    {
                        diskPoint_0 = _S52;
                        break;
                    }
                }
                var shadowRayDir_1 : vec3<f32> = normalize(_RayTraceCSCB.PosLightPosition_0 + vec3<f32>(diskPoint_0.x) * lightTangent_2 + vec3<f32>(diskPoint_0.y) * _S51 - shadowRayPos_0);
                if((RayVsScene_0(shadowRayPos_0, shadowRayDir_1, true).dist_0) >= 5000.0f)
                {
                    color_2 = color_2 + vec3<f32>((smoothstep(_RayTraceCSCB.PosLightShineCosThetaOuter_0, _RayTraceCSCB.PosLightShineCosThetaInner_0, dot(shadowRayDir_1, - normalize(_RayTraceCSCB.PosLightShineDir_0))) * clamp(dot(shadowRayDir_1, hitInfo_1.normal_0), 0.0f, 1.0f))) * hitInfo_1.albedo_0 * _RayTraceCSCB.PosLightColor_0 * vec3<f32>(_RayTraceCSCB.PosLightBrightness_0) / vec3<f32>(f32(_RayTraceCSCB.PosLightSampleCount_0));
                }
                i_1 = i_1 + i32(1);
            }
        }
    }
    else
    {
        if(bool(_RayTraceCSCB.DirLightEnabled_0))
        {
            var c_lightDir_3 : vec3<f32> = normalize(- _RayTraceCSCB.DirLightDirection_0);
            const _S57 : vec3<f32> = vec3<f32>(0.0f, 1.0f, 0.0f);
            if((abs(dot(c_lightDir_3, _S57))) < 0.5f)
            {
                lightTangent_0 = cross(c_lightDir_3, _S57);
            }
            else
            {
                lightTangent_0 = cross(c_lightDir_3, vec3<f32>(1.0f, 0.0f, 0.0f));
            }
            if((dot(rayDir_3, c_lightDir_3)) >= (dot(c_lightDir_3, normalize(c_lightDir_3 + normalize(lightTangent_0) * vec3<f32>(_RayTraceCSCB.DirLightRadius_0)))))
            {
                color_2 = _RayTraceCSCB.DirLightColor_0 * vec3<f32>(_RayTraceCSCB.DirLightBrightness_0) * vec3<f32>(10.0f);
            }
            else
            {
                color_2 = _S41;
            }
        }
        else
        {
            color_2 = _S41;
        }
    }
    var _S58 : bool;
    if(bool(_RayTraceCSCB.CameraChanged_0))
    {
        _S58 = true;
    }
    else
    {
        _S58 = bool(_RayTraceCSCB.ResetAccum_0);
    }
    if(_S58)
    {
        _S58 = true;
    }
    else
    {
        _S58 = (_RayTraceCSCB.FrameIndex_0) < u32(10);
    }
    if(_S58)
    {
        textureStore((Accum), (px_1), (vec4<f32>(color_2, 1.0f)));
    }
    else
    {
        switch(_RayTraceCSCB.TemporalFilter_0)
        {
        case i32(0), :
            {
                textureStore((Accum), (px_1), (vec4<f32>(color_2, 1.0f)));
                break;
            }
        case i32(1), :
            {
                var _S59 : vec4<f32> = (textureLoad((AccumReadOnly), (vec2<i32>(px_1))));
                var color_4 : vec3<f32> = mix(_S59.xyz, color_2, vec3<f32>(_RayTraceCSCB.EMAAlpha_0));
                textureStore((Accum), (px_1), (vec4<f32>(color_4, 1.0f)));
                color_2 = color_4;
                break;
            }
        case i32(2), :
            {
                var _S60 : vec4<f32> = (textureLoad((AccumReadOnly), (vec2<i32>(px_1))));
                var oldColor_0 : vec4<f32> = _S60.xyzw;
                var sampleCount_0 : f32 = oldColor_0.w;
                var color_5 : vec3<f32> = mix(oldColor_0.xyz, color_2, vec3<f32>((1.0f / sampleCount_0)));
                textureStore((Accum), (px_1), (vec4<f32>(color_5, sampleCount_0 + 1.0f)));
                color_2 = color_5;
                break;
            }
        case default, :
            {
                break;
            }
        }
    }
    var color_6 : vec3<f32> = max(color_2 * vec3<f32>(pow(2.0f, _RayTraceCSCB.ExposureFStops_0)), vec3<f32>(0.0f, 0.0f, 0.0f));
    switch(_RayTraceCSCB.ToneMapping_0)
    {
    case i32(0), :
        {
            color_2 = color_6;
            break;
        }
    case i32(1), :
        {
            color_2 = Reinhard_0(color_6);
            break;
        }
    case i32(2), :
        {
            color_2 = ACESFilm_0(color_6);
            break;
        }
    case i32(3), :
        {
            color_2 = ACESFitted_0(color_6);
            break;
        }
    case default, :
        {
            color_2 = color_6;
            break;
        }
    }
    textureStore((Output), (px_1), (vec4<f32>(LinearToSRGB_0(color_2), 1.0f)));
    return;
}

`;

// -------------------- Enums

static Enum_NoiseTypes =
{
    White: 0,
    FAST: 1,
    FAST_IS: 2,
    _count: 3,
}

static Enum_ToneMappingOperation =
{
    None: 0,
    Reinhard_Simple: 1,
    ACES_Luminance: 2,
    ACES: 3,
    _count: 4,
}

static Enum_TemporalFilters =
{
    None: 0,
    EMA: 1,
    MonteCarlo: 2,
    _count: 3,
}

// -------------------- Private Members

// Texture RenderTarget
texture_RenderTarget = null;
texture_RenderTarget_size = [0, 0, 0];
texture_RenderTarget_format = "";
texture_RenderTarget_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Accum
texture_Accum = null;
texture_Accum_size = [0, 0, 0];
texture_Accum_format = "";
texture_Accum_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture RayTrace_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_RayTrace_Output_ReadOnly = null;
texture_RayTrace_Output_ReadOnly_size = [0, 0, 0];
texture_RayTrace_Output_ReadOnly_format = "";
texture_RayTrace_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture RayTrace_Accum_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_RayTrace_Accum_ReadOnly = null;
texture_RayTrace_Accum_ReadOnly_size = [0, 0, 0];
texture_RayTrace_Accum_ReadOnly_format = "";
texture_RayTrace_Accum_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_1
texture__loadedTexture_1 = null;
texture__loadedTexture_1_size = [0, 0, 0];
texture__loadedTexture_1_format = "";
texture__loadedTexture_1_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_2
texture__loadedTexture_2 = null;
texture__loadedTexture_2_size = [0, 0, 0];
texture__loadedTexture_2_format = "";
texture__loadedTexture_2_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Constant buffer _RayTraceCSCB
constantBuffer__RayTraceCSCB = null;
constantBuffer__RayTraceCSCB_size = 256;
constantBuffer__RayTraceCSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader RayTrace
Hash_Compute_RayTrace = 0;
ShaderModule_Compute_RayTrace = null;
BindGroupLayout_Compute_RayTrace = null;
PipelineLayout_Compute_RayTrace = null;
Pipeline_Compute_RayTrace = null;

// -------------------- Public Variables

variable_NoiseType = this.constructor.Enum_NoiseTypes.FAST_IS;
variableDefault_NoiseType = this.constructor.Enum_NoiseTypes.FAST_IS;
variableChanged_NoiseType = false;
variable_ExposureFStops = 0.000000;  // Exposure, in FStops. Every +1 is double brightness. Every -1 is half brightness.
variableDefault_ExposureFStops = 0.000000;  // Exposure, in FStops. Every +1 is double brightness. Every -1 is half brightness.
variableChanged_ExposureFStops = false;
variable_ToneMapping = this.constructor.Enum_ToneMappingOperation.ACES;
variableDefault_ToneMapping = this.constructor.Enum_ToneMappingOperation.ACES;
variableChanged_ToneMapping = false;
variable_Animate = true;
variableDefault_Animate = true;
variableChanged_Animate = false;
variable_AmbientLight = [ 0.050000, 0.050000, 0.050000 ];
variableDefault_AmbientLight = [ 0.050000, 0.050000, 0.050000 ];
variableChanged_AmbientLight = [ false, false, false ];
variable_TemporalFilter = this.constructor.Enum_TemporalFilters.None;
variableDefault_TemporalFilter = this.constructor.Enum_TemporalFilters.None;
variableChanged_TemporalFilter = false;
variable_EMAAlpha = 0.100000;  // How much to lerp towards the new from the old, when using EMA temporal filtering
variableDefault_EMAAlpha = 0.100000;  // How much to lerp towards the new from the old, when using EMA temporal filtering
variableChanged_EMAAlpha = false;
variable_DirLightEnabled = true;
variableDefault_DirLightEnabled = true;
variableChanged_DirLightEnabled = false;
variable_DirLightSampleCount = 16;  // How many stochastic samples to take per frame
variableDefault_DirLightSampleCount = 16;  // How many stochastic samples to take per frame
variableChanged_DirLightSampleCount = false;
variable_DirLightColor = [ 1.000000, 0.800000, 0.500000 ];
variableDefault_DirLightColor = [ 1.000000, 0.800000, 0.500000 ];
variableChanged_DirLightColor = [ false, false, false ];
variable_DirLightBrightness = 1.000000;
variableDefault_DirLightBrightness = 1.000000;
variableChanged_DirLightBrightness = false;
variable_DirLightDirection = [ 1.000000, -1.000000, -1.000000 ];  // What direction the light shines in
variableDefault_DirLightDirection = [ 1.000000, -1.000000, -1.000000 ];  // What direction the light shines in
variableChanged_DirLightDirection = [ false, false, false ];
variable_DirLightRadius = 0.100000;  // radius of light disk at 1 unit away
variableDefault_DirLightRadius = 0.100000;  // radius of light disk at 1 unit away
variableChanged_DirLightRadius = false;
variable_FrameIndex = 0;
variableDefault_FrameIndex = 0;
variableChanged_FrameIndex = false;
variable_PosLightEnabled = true;
variableDefault_PosLightEnabled = true;
variableChanged_PosLightEnabled = false;
variable_PosLightSampleCount = 16;  // How many stochastic samples to take per frame
variableDefault_PosLightSampleCount = 16;  // How many stochastic samples to take per frame
variableChanged_PosLightSampleCount = false;
variable_PosLightColor = [ 0.500000, 0.800000, 1.000000 ];
variableDefault_PosLightColor = [ 0.500000, 0.800000, 1.000000 ];
variableChanged_PosLightColor = [ false, false, false ];
variable_PosLightBrightness = 1.000000;
variableDefault_PosLightBrightness = 1.000000;
variableChanged_PosLightBrightness = false;
variable_PosLightPosition = [ 0.000000, 60.000000, 40.000000 ];
variableDefault_PosLightPosition = [ 0.000000, 60.000000, 40.000000 ];
variableChanged_PosLightPosition = [ false, false, false ];
variable_PosLightRadius = 5.000000;
variableDefault_PosLightRadius = 5.000000;
variableChanged_PosLightRadius = false;
variable_PosLightShineDir = [ 0.000000, -1.000000, -1.000000 ];  // The direction of the spot light. Cos Theta Inner / Outer control the spot light
variableDefault_PosLightShineDir = [ 0.000000, -1.000000, -1.000000 ];  // The direction of the spot light. Cos Theta Inner / Outer control the spot light
variableChanged_PosLightShineDir = [ false, false, false ];
variable_PosLightShineCosThetaInner = 0.800000;  // direction to light, dotted by -c_lightDir. light starts to fade here.
variableDefault_PosLightShineCosThetaInner = 0.800000;  // direction to light, dotted by -c_lightDir. light starts to fade here.
variableChanged_PosLightShineCosThetaInner = false;
variable_PosLightShineCosThetaOuter = 0.700000;  // direction to light, dotted by -c_lightDir. light finishes fading here.
variableDefault_PosLightShineCosThetaOuter = 0.700000;  // direction to light, dotted by -c_lightDir. light finishes fading here.
variableChanged_PosLightShineCosThetaOuter = false;
variable_ResetAccum = false;
variableDefault_ResetAccum = false;
variableChanged_ResetAccum = false;

// -------------------- Private Variables

variable_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableDefault_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableChanged_CameraPos = [ false, false, false ];
variable_InvViewProjMtx = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_InvViewProjMtx = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_InvViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_CameraChanged = false;
variableDefault_CameraChanged = false;
variableChanged_CameraChanged = false;
// -------------------- Structs

static StructOffsets__RayTraceCSCB =
{
    AmbientLight_0: 0,
    AmbientLight_1: 4,
    AmbientLight_2: 8,
    CameraChanged: 12,
    CameraPos_0: 16,
    CameraPos_1: 20,
    CameraPos_2: 24,
    DirLightBrightness: 28,
    DirLightColor_0: 32,
    DirLightColor_1: 36,
    DirLightColor_2: 40,
    _padding0: 44,
    DirLightDirection_0: 48,
    DirLightDirection_1: 52,
    DirLightDirection_2: 56,
    DirLightEnabled: 60,
    DirLightRadius: 64,
    DirLightSampleCount: 68,
    EMAAlpha: 72,
    ExposureFStops: 76,
    FrameIndex: 80,
    _padding1: 84,
    _padding2: 88,
    _padding3: 92,
    InvViewProjMtx_0: 96,
    InvViewProjMtx_1: 100,
    InvViewProjMtx_2: 104,
    InvViewProjMtx_3: 108,
    InvViewProjMtx_4: 112,
    InvViewProjMtx_5: 116,
    InvViewProjMtx_6: 120,
    InvViewProjMtx_7: 124,
    InvViewProjMtx_8: 128,
    InvViewProjMtx_9: 132,
    InvViewProjMtx_10: 136,
    InvViewProjMtx_11: 140,
    InvViewProjMtx_12: 144,
    InvViewProjMtx_13: 148,
    InvViewProjMtx_14: 152,
    InvViewProjMtx_15: 156,
    NoiseType: 160,
    PosLightBrightness: 164,
    _padding4: 168,
    _padding5: 172,
    PosLightColor_0: 176,
    PosLightColor_1: 180,
    PosLightColor_2: 184,
    PosLightEnabled: 188,
    PosLightPosition_0: 192,
    PosLightPosition_1: 196,
    PosLightPosition_2: 200,
    PosLightRadius: 204,
    PosLightSampleCount: 208,
    PosLightShineCosThetaInner: 212,
    PosLightShineCosThetaOuter: 216,
    _padding6: 220,
    PosLightShineDir_0: 224,
    PosLightShineDir_1: 228,
    PosLightShineDir_2: 232,
    ResetAccum: 236,
    TemporalFilter: 240,
    ToneMapping: 244,
    _padding7: 248,
    _padding8: 252,
    _size: 256,
}


async SetVarsBefore()
{
    if (this.variable_Animate)
    {
        this.variableChanged_FrameIndex = true;
        this.variable_FrameIndex = this.variable_FrameIndex + 1;
    }


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
    // Handle (re)creation of texture RenderTarget
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1024) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 768) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_RenderTarget !== null && (this.texture_RenderTarget_format != desiredFormat || this.texture_RenderTarget_size[0] != desiredSize[0] || this.texture_RenderTarget_size[1] != desiredSize[1] || this.texture_RenderTarget_size[2] != desiredSize[2]))
        {
            this.texture_RenderTarget.destroy();
            this.texture_RenderTarget = null;
        }

        if (this.texture_RenderTarget === null)
        {
            this.texture_RenderTarget_size = desiredSize.slice();
            this.texture_RenderTarget_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_RenderTarget_format))
                viewFormats.push(this.texture_RenderTarget_format);

            this.texture_RenderTarget = device.createTexture({
                label: "texture BlueNoiseAreaLights.RenderTarget",
                size: this.texture_RenderTarget_size,
                format: Shared.GetNonSRGBFormat(this.texture_RenderTarget_format),
                usage: this.texture_RenderTarget_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Accum
    {
        const baseSize = this.texture_RenderTarget_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba32float";
        if (this.texture_Accum !== null && (this.texture_Accum_format != desiredFormat || this.texture_Accum_size[0] != desiredSize[0] || this.texture_Accum_size[1] != desiredSize[1] || this.texture_Accum_size[2] != desiredSize[2]))
        {
            this.texture_Accum.destroy();
            this.texture_Accum = null;
        }

        if (this.texture_Accum === null)
        {
            this.texture_Accum_size = desiredSize.slice();
            this.texture_Accum_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Accum_format))
                viewFormats.push(this.texture_Accum_format);

            this.texture_Accum = device.createTexture({
                label: "texture BlueNoiseAreaLights.Accum",
                size: this.texture_Accum_size,
                format: Shared.GetNonSRGBFormat(this.texture_Accum_format),
                usage: this.texture_Accum_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture RayTrace_Output_ReadOnly
    {
        const baseSize = this.texture_RenderTarget_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_RenderTarget_format;
        if (this.texture_RayTrace_Output_ReadOnly !== null && (this.texture_RayTrace_Output_ReadOnly_format != desiredFormat || this.texture_RayTrace_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_RayTrace_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_RayTrace_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_RayTrace_Output_ReadOnly.destroy();
            this.texture_RayTrace_Output_ReadOnly = null;
        }

        if (this.texture_RayTrace_Output_ReadOnly === null)
        {
            this.texture_RayTrace_Output_ReadOnly_size = desiredSize.slice();
            this.texture_RayTrace_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_RayTrace_Output_ReadOnly_format))
                viewFormats.push(this.texture_RayTrace_Output_ReadOnly_format);

            this.texture_RayTrace_Output_ReadOnly = device.createTexture({
                label: "texture BlueNoiseAreaLights.RayTrace_Output_ReadOnly",
                size: this.texture_RayTrace_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_RayTrace_Output_ReadOnly_format),
                usage: this.texture_RayTrace_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture RayTrace_Accum_ReadOnly
    {
        const baseSize = this.texture_Accum_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Accum_format;
        if (this.texture_RayTrace_Accum_ReadOnly !== null && (this.texture_RayTrace_Accum_ReadOnly_format != desiredFormat || this.texture_RayTrace_Accum_ReadOnly_size[0] != desiredSize[0] || this.texture_RayTrace_Accum_ReadOnly_size[1] != desiredSize[1] || this.texture_RayTrace_Accum_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_RayTrace_Accum_ReadOnly.destroy();
            this.texture_RayTrace_Accum_ReadOnly = null;
        }

        if (this.texture_RayTrace_Accum_ReadOnly === null)
        {
            this.texture_RayTrace_Accum_ReadOnly_size = desiredSize.slice();
            this.texture_RayTrace_Accum_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_RayTrace_Accum_ReadOnly_format))
                viewFormats.push(this.texture_RayTrace_Accum_ReadOnly_format);

            this.texture_RayTrace_Accum_ReadOnly = device.createTexture({
                label: "texture BlueNoiseAreaLights.RayTrace_Accum_ReadOnly",
                size: this.texture_RayTrace_Accum_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_RayTrace_Accum_ReadOnly_format),
                usage: this.texture_RayTrace_Accum_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Load texture _loadedTexture_0 from "Arches_E_PineTree_3k.png"
    if (this.texture__loadedTexture_0 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/Arches_E_PineTree_3k.png", this.texture__loadedTexture_0_usageFlags, "2d");
        this.texture__loadedTexture_0 = loadedTex.texture;
        this.texture__loadedTexture_0_size = loadedTex.size;
        this.texture__loadedTexture_0_format = "rgba8unorm";
        this.texture__loadedTexture_0_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_1 from "FASTUniformSquare/vector2_uniform_gauss1_0_Gauss10_separate05_%i.png"
    if (this.texture__loadedTexture_1 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/FASTUniformSquare/vector2_uniform_gauss1_0_Gauss10_separate05_%i.png", this.texture__loadedTexture_1_usageFlags, "2d");
        this.texture__loadedTexture_1 = loadedTex.texture;
        this.texture__loadedTexture_1_size = loadedTex.size;
        this.texture__loadedTexture_1_format = "rgba8unorm";
        this.texture__loadedTexture_1_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_2 from "FASTUniformCircle/UniformCircle_%i.png"
    if (this.texture__loadedTexture_2 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/FASTUniformCircle/UniformCircle_%i.png", this.texture__loadedTexture_2_usageFlags, "2d");
        this.texture__loadedTexture_2 = loadedTex.texture;
        this.texture__loadedTexture_2_size = loadedTex.size;
        this.texture__loadedTexture_2_format = "rgba8unorm";
        this.texture__loadedTexture_2_usageFlags = loadedTex.usageFlags;
    }
    // (Re)create compute shader RayTrace
    {
        const bindGroupEntries =
        [
            {
                // Output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_RenderTarget_format), viewDimension: "2d" }
            },
            {
                // Accum
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Accum_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_RayTrace_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // AccumReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_RayTrace_Accum_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _loadedTexture_0
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
            {
                // _loadedTexture_1
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_1_format).sampleType }
            },
            {
                // _loadedTexture_2
                binding: 6,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_2_format).sampleType }
            },
            {
                // _RayTraceCSCB
                binding: 7,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
            {
                // texSampler
                binding: 8,
                visibility: GPUShaderStage.COMPUTE,
                sampler: { },
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_RayTrace === null || newHash !== this.Hash_Compute_RayTrace)
        {
            this.Hash_Compute_RayTrace = newHash;

            let shaderCode = class_BlueNoiseAreaLights.ShaderCode_RayTrace_RayTraceCS;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_RenderTarget_format));
            shaderCode = shaderCode.replace("/*(Accum_format)*/", Shared.GetNonSRGBFormat(this.texture_Accum_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_RayTrace_Output_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(AccumReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_RayTrace_Accum_ReadOnly_format));

            this.ShaderModule_Compute_RayTrace = device.createShaderModule({ code: shaderCode, label: "Compute Shader RayTrace"});
            this.BindGroupLayout_Compute_RayTrace = device.createBindGroupLayout({
                label: "Compute Bind Group Layout RayTrace",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_RayTrace = device.createPipelineLayout({
                label: "Compute Pipeline Layout RayTrace",
                bindGroupLayouts: [this.BindGroupLayout_Compute_RayTrace],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_RayTrace = device.createComputePipeline({
                    label: "Compute Pipeline RayTrace",
                    layout: this.PipelineLayout_Compute_RayTrace,
                    compute: {
                        module: this.ShaderModule_Compute_RayTrace,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("RayTrace");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline RayTrace",
                    layout: this.PipelineLayout_Compute_RayTrace,
                    compute: {
                        module: this.ShaderModule_Compute_RayTrace,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_RayTrace = handle; this.loadingPromises.delete("RayTrace"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("BlueNoiseAreaLights.RenderTarget");

    encoder.popDebugGroup(); // "BlueNoiseAreaLights.RenderTarget"

    encoder.pushDebugGroup("BlueNoiseAreaLights.Accum");

    encoder.popDebugGroup(); // "BlueNoiseAreaLights.Accum"

    encoder.pushDebugGroup("BlueNoiseAreaLights.RayTrace_Output_ReadOnly");

    encoder.popDebugGroup(); // "BlueNoiseAreaLights.RayTrace_Output_ReadOnly"

    encoder.pushDebugGroup("BlueNoiseAreaLights.Copy_RayTrace_Output");

    // Copy texture RenderTarget to texture RayTrace_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_RenderTarget.mipLevelCount, this.texture_RayTrace_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_RenderTarget.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_RenderTarget.height >> mipIndex, 1);
            let mipDepth = this.texture_RenderTarget.depthOrArrayLayers;

            if (this.texture_RenderTarget.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_RenderTarget, mipLevel: mipIndex },
                { texture: this.texture_RayTrace_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "BlueNoiseAreaLights.Copy_RayTrace_Output"

    encoder.pushDebugGroup("BlueNoiseAreaLights.RayTrace_Accum_ReadOnly");

    encoder.popDebugGroup(); // "BlueNoiseAreaLights.RayTrace_Accum_ReadOnly"

    encoder.pushDebugGroup("BlueNoiseAreaLights.Copy_RayTrace_Accum");

    // Copy texture Accum to texture RayTrace_Accum_ReadOnly
    {
        const numMips = Math.min(this.texture_Accum.mipLevelCount, this.texture_RayTrace_Accum_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Accum.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Accum.height >> mipIndex, 1);
            let mipDepth = this.texture_Accum.depthOrArrayLayers;

            if (this.texture_Accum.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Accum, mipLevel: mipIndex },
                { texture: this.texture_RayTrace_Accum_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "BlueNoiseAreaLights.Copy_RayTrace_Accum"

    encoder.pushDebugGroup("BlueNoiseAreaLights._loadedTexture_0");

    encoder.popDebugGroup(); // "BlueNoiseAreaLights._loadedTexture_0"

    encoder.pushDebugGroup("BlueNoiseAreaLights._loadedTexture_1");

    encoder.popDebugGroup(); // "BlueNoiseAreaLights._loadedTexture_1"

    encoder.pushDebugGroup("BlueNoiseAreaLights._loadedTexture_2");

    encoder.popDebugGroup(); // "BlueNoiseAreaLights._loadedTexture_2"

    encoder.pushDebugGroup("BlueNoiseAreaLights._RayTraceCSCB");

    // Create constant buffer _RayTraceCSCB
    if (this.constantBuffer__RayTraceCSCB === null)
    {
        this.constantBuffer__RayTraceCSCB = device.createBuffer({
            label: "BlueNoiseAreaLights._RayTraceCSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__RayTraceCSCB._size),
            usage: this.constantBuffer__RayTraceCSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _RayTraceCSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__RayTraceCSCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.AmbientLight_0, this.variable_AmbientLight[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.AmbientLight_1, this.variable_AmbientLight[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.AmbientLight_2, this.variable_AmbientLight[2], true);
        view.setUint32(this.constructor.StructOffsets__RayTraceCSCB.CameraChanged, (this.variable_CameraChanged === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.CameraPos_0, this.variable_CameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.CameraPos_1, this.variable_CameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.CameraPos_2, this.variable_CameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightBrightness, this.variable_DirLightBrightness, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightColor_0, this.variable_DirLightColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightColor_1, this.variable_DirLightColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightColor_2, this.variable_DirLightColor[2], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightDirection_0, this.variable_DirLightDirection[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightDirection_1, this.variable_DirLightDirection[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightDirection_2, this.variable_DirLightDirection[2], true);
        view.setUint32(this.constructor.StructOffsets__RayTraceCSCB.DirLightEnabled, (this.variable_DirLightEnabled === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.DirLightRadius, this.variable_DirLightRadius, true);
        view.setInt32(this.constructor.StructOffsets__RayTraceCSCB.DirLightSampleCount, this.variable_DirLightSampleCount, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.EMAAlpha, this.variable_EMAAlpha, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.ExposureFStops, this.variable_ExposureFStops, true);
        view.setUint32(this.constructor.StructOffsets__RayTraceCSCB.FrameIndex, this.variable_FrameIndex, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_0, this.variable_InvViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_1, this.variable_InvViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_2, this.variable_InvViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_3, this.variable_InvViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_4, this.variable_InvViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_5, this.variable_InvViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_6, this.variable_InvViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_7, this.variable_InvViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_8, this.variable_InvViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_9, this.variable_InvViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_10, this.variable_InvViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_11, this.variable_InvViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_12, this.variable_InvViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_13, this.variable_InvViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_14, this.variable_InvViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.InvViewProjMtx_15, this.variable_InvViewProjMtx[15], true);
        view.setInt32(this.constructor.StructOffsets__RayTraceCSCB.NoiseType, this.variable_NoiseType, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightBrightness, this.variable_PosLightBrightness, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightColor_0, this.variable_PosLightColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightColor_1, this.variable_PosLightColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightColor_2, this.variable_PosLightColor[2], true);
        view.setUint32(this.constructor.StructOffsets__RayTraceCSCB.PosLightEnabled, (this.variable_PosLightEnabled === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightPosition_0, this.variable_PosLightPosition[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightPosition_1, this.variable_PosLightPosition[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightPosition_2, this.variable_PosLightPosition[2], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightRadius, this.variable_PosLightRadius, true);
        view.setInt32(this.constructor.StructOffsets__RayTraceCSCB.PosLightSampleCount, this.variable_PosLightSampleCount, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightShineCosThetaInner, this.variable_PosLightShineCosThetaInner, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightShineCosThetaOuter, this.variable_PosLightShineCosThetaOuter, true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightShineDir_0, this.variable_PosLightShineDir[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightShineDir_1, this.variable_PosLightShineDir[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayTraceCSCB.PosLightShineDir_2, this.variable_PosLightShineDir[2], true);
        view.setUint32(this.constructor.StructOffsets__RayTraceCSCB.ResetAccum, (this.variable_ResetAccum === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__RayTraceCSCB.TemporalFilter, this.variable_TemporalFilter, true);
        view.setInt32(this.constructor.StructOffsets__RayTraceCSCB.ToneMapping, this.variable_ToneMapping, true);
        device.queue.writeBuffer(this.constantBuffer__RayTraceCSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "BlueNoiseAreaLights._RayTraceCSCB"

    encoder.pushDebugGroup("BlueNoiseAreaLights.RayTrace");

    // Run compute shader RayTrace
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group RayTrace",
            layout: this.BindGroupLayout_Compute_RayTrace,
            entries: [
                {
                    // Output
                    binding: 0,
                    resource: this.texture_RenderTarget.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // Accum
                    binding: 1,
                    resource: this.texture_Accum.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 2,
                    resource: this.texture_RayTrace_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // AccumReadOnly
                    binding: 3,
                    resource: this.texture_RayTrace_Accum_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 4,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_1
                    binding: 5,
                    resource: this.texture__loadedTexture_1.createView({ dimension: "2d-array", format: this.texture__loadedTexture_1_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_2
                    binding: 6,
                    resource: this.texture__loadedTexture_2.createView({ dimension: "2d-array", format: this.texture__loadedTexture_2_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _RayTraceCSCB
                    binding: 7,
                    resource: { buffer: this.constantBuffer__RayTraceCSCB }
                },
                {
                    // texSampler
                    binding: 8,
                    resource: device.createSampler({
                        label: "Sampler texSampler",
                        addressModeU: "clamp-to-edge",
                        addressModeV: "clamp-to-edge",
                        addressModeW: "clamp-to-edge",
                        magFilter: "linear",
                        minFilter: "linear",
                        mipmapFilter: "linear",
                    }),
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_RenderTarget_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_RayTrace !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_RayTrace);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "BlueNoiseAreaLights.RayTrace"

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

var BlueNoiseAreaLights = new class_BlueNoiseAreaLights;

export default BlueNoiseAreaLights;
