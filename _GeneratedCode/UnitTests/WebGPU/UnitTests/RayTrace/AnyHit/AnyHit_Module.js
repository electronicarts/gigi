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

class class_AnyHit
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Non BVH Variation of RTRayGen shader "RayGen", node "DoRT"
static ShaderCode_DoRT_RayGen_NonBVH = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_RayGenCB_std140_0
{
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) _dispatchSize_RayGen_0 : vec3<u32>,
    @align(4) depthNearPlane_0 : f32,
};

@binding(4) @group(0) var<uniform> _RayGenCB : Struct_RayGenCB_std140_0;
struct Struct_VertexBuffer_std430_0
{
    @align(16) position_0 : vec3<f32>,
    @align(4) _padding0_1 : f32,
    @align(16) normal_0 : vec3<f32>,
    @align(4) _padding1_0 : f32,
    @align(16) uv_0 : vec2<f32>,
    @align(8) materialId_0 : u32,
    @align(4) _padding2_0 : f32,
};

@binding(2) @group(0) var<storage, read> vertexBuffer : array<Struct_VertexBuffer_std430_0>;

@binding(1) @group(0) var<storage, read> scene : array<Struct_VertexBuffer_std430_0>;

@binding(0) @group(0) var output : texture_storage_2d</*(output_format)*/, write>;

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

struct RayDesc_0
{
     Origin_0 : vec3<f32>,
     TMin_0 : f32,
     Direction_0 : vec3<f32>,
     TMax_0 : f32,
};

struct Payload_0
{
     hit_0 : bool,
     color_0 : vec3<f32>,
     px_0 : vec2<u32>,
};

struct BuiltInTriangleIntersectionAttributes_0
{
     barycentrics_0 : vec2<f32>,
};

fn RunIntersectionShader_0( Ray_0 : RayDesc_0,  minT_0 : ptr<function, f32>,  primitiveIndex_0 : u32,  hitIndex_0 : ptr<function, u32>,  hitGroupIndex_0 : u32,  payload_0 : ptr<function, Payload_0>,  intersectionInfo_0 : ptr<function, BuiltInTriangleIntersectionAttributes_0>)
{
    return;
}

fn Miss_0( payload_1 : ptr<function, Payload_0>)
{
    (*payload_1).hit_0 = false;
    return;
}

