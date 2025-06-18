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

class class_CasualPathtracing
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "RenderSceneCS", node "RenderScene"
static ShaderCode_RenderScene_RenderSceneCS = `
@binding(1) @group(0) var Output : texture_storage_2d</*(Output_format)*/, write>;

struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct Struct_RenderSceneCSCB_std140_0
{
    @align(16) CameraChanged_0 : u32,
    @align(4) _padding0_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
    @align(16) CameraPos_0 : vec3<f32>,
    @align(4) Exposure_0 : f32,
    @align(16) FrameIndex_0 : u32,
    @align(4) _padding3_0 : f32,
    @align(8) _padding4_0 : f32,
    @align(4) _padding5_0 : f32,
    @align(16) InvViewProjMtx_0 : _MatrixStorage_float4x4std140_0,
    @align(16) NumBounces_0 : i32,
    @align(4) RaysPerPixel_0 : i32,
    @align(8) Reset_0 : u32,
    @align(4) Scene_0 : i32,
    @align(16) SceneLastFrame_0 : i32,
    @align(4) SkyboxBrightness_0 : f32,
    @align(8) _padding6_0 : f32,
    @align(4) _padding7_0 : f32,
};

@binding(5) @group(0) var<uniform> _RenderSceneCSCB : Struct_RenderSceneCSCB_std140_0;
@binding(4) @group(0) var _loadedTexture_0 : texture_2d<f32>;

@binding(6) @group(0) var texSampler : sampler;

@binding(2) @group(0) var AccumReadOnly : texture_storage_2d</*(AccumReadOnly_format)*/, read>;

@binding(0) @group(0) var Accum : texture_storage_2d</*(Accum_format)*/, write>;

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

struct SMaterialInfo_0
{
     albedo_0 : vec3<f32>,
     emissive_0 : vec3<f32>,
     specularChance_0 : f32,
     specularRoughness_0 : f32,
     specularColor_0 : vec3<f32>,
     IOR_0 : f32,
     refractionChance_0 : f32,
     refractionRoughness_0 : f32,
     refractionColor_0 : vec3<f32>,
};

fn GetZeroedMaterial_0() -> SMaterialInfo_0
{
    var ret_0 : SMaterialInfo_0;
    const _S5 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    ret_0.albedo_0 = _S5;
    ret_0.emissive_0 = _S5;
    ret_0.specularChance_0 = 0.0f;
    ret_0.specularRoughness_0 = 0.0f;
    ret_0.specularColor_0 = _S5;
    ret_0.IOR_0 = 1.0f;
    ret_0.refractionChance_0 = 0.0f;
    ret_0.refractionRoughness_0 = 0.0f;
    ret_0.refractionColor_0 = _S5;
    return ret_0;
}

fn ScalarTriple_0( u_0 : vec3<f32>,  v_0 : vec3<f32>,  w_0 : vec3<f32>) -> f32
{
    return dot(cross(u_0, v_0), w_0);
}

struct SRayHitInfo_0
{
     fromInside_0 : bool,
     dist_0 : f32,
     normal_0 : vec3<f32>,
     material_0 : SMaterialInfo_0,
};

fn TestQuadTrace_0( rayPos_0 : vec3<f32>,  rayDir_0 : vec3<f32>,  info_0 : ptr<function, SRayHitInfo_0>,  a_0 : vec3<f32>,  b_0 : vec3<f32>,  c_0 : vec3<f32>,  d_0 : vec3<f32>) -> bool
{
    var normal_1 : vec3<f32> = normalize(cross(c_0 - a_0, c_0 - b_0));
    var intersectPos_0 : vec3<f32>;
    var _S6 : vec3<f32>;
    var _S7 : vec3<f32>;
    var _S8 : vec3<f32>;
    var normal_2 : vec3<f32>;
    if((dot(normal_1, rayDir_0)) > 0.0f)
    {
        var normal_3 : vec3<f32> = normal_1 * vec3<f32>(-1.0f);
        intersectPos_0 = d_0;
        _S6 = c_0;
        _S7 = b_0;
        _S8 = a_0;
        normal_2 = normal_3;
    }
    else
    {
        intersectPos_0 = a_0;
        _S6 = b_0;
        _S7 = c_0;
        _S8 = d_0;
        normal_2 = normal_1;
    }
    var pq_0 : vec3<f32> = rayPos_0 + rayDir_0 - rayPos_0;
    var pa_0 : vec3<f32> = intersectPos_0 - rayPos_0;
    var pb_0 : vec3<f32> = _S6 - rayPos_0;
    var m_0 : vec3<f32> = cross(_S7 - rayPos_0, pq_0);
    var v_1 : f32 = dot(pa_0, m_0);
    if(v_1 >= 0.0f)
    {
        var u_1 : f32 = - dot(pb_0, m_0);
        if(u_1 < 0.0f)
        {
            return false;
        }
        var w_1 : f32 = ScalarTriple_0(pq_0, pb_0, pa_0);
        if(w_1 < 0.0f)
        {
            return false;
        }
        var denom_0 : f32 = 1.0f / (u_1 + v_1 + w_1);
        intersectPos_0 = vec3<f32>((u_1 * denom_0)) * intersectPos_0 + vec3<f32>((v_1 * denom_0)) * _S6 + vec3<f32>((w_1 * denom_0)) * _S7;
    }
    else
    {
        var pd_0 : vec3<f32> = _S8 - rayPos_0;
        var u_2 : f32 = dot(pd_0, m_0);
        if(u_2 < 0.0f)
        {
            return false;
        }
        var w_2 : f32 = ScalarTriple_0(pq_0, pa_0, pd_0);
        if(w_2 < 0.0f)
        {
            return false;
        }
        var v_2 : f32 = - v_1;
        var denom_1 : f32 = 1.0f / (u_2 + v_2 + w_2);
        intersectPos_0 = vec3<f32>((u_2 * denom_1)) * intersectPos_0 + vec3<f32>((v_2 * denom_1)) * _S8 + vec3<f32>((w_2 * denom_1)) * _S7;
    }
    var _S9 : f32 = rayDir_0.x;
    var dist_1 : f32;
    if((abs(_S9)) > 0.10000000149011612f)
    {
        dist_1 = (intersectPos_0.x - rayPos_0.x) / _S9;
    }
    else
    {
        var _S10 : f32 = rayDir_0.y;
        if((abs(_S10)) > 0.10000000149011612f)
        {
            dist_1 = (intersectPos_0.y - rayPos_0.y) / _S10;
        }
        else
        {
            dist_1 = (intersectPos_0.z - rayPos_0.z) / rayDir_0.z;
        }
    }
    var _S11 : bool;
    if(dist_1 > 0.00999999977648258f)
    {
        _S11 = dist_1 < ((*info_0).dist_0);
    }
    else
    {
        _S11 = false;
    }
    if(_S11)
    {
        (*info_0).fromInside_0 = false;
        (*info_0).dist_0 = dist_1;
        (*info_0).normal_0 = normal_2;
        return true;
    }
    return false;
}

fn mod_0( x_0 : f32,  y_0 : f32) -> f32
{
    return x_0 - y_0 * floor(x_0 / y_0);
}

fn TestSphereTrace_0( rayPos_1 : vec3<f32>,  rayDir_1 : vec3<f32>,  info_1 : ptr<function, SRayHitInfo_0>,  sphere_0 : vec4<f32>) -> bool
{
    var _S12 : vec3<f32> = sphere_0.xyz;
    var m_1 : vec3<f32> = rayPos_1 - _S12;
    var b_1 : f32 = dot(m_1, rayDir_1);
    var _S13 : f32 = sphere_0.w;
    var c_1 : f32 = dot(m_1, m_1) - _S13 * _S13;
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
    var discr_0 : f32 = b_1 * b_1 - c_1;
    if(discr_0 < 0.0f)
    {
        return false;
    }
    var _S15 : f32 = - b_1;
    var _S16 : f32 = sqrt(discr_0);
    var dist_2 : f32 = _S15 - _S16;
    var _S17 : bool = dist_2 < 0.0f;
    var dist_3 : f32;
    if(_S17)
    {
        dist_3 = _S15 + _S16;
    }
    else
    {
        dist_3 = dist_2;
    }
    if(dist_3 > 0.00999999977648258f)
    {
        _S14 = dist_3 < ((*info_1).dist_0);
    }
    else
    {
        _S14 = false;
    }
    if(_S14)
    {
        (*info_1).fromInside_0 = _S17;
        (*info_1).dist_0 = dist_3;
        var _S18 : vec3<f32> = normalize(rayPos_1 + rayDir_1 * vec3<f32>(dist_3) - _S12);
        if(_S17)
        {
            dist_3 = -1.0f;
        }
        else
        {
            dist_3 = 1.0f;
        }
        (*info_1).normal_0 = _S18 * vec3<f32>(dist_3);
        return true;
    }
    return false;
}

fn TestSceneTrace_0( rayPos_2 : vec3<f32>,  rayDir_2 : vec3<f32>,  hitInfo_0 : ptr<function, SRayHitInfo_0>)
{
    var _S19 : bool = TestQuadTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec3<f32>(-25.0f, -12.5f, 5.0f), vec3<f32>(25.0f, -12.5f, 5.0f), vec3<f32>(25.0f, -12.5f, -5.0f), vec3<f32>(-25.0f, -12.5f, -5.0f));
    if(_S19)
    {
        (*hitInfo_0).material_0 = GetZeroedMaterial_0();
        (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.69999998807907104f, 0.69999998807907104f, 0.69999998807907104f);
    }
    var _S20 : bool = TestQuadTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec3<f32>(-25.0f, -1.5f, 5.0f), vec3<f32>(25.0f, -1.5f, 5.0f), vec3<f32>(25.0f, -10.5f, 5.0f), vec3<f32>(-25.0f, -10.5f, 5.0f));
    if(_S20)
    {
        (*hitInfo_0).material_0 = GetZeroedMaterial_0();
        var shade_0 : f32 = floor(mod_0((rayPos_2 + rayDir_2 * vec3<f32>((*hitInfo_0).dist_0)).x, 1.0f) * 2.0f);
        (*hitInfo_0).material_0.albedo_0 = vec3<f32>(shade_0, shade_0, shade_0);
    }
    var _S21 : bool = TestQuadTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec3<f32>(-7.5f, 12.5f, 5.0f), vec3<f32>(7.5f, 12.5f, 5.0f), vec3<f32>(7.5f, 12.5f, -5.0f), vec3<f32>(-7.5f, 12.5f, -5.0f));
    if(_S21)
    {
        (*hitInfo_0).material_0 = GetZeroedMaterial_0();
        (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.69999998807907104f, 0.69999998807907104f, 0.69999998807907104f);
    }
    var _S22 : bool = TestQuadTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec3<f32>(-5.0f, 12.39999961853027344f, 2.5f), vec3<f32>(5.0f, 12.39999961853027344f, 2.5f), vec3<f32>(5.0f, 12.39999961853027344f, -2.5f), vec3<f32>(-5.0f, 12.39999961853027344f, -2.5f));
    if(_S22)
    {
        (*hitInfo_0).material_0 = GetZeroedMaterial_0();
        (*hitInfo_0).material_0.emissive_0 = vec3<f32>(1.0f, 0.89999997615814209f, 0.69999998807907104f) * vec3<f32>(20.0f);
    }
    var sphereIndex_0 : i32;
    if((_RenderSceneCSCB.Scene_0) == i32(0))
    {
        sphereIndex_0 = i32(0);
        for(;;)
        {
            if(sphereIndex_0 < i32(7))
            {
            }
            else
            {
                break;
            }
            var _S23 : f32 = f32(sphereIndex_0);
            var _S24 : bool = TestSphereTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec4<f32>(-18.0f + 6.0f * _S23, -8.0f, 0.0f, 2.79999995231628418f));
            if(_S24)
            {
                var r_0 : f32 = _S23 / 6.0f * 0.5f;
                (*hitInfo_0).material_0 = GetZeroedMaterial_0();
                (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.89999997615814209f, 0.25f, 0.25f);
                (*hitInfo_0).material_0.emissive_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
                (*hitInfo_0).material_0.specularChance_0 = 0.01999999955296516f;
                (*hitInfo_0).material_0.specularRoughness_0 = r_0;
                (*hitInfo_0).material_0.specularColor_0 = vec3<f32>(0.80000001192092896f);
                (*hitInfo_0).material_0.IOR_0 = 1.10000002384185791f;
                (*hitInfo_0).material_0.refractionChance_0 = 1.0f;
                (*hitInfo_0).material_0.refractionRoughness_0 = r_0;
                (*hitInfo_0).material_0.refractionColor_0 = vec3<f32>(0.0f, 0.5f, 1.0f);
            }
            sphereIndex_0 = sphereIndex_0 + i32(1);
        }
    }
    else
    {
        if((_RenderSceneCSCB.Scene_0) == i32(1))
        {
            sphereIndex_0 = i32(0);
            for(;;)
            {
                if(sphereIndex_0 < i32(7))
                {
                }
                else
                {
                    break;
                }
                var _S25 : f32 = f32(sphereIndex_0);
                var _S26 : bool = TestSphereTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec4<f32>(-18.0f + 6.0f * _S25, -8.0f, 0.0f, 2.79999995231628418f));
                if(_S26)
                {
                    var ior_0 : f32 = 1.0f + 0.5f * _S25 / 6.0f;
                    (*hitInfo_0).material_0 = GetZeroedMaterial_0();
                    (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.89999997615814209f, 0.25f, 0.25f);
                    (*hitInfo_0).material_0.emissive_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
                    (*hitInfo_0).material_0.specularChance_0 = 0.01999999955296516f;
                    (*hitInfo_0).material_0.specularRoughness_0 = 0.0f;
                    (*hitInfo_0).material_0.specularColor_0 = vec3<f32>(0.80000001192092896f);
                    (*hitInfo_0).material_0.IOR_0 = ior_0;
                    (*hitInfo_0).material_0.refractionChance_0 = 1.0f;
                    (*hitInfo_0).material_0.refractionRoughness_0 = 0.0f;
                }
                sphereIndex_0 = sphereIndex_0 + i32(1);
            }
        }
        else
        {
            if((_RenderSceneCSCB.Scene_0) == i32(2))
            {
                sphereIndex_0 = i32(0);
                for(;;)
                {
                    if(sphereIndex_0 < i32(7))
                    {
                    }
                    else
                    {
                        break;
                    }
                    var _S27 : f32 = f32(sphereIndex_0);
                    var _S28 : bool = TestSphereTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec4<f32>(-18.0f + 6.0f * _S27, -8.0f, 0.0f, 2.79999995231628418f));
                    if(_S28)
                    {
                        var ior_1 : f32 = 1.0f + _S27 / 6.0f;
                        (*hitInfo_0).material_0 = GetZeroedMaterial_0();
                        (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.89999997615814209f, 0.25f, 0.25f);
                        (*hitInfo_0).material_0.emissive_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
                        (*hitInfo_0).material_0.specularChance_0 = 0.01999999955296516f;
                        (*hitInfo_0).material_0.specularRoughness_0 = 0.0f;
                        (*hitInfo_0).material_0.specularColor_0 = vec3<f32>(0.80000001192092896f);
                        (*hitInfo_0).material_0.IOR_0 = ior_1;
                        (*hitInfo_0).material_0.refractionChance_0 = 0.0f;
                    }
                    sphereIndex_0 = sphereIndex_0 + i32(1);
                }
            }
            else
            {
                if((_RenderSceneCSCB.Scene_0) == i32(3))
                {
                    sphereIndex_0 = i32(0);
                    for(;;)
                    {
                        if(sphereIndex_0 < i32(7))
                        {
                        }
                        else
                        {
                            break;
                        }
                        var _S29 : f32 = f32(sphereIndex_0);
                        var _S30 : bool = TestSphereTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec4<f32>(-18.0f + 6.0f * _S29, -8.0f, 0.0f, 2.79999995231628418f));
                        if(_S30)
                        {
                            var absorb_0 : f32 = _S29 / 6.0f;
                            (*hitInfo_0).material_0 = GetZeroedMaterial_0();
                            (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.89999997615814209f, 0.25f, 0.25f);
                            (*hitInfo_0).material_0.emissive_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
                            (*hitInfo_0).material_0.specularChance_0 = 0.01999999955296516f;
                            (*hitInfo_0).material_0.specularRoughness_0 = 0.0f;
                            (*hitInfo_0).material_0.specularColor_0 = vec3<f32>(0.80000001192092896f);
                            (*hitInfo_0).material_0.IOR_0 = 1.10000002384185791f;
                            (*hitInfo_0).material_0.refractionChance_0 = 1.0f;
                            (*hitInfo_0).material_0.refractionRoughness_0 = 0.0f;
                            (*hitInfo_0).material_0.refractionColor_0 = vec3<f32>(1.0f, 2.0f, 3.0f) * vec3<f32>(absorb_0);
                        }
                        sphereIndex_0 = sphereIndex_0 + i32(1);
                    }
                }
                else
                {
                    if((_RenderSceneCSCB.Scene_0) == i32(4))
                    {
                        sphereIndex_0 = i32(0);
                        for(;;)
                        {
                            if(sphereIndex_0 < i32(7))
                            {
                            }
                            else
                            {
                                break;
                            }
                            var _S31 : f32 = f32(sphereIndex_0);
                            var _S32 : bool = TestSphereTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec4<f32>(-18.0f + 6.0f * _S31, -9.0f + 0.75f * _S31, 0.0f, 2.79999995231628418f));
                            if(_S32)
                            {
                                (*hitInfo_0).material_0 = GetZeroedMaterial_0();
                                (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.89999997615814209f, 0.25f, 0.25f);
                                (*hitInfo_0).material_0.emissive_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
                                (*hitInfo_0).material_0.specularChance_0 = 0.01999999955296516f;
                                (*hitInfo_0).material_0.specularRoughness_0 = 0.0f;
                                (*hitInfo_0).material_0.specularColor_0 = vec3<f32>(0.80000001192092896f);
                                (*hitInfo_0).material_0.IOR_0 = 1.5f;
                                (*hitInfo_0).material_0.refractionChance_0 = 1.0f;
                                (*hitInfo_0).material_0.refractionRoughness_0 = 0.0f;
                            }
                            sphereIndex_0 = sphereIndex_0 + i32(1);
                        }
                    }
                    else
                    {
                        if((_RenderSceneCSCB.Scene_0) == i32(5))
                        {
                            sphereIndex_0 = i32(0);
                            for(;;)
                            {
                                if(sphereIndex_0 < i32(7))
                                {
                                }
                                else
                                {
                                    break;
                                }
                                var _S33 : f32 = f32(sphereIndex_0);
                                var _S34 : bool = TestSphereTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec4<f32>(-18.0f + 6.0f * _S33, -9.0f, 0.0f, 2.79999995231628418f));
                                if(_S34)
                                {
                                    var transparency_0 : f32 = _S33 / 6.0f;
                                    (*hitInfo_0).material_0 = GetZeroedMaterial_0();
                                    (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.89999997615814209f, 0.25f, 0.25f);
                                    (*hitInfo_0).material_0.emissive_0 = vec3<f32>(0.0f, 0.0f, 0.0f);
                                    (*hitInfo_0).material_0.specularChance_0 = 0.01999999955296516f;
                                    (*hitInfo_0).material_0.specularRoughness_0 = 0.0f;
                                    (*hitInfo_0).material_0.specularColor_0 = vec3<f32>(0.80000001192092896f);
                                    (*hitInfo_0).material_0.IOR_0 = 1.10000002384185791f;
                                    (*hitInfo_0).material_0.refractionChance_0 = 1.0f - transparency_0;
                                    (*hitInfo_0).material_0.refractionRoughness_0 = 0.0f;
                                }
                                sphereIndex_0 = sphereIndex_0 + i32(1);
                            }
                        }
                        else
                        {
                            if((_RenderSceneCSCB.Scene_0) == i32(6))
                            {
                                sphereIndex_0 = i32(0);
                                for(;;)
                                {
                                    if(sphereIndex_0 < i32(7))
                                    {
                                    }
                                    else
                                    {
                                        break;
                                    }
                                    var _S35 : f32 = f32(sphereIndex_0);
                                    var _S36 : bool = TestSphereTrace_0(rayPos_2, rayDir_2, &((*hitInfo_0)), vec4<f32>(-18.0f + 6.0f * _S35, -8.0f, 0.0f, 2.79999995231628418f));
                                    if(_S36)
                                    {
                                        var r_1 : f32 = _S35 / 6.0f * 0.5f;
                                        (*hitInfo_0).material_0 = GetZeroedMaterial_0();
                                        (*hitInfo_0).material_0.albedo_0 = vec3<f32>(0.89999997615814209f, 0.25f, 0.25f);
                                        const _S37 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
                                        (*hitInfo_0).material_0.emissive_0 = _S37;
                                        (*hitInfo_0).material_0.specularChance_0 = 0.01999999955296516f;
                                        (*hitInfo_0).material_0.specularRoughness_0 = r_1;
                                        (*hitInfo_0).material_0.specularColor_0 = vec3<f32>(0.80000001192092896f);
                                        (*hitInfo_0).material_0.IOR_0 = 1.10000002384185791f;
                                        (*hitInfo_0).material_0.refractionChance_0 = 1.0f;
                                        (*hitInfo_0).material_0.refractionRoughness_0 = r_1;
                                        (*hitInfo_0).material_0.refractionColor_0 = _S37;
                                    }
                                    sphereIndex_0 = sphereIndex_0 + i32(1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return;
}

fn SampleSphericalMap_0( v_3 : vec3<f32>) -> vec2<f32>
{
    return vec2<f32>(atan2(v_3.z, v_3.x), asin(- v_3.y)) * vec2<f32>(0.1590999960899353f, 0.31830000877380371f) + vec2<f32>(0.5f);
}

fn FresnelReflectAmount_0( n1_0 : f32,  n2_0 : f32,  normal_4 : vec3<f32>,  incident_0 : vec3<f32>,  f0_0 : f32,  f90_0 : f32) -> f32
{
    var r0_0 : f32 = (n1_0 - n2_0) / (n1_0 + n2_0);
    var r0_1 : f32 = r0_0 * r0_0;
    var cosX_0 : f32 = - dot(normal_4, incident_0);
    var cosX_1 : f32;
    if(n1_0 > n2_0)
    {
        var n_0 : f32 = n1_0 / n2_0;
        var sinT2_0 : f32 = n_0 * n_0 * (1.0f - cosX_0 * cosX_0);
        if(sinT2_0 > 1.0f)
        {
            return f90_0;
        }
        cosX_1 = sqrt(1.0f - sinT2_0);
    }
    else
    {
        cosX_1 = cosX_0;
    }
    var x_1 : f32 = 1.0f - cosX_1;
    return mix(f0_0, f90_0, r0_1 + (1.0f - r0_1) * x_1 * x_1 * x_1 * x_1 * x_1);
}

fn RandomUnitVector_0( state_1 : ptr<function, u32>) -> vec3<f32>
{
    var _S38 : f32 = RandomFloat01_0(&((*state_1)));
    var z_0 : f32 = _S38 * 2.0f - 1.0f;
    var _S39 : f32 = RandomFloat01_0(&((*state_1)));
    var a_1 : f32 = _S39 * 6.28318548202514648f;
    var r_2 : f32 = sqrt(1.0f - z_0 * z_0);
    return vec3<f32>(r_2 * cos(a_1), r_2 * sin(a_1), z_0);
}

fn GetColorForRay_0( startRayPos_0 : vec3<f32>,  startRayDir_0 : vec3<f32>,  rngState_0 : ptr<function, u32>) -> vec3<f32>
{
    const _S40 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const _S41 : vec3<f32> = vec3<f32>(1.0f, 1.0f, 1.0f);
    var rayPos_3 : vec3<f32> = startRayPos_0;
    var rayDir_3 : vec3<f32> = startRayDir_0;
    var bounceIndex_0 : i32 = i32(0);
    var throughput_0 : vec3<f32> = _S41;
    var ret_1 : vec3<f32> = _S40;
    for(;;)
    {
        if(bounceIndex_0 <= (_RenderSceneCSCB.NumBounces_0))
        {
        }
        else
        {
            break;
        }
        var hitInfo_1 : SRayHitInfo_0;
        hitInfo_1.material_0 = GetZeroedMaterial_0();
        hitInfo_1.dist_0 = 10000.0f;
        hitInfo_1.fromInside_0 = false;
        TestSceneTrace_0(rayPos_3, rayDir_3, &(hitInfo_1));
        if((hitInfo_1.dist_0) == 10000.0f)
        {
            ret_1 = ret_1 + (textureSampleLevel((_loadedTexture_0), (texSampler), (SampleSphericalMap_0(rayDir_3)), (0.0f))).xyz * vec3<f32>(_RenderSceneCSCB.SkyboxBrightness_0) * throughput_0;
            break;
        }
        var throughput_1 : vec3<f32>;
        if(hitInfo_1.fromInside_0)
        {
            throughput_1 = throughput_0 * exp(- hitInfo_1.material_0.refractionColor_0 * vec3<f32>(hitInfo_1.dist_0));
        }
        else
        {
            throughput_1 = throughput_0;
        }
        var specularChance_1 : f32 = hitInfo_1.material_0.specularChance_0;
        var refractionChance_1 : f32 = hitInfo_1.material_0.refractionChance_0;
        var specularChance_2 : f32;
        var refractionChance_2 : f32;
        if((hitInfo_1.material_0.specularChance_0) > 0.0f)
        {
            if(hitInfo_1.fromInside_0)
            {
                specularChance_2 = hitInfo_1.material_0.IOR_0;
            }
            else
            {
                specularChance_2 = 1.0f;
            }
            if(!hitInfo_1.fromInside_0)
            {
                refractionChance_2 = hitInfo_1.material_0.IOR_0;
            }
            else
            {
                refractionChance_2 = 1.0f;
            }
            var specularChance_3 : f32 = FresnelReflectAmount_0(specularChance_2, refractionChance_2, rayDir_3, hitInfo_1.normal_0, hitInfo_1.material_0.specularChance_0, 1.0f);
            var refractionChance_3 : f32 = refractionChance_1 * ((1.0f - specularChance_3) / (1.0f - hitInfo_1.material_0.specularChance_0));
            specularChance_2 = specularChance_3;
            refractionChance_2 = refractionChance_3;
        }
        else
        {
            specularChance_2 = specularChance_1;
            refractionChance_2 = refractionChance_1;
        }
        var raySelectRoll_0 : f32 = RandomFloat01_0(&((*rngState_0)));
        var _S42 : bool;
        if(specularChance_2 > 0.0f)
        {
            _S42 = raySelectRoll_0 < specularChance_2;
        }
        else
        {
            _S42 = false;
        }
        var rayProbability_0 : f32;
        var doRefraction_0 : f32;
        var doSpecular_0 : f32;
        if(_S42)
        {
            rayProbability_0 = specularChance_2;
            doRefraction_0 = 0.0f;
            doSpecular_0 = 1.0f;
        }
        else
        {
            var _S43 : bool;
            if(refractionChance_2 > 0.0f)
            {
                _S43 = raySelectRoll_0 < (specularChance_2 + refractionChance_2);
            }
            else
            {
                _S43 = false;
            }
            if(_S43)
            {
                rayProbability_0 = refractionChance_2;
                doRefraction_0 = 1.0f;
            }
            else
            {
                rayProbability_0 = 1.0f - (specularChance_2 + refractionChance_2);
                doRefraction_0 = 0.0f;
            }
            doSpecular_0 = 0.0f;
        }
        var rayProbability_1 : f32 = max(rayProbability_0, 0.00100000004749745f);
        if(doRefraction_0 == 1.0f)
        {
            rayPos_3 = rayPos_3 + rayDir_3 * vec3<f32>(hitInfo_1.dist_0) - hitInfo_1.normal_0 * vec3<f32>(0.00999999977648258f);
        }
        else
        {
            rayPos_3 = rayPos_3 + rayDir_3 * vec3<f32>(hitInfo_1.dist_0) + hitInfo_1.normal_0 * vec3<f32>(0.00999999977648258f);
        }
        var _S44 : vec3<f32> = RandomUnitVector_0(&((*rngState_0)));
        var diffuseRayDir_0 : vec3<f32> = normalize(hitInfo_1.normal_0 + _S44);
        var specularRayDir_0 : vec3<f32> = normalize(mix(reflect(rayDir_3, hitInfo_1.normal_0), diffuseRayDir_0, vec3<f32>((hitInfo_1.material_0.specularRoughness_0 * hitInfo_1.material_0.specularRoughness_0))));
        var _S45 : vec3<f32> = hitInfo_1.normal_0;
        var _S46 : f32;
        if(hitInfo_1.fromInside_0)
        {
            _S46 = hitInfo_1.material_0.IOR_0;
        }
        else
        {
            _S46 = 1.0f / hitInfo_1.material_0.IOR_0;
        }
        var refractionRayDir_0 : vec3<f32> = refract(rayDir_3, _S45, _S46);
        var _S47 : vec3<f32> = - hitInfo_1.normal_0;
        var _S48 : vec3<f32> = RandomUnitVector_0(&((*rngState_0)));
        var _S49 : vec3<f32> = vec3<f32>(doSpecular_0);
        var _S50 : vec3<f32> = mix(mix(diffuseRayDir_0, specularRayDir_0, _S49), normalize(mix(refractionRayDir_0, normalize(_S47 + _S48), vec3<f32>((hitInfo_1.material_0.refractionRoughness_0 * hitInfo_1.material_0.refractionRoughness_0)))), vec3<f32>(doRefraction_0));
        var ret_2 : vec3<f32> = ret_1 + hitInfo_1.material_0.emissive_0 * throughput_1;
        var throughput_2 : vec3<f32>;
        if(doRefraction_0 == 0.0f)
        {
            throughput_2 = throughput_1 * mix(hitInfo_1.material_0.albedo_0, hitInfo_1.material_0.specularColor_0, _S49);
        }
        else
        {
            throughput_2 = throughput_1;
        }
        var throughput_3 : vec3<f32> = throughput_2 / vec3<f32>(rayProbability_1);
        var p_0 : f32 = max(throughput_3.x, max(throughput_3.y, throughput_3.z));
        var _S51 : f32 = RandomFloat01_0(&((*rngState_0)));
        if(_S51 > p_0)
        {
            ret_1 = ret_2;
            break;
        }
        var throughput_4 : vec3<f32> = throughput_3 * vec3<f32>((1.0f / p_0));
        var bounceIndex_1 : i32 = bounceIndex_0 + i32(1);
        rayDir_3 = _S50;
        bounceIndex_0 = bounceIndex_1;
        throughput_0 = throughput_4;
        ret_1 = ret_2;
    }
    return ret_1;
}

fn ACESFilm_0( x_2 : vec3<f32>) -> vec3<f32>
{
    return clamp(x_2 * (vec3<f32>(2.50999999046325684f) * x_2 + vec3<f32>(0.02999999932944775f)) / (x_2 * (vec3<f32>(2.43000006675720215f) * x_2 + vec3<f32>(0.5899999737739563f)) + vec3<f32>(0.14000000059604645f)), vec3<f32>(0.0f), vec3<f32>(1.0f));
}

fn LessThan_0( f_0 : vec3<f32>,  value_0 : f32) -> vec3<f32>
{
    var _S52 : f32;
    if((f_0.x) < value_0)
    {
        _S52 = 1.0f;
    }
    else
    {
        _S52 = 0.0f;
    }
    var _S53 : f32;
    if((f_0.y) < value_0)
    {
        _S53 = 1.0f;
    }
    else
    {
        _S53 = 0.0f;
    }
    var _S54 : f32;
    if((f_0.z) < value_0)
    {
        _S54 = 1.0f;
    }
    else
    {
        _S54 = 0.0f;
    }
    return vec3<f32>(_S52, _S53, _S54);
}

fn LinearToSRGB_0( rgb_0 : vec3<f32>) -> vec3<f32>
{
    var _S55 : vec3<f32> = clamp(rgb_0, vec3<f32>(0.0f), vec3<f32>(1.0f));
    return mix(pow(_S55, vec3<f32>(0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f), _S55 * vec3<f32>(12.92000007629394531f), LessThan_0(_S55, 0.00313080009073019f));
}

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var px_0 : vec2<u32> = DTid_0.xy;
    var renderSize_0 : vec2<u32>;
    var _S56 : u32 = renderSize_0[i32(0)];
    var _S57 : u32 = renderSize_0[i32(1)];
    {var dim = textureDimensions((Output));((_S56)) = dim.x;((_S57)) = dim.y;};
    renderSize_0[i32(0)] = _S56;
    renderSize_0[i32(1)] = _S57;
    var rngState_1 : u32 = ((px_0.x * u32(1973) + px_0.y * u32(9277) + _RenderSceneCSCB.FrameIndex_0 * u32(26699)) | (u32(1)));
    var _S58 : f32 = RandomFloat01_0(&(rngState_1));
    var _S59 : f32 = RandomFloat01_0(&(rngState_1));
    var _S60 : vec2<f32> = vec2<f32>(0.5f);
    var _S61 : vec2<f32> = (vec2<f32>(px_0) + (vec2<f32>(_S58, _S59) - _S60) + _S60) / vec2<f32>(renderSize_0) * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    var screenPos_0 : vec2<f32> = _S61;
    screenPos_0[i32(1)] = - _S61.y;
    var _S62 : vec4<f32> = (((mat4x4<f32>(_RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(0)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(1)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(2)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(0)][i32(3)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(0)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(1)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(2)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(1)][i32(3)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(0)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(1)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(2)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(2)][i32(3)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(0)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(1)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(2)], _RenderSceneCSCB.InvViewProjMtx_0.data_0[i32(3)][i32(3)])) * (vec4<f32>(screenPos_0, 1.0f, 1.0f))));
    var world_0 : vec4<f32> = _S62;
    var _S63 : vec3<f32> = _S62.xyz / vec3<f32>(_S62.w);
    world_0.x = _S63.x;
    world_0.y = _S63.y;
    world_0.z = _S63.z;
    var rayPos_4 : vec3<f32> = _RenderSceneCSCB.CameraPos_0;
    var _S64 : vec3<f32> = normalize(world_0.xyz - _RenderSceneCSCB.CameraPos_0);
    const _S65 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var index_0 : i32 = i32(0);
    var color_0 : vec3<f32> = _S65;
    for(;;)
    {
        if(index_0 < (_RenderSceneCSCB.RaysPerPixel_0))
        {
        }
        else
        {
            break;
        }
        var _S66 : vec3<f32> = GetColorForRay_0(rayPos_4, _S64, &(rngState_1));
        var color_1 : vec3<f32> = color_0 + _S66 / vec3<f32>(f32(_RenderSceneCSCB.RaysPerPixel_0));
        index_0 = index_0 + i32(1);
        color_0 = color_1;
    }
    var _S67 : vec4<f32> = (textureLoad((AccumReadOnly), (vec2<i32>(px_0))));
    var lastFrameColor_0 : vec4<f32> = _S67;
    var reset_0 : bool;
    if((_RenderSceneCSCB.FrameIndex_0) < u32(2))
    {
        reset_0 = true;
    }
    else
    {
        reset_0 = (_RenderSceneCSCB.Scene_0) != (_RenderSceneCSCB.SceneLastFrame_0);
    }
    if(reset_0)
    {
        reset_0 = true;
    }
    else
    {
        reset_0 = bool(_RenderSceneCSCB.Reset_0);
    }
    if(reset_0)
    {
        reset_0 = true;
    }
    else
    {
        reset_0 = bool(_RenderSceneCSCB.CameraChanged_0);
    }
    var _S68 : f32;
    if(reset_0)
    {
        _S68 = 1.0f;
    }
    else
    {
        _S68 = lastFrameColor_0.w + 1.0f;
    }
    lastFrameColor_0[i32(3)] = _S68;
    var color_2 : vec3<f32> = mix(lastFrameColor_0.xyz, color_0, vec3<f32>((1.0f / lastFrameColor_0.w)));
    textureStore((Accum), (px_0), (vec4<f32>(color_2, lastFrameColor_0.w)));
    textureStore((Output), (px_0), (vec4<f32>(LinearToSRGB_0(ACESFilm_0(color_2 * vec3<f32>(pow(2.0f, _RenderSceneCSCB.Exposure_0)))), 1.0f)));
    return;
}

`;

