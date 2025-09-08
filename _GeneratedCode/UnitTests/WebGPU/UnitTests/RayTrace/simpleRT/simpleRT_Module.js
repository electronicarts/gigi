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

class class_simpleRT
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Non BVH Variation of RTRayGen shader "SimpleRTRayGen", node "DoRT"
static ShaderCode_DoRT_SimpleRTRayGen_NonBVH = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_SimpleRTRayGenCB_std140_0
{
    @align(16) _dispatchSize_SimpleRTRayGen_0 : vec3<u32>,
    @align(4) _padding0_0 : f32,
    @align(16) cameraPos_0 : vec3<f32>,
    @align(4) _padding1_0 : f32,
    @align(16) clipToWorld_0 : _MatrixStorage_float4x4std140_0,
    @align(16) depthNearPlane_0 : f32,
    @align(4) _padding2_0 : f32,
    @align(8) _padding3_0 : f32,
    @align(4) _padding4_0 : f32,
    @align(16) hitColor_0 : vec3<f32>,
    @align(4) _padding5_0 : f32,
    @align(16) missColor_0 : vec3<f32>,
    @align(4) _padding6_0 : f32,
};

@binding(3) @group(0) var<uniform> _SimpleRTRayGenCB : Struct_SimpleRTRayGenCB_std140_0;
struct Struct_VertexBuffer_std430_0
{
    @align(16) Color_0 : vec3<f32>,
    @align(4) _padding0_1 : f32,
    @align(16) Position_0 : vec3<f32>,
    @align(4) _padding1_1 : f32,
};

@binding(1) @group(0) var<storage, read> g_scene : array<Struct_VertexBuffer_std430_0>;

@binding(0) @group(0) var g_texture : texture_storage_2d</*(g_texture_format)*/, write>;

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
     RGB_0 : vec3<f32>,
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
    (*payload_1).RGB_0 = _SimpleRTRayGenCB.missColor_0;
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
    (*payload_3).RGB_0 = _SimpleRTRayGenCB.hitColor_0;
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

fn RunAnyHitShader_0( payload_5 : ptr<function, Payload_0>,  intersectionInfo_2 : ptr<function, BuiltInTriangleIntersectionAttributes_0>,  anyHitResult_0 : ptr<function, u32>,  Ray_1 : RayDesc_0,  minT_1 : f32,  primitiveIndex_1 : u32,  hitGroupIndex_2 : u32)
{
    return;
}

fn RayVsScene_AABBs_0( _S7 : u32,  _S8 : u32,  _S9 : u32,  _S10 : u32,  _S11 : u32,  _S12 : RayDesc_0,  _S13 : ptr<function, Payload_0>)
{
    var numVertsStride_0 : vec2<u32>;
    var _S14 : vec2<u32> = vec2<u32>(arrayLength(&g_scene), 32);
    numVertsStride_0[i32(0)] = _S14.x;
    numVertsStride_0[i32(1)] = _S14.y;
    var _S15 : u32 = numVertsStride_0.x / u32(6);
    var minTAndBary_0 : vec3<f32> = vec3<f32>(_S12.TMax_0, 0.0f, 0.0f);
    var hitIndex_1 : u32 = u32(4294967295);
    var intersectionInfo_3 : BuiltInTriangleIntersectionAttributes_0;
    intersectionInfo_3.barycentrics_0 = vec2<f32>(0.0f, 0.0f);
    var i_0 : u32 = u32(0);
    for(;;)
    {
        if(i_0 < _S15)
        {
        }
        else
        {
            break;
        }
        var _S16 : u32 = i_0 * u32(6);
        if((RayVsAABB_0(_S12.Origin_0, _S12.Direction_0, vec3<f32>(g_scene[_S16].Position_0.x, g_scene[_S16 + u32(1)].Position_0.x, g_scene[_S16 + u32(2)].Position_0.x), vec3<f32>(g_scene[_S16 + u32(3)].Position_0.x, g_scene[_S16 + u32(4)].Position_0.x, g_scene[_S16 + u32(5)].Position_0.x))) >= 0.0f)
        {
            var _S17 : f32 = minTAndBary_0[i32(0)];
            RunIntersectionShader_0(_S12, &(_S17), i_0, &(hitIndex_1), _S9, &((*_S13)), &(intersectionInfo_3));
            minTAndBary_0[i32(0)] = _S17;
        }
        i_0 = i_0 + u32(1);
    }
    if(hitIndex_1 == u32(4294967295))
    {
        RunMissShader_0(&((*_S13)), _S11);
    }
    else
    {
        intersectionInfo_3.barycentrics_0 = minTAndBary_0.yz;
        RunClosestHitShader_0(&((*_S13)), &(intersectionInfo_3), _S9, hitIndex_1);
    }
    return;
}

