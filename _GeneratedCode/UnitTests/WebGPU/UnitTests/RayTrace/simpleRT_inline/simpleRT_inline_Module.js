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

class class_simpleRT_inline
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Non BVH Variation of Compute shader "SimpleRTCS", node "DoRT"
static ShaderCode_DoRT_SimpleRTCS_NonBVH = `
@binding(0) @group(0) var g_texture : texture_storage_2d</*(g_texture_format)*/, write>;

struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_SimpleRTCSCB_std140_0
{
    @align(16) cameraPos_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) clipToWorld_0 : _MatrixStorage_float4x4std140_0,
    @align(16) depthNearPlane_0 : f32,
    @align(4) _padding1_0 : f32,
    @align(8) _padding2_0 : f32,
    @align(4) _padding3_0 : f32,
    @align(16) hitColor_0 : vec3<f32>,
    @align(4) _padding4_0 : f32,
    @align(16) missColor_0 : vec3<f32>,
    @align(4) _padding5_0 : f32,
};

@binding(4) @group(0) var<uniform> _SimpleRTCSCB : Struct_SimpleRTCSCB_std140_0;
struct Struct_VertexBuffer_std430_0
{
    @align(16) Color_0 : vec3<f32>,
    @align(4) _padding0_1 : f32,
    @align(16) Position_0 : vec3<f32>,
    @align(4) _padding1_1 : f32,
    @align(16) Normal_0 : vec3<f32>,
    @align(4) _padding2_1 : f32,
};

@binding(1) @group(0) var<storage, read> g_scene : array<Struct_VertexBuffer_std430_0>;

@binding(2) @group(0) var<storage, read> g_vertexBuffer : array<Struct_VertexBuffer_std430_0>;

fn RayVsTriangle_0( rayPos_0 : vec3<f32>,  rayDir_0 : vec3<f32>,  posA_0 : vec3<f32>,  posB_0 : vec3<f32>,  posC_0 : vec3<f32>) -> vec3<f32>
{
    var e1_0 : vec3<f32> = posB_0 - posA_0;
    var e2_0 : vec3<f32> = posC_0 - posA_0;
    var q_0 : vec3<f32> = cross(rayDir_0, e2_0);
    var a_0 : f32 = dot(e1_0, q_0);
    if((abs(a_0)) == 0.0f)
    {
        return vec3<f32>(-1.0f, 0.0f, 0.0f);
    }
    var s_0 : vec3<f32> = (rayPos_0 - posA_0) / vec3<f32>(a_0);
    var r_0 : vec3<f32> = cross(s_0, e1_0);
    var b_0 : vec3<f32>;
    b_0[i32(0)] = dot(s_0, q_0);
    b_0[i32(1)] = dot(r_0, rayDir_0);
    b_0[i32(2)] = 1.0f - b_0.x - b_0.y;
    var _S1 : bool;
    if((b_0.x) < 0.0f)
    {
        _S1 = true;
    }
    else
    {
        _S1 = (b_0.y) < 0.0f;
    }
    if(_S1)
    {
        _S1 = true;
    }
    else
    {
        _S1 = (b_0.z) < 0.0f;
    }
    if(_S1)
    {
        return vec3<f32>(-1.0f, 0.0f, 0.0f);
    }
    return vec3<f32>(dot(e2_0, r_0), b_0.x, b_0.y);
}

struct RayDesc_0
{
     Origin_0 : vec3<f32>,
     TMin_0 : f32,
     Direction_0 : vec3<f32>,
     TMax_0 : f32,
};

struct RayQuery_0
{
     m_finished_0 : bool,
     m_candidatePrimitiveIndex_0 : i32,
     m_ray_0 : RayDesc_0,
     m_minTAndBary_0 : vec3<f32>,
     m_rayFlags_0 : u32,
};

fn RayQuery_Proceed_0( this_0 : ptr<function, RayQuery_0>) -> bool
{
    if((*this_0).m_finished_0)
    {
        return false;
    }
    (*this_0).m_minTAndBary_0 = vec3<f32>((*this_0).m_ray_0.TMax_0, 0.0f, 0.0f);
    (*this_0).m_candidatePrimitiveIndex_0 = i32(-1);
    var numVertsAndStride_0 : vec2<u32>;
    var _S2 : vec2<u32> = vec2<u32>(arrayLength(&g_scene), 48);
    numVertsAndStride_0[i32(0)] = _S2.x;
    numVertsAndStride_0[i32(1)] = _S2.y;
    var _S3 : u32 = numVertsAndStride_0.x / u32(3);
    var i_0 : u32 = u32(0);
    for(;;)
    {
        if(i_0 < _S3)
        {
        }
        else
        {
            break;
        }
        var _S4 : u32 = i_0 * u32(3);
        var hitTAndBary_0 : vec3<f32> = RayVsTriangle_0((*this_0).m_ray_0.Origin_0, (*this_0).m_ray_0.Direction_0, g_scene[_S4].Position_0, g_scene[_S4 + u32(1)].Position_0, g_scene[_S4 + u32(2)].Position_0);
        var _S5 : f32 = hitTAndBary_0.x;
        var _S6 : bool;
        if(_S5 >= ((*this_0).m_ray_0.TMin_0))
        {
            _S6 = _S5 <= ((*this_0).m_minTAndBary_0.x);
        }
        else
        {
            _S6 = false;
        }
        if(_S6)
        {
            (*this_0).m_minTAndBary_0 = hitTAndBary_0;
            (*this_0).m_candidatePrimitiveIndex_0 = i32(i_0);
        }
        i_0 = i_0 + u32(1);
    }
    (*this_0).m_finished_0 = true;
    return ((*this_0).m_candidatePrimitiveIndex_0) != i32(-1);
}

fn RayQuery_CommittedStatus_0( this_1 : RayQuery_0) -> u32
{
    var _S7 : i32;
    if((this_1.m_candidatePrimitiveIndex_0) != i32(-1))
    {
        _S7 = i32(1);
    }
    else
    {
        _S7 = i32(0);
    }
    return u32(_S7);
}

fn RayQuery_CandidatePrimitiveIndex_0( this_2 : RayQuery_0) -> u32
{
    return u32(this_2.m_candidatePrimitiveIndex_0);
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S8 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S8 = sRGBLo_0.x;
    }
    else
    {
        _S8 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S8;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S8 = sRGBLo_0.y;
    }
    else
    {
        _S8 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S8;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S8 = sRGBLo_0.z;
    }
    else
    {
        _S8 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S8;
    return sRGB_0;
}

fn RayQuery_TraceRayInline_0( _S9 : ptr<function, RayQuery_0>,  _S10 : u32,  _S11 : u32,  _S12 : RayDesc_0)
{
    (*_S9).m_finished_0 = false;
    (*_S9).m_candidatePrimitiveIndex_0 = i32(-1);
    (*_S9).m_ray_0 = _S12;
    (*_S9).m_minTAndBary_0 = vec3<f32>(-1.0f, 0.0f, 0.0f);
    (*_S9).m_rayFlags_0 = (_S10 | (u32(644)));
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn SimpleRTCS(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var w_0 : u32;
    var h_0 : u32;
    {var dim = textureDimensions((g_texture));((w_0)) = dim.x;((h_0)) = dim.y;};
    var _S13 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(f32(w_0), f32(h_0)) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S13;
    screenPos_0[i32(1)] = - _S13.y;
    var _S14 : vec4<f32> = (((mat4x4<f32>(_SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(3)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(3)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(3)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, _SimpleRTCSCB.depthNearPlane_0, 1.0f))));
    var world_0 : vec4<f32> = _S14;
    var _S15 : vec3<f32> = _S14.xyz / vec3<f32>(_S14.w);
    world_0.x = _S15.x;
    world_0.y = _S15.y;
    world_0.z = _S15.z;
    var ray_0 : RayDesc_0;
    ray_0.Origin_0 = _SimpleRTCSCB.cameraPos_0;
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 10000.0f;
    ray_0.Direction_0 = normalize(world_0.xyz - ray_0.Origin_0);
    var rayQuery_0 : RayQuery_0;
    RayQuery_TraceRayInline_0(&(rayQuery_0), u32(0), u32(255), ray_0);
    var _S16 : bool = RayQuery_Proceed_0(&(rayQuery_0));
    if((RayQuery_CommittedStatus_0(rayQuery_0)) == u32(1))
    {
        var _S17 : vec3<f32> = vec3<f32>(0.5f);
        textureStore((g_texture), (px_0), (vec4<f32>(LinearToSRGB_0(g_vertexBuffer[RayQuery_CandidatePrimitiveIndex_0(rayQuery_0) * u32(3)].Normal_0 * _S17 + _S17), 1.0f)));
    }
    else
    {
        textureStore((g_texture), (px_0), (vec4<f32>(LinearToSRGB_0(_SimpleRTCSCB.missColor_0), 1.0f)));
    }
    return;
}

`;