fn RunMissShader_0( payload_2 : ptr<function, Payload_0>,  missShaderIndex_0 : u32)
{
    switch(missShaderIndex_0)
    {
    case u32(0), :
        {
            Miss_0(&((*payload_2)));
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn ClosestHit_0( payload_3 : ptr<function, Payload_0>,  intersection_0 : BuiltInTriangleIntersectionAttributes_0)
{
    (*payload_3).hit_0 = true;
    return;
}

fn RunClosestHitShader_0( payload_4 : ptr<function, Payload_0>,  intersectionInfo_1 : ptr<function, BuiltInTriangleIntersectionAttributes_0>,  hitGroupIndex_1 : u32,  _PrimitiveIndex_0 : u32)
{
    switch(hitGroupIndex_1)
    {
    case u32(0), :
        {
            ClosestHit_0(&((*payload_4)), (*intersectionInfo_1));
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn RayVsTriangle_0( rayPos_1 : vec3<f32>,  rayDir_1 : vec3<f32>,  posA_0 : vec3<f32>,  posB_0 : vec3<f32>,  posC_0 : vec3<f32>) -> vec3<f32>
{
    var e1_0 : vec3<f32> = posB_0 - posA_0;
    var e2_0 : vec3<f32> = posC_0 - posA_0;
    var q_0 : vec3<f32> = cross(rayDir_1, e2_0);
    var a_0 : f32 = dot(e1_0, q_0);
    if((abs(a_0)) == 0.0f)
    {
        return vec3<f32>(-1.0f, 0.0f, 0.0f);
    }
    var s_0 : vec3<f32> = (rayPos_1 - posA_0) / vec3<f32>(a_0);
    var r_0 : vec3<f32> = cross(s_0, e1_0);
    var b_0 : vec3<f32>;
    b_0[i32(0)] = dot(s_0, q_0);
    b_0[i32(1)] = dot(r_0, rayDir_1);
    b_0[i32(2)] = 1.0f - b_0.x - b_0.y;
    var _S6 : bool;
    if((b_0.x) < 0.0f)
    {
        _S6 = true;
    }
    else
    {
        _S6 = (b_0.y) < 0.0f;
    }
    if(_S6)
    {
        _S6 = true;
    }
    else
    {
        _S6 = (b_0.z) < 0.0f;
    }
    if(_S6)
    {
        return vec3<f32>(-1.0f, 0.0f, 0.0f);
    }
    return vec3<f32>(dot(e2_0, r_0), b_0.x, b_0.y);
}

fn AnyHit_0( payload_5 : ptr<function, Payload_0>,  attr_0 : BuiltInTriangleIntersectionAttributes_0,  _anyHitResult_0 : ptr<function, u32>,  _ray_0 : RayDesc_0,  _RayTCurrent_0 : f32,  _PrimitiveIndex_1 : u32)
{
    var _S7 : u32 = _PrimitiveIndex_1 * u32(3);
    var _S8 : u32 = _S7 + u32(1);
    var _S9 : u32 = _S7 + u32(2);
    var _S10 : f32 = attr_0.barycentrics_0.x;
    var _S11 : f32 = attr_0.barycentrics_0.y;
    var _S12 : vec2<u32> = vec2<u32>((vertexBuffer[_S7].uv_0 + vec2<f32>(_S10) * (vertexBuffer[_S8].uv_0 - vertexBuffer[_S7].uv_0) + vec2<f32>(_S11) * (vertexBuffer[_S9].uv_0 - vertexBuffer[_S7].uv_0)) * vec2<f32>(64.0f));
    if(((_S12.x / u32(4) + _S12.y / u32(4)) % u32(2)) == u32(0))
    {
        (*_anyHitResult_0) = u32(1);
        return;
    }
    var _S13 : vec3<f32> = vec3<f32>(0.5f);
    (*payload_5).color_0 = normalize(vertexBuffer[_S7].normal_0 + vec3<f32>(_S10) * (vertexBuffer[_S8].normal_0 - vertexBuffer[_S7].normal_0) + vec3<f32>(_S11) * (vertexBuffer[_S9].normal_0 - vertexBuffer[_S7].normal_0)) * _S13 + _S13;
    return;
}

fn RunAnyHitShader_0( payload_6 : ptr<function, Payload_0>,  intersectionInfo_2 : ptr<function, BuiltInTriangleIntersectionAttributes_0>,  anyHitResult_0 : ptr<function, u32>,  Ray_1 : RayDesc_0,  minT_1 : f32,  primitiveIndex_1 : u32,  hitGroupIndex_2 : u32)
{
    switch(hitGroupIndex_2)
    {
    case u32(0), :
        {
            AnyHit_0(&((*payload_6)), (*intersectionInfo_2), &((*anyHitResult_0)), Ray_1, minT_1, primitiveIndex_1);
            break;
        }
    case default, :
        {
            break;
        }
    }
    return;
}

fn RayVsScene_AABBs_0( _S14 : u32,  _S15 : u32,  _S16 : u32,  _S17 : u32,  _S18 : u32,  _S19 : RayDesc_0,  _S20 : ptr<function, Payload_0>)
{
    var numVertsStride_0 : vec2<u32>;
    var _S21 : vec2<u32> = vec2<u32>(arrayLength(&scene), 48);
    numVertsStride_0[i32(0)] = _S21.x;
    numVertsStride_0[i32(1)] = _S21.y;
    var _S22 : u32 = numVertsStride_0.x / u32(6);
    var minTAndBary_0 : vec3<f32> = vec3<f32>(_S19.TMax_0, 0.0f, 0.0f);
    var hitIndex_1 : u32 = u32(4294967295);
    var intersectionInfo_3 : BuiltInTriangleIntersectionAttributes_0;
    intersectionInfo_3.barycentrics_0 = vec2<f32>(0.0f, 0.0f);
    var i_0 : u32 = u32(0);
    for(;;)
    {
        if(i_0 < _S22)
        {
        }
        else
        {
            break;
        }
        var _S23 : u32 = i_0 * u32(6);
        if((RayVsAABB_0(_S19.Origin_0, _S19.Direction_0, vec3<f32>(scene[_S23].position_0.x, scene[_S23 + u32(1)].position_0.x, scene[_S23 + u32(2)].position_0.x), vec3<f32>(scene[_S23 + u32(3)].position_0.x, scene[_S23 + u32(4)].position_0.x, scene[_S23 + u32(5)].position_0.x))) >= 0.0f)
        {
            var _S24 : f32 = minTAndBary_0[i32(0)];
            RunIntersectionShader_0(_S19, &(_S24), i_0, &(hitIndex_1), _S16, &((*_S20)), &(intersectionInfo_3));
            minTAndBary_0[i32(0)] = _S24;
        }
        i_0 = i_0 + u32(1);
    }
    if(hitIndex_1 == u32(4294967295))
    {
        RunMissShader_0(&((*_S20)), _S18);
    }
    else
    {
        intersectionInfo_3.barycentrics_0 = minTAndBary_0.yz;
        RunClosestHitShader_0(&((*_S20)), &(intersectionInfo_3), _S16, hitIndex_1);
    }
    return;
}

fn RayVsScene_Triangles_0( _S25 : u32,  _S26 : u32,  _S27 : u32,  _S28 : u32,  _S29 : u32,  _S30 : RayDesc_0,  _S31 : ptr<function, Payload_0>)
{
    var numVertsStride_1 : vec2<u32>;
    var _S32 : vec2<u32> = vec2<u32>(arrayLength(&scene), 48);
    numVertsStride_1[i32(0)] = _S32.x;
    numVertsStride_1[i32(1)] = _S32.y;
    var _S33 : u32 = numVertsStride_1.x / u32(3);
    var _S34 : vec3<f32> = vec3<f32>(_S30.TMax_0, 0.0f, 0.0f);
    var intersectionInfo_4 : BuiltInTriangleIntersectionAttributes_0;
    intersectionInfo_4.barycentrics_0 = vec2<f32>(0.0f, 0.0f);
    var minTAndBary_1 : vec3<f32> = _S34;
    var hitIndex_2 : i32 = i32(-1);
    var i_1 : u32 = u32(0);
    for(;;)
    {
        if(i_1 < _S33)
        {
        }
        else
        {
            break;
        }
        var _S35 : u32 = i_1 * u32(3);
        var hitTAndBary_0 : vec3<f32> = RayVsTriangle_0(_S30.Origin_0, _S30.Direction_0, scene[_S35].position_0, scene[_S35 + u32(1)].position_0, scene[_S35 + u32(2)].position_0);
        var _S36 : f32 = hitTAndBary_0.x;
        var _S37 : bool;
        if(_S36 >= (_S30.TMin_0))
        {
            _S37 = _S36 <= (minTAndBary_1.x);
        }
        else
        {
            _S37 = false;
        }
        var minTAndBary_2 : vec3<f32>;
        var hitIndex_3 : i32;
        if(_S37)
        {
            intersectionInfo_4.barycentrics_0 = hitTAndBary_0.yz;
            var anyHitResult_1 : u32 = u32(0);
            RunAnyHitShader_0(&((*_S31)), &(intersectionInfo_4), &(anyHitResult_1), _S30, _S36, i_1, _S27);
            if(anyHitResult_1 == u32(1))
            {
                i_1 = i_1 + u32(1);
                continue;
            }
            var _S38 : i32 = i32(i_1);
            if(anyHitResult_1 == u32(2))
            {
                hitIndex_2 = _S38;
                minTAndBary_1 = hitTAndBary_0;
                break;
            }
            minTAndBary_2 = hitTAndBary_0;
            hitIndex_3 = _S38;
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
        RunMissShader_0(&((*_S31)), _S29);
    }
    else
    {
        intersectionInfo_4.barycentrics_0 = minTAndBary_1.yz;
        RunClosestHitShader_0(&((*_S31)), &(intersectionInfo_4), _S27, u32(hitIndex_2));
    }
    return;
}

fn RayVsScene_0( _S39 : u32,  _S40 : u32,  _S41 : u32,  _S42 : u32,  _S43 : u32,  _S44 : RayDesc_0,  _S45 : ptr<function, Payload_0>)
{
    var _S46 : bool = SCENE_IS_AABBS();
    if(_S46)
    {
        RayVsScene_AABBs_0(_S39, _S40, _S41, _S42, _S43, _S44, &((*_S45)));
    }
    else
    {
        RayVsScene_Triangles_0(_S39, _S40, _S41, _S42, _S43, _S44, &((*_S45)));
    }
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn RayGen(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_1 : vec2<u32> = DTid_0.xy;
    var _S47 : vec2<f32> = (vec2<f32>(px_1) + vec2<f32>(0.5f)) / vec2<f32>(_RayGenCB._dispatchSize_RayGen_0.xy) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S47;
    screenPos_0[i32(1)] = - _S47.y;
    var _S48 : vec4<f32> = (((mat4x4<f32>(_RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, _RayGenCB.depthNearPlane_0, 1.0f))));
    var world_0 : vec4<f32> = _S48;
    var _S49 : vec3<f32> = _S48.xyz / vec3<f32>(_S48.w);
    world_0.x = _S49.x;
    world_0.y = _S49.y;
    world_0.z = _S49.z;
    var ray_0 : RayDesc_0;
    ray_0.Origin_0 = _RayGenCB.CameraPos_0;
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 1000.0f;
    ray_0.Direction_0 = normalize(world_0.xyz - ray_0.Origin_0);
    const _S50 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const _S51 : vec2<u32> = vec2<u32>(u32(0), u32(0));
    var payload_7 : Payload_0;
    payload_7.hit_0 = false;
    payload_7.color_0 = _S50;
    payload_7.px_0 = _S51;
    payload_7.px_0 = px_1;
    RayVsScene_0(u32(0), u32(255), u32(0), u32(0), u32(0), ray_0, &(payload_7));
    var _S52 : vec3<f32>;
    if(payload_7.hit_0)
    {
        _S52 = payload_7.color_0;
    }
    else
    {
        _S52 = vec3<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f);
    }
    textureStore((output), (px_1), (vec4<f32>(_S52, 1.0f)));
    return;
}

`;

// Shader code for BVH Variation of RTRayGen shader "RayGen", node "DoRT"
static ShaderCode_DoRT_RayGen_BVH = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_RayGenCB_std140_0
{
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) _padding0_0 : f32,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) _dispatchSize_RayGen_0 : vec3<u32>,
    @align(4) depthNearPlane_0 : f32,
};

@binding(4) @group(0) var<uniform> _RayGenCB : Struct_RayGenCB_std140_0;
struct Struct_VertexBuffer_std430_0
{
    @align(16) position_0 : vec3<f32>,
    @align(4) _padding0_1 : f32,
    @align(16) normal_0 : vec3<f32>,
    @align(4) _padding1_0 : f32,
    @align(16) uv_0 : vec2<f32>,
    @align(8) materialId_0 : u32,
    @align(4) _padding2_0 : f32,
};

@binding(2) @group(0) var<storage, read> vertexBuffer : array<Struct_VertexBuffer_std430_0>;

@binding(1) @group(0) var<storage, read> scene : array<u32>;

@binding(0) @group(0) var output : texture_storage_2d</*(output_format)*/, write>;

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
     color_0 : vec3<f32>,
     px_0 : vec2<u32>,
};

struct BuiltInTriangleIntersectionAttributes_0
{
     barycentrics_0 : vec2<f32>,
};

struct RayDesc_0
{
     Origin_0 : vec3<f32>,
     TMin_0 : f32,
     Direction_0 : vec3<f32>,
     TMax_0 : f32,
};

fn AnyHit_0( payload_0 : ptr<function, Payload_0>,  attr_0 : BuiltInTriangleIntersectionAttributes_0,  _anyHitResult_0 : ptr<function, u32>,  _ray_0 : RayDesc_0,  _RayTCurrent_0 : f32,  _PrimitiveIndex_0 : u32)
{
    var _S3 : u32 = _PrimitiveIndex_0 * u32(3);
    var _S4 : u32 = _S3 + u32(1);
    var _S5 : u32 = _S3 + u32(2);
    var _S6 : f32 = attr_0.barycentrics_0.x;
    var _S7 : f32 = attr_0.barycentrics_0.y;
    var _S8 : vec2<u32> = vec2<u32>((vertexBuffer[_S3].uv_0 + vec2<f32>(_S6) * (vertexBuffer[_S4].uv_0 - vertexBuffer[_S3].uv_0) + vec2<f32>(_S7) * (vertexBuffer[_S5].uv_0 - vertexBuffer[_S3].uv_0)) * vec2<f32>(64.0f));
    if(((_S8.x / u32(4) + _S8.y / u32(4)) % u32(2)) == u32(0))
    {
        (*_anyHitResult_0) = u32(1);
        return;
    }
    var _S9 : vec3<f32> = vec3<f32>(0.5f);
    (*payload_0).color_0 = normalize(vertexBuffer[_S3].normal_0 + vec3<f32>(_S6) * (vertexBuffer[_S4].normal_0 - vertexBuffer[_S3].normal_0) + vec3<f32>(_S7) * (vertexBuffer[_S5].normal_0 - vertexBuffer[_S3].normal_0)) * _S9 + _S9;
    return;
}

fn RunAnyHitShader_0( payload_1 : ptr<function, Payload_0>,  intersectionInfo_0 : ptr<function, BuiltInTriangleIntersectionAttributes_0>,  anyHitResult_0 : ptr<function, u32>,  Ray_0 : RayDesc_0,  minT_0 : f32,  primitiveIndex_0 : u32,  hitGroupIndex_0 : u32)
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

fn ClosestHit_0( payload_4 : ptr<function, Payload_0>,  intersection_0 : BuiltInTriangleIntersectionAttributes_0)
{
    (*payload_4).hit_0 = true;
    return;
}

fn RunClosestHitShader_0( payload_5 : ptr<function, Payload_0>,  intersectionInfo_1 : ptr<function, BuiltInTriangleIntersectionAttributes_0>,  hitGroupIndex_1 : u32,  _PrimitiveIndex_1 : u32)
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

fn RayVsScene_BVH_0( _S10 : u32,  _S11 : u32,  _S12 : u32,  _S13 : u32,  _S14 : u32,  _S15 : RayDesc_0,  _S16 : ptr<function, Payload_0>)
{
    var intersectionInfo_2 : BuiltInTriangleIntersectionAttributes_0;
    intersectionInfo_2.barycentrics_0 = vec2<f32>(0.0f, 0.0f);
    var _S17 : vec3<f32> = rcp_1(_S15.Direction_0);
    var altNodeOffset_0 : u32 = u32(2) + scene[i32(0)] / u32(4);
    var _S18 : u32 = altNodeOffset_0 + scene[i32(1)] / u32(4);
    var hit_1 : vec4<f32>;
    hit_1[i32(0)] = _S15.TMax_0;
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
        var _S19 : u32 = altNodeOffset_0 + node_0 * u32(16);
        var _S20 : f32 = (bitcast<f32>((scene[_S19])));
        var _S21 : f32 = (bitcast<f32>((scene[_S19 + u32(1)])));
        var _S22 : f32 = (bitcast<f32>((scene[_S19 + u32(2)])));
        var _S23 : u32 = _S19 + u32(4);
        var _S24 : f32 = (bitcast<f32>((scene[_S23])));
        var _S25 : f32 = (bitcast<f32>((scene[_S23 + u32(1)])));
        var _S26 : f32 = (bitcast<f32>((scene[_S23 + u32(2)])));
        var _S27 : u32 = _S19 + u32(8);
        var _S28 : f32 = (bitcast<f32>((scene[_S27])));
        var _S29 : f32 = (bitcast<f32>((scene[_S27 + u32(1)])));
        var _S30 : f32 = (bitcast<f32>((scene[_S27 + u32(2)])));
        var _S31 : u32 = _S19 + u32(12);
        var _S32 : f32 = (bitcast<f32>((scene[_S31])));
        var _S33 : f32 = (bitcast<f32>((scene[_S31 + u32(1)])));
        var _S34 : f32 = (bitcast<f32>((scene[_S31 + u32(2)])));
        var triCount_0 : u32 = scene[_S19 + u32(11)];
        var i_0 : u32;
        if(triCount_0 > u32(0))
        {
            var _S35 : u32 = scene[_S19 + u32(15)];
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
                var triIdx_0 : u32 = scene[_S18 + _S35 + i_0];
                var _S36 : u32 = u32(3) * triIdx_0;
                var _S37 : u32 = u32(2) + _S36 * u32(4);
                var _S38 : f32 = (bitcast<f32>((scene[_S37])));
                var _S39 : f32 = (bitcast<f32>((scene[_S37 + u32(1)])));
                var _S40 : f32 = (bitcast<f32>((scene[_S37 + u32(2)])));
                var _S41 : vec4<f32> = vec4<f32>(_S38, _S39, _S40, (bitcast<f32>((scene[_S37 + u32(3)]))));
                var _S42 : u32 = u32(2) + (_S36 + u32(1)) * u32(4);
                var _S43 : u32 = u32(2) + (_S36 + u32(2)) * u32(4);
                var _S44 : vec3<f32> = (vec4<f32>((bitcast<f32>((scene[_S43]))), (bitcast<f32>((scene[_S43 + u32(1)]))), (bitcast<f32>((scene[_S43 + u32(2)]))), (bitcast<f32>((scene[_S43 + u32(3)])))) - _S41).xyz;
                var h_0 : vec3<f32> = cross(_S15.Direction_0, _S44);
                var _S45 : vec3<f32> = (vec4<f32>((bitcast<f32>((scene[_S42]))), (bitcast<f32>((scene[_S42 + u32(1)]))), (bitcast<f32>((scene[_S42 + u32(2)]))), (bitcast<f32>((scene[_S42 + u32(3)])))) - _S41).xyz;
                var a_0 : f32 = dot(_S45, h_0);
                var forceExitSearch_2 : bool;
                if((abs(a_0)) < 1.00000001168609742e-07f)
                {
                    forceExitSearch_2 = forceExitSearch_1;
                    var _S46 : u32 = i_0 + u32(1);
                    forceExitSearch_1 = forceExitSearch_2;
                    i_0 = _S46;
                    continue;
                }
                var f_0 : f32 = 1.0f / a_0;
                var s_0 : vec3<f32> = _S15.Origin_0 - vec3<f32>(_S38, _S39, _S40);
                var u_0 : f32 = f_0 * dot(s_0, h_0);
                var q_0 : vec3<f32> = cross(s_0, _S45);
                var v_2 : f32 = f_0 * dot(_S15.Direction_0, q_0);
                var _S47 : bool;
                if(u_0 < 0.0f)
                {
                    _S47 = true;
                }
                else
                {
                    _S47 = v_2 < 0.0f;
                }
                var _S48 : bool;
                if(_S47)
                {
                    _S48 = true;
                }
                else
                {
                    _S48 = (u_0 + v_2) > 1.0f;
                }
                if(_S48)
                {
                    forceExitSearch_2 = forceExitSearch_1;
                    var _S46 : u32 = i_0 + u32(1);
                    forceExitSearch_1 = forceExitSearch_2;
                    i_0 = _S46;
                    continue;
                }
                var d_0 : f32 = f_0 * dot(_S44, q_0);
                var _S49 : bool;
                if(d_0 > 0.0f)
                {
                    _S49 = d_0 < (hit_1.x);
                }
                else
                {
                    _S49 = false;
                }
                if(_S49)
                {
                    var anyHitResult_1 : u32 = u32(0);
                    RunAnyHitShader_0(&((*_S16)), &(intersectionInfo_2), &(anyHitResult_1), _S15, d_0, triIdx_0, _S12);
                    if(anyHitResult_1 == u32(1))
                    {
                        forceExitSearch_2 = forceExitSearch_1;
                        var _S46 : u32 = i_0 + u32(1);
                        forceExitSearch_1 = forceExitSearch_2;
                        i_0 = _S46;
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
                var _S46 : u32 = i_0 + u32(1);
                forceExitSearch_1 = forceExitSearch_2;
                i_0 = _S46;
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
        var left_0 : u32 = scene[_S19 + u32(3)];
        var right_0 : u32 = scene[_S19 + u32(7)];
        var t1a_0 : vec3<f32> = (vec3<f32>(_S20, _S21, _S22) - _S15.Origin_0) * _S17;
        var t2a_0 : vec3<f32> = (vec3<f32>(_S24, _S25, _S26) - _S15.Origin_0) * _S17;
        var t1b_0 : vec3<f32> = (vec3<f32>(_S28, _S29, _S30) - _S15.Origin_0) * _S17;
        var t2b_0 : vec3<f32> = (vec3<f32>(_S32, _S33, _S34) - _S15.Origin_0) * _S17;
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
            var _S50 : u32 = u32(0);
            var _S51 : u32 = u32(0);
            if(stackPtr_0 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_2 : u32 = stackPtr_0 - u32(1);
                _S51 = stackPtr_2;
                _S50 = stack_0[stackPtr_2];
            }
            node_0 = _S50;
            stackPtr_0 = _S51;
        }
        else
        {
            var stackPtr_3 : u32;
            if(dist2_1 != 1.00000001504746622e+30f)
            {
                var _S52 : u32 = stackPtr_0 + u32(1);
                stack_0[stackPtr_0] = right_1;
                stackPtr_3 = _S52;
            }
            else
            {
                stackPtr_3 = stackPtr_0;
            }
            node_0 = i_0;
            stackPtr_0 = stackPtr_3;
        }
    }
    if((hit_1.x) == (_S15.TMax_0))
    {
        RunMissShader_0(&((*_S16)), _S14);
    }
    else
    {
        intersectionInfo_2.barycentrics_0 = hit_1.yz;
        RunClosestHitShader_0(&((*_S16)), &(intersectionInfo_2), _S12, (bitcast<u32>((hit_1.w))));
    }
    return;
}

fn RayVsScene_0( _S53 : u32,  _S54 : u32,  _S55 : u32,  _S56 : u32,  _S57 : u32,  _S58 : RayDesc_0,  _S59 : ptr<function, Payload_0>)
{
    RayVsScene_BVH_0(_S53, _S54, _S55, _S56, _S57, _S58, &((*_S59)));
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn RayGen(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_1 : vec2<u32> = DTid_0.xy;
    var _S60 : vec2<f32> = (vec2<f32>(px_1) + vec2<f32>(0.5f)) / vec2<f32>(_RayGenCB._dispatchSize_RayGen_0.xy) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S60;
    screenPos_0[i32(1)] = - _S60.y;
    var _S61 : vec4<f32> = (((mat4x4<f32>(_RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _RayGenCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, _RayGenCB.depthNearPlane_0, 1.0f))));
    var world_0 : vec4<f32> = _S61;
    var _S62 : vec3<f32> = _S61.xyz / vec3<f32>(_S61.w);
    world_0.x = _S62.x;
    world_0.y = _S62.y;
    world_0.z = _S62.z;
    var ray_0 : RayDesc_0;
    ray_0.Origin_0 = _RayGenCB.CameraPos_0;
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 1000.0f;
    ray_0.Direction_0 = normalize(world_0.xyz - ray_0.Origin_0);
    const _S63 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const _S64 : vec2<u32> = vec2<u32>(u32(0), u32(0));
    var payload_6 : Payload_0;
    payload_6.hit_0 = false;
    payload_6.color_0 = _S63;
    payload_6.px_0 = _S64;
    payload_6.px_0 = px_1;
    RayVsScene_0(u32(0), u32(255), u32(0), u32(0), u32(0), ray_0, &(payload_6));
    var _S65 : vec3<f32>;
    if(payload_6.hit_0)
    {
        _S65 = payload_6.color_0;
    }
    else
    {
        _S65 = vec3<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f);
    }
    textureStore((output), (px_1), (vec4<f32>(_S65, 1.0f)));
    return;
}

`;

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct VertexBuffer
static StructVertexBufferAttributes_VertexBuffer =
[
    // position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // normal
    {
        format: "float32x3",
        offset: 16,
        shaderLocation: 1,
    },
    // uv
    {
        format: "float32x2",
        offset: 32,
        shaderLocation: 2,
    },
    // materialId
    {
        format: "uint32",
        offset: 40,
        shaderLocation: 3,
    },
];

// Vertex buffer attributes for struct VertexBuffer_Unpadded
static StructVertexBufferAttributes_VertexBuffer_Unpadded =
[
    // position
    {
        format: "float32x3",
        offset: 0,
        shaderLocation: 0,
    },
    // normal
    {
        format: "float32x3",
        offset: 12,
        shaderLocation: 1,
    },
    // uv
    {
        format: "float32x2",
        offset: 24,
        shaderLocation: 2,
    },
    // materialId
    {
        format: "uint32",
        offset: 32,
        shaderLocation: 3,
    },
];

// -------------------- Private Members

// Texture DoRT_output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_DoRT_output_ReadOnly = null;
texture_DoRT_output_ReadOnly_size = [0, 0, 0];
texture_DoRT_output_ReadOnly_format = "";
texture_DoRT_output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _RayGenCB
constantBuffer__RayGenCB = null;
constantBuffer__RayGenCB_size = 96;
constantBuffer__RayGenCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// RayGen Shader DoRT
Hash_RayGen_DoRT = 0;
ShaderModule_RayGen_DoRT = null;
BindGroupLayout_RayGen_DoRT = null;
PipelineLayout_RayGen_DoRT = null;
Pipeline_RayGen_DoRT = null;

// -------------------- Imported Members

// Buffer Scene
buffer_Scene = null;
buffer_Scene_count = 0;
buffer_Scene_stride = 0;
buffer_Scene_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;
buffer_Scene_isAABBs = false; // For raytracing: if true, treated as AABBs for an intersection shader, else treated as triangles
buffer_Scene_isBVH = false; // For raytracing: if true, treated as a BVH of triangles

// Buffer SceneVB
buffer_SceneVB = null;
buffer_SceneVB_count = 0;
buffer_SceneVB_stride = 0;
buffer_SceneVB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Exported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_depthNearPlane = 0.000000;
variableDefault_depthNearPlane = 0.000000;
variableChanged_depthNearPlane = false;
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
variable_CameraChanged = false;
variableDefault_CameraChanged = false;
variableChanged_CameraChanged = false;

// -------------------- Private Variables

variable__dispatchSize_RayGen = [ 0,0,0 ];
variableDefault__dispatchSize_RayGen = [ 0,0,0 ];
variableChanged__dispatchSize_RayGen = [ false, false, false ];
// -------------------- Structs

static StructOffsets_VertexBuffer =
{
    position_0: 0,
    position_1: 4,
    position_2: 8,
    _padding0: 12,
    normal_0: 16,
    normal_1: 20,
    normal_2: 24,
    _padding1: 28,
    uv_0: 32,
    uv_1: 36,
    materialId: 40,
    _padding2: 44,
    _size: 48,
}

static StructOffsets__RayGenCB =
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
    _dispatchSize_RayGen_0: 80,
    _dispatchSize_RayGen_1: 84,
    _dispatchSize_RayGen_2: 88,
    depthNearPlane: 92,
    _size: 96,
}

static StructOffsets_VertexBuffer_Unpadded =
{
    position_0: 0,
    position_1: 4,
    position_2: 8,
    normal_0: 12,
    normal_1: 16,
    normal_2: 20,
    uv_0: 24,
    uv_1: 28,
    materialId: 32,
    _size: 36,
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

    // Validate buffer SceneVB
    if (this.buffer_SceneVB === null)
    {
        Shared.LogError("Imported resource buffer_SceneVB was not provided");
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
            Math.floor(((parseInt(baseSize[0]) + 0) * 512) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 512) / 1) + 0,
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
                label: "texture AnyHit.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture DoRT_output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_DoRT_output_ReadOnly !== null && (this.texture_DoRT_output_ReadOnly_format != desiredFormat || this.texture_DoRT_output_ReadOnly_size[0] != desiredSize[0] || this.texture_DoRT_output_ReadOnly_size[1] != desiredSize[1] || this.texture_DoRT_output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_DoRT_output_ReadOnly.destroy();
            this.texture_DoRT_output_ReadOnly = null;
        }

        if (this.texture_DoRT_output_ReadOnly === null)
        {
            this.texture_DoRT_output_ReadOnly_size = desiredSize.slice();
            this.texture_DoRT_output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_DoRT_output_ReadOnly_format))
                viewFormats.push(this.texture_DoRT_output_ReadOnly_format);

            this.texture_DoRT_output_ReadOnly = device.createTexture({
                label: "texture AnyHit.DoRT_output_ReadOnly",
                size: this.texture_DoRT_output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_DoRT_output_ReadOnly_format),
                usage: this.texture_DoRT_output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create raygen shader DoRT
    {
        const bindGroupEntries =
        [
            {
                // output
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // scene
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // vertexBuffer
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // outputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_DoRT_output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _RayGenCB
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        hashString = hashString + this.buffer_Scene_isBVH.toString();
        const newHash = hashString.hashCode();

        if (this.ShaderModule_RayGen_DoRT === null || newHash !== this.Hash_RayGen_DoRT)
        {
            this.Hash_RayGen_DoRT = newHash;

            let shaderCode = this.buffer_Scene_isBVH
                ? class_AnyHit.ShaderCode_DoRT_RayGen_BVH
                : class_AnyHit.ShaderCode_DoRT_RayGen_NonBVH
            ;
            shaderCode = "fn SCENE_IS_AABBS() -> bool { return " + this.buffer_Scene_isAABBs.toString() + "; }\n" + shaderCode;
            shaderCode = shaderCode.replace("/*(output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(outputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_DoRT_output_ReadOnly_format));

            this.ShaderModule_RayGen_DoRT = device.createShaderModule({ code: shaderCode, label: "RayGen Shader DoRT"});
            this.BindGroupLayout_RayGen_DoRT = device.createBindGroupLayout({
                label: "RayGen Bind Group Layout DoRT",
                entries: bindGroupEntries
            });

            this.PipelineLayout_RayGen_DoRT = device.createPipelineLayout({
                label: "RayGen Pipeline Layout DoRT",
                bindGroupLayouts: [this.BindGroupLayout_RayGen_DoRT],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_RayGen_DoRT = device.createComputePipeline({
                    label: "RayGen Pipeline DoRT",
                    layout: this.PipelineLayout_RayGen_DoRT,
                    compute: {
                        module: this.ShaderModule_RayGen_DoRT,
                        entryPoint: "RayGen",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("DoRT");

                device.createComputePipelineAsync({
                    label: "RayGen Pipeline DoRT",
                    layout: this.PipelineLayout_RayGen_DoRT,
                    compute: {
                        module: this.ShaderModule_RayGen_DoRT,
                        entryPoint: "RayGen",
                    }
                }).then( handle => { this.Pipeline_RayGen_DoRT = handle; this.loadingPromises.delete("DoRT"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("AnyHit.Output");

    encoder.popDebugGroup(); // "AnyHit.Output"

    encoder.pushDebugGroup("AnyHit.Scene");

    encoder.popDebugGroup(); // "AnyHit.Scene"

    encoder.pushDebugGroup("AnyHit.SceneVB");

    encoder.popDebugGroup(); // "AnyHit.SceneVB"

    encoder.pushDebugGroup("AnyHit.DoRT_output_ReadOnly");

    encoder.popDebugGroup(); // "AnyHit.DoRT_output_ReadOnly"

    encoder.pushDebugGroup("AnyHit.Copy_DoRT_output");

    // Copy texture Output to texture DoRT_output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_DoRT_output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_DoRT_output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "AnyHit.Copy_DoRT_output"

    encoder.pushDebugGroup("AnyHit._RayGenCB");

    // Create constant buffer _RayGenCB
    if (this.constantBuffer__RayGenCB === null)
    {
        this.constantBuffer__RayGenCB = device.createBuffer({
            label: "AnyHit._RayGenCB",
            size: Shared.Align(16, this.constructor.StructOffsets__RayGenCB._size),
            usage: this.constantBuffer__RayGenCB_usageFlags,
        });
    }

    // Upload values to constant buffer _RayGenCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__RayGenCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.CameraPos_0, this.variable_CameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.CameraPos_1, this.variable_CameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.CameraPos_2, this.variable_CameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_0, this.variable_InvViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_1, this.variable_InvViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_2, this.variable_InvViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_3, this.variable_InvViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_4, this.variable_InvViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_5, this.variable_InvViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_6, this.variable_InvViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_7, this.variable_InvViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_8, this.variable_InvViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_9, this.variable_InvViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_10, this.variable_InvViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_11, this.variable_InvViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_12, this.variable_InvViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_13, this.variable_InvViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_14, this.variable_InvViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.InvViewProjMtx_15, this.variable_InvViewProjMtx[15], true);
        view.setUint32(this.constructor.StructOffsets__RayGenCB._dispatchSize_RayGen_0, this.variable__dispatchSize_RayGen[0], true);
        view.setUint32(this.constructor.StructOffsets__RayGenCB._dispatchSize_RayGen_1, this.variable__dispatchSize_RayGen[1], true);
        view.setUint32(this.constructor.StructOffsets__RayGenCB._dispatchSize_RayGen_2, this.variable__dispatchSize_RayGen[2], true);
        view.setFloat32(this.constructor.StructOffsets__RayGenCB.depthNearPlane, this.variable_depthNearPlane, true);
        device.queue.writeBuffer(this.constantBuffer__RayGenCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "AnyHit._RayGenCB"

    encoder.pushDebugGroup("AnyHit.DoRT");

    // Run raygen shader DoRT
    {
        const bindGroup = device.createBindGroup({
            label: "RayGen Bind Group DoRT",
            layout: this.BindGroupLayout_RayGen_DoRT,
            entries: [
                {
                    // output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // scene
                    binding: 1,
                    resource: { buffer: this.buffer_Scene }
                },
                {
                    // vertexBuffer
                    binding: 2,
                    resource: { buffer: this.buffer_SceneVB }
                },
                {
                    // outputReadOnly
                    binding: 3,
                    resource: this.texture_DoRT_output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _RayGenCB
                    binding: 4,
                    resource: { buffer: this.constantBuffer__RayGenCB }
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
            device.queue.writeBuffer(this.constantBuffer__RayGenCB, this.constructor.StructOffsets__RayGenCB._dispatchSize_RayGen_0, specialVariablesBuffer);
        }

        if (this.Pipeline_RayGen_DoRT !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_RayGen_DoRT);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "AnyHit.DoRT"

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

var AnyHit = new class_AnyHit;

export default AnyHit;