// -------------------- Enums

static Enum_Scenes =
{
    Transparent_Orange: 0,
    Transparent_Clear: 1,
    Pink: 2,
    Coffee: 3,
    Focus: 4,
    Transparent_Pink: 5,
    _count: 6,
}

// -------------------- Private Members

// Texture Accum
texture_Accum = null;
texture_Accum_size = [0, 0, 0];
texture_Accum_format = "";
texture_Accum_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture RenderScene_Accum_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_RenderScene_Accum_ReadOnly = null;
texture_RenderScene_Accum_ReadOnly_size = [0, 0, 0];
texture_RenderScene_Accum_ReadOnly_format = "";
texture_RenderScene_Accum_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture RenderScene_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_RenderScene_Output_ReadOnly = null;
texture_RenderScene_Output_ReadOnly_size = [0, 0, 0];
texture_RenderScene_Output_ReadOnly_format = "";
texture_RenderScene_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Constant buffer _RenderSceneCSCB
constantBuffer__RenderSceneCSCB = null;
constantBuffer__RenderSceneCSCB_size = 144;
constantBuffer__RenderSceneCSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader RenderScene
Hash_Compute_RenderScene = 0;
ShaderModule_Compute_RenderScene = null;
BindGroupLayout_Compute_RenderScene = null;
PipelineLayout_Compute_RenderScene = null;
Pipeline_Compute_RenderScene = null;

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
variable_Scene = this.constructor.Enum_Scenes.Transparent_Orange;
variableDefault_Scene = this.constructor.Enum_Scenes.Transparent_Orange;
variableChanged_Scene = false;
variable_Animate = true;
variableDefault_Animate = true;
variableChanged_Animate = false;
variable_Reset = false;
variableDefault_Reset = false;
variableChanged_Reset = false;
variable_NumBounces = 8;  // number of ray bounces allowed max
variableDefault_NumBounces = 8;  // number of ray bounces allowed max
variableChanged_NumBounces = false;
variable_SkyboxBrightness = 1.000000;  // a multiplier for the skybox brightness
variableDefault_SkyboxBrightness = 1.000000;  // a multiplier for the skybox brightness
variableChanged_SkyboxBrightness = false;
variable_Exposure = 0.000000;  // a pixel value multiplier of light before tone mapping and sRGB. In FStops. Every +1 doubles brightness, every -1 halves brightness/
variableDefault_Exposure = 0.000000;  // a pixel value multiplier of light before tone mapping and sRGB. In FStops. Every +1 doubles brightness, every -1 halves brightness/
variableChanged_Exposure = false;
variable_RaysPerPixel = 16;  // How many rays per pixel, per frame?
variableDefault_RaysPerPixel = 16;  // How many rays per pixel, per frame?
variableChanged_RaysPerPixel = false;

