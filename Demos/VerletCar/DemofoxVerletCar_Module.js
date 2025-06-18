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

class class_DemofoxVerletCar
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "CSGameplay", node "Gameplay"
static ShaderCode_Gameplay_CSGameplay = `
struct Struct_GameState_std430_0
{
    @align(16) state_0 : vec4<f32>,
    @align(16) state2_0 : vec4<f32>,
    @align(16) state3_0 : vec4<f32>,
    @align(16) state4_0 : vec4<f32>,
    @align(16) backWheel_0 : vec4<f32>,
    @align(16) frontWheel_0 : vec4<f32>,
    @align(16) frontWheelCP1_0 : vec4<f32>,
    @align(16) frontWheelCP2_0 : vec4<f32>,
    @align(16) backWheelCP1_0 : vec4<f32>,
    @align(16) backWheelCP2_0 : vec4<f32>,
};

@binding(0) @group(0) var<storage, read_write> gameState : array<Struct_GameState_std430_0>;

struct Struct_CSGameplayCB_std140_0
{
    @align(16) iFrame_0 : i32,
    @align(4) iTimeDelta_0 : f32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(2) @group(0) var<uniform> _CSGameplayCB : Struct_CSGameplayCB_std140_0;
@binding(1) @group(0) var<storage, read> keyStates : array<u32>;

fn _S1( _S2 : i32) -> u32
{
    return keyStates[_S2];
}

fn KeyPressed_0( key_0 : f32) -> bool
{
    return f32(_S1(i32(key_0))) != 0.0f;
}

fn AsyncPointPos_0( p_0 : vec4<f32>,  frameFraction_0 : f32) -> vec2<f32>
{
    return mix(p_0.zw, p_0.xy, vec2<f32>(frameFraction_0));
}

fn AsyncBikePos_0( backWheel_1 : vec4<f32>,  frontWheel_1 : vec4<f32>,  frameFraction_1 : f32) -> vec2<f32>
{
    return (AsyncPointPos_0(backWheel_1, frameFraction_1) + AsyncPointPos_0(frontWheel_1, frameFraction_1)) * vec2<f32>(0.5f);
}

fn VerletIntegrate_0( p_1 : ptr<function, vec4<f32>>,  acceleration_0 : vec2<f32>)
{
    var currentPos_0 : vec2<f32> = (*p_1).xy;
    var newPos_0 : vec2<f32> = currentPos_0 + currentPos_0 - (*p_1).zw + acceleration_0 * vec2<f32>(0.00390625f);
    (*p_1).x = newPos_0.x;
    (*p_1).y = newPos_0.y;
    (*p_1).z = currentPos_0.x;
    (*p_1).w = currentPos_0.y;
    return;
}

fn RotatePoint_0( p_2 : vec2<f32>,  theta_0 : f32) -> vec2<f32>
{
    var _S3 : f32 = p_2.x;
    var _S4 : f32 = cos(theta_0);
    var _S5 : f32 = p_2.y;
    var _S6 : f32 = sin(theta_0);
    return vec2<f32>(_S3 * _S4 - _S5 * _S6, _S5 * _S4 + _S3 * _S6);
}

fn RotatePointAroundPoint_0( p_3 : vec2<f32>,  origin_0 : vec2<f32>,  theta_1 : f32) -> vec2<f32>
{
    return RotatePoint_0(p_3 - origin_0, theta_1) + origin_0;
}

fn GroundHeightAtX_0( x_0 : f32,  scale_0 : f32) -> f32
{
    var _S7 : f32 = x_0 * scale_0;
    return (sin(_S7 * 0.63400000333786011f) * clamp(_S7 / 0.00100000004749745f, 0.0f, 1.0f) * 0.55000001192092896f * (sin(_S7 * 0.10000000149011612f) * 0.5f + 0.5f) + sin(_S7) * clamp(_S7 / 50.0f, 0.0f, 1.0f) * 0.5f * (sin(_S7 * 0.37000000476837158f) * 0.5f + 0.5f) + sin(_S7 * 3.17000007629394531f) * clamp((_S7 - 75.0f) / 50.0f, 0.0f, 1.0f) * 0.10000000149011612f * (sin(_S7 * 0.05400000140070915f) * 0.5f + 0.5f) + sin(_S7 * 9.17000007629394531f) * clamp((_S7 - 150.0f) / 50.0f, 0.0f, 1.0f) * 0.05000000074505806f * (sin(_S7 * 0.00499999988824129f) * 0.5f + 0.5f) + sin(_S7 * 0.30000001192092896f) * clamp((_S7 - 225.0f) / 10.0f, 0.0f, 1.0f) * 0.89999997615814209f * (sin(_S7 * 0.00999999977648258f) * 0.5f + 0.5f)) * smoothstep(_S7 / 2.0f, 0.0f, 1.0f) * scale_0;
}

fn GroundFunction_0( p_4 : vec2<f32>,  scale_1 : f32) -> f32
{
    return GroundHeightAtX_0(p_4.x, scale_1) - p_4.y;
}

fn GroundFunctionGradiant_0( coords_0 : vec2<f32>,  scale_2 : f32) -> vec2<f32>
{
    const _S8 : vec2<f32> = vec2<f32>(0.00999999977648258f, 0.0f);
    const _S9 : vec2<f32> = vec2<f32>(0.0f, 0.00999999977648258f);
    return vec2<f32>(GroundFunction_0(coords_0 + _S8, scale_2) - GroundFunction_0(coords_0 - _S8, scale_2), GroundFunction_0(coords_0 + _S9, scale_2) - GroundFunction_0(coords_0 - _S9, scale_2)) / vec2<f32>(0.01999999955296516f);
}

fn EstimatedDistanceFromPointToGround_0( p_5 : vec2<f32>,  scale_3 : f32,  frequencyScale_0 : f32,  gradient_0 : ptr<function, vec2<f32>>) -> f32
{
    var v_0 : f32 = GroundFunction_0(p_5, scale_3);
    var _S10 : vec2<f32> = GroundFunctionGradiant_0(p_5, scale_3);
    (*gradient_0) = _S10;
    return v_0 / length(_S10);
}

fn ResolveGroundCollision_0( p_6 : ptr<function, vec2<f32>>,  pointTouchingGround_0 : ptr<function, bool>)
{
    var gradient_1 : vec2<f32>;
    var _S11 : f32 = EstimatedDistanceFromPointToGround_0((*p_6), 1.0f, 1.0f, &(gradient_1));
    var dist_0 : f32 = _S11 * -1.0f;
    if(dist_0 < 0.03999999910593033f)
    {
        (*p_6) = (*p_6) - normalize(gradient_1) * vec2<f32>((0.03999999910593033f - dist_0));
        (*pointTouchingGround_0) = true;
    }
    return;
}

fn ResolveDistanceConstraint_0( pointA_0 : ptr<function, vec2<f32>>,  pointB_0 : ptr<function, vec2<f32>>,  distance_0 : f32)
{
    var _S12 : vec2<f32> = normalize((*pointB_0) - (*pointA_0)) * vec2<f32>(((distance_0 - length((*pointB_0) - (*pointA_0))) * 0.5f));
    (*pointA_0) = (*pointA_0) - _S12;
    (*pointB_0) = (*pointB_0) + _S12;
    return;
}

fn mod_0( x_1 : f32,  y_0 : f32) -> f32
{
    return x_1 - y_0 * floor(x_1 / y_0);
}

struct Struct_GameState_0
{
    @align(16) state_0 : vec4<f32>,
    @align(16) state2_0 : vec4<f32>,
    @align(16) state3_0 : vec4<f32>,
    @align(16) state4_0 : vec4<f32>,
    @align(16) backWheel_0 : vec4<f32>,
    @align(16) frontWheel_0 : vec4<f32>,
    @align(16) frontWheelCP1_0 : vec4<f32>,
    @align(16) frontWheelCP2_0 : vec4<f32>,
    @align(16) backWheelCP1_0 : vec4<f32>,
    @align(16) backWheelCP2_0 : vec4<f32>,
};

@compute
@workgroup_size(1, 1, 1)
fn CSGameplay(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S13 : vec4<f32> = gameState[i32(0)].state2_0;
    var _S14 : vec4<f32> = gameState[i32(0)].state3_0;
    var _S15 : vec4<f32> = gameState[i32(0)].state4_0;
    var _S16 : vec4<f32> = gameState[i32(0)].backWheel_0;
    var _S17 : vec4<f32> = gameState[i32(0)].frontWheel_0;
    var _S18 : vec4<f32> = gameState[i32(0)].frontWheelCP1_0;
    var _S19 : vec4<f32> = gameState[i32(0)].frontWheelCP2_0;
    var _S20 : vec4<f32> = gameState[i32(0)].backWheelCP1_0;
    var _S21 : vec4<f32> = gameState[i32(0)].backWheelCP2_0;
    var s_0 : Struct_GameState_0;
    s_0.state_0 = gameState[i32(0)].state_0;
    s_0.state2_0 = _S13;
    s_0.state3_0 = _S14;
    s_0.state4_0 = _S15;
    s_0.backWheel_0 = _S16;
    s_0.frontWheel_0 = _S17;
    s_0.frontWheelCP1_0 = _S18;
    s_0.frontWheelCP2_0 = _S19;
    s_0.backWheelCP1_0 = _S20;
    s_0.backWheelCP2_0 = _S21;
    var state_1 : vec4<f32> = s_0.state_0;
    var state2_1 : vec4<f32> = s_0.state2_0;
    var state3_1 : vec4<f32> = s_0.state3_0;
    var state4_1 : vec4<f32> = s_0.state4_0;
    var backWheel_2 : vec4<f32> = s_0.backWheel_0;
    var frontWheel_2 : vec4<f32> = s_0.frontWheel_0;
    var _S22 : vec4<f32> = s_0.frontWheelCP1_0;
    var _S23 : vec4<f32> = s_0.frontWheelCP2_0;
    var _S24 : vec4<f32> = s_0.backWheelCP1_0;
    var _S25 : vec4<f32> = s_0.backWheelCP2_0;
    var frontWheelCP2_1 : vec4<f32>;
    var backWheelCP2_1 : vec4<f32>;
    var frontWheelCP1_1 : vec4<f32>;
    var backWheelCP1_1 : vec4<f32>;
    if((_CSGameplayCB.iFrame_0) < i32(10))
    {
        const _S26 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
        state_1 = _S26;
        state2_1 = _S26;
        state3_1 = _S26;
        state4_1 = _S26;
        state_1[i32(0)] = 0.0f;
        state_1[i32(1)] = 0.5f;
        const _S27 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
        state_1.z = _S27.x;
        state_1.w = _S27.y;
        state2_1[i32(0)] = 2.0f;
        state2_1[i32(1)] = 0.0f;
        state2_1[i32(2)] = 0.0f;
        state2_1[i32(3)] = 0.0f;
        state3_1[i32(0)] = 1.0f;
        state3_1[i32(2)] = 0.0f;
        state3_1[i32(1)] = 0.0f;
        state3_1[i32(3)] = 1.0f;
        const _S28 : vec4<f32> = vec4<f32>(-0.0625f, 0.03999999910593033f, -0.0625f, 0.03999999910593033f);
        backWheel_2 = _S28;
        const _S29 : vec4<f32> = vec4<f32>(0.0625f, 0.03999999910593033f, 0.0625f, 0.03999999910593033f);
        frontWheel_2 = _S29;
        state4_1[i32(0)] = state3_1.w;
        state4_1[i32(1)] = state3_1.w;
        state4_1[i32(3)] = 0.0f;
        frontWheelCP2_1 = _S29;
        backWheelCP2_1 = _S28;
        frontWheelCP1_1 = _S29;
        backWheelCP1_1 = _S28;
    }
    else
    {
        frontWheelCP2_1 = _S23;
        backWheelCP2_1 = _S25;
        frontWheelCP1_1 = _S22;
        backWheelCP1_1 = _S24;
    }
    var _S30 : bool;
    if((state2_1.w) == 1.0f)
    {
        _S30 = KeyPressed_0(32.0f);
    }
    else
    {
        _S30 = false;
    }
    if(_S30)
    {
        frontWheel_2 = frontWheelCP2_1;
        backWheel_2 = backWheelCP2_1;
        state3_1[i32(3)] = max(state4_1.y, 0.5f);
        state2_1[i32(3)] = 0.0f;
        state2_1[i32(1)] = 0.0f;
        state2_1[i32(2)] = 0.0f;
        state3_1[i32(1)] = 0.0f;
        state4_1[i32(3)] = floor(backWheelCP2_1.x / 5.0f) * 5.0f;
        state4_1[i32(0)] = state4_1.y;
        frontWheelCP1_1 = frontWheelCP2_1;
        backWheelCP1_1 = backWheelCP2_1;
    }
    var _S31 : vec2<f32> = AsyncBikePos_0(backWheel_2, frontWheel_2, state_1.x);
    state_1.z = _S31.x;
    state_1.w = _S31.y;
    if(KeyPressed_0(39.0f))
    {
        _S30 = true;
    }
    else
    {
        _S30 = KeyPressed_0(38.0f);
    }
    if(_S30)
    {
        state_1[i32(1)] = 1.0f;
    }
    if(KeyPressed_0(37.0f))
    {
        _S30 = true;
    }
    else
    {
        _S30 = KeyPressed_0(40.0f);
    }
    if(_S30)
    {
        state_1[i32(1)] = 0.0f;
    }
    var iTimeDelta_1 : f32 = _CSGameplayCB.iTimeDelta_0;
    if((state2_1.w) == 1.0f)
    {
        state3_1[i32(0)] = state3_1[i32(0)] + iTimeDelta_1 / 3.0f;
        state3_1[i32(0)] = min(state3_1.x, 1.0f);
    }
    else
    {
        state3_1[i32(0)] = 1.0f;
    }
    state_1[i32(0)] = state_1[i32(0)] + iTimeDelta_1 * mix(0.0f, 16.0f, pow(state3_1.x, 2.0f));
    if((state_1.x) > 1.0f)
    {
        state_1[i32(0)] = fract(state_1.x);
        var frontWheelRelativeToBackWheel_0 : vec2<f32> = frontWheel_2.xy - backWheel_2.xy;
        if((state2_1.w) != 1.0f)
        {
            _S30 = (state2_1.y) == 1.0f;
        }
        else
        {
            _S30 = false;
        }
        if(_S30)
        {
            _S30 = (state2_1.z) == 1.0f;
        }
        else
        {
            _S30 = false;
        }
        if(_S30)
        {
            _S30 = (dot(frontWheelRelativeToBackWheel_0, vec2<f32>(-1.0f, 0.0f))) > 0.0f;
        }
        else
        {
            _S30 = false;
        }
        if(_S30)
        {
            state2_1[i32(3)] = 1.0f;
            state3_1[i32(0)] = 0.0f;
        }
        if((state2_1.w) == 1.0f)
        {
            state_1[i32(1)] = 0.5f;
        }
        if((state_1.y) != 0.5f)
        {
            state3_1[i32(3)] = state3_1[i32(3)] - 0.00625000009313226f;
            if((state3_1.w) < 0.0f)
            {
                state2_1[i32(3)] = 1.0f;
                state3_1[i32(0)] = 0.0f;
                state3_1[i32(3)] = 0.0f;
                state_1[i32(1)] = 0.5f;
            }
        }
        if((state2_1.w) == 0.0f)
        {
            _S30 = (backWheel_2.x - state4_1.w) > 5.0f;
        }
        else
        {
            _S30 = false;
        }
        if(_S30)
        {
            state4_1[i32(3)] = floor(backWheel_2.x / 5.0f) * 5.0f;
            state4_1[i32(1)] = state4_1.x;
            state4_1[i32(0)] = state3_1.w;
            var _S32 : vec4<f32> = frontWheelCP1_1;
            var _S33 : vec4<f32> = backWheelCP1_1;
            frontWheelCP1_1 = frontWheel_2;
            frontWheelCP2_1 = _S32;
            backWheelCP1_1 = backWheel_2;
            backWheelCP2_1 = _S33;
        }
        var _S34 : vec2<f32>;
        if((state2_1.y) == 1.0f)
        {
            _S34 = vec2<f32>(state_1.y * 2.0f - 1.0f, 0.0f) * vec2<f32>(4.0f);
        }
        else
        {
            _S34 = vec2<f32>(0.0f, 0.0f);
        }
        var acceleration_1 : vec2<f32> = vec2<f32>(0.0f, -5.0f) + _S34;
        var spin_0 : f32 = (state_1.y * 2.0f - 1.0f) * 0.10000000149011612f;
        state_1[i32(1)] = 0.5f;
        VerletIntegrate_0(&(backWheel_2), acceleration_1);
        VerletIntegrate_0(&(frontWheel_2), acceleration_1);
        var _S35 : vec2<f32> = RotatePointAroundPoint_0(frontWheel_2.xy, backWheel_2.xy, spin_0);
        frontWheel_2.x = _S35.x;
        frontWheel_2.y = _S35.y;
        var backWheelOnGround_0 : bool = false;
        var frontWheelOnGround_0 : bool = false;
        var i_0 : i32 = i32(0);
        for(;;)
        {
            if(i_0 < i32(1))
            {
            }
            else
            {
                break;
            }
            var _S36 : vec2<f32> = backWheel_2.xy;
            ResolveGroundCollision_0(&(_S36), &(backWheelOnGround_0));
            var _S37 : vec2<f32> = _S36;
            backWheel_2.x = _S37.x;
            backWheel_2.y = _S37.y;
            var _S38 : vec2<f32> = frontWheel_2.xy;
            ResolveGroundCollision_0(&(_S38), &(frontWheelOnGround_0));
            var _S39 : vec2<f32> = _S38;
            frontWheel_2.x = _S39.x;
            frontWheel_2.y = _S39.y;
            var _S40 : vec2<f32> = backWheel_2.xy;
            var _S41 : vec2<f32> = frontWheel_2.xy;
            ResolveDistanceConstraint_0(&(_S40), &(_S41), 0.125f);
            var _S42 : vec2<f32> = _S40;
            backWheel_2.x = _S42.x;
            backWheel_2.y = _S42.y;
            var _S43 : vec2<f32> = _S41;
            frontWheel_2.x = _S43.x;
            frontWheel_2.y = _S43.y;
            i_0 = i_0 + i32(1);
        }
        var _S44 : f32;
        if(backWheelOnGround_0)
        {
            _S44 = 1.0f;
        }
        else
        {
            _S44 = 0.0f;
        }
        state2_1[i32(1)] = _S44;
        if(frontWheelOnGround_0)
        {
            _S44 = 1.0f;
        }
        else
        {
            _S44 = 0.0f;
        }
        state2_1[i32(2)] = _S44;
        if((backWheel_2.x) < -50.0f)
        {
            const _S45 : vec4<f32> = vec4<f32>(2999.9375f, 0.5f, 2999.9375f, 0.5f);
            backWheel_2 = _S45;
            const _S46 : vec4<f32> = vec4<f32>(3000.0625f, 0.5f, 3000.0625f, 0.5f);
            frontWheel_2 = _S46;
            var _S47 : vec2<f32> = AsyncBikePos_0(_S45, _S46, state_1.x);
            state_1.z = _S47.x;
            state_1.w = _S47.y;
            state3_1[i32(3)] = 9999.0f;
        }
    }
    var asyncBikePos_0 : vec2<f32> = AsyncBikePos_0(backWheel_2, frontWheel_2, state_1.x);
    var uvFuel_0 : vec2<f32>;
    var _S48 : f32 = asyncBikePos_0.x;
    uvFuel_0[i32(0)] = mod_0(_S48, 20.0f) - 10.0f;
    uvFuel_0[i32(1)] = GroundHeightAtX_0(floor(_S48 / 20.0f) * 20.0f + 10.0f, 1.0f) + 0.08250000327825546f;
    if((state3_1.y) < _S48)
    {
        _S30 = (length(asyncBikePos_0 - vec2<f32>(uvFuel_0.x + _S48, uvFuel_0.y))) < 0.15000000596046448f;
    }
    else
    {
        _S30 = false;
    }
    if(_S30)
    {
        state3_1[i32(3)] = max(state3_1.w, 1.0f);
        state3_1[i32(1)] = floor(_S48 / 20.0f) * 20.0f + 10.0f + 0.15000000596046448f;
    }
    state3_1[i32(2)] = state3_1[i32(2)] + (1.20000004768371582f * (length(backWheel_2.xy - backWheel_2.zw) + length(frontWheel_2.xy - frontWheel_2.zw)) * 0.5f - state3_1.z) * 3.0f * iTimeDelta_1;
    state_1.z = asyncBikePos_0.x;
    state_1.w = asyncBikePos_0.y;
    state2_1[i32(0)] = mix(state2_1.x, max(0.0f, state3_1.z * 4.0f + 2.0f), iTimeDelta_1 * mix(0.0f, 16.0f, pow(state3_1.x, 2.0f)) / 25.0f);
    state2_1[i32(0)] = clamp(state2_1.x, 2.0f, 6.0f);
    s_0.state_0 = state_1;
    s_0.state2_0 = state2_1;
    s_0.state3_0 = state3_1;
    s_0.state4_0 = state4_1;
    s_0.backWheel_0 = backWheel_2;
    s_0.frontWheel_0 = frontWheel_2;
    s_0.frontWheelCP1_0 = frontWheelCP1_1;
    s_0.frontWheelCP2_0 = frontWheelCP2_1;
    s_0.backWheelCP1_0 = backWheelCP1_1;
    s_0.backWheelCP2_0 = backWheelCP2_1;
    var _S49 : Struct_GameState_0 = s_0;
    gameState[i32(0)].state_0 = s_0.state_0;
    gameState[i32(0)].state2_0 = _S49.state2_0;
    gameState[i32(0)].state3_0 = _S49.state3_0;
    gameState[i32(0)].state4_0 = _S49.state4_0;
    gameState[i32(0)].backWheel_0 = _S49.backWheel_0;
    gameState[i32(0)].frontWheel_0 = _S49.frontWheel_0;
    gameState[i32(0)].frontWheelCP1_0 = _S49.frontWheelCP1_0;
    gameState[i32(0)].frontWheelCP2_0 = _S49.frontWheelCP2_0;
    gameState[i32(0)].backWheelCP1_0 = _S49.backWheelCP1_0;
    gameState[i32(0)].backWheelCP2_0 = _S49.backWheelCP2_0;
    return;
}

`;

