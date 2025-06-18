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

class class_DemofoxNeonDesert
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "PathTrace", node "PathTrace"
static ShaderCode_PathTrace_PathTrace = `
struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_PathTraceCB_std140_0
{
    @align(16) CameraChanged_0 : u32,
    @align(4) _padding0_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) _padding3_0 : f32,
    @align(16) InvViewMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) Reset_0 : u32,
    @align(4) apertureRadius_0 : f32,
    @align(8) focalPlaneDistance_0 : f32,
    @align(4) iFrame_0 : i32,
    @align(16) minStepDistance_0 : f32,
    @align(4) numBounces_0 : i32,
    @align(8) rayMarchSteps_0 : i32,
    @align(4) _padding4_0 : f32,
};

@binding(2) @group(0) var<uniform> _PathTraceCB : Struct_PathTraceCB_std140_0;
@binding(0) @group(0) var accumulation : texture_storage_2d</*(accumulation_format)*/, write>;

@binding(1) @group(0) var accumulationReadOnly : texture_storage_2d</*(accumulationReadOnly_format)*/, read>;

fn wang_hash_0( seed_0 : ptr<function, u32>) -> u32
{
    var _S1 : u32 = (((((*seed_0) ^ (u32(61)))) ^ ((((*seed_0) >> (u32(16))))))) * u32(9);
    var _S2 : u32 = ((_S1 ^ (((_S1 >> (u32(4))))))) * u32(668265261);
    var _S3 : u32 = (_S2 ^ (((_S2 >> (u32(15))))));
    (*seed_0) = _S3;
    return _S3;
}

fn RandomFloat01_0( state_0 : ptr<function, u32>) -> f32
{
    var _S4 : u32 = wang_hash_0(&((*state_0)));
    return f32(_S4) / 4.294967296e+09f;
}

struct SMaterial_0
{
     diffuse_0 : vec3<f32>,
     specular_0 : vec3<f32>,
     roughness_0 : f32,
     emissive_0 : vec3<f32>,
};

struct SRayHitInfo_0
{
     hitAnObject_0 : bool,
     rayMarchedObject_0 : bool,
     dist_0 : f32,
     normal_0 : vec3<f32>,
     material_0 : SMaterial_0,
};

fn TestPlaneTrace_0( rayPos_0 : vec3<f32>,  rayDir_0 : vec3<f32>,  info_0 : ptr<function, SRayHitInfo_0>,  plane_0 : vec4<f32>,  material_1 : SMaterial_0) -> bool
{
    var _S5 : vec3<f32> = plane_0.xyz;
    var denom_0 : f32 = dot(_S5, rayDir_0);
    if((abs(denom_0)) > 0.00100000004749745f)
    {
        var dist_1 : f32 = (plane_0.w - dot(_S5, rayPos_0)) / denom_0;
        var _S6 : bool;
        if(dist_1 > 0.10000000149011612f)
        {
            _S6 = dist_1 < ((*info_0).dist_0);
        }
        else
        {
            _S6 = false;
        }
        if(_S6)
        {
            (*info_0).hitAnObject_0 = true;
            (*info_0).rayMarchedObject_0 = false;
            (*info_0).dist_0 = dist_1;
            (*info_0).normal_0 = _S5;
            (*info_0).material_0 = material_1;
            return true;
        }
    }
    return false;
}

fn TestSphereTrace_0( rayPos_1 : vec3<f32>,  rayDir_1 : vec3<f32>,  info_1 : ptr<function, SRayHitInfo_0>,  sphere_0 : vec4<f32>,  material_2 : SMaterial_0) -> bool
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
    if(dist_3 > 0.10000000149011612f)
    {
        _S9 = dist_3 < ((*info_1).dist_0);
    }
    else
    {
        _S9 = false;
    }
    if(_S9)
    {
        (*info_1).hitAnObject_0 = true;
        (*info_1).rayMarchedObject_0 = false;
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
        (*info_1).material_0 = material_2;
        return true;
    }
    return false;
}

fn TestCylinderTrace_0( rayPos_2 : vec3<f32>,  rayDir_2 : vec3<f32>,  info_2 : ptr<function, SRayHitInfo_0>,  pos_0 : vec3<f32>,  radius_0 : f32,  height_0 : f32,  material_3 : SMaterial_0) -> bool
{
    var relativeRayPos_0 : vec3<f32> = rayPos_2 - pos_0;
    var projectedRayPos_0 : vec3<f32> = vec3<f32>(relativeRayPos_0.x, 0.0f, relativeRayPos_0.z);
    var projectedRayDir_0 : vec3<f32> = vec3<f32>(rayDir_2.x, 0.0f, rayDir_2.z);
    var lengthCorrection_0 : f32 = length(projectedRayDir_0);
    var b_1 : f32 = dot(projectedRayPos_0, projectedRayDir_0 / vec3<f32>(lengthCorrection_0));
    var c_1 : f32 = dot(projectedRayPos_0, projectedRayPos_0) - radius_0 * radius_0;
    var _S14 : bool;
    if(c_1 > 0.0f)
    {
        _S14 = b_1 > 0.0f;
    }
    else
    {
        _S14 = false;
    }
    if(_S14)
    {
        return false;
    }
    var discr_1 : f32 = b_1 * b_1 - c_1;
    if(discr_1 < 0.0f)
    {
        return false;
    }
    var _S15 : f32 = - b_1;
    var _S16 : f32 = sqrt(discr_1);
    var dist_4 : f32 = _S15 - _S16;
    var _S17 : bool = dist_4 < 0.0f;
    var dist_5 : f32;
    if(_S17)
    {
        dist_5 = _S15 + _S16;
    }
    else
    {
        dist_5 = dist_4;
    }
    var dist_6 : f32 = dist_5 / lengthCorrection_0;
    var relativeHitPos_0 : vec3<f32> = relativeRayPos_0 + rayDir_2 * vec3<f32>(dist_6);
    var _S18 : f32 = relativeHitPos_0.y;
    if(_S18 < 0.0f)
    {
        _S14 = true;
    }
    else
    {
        _S14 = _S18 > height_0;
    }
    var relativeHitPos_1 : vec3<f32>;
    if(_S14)
    {
        var dist_7 : f32 = (_S15 + _S16) / lengthCorrection_0;
        var relativeHitPos_2 : vec3<f32> = relativeRayPos_0 + rayDir_2 * vec3<f32>(dist_7);
        var _S19 : f32 = relativeHitPos_2.y;
        if(_S19 < 0.0f)
        {
            _S14 = true;
        }
        else
        {
            _S14 = _S19 > height_0;
        }
        if(_S14)
        {
            return false;
        }
        relativeHitPos_1 = relativeHitPos_2;
        dist_5 = dist_7;
    }
    else
    {
        relativeHitPos_1 = relativeHitPos_0;
        dist_5 = dist_6;
    }
    if(dist_5 > 0.10000000149011612f)
    {
        _S14 = dist_5 < ((*info_2).dist_0);
    }
    else
    {
        _S14 = false;
    }
    if(_S14)
    {
        (*info_2).hitAnObject_0 = true;
        (*info_2).rayMarchedObject_0 = false;
        (*info_2).dist_0 = dist_5;
        var _S20 : vec3<f32> = normalize(vec3<f32>(relativeHitPos_1.x, 0.0f, relativeHitPos_1.z));
        if(_S17)
        {
            dist_5 = -1.0f;
        }
        else
        {
            dist_5 = 1.0f;
        }
        (*info_2).normal_0 = _S20 * vec3<f32>(dist_5);
        (*info_2).material_0 = material_3;
        return true;
    }
    return false;
}

fn mod_0( x_0 : f32,  y_0 : f32) -> f32
{
    return x_0 - y_0 * floor(x_0 / y_0);
}

fn hash12_0( p_0 : vec2<f32>) -> f32
{
    var p3_0 : vec3<f32> = fract(p_0.xyx * vec3<f32>(0.1031000018119812f));
    var p3_1 : vec3<f32> = p3_0 + vec3<f32>(dot(p3_0, p3_0.yzx + vec3<f32>(33.3300018310546875f)));
    return fract((p3_1.x + p3_1.y) * p3_1.z);
}

fn RayTraceMountainCylinders_0( rayPos_3 : vec3<f32>,  rayDir_3 : vec3<f32>,  hitInfo_0 : ptr<function, SRayHitInfo_0>,  seed_1 : f32,  radius_1 : f32,  maxMountainHeight_0 : f32)
{
    var material_4 : SMaterial_0;
    material_4.diffuse_0 = vec3<f32>(1.0f, 1.0f, 1.0f);
    const _S21 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    material_4.specular_0 = _S21;
    material_4.roughness_0 = 0.02500000037252903f;
    material_4.emissive_0 = _S21;
    var oldHitInfo_0 : SRayHitInfo_0 = (*hitInfo_0);
    var _S22 : bool = TestCylinderTrace_0(rayPos_3, rayDir_3, &((*hitInfo_0)), vec3<f32>(0.0f, -15.0f, 0.0f), radius_1, maxMountainHeight_0, material_4);
    if(_S22)
    {
        var hitPos_0 : vec3<f32> = rayPos_3 + rayDir_3 * vec3<f32>((*hitInfo_0).dist_0);
        var heightPercent_0 : f32 = (hitPos_0.y + 15.0f) / maxMountainHeight_0;
        var _S23 : f32 = clamp((atan2(hitPos_0.z, hitPos_0.x) + 3.14159274101257324f) / 6.28318548202514648f, 0.0f, 1.0f) * 20.0f;
        var thisHeightIndex_0 : f32 = floor(_S23);
        var mountainHeight_0 : f32 = mix(hash12_0(vec2<f32>(thisHeightIndex_0, seed_1)) * 0.75f + 0.25f, hash12_0(vec2<f32>(mod_0(thisHeightIndex_0 + 1.0f, 20.0f), seed_1)) * 0.75f + 0.25f, fract(_S23));
        if(heightPercent_0 > mountainHeight_0)
        {
            (*hitInfo_0) = oldHitInfo_0;
        }
        else
        {
            (*hitInfo_0).material_0.emissive_0 = pow(vec3<f32>(0.99000000953674316f, 0.03999999910593033f, 0.98000001907348633f), vec3<f32>(2.20000004768371582f, 2.20000004768371582f, 2.20000004768371582f)) * vec3<f32>(5.0f) * vec3<f32>(smoothstep(mountainHeight_0 - 0.25f / maxMountainHeight_0, mountainHeight_0, heightPercent_0));
        }
    }
    return;
}

fn Bias_0( x_1 : f32,  bias_0 : f32) -> f32
{
    return x_1 / ((1.0f / bias_0 - 2.0f) * (1.0f - x_1) + 1.0f);
}

fn hash22_0( p_1 : vec2<f32>) -> vec2<f32>
{
    var p3_2 : vec3<f32> = fract(p_1.xyx * vec3<f32>(0.1031000018119812f, 0.10300000011920929f, 0.09730000048875809f));
    var p3_3 : vec3<f32> = p3_2 + vec3<f32>(dot(p3_2, p3_2.yzx + vec3<f32>(33.3300018310546875f)));
    return fract((p3_3.xx + p3_3.yz) * p3_3.zy);
}

fn sdStar_0( p_2 : vec2<f32>,  r_0 : f32,  n_0 : i32,  m_1 : f32) -> f32
{
    var an_0 : f32 = 3.14159297943115234f / f32(n_0);
    var en_0 : f32 = 3.14159297943115234f / m_1;
    var _S24 : f32 = sin(an_0);
    var _S25 : f32 = sin(en_0);
    var ecs_0 : vec2<f32> = vec2<f32>(cos(en_0), _S25);
    var bn_0 : f32 = mod_0(atan2(p_2.x, p_2.y), 2.0f * an_0) - an_0;
    var _S26 : vec2<f32> = vec2<f32>(length(p_2)) * vec2<f32>(cos(bn_0), abs(sin(bn_0))) - vec2<f32>(r_0) * vec2<f32>(cos(an_0), _S24);
    var _S27 : vec2<f32> = _S26 + ecs_0 * vec2<f32>(clamp(- dot(_S26, ecs_0), 0.0f, r_0 * _S24 / _S25));
    return length(_S27) * f32(sign(_S27.x));
}

fn RayTraceScene_0( rayPos_4 : vec3<f32>,  rayDir_4 : vec3<f32>,  hitInfo_1 : ptr<function, SRayHitInfo_0>)
{
    var material_5 : SMaterial_0;
    material_5.diffuse_0 = vec3<f32>(0.10000000149011612f, 0.10000000149011612f, 0.10000000149011612f);
    const _S28 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    material_5.specular_0 = _S28;
    material_5.roughness_0 = 0.01999999955296516f;
    material_5.emissive_0 = _S28;
    var _S29 : bool = TestPlaneTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(normalize(vec3<f32>(0.0f, 1.0f, 0.0f)), -5.0f), material_5);
    if(_S29)
    {
        var intersectPos_0 : vec3<f32> = rayPos_4 + rayDir_4 * vec3<f32>((*hitInfo_1).dist_0);
        var uv_0 : vec2<f32> = fract(intersectPos_0.xz / vec2<f32>(75.0f));
        const _S30 : vec3<f32> = vec3<f32>(2.20000004768371582f, 2.20000004768371582f, 2.20000004768371582f);
        var _S31 : vec3<f32> = vec3<f32>(step(min(abs(uv_0.x - 0.5f), abs(uv_0.y - 0.5f)), 0.00499999988824129f));
        (*hitInfo_1).material_0.emissive_0 = mix(pow(vec3<f32>(0.11999999731779099f, 0.01999999955296516f, 0.09000000357627869f), _S30), mix(pow(vec3<f32>(0.73000001907348633f, 0.05999999865889549f, 0.99000000953674316f), _S30), vec3<f32>(0.0625f, 0.0f, 1.0f), vec3<f32>(((cos(intersectPos_0.x * 0.01530000008642673f + intersectPos_0.y * 0.06319999694824219f) + 1.0f) / 2.5f))), _S31);
        (*hitInfo_1).material_0.specular_0 = mix(vec3<f32>(0.33000001311302185f, 0.33000001311302185f, 0.33000001311302185f), _S28, _S31);
    }
    var material_6 : SMaterial_0;
    material_6.diffuse_0 = vec3<f32>(0.5f, 0.5f, 0.5f);
    const _S32 : vec3<f32> = vec3<f32>(1.0f, 1.0f, 1.0f);
    material_6.specular_0 = _S32;
    material_6.roughness_0 = 0.03500000014901161f;
    material_6.emissive_0 = _S28;
    var _S33 : bool = TestSphereTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(20.0f, 5.0f, -20.0f, 10.0f), material_6);
    var _S34 : bool = TestSphereTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(-60.0f, 0.0f, 20.0f, 10.0f), material_6);
    var _S35 : bool = TestSphereTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(0.0f, 5.0f, -65.0f, 10.0f), material_6);
    var _S36 : bool = TestSphereTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(15.0f, -3.0f, 65.0f, 2.0f), material_6);
    RayTraceMountainCylinders_0(rayPos_4, rayDir_4, &((*hitInfo_1)), 284.0f, 700.0f, 50.0f);
    RayTraceMountainCylinders_0(rayPos_4, rayDir_4, &((*hitInfo_1)), 1337.0f, 800.0f, 150.0f);
    RayTraceMountainCylinders_0(rayPos_4, rayDir_4, &((*hitInfo_1)), 1932.0f, 900.0f, 250.0f);
    var material_7 : SMaterial_0;
    material_7.diffuse_0 = _S28;
    material_7.specular_0 = _S28;
    material_7.roughness_0 = 0.0f;
    material_7.emissive_0 = _S28;
    var oldHitInfo_1 : SRayHitInfo_0 = (*hitInfo_1);
    var _S37 : bool = TestPlaneTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(normalize(vec3<f32>(0.0f, 0.0f, -1.0f)), -1000.0f), material_7);
    if(_S37)
    {
        var uv_1 : vec2<f32> = (rayPos_4 + rayDir_4 * vec3<f32>((*hitInfo_1).dist_0)).xy;
        var _S38 : f32 = uv_1.y;
        var stripe_0 : i32 = i32((- _S38 + 500.0f) / 50.0f);
        var stripe_1 : i32;
        if(stripe_0 > i32(1))
        {
            stripe_1 = (stripe_0 - i32(1)) % i32(2);
        }
        else
        {
            stripe_1 = i32(0);
        }
        if((length(uv_1)) > 500.0f)
        {
            (*hitInfo_1) = oldHitInfo_1;
        }
        else
        {
            if(stripe_1 == i32(1))
            {
                (*hitInfo_1).material_0.emissive_0 = pow(vec3<f32>(0.28999999165534973f, 0.02999999932944775f, 0.41999998688697815f) * vec3<f32>(0.55000001192092896f), vec3<f32>(2.20000004768371582f, 2.20000004768371582f, 2.20000004768371582f));
            }
            else
            {
                const _S39 : vec3<f32> = vec3<f32>(2.20000004768371582f, 2.20000004768371582f, 2.20000004768371582f);
                (*hitInfo_1).material_0.emissive_0 = mix(pow(vec3<f32>(0.11999999731779099f, 0.01999999955296516f, 0.09000000357627869f), _S39), pow(vec3<f32>(0.99000000953674316f, 0.03999999910593033f, 0.98000001907348633f), _S39), vec3<f32>(Bias_0(clamp((_S38 - 100.0f) / 400.0f, 0.0f, 1.0f), 0.05000000074505806f)));
            }
        }
    }
    var material_8 : SMaterial_0;
    material_8.diffuse_0 = _S28;
    material_8.specular_0 = _S28;
    material_8.roughness_0 = 0.0f;
    material_8.emissive_0 = _S28;
    var _S40 : bool = TestSphereTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(0.0f, 500.0f, -1200.0f, 200.0f), material_8);
    if(_S40)
    {
        var _S41 : f32 = (rayPos_4 + rayDir_4 * vec3<f32>((*hitInfo_1).dist_0)).y - 500.0f;
        (*hitInfo_1).material_0.emissive_0 = vec3<f32>(1.0f, 1.0f, 0.0f) * vec3<f32>(Bias_0(clamp(_S41 / 200.0f, 0.0f, 1.0f), 0.10000000149011612f)) + vec3<f32>(1.0f, 0.0f, 0.0f) * vec3<f32>(Bias_0(1.0f - clamp(_S41 / 200.0f, 0.0f, 1.0f), 0.89999997615814209f)) + vec3<f32>(1.0f, 0.0f, 1.0f) * vec3<f32>(Bias_0(clamp(_S41 / -200.0f, 0.0f, 1.0f), 0.10000000149011612f));
    }
    var material_9 : SMaterial_0;
    material_9.diffuse_0 = _S28;
    material_9.specular_0 = _S28;
    material_9.roughness_0 = 0.0f;
    material_9.emissive_0 = vec3<f32>(0.80000001192092896f, 0.0f, 1.0f);
    const _S42 : vec3<f32> = vec3<f32>(0.0f, 460.0f, -1200.0f);
    var _S43 : bool = TestCylinderTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), _S42, 300.0f, 10.0f, material_9);
    var _S44 : bool = TestCylinderTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), _S42, 350.0f, 10.0f, material_9);
    var _S45 : bool = TestCylinderTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), _S42, 375.0f, 10.0f, material_9);
    var material_10 : SMaterial_0;
    material_10.diffuse_0 = _S28;
    material_10.specular_0 = _S28;
    material_10.roughness_0 = 0.0f;
    material_10.emissive_0 = pow(vec3<f32>(0.28999999165534973f, 0.02999999932944775f, 0.41999998688697815f) * vec3<f32>(0.55000001192092896f), vec3<f32>(2.20000004768371582f, 2.20000004768371582f, 2.20000004768371582f));
    var _S46 : bool = TestSphereTrace_0(rayPos_4, rayDir_4, &((*hitInfo_1)), vec4<f32>(0.0f, 0.0f, 0.0f, 2000.0f), material_10);
    if(_S46)
    {
        var hitPos_1 : vec3<f32> = (rayPos_4 + rayDir_4 * vec3<f32>((*hitInfo_1).dist_0)).yzx;
        var _S47 : f32 = hitPos_1.y;
        var _S48 : f32 = hitPos_1.x;
        var _S49 : f32 = (atan2(_S47, _S48) + 3.14159274101257324f) / 6.28318548202514648f * 45.0f;
        var _S50 : f32 = (atan2(sqrt(_S48 * _S48 + _S47 * _S47), hitPos_1.z) + 3.14159274101257324f) / 6.28318548202514648f * 45.0f;
        var cellIndex_0 : vec2<f32> = vec2<f32>(floor(_S49), floor(_S50));
        var starRadius_0 : f32 = hash12_0(cellIndex_0) * 0.10000000149011612f + 0.07000000029802322f;
        var _S51 : vec2<f32> = hash22_0(cellIndex_0) * vec2<f32>((1.0f - starRadius_0 * 2.0f)) + vec2<f32>(starRadius_0) - vec2<f32>(fract(_S49), fract(_S50));
        var _S52 : f32 = length(_S51);
        (*hitInfo_1).material_0.emissive_0 = mix(material_10.emissive_0, _S32, vec3<f32>((step(sdStar_0(_S51, starRadius_0, i32(4), 3.75f) * pow(_S52 / starRadius_0, 10.0f), 0.0f) * pow(1.0f - clamp(_S52 / starRadius_0, 0.0f, 1.0f), 3.0f))));
    }
    return;
}

fn LineDistance_0( A_0 : vec3<f32>,  B_0 : vec3<f32>,  width_0 : f32,  p_3 : vec3<f32>,  normal_1 : ptr<function, vec3<f32>>) -> f32
{
    var AB_0 : vec3<f32> = B_0 - A_0;
    var _S53 : vec3<f32> = p_3 - (A_0 + vec3<f32>(min(1.0f, max(0.0f, dot(p_3 - A_0, AB_0) / dot(AB_0, AB_0)))) * AB_0);
    (*normal_1) = normalize(_S53);
    return length(_S53) - width_0;
}

fn TestLineMarch_0( rayPos_5 : vec3<f32>,  info_3 : ptr<function, SRayHitInfo_0>,  A_1 : vec3<f32>,  B_1 : vec3<f32>,  width_1 : f32,  material_11 : SMaterial_0)
{
    var normal_2 : vec3<f32>;
    var dist_8 : f32 = LineDistance_0(A_1, B_1, width_1, rayPos_5, &(normal_2));
    if(dist_8 < ((*info_3).dist_0))
    {
        (*info_3).rayMarchedObject_0 = true;
        (*info_3).dist_0 = dist_8;
        (*info_3).normal_0 = normal_2;
        (*info_3).material_0 = material_11;
    }
    return;
}

fn dot2_0( v_0 : vec3<f32>) -> f32
{
    return dot(v_0, v_0);
}

fn BezierDistance_0( pos_1 : vec3<f32>,  A_2 : vec3<f32>,  B_2 : vec3<f32>,  C_0 : vec3<f32>,  width_2 : f32) -> f32
{
    var a_0 : vec3<f32> = B_2 - A_2;
    var _S54 : vec3<f32> = vec3<f32>(2.0f);
    var b_2 : vec3<f32> = A_2 - _S54 * B_2 + C_0;
    var c_2 : vec3<f32> = a_0 * _S54;
    var d_0 : vec3<f32> = A_2 - pos_1;
    var kk_0 : f32 = 1.0f / dot(b_2, b_2);
    var kx_0 : f32 = kk_0 * dot(a_0, b_2);
    var ky_0 : f32 = kk_0 * (2.0f * dot(a_0, a_0) + dot(d_0, b_2)) / 3.0f;
    var p_4 : f32 = ky_0 - kx_0 * kx_0;
    var q_0 : f32 = kx_0 * (2.0f * kx_0 * kx_0 - 3.0f * ky_0) + kk_0 * dot(d_0, a_0);
    var h_0 : f32 = q_0 * q_0 + 4.0f * (p_4 * p_4 * p_4);
    var res_0 : f32;
    if(h_0 >= 0.0f)
    {
        var h_1 : f32 = sqrt(h_0);
        var x_2 : vec2<f32> = (vec2<f32>(h_1, - h_1) - vec2<f32>(q_0)) / vec2<f32>(2.0f);
        var uv_2 : vec2<f32> = vec2<f32>(sign(x_2)) * pow(abs(x_2), vec2<f32>(0.3333333432674408f));
        var _S55 : vec3<f32> = vec3<f32>(clamp(uv_2.x + uv_2.y - kx_0, 0.0f, 1.0f));
        res_0 = dot2_0(d_0 + (c_2 + b_2 * _S55) * _S55);
    }
    else
    {
        var z_0 : f32 = sqrt(- p_4);
        var v_1 : f32 = acos(q_0 / (p_4 * z_0 * 2.0f)) / 3.0f;
        var m_2 : f32 = cos(v_1);
        var n_1 : f32 = sin(v_1) * 1.73205077648162842f;
        var t_0 : vec3<f32> = clamp(vec3<f32>(m_2 + m_2, - n_1 - m_2, n_1 - m_2) * vec3<f32>(z_0) - vec3<f32>(kx_0), vec3<f32>(0.0f), vec3<f32>(1.0f));
        var _S56 : vec3<f32> = vec3<f32>(t_0.x);
        var _S57 : vec3<f32> = vec3<f32>(t_0.y);
        res_0 = min(dot2_0(d_0 + (c_2 + b_2 * _S56) * _S56), dot2_0(d_0 + (c_2 + b_2 * _S57) * _S57));
    }
    return sqrt(res_0) - width_2;
}

fn TestBezierMarch_0( rayPos_6 : vec3<f32>,  info_4 : ptr<function, SRayHitInfo_0>,  A_3 : vec3<f32>,  B_3 : vec3<f32>,  C_1 : vec3<f32>,  width_3 : f32,  material_12 : SMaterial_0)
{
    var dist_9 : f32 = BezierDistance_0(rayPos_6, A_3, B_3, C_1, width_3);
    if(dist_9 < ((*info_4).dist_0))
    {
        (*info_4).rayMarchedObject_0 = true;
        (*info_4).dist_0 = dist_9;
        (*info_4).normal_0 = vec3<f32>(1.0f, 0.0f, 0.0f);
        (*info_4).material_0 = material_12;
    }
    return;
}

fn TestSceneMarch_0( rayPos_7 : vec3<f32>) -> SRayHitInfo_0
{
    var hitInfo_2 : SRayHitInfo_0;
    hitInfo_2.hitAnObject_0 = false;
    hitInfo_2.dist_0 = 10000.0f;
    const A_4 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const B_4 : vec3<f32> = vec3<f32>(1.5f, 3.0f, 0.0f);
    const C_2 : vec3<f32> = vec3<f32>(3.0f, 0.0f, 0.0f);
    var _S58 : vec3<f32> = vec3<f32>(3.0f);
    var center_0 : vec3<f32> = (B_4 + C_2) / _S58;
    var A_5 : vec3<f32> = (A_4 - center_0) * _S58;
    var B_5 : vec3<f32> = (B_4 - center_0) * _S58;
    var C_3 : vec3<f32> = (C_2 - center_0) * _S58;
    var material_13 : SMaterial_0;
    material_13.diffuse_0 = A_4;
    material_13.specular_0 = A_4;
    material_13.roughness_0 = 0.0f;
    const _S59 : vec3<f32> = vec3<f32>(2.20000004768371582f, 2.20000004768371582f, 2.20000004768371582f);
    var _S60 : vec3<f32> = vec3<f32>(10.0f);
    var _S61 : vec3<f32> = pow(vec3<f32>(0.73000001907348633f, 0.05999999865889549f, 0.99000000953674316f), _S59) * _S60;
    material_13.emissive_0 = _S61;
    TestLineMarch_0(rayPos_7, &(hitInfo_2), A_5, B_5, 0.10000000149011612f, material_13);
    TestLineMarch_0(rayPos_7, &(hitInfo_2), B_5, C_3, 0.10000000149011612f, material_13);
    TestLineMarch_0(rayPos_7, &(hitInfo_2), C_3, A_5, 0.10000000149011612f, material_13);
    material_13.emissive_0 = pow(vec3<f32>(0.30000001192092896f, 0.15000000596046448f, 1.0f), _S59) * _S60;
    const _S62 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 5.0f);
    var A_6 : vec3<f32> = A_5 + _S62;
    var B_6 : vec3<f32> = B_5 + _S62;
    var C_4 : vec3<f32> = C_3 + _S62;
    TestLineMarch_0(rayPos_7, &(hitInfo_2), A_6, B_6, 0.10000000149011612f, material_13);
    TestLineMarch_0(rayPos_7, &(hitInfo_2), B_6, C_4, 0.10000000149011612f, material_13);
    TestLineMarch_0(rayPos_7, &(hitInfo_2), C_4, A_6, 0.10000000149011612f, material_13);
    material_13.emissive_0 = pow(vec3<f32>(1.0f, 0.15000000596046448f, 0.30000001192092896f), _S59) * _S60;
    var A_7 : vec3<f32> = A_6 + _S62;
    var B_7 : vec3<f32> = B_6 + _S62;
    var C_5 : vec3<f32> = C_4 + _S62;
    TestLineMarch_0(rayPos_7, &(hitInfo_2), A_7, B_7, 0.10000000149011612f, material_13);
    TestLineMarch_0(rayPos_7, &(hitInfo_2), B_7, C_5, 0.10000000149011612f, material_13);
    TestLineMarch_0(rayPos_7, &(hitInfo_2), C_5, A_7, 0.10000000149011612f, material_13);
    var material_14 : SMaterial_0;
    material_14.diffuse_0 = A_4;
    material_14.specular_0 = A_4;
    material_14.roughness_0 = 0.0f;
    material_14.emissive_0 = _S61;
    const cactusOffset_0 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 50.0f);
    var B_8 : vec3<f32> = vec3<f32>(-40.0f, 5.0f, 1.0f) + cactusOffset_0;
    TestBezierMarch_0(rayPos_7, &(hitInfo_2), vec3<f32>(-40.0f, -10.0f, 0.0f) + cactusOffset_0, B_8, vec3<f32>(-40.0f, 20.0f, 0.0f) + cactusOffset_0, 2.0f, material_14);
    TestBezierMarch_0(rayPos_7, &(hitInfo_2), B_8, vec3<f32>(-32.5f, 10.0f, 0.0f) + cactusOffset_0, vec3<f32>(-32.5f, 15.0f, -1.0f) + cactusOffset_0, 1.0f, material_14);
    TestBezierMarch_0(rayPos_7, &(hitInfo_2), vec3<f32>(-40.0f, 2.0f, 1.0f) + cactusOffset_0, vec3<f32>(-47.5f, 7.0f, 2.0f) + cactusOffset_0, vec3<f32>(-47.5f, 13.0f, 4.0f) + cactusOffset_0, 1.0f, material_14);
    return hitInfo_2;
}

fn RayMarchScene_0( startingRayPos_0 : vec3<f32>,  rayDir_5 : vec3<f32>,  oldHitInfo_2 : ptr<function, SRayHitInfo_0>)
{
    const _S63 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var newHitInfo_0 : SRayHitInfo_0 = (*oldHitInfo_2);
    newHitInfo_0.hitAnObject_0 = false;
    var lastHitInfoDist_0 : f32 = 0.0f;
    var lastRayDistance_0 : f32 = 0.10000000149011612f;
    var stepIndex_0 : i32 = i32(0);
    var rayDistance_0 : f32 = 0.10000000149011612f;
    for(;;)
    {
        if(stepIndex_0 < (_PathTraceCB.rayMarchSteps_0))
        {
        }
        else
        {
            break;
        }
        newHitInfo_0 = TestSceneMarch_0(startingRayPos_0 + vec3<f32>(rayDistance_0) * rayDir_5);
        newHitInfo_0.normal_0 = _S63;
        newHitInfo_0.material_0.diffuse_0 = _S63;
        newHitInfo_0.material_0.specular_0 = _S63;
        newHitInfo_0.material_0.roughness_0 = 0.0f;
        newHitInfo_0.material_0.emissive_0 = _S63;
        var _S64 : bool = (newHitInfo_0.dist_0) < 0.0f;
        newHitInfo_0.hitAnObject_0 = _S64;
        if(_S64)
        {
            break;
        }
        var rayDistance_1 : f32 = rayDistance_0 + max(newHitInfo_0.dist_0, _PathTraceCB.minStepDistance_0);
        var _S65 : f32 = newHitInfo_0.dist_0;
        if(rayDistance_1 > ((*oldHitInfo_2).dist_0))
        {
            lastHitInfoDist_0 = _S65;
            lastRayDistance_0 = rayDistance_0;
            rayDistance_0 = rayDistance_1;
            break;
        }
        var stepIndex_1 : i32 = stepIndex_0 + i32(1);
        lastHitInfoDist_0 = _S65;
        lastRayDistance_0 = rayDistance_0;
        rayDistance_0 = rayDistance_1;
        stepIndex_0 = stepIndex_1;
    }
    if(newHitInfo_0.hitAnObject_0)
    {
        var _S66 : f32 = mix(lastRayDistance_0, rayDistance_0, lastHitInfoDist_0 / (lastHitInfoDist_0 - newHitInfo_0.dist_0));
        newHitInfo_0.dist_0 = _S66;
        if(_S66 < ((*oldHitInfo_2).dist_0))
        {
            (*oldHitInfo_2) = newHitInfo_0;
        }
    }
    return;
}

fn RandomUnitVector_0( state_1 : ptr<function, u32>) -> vec3<f32>
{
    var _S67 : f32 = RandomFloat01_0(&((*state_1)));
    var z_1 : f32 = _S67 * 2.0f - 1.0f;
    var _S68 : f32 = RandomFloat01_0(&((*state_1)));
    var a_1 : f32 = _S68 * 6.28318548202514648f;
    var r_1 : f32 = sqrt(1.0f - z_1 * z_1);
    return vec3<f32>(r_1 * cos(a_1), r_1 * sin(a_1), z_1);
}

fn GetColorForRay_0( startRayPos_0 : vec3<f32>,  startRayDir_0 : vec3<f32>,  rngState_0 : ptr<function, u32>) -> vec3<f32>
{
    const _S69 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const _S70 : vec3<f32> = vec3<f32>(1.0f, 1.0f, 1.0f);
    var i_0 : i32 = i32(0);
    var rayPos_8 : vec3<f32> = startRayPos_0;
    var rayDir_6 : vec3<f32> = startRayDir_0;
    var colorMultiplier_0 : vec3<f32> = _S70;
    var ret_0 : vec3<f32> = _S69;
    for(;;)
    {
        if(i_0 <= (_PathTraceCB.numBounces_0))
        {
        }
        else
        {
            break;
        }
        var hitInfo_3 : SRayHitInfo_0;
        hitInfo_3.hitAnObject_0 = false;
        hitInfo_3.dist_0 = 10000.0f;
        RayTraceScene_0(rayPos_8, rayDir_6, &(hitInfo_3));
        RayMarchScene_0(rayPos_8, rayDir_6, &(hitInfo_3));
        if(!hitInfo_3.hitAnObject_0)
        {
            break;
        }
        var rayPos_9 : vec3<f32> = rayPos_8 + rayDir_6 * vec3<f32>(hitInfo_3.dist_0);
        if(hitInfo_3.rayMarchedObject_0)
        {
            hitInfo_3 = TestSceneMarch_0(rayPos_9);
        }
        var ret_1 : vec3<f32> = ret_0 + hitInfo_3.material_0.emissive_0 * colorMultiplier_0;
        var specularLength_0 : f32 = length(hitInfo_3.material_0.specular_0);
        var _S71 : f32 = length(hitInfo_3.material_0.diffuse_0) + specularLength_0;
        if(_S71 == 0.0f)
        {
            ret_0 = ret_1;
            break;
        }
        var specularWeight_0 : f32 = specularLength_0 / _S71;
        var _S72 : f32 = RandomFloat01_0(&((*rngState_0)));
        var doSpecular_0 : f32 = f32(_S72 < specularWeight_0);
        var roughness_1 : f32 = mix(1.0f, hitInfo_3.material_0.roughness_0, doSpecular_0);
        var reflectDir_0 : vec3<f32> = reflect(rayDir_6, hitInfo_3.normal_0);
        var randomDir_0 : vec3<f32> = RandomUnitVector_0(&((*rngState_0)));
        var rayDir_7 : vec3<f32> = normalize(mix(reflectDir_0, randomDir_0, vec3<f32>(roughness_1)));
        if((dot(rayDir_7, hitInfo_3.normal_0)) < 0.0f)
        {
            rayDir_6 = rayDir_7 * vec3<f32>(-1.0f);
        }
        else
        {
            rayDir_6 = rayDir_7;
        }
        var rayPos_10 : vec3<f32> = rayPos_9 + hitInfo_3.normal_0 * vec3<f32>(0.00999999977648258f);
        var colorMultiplier_1 : vec3<f32> = colorMultiplier_0 * mix(hitInfo_3.material_0.diffuse_0 * vec3<f32>(dot(hitInfo_3.normal_0, rayDir_6)), hitInfo_3.material_0.specular_0, vec3<f32>(doSpecular_0));
        i_0 = i_0 + i32(1);
        rayPos_8 = rayPos_10;
        colorMultiplier_0 = colorMultiplier_1;
        ret_0 = ret_1;
    }
    return ret_0;
}

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var rngState_1 : u32 = ((DTid_0.x * u32(1973) + DTid_0.y * u32(9277) + u32(_PathTraceCB.iFrame_0) * u32(26699)) | (u32(1)));
    var _S73 : vec2<u32> = DTid_0.xy;
    var _S74 : vec2<f32> = vec2<f32>(_S73);
    var _S75 : f32 = RandomFloat01_0(&(rngState_1));
    var _S76 : f32 = RandomFloat01_0(&(rngState_1));
    var fragCoordJittered_0 : vec2<f32> = _S74 + (vec2<f32>(_S75, _S76) - vec2<f32>(0.5f));
    var w_0 : u32;
    var h_2 : u32;
    {var dim = textureDimensions((accumulation));((w_0)) = dim.x;((h_2)) = dim.y;};
    var _S77 : vec2<f32> = fragCoordJittered_0 / vec2<f32>(f32(w_0), f32(h_2)) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S77;
    screenPos_0[i32(1)] = - _S77.y;
    var _S78 : vec4<f32> = (((mat4x4<f32>(_PathTraceCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _PathTraceCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, 0.0f, 1.0f))));
    var world_0 : vec4<f32> = _S78;
    var _S79 : vec3<f32> = _S78.xyz / vec3<f32>(_S78.w);
    world_0.x = _S79.x;
    world_0.y = _S79.y;
    world_0.z = _S79.z;
    var rayOrigin_0 : vec3<f32> = _PathTraceCB.CameraPos_0;
    var rayDirection_0 : vec3<f32> = normalize(world_0.xyz - _PathTraceCB.CameraPos_0);
    var rayDirection_1 : vec3<f32>;
    var rayOrigin_1 : vec3<f32>;
    if((_PathTraceCB.apertureRadius_0) > 0.0f)
    {
        var cameraRight_0 : vec3<f32> = (((mat4x4<f32>(_PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(3)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(3)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(3)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(1.0f, 0.0f, 0.0f, 0.0f)))).xyz;
        var cameraUp_0 : vec3<f32> = (((mat4x4<f32>(_PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(0)][i32(3)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(1)][i32(3)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(2)][i32(3)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(0)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(1)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(2)], _PathTraceCB.InvViewMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(0.0f, 1.0f, 0.0f, 0.0f)))).xyz;
        var focalPlanePoint_0 : vec3<f32> = rayOrigin_0 + rayDirection_0 * vec3<f32>(_PathTraceCB.focalPlaneDistance_0);
        var _S80 : f32 = RandomFloat01_0(&(rngState_1));
        var angle_0 : f32 = _S80 * 2.0f * 3.14159274101257324f;
        var _S81 : f32 = RandomFloat01_0(&(rngState_1));
        var offset_0 : vec2<f32> = vec2<f32>(cos(angle_0), sin(angle_0)) * vec2<f32>((sqrt(_S81) * _PathTraceCB.apertureRadius_0));
        var rayOrigin_2 : vec3<f32> = rayOrigin_0 + (vec3<f32>(offset_0.x) * cameraRight_0 + vec3<f32>(offset_0.y) * cameraUp_0);
        rayDirection_1 = normalize(focalPlanePoint_0 - rayOrigin_2);
        rayOrigin_1 = rayOrigin_2;
    }
    else
    {
        rayDirection_1 = rayDirection_0;
        rayOrigin_1 = rayOrigin_0;
    }
    var color_0 : vec3<f32> = GetColorForRay_0(rayOrigin_1, rayDirection_1, &(rngState_1));
    var _S82 : vec4<f32> = (textureLoad((accumulationReadOnly), (vec2<i32>(_S73))));
    var _S83 : bool;
    if((_PathTraceCB.iFrame_0) < i32(2))
    {
        _S83 = true;
    }
    else
    {
        _S83 = bool(_PathTraceCB.Reset_0);
    }
    if(_S83)
    {
        _S83 = true;
    }
    else
    {
        _S83 = (_S82.w) == 0.0f;
    }
    if(_S83)
    {
        _S83 = true;
    }
    else
    {
        _S83 = bool(_PathTraceCB.CameraChanged_0);
    }
    var blend_0 : f32;
    if(_S83)
    {
        blend_0 = 1.0f;
    }
    else
    {
        blend_0 = 1.0f / (1.0f + 1.0f / _S82.w);
    }
    textureStore((accumulation), (_S73), (vec4<f32>(mix(_S82.xyz, color_0, vec3<f32>(blend_0)), blend_0)));
    return;
}

`;