// -------------------- Private Variables

variable_FrameIndex = 0;
variableDefault_FrameIndex = 0;
variableChanged_FrameIndex = false;
variable_InvViewProjMtx = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_InvViewProjMtx = [ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_InvViewProjMtx = [ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false ];
variable_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableDefault_CameraPos = [ 0.000000, 0.000000, 0.000000 ];
variableChanged_CameraPos = [ false, false, false ];
variable_SceneLastFrame = this.constructor.Enum_Scenes.Transparent_Orange;
variableDefault_SceneLastFrame = this.constructor.Enum_Scenes.Transparent_Orange;
variableChanged_SceneLastFrame = false;
variable_CameraChanged = false;
variableDefault_CameraChanged = false;
variableChanged_CameraChanged = false;
// -------------------- Structs

static StructOffsets__RenderSceneCSCB =
{
    CameraChanged: 0,
    _padding0: 4,
    _padding1: 8,
    _padding2: 12,
    CameraPos_0: 16,
    CameraPos_1: 20,
    CameraPos_2: 24,
    Exposure: 28,
    FrameIndex: 32,
    _padding3: 36,
    _padding4: 40,
    _padding5: 44,
    InvViewProjMtx_0: 48,
    InvViewProjMtx_1: 52,
    InvViewProjMtx_2: 56,
    InvViewProjMtx_3: 60,
    InvViewProjMtx_4: 64,
    InvViewProjMtx_5: 68,
    InvViewProjMtx_6: 72,
    InvViewProjMtx_7: 76,
    InvViewProjMtx_8: 80,
    InvViewProjMtx_9: 84,
    InvViewProjMtx_10: 88,
    InvViewProjMtx_11: 92,
    InvViewProjMtx_12: 96,
    InvViewProjMtx_13: 100,
    InvViewProjMtx_14: 104,
    InvViewProjMtx_15: 108,
    NumBounces: 112,
    RaysPerPixel: 116,
    Reset: 120,
    Scene: 124,
    SceneLastFrame: 128,
    SkyboxBrightness: 132,
    _padding6: 136,
    _padding7: 140,
    _size: 144,
}


async SetVarsBefore()
{

}

async SetVarsAfter()
{
    if (this.variable_Animate)
    {
        this.variableChanged_FrameIndex = true;
        this.variable_FrameIndex = this.variable_FrameIndex + 1;
    }

    {
        this.variableChanged_SceneLastFrame = true;
        this.variable_SceneLastFrame = this.variable_Scene;
    }


}

async ValidateImports()
{
    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Accum
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
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
                label: "texture CasualPathtracing.Accum",
                size: this.texture_Accum_size,
                format: Shared.GetNonSRGBFormat(this.texture_Accum_format),
                usage: this.texture_Accum_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

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
                label: "texture CasualPathtracing.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture RenderScene_Accum_ReadOnly
    {
        const baseSize = this.texture_Accum_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Accum_format;
        if (this.texture_RenderScene_Accum_ReadOnly !== null && (this.texture_RenderScene_Accum_ReadOnly_format != desiredFormat || this.texture_RenderScene_Accum_ReadOnly_size[0] != desiredSize[0] || this.texture_RenderScene_Accum_ReadOnly_size[1] != desiredSize[1] || this.texture_RenderScene_Accum_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_RenderScene_Accum_ReadOnly.destroy();
            this.texture_RenderScene_Accum_ReadOnly = null;
        }

        if (this.texture_RenderScene_Accum_ReadOnly === null)
        {
            this.texture_RenderScene_Accum_ReadOnly_size = desiredSize.slice();
            this.texture_RenderScene_Accum_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_RenderScene_Accum_ReadOnly_format))
                viewFormats.push(this.texture_RenderScene_Accum_ReadOnly_format);

            this.texture_RenderScene_Accum_ReadOnly = device.createTexture({
                label: "texture CasualPathtracing.RenderScene_Accum_ReadOnly",
                size: this.texture_RenderScene_Accum_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_RenderScene_Accum_ReadOnly_format),
                usage: this.texture_RenderScene_Accum_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture RenderScene_Output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_RenderScene_Output_ReadOnly !== null && (this.texture_RenderScene_Output_ReadOnly_format != desiredFormat || this.texture_RenderScene_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_RenderScene_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_RenderScene_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_RenderScene_Output_ReadOnly.destroy();
            this.texture_RenderScene_Output_ReadOnly = null;
        }

        if (this.texture_RenderScene_Output_ReadOnly === null)
        {
            this.texture_RenderScene_Output_ReadOnly_size = desiredSize.slice();
            this.texture_RenderScene_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_RenderScene_Output_ReadOnly_format))
                viewFormats.push(this.texture_RenderScene_Output_ReadOnly_format);

            this.texture_RenderScene_Output_ReadOnly = device.createTexture({
                label: "texture CasualPathtracing.RenderScene_Output_ReadOnly",
                size: this.texture_RenderScene_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_RenderScene_Output_ReadOnly_format),
                usage: this.texture_RenderScene_Output_ReadOnly_usageFlags,
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
    // (Re)create compute shader RenderScene
    {
        const bindGroupEntries =
        [
            {
                // Accum
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Accum_format), viewDimension: "2d" }
            },
            {
                // Output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // AccumReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_RenderScene_Accum_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // OutputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_RenderScene_Output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _loadedTexture_0
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
            {
                // _RenderSceneCSCB
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
            {
                // texSampler
                binding: 6,
                visibility: GPUShaderStage.COMPUTE,
                sampler: { },
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_RenderScene === null || newHash !== this.Hash_Compute_RenderScene)
        {
            this.Hash_Compute_RenderScene = newHash;

            let shaderCode = class_CasualPathtracing.ShaderCode_RenderScene_RenderSceneCS;
            shaderCode = shaderCode.replace("/*(Accum_format)*/", Shared.GetNonSRGBFormat(this.texture_Accum_format));
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(AccumReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_RenderScene_Accum_ReadOnly_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_RenderScene_Output_ReadOnly_format));

            this.ShaderModule_Compute_RenderScene = device.createShaderModule({ code: shaderCode, label: "Compute Shader RenderScene"});
            this.BindGroupLayout_Compute_RenderScene = device.createBindGroupLayout({
                label: "Compute Bind Group Layout RenderScene",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_RenderScene = device.createPipelineLayout({
                label: "Compute Pipeline Layout RenderScene",
                bindGroupLayouts: [this.BindGroupLayout_Compute_RenderScene],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_RenderScene = device.createComputePipeline({
                    label: "Compute Pipeline RenderScene",
                    layout: this.PipelineLayout_Compute_RenderScene,
                    compute: {
                        module: this.ShaderModule_Compute_RenderScene,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("RenderScene");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline RenderScene",
                    layout: this.PipelineLayout_Compute_RenderScene,
                    compute: {
                        module: this.ShaderModule_Compute_RenderScene,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_RenderScene = handle; this.loadingPromises.delete("RenderScene"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("CasualPathtracing.Accum");

    encoder.popDebugGroup(); // "CasualPathtracing.Accum"

    encoder.pushDebugGroup("CasualPathtracing.Output");

    encoder.popDebugGroup(); // "CasualPathtracing.Output"

    encoder.pushDebugGroup("CasualPathtracing.RenderScene_Accum_ReadOnly");

    encoder.popDebugGroup(); // "CasualPathtracing.RenderScene_Accum_ReadOnly"

    encoder.pushDebugGroup("CasualPathtracing.Copy_RenderScene_Accum");

    // Copy texture Accum to texture RenderScene_Accum_ReadOnly
    {
        const numMips = Math.min(this.texture_Accum.mipLevelCount, this.texture_RenderScene_Accum_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Accum.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Accum.height >> mipIndex, 1);
            let mipDepth = this.texture_Accum.depthOrArrayLayers;

            if (this.texture_Accum.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Accum, mipLevel: mipIndex },
                { texture: this.texture_RenderScene_Accum_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "CasualPathtracing.Copy_RenderScene_Accum"

    encoder.pushDebugGroup("CasualPathtracing.RenderScene_Output_ReadOnly");

    encoder.popDebugGroup(); // "CasualPathtracing.RenderScene_Output_ReadOnly"

    encoder.pushDebugGroup("CasualPathtracing.Copy_RenderScene_Output");

    // Copy texture Output to texture RenderScene_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_RenderScene_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_RenderScene_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "CasualPathtracing.Copy_RenderScene_Output"

    encoder.pushDebugGroup("CasualPathtracing._loadedTexture_0");

    encoder.popDebugGroup(); // "CasualPathtracing._loadedTexture_0"

    encoder.pushDebugGroup("CasualPathtracing._RenderSceneCSCB");

    // Create constant buffer _RenderSceneCSCB
    if (this.constantBuffer__RenderSceneCSCB === null)
    {
        this.constantBuffer__RenderSceneCSCB = device.createBuffer({
            label: "CasualPathtracing._RenderSceneCSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__RenderSceneCSCB._size),
            usage: this.constantBuffer__RenderSceneCSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _RenderSceneCSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__RenderSceneCSCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__RenderSceneCSCB.CameraChanged, (this.variable_CameraChanged === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.CameraPos_0, this.variable_CameraPos[0], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.CameraPos_1, this.variable_CameraPos[1], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.CameraPos_2, this.variable_CameraPos[2], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.Exposure, this.variable_Exposure, true);
        view.setUint32(this.constructor.StructOffsets__RenderSceneCSCB.FrameIndex, this.variable_FrameIndex, true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_0, this.variable_InvViewProjMtx[0], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_1, this.variable_InvViewProjMtx[1], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_2, this.variable_InvViewProjMtx[2], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_3, this.variable_InvViewProjMtx[3], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_4, this.variable_InvViewProjMtx[4], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_5, this.variable_InvViewProjMtx[5], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_6, this.variable_InvViewProjMtx[6], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_7, this.variable_InvViewProjMtx[7], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_8, this.variable_InvViewProjMtx[8], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_9, this.variable_InvViewProjMtx[9], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_10, this.variable_InvViewProjMtx[10], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_11, this.variable_InvViewProjMtx[11], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_12, this.variable_InvViewProjMtx[12], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_13, this.variable_InvViewProjMtx[13], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_14, this.variable_InvViewProjMtx[14], true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.InvViewProjMtx_15, this.variable_InvViewProjMtx[15], true);
        view.setInt32(this.constructor.StructOffsets__RenderSceneCSCB.NumBounces, this.variable_NumBounces, true);
        view.setInt32(this.constructor.StructOffsets__RenderSceneCSCB.RaysPerPixel, this.variable_RaysPerPixel, true);
        view.setUint32(this.constructor.StructOffsets__RenderSceneCSCB.Reset, (this.variable_Reset === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__RenderSceneCSCB.Scene, this.variable_Scene, true);
        view.setInt32(this.constructor.StructOffsets__RenderSceneCSCB.SceneLastFrame, this.variable_SceneLastFrame, true);
        view.setFloat32(this.constructor.StructOffsets__RenderSceneCSCB.SkyboxBrightness, this.variable_SkyboxBrightness, true);
        device.queue.writeBuffer(this.constantBuffer__RenderSceneCSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "CasualPathtracing._RenderSceneCSCB"

    encoder.pushDebugGroup("CasualPathtracing.RenderScene");

    // Run compute shader RenderScene
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group RenderScene",
            layout: this.BindGroupLayout_Compute_RenderScene,
            entries: [
                {
                    // Accum
                    binding: 0,
                    resource: this.texture_Accum.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // Output
                    binding: 1,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // AccumReadOnly
                    binding: 2,
                    resource: this.texture_RenderScene_Accum_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 3,
                    resource: this.texture_RenderScene_Output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 4,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _RenderSceneCSCB
                    binding: 5,
                    resource: { buffer: this.constantBuffer__RenderSceneCSCB }
                },
                {
                    // texSampler
                    binding: 6,
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
        const baseDispatchSize = this.texture_Accum_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_RenderScene !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_RenderScene);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "CasualPathtracing.RenderScene"

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

var CasualPathtracing = new class_CasualPathtracing;

export default CasualPathtracing;
