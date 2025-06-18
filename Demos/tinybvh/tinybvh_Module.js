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

class class_tinybvh
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "DoRT", node "DoRTCS"
static ShaderCode_DoRTCS_DoRT = `
@binding(4) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_DoRTCB_std140_0
{
    @align(16) Albedo_0 : vec3<f32>,
    @align(4) AmbientBrightness_0 : f32,
    @align(16) AmbientColor_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) CostDivider_0 : f32,
    @align(16) DistanceDivider_0 : f32,
    @align(4) _padding1_0 : f32,
    @align(8) _padding2_0 : f32,
    @align(4) _padding3_0 : f32,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) LightBrightness_0 : f32,
    @align(4) _padding4_0 : f32,
    @align(8) _padding5_0 : f32,
    @align(4) _padding6_0 : f32,
    @align(16) LightColor_0 : vec3<f32>,
    @align(4) _padding7_0 : f32,
    @align(16) LightDirection_0 : vec3<f32>,
    @align(4) Metalic_0 : f32,
    @align(16) RayTestAgainst_0 : i32,
    @align(4) Roughness_0 : f32,
    @align(8) SpecularLevel_0 : f32,
    @align(4) TMax_0 : f32,
    @align(16) ViewMode_0 : i32,
    @align(4) _padding8_0 : f32,
    @align(8) _padding9_0 : f32,
    @align(4) _padding10_0 : f32,
};

@binding(7) @group(0) var<uniform> _DoRTCB : Struct_DoRTCB_std140_0;
@binding(6) @group(0) var _loadedTexture_0 : texture_2d<f32>;

@binding(8) @group(0) var texSampler : sampler;

struct Struct_BVHNode_std430_0
{
    @align(16) lmin_0 : vec3<f32>,
    @align(4) left_0 : u32,
    @align(16) lmax_0 : vec3<f32>,
    @align(4) right_0 : u32,
    @align(16) rmin_0 : vec3<f32>,
    @align(4) triCount_0 : u32,
    @align(16) rmax_0 : vec3<f32>,
    @align(4) firstTri_0 : u32,
};

@binding(2) @group(0) var<storage, read> BVHNodes : array<Struct_BVHNode_std430_0>;

@binding(1) @group(0) var<storage, read> TriIndices : array<u32>;

@binding(0) @group(0) var<storage, read> Vertices : array<vec4<f32>>;

@binding(3) @group(0) var<storage, read> BVHCombinedData : array<u32>;

fn rcp_0( v_0 : f32) -> f32
{
    var _S1 : bool;
    if(v_0 > 9.999999960041972e-13f)
    {
        _S1 = true;
    }
    else
    {
        _S1 = v_0 < -9.999999960041972e-13f;
    }
    if(_S1)
    {
        return 1.0f / v_0;
    }
    else
    {
        var _S2 : f32;
        if(v_0 >= 0.0f)
        {
            _S2 = 1.00000001504746622e+30f;
        }
        else
        {
            _S2 = -1.00000001504746622e+30f;
        }
        return _S2;
    }
}

fn rcp_1( v_1 : vec3<f32>) -> vec3<f32>
{
    var ret_0 : vec3<f32>;
    ret_0[i32(0)] = rcp_0(v_1.x);
    ret_0[i32(1)] = rcp_0(v_1.y);
    ret_0[i32(2)] = rcp_0(v_1.z);
    return ret_0;
}

fn SampleSphericalMap_0( v_2 : vec3<f32>) -> vec2<f32>
{
    return vec2<f32>(atan2(v_2.z, v_2.x), asin(- v_2.y)) * vec2<f32>(0.1590999960899353f, 0.31830000877380371f) + vec2<f32>(0.5f);
}

fn fresnelSchlick_0( cosTheta_0 : f32,  F0_0 : vec3<f32>) -> vec3<f32>
{
    return F0_0 + (vec3<f32>(1.0f) - F0_0) * vec3<f32>(pow(1.0f - cosTheta_0, 5.0f));
}

fn D_GGX_0( NoH_0 : f32,  alpha_0 : f32) -> f32
{
    var alpha2_0 : f32 = alpha_0 * alpha_0;
    var d_0 : f32 = NoH_0 * NoH_0 * (alpha2_0 - 1.0f) + 1.0f;
    return alpha2_0 * 0.31830987334251404f / (d_0 * d_0);
}

fn G1_GGX_Schlick_0( NoV_0 : f32,  alpha_1 : f32) -> f32
{
    var k_0 : f32 = alpha_1 / 2.0f;
    return max(NoV_0, 0.00100000004749745f) / (NoV_0 * (1.0f - k_0) + k_0);
}

fn G_Smith_0( NoV_1 : f32,  NoL_0 : f32,  alpha_2 : f32) -> f32
{
    return G1_GGX_Schlick_0(NoL_0, alpha_2) * G1_GGX_Schlick_0(NoV_1, alpha_2);
}

fn microfacetBRDF_0( L_0 : vec3<f32>,  V_0 : vec3<f32>,  N_0 : vec3<f32>,  metallic_0 : f32,  roughness_0 : f32,  baseColor_0 : vec3<f32>,  specularlevel_0 : f32) -> vec3<f32>
{
    var H_0 : vec3<f32> = normalize(V_0 + L_0);
    var NoV_2 : f32 = clamp(dot(N_0, V_0), 0.0f, 1.0f);
    var NoL_1 : f32 = clamp(dot(N_0, L_0), 0.0f, 1.0f);
    var alpha_3 : f32 = roughness_0 * roughness_0;
    var F_0 : vec3<f32> = fresnelSchlick_0(clamp(dot(V_0, H_0), 0.0f, 1.0f), mix(vec3<f32>((0.15999999642372131f * (specularlevel_0 * specularlevel_0))), baseColor_0, vec3<f32>(metallic_0)));
    return baseColor_0 * (vec3<f32>(1.0f) - F_0) * vec3<f32>((1.0f - metallic_0)) * vec3<f32>(0.31830987334251404f) + F_0 * vec3<f32>(D_GGX_0(clamp(dot(N_0, H_0), 0.0f, 1.0f), alpha_3)) * vec3<f32>(G_Smith_0(NoV_2, NoL_1, alpha_3)) / vec3<f32>((4.0f * max(NoV_2, 0.00100000004749745f) * max(NoL_1, 0.00100000004749745f)));
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S3 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S3 = sRGBLo_0.x;
    }
    else
    {
        _S3 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S3;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S3 = sRGBLo_0.y;
    }
    else
    {
        _S3 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S3;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S3 = sRGBLo_0.z;
    }
    else
    {
        _S3 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S3;
    return sRGB_0;
}

struct RayVsBVHResult_0
{
     distance_0 : f32,
     barycentrics_0 : vec2<f32>,
     triangleIndex_0 : u32,
};

fn RayVsMesh_BVH_0( _S4 : vec3<f32>,  _S5 : vec3<f32>,  _S6 : vec3<f32>,  _S7 : f32) -> RayVsBVHResult_0
{
    var dist1_0 : f32;
    var hit_0 : vec4<f32>;
    hit_0[i32(0)] = _S7;
    var stack_0 : array<u32, i32(32)>;
    var node_0 : u32 = u32(0);
    var stackPtr_0 : u32 = u32(0);
    for(;;)
    {
        var _S8 : vec3<f32> = BVHNodes[node_0].lmin_0;
        var _S9 : vec3<f32> = BVHNodes[node_0].lmax_0;
        var _S10 : vec3<f32> = BVHNodes[node_0].rmin_0;
        var _S11 : vec3<f32> = BVHNodes[node_0].rmax_0;
        var _S12 : u32 = BVHNodes[node_0].triCount_0;
        var i_0 : u32;
        if((BVHNodes[node_0].triCount_0) > u32(0))
        {
            var _S13 : u32 = BVHNodes[node_0].firstTri_0;
            i_0 = u32(0);
            for(;;)
            {
                if(i_0 < _S12)
                {
                }
                else
                {
                    break;
                }
                var triIdx_0 : u32 = TriIndices[_S13 + i_0];
                var _S14 : u32 = u32(3) * triIdx_0;
                var _S15 : vec4<f32> = Vertices[_S14];
                var _S16 : vec3<f32> = (Vertices[_S14 + u32(2)] - _S15).xyz;
                var h_0 : vec3<f32> = cross(_S5, _S16);
                var _S17 : vec3<f32> = (Vertices[_S14 + u32(1)] - _S15).xyz;
                var a_0 : f32 = dot(_S17, h_0);
                if((abs(a_0)) < 1.00000001168609742e-07f)
                {
                    i_0 = i_0 + u32(1);
                    continue;
                }
                var f_0 : f32 = 1.0f / a_0;
                var s_0 : vec3<f32> = _S4 - _S15.xyz;
                var u_0 : f32 = f_0 * dot(s_0, h_0);
                var q_0 : vec3<f32> = cross(s_0, _S17);
                var v_3 : f32 = f_0 * dot(_S5, q_0);
                var _S18 : bool;
                if(u_0 < 0.0f)
                {
                    _S18 = true;
                }
                else
                {
                    _S18 = v_3 < 0.0f;
                }
                var _S19 : bool;
                if(_S18)
                {
                    _S19 = true;
                }
                else
                {
                    _S19 = (u_0 + v_3) > 1.0f;
                }
                if(_S19)
                {
                    i_0 = i_0 + u32(1);
                    continue;
                }
                var d_1 : f32 = f_0 * dot(_S16, q_0);
                var _S20 : bool;
                if(d_1 > 0.0f)
                {
                    _S20 = d_1 < (hit_0.x);
                }
                else
                {
                    _S20 = false;
                }
                if(_S20)
                {
                    hit_0 = vec4<f32>(d_1, u_0, v_3, (bitcast<f32>((triIdx_0))));
                }
                i_0 = i_0 + u32(1);
            }
            if(stackPtr_0 == u32(0))
            {
                break;
            }
            var stackPtr_1 : u32 = stackPtr_0 - u32(1);
            node_0 = stack_0[stackPtr_1];
            stackPtr_0 = stackPtr_1;
            continue;
        }
        var _S21 : u32 = BVHNodes[node_0].left_0;
        var _S22 : u32 = BVHNodes[node_0].right_0;
        var t1a_0 : vec3<f32> = (_S8.xyz - _S4) * _S6;
        var t2a_0 : vec3<f32> = (_S9.xyz - _S4) * _S6;
        var t1b_0 : vec3<f32> = (_S10.xyz - _S4) * _S6;
        var t2b_0 : vec3<f32> = (_S11.xyz - _S4) * _S6;
        var minta_0 : vec3<f32> = min(t1a_0, t2a_0);
        var maxta_0 : vec3<f32> = max(t1a_0, t2a_0);
        var mintb_0 : vec3<f32> = min(t1b_0, t2b_0);
        var maxtb_0 : vec3<f32> = max(t1b_0, t2b_0);
        var tmina_0 : f32 = max(max(max(minta_0.x, minta_0.y), minta_0.z), 0.0f);
        var tminb_0 : f32 = max(max(max(mintb_0.x, mintb_0.y), mintb_0.z), 0.0f);
        var tmaxb_0 : f32 = min(min(min(maxtb_0.x, maxtb_0.y), maxtb_0.z), hit_0.x);
        if(tmina_0 > (min(min(min(maxta_0.x, maxta_0.y), maxta_0.z), hit_0.x)))
        {
            dist1_0 = 1.00000001504746622e+30f;
        }
        else
        {
            dist1_0 = tmina_0;
        }
        var dist2_0 : f32;
        if(tminb_0 > tmaxb_0)
        {
            dist2_0 = 1.00000001504746622e+30f;
        }
        else
        {
            dist2_0 = tminb_0;
        }
        var right_1 : u32;
        var dist1_1 : f32;
        var dist2_1 : f32;
        if(dist1_0 > dist2_0)
        {
            dist1_1 = dist2_0;
            i_0 = _S22;
            dist2_1 = dist1_0;
            right_1 = _S21;
        }
        else
        {
            dist1_1 = dist1_0;
            i_0 = _S21;
            dist2_1 = dist2_0;
            right_1 = _S22;
        }
        if(dist1_1 == 1.00000001504746622e+30f)
        {
            var _S23 : u32 = u32(0);
            var _S24 : u32 = u32(0);
            if(stackPtr_0 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_2 : u32 = stackPtr_0 - u32(1);
                _S24 = stackPtr_2;
                _S23 = stack_0[stackPtr_2];
            }
            node_0 = _S23;
            stackPtr_0 = _S24;
        }
        else
        {
            var stackPtr_3 : u32;
            if(dist2_1 != 1.00000001504746622e+30f)
            {
                var _S25 : u32 = stackPtr_0 + u32(1);
                stack_0[stackPtr_0] = right_1;
                stackPtr_3 = _S25;
            }
            else
            {
                stackPtr_3 = stackPtr_0;
            }
            node_0 = i_0;
            stackPtr_0 = stackPtr_3;
        }
    }
    var result_0 : RayVsBVHResult_0;
    if((hit_0.x) < _S7)
    {
        dist1_0 = hit_0.x;
    }
    else
    {
        dist1_0 = -1.0f;
    }
    result_0.distance_0 = dist1_0;
    result_0.barycentrics_0 = hit_0.yz;
    result_0.triangleIndex_0 = (bitcast<u32>((hit_0.w)));
    return result_0;
}

fn RayVsMesh_BVH_1( _S26 : vec3<f32>,  _S27 : vec3<f32>,  _S28 : vec3<f32>,  _S29 : f32) -> RayVsBVHResult_0
{
    var dist1_2 : f32;
    var altNodeOffset_0 : u32 = u32(2) + BVHCombinedData[i32(0)] / u32(4);
    var _S30 : u32 = altNodeOffset_0 + BVHCombinedData[i32(1)] / u32(4);
    var hit_1 : vec4<f32>;
    hit_1[i32(0)] = _S29;
    var stack_1 : array<u32, i32(32)>;
    var node_1 : u32 = u32(0);
    var stackPtr_4 : u32 = u32(0);
    for(;;)
    {
        var _S31 : u32 = altNodeOffset_0 + node_1 * u32(16);
        var _S32 : f32 = (bitcast<f32>((BVHCombinedData[_S31])));
        var _S33 : f32 = (bitcast<f32>((BVHCombinedData[_S31 + u32(1)])));
        var _S34 : f32 = (bitcast<f32>((BVHCombinedData[_S31 + u32(2)])));
        var _S35 : u32 = _S31 + u32(4);
        var _S36 : f32 = (bitcast<f32>((BVHCombinedData[_S35])));
        var _S37 : f32 = (bitcast<f32>((BVHCombinedData[_S35 + u32(1)])));
        var _S38 : f32 = (bitcast<f32>((BVHCombinedData[_S35 + u32(2)])));
        var _S39 : u32 = _S31 + u32(8);
        var _S40 : f32 = (bitcast<f32>((BVHCombinedData[_S39])));
        var _S41 : f32 = (bitcast<f32>((BVHCombinedData[_S39 + u32(1)])));
        var _S42 : f32 = (bitcast<f32>((BVHCombinedData[_S39 + u32(2)])));
        var _S43 : u32 = _S31 + u32(12);
        var _S44 : f32 = (bitcast<f32>((BVHCombinedData[_S43])));
        var _S45 : f32 = (bitcast<f32>((BVHCombinedData[_S43 + u32(1)])));
        var _S46 : f32 = (bitcast<f32>((BVHCombinedData[_S43 + u32(2)])));
        var triCount_1 : u32 = BVHCombinedData[_S31 + u32(11)];
        var i_1 : u32;
        if(triCount_1 > u32(0))
        {
            var _S47 : u32 = BVHCombinedData[_S31 + u32(15)];
            i_1 = u32(0);
            for(;;)
            {
                if(i_1 < triCount_1)
                {
                }
                else
                {
                    break;
                }
                var triIdx_1 : u32 = BVHCombinedData[_S30 + _S47 + i_1];
                var _S48 : u32 = u32(3) * triIdx_1;
                var _S49 : u32 = u32(2) + _S48 * u32(4);
                var _S50 : f32 = (bitcast<f32>((BVHCombinedData[_S49])));
                var _S51 : f32 = (bitcast<f32>((BVHCombinedData[_S49 + u32(1)])));
                var _S52 : f32 = (bitcast<f32>((BVHCombinedData[_S49 + u32(2)])));
                var _S53 : vec4<f32> = vec4<f32>(_S50, _S51, _S52, (bitcast<f32>((BVHCombinedData[_S49 + u32(3)]))));
                var _S54 : u32 = u32(2) + (_S48 + u32(1)) * u32(4);
                var _S55 : u32 = u32(2) + (_S48 + u32(2)) * u32(4);
                var _S56 : vec3<f32> = (vec4<f32>((bitcast<f32>((BVHCombinedData[_S55]))), (bitcast<f32>((BVHCombinedData[_S55 + u32(1)]))), (bitcast<f32>((BVHCombinedData[_S55 + u32(2)]))), (bitcast<f32>((BVHCombinedData[_S55 + u32(3)])))) - _S53).xyz;
                var h_1 : vec3<f32> = cross(_S27, _S56);
                var _S57 : vec3<f32> = (vec4<f32>((bitcast<f32>((BVHCombinedData[_S54]))), (bitcast<f32>((BVHCombinedData[_S54 + u32(1)]))), (bitcast<f32>((BVHCombinedData[_S54 + u32(2)]))), (bitcast<f32>((BVHCombinedData[_S54 + u32(3)])))) - _S53).xyz;
                var a_1 : f32 = dot(_S57, h_1);
                if((abs(a_1)) < 1.00000001168609742e-07f)
                {
                    i_1 = i_1 + u32(1);
                    continue;
                }
                var f_1 : f32 = 1.0f / a_1;
                var s_1 : vec3<f32> = _S26 - vec3<f32>(_S50, _S51, _S52);
                var u_1 : f32 = f_1 * dot(s_1, h_1);
                var q_1 : vec3<f32> = cross(s_1, _S57);
                var v_4 : f32 = f_1 * dot(_S27, q_1);
                var _S58 : bool;
                if(u_1 < 0.0f)
                {
                    _S58 = true;
                }
                else
                {
                    _S58 = v_4 < 0.0f;
                }
                var _S59 : bool;
                if(_S58)
                {
                    _S59 = true;
                }
                else
                {
                    _S59 = (u_1 + v_4) > 1.0f;
                }
                if(_S59)
                {
                    i_1 = i_1 + u32(1);
                    continue;
                }
                var d_2 : f32 = f_1 * dot(_S56, q_1);
                var _S60 : bool;
                if(d_2 > 0.0f)
                {
                    _S60 = d_2 < (hit_1.x);
                }
                else
                {
                    _S60 = false;
                }
                if(_S60)
                {
                    hit_1 = vec4<f32>(d_2, u_1, v_4, (bitcast<f32>((triIdx_1))));
                }
                i_1 = i_1 + u32(1);
            }
            if(stackPtr_4 == u32(0))
            {
                break;
            }
            var stackPtr_5 : u32 = stackPtr_4 - u32(1);
            node_1 = stack_1[stackPtr_5];
            stackPtr_4 = stackPtr_5;
            continue;
        }
        var left_1 : u32 = BVHCombinedData[_S31 + u32(3)];
        var right_2 : u32 = BVHCombinedData[_S31 + u32(7)];
        var t1a_1 : vec3<f32> = (vec3<f32>(_S32, _S33, _S34) - _S26) * _S28;
        var t2a_1 : vec3<f32> = (vec3<f32>(_S36, _S37, _S38) - _S26) * _S28;
        var t1b_1 : vec3<f32> = (vec3<f32>(_S40, _S41, _S42) - _S26) * _S28;
        var t2b_1 : vec3<f32> = (vec3<f32>(_S44, _S45, _S46) - _S26) * _S28;
        var minta_1 : vec3<f32> = min(t1a_1, t2a_1);
        var maxta_1 : vec3<f32> = max(t1a_1, t2a_1);
        var mintb_1 : vec3<f32> = min(t1b_1, t2b_1);
        var maxtb_1 : vec3<f32> = max(t1b_1, t2b_1);
        var tmina_1 : f32 = max(max(max(minta_1.x, minta_1.y), minta_1.z), 0.0f);
        var tminb_1 : f32 = max(max(max(mintb_1.x, mintb_1.y), mintb_1.z), 0.0f);
        var tmaxb_1 : f32 = min(min(min(maxtb_1.x, maxtb_1.y), maxtb_1.z), hit_1.x);
        if(tmina_1 > (min(min(min(maxta_1.x, maxta_1.y), maxta_1.z), hit_1.x)))
        {
            dist1_2 = 1.00000001504746622e+30f;
        }
        else
        {
            dist1_2 = tmina_1;
        }
        var dist2_2 : f32;
        if(tminb_1 > tmaxb_1)
        {
            dist2_2 = 1.00000001504746622e+30f;
        }
        else
        {
            dist2_2 = tminb_1;
        }
        var right_3 : u32;
        var dist1_3 : f32;
        var dist2_3 : f32;
        if(dist1_2 > dist2_2)
        {
            dist1_3 = dist2_2;
            i_1 = right_2;
            dist2_3 = dist1_2;
            right_3 = left_1;
        }
        else
        {
            dist1_3 = dist1_2;
            i_1 = left_1;
            dist2_3 = dist2_2;
            right_3 = right_2;
        }
        if(dist1_3 == 1.00000001504746622e+30f)
        {
            var _S61 : u32 = u32(0);
            var _S62 : u32 = u32(0);
            if(stackPtr_4 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_6 : u32 = stackPtr_4 - u32(1);
                _S62 = stackPtr_6;
                _S61 = stack_1[stackPtr_6];
            }
            node_1 = _S61;
            stackPtr_4 = _S62;
        }
        else
        {
            var stackPtr_7 : u32;
            if(dist2_3 != 1.00000001504746622e+30f)
            {
                var _S63 : u32 = stackPtr_4 + u32(1);
                stack_1[stackPtr_4] = right_3;
                stackPtr_7 = _S63;
            }
            else
            {
                stackPtr_7 = stackPtr_4;
            }
            node_1 = i_1;
            stackPtr_4 = stackPtr_7;
        }
    }
    var result_1 : RayVsBVHResult_0;
    if((hit_1.x) < _S29)
    {
        dist1_2 = hit_1.x;
    }
    else
    {
        dist1_2 = -1.0f;
    }
    result_1.distance_0 = dist1_2;
    result_1.barycentrics_0 = hit_1.yz;
    result_1.triangleIndex_0 = (bitcast<u32>((hit_1.w)));
    return result_1;
}

fn RayVsMesh_BruteForce_0( _S64 : vec3<f32>,  _S65 : vec3<f32>,  _S66 : f32) -> RayVsBVHResult_0
{
    var result_2 : RayVsBVHResult_0;
    result_2.distance_0 = -1.0f;
    result_2.barycentrics_0 = vec2<f32>(0.0f, 0.0f);
    result_2.triangleIndex_0 = u32(0);
    var _S67 : vec2<u32> = vec2<u32>(arrayLength(&Vertices), 16);
    var _S68 : u32 = _S67.x;
    var _S69 : f32 = _S66;
    var vertIndex_0 : u32 = u32(0);
    for(;;)
    {
        var _S70 : u32 = vertIndex_0 + u32(3);
        if(_S70 < _S68)
        {
        }
        else
        {
            break;
        }
        var _S71 : vec4<f32> = Vertices[vertIndex_0];
        var _S72 : vec3<f32> = (Vertices[vertIndex_0 + u32(2)] - _S71).xyz;
        var h_2 : vec3<f32> = cross(_S65, _S72);
        var _S73 : vec3<f32> = (Vertices[vertIndex_0 + u32(1)] - _S71).xyz;
        var a_2 : f32 = dot(_S73, h_2);
        if((abs(a_2)) < 1.00000001168609742e-07f)
        {
            vertIndex_0 = _S70;
            continue;
        }
        var f_2 : f32 = 1.0f / a_2;
        var s_2 : vec3<f32> = _S64 - _S71.xyz;
        var u_2 : f32 = f_2 * dot(s_2, h_2);
        var q_2 : vec3<f32> = cross(s_2, _S73);
        var v_5 : f32 = f_2 * dot(_S65, q_2);
        var _S74 : bool;
        if(u_2 < 0.0f)
        {
            _S74 = true;
        }
        else
        {
            _S74 = v_5 < 0.0f;
        }
        var _S75 : bool;
        if(_S74)
        {
            _S75 = true;
        }
        else
        {
            _S75 = (u_2 + v_5) > 1.0f;
        }
        if(_S75)
        {
            vertIndex_0 = _S70;
            continue;
        }
        var d_3 : f32 = f_2 * dot(_S72, q_2);
        var _S76 : bool;
        if(d_3 > 0.0f)
        {
            _S76 = d_3 < _S69;
        }
        else
        {
            _S76 = false;
        }
        var _S77 : f32;
        if(_S76)
        {
            result_2.distance_0 = d_3;
            result_2.barycentrics_0 = vec2<f32>(u_2, v_5);
            result_2.triangleIndex_0 = vertIndex_0 / u32(3);
            _S77 = d_3;
        }
        else
        {
            _S77 = _S69;
        }
        _S69 = _S77;
        vertIndex_0 = _S70;
    }
    return result_2;
}

fn IsOccluded_BVH_0( _S78 : vec3<f32>,  _S79 : vec3<f32>,  _S80 : vec3<f32>,  _S81 : f32) -> bool
{
    var stack_2 : array<u32, i32(32)>;
    var node_2 : u32 = u32(0);
    var stackPtr_8 : u32 = u32(0);
    for(;;)
    {
        var _S82 : vec3<f32> = BVHNodes[node_2].lmin_0;
        var _S83 : vec3<f32> = BVHNodes[node_2].lmax_0;
        var _S84 : vec3<f32> = BVHNodes[node_2].rmin_0;
        var _S85 : vec3<f32> = BVHNodes[node_2].rmax_0;
        var _S86 : u32 = BVHNodes[node_2].triCount_0;
        var i_2 : u32;
        if((BVHNodes[node_2].triCount_0) > u32(0))
        {
            var _S87 : u32 = BVHNodes[node_2].firstTri_0;
            i_2 = u32(0);
            for(;;)
            {
                if(i_2 < _S86)
                {
                }
                else
                {
                    break;
                }
                var _S88 : u32 = u32(3) * TriIndices[_S87 + i_2];
                var _S89 : vec4<f32> = Vertices[_S88];
                var _S90 : vec3<f32> = (Vertices[_S88 + u32(2)] - _S89).xyz;
                var h_3 : vec3<f32> = cross(_S79, _S90);
                var _S91 : vec3<f32> = (Vertices[_S88 + u32(1)] - _S89).xyz;
                var a_3 : f32 = dot(_S91, h_3);
                if((abs(a_3)) < 1.00000001168609742e-07f)
                {
                    i_2 = i_2 + u32(1);
                    continue;
                }
                var f_3 : f32 = 1.0f / a_3;
                var s_3 : vec3<f32> = _S78 - _S89.xyz;
                var u_3 : f32 = f_3 * dot(s_3, h_3);
                var q_3 : vec3<f32> = cross(s_3, _S91);
                var v_6 : f32 = f_3 * dot(_S79, q_3);
                var _S92 : bool;
                if(u_3 < 0.0f)
                {
                    _S92 = true;
                }
                else
                {
                    _S92 = v_6 < 0.0f;
                }
                var _S93 : bool;
                if(_S92)
                {
                    _S93 = true;
                }
                else
                {
                    _S93 = (u_3 + v_6) > 1.0f;
                }
                if(_S93)
                {
                    i_2 = i_2 + u32(1);
                    continue;
                }
                var d_4 : f32 = f_3 * dot(_S90, q_3);
                var _S94 : bool;
                if(d_4 > 0.0f)
                {
                    _S94 = d_4 < _S81;
                }
                else
                {
                    _S94 = false;
                }
                if(_S94)
                {
                    return true;
                }
                i_2 = i_2 + u32(1);
            }
            if(stackPtr_8 == u32(0))
            {
                break;
            }
            var stackPtr_9 : u32 = stackPtr_8 - u32(1);
            node_2 = stack_2[stackPtr_9];
            stackPtr_8 = stackPtr_9;
            continue;
        }
        var _S95 : u32 = BVHNodes[node_2].left_0;
        var _S96 : u32 = BVHNodes[node_2].right_0;
        var t1a_2 : vec3<f32> = (_S82.xyz - _S78) * _S80;
        var t2a_2 : vec3<f32> = (_S83.xyz - _S78) * _S80;
        var t1b_2 : vec3<f32> = (_S84.xyz - _S78) * _S80;
        var t2b_2 : vec3<f32> = (_S85.xyz - _S78) * _S80;
        var minta_2 : vec3<f32> = min(t1a_2, t2a_2);
        var maxta_2 : vec3<f32> = max(t1a_2, t2a_2);
        var mintb_2 : vec3<f32> = min(t1b_2, t2b_2);
        var maxtb_2 : vec3<f32> = max(t1b_2, t2b_2);
        var tmina_2 : f32 = max(max(max(minta_2.x, minta_2.y), minta_2.z), 0.0f);
        var tminb_2 : f32 = max(max(max(mintb_2.x, mintb_2.y), mintb_2.z), 0.0f);
        var tmaxb_2 : f32 = min(min(min(maxtb_2.x, maxtb_2.y), maxtb_2.z), _S81);
        var dist1_4 : f32;
        if(tmina_2 > (min(min(min(maxta_2.x, maxta_2.y), maxta_2.z), _S81)))
        {
            dist1_4 = 1.00000001504746622e+30f;
        }
        else
        {
            dist1_4 = tmina_2;
        }
        var dist2_4 : f32;
        if(tminb_2 > tmaxb_2)
        {
            dist2_4 = 1.00000001504746622e+30f;
        }
        else
        {
            dist2_4 = tminb_2;
        }
        var right_4 : u32;
        var dist1_5 : f32;
        var dist2_5 : f32;
        if(dist1_4 > dist2_4)
        {
            dist1_5 = dist2_4;
            i_2 = _S96;
            dist2_5 = dist1_4;
            right_4 = _S95;
        }
        else
        {
            dist1_5 = dist1_4;
            i_2 = _S95;
            dist2_5 = dist2_4;
            right_4 = _S96;
        }
        if(dist1_5 == 1.00000001504746622e+30f)
        {
            var _S97 : u32 = u32(0);
            var _S98 : u32 = u32(0);
            if(stackPtr_8 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_10 : u32 = stackPtr_8 - u32(1);
                _S98 = stackPtr_10;
                _S97 = stack_2[stackPtr_10];
            }
            node_2 = _S97;
            stackPtr_8 = _S98;
        }
        else
        {
            var stackPtr_11 : u32;
            if(dist2_5 != 1.00000001504746622e+30f)
            {
                var _S99 : u32 = stackPtr_8 + u32(1);
                stack_2[stackPtr_8] = right_4;
                stackPtr_11 = _S99;
            }
            else
            {
                stackPtr_11 = stackPtr_8;
            }
            node_2 = i_2;
            stackPtr_8 = stackPtr_11;
        }
    }
    return false;
}

fn IsOccluded_BVH_1( _S100 : vec3<f32>,  _S101 : vec3<f32>,  _S102 : vec3<f32>,  _S103 : f32) -> bool
{
    var altNodeOffset_1 : u32 = u32(2) + BVHCombinedData[i32(0)] / u32(4);
    var _S104 : u32 = altNodeOffset_1 + BVHCombinedData[i32(1)] / u32(4);
    var stack_3 : array<u32, i32(32)>;
    var node_3 : u32 = u32(0);
    var stackPtr_12 : u32 = u32(0);
    for(;;)
    {
        var _S105 : u32 = altNodeOffset_1 + node_3 * u32(16);
        var _S106 : f32 = (bitcast<f32>((BVHCombinedData[_S105])));
        var _S107 : f32 = (bitcast<f32>((BVHCombinedData[_S105 + u32(1)])));
        var _S108 : f32 = (bitcast<f32>((BVHCombinedData[_S105 + u32(2)])));
        var _S109 : u32 = _S105 + u32(4);
        var _S110 : f32 = (bitcast<f32>((BVHCombinedData[_S109])));
        var _S111 : f32 = (bitcast<f32>((BVHCombinedData[_S109 + u32(1)])));
        var _S112 : f32 = (bitcast<f32>((BVHCombinedData[_S109 + u32(2)])));
        var _S113 : u32 = _S105 + u32(8);
        var _S114 : f32 = (bitcast<f32>((BVHCombinedData[_S113])));
        var _S115 : f32 = (bitcast<f32>((BVHCombinedData[_S113 + u32(1)])));
        var _S116 : f32 = (bitcast<f32>((BVHCombinedData[_S113 + u32(2)])));
        var _S117 : u32 = _S105 + u32(12);
        var _S118 : f32 = (bitcast<f32>((BVHCombinedData[_S117])));
        var _S119 : f32 = (bitcast<f32>((BVHCombinedData[_S117 + u32(1)])));
        var _S120 : f32 = (bitcast<f32>((BVHCombinedData[_S117 + u32(2)])));
        var triCount_2 : u32 = BVHCombinedData[_S105 + u32(11)];
        var i_3 : u32;
        if(triCount_2 > u32(0))
        {
            var _S121 : u32 = BVHCombinedData[_S105 + u32(15)];
            i_3 = u32(0);
            for(;;)
            {
                if(i_3 < triCount_2)
                {
                }
                else
                {
                    break;
                }
                var _S122 : u32 = u32(3) * BVHCombinedData[_S104 + _S121 + i_3];
                var _S123 : u32 = u32(2) + _S122 * u32(4);
                var _S124 : f32 = (bitcast<f32>((BVHCombinedData[_S123])));
                var _S125 : f32 = (bitcast<f32>((BVHCombinedData[_S123 + u32(1)])));
                var _S126 : f32 = (bitcast<f32>((BVHCombinedData[_S123 + u32(2)])));
                var _S127 : vec4<f32> = vec4<f32>(_S124, _S125, _S126, (bitcast<f32>((BVHCombinedData[_S123 + u32(3)]))));
                var _S128 : u32 = u32(2) + (_S122 + u32(1)) * u32(4);
                var _S129 : u32 = u32(2) + (_S122 + u32(2)) * u32(4);
                var _S130 : vec3<f32> = (vec4<f32>((bitcast<f32>((BVHCombinedData[_S129]))), (bitcast<f32>((BVHCombinedData[_S129 + u32(1)]))), (bitcast<f32>((BVHCombinedData[_S129 + u32(2)]))), (bitcast<f32>((BVHCombinedData[_S129 + u32(3)])))) - _S127).xyz;
                var h_4 : vec3<f32> = cross(_S101, _S130);
                var _S131 : vec3<f32> = (vec4<f32>((bitcast<f32>((BVHCombinedData[_S128]))), (bitcast<f32>((BVHCombinedData[_S128 + u32(1)]))), (bitcast<f32>((BVHCombinedData[_S128 + u32(2)]))), (bitcast<f32>((BVHCombinedData[_S128 + u32(3)])))) - _S127).xyz;
                var a_4 : f32 = dot(_S131, h_4);
                if((abs(a_4)) < 1.00000001168609742e-07f)
                {
                    i_3 = i_3 + u32(1);
                    continue;
                }
                var f_4 : f32 = 1.0f / a_4;
                var s_4 : vec3<f32> = _S100 - vec3<f32>(_S124, _S125, _S126);
                var u_4 : f32 = f_4 * dot(s_4, h_4);
                var q_4 : vec3<f32> = cross(s_4, _S131);
                var v_7 : f32 = f_4 * dot(_S101, q_4);
                var _S132 : bool;
                if(u_4 < 0.0f)
                {
                    _S132 = true;
                }
                else
                {
                    _S132 = v_7 < 0.0f;
                }
                var _S133 : bool;
                if(_S132)
                {
                    _S133 = true;
                }
                else
                {
                    _S133 = (u_4 + v_7) > 1.0f;
                }
                if(_S133)
                {
                    i_3 = i_3 + u32(1);
                    continue;
                }
                var d_5 : f32 = f_4 * dot(_S130, q_4);
                var _S134 : bool;
                if(d_5 > 0.0f)
                {
                    _S134 = d_5 < _S103;
                }
                else
                {
                    _S134 = false;
                }
                if(_S134)
                {
                    return true;
                }
                i_3 = i_3 + u32(1);
            }
            if(stackPtr_12 == u32(0))
            {
                break;
            }
            var stackPtr_13 : u32 = stackPtr_12 - u32(1);
            node_3 = stack_3[stackPtr_13];
            stackPtr_12 = stackPtr_13;
            continue;
        }
        var left_2 : u32 = BVHCombinedData[_S105 + u32(3)];
        var right_5 : u32 = BVHCombinedData[_S105 + u32(7)];
        var t1a_3 : vec3<f32> = (vec3<f32>(_S106, _S107, _S108) - _S100) * _S102;
        var t2a_3 : vec3<f32> = (vec3<f32>(_S110, _S111, _S112) - _S100) * _S102;
        var t1b_3 : vec3<f32> = (vec3<f32>(_S114, _S115, _S116) - _S100) * _S102;
        var t2b_3 : vec3<f32> = (vec3<f32>(_S118, _S119, _S120) - _S100) * _S102;
        var minta_3 : vec3<f32> = min(t1a_3, t2a_3);
        var maxta_3 : vec3<f32> = max(t1a_3, t2a_3);
        var mintb_3 : vec3<f32> = min(t1b_3, t2b_3);
        var maxtb_3 : vec3<f32> = max(t1b_3, t2b_3);
        var tmina_3 : f32 = max(max(max(minta_3.x, minta_3.y), minta_3.z), 0.0f);
        var tminb_3 : f32 = max(max(max(mintb_3.x, mintb_3.y), mintb_3.z), 0.0f);
        var tmaxb_3 : f32 = min(min(min(maxtb_3.x, maxtb_3.y), maxtb_3.z), _S103);
        var dist1_6 : f32;
        if(tmina_3 > (min(min(min(maxta_3.x, maxta_3.y), maxta_3.z), _S103)))
        {
            dist1_6 = 1.00000001504746622e+30f;
        }
        else
        {
            dist1_6 = tmina_3;
        }
        var dist2_6 : f32;
        if(tminb_3 > tmaxb_3)
        {
            dist2_6 = 1.00000001504746622e+30f;
        }
        else
        {
            dist2_6 = tminb_3;
        }
        var right_6 : u32;
        var dist1_7 : f32;
        var dist2_7 : f32;
        if(dist1_6 > dist2_6)
        {
            dist1_7 = dist2_6;
            i_3 = right_5;
            dist2_7 = dist1_6;
            right_6 = left_2;
        }
        else
        {
            dist1_7 = dist1_6;
            i_3 = left_2;
            dist2_7 = dist2_6;
            right_6 = right_5;
        }
        if(dist1_7 == 1.00000001504746622e+30f)
        {
            var _S135 : u32 = u32(0);
            var _S136 : u32 = u32(0);
            if(stackPtr_12 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_14 : u32 = stackPtr_12 - u32(1);
                _S136 = stackPtr_14;
                _S135 = stack_3[stackPtr_14];
            }
            node_3 = _S135;
            stackPtr_12 = _S136;
        }
        else
        {
            var stackPtr_15 : u32;
            if(dist2_7 != 1.00000001504746622e+30f)
            {
                var _S137 : u32 = stackPtr_12 + u32(1);
                stack_3[stackPtr_12] = right_6;
                stackPtr_15 = _S137;
            }
            else
            {
                stackPtr_15 = stackPtr_12;
            }
            node_3 = i_3;
            stackPtr_12 = stackPtr_15;
        }
    }
    return false;
}

fn IsOccluded_BruteForce_0( _S138 : vec3<f32>,  _S139 : vec3<f32>,  _S140 : f32) -> bool
{
    var _S141 : vec2<u32> = vec2<u32>(arrayLength(&Vertices), 16);
    var _S142 : u32 = _S141.x;
    var vertIndex_1 : u32 = u32(0);
    for(;;)
    {
        var _S143 : u32 = vertIndex_1 + u32(3);
        if(_S143 < _S142)
        {
        }
        else
        {
            break;
        }
        var _S144 : vec4<f32> = Vertices[vertIndex_1];
        var _S145 : vec3<f32> = (Vertices[vertIndex_1 + u32(2)] - _S144).xyz;
        var h_5 : vec3<f32> = cross(_S139, _S145);
        var _S146 : vec3<f32> = (Vertices[vertIndex_1 + u32(1)] - _S144).xyz;
        var a_5 : f32 = dot(_S146, h_5);
        if((abs(a_5)) < 1.00000001168609742e-07f)
        {
            vertIndex_1 = _S143;
            continue;
        }
        var f_5 : f32 = 1.0f / a_5;
        var s_5 : vec3<f32> = _S138 - _S144.xyz;
        var u_5 : f32 = f_5 * dot(s_5, h_5);
        var q_5 : vec3<f32> = cross(s_5, _S146);
        var v_8 : f32 = f_5 * dot(_S139, q_5);
        var _S147 : bool;
        if(u_5 < 0.0f)
        {
            _S147 = true;
        }
        else
        {
            _S147 = v_8 < 0.0f;
        }
        var _S148 : bool;
        if(_S147)
        {
            _S148 = true;
        }
        else
        {
            _S148 = (u_5 + v_8) > 1.0f;
        }
        if(_S148)
        {
            vertIndex_1 = _S143;
            continue;
        }
        var d_6 : f32 = f_5 * dot(_S145, q_5);
        var _S149 : bool;
        if(d_6 > 0.0f)
        {
            _S149 = d_6 < _S140;
        }
        else
        {
            _S149 = false;
        }
        if(_S149)
        {
            return true;
        }
        vertIndex_1 = _S143;
    }
    return false;
}

fn BVHCost_0( _S150 : vec3<f32>,  _S151 : vec3<f32>,  _S152 : vec3<f32>,  _S153 : f32) -> f32
{
    var hit_2 : vec4<f32>;
    hit_2[i32(0)] = _S153;
    var stack_4 : array<u32, i32(32)>;
    var node_4 : u32 = u32(0);
    var cost_0 : f32 = 0.0f;
    var stackPtr_16 : u32 = u32(0);
    for(;;)
    {
        var cost_1 : f32 = cost_0 + 1.20000004768371582f;
        var _S154 : vec3<f32> = BVHNodes[node_4].lmin_0;
        var _S155 : vec3<f32> = BVHNodes[node_4].lmax_0;
        var _S156 : vec3<f32> = BVHNodes[node_4].rmin_0;
        var _S157 : vec3<f32> = BVHNodes[node_4].rmax_0;
        var _S158 : u32 = BVHNodes[node_4].triCount_0;
        var i_4 : u32;
        var cost_2 : f32;
        if((BVHNodes[node_4].triCount_0) > u32(0))
        {
            var _S159 : u32 = BVHNodes[node_4].firstTri_0;
            i_4 = u32(0);
            cost_2 = cost_1;
            for(;;)
            {
                if(i_4 < _S158)
                {
                }
                else
                {
                    break;
                }
                var cost_3 : f32 = cost_2 + 1.0f;
                var triIdx_2 : u32 = TriIndices[_S159 + i_4];
                var _S160 : u32 = u32(3) * triIdx_2;
                var _S161 : vec4<f32> = Vertices[_S160];
                var _S162 : vec3<f32> = (Vertices[_S160 + u32(2)] - _S161).xyz;
                var h_6 : vec3<f32> = cross(_S151, _S162);
                var _S163 : vec3<f32> = (Vertices[_S160 + u32(1)] - _S161).xyz;
                var a_6 : f32 = dot(_S163, h_6);
                if((abs(a_6)) < 1.00000001168609742e-07f)
                {
                    i_4 = i_4 + u32(1);
                    cost_2 = cost_3;
                    continue;
                }
                var f_6 : f32 = 1.0f / a_6;
                var s_6 : vec3<f32> = _S150 - _S161.xyz;
                var u_6 : f32 = f_6 * dot(s_6, h_6);
                var q_6 : vec3<f32> = cross(s_6, _S163);
                var v_9 : f32 = f_6 * dot(_S151, q_6);
                var _S164 : bool;
                if(u_6 < 0.0f)
                {
                    _S164 = true;
                }
                else
                {
                    _S164 = v_9 < 0.0f;
                }
                var _S165 : bool;
                if(_S164)
                {
                    _S165 = true;
                }
                else
                {
                    _S165 = (u_6 + v_9) > 1.0f;
                }
                if(_S165)
                {
                    i_4 = i_4 + u32(1);
                    cost_2 = cost_3;
                    continue;
                }
                var d_7 : f32 = f_6 * dot(_S162, q_6);
                var _S166 : bool;
                if(d_7 > 0.0f)
                {
                    _S166 = d_7 < (hit_2.x);
                }
                else
                {
                    _S166 = false;
                }
                if(_S166)
                {
                    hit_2 = vec4<f32>(d_7, u_6, v_9, (bitcast<f32>((triIdx_2))));
                }
                i_4 = i_4 + u32(1);
                cost_2 = cost_3;
            }
            if(stackPtr_16 == u32(0))
            {
                cost_0 = cost_2;
                break;
            }
            var stackPtr_17 : u32 = stackPtr_16 - u32(1);
            node_4 = stack_4[stackPtr_17];
            cost_0 = cost_2;
            stackPtr_16 = stackPtr_17;
            continue;
        }
        var _S167 : u32 = BVHNodes[node_4].left_0;
        var _S168 : u32 = BVHNodes[node_4].right_0;
        var t1a_4 : vec3<f32> = (_S154.xyz - _S150) * _S152;
        var t2a_4 : vec3<f32> = (_S155.xyz - _S150) * _S152;
        var t1b_4 : vec3<f32> = (_S156.xyz - _S150) * _S152;
        var t2b_4 : vec3<f32> = (_S157.xyz - _S150) * _S152;
        var minta_4 : vec3<f32> = min(t1a_4, t2a_4);
        var maxta_4 : vec3<f32> = max(t1a_4, t2a_4);
        var mintb_4 : vec3<f32> = min(t1b_4, t2b_4);
        var maxtb_4 : vec3<f32> = max(t1b_4, t2b_4);
        var tmina_4 : f32 = max(max(max(minta_4.x, minta_4.y), minta_4.z), 0.0f);
        var tminb_4 : f32 = max(max(max(mintb_4.x, mintb_4.y), mintb_4.z), 0.0f);
        var tmaxb_4 : f32 = min(min(min(maxtb_4.x, maxtb_4.y), maxtb_4.z), hit_2.x);
        if(tmina_4 > (min(min(min(maxta_4.x, maxta_4.y), maxta_4.z), hit_2.x)))
        {
            cost_2 = 1.00000001504746622e+30f;
        }
        else
        {
            cost_2 = tmina_4;
        }
        var dist2_8 : f32;
        if(tminb_4 > tmaxb_4)
        {
            dist2_8 = 1.00000001504746622e+30f;
        }
        else
        {
            dist2_8 = tminb_4;
        }
        var right_7 : u32;
        var dist1_8 : f32;
        var dist2_9 : f32;
        if(cost_2 > dist2_8)
        {
            dist1_8 = dist2_8;
            i_4 = _S168;
            dist2_9 = cost_2;
            right_7 = _S167;
        }
        else
        {
            dist1_8 = cost_2;
            i_4 = _S167;
            dist2_9 = dist2_8;
            right_7 = _S168;
        }
        if(dist1_8 == 1.00000001504746622e+30f)
        {
            var _S169 : u32 = u32(0);
            var _S170 : u32 = u32(0);
            if(stackPtr_16 == u32(0))
            {
                cost_0 = cost_1;
                break;
            }
            else
            {
                var stackPtr_18 : u32 = stackPtr_16 - u32(1);
                _S170 = stackPtr_18;
                _S169 = stack_4[stackPtr_18];
            }
            node_4 = _S169;
            stackPtr_16 = _S170;
        }
        else
        {
            var stackPtr_19 : u32;
            if(dist2_9 != 1.00000001504746622e+30f)
            {
                var _S171 : u32 = stackPtr_16 + u32(1);
                stack_4[stackPtr_16] = right_7;
                stackPtr_19 = _S171;
            }
            else
            {
                stackPtr_19 = stackPtr_16;
            }
            node_4 = i_4;
            stackPtr_16 = stackPtr_19;
        }
        cost_0 = cost_1;
    }
    return cost_0;
}

fn BVHCost_1( _S172 : vec3<f32>,  _S173 : vec3<f32>,  _S174 : vec3<f32>,  _S175 : f32) -> f32
{
    var altNodeOffset_2 : u32 = u32(2) + BVHCombinedData[i32(0)] / u32(4);
    var _S176 : u32 = altNodeOffset_2 + BVHCombinedData[i32(1)] / u32(4);
    var hit_3 : vec4<f32>;
    hit_3[i32(0)] = _S175;
    var stack_5 : array<u32, i32(32)>;
    var node_5 : u32 = u32(0);
    var cost_4 : f32 = 0.0f;
    var stackPtr_20 : u32 = u32(0);
    for(;;)
    {
        var cost_5 : f32 = cost_4 + 1.20000004768371582f;
        var _S177 : u32 = altNodeOffset_2 + node_5 * u32(16);
        var _S178 : f32 = (bitcast<f32>((BVHCombinedData[_S177])));
        var _S179 : f32 = (bitcast<f32>((BVHCombinedData[_S177 + u32(1)])));
        var _S180 : f32 = (bitcast<f32>((BVHCombinedData[_S177 + u32(2)])));
        var _S181 : u32 = _S177 + u32(4);
        var _S182 : f32 = (bitcast<f32>((BVHCombinedData[_S181])));
        var _S183 : f32 = (bitcast<f32>((BVHCombinedData[_S181 + u32(1)])));
        var _S184 : f32 = (bitcast<f32>((BVHCombinedData[_S181 + u32(2)])));
        var _S185 : u32 = _S177 + u32(8);
        var _S186 : f32 = (bitcast<f32>((BVHCombinedData[_S185])));
        var _S187 : f32 = (bitcast<f32>((BVHCombinedData[_S185 + u32(1)])));
        var _S188 : f32 = (bitcast<f32>((BVHCombinedData[_S185 + u32(2)])));
        var _S189 : u32 = _S177 + u32(12);
        var _S190 : f32 = (bitcast<f32>((BVHCombinedData[_S189])));
        var _S191 : f32 = (bitcast<f32>((BVHCombinedData[_S189 + u32(1)])));
        var _S192 : f32 = (bitcast<f32>((BVHCombinedData[_S189 + u32(2)])));
        var triCount_3 : u32 = BVHCombinedData[_S177 + u32(11)];
        var i_5 : u32;
        var cost_6 : f32;
        if(triCount_3 > u32(0))
        {
            var _S193 : u32 = BVHCombinedData[_S177 + u32(15)];
            i_5 = u32(0);
            cost_6 = cost_5;
            for(;;)
            {
                if(i_5 < triCount_3)
                {
                }
                else
                {
                    break;
                }
                var cost_7 : f32 = cost_6 + 1.0f;
                var triIdx_3 : u32 = BVHCombinedData[_S176 + _S193 + i_5];
                var _S194 : u32 = u32(3) * triIdx_3;
                var _S195 : u32 = u32(2) + _S194 * u32(4);
                var _S196 : f32 = (bitcast<f32>((BVHCombinedData[_S195])));
                var _S197 : f32 = (bitcast<f32>((BVHCombinedData[_S195 + u32(1)])));
                var _S198 : f32 = (bitcast<f32>((BVHCombinedData[_S195 + u32(2)])));
                var _S199 : vec4<f32> = vec4<f32>(_S196, _S197, _S198, (bitcast<f32>((BVHCombinedData[_S195 + u32(3)]))));
                var _S200 : u32 = u32(2) + (_S194 + u32(1)) * u32(4);
                var _S201 : u32 = u32(2) + (_S194 + u32(2)) * u32(4);
                var _S202 : vec3<f32> = (vec4<f32>((bitcast<f32>((BVHCombinedData[_S201]))), (bitcast<f32>((BVHCombinedData[_S201 + u32(1)]))), (bitcast<f32>((BVHCombinedData[_S201 + u32(2)]))), (bitcast<f32>((BVHCombinedData[_S201 + u32(3)])))) - _S199).xyz;
                var h_7 : vec3<f32> = cross(_S173, _S202);
                var _S203 : vec3<f32> = (vec4<f32>((bitcast<f32>((BVHCombinedData[_S200]))), (bitcast<f32>((BVHCombinedData[_S200 + u32(1)]))), (bitcast<f32>((BVHCombinedData[_S200 + u32(2)]))), (bitcast<f32>((BVHCombinedData[_S200 + u32(3)])))) - _S199).xyz;
                var a_7 : f32 = dot(_S203, h_7);
                if((abs(a_7)) < 1.00000001168609742e-07f)
                {
                    i_5 = i_5 + u32(1);
                    cost_6 = cost_7;
                    continue;
                }
                var f_7 : f32 = 1.0f / a_7;
                var s_7 : vec3<f32> = _S172 - vec3<f32>(_S196, _S197, _S198);
                var u_7 : f32 = f_7 * dot(s_7, h_7);
                var q_7 : vec3<f32> = cross(s_7, _S203);
                var v_10 : f32 = f_7 * dot(_S173, q_7);
                var _S204 : bool;
                if(u_7 < 0.0f)
                {
                    _S204 = true;
                }
                else
                {
                    _S204 = v_10 < 0.0f;
                }
                var _S205 : bool;
                if(_S204)
                {
                    _S205 = true;
                }
                else
                {
                    _S205 = (u_7 + v_10) > 1.0f;
                }
                if(_S205)
                {
                    i_5 = i_5 + u32(1);
                    cost_6 = cost_7;
                    continue;
                }
                var d_8 : f32 = f_7 * dot(_S202, q_7);
                var _S206 : bool;
                if(d_8 > 0.0f)
                {
                    _S206 = d_8 < (hit_3.x);
                }
                else
                {
                    _S206 = false;
                }
                if(_S206)
                {
                    hit_3 = vec4<f32>(d_8, u_7, v_10, (bitcast<f32>((triIdx_3))));
                }
                i_5 = i_5 + u32(1);
                cost_6 = cost_7;
            }
            if(stackPtr_20 == u32(0))
            {
                cost_4 = cost_6;
                break;
            }
            var stackPtr_21 : u32 = stackPtr_20 - u32(1);
            node_5 = stack_5[stackPtr_21];
            cost_4 = cost_6;
            stackPtr_20 = stackPtr_21;
            continue;
        }
        var left_3 : u32 = BVHCombinedData[_S177 + u32(3)];
        var right_8 : u32 = BVHCombinedData[_S177 + u32(7)];
        var t1a_5 : vec3<f32> = (vec3<f32>(_S178, _S179, _S180) - _S172) * _S174;
        var t2a_5 : vec3<f32> = (vec3<f32>(_S182, _S183, _S184) - _S172) * _S174;
        var t1b_5 : vec3<f32> = (vec3<f32>(_S186, _S187, _S188) - _S172) * _S174;
        var t2b_5 : vec3<f32> = (vec3<f32>(_S190, _S191, _S192) - _S172) * _S174;
        var minta_5 : vec3<f32> = min(t1a_5, t2a_5);
        var maxta_5 : vec3<f32> = max(t1a_5, t2a_5);
        var mintb_5 : vec3<f32> = min(t1b_5, t2b_5);
        var maxtb_5 : vec3<f32> = max(t1b_5, t2b_5);
        var tmina_5 : f32 = max(max(max(minta_5.x, minta_5.y), minta_5.z), 0.0f);
        var tminb_5 : f32 = max(max(max(mintb_5.x, mintb_5.y), mintb_5.z), 0.0f);
        var tmaxb_5 : f32 = min(min(min(maxtb_5.x, maxtb_5.y), maxtb_5.z), hit_3.x);
        if(tmina_5 > (min(min(min(maxta_5.x, maxta_5.y), maxta_5.z), hit_3.x)))
        {
            cost_6 = 1.00000001504746622e+30f;
        }
        else
        {
            cost_6 = tmina_5;
        }
        var dist2_10 : f32;
        if(tminb_5 > tmaxb_5)
        {
            dist2_10 = 1.00000001504746622e+30f;
        }
        else
        {
            dist2_10 = tminb_5;
        }
        var right_9 : u32;
        var dist1_9 : f32;
        var dist2_11 : f32;
        if(cost_6 > dist2_10)
        {
            dist1_9 = dist2_10;
            i_5 = right_8;
            dist2_11 = cost_6;
            right_9 = left_3;
        }
        else
        {
            dist1_9 = cost_6;
            i_5 = left_3;
            dist2_11 = dist2_10;
            right_9 = right_8;
        }
        if(dist1_9 == 1.00000001504746622e+30f)
        {
            var _S207 : u32 = u32(0);
            var _S208 : u32 = u32(0);
            if(stackPtr_20 == u32(0))
            {
                cost_4 = cost_5;
                break;
            }
            else
            {
                var stackPtr_22 : u32 = stackPtr_20 - u32(1);
                _S208 = stackPtr_22;
                _S207 = stack_5[stackPtr_22];
            }
            node_5 = _S207;
            stackPtr_20 = _S208;
        }
        else
        {
            var stackPtr_23 : u32;
            if(dist2_11 != 1.00000001504746622e+30f)
            {
                var _S209 : u32 = stackPtr_20 + u32(1);
                stack_5[stackPtr_20] = right_9;
                stackPtr_23 = _S209;
            }
            else
            {
                stackPtr_23 = stackPtr_20;
            }
            node_5 = i_5;
            stackPtr_20 = stackPtr_23;
        }
        cost_4 = cost_5;
    }
    return cost_4;
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var dims_0 : vec2<u32>;
    var _S210 : u32 = dims_0[i32(0)];
    var _S211 : u32 = dims_0[i32(1)];
    {var dim = textureDimensions((Output));((_S210)) = dim.x;((_S211)) = dim.y;};
    dims_0[i32(0)] = _S210;
    dims_0[i32(1)] = _S211;
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S212 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(dims_0) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S212;
    screenPos_0[i32(1)] = - _S212.y;
    var _S213 : vec4<f32> = (((mat4x4<f32>(_DoRTCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _DoRTCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _DoRTCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _DoRTCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _DoRTCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _DoRTCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _DoRTCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _DoRTCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _DoRTCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _DoRTCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _DoRTCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _DoRTCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _DoRTCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _DoRTCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _DoRTCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _DoRTCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, 0.89999997615814209f, 1.0f))));
    var world_0 : vec4<f32> = _S213;
    var _S214 : vec3<f32> = _S213.xyz / vec3<f32>(_S213.w);
    world_0.x = _S214.x;
    world_0.y = _S214.y;
    world_0.z = _S214.z;
    var rayPos_0 : vec3<f32> = _DoRTCB.CameraPos_0;
    var rayDir_0 : vec3<f32> = normalize(world_0.xyz - _DoRTCB.CameraPos_0);
    var rayDirRecip_0 : vec3<f32> = rcp_1(rayDir_0);
    var color_0 : vec3<f32> = (textureSampleLevel((_loadedTexture_0), (texSampler), (SampleSphericalMap_0(rayDir_0)), (0.0f))).xyz;
    switch(_DoRTCB.ViewMode_0)
    {
    case i32(0), i32(3), i32(4), i32(5), :
        {
            var result_3 : RayVsBVHResult_0;
            switch(_DoRTCB.RayTestAgainst_0)
            {
            case i32(0), :
                {
                    result_3 = RayVsMesh_BVH_0(rayPos_0, rayDir_0, rayDirRecip_0, _DoRTCB.TMax_0);
                    break;
                }
            case i32(1), :
                {
                    result_3 = RayVsMesh_BVH_1(rayPos_0, rayDir_0, rayDirRecip_0, _DoRTCB.TMax_0);
                    break;
                }
            case i32(2), :
                {
                    var _S215 : RayVsBVHResult_0 = RayVsMesh_BruteForce_0(rayPos_0, rayDir_0, _DoRTCB.TMax_0);
                    result_3 = _S215;
                    break;
                }
            case default, :
                {
                    break;
                }
            }
            var _S216 : RayVsBVHResult_0 = result_3;
            if((result_3.distance_0) >= 0.0f)
            {
                var _S217 : u32 = result_3.triangleIndex_0 * u32(3);
                var A_0 : vec3<f32> = Vertices[_S217].xyz;
                var normal_0 : vec3<f32> = normalize(cross(Vertices[_S217 + u32(1)].xyz - A_0, Vertices[_S217 + u32(2)].xyz - A_0));
                switch(_DoRTCB.ViewMode_0)
                {
                case i32(3), :
                    {
                        color_0 = normal_0;
                        break;
                    }
                case i32(4), :
                    {
                        color_0 = vec3<f32>(_S216.distance_0 / _DoRTCB.DistanceDivider_0, 0.0f, 0.0f);
                        break;
                    }
                case i32(5), :
                    {
                        color_0 = vec3<f32>(result_3.barycentrics_0, 0.0f);
                        break;
                    }
                case i32(0), :
                    {
                        var lightDirection_0 : vec3<f32> = normalize(- _DoRTCB.LightDirection_0);
                        color_0 = vec3<f32>(max(dot(lightDirection_0, normal_0), 0.0f)) * (_DoRTCB.LightColor_0 * vec3<f32>(_DoRTCB.LightBrightness_0)) * microfacetBRDF_0(lightDirection_0, - rayDir_0, normal_0, _DoRTCB.Metalic_0, _DoRTCB.Roughness_0, _DoRTCB.Albedo_0, _DoRTCB.SpecularLevel_0) + _DoRTCB.AmbientColor_0 * vec3<f32>(_DoRTCB.AmbientBrightness_0);
                        break;
                    }
                case default, :
                    {
                        break;
                    }
                }
            }
            break;
        }
    case i32(1), :
        {
            var isOccluded_0 : bool;
            switch(_DoRTCB.RayTestAgainst_0)
            {
            case i32(0), :
                {
                    isOccluded_0 = IsOccluded_BVH_0(rayPos_0, rayDir_0, rayDirRecip_0, _DoRTCB.TMax_0);
                    break;
                }
            case i32(1), :
                {
                    isOccluded_0 = IsOccluded_BVH_1(rayPos_0, rayDir_0, rayDirRecip_0, _DoRTCB.TMax_0);
                    break;
                }
            case i32(2), :
                {
                    var _S218 : bool = IsOccluded_BruteForce_0(rayPos_0, rayDir_0, _DoRTCB.TMax_0);
                    isOccluded_0 = _S218;
                    break;
                }
            case default, :
                {
                    isOccluded_0 = false;
                    break;
                }
            }
            if(isOccluded_0)
            {
                color_0 = vec3<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f);
            }
            break;
        }
    case i32(2), :
        {
            color_0 = vec3<f32>(1.0f, 0.0f, 0.0f);
            switch(_DoRTCB.RayTestAgainst_0)
            {
            case i32(0), :
                {
                    color_0[i32(0)] = BVHCost_0(rayPos_0, rayDir_0, rayDirRecip_0, _DoRTCB.TMax_0) / _DoRTCB.CostDivider_0;
                    break;
                }
            case i32(1), :
                {
                    color_0[i32(0)] = BVHCost_1(rayPos_0, rayDir_0, rayDirRecip_0, _DoRTCB.TMax_0) / _DoRTCB.CostDivider_0;
                    break;
                }
            case i32(2), :
                {
                    break;
                }
            case default, :
                {
                    break;
                }
            }
            break;
        }
    case default, :
        {
            break;
        }
    }
    textureStore((Output), (px_0), (vec4<f32>(LinearToSRGB_0(color_0), 1.0f)));
    return;
}

`;