// Shader code for Compute shader "Bloom_Horizontal", node "Bloom_Horizontal"
static ShaderCode_Bloom_Horizontal_Bloom_Horizontal = `
@binding(0) @group(0) var accumulation : texture_2d<f32>;

@binding(1) @group(0) var scratch : texture_storage_2d</*(scratch_format)*/, write>;

const c_bloomKernel_0 : array<f32, i32(6)> = array<f32, i32(6)>( 0.1630530059337616f, 0.15067699551582336f, 0.1189040020108223f, 0.08012700080871582f, 0.04610799998044968f, 0.02265699952840805f );
fn BloomPass1_0( pixelPos_0 : vec2<f32>,  linearImage_0 : texture_2d<f32>) -> vec3<f32>
{
    const _S1 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var ix_0 : i32 = i32(-5);
    var sum_0 : vec3<f32> = _S1;
    for(;;)
    {
        if(ix_0 <= i32(5))
        {
        }
        else
        {
            break;
        }
        var _S2 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(pixelPos_0 + vec2<f32>(f32(ix_0), 0.0f)), u32(0)));
        var bloomColor_0 : vec3<f32> = clamp((textureLoad((linearImage_0), ((_S2)).xy, ((_S2)).z)).xyz - vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(0.0f), vec3<f32>(1.0f));
        var bloomColor_1 : vec3<f32>;
        if((dot(bloomColor_0, vec3<f32>(1.0f, 1.0f, 1.0f))) > 0.0f)
        {
            bloomColor_1 = bloomColor_0;
        }
        else
        {
            bloomColor_1 = _S1;
        }
        var sum_1 : vec3<f32> = sum_0 + bloomColor_1 * vec3<f32>(c_bloomKernel_0[abs(ix_0)]);
        ix_0 = ix_0 + i32(1);
        sum_0 = sum_1;
    }
    return sum_0;
}

@compute
@workgroup_size(8, 8, 1)
fn bloomh(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S3 : vec2<u32> = DTid_0.xy;
    textureStore((scratch), (_S3), (vec4<f32>(BloomPass1_0(vec2<f32>(_S3), accumulation), 1.0f)));
    return;
}

`;