// Shader code for BVH Variation of Compute shader "SimpleRTCS", node "DoRT"
static ShaderCode_DoRT_SimpleRTCS_BVH = `
@binding(0) @group(0) var g_texture : texture_storage_2d</*(g_texture_format)*/, write>;

struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_SimpleRTCSCB_std140_0
{
    @align(16) cameraPos_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) clipToWorld_0 : _MatrixStorage_float4x4std140_0,
    @align(16) depthNearPlane_0 : f32,
    @align(4) _padding1_0 : f32,
    @align(8) _padding2_0 : f32,
    @align(4) _padding3_0 : f32,
    @align(16) hitColor_0 : vec3<f32>,
    @align(4) _padding4_0 : f32,
    @align(16) missColor_0 : vec3<f32>,
    @align(4) _padding5_0 : f32,
};

@binding(4) @group(0) var<uniform> _SimpleRTCSCB : Struct_SimpleRTCSCB_std140_0;
@binding(1) @group(0) var<storage, read> g_scene : array<u32>;

struct Struct_VertexBuffer_std430_0
{
    @align(16) Color_0 : vec3<f32>,
    @align(4) _padding0_1 : f32,
    @align(16) Position_0 : vec3<f32>,
    @align(4) _padding1_1 : f32,
    @align(16) Normal_0 : vec3<f32>,
    @align(4) _padding2_1 : f32,
};

@binding(2) @group(0) var<storage, read> g_vertexBuffer : array<Struct_VertexBuffer_std430_0>;

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

struct RayDesc_0
{
     Origin_0 : vec3<f32>,
     TMin_0 : f32,
     Direction_0 : vec3<f32>,
     TMax_0 : f32,
};

struct RayQuery_0
{
     m_finished_0 : bool,
     m_candidatePrimitiveIndex_0 : i32,
     m_ray_0 : RayDesc_0,
     m_minTAndBary_0 : vec3<f32>,
     m_rayFlags_0 : u32,
};

struct RayVsBVHResult_0
{
     distance_0 : f32,
     barycentrics_0 : vec2<f32>,
     triangleIndex_0 : u32,
};

fn RayVsMesh_BVH_0( _S3 : vec3<f32>,  _S4 : vec3<f32>,  _S5 : vec3<f32>,  _S6 : f32) -> RayVsBVHResult_0
{
    var dist1_0 : f32;
    var altNodeOffset_0 : u32 = u32(2) + g_scene[i32(0)] / u32(4);
    var _S7 : u32 = altNodeOffset_0 + g_scene[i32(1)] / u32(4);
    var hit_0 : vec4<f32>;
    hit_0[i32(0)] = _S6;
    var stack_0 : array<u32, i32(32)>;
    var node_0 : u32 = u32(0);
    var stackPtr_0 : u32 = u32(0);
    for(;;)
    {
        var _S8 : u32 = altNodeOffset_0 + node_0 * u32(16);
        var _S9 : f32 = (bitcast<f32>((g_scene[_S8])));
        var _S10 : f32 = (bitcast<f32>((g_scene[_S8 + u32(1)])));
        var _S11 : f32 = (bitcast<f32>((g_scene[_S8 + u32(2)])));
        var _S12 : u32 = _S8 + u32(4);
        var _S13 : f32 = (bitcast<f32>((g_scene[_S12])));
        var _S14 : f32 = (bitcast<f32>((g_scene[_S12 + u32(1)])));
        var _S15 : f32 = (bitcast<f32>((g_scene[_S12 + u32(2)])));
        var _S16 : u32 = _S8 + u32(8);
        var _S17 : f32 = (bitcast<f32>((g_scene[_S16])));
        var _S18 : f32 = (bitcast<f32>((g_scene[_S16 + u32(1)])));
        var _S19 : f32 = (bitcast<f32>((g_scene[_S16 + u32(2)])));
        var _S20 : u32 = _S8 + u32(12);
        var _S21 : f32 = (bitcast<f32>((g_scene[_S20])));
        var _S22 : f32 = (bitcast<f32>((g_scene[_S20 + u32(1)])));
        var _S23 : f32 = (bitcast<f32>((g_scene[_S20 + u32(2)])));
        var triCount_0 : u32 = g_scene[_S8 + u32(11)];
        var i_0 : u32;
        if(triCount_0 > u32(0))
        {
            var _S24 : u32 = g_scene[_S8 + u32(15)];
            i_0 = u32(0);
            for(;;)
            {
                if(i_0 < triCount_0)
                {
                }
                else
                {
                    break;
                }
                var triIdx_0 : u32 = g_scene[_S7 + _S24 + i_0];
                var _S25 : u32 = u32(3) * triIdx_0;
                var _S26 : u32 = u32(2) + _S25 * u32(4);
                var _S27 : f32 = (bitcast<f32>((g_scene[_S26])));
                var _S28 : f32 = (bitcast<f32>((g_scene[_S26 + u32(1)])));
                var _S29 : f32 = (bitcast<f32>((g_scene[_S26 + u32(2)])));
                var _S30 : vec4<f32> = vec4<f32>(_S27, _S28, _S29, (bitcast<f32>((g_scene[_S26 + u32(3)]))));
                var _S31 : u32 = u32(2) + (_S25 + u32(1)) * u32(4);
                var _S32 : u32 = u32(2) + (_S25 + u32(2)) * u32(4);
                var _S33 : vec3<f32> = (vec4<f32>((bitcast<f32>((g_scene[_S32]))), (bitcast<f32>((g_scene[_S32 + u32(1)]))), (bitcast<f32>((g_scene[_S32 + u32(2)]))), (bitcast<f32>((g_scene[_S32 + u32(3)])))) - _S30).xyz;
                var h_0 : vec3<f32> = cross(_S4, _S33);
                var _S34 : vec3<f32> = (vec4<f32>((bitcast<f32>((g_scene[_S31]))), (bitcast<f32>((g_scene[_S31 + u32(1)]))), (bitcast<f32>((g_scene[_S31 + u32(2)]))), (bitcast<f32>((g_scene[_S31 + u32(3)])))) - _S30).xyz;
                var a_0 : f32 = dot(_S34, h_0);
                if((abs(a_0)) < 1.00000001168609742e-07f)
                {
                    i_0 = i_0 + u32(1);
                    continue;
                }
                var f_0 : f32 = 1.0f / a_0;
                var s_0 : vec3<f32> = _S3 - vec3<f32>(_S27, _S28, _S29);
                var u_0 : f32 = f_0 * dot(s_0, h_0);
                var q_0 : vec3<f32> = cross(s_0, _S34);
                var v_2 : f32 = f_0 * dot(_S4, q_0);
                var _S35 : bool;
                if(u_0 < 0.0f)
                {
                    _S35 = true;
                }
                else
                {
                    _S35 = v_2 < 0.0f;
                }
                var _S36 : bool;
                if(_S35)
                {
                    _S36 = true;
                }
                else
                {
                    _S36 = (u_0 + v_2) > 1.0f;
                }
                if(_S36)
                {
                    i_0 = i_0 + u32(1);
                    continue;
                }
                var d_0 : f32 = f_0 * dot(_S33, q_0);
                var _S37 : bool;
                if(d_0 > 0.0f)
                {
                    _S37 = d_0 < (hit_0.x);
                }
                else
                {
                    _S37 = false;
                }
                if(_S37)
                {
                    hit_0 = vec4<f32>(d_0, u_0, v_2, (bitcast<f32>((triIdx_0))));
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
        var left_0 : u32 = g_scene[_S8 + u32(3)];
        var right_0 : u32 = g_scene[_S8 + u32(7)];
        var t1a_0 : vec3<f32> = (vec3<f32>(_S9, _S10, _S11) - _S3) * _S5;
        var t2a_0 : vec3<f32> = (vec3<f32>(_S13, _S14, _S15) - _S3) * _S5;
        var t1b_0 : vec3<f32> = (vec3<f32>(_S17, _S18, _S19) - _S3) * _S5;
        var t2b_0 : vec3<f32> = (vec3<f32>(_S21, _S22, _S23) - _S3) * _S5;
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
            i_0 = right_0;
            dist2_1 = dist1_0;
            right_1 = left_0;
        }
        else
        {
            dist1_1 = dist1_0;
            i_0 = left_0;
            dist2_1 = dist2_0;
            right_1 = right_0;
        }
        if(dist1_1 == 1.00000001504746622e+30f)
        {
            var _S38 : u32 = u32(0);
            var _S39 : u32 = u32(0);
            if(stackPtr_0 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_2 : u32 = stackPtr_0 - u32(1);
                _S39 = stackPtr_2;
                _S38 = stack_0[stackPtr_2];
            }
            node_0 = _S38;
            stackPtr_0 = _S39;
        }
        else
        {
            var stackPtr_3 : u32;
            if(dist2_1 != 1.00000001504746622e+30f)
            {
                var _S40 : u32 = stackPtr_0 + u32(1);
                stack_0[stackPtr_0] = right_1;
                stackPtr_3 = _S40;
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
    if((hit_0.x) < _S6)
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

fn RayQuery_Proceed_0( this_0 : ptr<function, RayQuery_0>) -> bool
{
    if((*this_0).m_finished_0)
    {
        return false;
    }
    (*this_0).m_minTAndBary_0 = vec3<f32>((*this_0).m_ray_0.TMax_0, 0.0f, 0.0f);
    (*this_0).m_candidatePrimitiveIndex_0 = i32(-1);
    var _S41 : RayVsBVHResult_0 = RayVsMesh_BVH_0((*this_0).m_ray_0.Origin_0, (*this_0).m_ray_0.Direction_0, rcp_1((*this_0).m_ray_0.Direction_0), (*this_0).m_ray_0.TMax_0);
    if((_S41.distance_0) >= 0.0f)
    {
        (*this_0).m_minTAndBary_0 = vec3<f32>(_S41.distance_0, _S41.barycentrics_0);
        (*this_0).m_candidatePrimitiveIndex_0 = i32(_S41.triangleIndex_0);
    }
    (*this_0).m_finished_0 = true;
    return ((*this_0).m_candidatePrimitiveIndex_0) != i32(-1);
}

fn RayQuery_CommittedStatus_0( this_1 : RayQuery_0) -> u32
{
    var _S42 : i32;
    if((this_1.m_candidatePrimitiveIndex_0) != i32(-1))
    {
        _S42 = i32(1);
    }
    else
    {
        _S42 = i32(0);
    }
    return u32(_S42);
}

fn RayQuery_CandidatePrimitiveIndex_0( this_2 : RayQuery_0) -> u32
{
    return u32(this_2.m_candidatePrimitiveIndex_0);
}

fn LinearToSRGB_0( linearCol_0 : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol_0 * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol_0), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S43 : f32;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {
        _S43 = sRGBLo_0.x;
    }
    else
    {
        _S43 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S43;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {
        _S43 = sRGBLo_0.y;
    }
    else
    {
        _S43 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S43;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {
        _S43 = sRGBLo_0.z;
    }
    else
    {
        _S43 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S43;
    return sRGB_0;
}

fn RayQuery_TraceRayInline_0( _S44 : ptr<function, RayQuery_0>,  _S45 : u32,  _S46 : u32,  _S47 : RayDesc_0)
{
    (*_S44).m_finished_0 = false;
    (*_S44).m_candidatePrimitiveIndex_0 = i32(-1);
    (*_S44).m_ray_0 = _S47;
    (*_S44).m_minTAndBary_0 = vec3<f32>(-1.0f, 0.0f, 0.0f);
    (*_S44).m_rayFlags_0 = (_S45 | (u32(644)));
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn SimpleRTCS(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var w_0 : u32;
    var h_1 : u32;
    {var dim = textureDimensions((g_texture));((w_0)) = dim.x;((h_1)) = dim.y;};
    var _S48 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(f32(w_0), f32(h_1)) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S48;
    screenPos_0[i32(1)] = - _S48.y;
    var _S49 : vec4<f32> = (((mat4x4<f32>(_SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(0)][i32(3)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(1)][i32(3)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(2)][i32(3)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(0)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(1)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(2)], _SimpleRTCSCB.clipToWorld_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, _SimpleRTCSCB.depthNearPlane_0, 1.0f))));
    var world_0 : vec4<f32> = _S49;
    var _S50 : vec3<f32> = _S49.xyz / vec3<f32>(_S49.w);
    world_0.x = _S50.x;
    world_0.y = _S50.y;
    world_0.z = _S50.z;
    var ray_0 : RayDesc_0;
    ray_0.Origin_0 = _SimpleRTCSCB.cameraPos_0;
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 10000.0f;
    ray_0.Direction_0 = normalize(world_0.xyz - ray_0.Origin_0);
    var rayQuery_0 : RayQuery_0;
    RayQuery_TraceRayInline_0(&(rayQuery_0), u32(0), u32(255), ray_0);
    var _S51 : bool = RayQuery_Proceed_0(&(rayQuery_0));
    if((RayQuery_CommittedStatus_0(rayQuery_0)) == u32(1))
    {
        var _S52 : vec3<f32> = vec3<f32>(0.5f);
        textureStore((g_texture), (px_0), (vec4<f32>(LinearToSRGB_0(g_vertexBuffer[RayQuery_CandidatePrimitiveIndex_0(rayQuery_0) * u32(3)].Normal_0 * _S52 + _S52), 1.0f)));
    }
    else
    {
        textureStore((g_texture), (px_0), (vec4<f32>(LinearToSRGB_0(_SimpleRTCSCB.missColor_0), 1.0f)));
    }
    return;
}

`;

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct VertexBuffer
static StructVertexBufferAttributes_VertexBuffer =
[
    // Color
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // Position
    {
        format: "float32x3",
        offset: 16,
        shaderLocation: 1,
    },
    // Normal
    {
        format: "float32x3",
        offset: 32,
        shaderLocation: 2,
    },
];