// Shader code for Compute shader "CSDisplay", node "Display"
static ShaderCode_Display_CSDisplay = `
struct Struct_GameState_std430_0
{
    @align(16) state_0 : vec4<f32>,
    @align(16) state2_0 : vec4<f32>,
    @align(16) state3_0 : vec4<f32>,
    @align(16) state4_0 : vec4<f32>,
    @align(16) backWheel_0 : vec4<f32>,
    @align(16) frontWheel_0 : vec4<f32>,
    @align(16) frontWheelCP1_0 : vec4<f32>,
    @align(16) frontWheelCP2_0 : vec4<f32>,
    @align(16) backWheelCP1_0 : vec4<f32>,
    @align(16) backWheelCP2_0 : vec4<f32>,
};

@binding(0) @group(0) var<storage, read> gameState : array<Struct_GameState_std430_0>;

@binding(1) @group(0) var output : texture_storage_2d</*(output_format)*/, write>;

struct Struct_CSDisplayCB_std140_0
{
    @align(16) iTime_0 : f32,
    @align(4) _padding0_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
};

@binding(3) @group(0) var<uniform> _CSDisplayCB : Struct_CSDisplayCB_std140_0;
fn DrawSky_0( uv_0 : vec2<f32>,  cameraOffset_0 : vec2<f32>,  pixelColor_0 : ptr<function, vec3<f32>>)
{
    (*pixelColor_0) = mix(vec3<f32>(0.25f, 0.60000002384185791f, 1.0f), vec3<f32>(0.25f, 0.10000000149011612f, 0.30000001192092896f), vec3<f32>(smoothstep(0.0f, 1.0f, clamp(0.0f, 1.0f, uv_0.y + cameraOffset_0.y * -0.89999997615814209f))));
    return;
}

fn GroundHeightAtX_0( x_0 : f32,  scale_0 : f32) -> f32
{
    var _S1 : f32 = x_0 * scale_0;
    return (sin(_S1 * 0.63400000333786011f) * clamp(_S1 / 0.00100000004749745f, 0.0f, 1.0f) * 0.55000001192092896f * (sin(_S1 * 0.10000000149011612f) * 0.5f + 0.5f) + sin(_S1) * clamp(_S1 / 50.0f, 0.0f, 1.0f) * 0.5f * (sin(_S1 * 0.37000000476837158f) * 0.5f + 0.5f) + sin(_S1 * 3.17000007629394531f) * clamp((_S1 - 75.0f) / 50.0f, 0.0f, 1.0f) * 0.10000000149011612f * (sin(_S1 * 0.05400000140070915f) * 0.5f + 0.5f) + sin(_S1 * 9.17000007629394531f) * clamp((_S1 - 150.0f) / 50.0f, 0.0f, 1.0f) * 0.05000000074505806f * (sin(_S1 * 0.00499999988824129f) * 0.5f + 0.5f) + sin(_S1 * 0.30000001192092896f) * clamp((_S1 - 225.0f) / 10.0f, 0.0f, 1.0f) * 0.89999997615814209f * (sin(_S1 * 0.00999999977648258f) * 0.5f + 0.5f)) * smoothstep(_S1 / 2.0f, 0.0f, 1.0f) * scale_0;
}

fn GroundFunction_0( p_0 : vec2<f32>,  scale_1 : f32) -> f32
{
    return GroundHeightAtX_0(p_0.x, scale_1) - p_0.y;
}

fn GroundFunctionGradiant_0( coords_0 : vec2<f32>,  scale_2 : f32) -> vec2<f32>
{
    const _S2 : vec2<f32> = vec2<f32>(0.00999999977648258f, 0.0f);
    const _S3 : vec2<f32> = vec2<f32>(0.0f, 0.00999999977648258f);
    return vec2<f32>(GroundFunction_0(coords_0 + _S2, scale_2) - GroundFunction_0(coords_0 - _S2, scale_2), GroundFunction_0(coords_0 + _S3, scale_2) - GroundFunction_0(coords_0 - _S3, scale_2)) / vec2<f32>(0.01999999955296516f);
}

fn EstimatedDistanceFromPointToGround_0( p_1 : vec2<f32>,  scale_3 : f32) -> f32
{
    return GroundFunction_0(p_1, scale_3) / length(GroundFunctionGradiant_0(p_1, scale_3));
}

fn rand_0( co_0 : vec2<f32>) -> f32
{
    return fract(sin(dot(co_0.xy, vec2<f32>(12.98980045318603516f, 78.233001708984375f))) * 43758.546875f);
}

fn RandomFloat_0( seed_0 : vec2<f32>) -> f32
{
    return rand_0(vec2<f32>(seed_0.x * 0.64499998092651367f, 0.45300000905990601f + seed_0.y * 0.32899999618530273f));
}

fn UDFatLineSegment_0( coords_1 : vec2<f32>,  A_0 : vec2<f32>,  B_0 : vec2<f32>,  height_0 : f32) -> f32
{
    var _S4 : vec2<f32> = B_0 - A_0;
    var xAxis_0 : vec2<f32> = normalize(_S4);
    var yAxis_0 : vec2<f32> = vec2<f32>(xAxis_0.y, - xAxis_0.x);
    var width_0 : f32 = length(_S4);
    var _S5 : vec2<f32> = coords_1 - A_0;
    var relCoords_0 : vec2<f32>;
    relCoords_0[i32(0)] = dot(_S5, xAxis_0);
    relCoords_0[i32(1)] = dot(_S5, yAxis_0);
    var closestPoint_0 : vec2<f32>;
    closestPoint_0[i32(0)] = clamp(relCoords_0.x, 0.0f, width_0);
    closestPoint_0[i32(1)] = clamp(relCoords_0.y, - height_0 * 0.5f, height_0 * 0.5f);
    return length(relCoords_0 - closestPoint_0);
}

fn DrawGrass_0( uv_1 : vec2<f32>,  pixelColor_1 : ptr<function, vec3<f32>>,  tint_0 : vec3<f32>,  scale_4 : f32)
{
    var w_0 : u32;
    var h_0 : u32;
    {var dim = textureDimensions((output));((w_0)) = dim.x;((h_0)) = dim.y;};
    var _S6 : f32 = f32(w_0);
    var grassOrigin_0 : vec2<f32>;
    var _S7 : f32 = uv_1.x;
    grassOrigin_0[i32(0)] = floor(_S7 / 0.25f) * 0.25f + 0.125f;
    grassOrigin_0[i32(1)] = GroundHeightAtX_0(grassOrigin_0.x, scale_4);
    var forceTop_0 : f32;
    if((RandomFloat_0(grassOrigin_0 + vec2<f32>(0.34200000762939453f, 0.85600000619888306f))) > 0.25f)
    {
        forceTop_0 = 1.0f;
    }
    else
    {
        forceTop_0 = 0.0f;
    }
    grassOrigin_0[i32(1)] = grassOrigin_0[i32(1)] - forceTop_0 * RandomFloat_0(grassOrigin_0 + vec2<f32>(0.75599998235702515f, 0.56400001049041748f));
    var grassYAxis_0 : vec2<f32> = - GroundFunctionGradiant_0(grassOrigin_0, scale_4);
    var uvRelative_0 : vec2<f32> = uv_1 - grassOrigin_0;
    var uvLocal_0 : vec2<f32>;
    uvLocal_0[i32(0)] = dot(uvRelative_0, vec2<f32>(grassYAxis_0.y, - grassYAxis_0.x));
    uvLocal_0[i32(1)] = dot(uvRelative_0, grassYAxis_0);
    uvLocal_0 = uvLocal_0 / vec2<f32>(scale_4);
    var snowLine_0 : f32 = sin(_S7 * 2.34999990463256836f) * 0.10000000149011612f + sin(_S7 * 3.1400001049041748f) * 0.00999999977648258f;
    var _S8 : vec3<f32> = mix(vec3<f32>(0.30000001192092896f, 0.40000000596046448f, 0.10000000149011612f), vec3<f32>(0.69999998807907104f, 0.80000001192092896f, 0.5f), vec3<f32>((smoothstep(snowLine_0 - 0.30000001192092896f, snowLine_0 + 0.30000001192092896f, uv_1.y) * 0.5f)));
    var i_0 : i32 = i32(0);
    for(;;)
    {
        if(i_0 < i32(5))
        {
        }
        else
        {
            break;
        }
        var endPoint_0 : vec2<f32>;
        var _S9 : vec2<f32> = vec2<f32>(0.25400000810623169f, 0.87300002574920654f) * vec2<f32>(f32(i_0));
        endPoint_0[i32(0)] = (RandomFloat_0(grassOrigin_0 + _S9) * 2.0f - 1.0f) * 0.10000000149011612f;
        endPoint_0[i32(1)] = RandomFloat_0(grassOrigin_0 + _S9) * 0.02999999932944775f + 0.01999999955296516f;
        var startingOffset_0 : vec2<f32>;
        startingOffset_0[i32(0)] = endPoint_0.x * 0.60000002384185791f;
        startingOffset_0[i32(1)] = 0.0f;
        (*pixelColor_1) = mix((*pixelColor_1), _S8 * tint_0, vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S6, UDFatLineSegment_0(uvLocal_0, startingOffset_0, endPoint_0, 0.00999999977648258f)))));
        i_0 = i_0 + i32(1);
    }
    return;
}

fn UDCircle_0( coords_2 : vec2<f32>,  circle_0 : vec2<f32>,  radius_0 : f32) -> f32
{
    return max(length(coords_2 - circle_0.xy) - radius_0, 0.0f);
}

fn DrawTrees_0( uv_2 : vec2<f32>,  pixelColor_2 : ptr<function, vec3<f32>>,  tint_1 : vec3<f32>,  scale_5 : f32)
{
    var w_1 : u32;
    var h_1 : u32;
    {var dim = textureDimensions((output));((w_1)) = dim.x;((h_1)) = dim.y;};
    var _S10 : f32 = f32(w_1);
    var treeOrigin_0 : vec2<f32>;
    treeOrigin_0[i32(0)] = floor(uv_2.x / 3.0f) * 3.0f + 1.5f;
    var _S11 : f32 = GroundHeightAtX_0(treeOrigin_0.x, scale_5);
    treeOrigin_0[i32(1)] = _S11;
    treeOrigin_0[i32(1)] = _S11 - RandomFloat_0(treeOrigin_0 + vec2<f32>(0.75599998235702515f, 0.56400001049041748f)) * 0.5f;
    var treeYAxis_0 : vec2<f32> = - GroundFunctionGradiant_0(treeOrigin_0, scale_5);
    var uvRelative_1 : vec2<f32> = uv_2 - treeOrigin_0;
    var uvLocal_1 : vec2<f32>;
    uvLocal_1[i32(0)] = dot(uvRelative_1, vec2<f32>(treeYAxis_0.y, - treeYAxis_0.x));
    uvLocal_1[i32(1)] = dot(uvRelative_1, treeYAxis_0);
    var _S12 : vec2<f32> = uvLocal_1 / vec2<f32>(scale_5);
    uvLocal_1 = _S12;
    (*pixelColor_2) = mix((*pixelColor_2), vec3<f32>(0.60000002384185791f, 0.30000001192092896f, 0.10000000149011612f) * tint_1, vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S10, UDFatLineSegment_0(_S12, vec2<f32>(0.0f, 0.0f), vec2<f32>(0.0f, 0.15000000596046448f), 0.03500000014901161f)))));
    var dist_0 : f32 = 1.0f;
    var i_1 : i32 = i32(0);
    for(;;)
    {
        if(i_1 < i32(5))
        {
        }
        else
        {
            break;
        }
        var circle_1 : vec3<f32>;
        var _S13 : vec2<f32> = vec2<f32>(f32(i_1));
        circle_1[i32(0)] = 0.05000000074505806f * (RandomFloat_0(treeOrigin_0 + vec2<f32>(0.45300000905990601f, 0.92299997806549072f) * _S13) * 2.0f - 1.0f);
        circle_1[i32(1)] = 0.07999999821186066f + 0.20000000298023224f * RandomFloat_0(treeOrigin_0 + vec2<f32>(0.5429999828338623f, 0.13199999928474426f) * _S13);
        circle_1[i32(2)] = 0.05000000074505806f + 0.01999999955296516f * RandomFloat_0(treeOrigin_0 + vec2<f32>(0.13199999928474426f, 0.64499998092651367f) * _S13);
        var _S14 : f32 = min(dist_0, UDCircle_0(uvLocal_1, circle_1.xy, circle_1.z));
        var i_2 : i32 = i_1 + i32(1);
        dist_0 = _S14;
        i_1 = i_2;
    }
    (*pixelColor_2) = mix((*pixelColor_2), vec3<f32>(0.0f, 0.40000000596046448f, 0.0f) * tint_1, vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S10 * 3.0f, dist_0))));
    return;
}

fn DrawHills_0( uv_3 : vec2<f32>,  pixelColor_3 : ptr<function, vec3<f32>>,  tint_2 : vec3<f32>,  scale_6 : f32)
{
    var w_2 : u32;
    var h_2 : u32;
    {var dim = textureDimensions((output));((w_2)) = dim.x;((h_2)) = dim.y;};
    var _S15 : f32 = f32(w_2);
    var _S16 : f32 = uv_3.x;
    var snowLine_1 : f32 = sin(_S16 * 2.34999990463256836f) * 0.10000000149011612f + sin(_S16 * 3.1400001049041748f) * 0.00999999977648258f;
    var dist_1 : f32 = EstimatedDistanceFromPointToGround_0(uv_3, scale_6) * -1.0f;
    var _S17 : f32 = 0.85000002384185791f - smoothstep(0.0f, 1.0f, clamp(dist_1 * -3.0f, 0.0f, 1.0f)) * 0.25f;
    (*pixelColor_3) = mix(mix((*pixelColor_3), vec3<f32>(0.34999999403953552f, _S17, 0.15000000596046448f) * tint_2, vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S15, dist_1)))), mix((*pixelColor_3), vec3<f32>(_S17, _S17, _S17) * tint_2, vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S15, dist_1)))), vec3<f32>(smoothstep(snowLine_1 - 0.30000001192092896f, snowLine_1 + 0.30000001192092896f, uv_3.y)));
    DrawGrass_0(uv_3, &((*pixelColor_3)), tint_2, scale_6);
    DrawTrees_0(uv_3, &((*pixelColor_3)), tint_2, scale_6);
    return;
}

fn DrawClouds_0( uv_4 : vec2<f32>,  pixelColor_4 : ptr<function, vec3<f32>>,  tint_3 : vec3<f32>,  scale_7 : f32,  alpha_0 : f32)
{
    var w_3 : u32;
    var h_3 : u32;
    {var dim = textureDimensions((output));((w_3)) = dim.x;((h_3)) = dim.y;};
    var _S18 : f32 = f32(w_3);
    var cloudOrigin_0 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
    cloudOrigin_0[i32(0)] = floor(uv_4.x / 2.0f) * 2.0f + 1.0f;
    cloudOrigin_0[i32(1)] = RandomFloat_0(cloudOrigin_0 + vec2<f32>(0.45300000905990601f, 0.74800002574920654f) * vec2<f32>(2.0f) - vec2<f32>(1.0f)) * 2.0f;
    var uvRelative_2 : vec2<f32> = (uv_4 - cloudOrigin_0) / vec2<f32>(scale_7);
    var dist_2 : f32 = 1.0f;
    var i_3 : i32 = i32(0);
    for(;;)
    {
        if(i_3 < i32(10))
        {
        }
        else
        {
            break;
        }
        var circle_2 : vec3<f32>;
        var _S19 : vec2<f32> = vec2<f32>(f32(i_3));
        circle_2[i32(0)] = 0.5f * (RandomFloat_0(cloudOrigin_0 + vec2<f32>(0.45300000905990601f, 0.92299997806549072f) * _S19) * 2.0f - 1.0f);
        circle_2[i32(1)] = 0.07999999821186066f + 0.20000000298023224f * RandomFloat_0(cloudOrigin_0 + vec2<f32>(0.5429999828338623f, 0.13199999928474426f) * _S19);
        circle_2[i32(2)] = 0.10000000149011612f + 0.10000000149011612f * RandomFloat_0(cloudOrigin_0 + vec2<f32>(0.13199999928474426f, 0.64499998092651367f) * _S19);
        var _S20 : f32 = min(dist_2, UDCircle_0(uvRelative_2, circle_2.xy, circle_2.z));
        var i_4 : i32 = i_3 + i32(1);
        dist_2 = _S20;
        i_3 = i_4;
    }
    (*pixelColor_4) = mix((*pixelColor_4), tint_3, vec3<f32>(((1.0f - smoothstep(0.0f, 7.0f / _S18 * 40.0f, dist_2)) * alpha_0)));
    return;
}

fn AsyncPointPos_0( p_2 : vec4<f32>,  frameFraction_0 : f32) -> vec2<f32>
{
    return mix(p_2.zw, p_2.xy, vec2<f32>(frameFraction_0));
}

fn DrawWheel_0( uv_5 : vec2<f32>,  wheelPos_0 : vec2<f32>,  pixelColor_5 : ptr<function, vec3<f32>>,  touchingGround_0 : bool)
{
    var w_4 : u32;
    var h_4 : u32;
    {var dim = textureDimensions((output));((w_4)) = dim.x;((h_4)) = dim.y;};
    var _S21 : f32 = f32(w_4);
    (*pixelColor_5) = mix(mix((*pixelColor_5), vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S21, UDCircle_0(uv_5, wheelPos_0, 0.03999999910593033f))))), vec3<f32>(0.75f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S21, UDCircle_0(uv_5, wheelPos_0, 0.01999999955296516f)))));
    return;
}

fn DrawCar_0( uv_6 : vec2<f32>,  pixelColor_6 : ptr<function, vec3<f32>>,  backWheel_1 : vec4<f32>,  frontWheel_1 : vec4<f32>,  state_1 : vec4<f32>,  state2_1 : vec4<f32>)
{
    var w_5 : u32;
    var h_5 : u32;
    {var dim = textureDimensions((output));((w_5)) = dim.x;((h_5)) = dim.y;};
    var _S22 : f32 = f32(w_5);
    var _S23 : f32 = state_1.x;
    var backWheelPos_0 : vec2<f32> = AsyncPointPos_0(backWheel_1, _S23);
    var frontWheelPos_0 : vec2<f32> = AsyncPointPos_0(frontWheel_1, _S23);
    DrawWheel_0(uv_6, backWheelPos_0, &((*pixelColor_6)), (state2_1.y) == 1.0f);
    DrawWheel_0(uv_6, frontWheelPos_0, &((*pixelColor_6)), (state2_1.z) == 1.0f);
    var xAxis_1 : vec2<f32> = normalize(frontWheelPos_0 - backWheelPos_0);
    var yAxis_1 : vec2<f32> = vec2<f32>(- xAxis_1.y, xAxis_1.x);
    var uvRelative_3 : vec2<f32> = uv_6 - backWheelPos_0;
    var uvLocal_2 : vec2<f32>;
    uvLocal_2[i32(0)] = dot(uvRelative_3, xAxis_1);
    uvLocal_2[i32(1)] = dot(uvRelative_3, yAxis_1);
    const _S24 : vec2<f32> = vec2<f32>(0.03999999910593033f, 0.09000000357627869f);
    const _S25 : vec2<f32> = vec2<f32>(0.07999999821186066f, 0.09000000357627869f);
    (*pixelColor_6) = mix((*pixelColor_6), vec3<f32>(0.10000000149011612f, 0.0f, 0.0f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S22, min(min(min(UDFatLineSegment_0(uvLocal_2, vec2<f32>(-0.0625f, 0.03999999910593033f), vec2<f32>(0.20000000298023224f, 0.02999999932944775f), 0.03500000014901161f), UDFatLineSegment_0(uvLocal_2, vec2<f32>(-0.05999999865889549f, 0.03999999910593033f), _S24, 0.00999999977648258f)), UDFatLineSegment_0(uvLocal_2, _S24, _S25, 0.00999999977648258f)), UDFatLineSegment_0(uvLocal_2, _S25, vec2<f32>(0.11999999731779099f, 0.03999999910593033f), 0.00999999977648258f)) - 0.00249999994412065f))));
    return;
}

fn mod_0( x_1 : f32,  y_0 : f32) -> f32
{
    return x_1 - y_0 * floor(x_1 / y_0);
}

fn DrawGround_0( uv_7 : vec2<f32>,  cameraOffset_1 : vec2<f32>,  pixelColor_7 : ptr<function, vec3<f32>>,  backWheel_2 : vec4<f32>,  frontWheel_2 : vec4<f32>,  state_2 : vec4<f32>,  state2_2 : vec4<f32>,  state3_1 : vec4<f32>)
{
    var w_6 : u32;
    var h_6 : u32;
    {var dim = textureDimensions((output));((w_6)) = dim.x;((h_6)) = dim.y;};
    var _S26 : f32 = f32(w_6);
    var iTime_1 : f32 = _CSDisplayCB.iTime_0;
    var _S27 : vec2<f32> = uv_7 + vec2<f32>(1000.0f, -0.30000001192092896f);
    DrawHills_0(_S27 + cameraOffset_1 * vec2<f32>(-0.89999997615814209f), &((*pixelColor_7)), vec3<f32>(0.25f), 0.69999998807907104f);
    var _S28 : vec2<f32> = vec2<f32>(iTime_1);
    DrawClouds_0(_S27 + _S28 * vec2<f32>(0.05000000074505806f, 0.0f) + cameraOffset_1 * vec2<f32>(-0.85000002384185791f), &((*pixelColor_7)), vec3<f32>(0.30000001192092896f), 0.75f, 0.75f);
    var _S29 : vec2<f32> = uv_7 + vec2<f32>(300.0f, -0.10000000149011612f);
    DrawHills_0(_S29 + cameraOffset_1 * vec2<f32>(-0.80000001192092896f), &((*pixelColor_7)), vec3<f32>(0.5f), 0.80000001192092896f);
    DrawClouds_0(_S29 + _S28 * vec2<f32>(0.15000000596046448f, 0.0f) + cameraOffset_1 * vec2<f32>(-0.69999998807907104f), &((*pixelColor_7)), vec3<f32>(0.60000002384185791f), 0.75f, 0.75f);
    DrawCar_0(uv_7, &((*pixelColor_7)), backWheel_2, frontWheel_2, state_2, state2_2);
    var _S30 : vec3<f32> = vec3<f32>(1.0f);
    DrawHills_0(uv_7, &((*pixelColor_7)), _S30, 1.0f);
    var _S31 : f32 = uv_7.x;
    if(_S31 > (state3_1.y))
    {
        (*pixelColor_7) = mix((*pixelColor_7), vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S26 * 10.0f, UDCircle_0(uv_7, vec2<f32>(mod_0(_S31, 20.0f) - 10.0f + _S31, GroundHeightAtX_0(floor(_S31 / 20.0f) * 20.0f + 10.0f, 1.0f) + 0.08250000327825546f), 0.03750000149011612f)))));
    }
    DrawClouds_0(uv_7 + vec2<f32>(700.0f, -1.25f) + _S28 * vec2<f32>(0.25f, 0.0f) + cameraOffset_1 * vec2<f32>(0.5f), &((*pixelColor_7)), _S30, 1.0f, 0.5f);
    return;
}

fn number_0( x_2 : i32,  y_1 : i32,  n_0 : i32) -> bool
{
    var _S32 : bool;
    if(y_1 == i32(1))
    {
        _S32 = x_2 > i32(1);
    }
    else
    {
        _S32 = false;
    }
    if(_S32)
    {
        _S32 = x_2 < i32(5);
    }
    else
    {
        _S32 = false;
    }
    if(_S32)
    {
        _S32 = n_0 != i32(1);
    }
    else
    {
        _S32 = false;
    }
    if(_S32)
    {
        _S32 = n_0 != i32(4);
    }
    else
    {
        _S32 = false;
    }
    if(_S32)
    {
        _S32 = true;
    }
    else
    {
        if(y_1 == i32(5))
        {
            _S32 = x_2 > i32(1);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = x_2 < i32(5);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(0);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(1);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(7);
        }
        else
        {
            _S32 = false;
        }
    }
    if(_S32)
    {
        _S32 = true;
    }
    else
    {
        if(y_1 == i32(9))
        {
            _S32 = x_2 > i32(1);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = x_2 < i32(5);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(1);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(4);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(7);
        }
        else
        {
            _S32 = false;
        }
    }
    if(_S32)
    {
        _S32 = true;
    }
    else
    {
        if(x_2 == i32(1))
        {
            _S32 = y_1 > i32(1);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = y_1 < i32(5);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(1);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(2);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(3);
        }
        else
        {
            _S32 = false;
        }
    }
    if(_S32)
    {
        _S32 = true;
    }
    else
    {
        if(x_2 == i32(5))
        {
            _S32 = y_1 > i32(1);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = y_1 < i32(5);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(5);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(6);
        }
        else
        {
            _S32 = false;
        }
    }
    if(_S32)
    {
        _S32 = true;
    }
    else
    {
        if(x_2 == i32(1))
        {
            _S32 = y_1 > i32(5);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = y_1 < i32(9);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            if(n_0 == i32(0))
            {
                _S32 = true;
            }
            else
            {
                _S32 = n_0 == i32(2);
            }
            if(_S32)
            {
                _S32 = true;
            }
            else
            {
                _S32 = n_0 == i32(6);
            }
            if(_S32)
            {
                _S32 = true;
            }
            else
            {
                _S32 = n_0 == i32(8);
            }
        }
        else
        {
            _S32 = false;
        }
    }
    if(_S32)
    {
        _S32 = true;
    }
    else
    {
        if(x_2 == i32(5))
        {
            _S32 = y_1 > i32(5);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = y_1 < i32(9);
        }
        else
        {
            _S32 = false;
        }
        if(_S32)
        {
            _S32 = n_0 != i32(2);
        }
        else
        {
            _S32 = false;
        }
    }
    return _S32;
}

fn DrawDigit_0( fragCoord_0 : vec2<f32>,  digitValue_0 : i32,  digitIndex_0 : i32,  pixelColor_8 : ptr<function, vec3<f32>>)
{
    var _S33 : i32;
    if(digitValue_0 < i32(0))
    {
        _S33 = i32(0);
    }
    else
    {
        _S33 = digitValue_0;
    }
    var w_7 : u32;
    var h_7 : u32;
    {var dim = textureDimensions((output));((w_7)) = dim.x;((h_7)) = dim.y;};
    var _S34 : f32 = fragCoord_0.x;
    var indexX_0 : i32 = i32(_S34 / 25.0f);
    var _S35 : f32 = f32(h_7) - fragCoord_0.y;
    var _S36 : bool;
    if(i32(_S35 / 25.0f) > i32(0))
    {
        _S36 = true;
    }
    else
    {
        _S36 = indexX_0 != digitIndex_0;
    }
    if(_S36)
    {
        return;
    }
    var percent_0 : vec2<f32> = fract(vec2<f32>(_S34, _S35) / vec2<f32>(25.0f));
    if(number_0(i32(percent_0.x * 25.0f / 2.0f), i32(percent_0.y * 25.0f / 2.0f), _S33))
    {
        (*pixelColor_8) = vec3<f32>(1.0f);
    }
    return;
}

fn DrawScore_0( fragCoord_1 : vec2<f32>,  score_0 : f32,  pixelColor_9 : ptr<function, vec3<f32>>)
{
    var _S37 : f32 = clamp(score_0, 0.0f, 9999.0f);
    var digit1_0 : i32 = i32(mod_0(_S37 / 10.0f, 10.0f));
    var digit2_0 : i32 = i32(mod_0(_S37 / 100.0f, 10.0f));
    var digit3_0 : i32 = i32(mod_0(_S37 / 1000.0f, 10.0f));
    DrawDigit_0(fragCoord_1, i32(mod_0(_S37, 10.0f)), i32(3), &((*pixelColor_9)));
    DrawDigit_0(fragCoord_1, digit1_0, i32(2), &((*pixelColor_9)));
    DrawDigit_0(fragCoord_1, digit2_0, i32(1), &((*pixelColor_9)));
    DrawDigit_0(fragCoord_1, digit3_0, i32(0), &((*pixelColor_9)));
    return;
}

fn DrawSpeedometer_0( fragCoord_2 : vec2<f32>,  speedPercent_0 : f32,  pixelColor_10 : ptr<function, vec3<f32>>)
{
    var w_8 : u32;
    var h_8 : u32;
    {var dim = textureDimensions((output));((w_8)) = dim.x;((h_8)) = dim.y;};
    var _S38 : f32 = f32(w_8);
    var _S39 : f32 = f32(h_8);
    var aspectRatio_0 : f32 = _S38 / _S39;
    var uv_8 : vec2<f32> = fragCoord_2 / vec2<f32>(_S38, _S39) - vec2<f32>(0.5f);
    uv_8[i32(0)] = uv_8[i32(0)] * aspectRatio_0;
    var center_0 : vec2<f32> = vec2<f32>(0.5f * aspectRatio_0 - (0.15000000596046448f + 7.0f / _S38), -0.5f + (0.15000000596046448f + 7.0f / _S38));
    if((UDCircle_0(uv_8, center_0, 0.15000000596046448f)) > (7.0f / _S38))
    {
        return;
    }
    (*pixelColor_10) = mix(mix((*pixelColor_10), vec3<f32>(0.80000001192092896f, 0.60000002384185791f, 0.0f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S38, UDCircle_0(uv_8, center_0, 0.15000000596046448f))))), vec3<f32>(0.10000000149011612f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S38, UDCircle_0(uv_8, center_0, 0.15000000596046448f - 7.0f / _S38)))));
    var relativePoint_0 : vec2<f32> = uv_8 - center_0;
    var relativePointAngle_0 : f32 = mod_0(atan2(relativePoint_0.y, relativePoint_0.x) + 0.78539818525314331f, 6.28318548202514648f);
    if(relativePointAngle_0 < 4.71238899230957031f)
    {
        var fakePoint_0 : vec2<f32> = vec2<f32>(length(relativePoint_0) / 0.15000000596046448f, relativePointAngle_0);
        fakePoint_0[i32(1)] = mod_0(relativePointAngle_0, 0.40000000596046448f) - 0.20000000298023224f;
        (*pixelColor_10) = mix((*pixelColor_10), vec3<f32>(1.0f, 1.0f, 0.0f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S38 * 5.0f, UDFatLineSegment_0(fakePoint_0, vec2<f32>(0.85000002384185791f, 0.0f), vec2<f32>(0.94999998807907104f, 0.0f), 0.05000000074505806f)))));
    }
    var targetAngle_0 : f32 = (1.0f - clamp(speedPercent_0, 0.0f, 1.0f)) * 3.14159274101257324f * 1.5f - 0.78539818525314331f;
    const _S40 : vec3<f32> = vec3<f32>(1.0f, 0.0f, 0.0f);
    (*pixelColor_10) = mix(mix((*pixelColor_10), _S40, vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S38, UDFatLineSegment_0(uv_8, center_0, center_0 + vec2<f32>(0.13500000536441803f) * vec2<f32>(cos(targetAngle_0), sin(targetAngle_0)), 0.00300000002607703f))))), _S40, vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S38, UDCircle_0(uv_8, center_0, 7.0f / _S38)))));
    return;
}

fn DrawFuelBar_0( fragCoord_3 : vec2<f32>,  fuelPercent_0 : f32,  pixelColor_11 : ptr<function, vec3<f32>>)
{
    var w_9 : u32;
    var h_9 : u32;
    {var dim = textureDimensions((output));((w_9)) = dim.x;((h_9)) = dim.y;};
    var _S41 : f32 = f32(w_9);
    var _S42 : f32 = f32(h_9);
    var _S43 : f32 = min(fuelPercent_0, 1.0f);
    var aspectRatio_1 : f32 = _S41 / _S42;
    var uv_9 : vec2<f32> = fragCoord_3 / vec2<f32>(_S41, _S42) - vec2<f32>(0.5f);
    uv_9[i32(0)] = uv_9[i32(0)] * aspectRatio_1;
    var _S44 : f32 = -0.5f * aspectRatio_1 + 0.00999999977648258f;
    var boxPosLeft_0 : vec2<f32> = vec2<f32>(_S44, 0.5f - (0.05000000074505806f + 25.0f / _S42));
    var _S45 : vec2<f32> = vec2<f32>(_S44 + 0.20000000298023224f, 0.5f - (0.05000000074505806f + 25.0f / _S42));
    var boxPosRight_0 : vec2<f32> = _S45;
    (*pixelColor_11) = mix((*pixelColor_11), vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S41, UDFatLineSegment_0(uv_9, boxPosLeft_0, _S45, 0.05000000074505806f)))));
    if(_S43 > 0.0f)
    {
        boxPosRight_0[i32(0)] = _S44 + (boxPosRight_0.x - _S44) * _S43;
        (*pixelColor_11) = mix((*pixelColor_11), vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>((1.0f - smoothstep(0.0f, 7.0f / _S41, UDFatLineSegment_0(uv_9, boxPosLeft_0, boxPosRight_0, 0.05000000074505806f)))));
    }
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn CSDisplay(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S46 : vec4<f32> = gameState[i32(0)].state_0;
    var _S47 : vec4<f32> = gameState[i32(0)].state2_0;
    var _S48 : vec4<f32> = gameState[i32(0)].state3_0;
    var _S49 : vec4<f32> = gameState[i32(0)].backWheel_0;
    var _S50 : vec4<f32> = gameState[i32(0)].frontWheel_0;
    var w_10 : u32;
    var h_10 : u32;
    {var dim = textureDimensions((output));((w_10)) = dim.x;((h_10)) = dim.y;};
    var _S51 : f32 = f32(w_10);
    var _S52 : f32 = f32(h_10);
    var _S53 : vec2<u32> = DTid_0.xy;
    var _S54 : vec2<f32> = vec2<f32>(_S53);
    var fragCoord_4 : vec2<f32> = _S54;
    fragCoord_4[i32(1)] = _S52 - _S54.y;
    var aspectRatio_2 : f32 = _S51 / _S52;
    var uv_10 : vec2<f32> = fragCoord_4 / vec2<f32>(_S51, _S52) - vec2<f32>(0.5f);
    uv_10[i32(0)] = uv_10[i32(0)] * aspectRatio_2;
    var _S55 : vec2<f32> = _S46.zw;
    var _S56 : vec2<f32> = uv_10 * vec2<f32>(_S47.x) + _S55;
    uv_10 = _S56;
    var pixelColor_12 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    DrawSky_0(_S56, _S55, &(pixelColor_12));
    DrawGround_0(_S56, _S55, &(pixelColor_12), _S49, _S50, _S46, _S47, _S48);
    DrawScore_0(fragCoord_4, _S55.x, &(pixelColor_12));
    DrawSpeedometer_0(fragCoord_4, _S48.z, &(pixelColor_12));
    DrawFuelBar_0(fragCoord_4, _S48.w, &(pixelColor_12));
    if((_S47.w) == 1.0f)
    {
        pixelColor_12 = mix(vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>(dot(pixelColor_12, vec3<f32>(0.30000001192092896f, 0.5899999737739563f, 0.10999999940395355f))), vec3<f32>((_S48.x * 0.75f + 0.25f)));
    }
    textureStore((output), (_S53), (vec4<f32>(pixelColor_12, 1.0f)));
    return;
}

`;

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct GameState
static StructVertexBufferAttributes_GameState =
[
    // state
    {
        format: "float32x4",
        offset: 0,
        shaderLocation: 0,
    },
    // state2
    {
        format: "float32x4",
        offset: 16,
        shaderLocation: 1,
    },
    // state3
    {
        format: "float32x4",
        offset: 32,
        shaderLocation: 2,
    },
    // state4
    {
        format: "float32x4",
        offset: 48,
        shaderLocation: 3,
    },
    // backWheel
    {
        format: "float32x4",
        offset: 64,
        shaderLocation: 4,
    },
    // frontWheel
    {
        format: "float32x4",
        offset: 80,
        shaderLocation: 5,
    },
    // frontWheelCP1
    {
        format: "float32x4",
        offset: 96,
        shaderLocation: 6,
    },
    // frontWheelCP2
    {
        format: "float32x4",
        offset: 112,
        shaderLocation: 7,
    },
    // backWheelCP1
    {
        format: "float32x4",
        offset: 128,
        shaderLocation: 8,
    },
    // backWheelCP2
    {
        format: "float32x4",
        offset: 144,
        shaderLocation: 9,
    },
];