// Shader code for Compute shader "Bloom_Vertical_and_Finalize", node "Bloom_Vertical_and_Finalize"
static ShaderCode_Bloom_Vertical_and_Finalize_Bloom_Vertical_and_Finalize = `
@binding(1) @group(0) var accumulation : texture_2d<f32>;

@binding(2) @group(0) var scratch : texture_2d<f32>;

struct Struct_Bloom_Vertical_and_FinalizeCB_std140_0
{
    @align(16) ACESToneMapping_0 : u32,
    @align(4) exposure_0 : f32,
    @align(8) sRGB_0 : u32,
    @align(4) _padding0_0 : f32,
};

@binding(4) @group(0) var<uniform> _Bloom_Vertical_and_FinalizeCB : Struct_Bloom_Vertical_and_FinalizeCB_std140_0;
@binding(0) @group(0) var output : texture_storage_2d</*(output_format)*/, write>;

const c_bloomKernel_0 : array<f32, i32(6)> = array<f32, i32(6)>( 0.1630530059337616f, 0.15067699551582336f, 0.1189040020108223f, 0.08012700080871582f, 0.04610799998044968f, 0.02265699952840805f );
fn BloomPass2_0( pixelPos_0 : vec2<f32>,  bloomImage_0 : texture_2d<f32>) -> vec3<f32>
{
    const _S1 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var iy_0 : i32 = i32(-5);
    var sum_0 : vec3<f32> = _S1;
    for(;;)
    {
        if(iy_0 <= i32(5))
        {
        }
        else
        {
            break;
        }
        var _S2 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(pixelPos_0 + vec2<f32>(0.0f, f32(iy_0))), u32(0)));
        var sum_1 : vec3<f32> = sum_0 + (textureLoad((bloomImage_0), ((_S2)).xy, ((_S2)).z)).xyz * vec3<f32>(c_bloomKernel_0[abs(iy_0)]);
        iy_0 = iy_0 + i32(1);
        sum_0 = sum_1;
    }
    return sum_0;
}

fn ACESFilm_0( x_0 : vec3<f32>) -> vec3<f32>
{
    return clamp(x_0 * (vec3<f32>(2.50999999046325684f) * x_0 + vec3<f32>(0.02999999932944775f)) / (x_0 * (vec3<f32>(2.43000006675720215f) * x_0 + vec3<f32>(0.5899999737739563f)) + vec3<f32>(0.14000000059604645f)), vec3<f32>(0.0f), vec3<f32>(1.0f));
}

fn LessThan_0( f_0 : vec3<f32>,  value_0 : f32) -> vec3<f32>
{
    var _S3 : f32;
    if((f_0.x) < value_0)
    {
        _S3 = 1.0f;
    }
    else
    {
        _S3 = 0.0f;
    }
    var _S4 : f32;
    if((f_0.y) < value_0)
    {
        _S4 = 1.0f;
    }
    else
    {
        _S4 = 0.0f;
    }
    var _S5 : f32;
    if((f_0.z) < value_0)
    {
        _S5 = 1.0f;
    }
    else
    {
        _S5 = 0.0f;
    }
    return vec3<f32>(_S3, _S4, _S5);
}

fn LinearToSRGB_0( rgb_0 : vec3<f32>) -> vec3<f32>
{
    var _S6 : vec3<f32> = clamp(rgb_0, vec3<f32>(0.0f), vec3<f32>(1.0f));
    return mix(pow(_S6 * vec3<f32>(1.0549999475479126f), vec3<f32>(0.4166666567325592f)) - vec3<f32>(0.05499999970197678f), _S6 * vec3<f32>(12.92000007629394531f), LessThan_0(_S6, 0.00313080009073019f));
}

@compute
@workgroup_size(8, 8, 1)
fn finalize(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S7 : vec2<u32> = DTid_0.xy;
    var _S8 : vec3<i32> = vec3<i32>(vec3<u32>(_S7, u32(0)));
    var pixelColor_0 : vec3<f32> = ((textureLoad((accumulation), ((_S8)).xy, ((_S8)).z)).xyz + BloomPass2_0(vec2<f32>(_S7), scratch)) * vec3<f32>(pow(2.0f, _Bloom_Vertical_and_FinalizeCB.exposure_0));
    var pixelColor_1 : vec3<f32>;
    if(bool(_Bloom_Vertical_and_FinalizeCB.ACESToneMapping_0))
    {
        pixelColor_1 = ACESFilm_0(pixelColor_0);
    }
    else
    {
        pixelColor_1 = pixelColor_0;
    }
    if(bool(_Bloom_Vertical_and_FinalizeCB.sRGB_0))
    {
        pixelColor_1 = LinearToSRGB_0(pixelColor_1);
    }
    else
    {
    }
    textureStore((output), (_S7), (vec4<f32>(pixelColor_1, 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture Accumulation
texture_Accumulation = null;
texture_Accumulation_size = [0, 0, 0];
texture_Accumulation_format = "";
texture_Accumulation_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture Bloom_Scratch
texture_Bloom_Scratch = null;
texture_Bloom_Scratch_size = [0, 0, 0];
texture_Bloom_Scratch_format = "";
texture_Bloom_Scratch_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture PathTrace_accumulation_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_PathTrace_accumulation_ReadOnly = null;
texture_PathTrace_accumulation_ReadOnly_size = [0, 0, 0];
texture_PathTrace_accumulation_ReadOnly_format = "";
texture_PathTrace_accumulation_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Bloom_Horizontal_scratch_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Bloom_Horizontal_scratch_ReadOnly = null;
texture_Bloom_Horizontal_scratch_ReadOnly_size = [0, 0, 0];
texture_Bloom_Horizontal_scratch_ReadOnly_format = "";
texture_Bloom_Horizontal_scratch_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Bloom_Vertical_and_Finalize_output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Bloom_Vertical_and_Finalize_output_ReadOnly = null;
texture_Bloom_Vertical_and_Finalize_output_ReadOnly_size = [0, 0, 0];
texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format = "";
texture_Bloom_Vertical_and_Finalize_output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _PathTraceCB
constantBuffer__PathTraceCB = null;
constantBuffer__PathTraceCB_size = 192;
constantBuffer__PathTraceCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader PathTrace
Hash_Compute_PathTrace = 0;
ShaderModule_Compute_PathTrace = null;
BindGroupLayout_Compute_PathTrace = null;
PipelineLayout_Compute_PathTrace = null;
Pipeline_Compute_PathTrace = null;

// Compute Shader Bloom_Horizontal
Hash_Compute_Bloom_Horizontal = 0;
ShaderModule_Compute_Bloom_Horizontal = null;
BindGroupLayout_Compute_Bloom_Horizontal = null;
PipelineLayout_Compute_Bloom_Horizontal = null;
Pipeline_Compute_Bloom_Horizontal = null;

// Constant buffer _Bloom_Vertical_and_FinalizeCB
constantBuffer__Bloom_Vertical_and_FinalizeCB = null;
constantBuffer__Bloom_Vertical_and_FinalizeCB_size = 16;
constantBuffer__Bloom_Vertical_and_FinalizeCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Bloom_Vertical_and_Finalize
Hash_Compute_Bloom_Vertical_and_Finalize = 0;
ShaderModule_Compute_Bloom_Vertical_and_Finalize = null;
BindGroupLayout_Compute_Bloom_Vertical_and_Finalize = null;
PipelineLayout_Compute_Bloom_Vertical_and_Finalize = null;
Pipeline_Compute_Bloom_Vertical_and_Finalize = null;

// -------------------- Exported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_RenderSize = [ 1280, 720 ];
variableDefault_RenderSize = [ 1280, 720 ];
variableChanged_RenderSize = [ false, false ];
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
variable_Reset = false;
variableDefault_Reset = false;
variableChanged_Reset = false;
variable_exposure = 0.000000;  // in FStops. -inf to +inf.
variableDefault_exposure = 0.000000;  // in FStops. -inf to +inf.
variableChanged_exposure = false;
variable_apertureRadius = 0.400000;  // in world units. 0 for pinhole camera.
variableDefault_apertureRadius = 0.400000;  // in world units. 0 for pinhole camera.
variableChanged_apertureRadius = false;
variable_focalPlaneDistance = 80.000000;  // in world units. How far from the camera things are in focus at.
variableDefault_focalPlaneDistance = 80.000000;  // in world units. How far from the camera things are in focus at.
variableChanged_focalPlaneDistance = false;
variable_numBounces = 4;
variableDefault_numBounces = 4;
variableChanged_numBounces = false;
variable_rayMarchSteps = 256;
variableDefault_rayMarchSteps = 256;
variableChanged_rayMarchSteps = false;
variable_minStepDistance = 0.100000;
variableDefault_minStepDistance = 0.100000;
variableChanged_minStepDistance = false;
variable_ACESToneMapping = true;
variableDefault_ACESToneMapping = true;
variableChanged_ACESToneMapping = false;
variable_sRGB = true;
variableDefault_sRGB = true;
variableChanged_sRGB = false;

// -------------------- Structs

static StructOffsets__PathTraceCB =
{
    CameraChanged: 0,
    _padding0: 4,
    _padding1: 8,
    _padding2: 12,
    CameraPos_0: 16,
    CameraPos_1: 20,
    CameraPos_2: 24,
    _padding3: 28,
    InvViewMtx_0: 32,
    InvViewMtx_1: 36,
    InvViewMtx_2: 40,
    InvViewMtx_3: 44,
    InvViewMtx_4: 48,
    InvViewMtx_5: 52,
    InvViewMtx_6: 56,
    InvViewMtx_7: 60,
    InvViewMtx_8: 64,
    InvViewMtx_9: 68,
    InvViewMtx_10: 72,
    InvViewMtx_11: 76,
    InvViewMtx_12: 80,
    InvViewMtx_13: 84,
    InvViewMtx_14: 88,
    InvViewMtx_15: 92,
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
    Reset: 160,
    apertureRadius: 164,
    focalPlaneDistance: 168,
    iFrame: 172,
    minStepDistance: 176,
    numBounces: 180,
    rayMarchSteps: 184,
    _padding4: 188,
    _size: 192,
}

static StructOffsets__Bloom_Vertical_and_FinalizeCB =
{
    ACESToneMapping: 0,
    exposure: 4,
    sRGB: 8,
    _padding0: 12,
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
                label: "texture DemofoxNeonDesert.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Accumulation
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba32float";
        if (this.texture_Accumulation !== null && (this.texture_Accumulation_format != desiredFormat || this.texture_Accumulation_size[0] != desiredSize[0] || this.texture_Accumulation_size[1] != desiredSize[1] || this.texture_Accumulation_size[2] != desiredSize[2]))
        {
            this.texture_Accumulation.destroy();
            this.texture_Accumulation = null;
        }

        if (this.texture_Accumulation === null)
        {
            this.texture_Accumulation_size = desiredSize.slice();
            this.texture_Accumulation_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Accumulation_format))
                viewFormats.push(this.texture_Accumulation_format);

            this.texture_Accumulation = device.createTexture({
                label: "texture DemofoxNeonDesert.Accumulation",
                size: this.texture_Accumulation_size,
                format: Shared.GetNonSRGBFormat(this.texture_Accumulation_format),
                usage: this.texture_Accumulation_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Bloom_Scratch
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba32float";
        if (this.texture_Bloom_Scratch !== null && (this.texture_Bloom_Scratch_format != desiredFormat || this.texture_Bloom_Scratch_size[0] != desiredSize[0] || this.texture_Bloom_Scratch_size[1] != desiredSize[1] || this.texture_Bloom_Scratch_size[2] != desiredSize[2]))
        {
            this.texture_Bloom_Scratch.destroy();
            this.texture_Bloom_Scratch = null;
        }

        if (this.texture_Bloom_Scratch === null)
        {
            this.texture_Bloom_Scratch_size = desiredSize.slice();
            this.texture_Bloom_Scratch_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Bloom_Scratch_format))
                viewFormats.push(this.texture_Bloom_Scratch_format);

            this.texture_Bloom_Scratch = device.createTexture({
                label: "texture DemofoxNeonDesert.Bloom_Scratch",
                size: this.texture_Bloom_Scratch_size,
                format: Shared.GetNonSRGBFormat(this.texture_Bloom_Scratch_format),
                usage: this.texture_Bloom_Scratch_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture PathTrace_accumulation_ReadOnly
    {
        const baseSize = this.texture_Accumulation_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Accumulation_format;
        if (this.texture_PathTrace_accumulation_ReadOnly !== null && (this.texture_PathTrace_accumulation_ReadOnly_format != desiredFormat || this.texture_PathTrace_accumulation_ReadOnly_size[0] != desiredSize[0] || this.texture_PathTrace_accumulation_ReadOnly_size[1] != desiredSize[1] || this.texture_PathTrace_accumulation_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_PathTrace_accumulation_ReadOnly.destroy();
            this.texture_PathTrace_accumulation_ReadOnly = null;
        }

        if (this.texture_PathTrace_accumulation_ReadOnly === null)
        {
            this.texture_PathTrace_accumulation_ReadOnly_size = desiredSize.slice();
            this.texture_PathTrace_accumulation_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_PathTrace_accumulation_ReadOnly_format))
                viewFormats.push(this.texture_PathTrace_accumulation_ReadOnly_format);

            this.texture_PathTrace_accumulation_ReadOnly = device.createTexture({
                label: "texture DemofoxNeonDesert.PathTrace_accumulation_ReadOnly",
                size: this.texture_PathTrace_accumulation_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_PathTrace_accumulation_ReadOnly_format),
                usage: this.texture_PathTrace_accumulation_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Bloom_Horizontal_scratch_ReadOnly
    {
        const baseSize = this.texture_Bloom_Scratch_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Bloom_Scratch_format;
        if (this.texture_Bloom_Horizontal_scratch_ReadOnly !== null && (this.texture_Bloom_Horizontal_scratch_ReadOnly_format != desiredFormat || this.texture_Bloom_Horizontal_scratch_ReadOnly_size[0] != desiredSize[0] || this.texture_Bloom_Horizontal_scratch_ReadOnly_size[1] != desiredSize[1] || this.texture_Bloom_Horizontal_scratch_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Bloom_Horizontal_scratch_ReadOnly.destroy();
            this.texture_Bloom_Horizontal_scratch_ReadOnly = null;
        }

        if (this.texture_Bloom_Horizontal_scratch_ReadOnly === null)
        {
            this.texture_Bloom_Horizontal_scratch_ReadOnly_size = desiredSize.slice();
            this.texture_Bloom_Horizontal_scratch_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Bloom_Horizontal_scratch_ReadOnly_format))
                viewFormats.push(this.texture_Bloom_Horizontal_scratch_ReadOnly_format);

            this.texture_Bloom_Horizontal_scratch_ReadOnly = device.createTexture({
                label: "texture DemofoxNeonDesert.Bloom_Horizontal_scratch_ReadOnly",
                size: this.texture_Bloom_Horizontal_scratch_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Bloom_Horizontal_scratch_ReadOnly_format),
                usage: this.texture_Bloom_Horizontal_scratch_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Bloom_Vertical_and_Finalize_output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly !== null && (this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format != desiredFormat || this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_size[0] != desiredSize[0] || this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_size[1] != desiredSize[1] || this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly.destroy();
            this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly = null;
        }

        if (this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly === null)
        {
            this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_size = desiredSize.slice();
            this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format))
                viewFormats.push(this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format);

            this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly = device.createTexture({
                label: "texture DemofoxNeonDesert.Bloom_Vertical_and_Finalize_output_ReadOnly",
                size: this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format),
                usage: this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader PathTrace
    {
        const bindGroupEntries =
        [
            {
                // accumulation
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Accumulation_format), viewDimension: "2d" }
            },
            {
                // accumulationReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_PathTrace_accumulation_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _PathTraceCB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_PathTrace === null || newHash !== this.Hash_Compute_PathTrace)
        {
            this.Hash_Compute_PathTrace = newHash;

            let shaderCode = class_DemofoxNeonDesert.ShaderCode_PathTrace_PathTrace;
            shaderCode = shaderCode.replace("/*(accumulation_format)*/", Shared.GetNonSRGBFormat(this.texture_Accumulation_format));
            shaderCode = shaderCode.replace("/*(accumulationReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_PathTrace_accumulation_ReadOnly_format));

            this.ShaderModule_Compute_PathTrace = device.createShaderModule({ code: shaderCode, label: "Compute Shader PathTrace"});
            this.BindGroupLayout_Compute_PathTrace = device.createBindGroupLayout({
                label: "Compute Bind Group Layout PathTrace",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_PathTrace = device.createPipelineLayout({
                label: "Compute Pipeline Layout PathTrace",
                bindGroupLayouts: [this.BindGroupLayout_Compute_PathTrace],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_PathTrace = device.createComputePipeline({
                    label: "Compute Pipeline PathTrace",
                    layout: this.PipelineLayout_Compute_PathTrace,
                    compute: {
                        module: this.ShaderModule_Compute_PathTrace,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("PathTrace");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline PathTrace",
                    layout: this.PipelineLayout_Compute_PathTrace,
                    compute: {
                        module: this.ShaderModule_Compute_PathTrace,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_PathTrace = handle; this.loadingPromises.delete("PathTrace"); } );
            }
        }
    }

    // (Re)create compute shader Bloom_Horizontal
    {
        const bindGroupEntries =
        [
            {
                // accumulation
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Accumulation_format).sampleType }
            },
            {
                // scratch
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Bloom_Scratch_format), viewDimension: "2d" }
            },
            {
                // scratchReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Bloom_Horizontal_scratch_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Bloom_Horizontal === null || newHash !== this.Hash_Compute_Bloom_Horizontal)
        {
            this.Hash_Compute_Bloom_Horizontal = newHash;

            let shaderCode = class_DemofoxNeonDesert.ShaderCode_Bloom_Horizontal_Bloom_Horizontal;
            shaderCode = shaderCode.replace("/*(scratch_format)*/", Shared.GetNonSRGBFormat(this.texture_Bloom_Scratch_format));
            shaderCode = shaderCode.replace("/*(scratchReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Bloom_Horizontal_scratch_ReadOnly_format));

            this.ShaderModule_Compute_Bloom_Horizontal = device.createShaderModule({ code: shaderCode, label: "Compute Shader Bloom_Horizontal"});
            this.BindGroupLayout_Compute_Bloom_Horizontal = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Bloom_Horizontal",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Bloom_Horizontal = device.createPipelineLayout({
                label: "Compute Pipeline Layout Bloom_Horizontal",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Bloom_Horizontal],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Bloom_Horizontal = device.createComputePipeline({
                    label: "Compute Pipeline Bloom_Horizontal",
                    layout: this.PipelineLayout_Compute_Bloom_Horizontal,
                    compute: {
                        module: this.ShaderModule_Compute_Bloom_Horizontal,
                        entryPoint: "bloomh",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Bloom_Horizontal");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Bloom_Horizontal",
                    layout: this.PipelineLayout_Compute_Bloom_Horizontal,
                    compute: {
                        module: this.ShaderModule_Compute_Bloom_Horizontal,
                        entryPoint: "bloomh",
                    }
                }).then( handle => { this.Pipeline_Compute_Bloom_Horizontal = handle; this.loadingPromises.delete("Bloom_Horizontal"); } );
            }
        }
    }

    // (Re)create compute shader Bloom_Vertical_and_Finalize
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
                // accumulation
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Accumulation_format).sampleType }
            },
            {
                // scratch
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Bloom_Scratch_format).sampleType }
            },
            {
                // outputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _Bloom_Vertical_and_FinalizeCB
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Bloom_Vertical_and_Finalize === null || newHash !== this.Hash_Compute_Bloom_Vertical_and_Finalize)
        {
            this.Hash_Compute_Bloom_Vertical_and_Finalize = newHash;

            let shaderCode = class_DemofoxNeonDesert.ShaderCode_Bloom_Vertical_and_Finalize_Bloom_Vertical_and_Finalize;
            shaderCode = shaderCode.replace("/*(output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(outputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly_format));

            this.ShaderModule_Compute_Bloom_Vertical_and_Finalize = device.createShaderModule({ code: shaderCode, label: "Compute Shader Bloom_Vertical_and_Finalize"});
            this.BindGroupLayout_Compute_Bloom_Vertical_and_Finalize = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Bloom_Vertical_and_Finalize",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Bloom_Vertical_and_Finalize = device.createPipelineLayout({
                label: "Compute Pipeline Layout Bloom_Vertical_and_Finalize",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Bloom_Vertical_and_Finalize],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Bloom_Vertical_and_Finalize = device.createComputePipeline({
                    label: "Compute Pipeline Bloom_Vertical_and_Finalize",
                    layout: this.PipelineLayout_Compute_Bloom_Vertical_and_Finalize,
                    compute: {
                        module: this.ShaderModule_Compute_Bloom_Vertical_and_Finalize,
                        entryPoint: "finalize",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Bloom_Vertical_and_Finalize");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Bloom_Vertical_and_Finalize",
                    layout: this.PipelineLayout_Compute_Bloom_Vertical_and_Finalize,
                    compute: {
                        module: this.ShaderModule_Compute_Bloom_Vertical_and_Finalize,
                        entryPoint: "finalize",
                    }
                }).then( handle => { this.Pipeline_Compute_Bloom_Vertical_and_Finalize = handle; this.loadingPromises.delete("Bloom_Vertical_and_Finalize"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("DemofoxNeonDesert.Output");

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Output"

    encoder.pushDebugGroup("DemofoxNeonDesert.Accumulation");

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Accumulation"

    encoder.pushDebugGroup("DemofoxNeonDesert.Bloom_Scratch");

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Bloom_Scratch"

    encoder.pushDebugGroup("DemofoxNeonDesert.PathTrace_accumulation_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxNeonDesert.PathTrace_accumulation_ReadOnly"

    encoder.pushDebugGroup("DemofoxNeonDesert.Copy_PathTrace_accumulation");

    // Copy texture Accumulation to texture PathTrace_accumulation_ReadOnly
    {
        const numMips = Math.min(this.texture_Accumulation.mipLevelCount, this.texture_PathTrace_accumulation_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Accumulation.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Accumulation.height >> mipIndex, 1);
            let mipDepth = this.texture_Accumulation.depthOrArrayLayers;

            if (this.texture_Accumulation.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Accumulation, mipLevel: mipIndex },
                { texture: this.texture_PathTrace_accumulation_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Copy_PathTrace_accumulation"

    encoder.pushDebugGroup("DemofoxNeonDesert.Bloom_Horizontal_scratch_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Bloom_Horizontal_scratch_ReadOnly"

    encoder.pushDebugGroup("DemofoxNeonDesert.Copy_Bloom_Horizontal_scratch");

    // Copy texture Bloom_Scratch to texture Bloom_Horizontal_scratch_ReadOnly
    {
        const numMips = Math.min(this.texture_Bloom_Scratch.mipLevelCount, this.texture_Bloom_Horizontal_scratch_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Bloom_Scratch.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Bloom_Scratch.height >> mipIndex, 1);
            let mipDepth = this.texture_Bloom_Scratch.depthOrArrayLayers;

            if (this.texture_Bloom_Scratch.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Bloom_Scratch, mipLevel: mipIndex },
                { texture: this.texture_Bloom_Horizontal_scratch_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Copy_Bloom_Horizontal_scratch"

    encoder.pushDebugGroup("DemofoxNeonDesert.Bloom_Vertical_and_Finalize_output_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Bloom_Vertical_and_Finalize_output_ReadOnly"

    encoder.pushDebugGroup("DemofoxNeonDesert.Copy_Bloom_Vertical_and_Finalize_output");

    // Copy texture Output to texture Bloom_Vertical_and_Finalize_output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Copy_Bloom_Vertical_and_Finalize_output"

    encoder.pushDebugGroup("DemofoxNeonDesert._PathTraceCB");

    // Create constant buffer _PathTraceCB
    if (this.constantBuffer__PathTraceCB === null)
    {
        this.constantBuffer__PathTraceCB = device.createBuffer({
            label: "DemofoxNeonDesert._PathTraceCB",
            size: Shared.Align(16, this.constructor.StructOffsets__PathTraceCB._size),
            usage: this.constantBuffer__PathTraceCB_usageFlags,
        });
    }

    // Upload values to constant buffer _PathTraceCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__PathTraceCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__PathTraceCB.CameraChanged, (this.variable_CameraChanged === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.CameraPos_0, this.variable_CameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.CameraPos_1, this.variable_CameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.CameraPos_2, this.variable_CameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_0, this.variable_InvViewMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_1, this.variable_InvViewMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_2, this.variable_InvViewMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_3, this.variable_InvViewMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_4, this.variable_InvViewMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_5, this.variable_InvViewMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_6, this.variable_InvViewMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_7, this.variable_InvViewMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_8, this.variable_InvViewMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_9, this.variable_InvViewMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_10, this.variable_InvViewMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_11, this.variable_InvViewMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_12, this.variable_InvViewMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_13, this.variable_InvViewMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_14, this.variable_InvViewMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewMtx_15, this.variable_InvViewMtx[15], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_0, this.variable_InvViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_1, this.variable_InvViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_2, this.variable_InvViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_3, this.variable_InvViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_4, this.variable_InvViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_5, this.variable_InvViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_6, this.variable_InvViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_7, this.variable_InvViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_8, this.variable_InvViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_9, this.variable_InvViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_10, this.variable_InvViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_11, this.variable_InvViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_12, this.variable_InvViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_13, this.variable_InvViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_14, this.variable_InvViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.InvViewProjMtx_15, this.variable_InvViewProjMtx[15], true);
        view.setUint32(this.constructor.StructOffsets__PathTraceCB.Reset, (this.variable_Reset === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.apertureRadius, this.variable_apertureRadius, true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.focalPlaneDistance, this.variable_focalPlaneDistance, true);
        view.setInt32(this.constructor.StructOffsets__PathTraceCB.iFrame, this.variable_iFrame, true);
        view.setFloat32(this.constructor.StructOffsets__PathTraceCB.minStepDistance, this.variable_minStepDistance, true);
        view.setInt32(this.constructor.StructOffsets__PathTraceCB.numBounces, this.variable_numBounces, true);
        view.setInt32(this.constructor.StructOffsets__PathTraceCB.rayMarchSteps, this.variable_rayMarchSteps, true);
        device.queue.writeBuffer(this.constantBuffer__PathTraceCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert._PathTraceCB"

    encoder.pushDebugGroup("DemofoxNeonDesert.PathTrace");

    // Run compute shader PathTrace
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group PathTrace",
            layout: this.BindGroupLayout_Compute_PathTrace,
            entries: [
                {
                    // accumulation
                    binding: 0,
                    resource: this.texture_Accumulation.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // accumulationReadOnly
                    binding: 1,
                    resource: this.texture_PathTrace_accumulation_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _PathTraceCB
                    binding: 2,
                    resource: { buffer: this.constantBuffer__PathTraceCB }
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

        if (this.Pipeline_Compute_PathTrace !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_PathTrace);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert.PathTrace"

    encoder.pushDebugGroup("DemofoxNeonDesert.Bloom_Horizontal");

    // Run compute shader Bloom_Horizontal
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Bloom_Horizontal",
            layout: this.BindGroupLayout_Compute_Bloom_Horizontal,
            entries: [
                {
                    // accumulation
                    binding: 0,
                    resource: this.texture_Accumulation.createView({ dimension: "2d", format: this.texture_Accumulation_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // scratch
                    binding: 1,
                    resource: this.texture_Bloom_Scratch.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // scratchReadOnly
                    binding: 2,
                    resource: this.texture_Bloom_Horizontal_scratch_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Accumulation_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Bloom_Horizontal !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Bloom_Horizontal);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Bloom_Horizontal"

    encoder.pushDebugGroup("DemofoxNeonDesert._Bloom_Vertical_and_FinalizeCB");

    // Create constant buffer _Bloom_Vertical_and_FinalizeCB
    if (this.constantBuffer__Bloom_Vertical_and_FinalizeCB === null)
    {
        this.constantBuffer__Bloom_Vertical_and_FinalizeCB = device.createBuffer({
            label: "DemofoxNeonDesert._Bloom_Vertical_and_FinalizeCB",
            size: Shared.Align(16, this.constructor.StructOffsets__Bloom_Vertical_and_FinalizeCB._size),
            usage: this.constantBuffer__Bloom_Vertical_and_FinalizeCB_usageFlags,
        });
    }

    // Upload values to constant buffer _Bloom_Vertical_and_FinalizeCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Bloom_Vertical_and_FinalizeCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__Bloom_Vertical_and_FinalizeCB.ACESToneMapping, (this.variable_ACESToneMapping === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__Bloom_Vertical_and_FinalizeCB.exposure, this.variable_exposure, true);
        view.setUint32(this.constructor.StructOffsets__Bloom_Vertical_and_FinalizeCB.sRGB, (this.variable_sRGB === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__Bloom_Vertical_and_FinalizeCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert._Bloom_Vertical_and_FinalizeCB"

    encoder.pushDebugGroup("DemofoxNeonDesert.Bloom_Vertical_and_Finalize");

    // Run compute shader Bloom_Vertical_and_Finalize
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Bloom_Vertical_and_Finalize",
            layout: this.BindGroupLayout_Compute_Bloom_Vertical_and_Finalize,
            entries: [
                {
                    // output
                    binding: 0,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // accumulation
                    binding: 1,
                    resource: this.texture_Accumulation.createView({ dimension: "2d", format: this.texture_Accumulation_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // scratch
                    binding: 2,
                    resource: this.texture_Bloom_Scratch.createView({ dimension: "2d", format: this.texture_Bloom_Scratch_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // outputReadOnly
                    binding: 3,
                    resource: this.texture_Bloom_Vertical_and_Finalize_output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _Bloom_Vertical_and_FinalizeCB
                    binding: 4,
                    resource: { buffer: this.constantBuffer__Bloom_Vertical_and_FinalizeCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Accumulation_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Bloom_Vertical_and_Finalize !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Bloom_Vertical_and_Finalize);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxNeonDesert.Bloom_Vertical_and_Finalize"

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

var DemofoxNeonDesert = new class_DemofoxNeonDesert;

export default DemofoxNeonDesert;