// -------------------- Enums

static Enum_ViewModes =
{
    Shaded: 0,
    Occluded: 1,
    Cost: 2,
    Normals: 3,
    Distance: 4,
    Barycentrics: 5,
    _count: 6,
}

static Enum_RayTestAgainsts =
{
    BVHSeparate: 0,
    BVHCombined: 1,
    NoBVH: 2,
    _count: 3,
}

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct BVHNode
static StructVertexBufferAttributes_BVHNode =
[
    // lmin
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // left
    {
        format: "uint32",
        offset: 12,
        shaderLocation: 1,
    },
    // lmax
    {
        format: "float32x3",
        offset: 16,
        shaderLocation: 2,
    },
    // right
    {
        format: "uint32",
        offset: 28,
        shaderLocation: 3,
    },
    // rmin
    {
        format: "float32x3",
        offset: 32,
        shaderLocation: 4,
    },
    // triCount
    {
        format: "uint32",
        offset: 44,
        shaderLocation: 5,
    },
    // rmax
    {
        format: "float32x3",
        offset: 48,
        shaderLocation: 6,
    },
    // firstTri
    {
        format: "uint32",
        offset: 60,
        shaderLocation: 7,
    },
];

// -------------------- Private Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture DoRTCS_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_DoRTCS_Output_ReadOnly = null;
texture_DoRTCS_Output_ReadOnly_size = [0, 0, 0];
texture_DoRTCS_Output_ReadOnly_format = "";
texture_DoRTCS_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Constant buffer _DoRTCB
constantBuffer__DoRTCB = null;
constantBuffer__DoRTCB_size = 208;
constantBuffer__DoRTCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader DoRTCS
Hash_Compute_DoRTCS = 0;
ShaderModule_Compute_DoRTCS = null;
BindGroupLayout_Compute_DoRTCS = null;
PipelineLayout_Compute_DoRTCS = null;
Pipeline_Compute_DoRTCS = null;