// Vertex buffer attributes for struct VertexBufferSimple
static StructVertexBufferAttributes_VertexBufferSimple =
[
    // Position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
];

// Vertex buffer attributes for struct VertexBufferFull
static StructVertexBufferAttributes_VertexBufferFull =
[
    // Position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // Color
    {
        format: "float32x3",
        offset: 16,
        shaderLocation: 1,
    },
    // Normal
    {
        format: "float32x3",
        offset: 32,
        shaderLocation: 2,
    },
    // Tangent
    {
        format: "float32x4",
        offset: 48,
        shaderLocation: 3,
    },
    // UV
    {
        format: "float32x2",
        offset: 64,
        shaderLocation: 4,
    },
    // MaterialID
    {
        format: "sint32",
        offset: 72,
        shaderLocation: 5,
    },
];

// Vertex buffer attributes for struct VertexBuffer_Unpadded
static StructVertexBufferAttributes_VertexBuffer_Unpadded =
[
    // Color
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // Position
    {
        format: "float32x3",
        offset: 12,
        shaderLocation: 1,
    },
    // Normal
    {
        format: "float32x3",
        offset: 24,
        shaderLocation: 2,
    },
];

// Vertex buffer attributes for struct VertexBufferSimple_Unpadded
static StructVertexBufferAttributes_VertexBufferSimple_Unpadded =
[
    // Position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
];