// -------------------- Private Members

// Buffer GameState
buffer_GameState = null;
buffer_GameState_count = 0;
buffer_GameState_stride = 0;
buffer_GameState_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer KeyStates
buffer_KeyStates = null;
buffer_KeyStates_count = 0;
buffer_KeyStates_stride = 0;
buffer_KeyStates_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Texture Display_output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Display_output_ReadOnly = null;
texture_Display_output_ReadOnly_size = [0, 0, 0];
texture_Display_output_ReadOnly_format = "";
texture_Display_output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _CSGameplayCB
constantBuffer__CSGameplayCB = null;
constantBuffer__CSGameplayCB_size = 16;
constantBuffer__CSGameplayCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Gameplay
Hash_Compute_Gameplay = 0;
ShaderModule_Compute_Gameplay = null;
BindGroupLayout_Compute_Gameplay = null;
PipelineLayout_Compute_Gameplay = null;
Pipeline_Compute_Gameplay = null;

// Constant buffer _CSDisplayCB
constantBuffer__CSDisplayCB = null;
constantBuffer__CSDisplayCB_size = 16;
constantBuffer__CSDisplayCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Display
Hash_Compute_Display = 0;
ShaderModule_Compute_Display = null;
BindGroupLayout_Compute_Display = null;
PipelineLayout_Compute_Display = null;
Pipeline_Compute_Display = null;