// -------------------- Imported Members

// Buffer Vertices
buffer_Vertices = null;
buffer_Vertices_count = 0;
buffer_Vertices_stride = 0;
buffer_Vertices_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer TriIndices
buffer_TriIndices = null;
buffer_TriIndices_count = 0;
buffer_TriIndices_stride = 0;
buffer_TriIndices_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer BVHNodes
buffer_BVHNodes = null;
buffer_BVHNodes_count = 0;
buffer_BVHNodes_stride = 0;
buffer_BVHNodes_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer BVHCombinedData
buffer_BVHCombinedData = null;
buffer_BVHCombinedData_count = 0;
buffer_BVHCombinedData_stride = 0;
buffer_BVHCombinedData_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Public Variables

variable_RenderSize = [ 1024, 768 ];
variableDefault_RenderSize = [ 1024, 768 ];
variableChanged_RenderSize = [ false, false ];
variable_ViewMode = this.constructor.Enum_ViewModes.Shaded;
variableDefault_ViewMode = this.constructor.Enum_ViewModes.Shaded;
variableChanged_ViewMode = false;
variable_TMax = 1000.000000;  // The maximum length ray allowed
variableDefault_TMax = 1000.000000;  // The maximum length ray allowed
variableChanged_TMax = false;
variable_CostDivider = 100.000000;  // Cost View Mode: divides by this number to help show the range.
variableDefault_CostDivider = 100.000000;  // Cost View Mode: divides by this number to help show the range.
variableChanged_CostDivider = false;
variable_DistanceDivider = 10.000000;  // Distance View Mode: divides by this number to help show the range.
variableDefault_DistanceDivider = 10.000000;  // Distance View Mode: divides by this number to help show the range.
variableChanged_DistanceDivider = false;
variable_LightDirection = [ 1.000000, -1.000000, 0.000000 ];  // The direction the light is shining.
variableDefault_LightDirection = [ 1.000000, -1.000000, 0.000000 ];  // The direction the light is shining.
variableChanged_LightDirection = [ false, false, false ];
variable_LightColor = [ 1.000000, 1.000000, 1.000000 ];
variableDefault_LightColor = [ 1.000000, 1.000000, 1.000000 ];
variableChanged_LightColor = [ false, false, false ];
variable_LightBrightness = 3.000000;
variableDefault_LightBrightness = 3.000000;
variableChanged_LightBrightness = false;
variable_AmbientColor = [ 0.020000, 0.020000, 0.020000 ];
variableDefault_AmbientColor = [ 0.020000, 0.020000, 0.020000 ];
variableChanged_AmbientColor = [ false, false, false ];
variable_AmbientBrightness = 1.000000;
variableDefault_AmbientBrightness = 1.000000;
variableChanged_AmbientBrightness = false;
variable_Albedo = [ 0.800000, 0.800000, 0.800000 ];
variableDefault_Albedo = [ 0.800000, 0.800000, 0.800000 ];
variableChanged_Albedo = [ false, false, false ];
variable_Roughness = 0.400000;
variableDefault_Roughness = 0.400000;
variableChanged_Roughness = false;
variable_SpecularLevel = 0.500000;
variableDefault_SpecularLevel = 0.500000;
variableChanged_SpecularLevel = false;
variable_Metalic = 0.000000;
variableDefault_Metalic = 0.000000;
variableChanged_Metalic = false;
variable_RayTestAgainst = this.constructor.Enum_RayTestAgainsts.BVHSeparate;
variableDefault_RayTestAgainst = this.constructor.Enum_RayTestAgainsts.BVHSeparate;
variableChanged_RayTestAgainst = false;