// Vertex buffer attributes for struct VertexBufferFull_Unpadded
static StructVertexBufferAttributes_VertexBufferFull_Unpadded =
[
    // Position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // Color
    {
        format: "float32x3",
        offset: 12,
        shaderLocation: 1,
    },
    // Normal
    {
        format: "float32x3",
        offset: 24,
        shaderLocation: 2,
    },
    // Tangent
    {
        format: "float32x4",
        offset: 36,
        shaderLocation: 3,
    },
    // UV
    {
        format: "float32x2",
        offset: 52,
        shaderLocation: 4,
    },
    // MaterialID
    {
        format: "sint32",
        offset: 60,
        shaderLocation: 5,
    },
];

// -------------------- Private Members

// Texture DoRT_g_texture_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_DoRT_g_texture_ReadOnly = null;
texture_DoRT_g_texture_ReadOnly_size = [0, 0, 0];
texture_DoRT_g_texture_ReadOnly_format = "";
texture_DoRT_g_texture_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _SimpleRTCSCB
constantBuffer__SimpleRTCSCB = null;
constantBuffer__SimpleRTCSCB_size = 128;
constantBuffer__SimpleRTCSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader DoRT
Hash_Compute_DoRT = 0;
ShaderModule_Compute_DoRT = null;
BindGroupLayout_Compute_DoRT = null;
PipelineLayout_Compute_DoRT = null;
Pipeline_Compute_DoRT = null;