// -------------------- Exported Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

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

// -------------------- Structs

static StructOffsets_GameState =
{
    state_0: 0,
    state_1: 4,
    state_2: 8,
    state_3: 12,
    state2_0: 16,
    state2_1: 20,
    state2_2: 24,
    state2_3: 28,
    state3_0: 32,
    state3_1: 36,
    state3_2: 40,
    state3_3: 44,
    state4_0: 48,
    state4_1: 52,
    state4_2: 56,
    state4_3: 60,
    backWheel_0: 64,
    backWheel_1: 68,
    backWheel_2: 72,
    backWheel_3: 76,
    frontWheel_0: 80,
    frontWheel_1: 84,
    frontWheel_2: 88,
    frontWheel_3: 92,
    frontWheelCP1_0: 96,
    frontWheelCP1_1: 100,
    frontWheelCP1_2: 104,
    frontWheelCP1_3: 108,
    frontWheelCP2_0: 112,
    frontWheelCP2_1: 116,
    frontWheelCP2_2: 120,
    frontWheelCP2_3: 124,
    backWheelCP1_0: 128,
    backWheelCP1_1: 132,
    backWheelCP1_2: 136,
    backWheelCP1_3: 140,
    backWheelCP2_0: 144,
    backWheelCP2_1: 148,
    backWheelCP2_2: 152,
    backWheelCP2_3: 156,
    _size: 160,
}