// -------------------- Private Variables

variable_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableDefault_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableChanged_CameraPos = [ false, false, false ];
variable_InvViewProjMtx = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_InvViewProjMtx = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_InvViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
// -------------------- Structs

static StructOffsets_BVHNode =
{
    lmin_0: 0,
    lmin_1: 4,
    lmin_2: 8,
    left: 12,
    lmax_0: 16,
    lmax_1: 20,
    lmax_2: 24,
    right: 28,
    rmin_0: 32,
    rmin_1: 36,
    rmin_2: 40,
    triCount: 44,
    rmax_0: 48,
    rmax_1: 52,
    rmax_2: 56,
    firstTri: 60,
    _size: 64,
}

static StructOffsets__DoRTCB =
{
    Albedo_0: 0,
    Albedo_1: 4,
    Albedo_2: 8,
    AmbientBrightness: 12,
    AmbientColor_0: 16,
    AmbientColor_1: 20,
    AmbientColor_2: 24,
    _padding0: 28,
    CameraPos_0: 32,
    CameraPos_1: 36,
    CameraPos_2: 40,
    CostDivider: 44,
    DistanceDivider: 48,
    _padding1: 52,
    _padding2: 56,
    _padding3: 60,
    InvViewProjMtx_0: 64,
    InvViewProjMtx_1: 68,
    InvViewProjMtx_2: 72,
    InvViewProjMtx_3: 76,
    InvViewProjMtx_4: 80,
    InvViewProjMtx_5: 84,
    InvViewProjMtx_6: 88,
    InvViewProjMtx_7: 92,
    InvViewProjMtx_8: 96,
    InvViewProjMtx_9: 100,
    InvViewProjMtx_10: 104,
    InvViewProjMtx_11: 108,
    InvViewProjMtx_12: 112,
    InvViewProjMtx_13: 116,
    InvViewProjMtx_14: 120,
    InvViewProjMtx_15: 124,
    LightBrightness: 128,
    _padding4: 132,
    _padding5: 136,
    _padding6: 140,
    LightColor_0: 144,
    LightColor_1: 148,
    LightColor_2: 152,
    _padding7: 156,
    LightDirection_0: 160,
    LightDirection_1: 164,
    LightDirection_2: 168,
    Metalic: 172,
    RayTestAgainst: 176,
    Roughness: 180,
    SpecularLevel: 184,
    TMax: 188,
    ViewMode: 192,
    _padding8: 196,
    _padding9: 200,
    _padding10: 204,
    _size: 208,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate buffer Vertices
    if (this.buffer_Vertices === null)
    {
        Shared.LogError("Imported resource buffer_Vertices was not provided");
        return false;
    }

    // Validate buffer TriIndices
    if (this.buffer_TriIndices === null)
    {
        Shared.LogError("Imported resource buffer_TriIndices was not provided");
        return false;
    }

    // Validate buffer BVHNodes
    if (this.buffer_BVHNodes === null)
    {
        Shared.LogError("Imported resource buffer_BVHNodes was not provided");
        return false;
    }

    // Validate buffer BVHCombinedData
    if (this.buffer_BVHCombinedData === null)
    {
        Shared.LogError("Imported resource buffer_BVHCombinedData was not provided");
        return false;
    }

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
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
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
                label: "texture tinybvh.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture DoRTCS_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_DoRTCS_Output_ReadOnly !== null && (this.texture_DoRTCS_Output_ReadOnly_format != desiredFormat || this.texture_DoRTCS_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_DoRTCS_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_DoRTCS_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_DoRTCS_Output_ReadOnly.destroy();
            this.texture_DoRTCS_Output_ReadOnly = null;
        }

        if (this.texture_DoRTCS_Output_ReadOnly === null)
        {
            this.texture_DoRTCS_Output_ReadOnly_size = desiredSize.slice();
            this.texture_DoRTCS_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_DoRTCS_Output_ReadOnly_format))
                viewFormats.push(this.texture_DoRTCS_Output_ReadOnly_format);

            this.texture_DoRTCS_Output_ReadOnly = device.createTexture({
                label: "texture tinybvh.DoRTCS_Output_ReadOnly",
                size: this.texture_DoRTCS_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_DoRTCS_Output_ReadOnly_format),
                usage: this.texture_DoRTCS_Output_ReadOnly_usageFlags,
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
    // (Re)create compute shader DoRTCS
    {
        const bindGroupEntries =
        [
            {
                // Vertices
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // TriIndices
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // BVHNodes
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // BVHCombinedData
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Output
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_DoRTCS_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _loadedTexture_0
                binding: 6,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
            {
                // _DoRTCB
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

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_Compute_DoRTCS === null || newHash !== this.Hash_Compute_DoRTCS)
        {
            this.Hash_Compute_DoRTCS = newHash;

            let shaderCode = class_tinybvh.ShaderCode_DoRTCS_DoRT;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_DoRTCS_Output_ReadOnly_format));

            this.ShaderModule_Compute_DoRTCS = device.createShaderModule({ code: shaderCode, label: "Compute Shader DoRTCS"});
            this.BindGroupLayout_Compute_DoRTCS = device.createBindGroupLayout({
                label: "Compute Bind Group Layout DoRTCS",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_DoRTCS = device.createPipelineLayout({
                label: "Compute Pipeline Layout DoRTCS",
                bindGroupLayouts: [this.BindGroupLayout_Compute_DoRTCS],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_DoRTCS = device.createComputePipeline({
                    label: "Compute Pipeline DoRTCS",
                    layout: this.PipelineLayout_Compute_DoRTCS,
                    compute: {
                        module: this.ShaderModule_Compute_DoRTCS,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("DoRTCS");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline DoRTCS",
                    layout: this.PipelineLayout_Compute_DoRTCS,
                    compute: {
                        module: this.ShaderModule_Compute_DoRTCS,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_DoRTCS = handle; this.loadingPromises.delete("DoRTCS"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("tinybvh.Vertices");

    encoder.popDebugGroup(); // "tinybvh.Vertices"

    encoder.pushDebugGroup("tinybvh.TriIndices");

    encoder.popDebugGroup(); // "tinybvh.TriIndices"

    encoder.pushDebugGroup("tinybvh.BVHNodes");

    encoder.popDebugGroup(); // "tinybvh.BVHNodes"

    encoder.pushDebugGroup("tinybvh.Output");

    encoder.popDebugGroup(); // "tinybvh.Output"

    encoder.pushDebugGroup("tinybvh.BVHCombinedData");

    encoder.popDebugGroup(); // "tinybvh.BVHCombinedData"

    encoder.pushDebugGroup("tinybvh.DoRTCS_Output_ReadOnly");

    encoder.popDebugGroup(); // "tinybvh.DoRTCS_Output_ReadOnly"

    encoder.pushDebugGroup("tinybvh.Copy_DoRTCS_Output");

    // Copy texture Output to texture DoRTCS_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_DoRTCS_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_DoRTCS_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "tinybvh.Copy_DoRTCS_Output"

    encoder.pushDebugGroup("tinybvh._loadedTexture_0");

    encoder.popDebugGroup(); // "tinybvh._loadedTexture_0"

    encoder.pushDebugGroup("tinybvh._DoRTCB");

    // Create constant buffer _DoRTCB
    if (this.constantBuffer__DoRTCB === null)
    {
        this.constantBuffer__DoRTCB = device.createBuffer({
            label: "tinybvh._DoRTCB",
            size: Shared.Align(16, this.constructor.StructOffsets__DoRTCB._size),
            usage: this.constantBuffer__DoRTCB_usageFlags,
        });
    }

    // Upload values to constant buffer _DoRTCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__DoRTCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.Albedo_0, this.variable_Albedo[0], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.Albedo_1, this.variable_Albedo[1], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.Albedo_2, this.variable_Albedo[2], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.AmbientBrightness, this.variable_AmbientBrightness, true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.AmbientColor_0, this.variable_AmbientColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.AmbientColor_1, this.variable_AmbientColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.AmbientColor_2, this.variable_AmbientColor[2], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.CameraPos_0, this.variable_CameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.CameraPos_1, this.variable_CameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.CameraPos_2, this.variable_CameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.CostDivider, this.variable_CostDivider, true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.DistanceDivider, this.variable_DistanceDivider, true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_0, this.variable_InvViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_1, this.variable_InvViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_2, this.variable_InvViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_3, this.variable_InvViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_4, this.variable_InvViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_5, this.variable_InvViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_6, this.variable_InvViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_7, this.variable_InvViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_8, this.variable_InvViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_9, this.variable_InvViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_10, this.variable_InvViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_11, this.variable_InvViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_12, this.variable_InvViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_13, this.variable_InvViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_14, this.variable_InvViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.InvViewProjMtx_15, this.variable_InvViewProjMtx[15], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.LightBrightness, this.variable_LightBrightness, true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.LightColor_0, this.variable_LightColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.LightColor_1, this.variable_LightColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.LightColor_2, this.variable_LightColor[2], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.LightDirection_0, this.variable_LightDirection[0], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.LightDirection_1, this.variable_LightDirection[1], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.LightDirection_2, this.variable_LightDirection[2], true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.Metalic, this.variable_Metalic, true);
        view.setInt32(this.constructor.StructOffsets__DoRTCB.RayTestAgainst, this.variable_RayTestAgainst, true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.Roughness, this.variable_Roughness, true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.SpecularLevel, this.variable_SpecularLevel, true);
        view.setFloat32(this.constructor.StructOffsets__DoRTCB.TMax, this.variable_TMax, true);
        view.setInt32(this.constructor.StructOffsets__DoRTCB.ViewMode, this.variable_ViewMode, true);
        device.queue.writeBuffer(this.constantBuffer__DoRTCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "tinybvh._DoRTCB"

    encoder.pushDebugGroup("tinybvh.DoRTCS");

    // Run compute shader DoRTCS
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group DoRTCS",
            layout: this.BindGroupLayout_Compute_DoRTCS,
            entries: [
                {
                    // Vertices
                    binding: 0,
                    resource: { buffer: this.buffer_Vertices }
                },
                {
                    // TriIndices
                    binding: 1,
                    resource: { buffer: this.buffer_TriIndices }
                },
                {
                    // BVHNodes
                    binding: 2,
                    resource: { buffer: this.buffer_BVHNodes }
                },
                {
                    // BVHCombinedData
                    binding: 3,
                    resource: { buffer: this.buffer_BVHCombinedData }
                },
                {
                    // Output
                    binding: 4,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 5,
                    resource: this.texture_DoRTCS_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 6,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _DoRTCB
                    binding: 7,
                    resource: { buffer: this.constantBuffer__DoRTCB }
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
        const baseDispatchSize = this.texture_Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_DoRTCS !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_DoRTCS);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "tinybvh.DoRTCS"

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

var tinybvh = new class_tinybvh;

export default tinybvh;