// -------------------- Imported Members

// Buffer Scene : The raytracing scene
buffer_Scene = null;
buffer_Scene_count = 0;
buffer_Scene_stride = 0;
buffer_Scene_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;
buffer_Scene_isAABBs = false; // For raytracing: if true, treated as AABBs for an intersection shader, else treated as triangles
buffer_Scene_isBVH = false; // For raytracing: if true, treated as a BVH of triangles

// Buffer Scene_VB
buffer_Scene_VB = null;
buffer_Scene_VB_count = 0;
buffer_Scene_VB_stride = 0;
buffer_Scene_VB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Exported Members

// Texture Texture : The texture that is rendered to
texture_Texture = null;
texture_Texture_size = [0, 0, 0];
texture_Texture_format = "";
texture_Texture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_enabled = false;
variableDefault_enabled = false;
variableChanged_enabled = false;
variable_clipToWorld = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_clipToWorld = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_clipToWorld = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_cameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableDefault_cameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableChanged_cameraPos = [ false, false, false ];
variable_depthNearPlane = 0.000000;  // The depth value for the near plane.
variableDefault_depthNearPlane = 0.000000;  // The depth value for the near plane.
variableChanged_depthNearPlane = false;
variable_hitColor = [ 0.000000, 1.000000, 0.000000 ];
variableDefault_hitColor = [ 0.000000, 1.000000, 0.000000 ];
variableChanged_hitColor = [ false, false, false ];
variable_missColor = [ 1.000000, 0.000000, 0.000000 ];
variableDefault_missColor = [ 1.000000, 0.000000, 0.000000 ];
variableChanged_missColor = [ false, false, false ];