fn RayVsScene_Triangles_0( _S18 : u32,  _S19 : u32,  _S20 : u32,  _S21 : u32,  _S22 : u32,  _S23 : RayDesc_0,  _S24 : ptr<function, Payload_0>)
{
    var numVertsStride_1 : vec2<u32>;
    var _S25 : vec2<u32> = vec2<u32>(arrayLength(&g_scene), 32);
    numVertsStride_1[i32(0)] = _S25.x;
    numVertsStride_1[i32(1)] = _S25.y;
    var _S26 : u32 = numVertsStride_1.x / u32(3);
    var _S27 : vec3<f32> = vec3<f32>(_S23.TMax_0, 0.0f, 0.0f);
    var intersectionInfo_4 : BuiltInTriangleIntersectionAttributes_0;
    intersectionInfo_4.barycentrics_0 = vec2<f32>(0.0f, 0.0f);
    var minTAndBary_1 : vec3<f32> = _S27;
    var hitIndex_2 : i32 = i32(-1);
    var i_1 : u32 = u32(0);
    for(;;)
    {
        if(i_1 < _S26)
        {
        }
        else
        {
            break;
        }
        var _S28 : u32 = i_1 * u32(3);
        var hitTAndBary_0 : vec3<f32> = RayVsTriangle_0(_S23.Origin_0, _S23.Direction_0, g_scene[_S28].Position_0, g_scene[_S28 + u32(1)].Position_0, g_scene[_S28 + u32(2)].Position_0);
        var _S29 : f32 = hitTAndBary_0.x;
        var _S30 : bool;
        if(_S29 >= (_S23.TMin_0))
        {
            _S30 = _S29 <= (minTAndBary_1.x);
        }
        else
        {
            _S30 = false;
        }
        var minTAndBary_2 : vec3<f32>;
        var hitIndex_3 : i32;
        if(_S30)
        {
            intersectionInfo_4.barycentrics_0 = hitTAndBary_0.yz;
            var anyHitResult_1 : u32 = u32(0);
            RunAnyHitShader_0(&((*_S24)), &(intersectionInfo_4), &(anyHitResult_1), _S23, _S29, i_1, _S20);
            if(anyHitResult_1 == u32(1))
            {
                i_1 = i_1 + u32(1);
                continue;
            }
            var _S31 : i32 = i32(i_1);
            if(anyHitResult_1 == u32(2))
            {
                hitIndex_2 = _S31;
                minTAndBary_1 = hitTAndBary_0;
                break;
            }
            minTAndBary_2 = hitTAndBary_0;
            hitIndex_3 = _S31;
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
        RunMissShader_0(&((*_S24)), _S22);
    }
    else
    {
        intersectionInfo_4.barycentrics_0 = minTAndBary_1.yz;
        RunClosestHitShader_0(&((*_S24)), &(intersectionInfo_4), _S20, u32(hitIndex_2));
    }
    return;
}

fn RayVsScene_0( _S32 : u32,  _S33 : u32,  _S34 : u32,  _S35 : u32,  _S36 : u32,  _S37 : RayDesc_0,  _S38 : ptr<function, Payload_0>)
{
    var _S39 : bool = SCENE_IS_AABBS();
    if(_S39)
    {
        RayVsScene_AABBs_0(_S32, _S33, _S34, _S35, _S36, _S37, &((*_S38)));
    }
    else
    {
        RayVsScene_Triangles_0(_S32, _S33, _S34, _S35, _S36, _S37, &((*_S38)));
    }
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn RayGen(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S40 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(_SimpleRTRayGenCB._dispatchSize_SimpleRTRayGen_0.xy) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S40;
    screenPos_0[i32(1)] = - _S40.y;
    var _S41 : vec4<f32> = (((mat4x4<f32>(_SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(3)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(3)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(3)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, _SimpleRTRayGenCB.depthNearPlane_0, 1.0f))));
    var world_0 : vec4<f32> = _S41;
    var _S42 : vec3<f32> = _S41.xyz / vec3<f32>(_S41.w);
    world_0.x = _S42.x;
    world_0.y = _S42.y;
    world_0.z = _S42.z;
    var ray_0 : RayDesc_0;
    ray_0.Origin_0 = _SimpleRTRayGenCB.cameraPos_0;
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 1000.0f;
    ray_0.Direction_0 = normalize(world_0.xyz - ray_0.Origin_0);
    var payload_6 : Payload_0;
    payload_6.RGB_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
    RayVsScene_0(u32(1), u32(255), u32(0), u32(0), u32(0), ray_0, &(payload_6));
    textureStore((g_texture), (px_0), (vec4<f32>(payload_6.RGB_0, 1.0f)));
    return;
}

`;

// Shader code for BVH Variation of RTRayGen shader "SimpleRTRayGen", node "DoRT"
static ShaderCode_DoRT_SimpleRTRayGen_BVH = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_SimpleRTRayGenCB_std140_0
{
    @align(16) _dispatchSize_SimpleRTRayGen_0 : vec3<u32>,
    @align(4) _padding0_0 : f32,
    @align(16) cameraPos_0 : vec3<f32>,
    @align(4) _padding1_0 : f32,
    @align(16) clipToWorld_0 : _MatrixStorage_float4x4std140_0,
    @align(16) depthNearPlane_0 : f32,
    @align(4) _padding2_0 : f32,
    @align(8) _padding3_0 : f32,
    @align(4) _padding4_0 : f32,
    @align(16) hitColor_0 : vec3<f32>,
    @align(4) _padding5_0 : f32,
    @align(16) missColor_0 : vec3<f32>,
    @align(4) _padding6_0 : f32,
};

@binding(3) @group(0) var<uniform> _SimpleRTRayGenCB : Struct_SimpleRTRayGenCB_std140_0;
@binding(1) @group(0) var<storage, read> g_scene : array<u32>;

@binding(0) @group(0) var g_texture : texture_storage_2d</*(g_texture_format)*/, write>;

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
     RGB_0 : vec3<f32>,
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

fn RunAnyHitShader_0( payload_0 : ptr<function, Payload_0>,  intersectionInfo_0 : ptr<function, BuiltInTriangleIntersectionAttributes_0>,  anyHitResult_0 : ptr<function, u32>,  Ray_0 : RayDesc_0,  minT_0 : f32,  primitiveIndex_0 : u32,  hitGroupIndex_0 : u32)
{
    return;
}

fn Miss_0( payload_1 : ptr<function, Payload_0>)
{
    (*payload_1).RGB_0 = _SimpleRTRayGenCB.missColor_0;
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
    (*payload_3).RGB_0 = _SimpleRTRayGenCB.hitColor_0;
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

fn RayVsScene_BVH_0( _S3 : u32,  _S4 : u32,  _S5 : u32,  _S6 : u32,  _S7 : u32,  _S8 : RayDesc_0,  _S9 : ptr<function, Payload_0>)
{
    var intersectionInfo_2 : BuiltInTriangleIntersectionAttributes_0;
    intersectionInfo_2.barycentrics_0 = vec2<f32>(0.0f, 0.0f);
    var _S10 : vec3<f32> = rcp_1(_S8.Direction_0);
    var altNodeOffset_0 : u32 = u32(2) + g_scene[i32(0)] / u32(4);
    var _S11 : u32 = altNodeOffset_0 + g_scene[i32(1)] / u32(4);
    var hit_0 : vec4<f32>;
    hit_0[i32(0)] = _S8.TMax_0;
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
        var _S12 : u32 = altNodeOffset_0 + node_0 * u32(16);
        var _S13 : f32 = (bitcast<f32>((g_scene[_S12])));
        var _S14 : f32 = (bitcast<f32>((g_scene[_S12 + u32(1)])));
        var _S15 : f32 = (bitcast<f32>((g_scene[_S12 + u32(2)])));
        var _S16 : u32 = _S12 + u32(4);
        var _S17 : f32 = (bitcast<f32>((g_scene[_S16])));
        var _S18 : f32 = (bitcast<f32>((g_scene[_S16 + u32(1)])));
        var _S19 : f32 = (bitcast<f32>((g_scene[_S16 + u32(2)])));
        var _S20 : u32 = _S12 + u32(8);
        var _S21 : f32 = (bitcast<f32>((g_scene[_S20])));
        var _S22 : f32 = (bitcast<f32>((g_scene[_S20 + u32(1)])));
        var _S23 : f32 = (bitcast<f32>((g_scene[_S20 + u32(2)])));
        var _S24 : u32 = _S12 + u32(12);
        var _S25 : f32 = (bitcast<f32>((g_scene[_S24])));
        var _S26 : f32 = (bitcast<f32>((g_scene[_S24 + u32(1)])));
        var _S27 : f32 = (bitcast<f32>((g_scene[_S24 + u32(2)])));
        var triCount_0 : u32 = g_scene[_S12 + u32(11)];
        var i_0 : u32;
        if(triCount_0 > u32(0))
        {
            var _S28 : u32 = g_scene[_S12 + u32(15)];
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
                var triIdx_0 : u32 = g_scene[_S11 + _S28 + i_0];
                var _S29 : u32 = u32(3) * triIdx_0;
                var _S30 : u32 = u32(2) + _S29 * u32(4);
                var _S31 : f32 = (bitcast<f32>((g_scene[_S30])));
                var _S32 : f32 = (bitcast<f32>((g_scene[_S30 + u32(1)])));
                var _S33 : f32 = (bitcast<f32>((g_scene[_S30 + u32(2)])));
                var _S34 : vec4<f32> = vec4<f32>(_S31, _S32, _S33, (bitcast<f32>((g_scene[_S30 + u32(3)]))));
                var _S35 : u32 = u32(2) + (_S29 + u32(1)) * u32(4);
                var _S36 : u32 = u32(2) + (_S29 + u32(2)) * u32(4);
                var _S37 : vec3<f32> = (vec4<f32>((bitcast<f32>((g_scene[_S36]))), (bitcast<f32>((g_scene[_S36 + u32(1)]))), (bitcast<f32>((g_scene[_S36 + u32(2)]))), (bitcast<f32>((g_scene[_S36 + u32(3)])))) - _S34).xyz;
                var h_0 : vec3<f32> = cross(_S8.Direction_0, _S37);
                var _S38 : vec3<f32> = (vec4<f32>((bitcast<f32>((g_scene[_S35]))), (bitcast<f32>((g_scene[_S35 + u32(1)]))), (bitcast<f32>((g_scene[_S35 + u32(2)]))), (bitcast<f32>((g_scene[_S35 + u32(3)])))) - _S34).xyz;
                var a_0 : f32 = dot(_S38, h_0);
                var forceExitSearch_2 : bool;
                if((abs(a_0)) < 1.00000001168609742e-07f)
                {
                    forceExitSearch_2 = forceExitSearch_1;
                    var _S39 : u32 = i_0 + u32(1);
                    forceExitSearch_1 = forceExitSearch_2;
                    i_0 = _S39;
                    continue;
                }
                var f_0 : f32 = 1.0f / a_0;
                var s_0 : vec3<f32> = _S8.Origin_0 - vec3<f32>(_S31, _S32, _S33);
                var u_0 : f32 = f_0 * dot(s_0, h_0);
                var q_0 : vec3<f32> = cross(s_0, _S38);
                var v_2 : f32 = f_0 * dot(_S8.Direction_0, q_0);
                var _S40 : bool;
                if(u_0 < 0.0f)
                {
                    _S40 = true;
                }
                else
                {
                    _S40 = v_2 < 0.0f;
                }
                var _S41 : bool;
                if(_S40)
                {
                    _S41 = true;
                }
                else
                {
                    _S41 = (u_0 + v_2) > 1.0f;
                }
                if(_S41)
                {
                    forceExitSearch_2 = forceExitSearch_1;
                    var _S39 : u32 = i_0 + u32(1);
                    forceExitSearch_1 = forceExitSearch_2;
                    i_0 = _S39;
                    continue;
                }
                var d_0 : f32 = f_0 * dot(_S37, q_0);
                var _S42 : bool;
                if(d_0 > 0.0f)
                {
                    _S42 = d_0 < (hit_0.x);
                }
                else
                {
                    _S42 = false;
                }
                if(_S42)
                {
                    var anyHitResult_1 : u32 = u32(0);
                    RunAnyHitShader_0(&((*_S9)), &(intersectionInfo_2), &(anyHitResult_1), _S8, d_0, triIdx_0, _S5);
                    if(anyHitResult_1 == u32(1))
                    {
                        forceExitSearch_2 = forceExitSearch_1;
                        var _S39 : u32 = i_0 + u32(1);
                        forceExitSearch_1 = forceExitSearch_2;
                        i_0 = _S39;
                        continue;
                    }
                    hit_0 = vec4<f32>(d_0, u_0, v_2, (bitcast<f32>((triIdx_0))));
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
                var _S39 : u32 = i_0 + u32(1);
                forceExitSearch_1 = forceExitSearch_2;
                i_0 = _S39;
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
        var left_0 : u32 = g_scene[_S12 + u32(3)];
        var right_0 : u32 = g_scene[_S12 + u32(7)];
        var t1a_0 : vec3<f32> = (vec3<f32>(_S13, _S14, _S15) - _S8.Origin_0) * _S10;
        var t2a_0 : vec3<f32> = (vec3<f32>(_S17, _S18, _S19) - _S8.Origin_0) * _S10;
        var t1b_0 : vec3<f32> = (vec3<f32>(_S21, _S22, _S23) - _S8.Origin_0) * _S10;
        var t2b_0 : vec3<f32> = (vec3<f32>(_S25, _S26, _S27) - _S8.Origin_0) * _S10;
        var minta_0 : vec3<f32> = min(t1a_0, t2a_0);
        var maxta_0 : vec3<f32> = max(t1a_0, t2a_0);
        var mintb_0 : vec3<f32> = min(t1b_0, t2b_0);
        var maxtb_0 : vec3<f32> = max(t1b_0, t2b_0);
        var tmina_0 : f32 = max(max(max(minta_0.x, minta_0.y), minta_0.z), 0.0f);
        var tminb_0 : f32 = max(max(max(mintb_0.x, mintb_0.y), mintb_0.z), 0.0f);
        var tmaxb_0 : f32 = min(min(min(maxtb_0.x, maxtb_0.y), maxtb_0.z), hit_0.x);
        var dist1_0 : f32;
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
            var _S43 : u32 = u32(0);
            var _S44 : u32 = u32(0);
            if(stackPtr_0 == u32(0))
            {
                break;
            }
            else
            {
                var stackPtr_2 : u32 = stackPtr_0 - u32(1);
                _S44 = stackPtr_2;
                _S43 = stack_0[stackPtr_2];
            }
            node_0 = _S43;
            stackPtr_0 = _S44;
        }
        else
        {
            var stackPtr_3 : u32;
            if(dist2_1 != 1.00000001504746622e+30f)
            {
                var _S45 : u32 = stackPtr_0 + u32(1);
                stack_0[stackPtr_0] = right_1;
                stackPtr_3 = _S45;
            }
            else
            {
                stackPtr_3 = stackPtr_0;
            }
            node_0 = i_0;
            stackPtr_0 = stackPtr_3;
        }
    }
    if((hit_0.x) == (_S8.TMax_0))
    {
        RunMissShader_0(&((*_S9)), _S7);
    }
    else
    {
        intersectionInfo_2.barycentrics_0 = hit_0.yz;
        RunClosestHitShader_0(&((*_S9)), &(intersectionInfo_2), _S5, (bitcast<u32>((hit_0.w))));
    }
    return;
}

fn RayVsScene_0( _S46 : u32,  _S47 : u32,  _S48 : u32,  _S49 : u32,  _S50 : u32,  _S51 : RayDesc_0,  _S52 : ptr<function, Payload_0>)
{
    RayVsScene_BVH_0(_S46, _S47, _S48, _S49, _S50, _S51, &((*_S52)));
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn RayGen(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var _S53 : vec2<f32> = (vec2<f32>(px_0) + vec2<f32>(0.5f)) / vec2<f32>(_SimpleRTRayGenCB._dispatchSize_SimpleRTRayGen_0.xy) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S53;
    screenPos_0[i32(1)] = - _S53.y;
    var _S54 : vec4<f32> = (((mat4x4<f32>(_SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(0)][i32(3)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(1)][i32(3)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(2)][i32(3)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(0)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(1)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(2)], _SimpleRTRayGenCB.clipToWorld_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, _SimpleRTRayGenCB.depthNearPlane_0, 1.0f))));
    var world_0 : vec4<f32> = _S54;
    var _S55 : vec3<f32> = _S54.xyz / vec3<f32>(_S54.w);
    world_0.x = _S55.x;
    world_0.y = _S55.y;
    world_0.z = _S55.z;
    var ray_0 : RayDesc_0;
    ray_0.Origin_0 = _SimpleRTRayGenCB.cameraPos_0;
    ray_0.TMin_0 = 0.0f;
    ray_0.TMax_0 = 1000.0f;
    ray_0.Direction_0 = normalize(world_0.xyz - ray_0.Origin_0);
    var payload_5 : Payload_0;
    payload_5.RGB_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
    RayVsScene_0(u32(1), u32(255), u32(0), u32(0), u32(0), ray_0, &(payload_5));
    textureStore((g_texture), (px_0), (vec4<f32>(payload_5.RGB_0, 1.0f)));
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

// Constant buffer _SimpleRTRayGenCB
constantBuffer__SimpleRTRayGenCB = null;
constantBuffer__SimpleRTRayGenCB_size = 144;
constantBuffer__SimpleRTRayGenCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// RayGen Shader DoRT
Hash_RayGen_DoRT = 0;
ShaderModule_RayGen_DoRT = null;
BindGroupLayout_RayGen_DoRT = null;
PipelineLayout_RayGen_DoRT = null;
Pipeline_RayGen_DoRT = null;

// -------------------- Imported Members

// Buffer Scene : The raytracing scene
buffer_Scene = null;
buffer_Scene_count = 0;
buffer_Scene_stride = 0;
buffer_Scene_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;
buffer_Scene_isAABBs = false; // For raytracing: if true, treated as AABBs for an intersection shader, else treated as triangles
buffer_Scene_isBVH = false; // For raytracing: if true, treated as a BVH of triangles

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
variable_clipToWorld = [ 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0. ];
variableDefault_clipToWorld = [ 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0. ];
variableChanged_clipToWorld = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_cameraPos = [ 0., 0., 0. ];
variableDefault_cameraPos = [ 0., 0., 0. ];
variableChanged_cameraPos = [ false, false, false ];
variable_depthNearPlane = 0.;  // The depth value for the near plane.
variableDefault_depthNearPlane = 0.;  // The depth value for the near plane.
variableChanged_depthNearPlane = false;
variable_hitColor = [ 0., 1., 0. ];
variableDefault_hitColor = [ 0., 1., 0. ];
variableChanged_hitColor = [ false, false, false ];
variable_missColor = [ 1., 0., 0. ];
variableDefault_missColor = [ 1., 0., 0. ];
variableChanged_missColor = [ false, false, false ];

// -------------------- Private Variables

variable__dispatchSize_SimpleRTRayGen = [ 0,0,0 ];
variableDefault__dispatchSize_SimpleRTRayGen = [ 0,0,0 ];
variableChanged__dispatchSize_SimpleRTRayGen = [ false, false, false ];
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
    _size: 32,
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

static StructOffsets__SimpleRTRayGenCB =
{
    _dispatchSize_SimpleRTRayGen_0: 0,
    _dispatchSize_SimpleRTRayGen_1: 4,
    _dispatchSize_SimpleRTRayGen_2: 8,
    _padding0: 12,
    cameraPos_0: 16,
    cameraPos_1: 20,
    cameraPos_2: 24,
    _padding1: 28,
    clipToWorld_0: 32,
    clipToWorld_1: 36,
    clipToWorld_2: 40,
    clipToWorld_3: 44,
    clipToWorld_4: 48,
    clipToWorld_5: 52,
    clipToWorld_6: 56,
    clipToWorld_7: 60,
    clipToWorld_8: 64,
    clipToWorld_9: 68,
    clipToWorld_10: 72,
    clipToWorld_11: 76,
    clipToWorld_12: 80,
    clipToWorld_13: 84,
    clipToWorld_14: 88,
    clipToWorld_15: 92,
    depthNearPlane: 96,
    _padding2: 100,
    _padding3: 104,
    _padding4: 108,
    hitColor_0: 112,
    hitColor_1: 116,
    hitColor_2: 120,
    _padding5: 124,
    missColor_0: 128,
    missColor_1: 132,
    missColor_2: 136,
    _padding6: 140,
    _size: 144,
}

static StructOffsets_VertexBuffer_Unpadded =
{
    Color_0: 0,
    Color_1: 4,
    Color_2: 8,
    Position_0: 12,
    Position_1: 16,
    Position_2: 20,
    _size: 24,
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

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Texture
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1280) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 720) / 1) + 0,
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
                label: "texture simpleRT.Texture",
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
                label: "texture simpleRT.DoRT_g_texture_ReadOnly",
                size: this.texture_DoRT_g_texture_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_DoRT_g_texture_ReadOnly_format),
                usage: this.texture_DoRT_g_texture_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create raygen shader DoRT
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
                // g_textureReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_DoRT_g_texture_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _SimpleRTRayGenCB
                binding: 3,
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
                ? class_simpleRT.ShaderCode_DoRT_SimpleRTRayGen_BVH
                : class_simpleRT.ShaderCode_DoRT_SimpleRTRayGen_NonBVH
            ;
            shaderCode = "fn SCENE_IS_AABBS() -> bool { return " + this.buffer_Scene_isAABBs.toString() + "; }\n" + shaderCode;
            shaderCode = shaderCode.replace("/*(g_texture_format)*/", Shared.GetNonSRGBFormat(this.texture_Texture_format));
            shaderCode = shaderCode.replace("/*(g_textureReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_DoRT_g_texture_ReadOnly_format));

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
    encoder.pushDebugGroup("simpleRT.Texture");

    encoder.popDebugGroup(); // "simpleRT.Texture"

    encoder.pushDebugGroup("simpleRT.Scene");

    encoder.popDebugGroup(); // "simpleRT.Scene"

    encoder.pushDebugGroup("simpleRT.DoRT_g_texture_ReadOnly");

    encoder.popDebugGroup(); // "simpleRT.DoRT_g_texture_ReadOnly"

    encoder.pushDebugGroup("simpleRT.Copy_DoRT_g_texture");

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

    encoder.popDebugGroup(); // "simpleRT.Copy_DoRT_g_texture"

    encoder.pushDebugGroup("simpleRT._SimpleRTRayGenCB");

    // Create constant buffer _SimpleRTRayGenCB
    if (this.constantBuffer__SimpleRTRayGenCB === null)
    {
        this.constantBuffer__SimpleRTRayGenCB = device.createBuffer({
            label: "simpleRT._SimpleRTRayGenCB",
            size: Shared.Align(16, this.constructor.StructOffsets__SimpleRTRayGenCB._size),
            usage: this.constantBuffer__SimpleRTRayGenCB_usageFlags,
        });
    }

    // Upload values to constant buffer _SimpleRTRayGenCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__SimpleRTRayGenCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__SimpleRTRayGenCB._dispatchSize_SimpleRTRayGen_0, this.variable__dispatchSize_SimpleRTRayGen[0], true);
        view.setUint32(this.constructor.StructOffsets__SimpleRTRayGenCB._dispatchSize_SimpleRTRayGen_1, this.variable__dispatchSize_SimpleRTRayGen[1], true);
        view.setUint32(this.constructor.StructOffsets__SimpleRTRayGenCB._dispatchSize_SimpleRTRayGen_2, this.variable__dispatchSize_SimpleRTRayGen[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.cameraPos_0, this.variable_cameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.cameraPos_1, this.variable_cameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.cameraPos_2, this.variable_cameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_0, this.variable_clipToWorld[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_1, this.variable_clipToWorld[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_2, this.variable_clipToWorld[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_3, this.variable_clipToWorld[3], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_4, this.variable_clipToWorld[4], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_5, this.variable_clipToWorld[5], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_6, this.variable_clipToWorld[6], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_7, this.variable_clipToWorld[7], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_8, this.variable_clipToWorld[8], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_9, this.variable_clipToWorld[9], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_10, this.variable_clipToWorld[10], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_11, this.variable_clipToWorld[11], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_12, this.variable_clipToWorld[12], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_13, this.variable_clipToWorld[13], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_14, this.variable_clipToWorld[14], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.clipToWorld_15, this.variable_clipToWorld[15], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.depthNearPlane, this.variable_depthNearPlane, true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.hitColor_0, this.variable_hitColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.hitColor_1, this.variable_hitColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.hitColor_2, this.variable_hitColor[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.missColor_0, this.variable_missColor[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.missColor_1, this.variable_missColor[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimpleRTRayGenCB.missColor_2, this.variable_missColor[2], true);
        device.queue.writeBuffer(this.constantBuffer__SimpleRTRayGenCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "simpleRT._SimpleRTRayGenCB"

    encoder.pushDebugGroup("simpleRT.DoRT");

    // Run raygen shader DoRT
    if (this.variable_enabled)
    {
        const bindGroup = device.createBindGroup({
            label: "RayGen Bind Group DoRT",
            layout: this.BindGroupLayout_RayGen_DoRT,
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
                    // g_textureReadOnly
                    binding: 2,
                    resource: this.texture_DoRT_g_texture_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _SimpleRTRayGenCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__SimpleRTRayGenCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Texture_size;
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
            device.queue.writeBuffer(this.constantBuffer__SimpleRTRayGenCB, this.constructor.StructOffsets__SimpleRTRayGenCB._dispatchSize_SimpleRTRayGen_0, specialVariablesBuffer);
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

    encoder.popDebugGroup(); // "simpleRT.DoRT"

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

var simpleRT = new class_simpleRT;

export default simpleRT;
