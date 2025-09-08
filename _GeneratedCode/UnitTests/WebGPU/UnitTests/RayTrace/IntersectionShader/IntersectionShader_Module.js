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

class class_IntersectionShader
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Non BVH Variation of RTRayGen shader "Ray_Gen", node "Do_RT"
static ShaderCode_Do_RT_Ray_Gen_NonBVH = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_Ray_GenCB_std140_0
{
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) _dispatchSize_Ray_Gen_0 : vec3<u32>,
    @align(4) _padding1_0 : f32,
};

@binding(4) @group(0) var<uniform> _Ray_GenCB : Struct_Ray_GenCB_std140_0;
@binding(1) @group(0) var<storage, read> AABBs : array<f32>;

@binding(0) @group(0) var<storage, read> Scene : array<f32>;

@binding(2) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

fn RayVsAABB_0( rayPos_0 : vec3<f32>,  rayDir_0 : vec3<f32>,  axesMin_0 : vec3<f32>,  axesMax_0 : vec3<f32>) -> f32
{
    var rayMinTime_0 : f32 = -3.4028234663852886e+38f;
    var rayMaxTime_0 : f32 = 3.4028234663852886e+38f;
    var axis_0 : i32 = i32(0);
    for(;;)
    {
        if(axis_0 < i32(3))
        {
        }
        else
        {
            break;
        }
        var _S1 : i32 = axis_0;
        var _S2 : i32 = axis_0;
        var _S3 : i32 = axis_0;
        if((abs(rayDir_0[axis_0])) < 0.00009999999747379f)
        {
            var _S4 : i32 = axis_0;
            var _S5 : bool;
            if((rayPos_0[axis_0]) < (axesMin_0[_S1]))
            {
                _S5 = true;
            }
            else
            {
                _S5 = (rayPos_0[_S4]) > (axesMax_0[_S2]);
            }
            if(_S5)
            {
                return -1.0f;
            }
        }
        else
        {
            var axisMinTime_0 : f32 = (axesMin_0[_S1] - rayPos_0[axis_0]) / rayDir_0[_S3];
            var axisMaxTime_0 : f32 = (axesMax_0[_S2] - rayPos_0[axis_0]) / rayDir_0[_S3];
            var axisMinTime_1 : f32;
            var axisMaxTime_1 : f32;
            if(axisMinTime_0 > axisMaxTime_0)
            {
                axisMinTime_1 = axisMaxTime_0;
                axisMaxTime_1 = axisMinTime_0;
            }
            else
            {
                axisMinTime_1 = axisMinTime_0;
                axisMaxTime_1 = axisMaxTime_0;
            }
            var rayMinTime_1 : f32 = max(rayMinTime_0, axisMinTime_1);
            var rayMaxTime_1 : f32 = min(rayMaxTime_0, axisMaxTime_1);
            if(rayMinTime_1 > rayMaxTime_1)
            {
                return -1.0f;
            }
            rayMinTime_0 = rayMinTime_1;
            rayMaxTime_0 = rayMaxTime_1;
        }
        axis_0 = axis_0 + i32(1);
    }
    if(rayMinTime_0 >= 0.0f)
    {
    }
    else
    {
        rayMinTime_0 = rayMaxTime_0;
    }
    return rayMinTime_0;
}

fn TestSphereTrace_0( rayPos_1 : vec3<f32>,  rayDir_1 : vec3<f32>,  sphere_0 : vec4<f32>,  normal_0 : ptr<function, vec3<f32>>) -> f32
{
    (*normal_0) = vec3<f32>(0.0f, 0.0f, 0.0f);
    var _S6 : vec3<f32> = sphere_0.xyz;
    var m_0 : vec3<f32> = rayPos_1 - _S6;
    var b_0 : f32 = dot(m_0, rayDir_1);
    var _S7 : f32 = sphere_0.w;
    var c_0 : f32 = dot(m_0, m_0) - _S7 * _S7;
    var _S8 : bool;
    if(c_0 > 0.0f)
    {
        _S8 = b_0 > 0.0f;
    }
    else
    {
        _S8 = false;
    }
    if(_S8)
    {
        return -1.0f;
    }
    var discr_0 : f32 = b_0 * b_0 - c_0;
    if(discr_0 < 0.0f)
    {
        return -1.0f;
    }
    var _S9 : f32 = - b_0;
    var _S10 : f32 = sqrt(discr_0);
    var dist_0 : f32 = _S9 - _S10;
    var _S11 : bool = dist_0 < 0.0f;
    var dist_1 : f32;
    if(_S11)
    {
        dist_1 = _S9 + _S10;
    }
    else
    {
        dist_1 = dist_0;
    }
    var _S12 : vec3<f32> = normalize(rayPos_1 + rayDir_1 * vec3<f32>(dist_1) - _S6);
    var _S13 : f32;
    if(_S11)
    {
        _S13 = -1.0f;
    }
    else
    {
        _S13 = 1.0f;
    }
    (*normal_0) = _S12 * vec3<f32>(_S13);
    return dist_1;
}

struct Payload_0
{
     hit_0 : bool,
     normal_1 : vec3<f32>,
};

struct HitAttributes_0
{
     normal_2 : vec3<f32>,
};

struct RayDesc_0
{
     Origin_0 : vec3<f32>,
     TMin_0 : f32,
     Direction_0 : vec3<f32>,
     TMax_0 : f32,
};

fn AnyHit_0( payload_0 : ptr<function, Payload_0>,  attr_0 : HitAttributes_0,  _anyHitResult_0 : ptr<function, u32>,  _ray_0 : RayDesc_0,  _RayTCurrent_0 : f32,  _PrimitiveIndex_0 : u32)
{
    var _S14 : f32 = attr_0.normal_2.y;
    var _S15 : bool;
    if(_S14 > 0.5f)
    {
        _S15 = _S14 < 0.60000002384185791f;
    }
    else
    {
        _S15 = false;
    }
    if(_S15)
    {
        (*_anyHitResult_0) = u32(1);
        return;
    }
    return;
}