// -------------------- Structs

static StructOffsets_VertexBuffer =
{
    Color_0: 0,
    Color_1: 4,
    Color_2: 8,
    _padding0: 12,
    Position_0: 16,
    Position_1: 20,
    Position_2: 24,
    _padding1: 28,
    Normal_0: 32,
    Normal_1: 36,
    Normal_2: 40,
    _padding2: 44,
    _size: 48,
}

static StructOffsets_VertexBufferSimple =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    _padding0: 12,
    _size: 16,
}

static StructOffsets_VertexBufferFull =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    _padding0: 12,
    Color_0: 16,
    Color_1: 20,
    Color_2: 24,
    _padding1: 28,
    Normal_0: 32,
    Normal_1: 36,
    Normal_2: 40,
    _padding2: 44,
    Tangent_0: 48,
    Tangent_1: 52,
    Tangent_2: 56,
    Tangent_3: 60,
    UV_0: 64,
    UV_1: 68,
    MaterialID: 72,
    _padding3: 76,
    _size: 80,
}

static StructOffsets__SimpleRTCSCB =
{
    cameraPos_0: 0,
    cameraPos_1: 4,
    cameraPos_2: 8,
    _padding0: 12,
    clipToWorld_0: 16,
    clipToWorld_1: 20,
    clipToWorld_2: 24,
    clipToWorld_3: 28,
    clipToWorld_4: 32,
    clipToWorld_5: 36,
    clipToWorld_6: 40,
    clipToWorld_7: 44,
    clipToWorld_8: 48,
    clipToWorld_9: 52,
    clipToWorld_10: 56,
    clipToWorld_11: 60,
    clipToWorld_12: 64,
    clipToWorld_13: 68,
    clipToWorld_14: 72,
    clipToWorld_15: 76,
    depthNearPlane: 80,
    _padding1: 84,
    _padding2: 88,
    _padding3: 92,
    hitColor_0: 96,
    hitColor_1: 100,
    hitColor_2: 104,
    _padding4: 108,
    missColor_0: 112,
    missColor_1: 116,
    missColor_2: 120,
    _padding5: 124,
    _size: 128,
}