static StructOffsets__CSGameplayCB =
{
    iFrame: 0,
    iTimeDelta: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__CSDisplayCB =
{
    iTime: 0,
    _padding0: 4,
    _padding1: 8,
    _padding2: 12,
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
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1024) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 768) / 1) + 0,
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
                label: "texture DemofoxVerletCar.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of buffer GameState
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = 160;
        if (this.buffer_GameState !== null && (this.buffer_GameState_count != desiredCount || this.buffer_GameState_stride != desiredStride))
        {
            this.buffer_GameState.destroy();
            this.buffer_GameState = null;
        }

        if (this.buffer_GameState === null)
        {
            this.buffer_GameState_count = desiredCount;
            this.buffer_GameState_stride = desiredStride;
            this.buffer_GameState = device.createBuffer({
                label: "buffer DemofoxVerletCar.GameState",
                size: Shared.Align(16, this.buffer_GameState_count * this.buffer_GameState_stride),
                usage: this.buffer_GameState_usageFlags,
            });
        }
    }

    // Handle (re)creation of buffer KeyStates
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 512) / 1) + 0;
        const desiredStride = 4;
        if (this.buffer_KeyStates !== null && (this.buffer_KeyStates_count != desiredCount || this.buffer_KeyStates_stride != desiredStride))
        {
            this.buffer_KeyStates.destroy();
            this.buffer_KeyStates = null;
        }

        if (this.buffer_KeyStates === null)
        {
            this.buffer_KeyStates_count = desiredCount;
            this.buffer_KeyStates_stride = desiredStride;
            this.buffer_KeyStates = device.createBuffer({
                label: "buffer DemofoxVerletCar.KeyStates",
                size: Shared.Align(16, this.buffer_KeyStates_count * this.buffer_KeyStates_stride),
                usage: this.buffer_KeyStates_usageFlags,
            });
        }
    }

    // Handle (re)creation of texture Display_output_ReadOnly
    {
        const baseSize = this.texture_Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Output_format;
        if (this.texture_Display_output_ReadOnly !== null && (this.texture_Display_output_ReadOnly_format != desiredFormat || this.texture_Display_output_ReadOnly_size[0] != desiredSize[0] || this.texture_Display_output_ReadOnly_size[1] != desiredSize[1] || this.texture_Display_output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Display_output_ReadOnly.destroy();
            this.texture_Display_output_ReadOnly = null;
        }

        if (this.texture_Display_output_ReadOnly === null)
        {
            this.texture_Display_output_ReadOnly_size = desiredSize.slice();
            this.texture_Display_output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Display_output_ReadOnly_format))
                viewFormats.push(this.texture_Display_output_ReadOnly_format);

            this.texture_Display_output_ReadOnly = device.createTexture({
                label: "texture DemofoxVerletCar.Display_output_ReadOnly",
                size: this.texture_Display_output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Display_output_ReadOnly_format),
                usage: this.texture_Display_output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Gameplay
    {
        const bindGroupEntries =
        [
            {
                // gameState
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // keyStates
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // _CSGameplayCB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Gameplay === null || newHash !== this.Hash_Compute_Gameplay)
        {
            this.Hash_Compute_Gameplay = newHash;

            let shaderCode = class_DemofoxVerletCar.ShaderCode_Gameplay_CSGameplay;

            this.ShaderModule_Compute_Gameplay = device.createShaderModule({ code: shaderCode, label: "Compute Shader Gameplay"});
            this.BindGroupLayout_Compute_Gameplay = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Gameplay",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Gameplay = device.createPipelineLayout({
                label: "Compute Pipeline Layout Gameplay",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Gameplay],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Gameplay = device.createComputePipeline({
                    label: "Compute Pipeline Gameplay",
                    layout: this.PipelineLayout_Compute_Gameplay,
                    compute: {
                        module: this.ShaderModule_Compute_Gameplay,
                        entryPoint: "CSGameplay",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Gameplay");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Gameplay",
                    layout: this.PipelineLayout_Compute_Gameplay,
                    compute: {
                        module: this.ShaderModule_Compute_Gameplay,
                        entryPoint: "CSGameplay",
                    }
                }).then( handle => { this.Pipeline_Compute_Gameplay = handle; this.loadingPromises.delete("Gameplay"); } );
            }
        }
    }

    // (Re)create compute shader Display
    {
        const bindGroupEntries =
        [
            {
                // gameState
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // output
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // outputReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Display_output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _CSDisplayCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Display === null || newHash !== this.Hash_Compute_Display)
        {
            this.Hash_Compute_Display = newHash;

            let shaderCode = class_DemofoxVerletCar.ShaderCode_Display_CSDisplay;
            shaderCode = shaderCode.replace("/*(output_format)*/", Shared.GetNonSRGBFormat(this.texture_Output_format));
            shaderCode = shaderCode.replace("/*(outputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Display_output_ReadOnly_format));

            this.ShaderModule_Compute_Display = device.createShaderModule({ code: shaderCode, label: "Compute Shader Display"});
            this.BindGroupLayout_Compute_Display = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Display",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Display = device.createPipelineLayout({
                label: "Compute Pipeline Layout Display",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Display],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Display = device.createComputePipeline({
                    label: "Compute Pipeline Display",
                    layout: this.PipelineLayout_Compute_Display,
                    compute: {
                        module: this.ShaderModule_Compute_Display,
                        entryPoint: "CSDisplay",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Display");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Display",
                    layout: this.PipelineLayout_Compute_Display,
                    compute: {
                        module: this.ShaderModule_Compute_Display,
                        entryPoint: "CSDisplay",
                    }
                }).then( handle => { this.Pipeline_Compute_Display = handle; this.loadingPromises.delete("Display"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("DemofoxVerletCar.Output");

    encoder.popDebugGroup(); // "DemofoxVerletCar.Output"

    encoder.pushDebugGroup("DemofoxVerletCar.GameState");

    encoder.popDebugGroup(); // "DemofoxVerletCar.GameState"

    encoder.pushDebugGroup("DemofoxVerletCar.KeyStates");

    encoder.popDebugGroup(); // "DemofoxVerletCar.KeyStates"

    encoder.pushDebugGroup("DemofoxVerletCar.Display_output_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxVerletCar.Display_output_ReadOnly"

    encoder.pushDebugGroup("DemofoxVerletCar.Copy_Display_output");

    // Copy texture Output to texture Display_output_ReadOnly
    {
        const numMips = Math.min(this.texture_Output.mipLevelCount, this.texture_Display_output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Output.depthOrArrayLayers;

            if (this.texture_Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Output, mipLevel: mipIndex },
                { texture: this.texture_Display_output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "DemofoxVerletCar.Copy_Display_output"

    encoder.pushDebugGroup("DemofoxVerletCar._CSGameplayCB");

    // Create constant buffer _CSGameplayCB
    if (this.constantBuffer__CSGameplayCB === null)
    {
        this.constantBuffer__CSGameplayCB = device.createBuffer({
            label: "DemofoxVerletCar._CSGameplayCB",
            size: Shared.Align(16, this.constructor.StructOffsets__CSGameplayCB._size),
            usage: this.constantBuffer__CSGameplayCB_usageFlags,
        });
    }

    // Upload values to constant buffer _CSGameplayCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__CSGameplayCB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__CSGameplayCB.iFrame, this.variable_iFrame, true);
        view.setFloat32(this.constructor.StructOffsets__CSGameplayCB.iTimeDelta, this.variable_iTimeDelta, true);
        device.queue.writeBuffer(this.constantBuffer__CSGameplayCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxVerletCar._CSGameplayCB"

    encoder.pushDebugGroup("DemofoxVerletCar.Gameplay");

    // Run compute shader Gameplay
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Gameplay",
            layout: this.BindGroupLayout_Compute_Gameplay,
            entries: [
                {
                    // gameState
                    binding: 0,
                    resource: { buffer: this.buffer_GameState }
                },
                {
                    // keyStates
                    binding: 1,
                    resource: { buffer: this.buffer_KeyStates }
                },
                {
                    // _CSGameplayCB
                    binding: 2,
                    resource: { buffer: this.constantBuffer__CSGameplayCB }
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

        if (this.Pipeline_Compute_Gameplay !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Gameplay);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxVerletCar.Gameplay"

    encoder.pushDebugGroup("DemofoxVerletCar._CSDisplayCB");

    // Create constant buffer _CSDisplayCB
    if (this.constantBuffer__CSDisplayCB === null)
    {
        this.constantBuffer__CSDisplayCB = device.createBuffer({
            label: "DemofoxVerletCar._CSDisplayCB",
            size: Shared.Align(16, this.constructor.StructOffsets__CSDisplayCB._size),
            usage: this.constantBuffer__CSDisplayCB_usageFlags,
        });
    }

    // Upload values to constant buffer _CSDisplayCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__CSDisplayCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__CSDisplayCB.iTime, this.variable_iTime, true);
        device.queue.writeBuffer(this.constantBuffer__CSDisplayCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxVerletCar._CSDisplayCB"

    encoder.pushDebugGroup("DemofoxVerletCar.Display");

    // Run compute shader Display
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Display",
            layout: this.BindGroupLayout_Compute_Display,
            entries: [
                {
                    // gameState
                    binding: 0,
                    resource: { buffer: this.buffer_GameState }
                },
                {
                    // output
                    binding: 1,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // outputReadOnly
                    binding: 2,
                    resource: this.texture_Display_output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _CSDisplayCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__CSDisplayCB }
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

        if (this.Pipeline_Compute_Display !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Display);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxVerletCar.Display"

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

var DemofoxVerletCar = new class_DemofoxVerletCar;

export default DemofoxVerletCar;