fn RunAnyHitShader_0( payload_1 : ptr<function, Payload_0>,  intersectionInfo_0 : ptr<function, HitAttributes_0>,  anyHitResult_0 : ptr<function, u32>,  Ray_0 : RayDesc_0,  minT_0 : f32,  primitiveIndex_0 : u32,  hitGroupIndex_0 : u32)
{
    switch(hitGroupIndex_0)
    {
    case u32(0), :
        {
            AnyHit_0(&((*payload_1)), (*intersectionInfo_0), &((*anyHitResult_0)), Ray_0, minT_0, primitiveIndex_0);
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn IntersectShaderReportHit_0( _ray_1 : RayDesc_0,  _RayTCurrent_1 : ptr<function, f32>,  _hitGroupIndex_0 : u32,  _payload_0 : ptr<function, Payload_0>,  _PrimitiveIndex_1 : u32,  _HitPrimitiveIndex_0 : ptr<function, u32>,  _attributes_0 : ptr<function, HitAttributes_0>,  tHit_0 : f32,  HitKind_0 : u32,  Attributes_0 : HitAttributes_0) -> bool
{
    var _S16 : HitAttributes_0 = Attributes_0;
    var _S17 : bool;
    if(tHit_0 < (_ray_1.TMin_0))
    {
        _S17 = true;
    }
    else
    {
        _S17 = tHit_0 > (*_RayTCurrent_1);
    }
    if(_S17)
    {
        return false;
    }
    var anyHitResult_1 : u32 = u32(0);
    RunAnyHitShader_0(&((*_payload_0)), &(_S16), &(anyHitResult_1), _ray_1, (*_RayTCurrent_1), _PrimitiveIndex_1, _hitGroupIndex_0);
    if(anyHitResult_1 == u32(1))
    {
        return false;
    }
    (*_RayTCurrent_1) = tHit_0;
    (*_HitPrimitiveIndex_0) = _PrimitiveIndex_1;
    (*_attributes_0) = _S16;
    return true;
}

fn Intersection_0( _ray_2 : RayDesc_0,  _RayTCurrent_2 : ptr<function, f32>,  _PrimitiveIndex_2 : u32,  _HitPrimitiveIndex_1 : ptr<function, u32>,  _hitGroupIndex_1 : u32,  _payload_1 : ptr<function, Payload_0>,  _attributes_1 : ptr<function, HitAttributes_0>)
{
    var _S18 : u32 = _PrimitiveIndex_2 * u32(6);
    var AABBMin_0 : vec3<f32> = vec3<f32>(AABBs[_S18], AABBs[_S18 + u32(1)], AABBs[_S18 + u32(2)]);
    var AABBMax_0 : vec3<f32> = vec3<f32>(AABBs[_S18 + u32(3)], AABBs[_S18 + u32(4)], AABBs[_S18 + u32(5)]);
    var AABBSize_0 : vec3<f32> = AABBMax_0 - AABBMin_0;
    var rayPos_2 : vec3<f32> = _ray_2.Origin_0 - (AABBMin_0 + AABBMax_0) / vec3<f32>(2.0f);
    var _S19 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, min(AABBSize_0.x, min(AABBSize_0.y, AABBSize_0.z)) / 2.0f);
    var attr_1 : HitAttributes_0;
    var _S20 : vec3<f32> = attr_1.normal_2;
    var hitT_0 : f32 = TestSphereTrace_0(rayPos_2, _ray_2.Direction_0, _S19, &(_S20));
    attr_1.normal_2 = _S20;
    if(hitT_0 > 0.0f)
    {
        var _S21 : bool = IntersectShaderReportHit_0(_ray_2, &((*_RayTCurrent_2)), _hitGroupIndex_1, &((*_payload_1)), _PrimitiveIndex_2, &((*_HitPrimitiveIndex_1)), &((*_attributes_1)), hitT_0, u32(0), attr_1);
    }
    return;
}

fn RunIntersectionShader_0( Ray_1 : RayDesc_0,  minT_1 : ptr<function, f32>,  primitiveIndex_1 : u32,  hitIndex_0 : ptr<function, u32>,  hitGroupIndex_1 : u32,  payload_2 : ptr<function, Payload_0>,  intersectionInfo_1 : ptr<function, HitAttributes_0>)
{
    switch(hitGroupIndex_1)
    {
    case u32(0), :
        {
            Intersection_0(Ray_1, &((*minT_1)), primitiveIndex_1, &((*hitIndex_0)), hitGroupIndex_1, &((*payload_2)), &((*intersectionInfo_1)));
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn Miss_0( payload_3 : ptr<function, Payload_0>)
{
    (*payload_3).hit_0 = false;
    return;
}

fn RunMissShader_0( payload_4 : ptr<function, Payload_0>,  missShaderIndex_0 : u32)
{
    switch(missShaderIndex_0)
    {
    case u32(0), :
        {
            Miss_0(&((*payload_4)));
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn ClosestHit_0( payload_5 : ptr<function, Payload_0>,  intersection_0 : HitAttributes_0)
{
    (*payload_5).normal_1 = intersection_0.normal_2;
    return;
}

fn RunClosestHitShader_0( payload_6 : ptr<function, Payload_0>,  intersectionInfo_2 : ptr<function, HitAttributes_0>,  hitGroupIndex_2 : u32,  _PrimitiveIndex_3 : u32)
{
    switch(hitGroupIndex_2)
    {
    case u32(0), :
        {
            ClosestHit_0(&((*payload_6)), (*intersectionInfo_2));
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn RayVsTriangle_0( rayPos_3 : vec3<f32>,  rayDir_2 : vec3<f32>,  posA_0 : vec3<f32>,  posB_0 : vec3<f32>,  posC_0 : vec3<f32>) -> vec3<f32>
{
    var e1_0 : vec3<f32> = posB_0 - posA_0;
    var e2_0 : vec3<f32> = posC_0 - posA_0;
    var q_0 : vec3<f32> = cross(rayDir_2, e2_0);
    var a_0 : f32 = dot(e1_0, q_0);
    if((abs(a_0)) == 0.0f)
    {
        return vec3<f32>(-1.0f, 0.0f, 0.0f);
    }
    var s_0 : vec3<f32> = (rayPos_3 - posA_0) / vec3<f32>(a_0);
    var r_0 : vec3<f32> = cross(s_0, e1_0);
    var b_1 : vec3<f32>;
    b_1[i32(0)] = dot(s_0, q_0);
    b_1[i32(1)] = dot(r_0, rayDir_2);
    b_1[i32(2)] = 1.0f - b_1.x - b_1.y;
    var _S22 : bool;
    if((b_1.x) < 0.0f)
    {
        _S22 = true;
    }
    else
    {
        _S22 = (b_1.y) < 0.0f;
    }
    if(_S22)
    {
        _S22 = true;
    }
    else
    {
        _S22 = (b_1.z) < 0.0f;
    }
    if(_S22)
    {
        return vec3<f32>(-1.0f, 0.0f, 0.0f);
    }
    return vec3<f32>(dot(e2_0, r_0), b_1.x, b_1.y);
}

fn RayVsScene_AABBs_0( _S23 : u32,  _S24 : u32,  _S25 : u32,  _S26 : u32,  _S27 : u32,  _S28 : RayDesc_0,  _S29 : ptr<function, Payload_0>)
{
    var numVertsStride_0 : vec2<u32>;
    var _S30 : vec2<u32> = vec2<u32>(arrayLength(&Scene), 4);
    numVertsStride_0[i32(0)] = _S30.x;
    numVertsStride_0[i32(1)] = _S30.y;
    var _S31 : u32 = numVertsStride_0.x / u32(6);
    var minTAndBary_0 : vec3<f32> = vec3<f32>(_S28.TMax_0, 0.0f, 0.0f);
    var hitIndex_1 : u32 = u32(4294967295);
    var intersectionInfo_3 : HitAttributes_0;
    intersectionInfo_3.normal_2 = vec3<f32>(0.0f, 0.0f, 0.0f);
    var i_0 : u32 = u32(0);
    for(;;)
    {
        if(i_0 < _S31)
        {
        }
        else
        {
            break;
        }
        var _S32 : u32 = i_0 * u32(6);
        if((RayVsAABB_0(_S28.Origin_0, _S28.Direction_0, vec3<f32>(Scene[_S32], Scene[_S32 + u32(1)], Scene[_S32 + u32(2)]), vec3<f32>(Scene[_S32 + u32(3)], Scene[_S32 + u32(4)], Scene[_S32 + u32(5)]))) >= 0.0f)
        {
            var _S33 : f32 = minTAndBary_0[i32(0)];
            RunIntersectionShader_0(_S28, &(_S33), i_0, &(hitIndex_1), _S25, &((*_S29)), &(intersectionInfo_3));
            minTAndBary_0[i32(0)] = _S33;
        }
        i_0 = i_0 + u32(1);
    }
    if(hitIndex_1 == u32(4294967295))
    {
        RunMissShader_0(&((*_S29)), _S27);
    }
    else
    {
        RunClosestHitShader_0(&((*_S29)), &(intersectionInfo_3), _S25, hitIndex_1);
    }
    return;
}

fn RayVsScene_Triangles_0( _S34 : u32,  _S35 : u32,  _S36 : u32,  _S37 : u32,  _S38 : u32,  _S39 : RayDesc_0,  _S40 : ptr<function, Payload_0>)
{
    var numVertsStride_1 : vec2<u32>;
    var _S41 : vec2<u32> = vec2<u32>(arrayLength(&Scene), 4);
    numVertsStride_1[i32(0)] = _S41.x;
    numVertsStride_1[i32(1)] = _S41.y;
    var _S42 : u32 = numVertsStride_1.x / u32(3);
    var _S43 : vec3<f32> = vec3<f32>(_S39.TMax_0, 0.0f, 0.0f);
    var intersectionInfo_4 : HitAttributes_0;
    intersectionInfo_4.normal_2 = vec3<f32>(0.0f, 0.0f, 0.0f);
    var minTAndBary_1 : vec3<f32> = _S43;
    var hitIndex_2 : i32 = i32(-1);
    var i_1 : u32 = u32(0);
    for(;;)
    {
        if(i_1 < _S42)
        {
        }
        else
        {
            break;
        }
        var _S44 : u32 = i_1 * u32(3);
        var hitTAndBary_0 : vec3<f32> = RayVsTriangle_0(_S39.Origin_0, _S39.Direction_0, vec3<f32>(Scene[_S44]), vec3<f32>(Scene[_S44 + u32(1)]), vec3<f32>(Scene[_S44 + u32(2)]));
        var _S45 : f32 = hitTAndBary_0.x;
        var _S46 : bool;
        if(_S45 >= (_S39.TMin_0))
        {
            _S46 = _S45 <= (minTAndBary_1.x);
        }
        else
        {
            _S46 = false;
        }
        var minTAndBary_2 : vec3<f32>;
        var hitIndex_3 : i32;
        if(_S46)
        {
            var anyHitResult_2 : u32 = u32(0);
            RunAnyHitShader_0(&((*_S40)), &(intersectionInfo_4), &(anyHitResult_2), _S39, _S45, i_1, _S36);
            if(anyHitResult_2 == u32(1))
            {
                i_1 = i_1 + u32(1);
                continue;
            }
            var _S47 : i32 = i32(i_1);
            if(anyHitResult_2 == u32(2))
            {
                hitIndex_2 = _S47;
                break;
            }
            minTAndBary_2 = hitTAndBary_0;
            hitIndex_3 = _S47;
        }
        else
        {
            minTAndBary_2 = minTAndBary_1;
            hitIndex_3 = hitIndex_2;
        }
        minTAndBary_1 = minTAndBary_2;
        hitIndex_2 = hitIndex_3;
        i_1 = i_1 + u32(1);
    }
    if(hitIndex_2 == i32(-1))
    {
        RunMissShader_0(&((*_S40)), _S38);
    }
    else
    {
        RunClosestHitShader_0(&((*_S40)), &(intersectionInfo_4), _S36, u32(hitIndex_2));
    }
    return;
}

fn RayVsScene_0( _S48 : u32,  _S49 : u32,  _S50 : u32,  _S51 : u32,  _S52 : u32,  _S53 : RayDesc_0,  _S54 : ptr<function, Payload_0>)
{
    var _S55 : bool = SCENE_IS_AABBS();
    if(_S55)
    {
        RayVsScene_AABBs_0(_S48, _S49, _S50, _S51, _S52, _S53, &((*_S54)));
    }
    else
    {
        RayVsScene_Triangles_0(_S48, _S49, _S50, _S51, _S52, _S53, &((*_S54)));
    }
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn RayGen(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S56 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(_Ray_GenCB._dispatchSize_Ray_Gen_0.xy) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S56;
    screenPos_0[i32(1)] = - _S56.y;
    var _S57 : vec4<f32> = (((mat4x4<f32>(_Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, 0.0f, 1.0f))));
    var world_0 : vec4<f32> = _S57;
    var _S58 : vec3<f32> = _S57.xyz / vec3<f32>(_S57.w);
    world_0.x = _S58.x;
    world_0.y = _S58.y;
    world_0.z = _S58.z;
    var ray_0 : RayDesc_0;
    var _S59 : vec3<f32> = _Ray_GenCB.CameraPos_0;
    ray_0.Origin_0 = _Ray_GenCB.CameraPos_0;
    ray_0.Direction_0 = normalize(world_0.xyz - _S59);
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 1000.0f;
    const _S60 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var payload_7 : Payload_0;
    payload_7.hit_0 = false;
    payload_7.normal_1 = _S60;
    payload_7.hit_0 = true;
    RayVsScene_0(u32(0), u32(255), u32(0), u32(0), u32(0), ray_0, &(payload_7));
    if(payload_7.hit_0)
    {
        var _S61 : vec3<f32> = vec3<f32>(0.5f);
        textureStore((Output), (px_0), (vec4<f32>(payload_7.normal_1 * _S61 + _S61, 1.0f)));
    }
    else
    {
        textureStore((Output), (px_0), (vec4<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f, 1.0f)));
    }
    return;
}

`;

// Shader code for BVH Variation of RTRayGen shader "Ray_Gen", node "Do_RT"
static ShaderCode_Do_RT_Ray_Gen_BVH = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_Ray_GenCB_std140_0
{
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) _dispatchSize_Ray_Gen_0 : vec3<u32>,
    @align(4) _padding1_0 : f32,
};

@binding(4) @group(0) var<uniform> _Ray_GenCB : Struct_Ray_GenCB_std140_0;
@binding(0) @group(0) var<storage, read> Scene : array<u32>;

@binding(2) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

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

struct Payload_0
{
     hit_0 : bool,
     normal_0 : vec3<f32>,
};

struct HitAttributes_0
{
     normal_1 : vec3<f32>,
};

struct RayDesc_0
{
     Origin_0 : vec3<f32>,
     TMin_0 : f32,
     Direction_0 : vec3<f32>,
     TMax_0 : f32,
};

fn AnyHit_0( payload_0 : ptr<function, Payload_0>,  attr_0 : HitAttributes_0,  _anyHitResult_0 : ptr<function, u32>,  _ray_0 : RayDesc_0,  _RayTCurrent_0 : f32,  _PrimitiveIndex_0 : u32)
{
    var _S3 : f32 = attr_0.normal_1.y;
    var _S4 : bool;
    if(_S3 > 0.5f)
    {
        _S4 = _S3 < 0.60000002384185791f;
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        (*_anyHitResult_0) = u32(1);
        return;
    }
    return;
}

fn RunAnyHitShader_0( payload_1 : ptr<function, Payload_0>,  intersectionInfo_0 : ptr<function, HitAttributes_0>,  anyHitResult_0 : ptr<function, u32>,  Ray_0 : RayDesc_0,  minT_0 : f32,  primitiveIndex_0 : u32,  hitGroupIndex_0 : u32)
{
    switch(hitGroupIndex_0)
    {
    case u32(0), :
        {
            AnyHit_0(&((*payload_1)), (*intersectionInfo_0), &((*anyHitResult_0)), Ray_0, minT_0, primitiveIndex_0);
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn Miss_0( payload_2 : ptr<function, Payload_0>)
{
    (*payload_2).hit_0 = false;
    return;
}

fn RunMissShader_0( payload_3 : ptr<function, Payload_0>,  missShaderIndex_0 : u32)
{
    switch(missShaderIndex_0)
    {
    case u32(0), :
        {
            Miss_0(&((*payload_3)));
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn ClosestHit_0( payload_4 : ptr<function, Payload_0>,  intersection_0 : HitAttributes_0)
{
    (*payload_4).normal_0 = intersection_0.normal_1;
    return;
}

fn RunClosestHitShader_0( payload_5 : ptr<function, Payload_0>,  intersectionInfo_1 : ptr<function, HitAttributes_0>,  hitGroupIndex_1 : u32,  _PrimitiveIndex_1 : u32)
{
    switch(hitGroupIndex_1)
    {
    case u32(0), :
        {
            ClosestHit_0(&((*payload_5)), (*intersectionInfo_1));
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn RayVsScene_BVH_0( _S5 : u32,  _S6 : u32,  _S7 : u32,  _S8 : u32,  _S9 : u32,  _S10 : RayDesc_0,  _S11 : ptr<function, Payload_0>)
{
    var intersectionInfo_2 : HitAttributes_0;
    intersectionInfo_2.normal_1 = vec3<f32>(0.0f, 0.0f, 0.0f);
    var _S12 : vec3<f32> = rcp_1(_S10.Direction_0);
    var altNodeOffset_0 : u32 = u32(2) + Scene[i32(0)] / u32(4);
    var _S13 : u32 = altNodeOffset_0 + Scene[i32(1)] / u32(4);
    var hit_1 : vec4<f32>;
    hit_1[i32(0)] = _S10.TMax_0;
    var stack_0 : array<u32, i32(32)>;
    var forceExitSearch_0 : bool = false;
    var node_0 : u32 = u32(0);
    var stackPtr_0 : u32 = u32(0);
    for(;;)
    {
        if(!forceExitSearch_0)
        {
        }
        else
        {
            break;
        }
        var _S14 : u32 = altNodeOffset_0 + node_0 * u32(16);
        var _S15 : f32 = (bitcast<f32>((Scene[_S14])));
        var _S16 : f32 = (bitcast<f32>((Scene[_S14 + u32(1)])));
        var _S17 : f32 = (bitcast<f32>((Scene[_S14 + u32(2)])));
        var _S18 : u32 = _S14 + u32(4);
        var _S19 : f32 = (bitcast<f32>((Scene[_S18])));
        var _S20 : f32 = (bitcast<f32>((Scene[_S18 + u32(1)])));
        var _S21 : f32 = (bitcast<f32>((Scene[_S18 + u32(2)])));
        var _S22 : u32 = _S14 + u32(8);
        var _S23 : f32 = (bitcast<f32>((Scene[_S22])));
        var _S24 : f32 = (bitcast<f32>((Scene[_S22 + u32(1)])));
        var _S25 : f32 = (bitcast<f32>((Scene[_S22 + u32(2)])));
        var _S26 : u32 = _S14 + u32(12);
        var _S27 : f32 = (bitcast<f32>((Scene[_S26])));
        var _S28 : f32 = (bitcast<f32>((Scene[_S26 + u32(1)])));
        var _S29 : f32 = (bitcast<f32>((Scene[_S26 + u32(2)])));
        var triCount_0 : u32 = Scene[_S14 + u32(11)];
        var i_0 : u32;
        if(triCount_0 > u32(0))
        {
            var _S30 : u32 = Scene[_S14 + u32(15)];
            var forceExitSearch_1 : bool = forceExitSearch_0;
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
                var triIdx_0 : u32 = Scene[_S13 + _S30 + i_0];
                var _S31 : u32 = u32(3) * triIdx_0;
                var _S32 : u32 = u32(2) + _S31 * u32(4);
                var _S33 : f32 = (bitcast<f32>((Scene[_S32])));
                var _S34 : f32 = (bitcast<f32>((Scene[_S32 + u32(1)])));
                var _S35 : f32 = (bitcast<f32>((Scene[_S32 + u32(2)])));
                var _S36 : vec4<f32> = vec4<f32>(_S33, _S34, _S35, (bitcast<f32>((Scene[_S32 + u32(3)]))));
                var _S37 : u32 = u32(2) + (_S31 + u32(1)) * u32(4);
                var _S38 : u32 = u32(2) + (_S31 + u32(2)) * u32(4);
                var _S39 : vec3<f32> = (vec4<f32>((bitcast<f32>((Scene[_S38]))), (bitcast<f32>((Scene[_S38 + u32(1)]))), (bitcast<f32>((Scene[_S38 + u32(2)]))), (bitcast<f32>((Scene[_S38 + u32(3)])))) - _S36).xyz;
                var h_0 : vec3<f32> = cross(_S10.Direction_0, _S39);
                var _S40 : vec3<f32> = (vec4<f32>((bitcast<f32>((Scene[_S37]))), (bitcast<f32>((Scene[_S37 + u32(1)]))), (bitcast<f32>((Scene[_S37 + u32(2)]))), (bitcast<f32>((Scene[_S37 + u32(3)])))) - _S36).xyz;
                var a_0 : f32 = dot(_S40, h_0);
                var forceExitSearch_2 : bool;
                if((abs(a_0)) < 1.00000001168609742e-07f)
                {
                    forceExitSearch_2 = forceExitSearch_1;
                    var _S41 : u32 = i_0 + u32(1);
                    forceExitSearch_1 = forceExitSearch_2;
                    i_0 = _S41;
                    continue;
                }
                var f_0 : f32 = 1.0f / a_0;
                var s_0 : vec3<f32> = _S10.Origin_0 - vec3<f32>(_S33, _S34, _S35);
                var u_0 : f32 = f_0 * dot(s_0, h_0);
                var q_0 : vec3<f32> = cross(s_0, _S40);
                var v_2 : f32 = f_0 * dot(_S10.Direction_0, q_0);
                var _S42 : bool;
                if(u_0 < 0.0f)
                {
                    _S42 = true;
                }
                else
                {
                    _S42 = v_2 < 0.0f;
                }
                var _S43 : bool;
                if(_S42)
                {
                    _S43 = true;
                }
                else
                {
                    _S43 = (u_0 + v_2) > 1.0f;
                }
                if(_S43)
                {
                    forceExitSearch_2 = forceExitSearch_1;
                    var _S41 : u32 = i_0 + u32(1);
                    forceExitSearch_1 = forceExitSearch_2;
                    i_0 = _S41;
                    continue;
                }
                var d_0 : f32 = f_0 * dot(_S39, q_0);
                var _S44 : bool;
                if(d_0 > 0.0f)
                {
                    _S44 = d_0 < (hit_1.x);
                }
                else
                {
                    _S44 = false;
                }
                if(_S44)
                {
                    var anyHitResult_1 : u32 = u32(0);
                    RunAnyHitShader_0(&((*_S11)), &(intersectionInfo_2), &(anyHitResult_1), _S10, d_0, triIdx_0, _S7);
                    if(anyHitResult_1 == u32(1))
                    {
                        forceExitSearch_2 = forceExitSearch_1;
                        var _S41 : u32 = i_0 + u32(1);
                        forceExitSearch_1 = forceExitSearch_2;
                        i_0 = _S41;
                        continue;
                    }
                    hit_1 = vec4<f32>(d_0, u_0, v_2, (bitcast<f32>((triIdx_0))));
                    if(anyHitResult_1 == u32(2))
                    {
                        forceExitSearch_2 = true;
                    }
                    else
                    {
                        forceExitSearch_2 = forceExitSearch_1;
                    }
                }
                else
                {
                    forceExitSearch_2 = forceExitSearch_1;
                }
                var _S41 : u32 = i_0 + u32(1);
                forceExitSearch_1 = forceExitSearch_2;
                i_0 = _S41;
            }
            if(forceExitSearch_1)
            {
                break;
            }
            if(stackPtr_0 == u32(0))
            {
                break;
            }
            var stackPtr_1 : u32 = stackPtr_0 - u32(1);
            forceExitSearch_0 = forceExitSearch_1;
            node_0 = stack_0[stackPtr_1];
            stackPtr_0 = stackPtr_1;
            continue;
        }
        var left_0 : u32 = Scene[_S14 + u32(3)];
        var right_0 : u32 = Scene[_S14 + u32(7)];
        var t1a_0 : vec3<f32> = (vec3<f32>(_S15, _S16, _S17) - _S10.Origin_0) * _S12;
        var t2a_0 : vec3<f32> = (vec3<f32>(_S19, _S20, _S21) - _S10.Origin_0) * _S12;
        var t1b_0 : vec3<f32> = (vec3<f32>(_S23, _S24, _S25) - _S10.Origin_0) * _S12;
        var t2b_0 : vec3<f32> = (vec3<f32>(_S27, _S28, _S29) - _S10.Origin_0) * _S12;
        var minta_0 : vec3<f32> = min(t1a_0, t2a_0);
        var maxta_0 : vec3<f32> = max(t1a_0, t2a_0);
        var mintb_0 : vec3<f32> = min(t1b_0, t2b_0);
        var maxtb_0 : vec3<f32> = max(t1b_0, t2b_0);
        var tmina_0 : f32 = max(max(max(minta_0.x, minta_0.y), minta_0.z), 0.0f);
        var tminb_0 : f32 = max(max(max(mintb_0.x, mintb_0.y), mintb_0.z), 0.0f);
        var tmaxb_0 : f32 = min(min(min(maxtb_0.x, maxtb_0.y), maxtb_0.z), hit_1.x);
        var dist1_0 : f32;
        if(tmina_0 > (min(min(min(maxta_0.x, maxta_0.y), maxta_0.z), hit_1.x)))
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
            var _S45 : u32 = u32(0);
            var _S46 : u32 = u32(0);
            if(stackPtr_0 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_2 : u32 = stackPtr_0 - u32(1);
                _S46 = stackPtr_2;
                _S45 = stack_0[stackPtr_2];
            }
            node_0 = _S45;
            stackPtr_0 = _S46;
        }
        else
        {
            var stackPtr_3 : u32;
            if(dist2_1 != 1.00000001504746622e+30f)
            {
                var _S47 : u32 = stackPtr_0 + u32(1);
                stack_0[stackPtr_0] = right_1;
                stackPtr_3 = _S47;
            }
            else
            {
                stackPtr_3 = stackPtr_0;
            }
            node_0 = i_0;
            stackPtr_0 = stackPtr_3;
        }
    }
    if((hit_1.x) == (_S10.TMax_0))
    {
        RunMissShader_0(&((*_S11)), _S9);
    }
    else
    {
        RunClosestHitShader_0(&((*_S11)), &(intersectionInfo_2), _S7, (bitcast<u32>((hit_1.w))));
    }
    return;
}

fn RayVsScene_0( _S48 : u32,  _S49 : u32,  _S50 : u32,  _S51 : u32,  _S52 : u32,  _S53 : RayDesc_0,  _S54 : ptr<function, Payload_0>)
{
    RayVsScene_BVH_0(_S48, _S49, _S50, _S51, _S52, _S53, &((*_S54)));
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn RayGen(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S55 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(_Ray_GenCB._dispatchSize_Ray_Gen_0.xy) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S55;
    screenPos_0[i32(1)] = - _S55.y;
    var _S56 : vec4<f32> = (((mat4x4<f32>(_Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _Ray_GenCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, 0.0f, 1.0f))));
    var world_0 : vec4<f32> = _S56;
    var _S57 : vec3<f32> = _S56.xyz / vec3<f32>(_S56.w);
    world_0.x = _S57.x;
    world_0.y = _S57.y;
    world_0.z = _S57.z;
    var ray_0 : RayDesc_0;
    var _S58 : vec3<f32> = _Ray_GenCB.CameraPos_0;
    ray_0.Origin_0 = _Ray_GenCB.CameraPos_0;
    ray_0.Direction_0 = normalize(world_0.xyz - _S58);
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 1000.0f;
    const _S59 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var payload_6 : Payload_0;
    payload_6.hit_0 = false;
    payload_6.normal_0 = _S59;
    payload_6.hit_0 = true;
    RayVsScene_0(u32(0), u32(255), u32(0), u32(0), u32(0), ray_0, &(payload_6));
    if(payload_6.hit_0)
    {
        var _S60 : vec3<f32> = vec3<f32>(0.5f);
        textureStore((Output), (px_0), (vec4<f32>(payload_6.normal_0 * _S60 + _S60, 1.0f)));
    }
    else
    {
        textureStore((Output), (px_0), (vec4<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f, 1.0f)));
    }
    return;
}

`;

// -------------------- Private Members

// Texture Do_RT_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Do_RT_Output_ReadOnly = null;
texture_Do_RT_Output_ReadOnly_size = [0, 0, 0];
texture_Do_RT_Output_ReadOnly_format = "";
texture_Do_RT_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _Ray_GenCB
constantBuffer__Ray_GenCB = null;
constantBuffer__Ray_GenCB_size = 96;
constantBuffer__Ray_GenCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// RayGen Shader Do_RT
Hash_RayGen_Do_RT = 0;
ShaderModule_RayGen_Do_RT = null;
BindGroupLayout_RayGen_Do_RT = null;
PipelineLayout_RayGen_Do_RT = null;
Pipeline_RayGen_Do_RT = null;

// -------------------- Imported Members

// Buffer AABBs
buffer_AABBs = null;
buffer_AABBs_count = 0;
buffer_AABBs_stride = 0;
buffer_AABBs_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;
buffer_AABBs_isAABBs = false; // For raytracing: if true, treated as AABBs for an intersection shader, else treated as triangles
buffer_AABBs_isBVH = false; // For raytracing: if true, treated as a BVH of triangles

// Buffer AABBsSRV
buffer_AABBsSRV = null;
buffer_AABBsSRV_count = 0;
buffer_AABBsSRV_stride = 0;
buffer_AABBsSRV_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Exported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

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
variable_CameraChanged = false;
variableDefault_CameraChanged = false;
variableChanged_CameraChanged = false;

// -------------------- Private Variables

variable__dispatchSize_Ray_Gen = [ 0,0,0 ];
variableDefault__dispatchSize_Ray_Gen = [ 0,0,0 ];
variableChanged__dispatchSize_Ray_Gen = [ false, false, false ];
// -------------------- Structs

static StructOffsets__Ray_GenCB =
{
    CameraPos_0: 0,
    CameraPos_1: 4,
    CameraPos_2: 8,
    _padding0: 12,
    InvViewProjMtx_0: 16,
    InvViewProjMtx_1: 20,
    InvViewProjMtx_2: 24,
    InvViewProjMtx_3: 28,
    InvViewProjMtx_4: 32,
    InvViewProjMtx_5: 36,
    InvViewProjMtx_6: 40,
    InvViewProjMtx_7: 44,
    InvViewProjMtx_8: 48,
    InvViewProjMtx_9: 52,
    InvViewProjMtx_10: 56,
    InvViewProjMtx_11: 60,
    InvViewProjMtx_12: 64,
    InvViewProjMtx_13: 68,
    InvViewProjMtx_14: 72,
    InvViewProjMtx_15: 76,
    _dispatchSize_Ray_Gen_0: 80,
    _dispatchSize_Ray_Gen_1: 84,
    _dispatchSize_Ray_Gen_2: 88,
    _padding1: 92,
    _size: 96,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate buffer AABBs
    if (this.buffer_AABBs === null)
    {
        Shared.LogError("Imported resource buffer_AABBs was not provided");
        return false;
    }

    // Validate buffer AABBsSRV
    if (this.buffer_AABBsSRV === null)
    {
        Shared.LogError("Imported resource buffer_AABBsSRV was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Output
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 256) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 256) / 1) + 0,
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
                label: "texture IntersectionShader.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Do_RT_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Do_RT_Output_ReadOnly !== null && (this.texture_Do_RT_Output_ReadOnly_format != desiredFormat || this.texture_Do_RT_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Do_RT_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Do_RT_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Do_RT_Output_ReadOnly.destroy();
            this.texture_Do_RT_Output_ReadOnly = null;
        }

        if (this.texture_Do_RT_Output_ReadOnly === null)
        {
            this.texture_Do_RT_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Do_RT_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Do_RT_Output_ReadOnly_format))
                viewFormats.push(this.texture_Do_RT_Output_ReadOnly_format);

            this.texture_Do_RT_Output_ReadOnly = device.createTexture({
                label: "texture IntersectionShader.Do_RT_Output_ReadOnly",
                size: this.texture_Do_RT_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Do_RT_Output_ReadOnly_format),
                usage: this.texture_Do_RT_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create raygen shader Do_RT
    {
        const bindGroupEntries =
        [
            {
                // Scene
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // AABBs
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Output
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Do_RT_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _Ray_GenCB
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        hashString = hashString + this.buffer_AABBs_isBVH.toString();
        const newHash = hashString.hashCode();

        if (this.ShaderModule_RayGen_Do_RT === null || newHash !== this.Hash_RayGen_Do_RT)
        {
            this.Hash_RayGen_Do_RT = newHash;

            let shaderCode = this.buffer_AABBs_isBVH
                ? class_IntersectionShader.ShaderCode_Do_RT_Ray_Gen_BVH
                : class_IntersectionShader.ShaderCode_Do_RT_Ray_Gen_NonBVH
            ;
            shaderCode = "fn SCENE_IS_AABBS() -> bool { return " + this.buffer_AABBs_isAABBs.toString() + "; }\n" + shaderCode;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Do_RT_Output_ReadOnly_format));

            this.ShaderModule_RayGen_Do_RT = device.createShaderModule({ code: shaderCode, label: "RayGen Shader Do_RT"});
            this.BindGroupLayout_RayGen_Do_RT = device.createBindGroupLayout({
                label: "RayGen Bind Group Layout Do_RT",
                entries: bindGroupEntries
            });

            this.PipelineLayout_RayGen_Do_RT = device.createPipelineLayout({
                label: "RayGen Pipeline Layout Do_RT",
                bindGroupLayouts: [this.BindGroupLayout_RayGen_Do_RT],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_RayGen_Do_RT = device.createComputePipeline({
                    label: "RayGen Pipeline Do_RT",
                    layout: this.PipelineLayout_RayGen_Do_RT,
                    compute: {
                        module: this.ShaderModule_RayGen_Do_RT,
                        entryPoint: "RayGen",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Do_RT");

                device.createComputePipelineAsync({
                    label: "RayGen Pipeline Do_RT",
                    layout: this.PipelineLayout_RayGen_Do_RT,
                    compute: {
                        module: this.ShaderModule_RayGen_Do_RT,
                        entryPoint: "RayGen",
                    }
                }).then( handle => { this.Pipeline_RayGen_Do_RT = handle; this.loadingPromises.delete("Do_RT"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("IntersectionShader.AABBs");

    encoder.popDebugGroup(); // "IntersectionShader.AABBs"

    encoder.pushDebugGroup("IntersectionShader.Output");

    encoder.popDebugGroup(); // "IntersectionShader.Output"

    encoder.pushDebugGroup("IntersectionShader.AABBsSRV");

    encoder.popDebugGroup(); // "IntersectionShader.AABBsSRV"

    encoder.pushDebugGroup("IntersectionShader.Do_RT_Output_ReadOnly");

    encoder.popDebugGroup(); // "IntersectionShader.Do_RT_Output_ReadOnly"

    encoder.pushDebugGroup("IntersectionShader.Copy_Do_RT_Output");

    // Copy texture Output to texture Do_RT_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Do_RT_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Do_RT_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "IntersectionShader.Copy_Do_RT_Output"

    encoder.pushDebugGroup("IntersectionShader._Ray_GenCB");

    // Create constant buffer _Ray_GenCB
    if (this.constantBuffer__Ray_GenCB === null)
    {
        this.constantBuffer__Ray_GenCB = device.createBuffer({
            label: "IntersectionShader._Ray_GenCB",
            size: Shared.Align(16, this.constructor.StructOffsets__Ray_GenCB._size),
            usage: this.constantBuffer__Ray_GenCB_usageFlags,
        });
    }

    // Upload values to constant buffer _Ray_GenCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Ray_GenCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.CameraPos_0, this.variable_CameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.CameraPos_1, this.variable_CameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.CameraPos_2, this.variable_CameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_0, this.variable_InvViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_1, this.variable_InvViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_2, this.variable_InvViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_3, this.variable_InvViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_4, this.variable_InvViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_5, this.variable_InvViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_6, this.variable_InvViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_7, this.variable_InvViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_8, this.variable_InvViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_9, this.variable_InvViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_10, this.variable_InvViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_11, this.variable_InvViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_12, this.variable_InvViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_13, this.variable_InvViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_14, this.variable_InvViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__Ray_GenCB.InvViewProjMtx_15, this.variable_InvViewProjMtx[15], true);
        view.setUint32(this.constructor.StructOffsets__Ray_GenCB._dispatchSize_Ray_Gen_0, this.variable__dispatchSize_Ray_Gen[0], true);
        view.setUint32(this.constructor.StructOffsets__Ray_GenCB._dispatchSize_Ray_Gen_1, this.variable__dispatchSize_Ray_Gen[1], true);
        view.setUint32(this.constructor.StructOffsets__Ray_GenCB._dispatchSize_Ray_Gen_2, this.variable__dispatchSize_Ray_Gen[2], true);
        device.queue.writeBuffer(this.constantBuffer__Ray_GenCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "IntersectionShader._Ray_GenCB"

    encoder.pushDebugGroup("IntersectionShader.Do_RT");

    // Run raygen shader Do_RT
    {
        const bindGroup = device.createBindGroup({
            label: "RayGen Bind Group Do_RT",
            layout: this.BindGroupLayout_RayGen_Do_RT,
            entries: [
                {
                    // Scene
                    binding: 0,
                    resource: { buffer: this.buffer_AABBs }
                },
                {
                    // AABBs
                    binding: 1,
                    resource: { buffer: this.buffer_AABBsSRV }
                },
                {
                    // Output
                    binding: 2,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 3,
                    resource: this.texture_Do_RT_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _Ray_GenCB
                    binding: 4,
                    resource: { buffer: this.constantBuffer__Ray_GenCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Output_size;
        const dispatchSizePreDiv = [
            Math.floor((parseInt(baseDispatchSize[0]) + 0) * 1) / 1 + 0,
            Math.floor((parseInt(baseDispatchSize[1]) + 0) * 1) / 1 + 0,
            Math.floor((parseInt(baseDispatchSize[2]) + 0) * 1) / 1 + 0
        ];

        const dispatchSize = [
            Math.floor((parseInt(dispatchSizePreDiv[0]) + 8 - 1) / 8),
            Math.floor((parseInt(dispatchSizePreDiv[1]) + 8 - 1) / 8),
            Math.floor((parseInt(dispatchSizePreDiv[2]) + 1 - 1) / 1)
        ];

        // Update the dispatch size in the constant buffer
        {
            let specialVariablesBuffer = new Uint32Array(3);
            specialVariablesBuffer[0] = dispatchSizePreDiv[0];
            specialVariablesBuffer[1] = dispatchSizePreDiv[1];
            specialVariablesBuffer[2] = dispatchSizePreDiv[2];
            device.queue.writeBuffer(this.constantBuffer__Ray_GenCB, this.constructor.StructOffsets__Ray_GenCB._dispatchSize_Ray_Gen_0, specialVariablesBuffer);
        }

        if (this.Pipeline_RayGen_Do_RT !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_RayGen_Do_RT);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "IntersectionShader.Do_RT"

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

var IntersectionShader = new class_IntersectionShader;

export default IntersectionShader;