static StructOffsets_VertexBuffer_Unpadded =
{
    Color_0: 0,
    Color_1: 4,
    Color_2: 8,
    Position_0: 12,
    Position_1: 16,
    Position_2: 20,
    Normal_0: 24,
    Normal_1: 28,
    Normal_2: 32,
    _size: 36,
}

static StructOffsets_VertexBufferSimple_Unpadded =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    _size: 12,
}

static StructOffsets_VertexBufferFull_Unpadded =
{
    Position_0: 0,
    Position_1: 4,
    Position_2: 8,
    Color_0: 12,
    Color_1: 16,
    Color_2: 20,
    Normal_0: 24,
    Normal_1: 28,
    Normal_2: 32,
    Tangent_0: 36,
    Tangent_1: 40,
    Tangent_2: 44,
    Tangent_3: 48,
    UV_0: 52,
    UV_1: 56,
    MaterialID: 60,
    _size: 64,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate buffer Scene
    if (this.buffer_Scene === null)
    {
        Shared.LogError("Imported resource buffer_Scene was not provided");
        return false;
    }

    // Validate buffer Scene_VB
    if (this.buffer_Scene_VB === null)
    {
        Shared.LogError("Imported resource buffer_Scene_VB was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Texture
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_Texture !== null && (this.texture_Texture_format != desiredFormat || this.texture_Texture_size[0] != desiredSize[0] || this.texture_Texture_size[1] != desiredSize[1] || this.texture_Texture_size[2] != desiredSize[2]))
        {
            this.texture_Texture.destroy();
            this.texture_Texture = null;
        }

        if (this.texture_Texture === null)
        {
            this.texture_Texture_size = desiredSize.slice();
            this.texture_Texture_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Texture_format))
                viewFormats.push(this.texture_Texture_format);

            this.texture_Texture = device.createTexture({
                label: "texture simpleRT_inline.Texture",
                size: this.texture_Texture_size,
                format: Shared.GetNonSRGBFormat(this.texture_Texture_format),
                usage: this.texture_Texture_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture DoRT_g_texture_ReadOnly
    {
        const baseSize = this.texture_Texture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Texture_format;
        if (this.texture_DoRT_g_texture_ReadOnly !== null && (this.texture_DoRT_g_texture_ReadOnly_format != desiredFormat || this.texture_DoRT_g_texture_ReadOnly_size[0] != desiredSize[0] || this.texture_DoRT_g_texture_ReadOnly_size[1] != desiredSize[1] || this.texture_DoRT_g_texture_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_DoRT_g_texture_ReadOnly.destroy();
            this.texture_DoRT_g_texture_ReadOnly = null;
        }

        if (this.texture_DoRT_g_texture_ReadOnly === null)
        {
            this.texture_DoRT_g_texture_ReadOnly_size = desiredSize.slice();
            this.texture_DoRT_g_texture_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_DoRT_g_texture_ReadOnly_format))
                viewFormats.push(this.texture_DoRT_g_texture_ReadOnly_format);

            this.texture_DoRT_g_texture_ReadOnly = device.createTexture({
                label: "texture simpleRT_inline.DoRT_g_texture_ReadOnly",
                size: this.texture_DoRT_g_texture_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_DoRT_g_texture_ReadOnly_format),
                usage: this.texture_DoRT_g_texture_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader DoRT
    {
        const bindGroupEntries =
        [
            {
                // g_texture
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Texture_format), viewDimension: "2d" }
            },
            {
                // g_scene
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // g_vertexBuffer
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // g_textureReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_DoRT_g_texture_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _SimpleRTCSCB
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        hashString = hashString + this.buffer_Scene_isBVH.toString();
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_DoRT === null || newHash !== this.Hash_Compute_DoRT)
        {
            this.Hash_Compute_DoRT = newHash;

            let shaderCode = this.buffer_Scene_isBVH
                ? class_simpleRT_inline.ShaderCode_DoRT_SimpleRTCS_BVH
                : class_simpleRT_inline.ShaderCode_DoRT_SimpleRTCS_NonBVH
            ;
            shaderCode = "fn SCENE_IS_AABBS() -> bool { return " + this.buffer_Scene_isAABBs.toString() + "; }\n" + shaderCode;
            shaderCode = shaderCode.replace("/*(g_texture_format)*/", Shared.GetNonSRGBFormat(this.texture_Texture_format));
            shaderCode = shaderCode.replace("/*(g_textureReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_DoRT_g_texture_ReadOnly_format));

            this.ShaderModule_Compute_DoRT = device.createShaderModule({ code: shaderCode, label: "Compute Shader DoRT"});
            this.BindGroupLayout_Compute_DoRT = device.createBindGroupLayout({
                label: "Compute Bind Group Layout DoRT",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_DoRT = device.createPipelineLayout({
                label: "Compute Pipeline Layout DoRT",
                bindGroupLayouts: [this.BindGroupLayout_Compute_DoRT],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_DoRT = device.createComputePipeline({
                    label: "Compute Pipeline DoRT",
                    layout: this.PipelineLayout_Compute_DoRT,
                    compute: {
                        module: this.ShaderModule_Compute_DoRT,
                        entryPoint: "SimpleRTCS",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("DoRT");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline DoRT",
                    layout: this.PipelineLayout_Compute_DoRT,
                    compute: {
                        module: this.ShaderModule_Compute_DoRT,
                        entryPoint: "SimpleRTCS",
                    }
                }).then( handle => { this.Pipeline_Compute_DoRT = handle; this.loadingPromises.delete("DoRT"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("simpleRT_inline.Texture");

    encoder.popDebugGroup(); // "simpleRT_inline.Texture"

    encoder.pushDebugGroup("simpleRT_inline.Scene");

    encoder.popDebugGroup(); // "simpleRT_inline.Scene"

    encoder.pushDebugGroup("simpleRT_inline.Scene_VB");

    encoder.popDebugGroup(); // "simpleRT_inline.Scene_VB"

    encoder.pushDebugGroup("simpleRT_inline.DoRT_g_texture_ReadOnly");

    encoder.popDebugGroup(); // "simpleRT_inline.DoRT_g_texture_ReadOnly"

    encoder.pushDebugGroup("simpleRT_inline.Copy_DoRT_g_texture");

    // Copy texture Texture to texture DoRT_g_texture_ReadOnly
    {
        const numMips = Math.min(this.texture_Texture.mipLevelCount, this.texture_DoRT_g_texture_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Texture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Texture.height >> mipIndex, 1);
            let mipDepth = this.texture_Texture.depthOrArrayLayers;

            if (this.texture_Texture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Texture, mipLevel: mipIndex },
                { texture: this.texture_DoRT_g_texture_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "simpleRT_inline.Copy_DoRT_g_texture"

    encoder.pushDebugGroup("simpleRT_inline._SimpleRTCSCB");

    // Create constant buffer _SimpleRTCSCB
    if (this.constantBuffer__SimpleRTCSCB === null)
    {
        this.constantBuffer__SimpleRTCSCB = device.createBuffer({
            label: "simpleRT_inline._SimpleRTCSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__SimpleRTCSCB._size),
            usage: this.constantBuffer__SimpleRTCSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _SimpleRTCSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__SimpleRTCSCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.cameraPos_0, this.variable_cameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.cameraPos_1, this.variable_cameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.cameraPos_2, this.variable_cameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_0, this.variable_clipToWorld[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_1, this.variable_clipToWorld[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_2, this.variable_clipToWorld[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_3, this.variable_clipToWorld[3], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_4, this.variable_clipToWorld[4], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_5, this.variable_clipToWorld[5], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_6, this.variable_clipToWorld[6], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_7, this.variable_clipToWorld[7], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_8, this.variable_clipToWorld[8], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_9, this.variable_clipToWorld[9], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_10, this.variable_clipToWorld[10], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_11, this.variable_clipToWorld[11], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_12, this.variable_clipToWorld[12], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_13, this.variable_clipToWorld[13], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_14, this.variable_clipToWorld[14], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.clipToWorld_15, this.variable_clipToWorld[15], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.depthNearPlane, this.variable_depthNearPlane, true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.hitColor_0, this.variable_hitColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.hitColor_1, this.variable_hitColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.hitColor_2, this.variable_hitColor[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.missColor_0, this.variable_missColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.missColor_1, this.variable_missColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTCSCB.missColor_2, this.variable_missColor[2], true);
        device.queue.writeBuffer(this.constantBuffer__SimpleRTCSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "simpleRT_inline._SimpleRTCSCB"

    encoder.pushDebugGroup("simpleRT_inline.DoRT");

    // Run compute shader DoRT
    if (this.variable_enabled)
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group DoRT",
            layout: this.BindGroupLayout_Compute_DoRT,
            entries: [
                {
                    // g_texture
                    binding: 0,
                    resource: this.texture_Texture.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // g_scene
                    binding: 1,
                    resource: { buffer: this.buffer_Scene }
                },
                {
                    // g_vertexBuffer
                    binding: 2,
                    resource: { buffer: this.buffer_Scene_VB }
                },
                {
                    // g_textureReadOnly
                    binding: 3,
                    resource: this.texture_DoRT_g_texture_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _SimpleRTCSCB
                    binding: 4,
                    resource: { buffer: this.constantBuffer__SimpleRTCSCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Texture_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_DoRT !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_DoRT);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "simpleRT_inline.DoRT"

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

var simpleRT_inline = new class_simpleRT_inline;

export default simpleRT_inline;
