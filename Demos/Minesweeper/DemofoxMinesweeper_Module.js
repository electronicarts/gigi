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

class class_DemofoxMinesweeper
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Gameplay_CS", node "Gameplay"
static ShaderCode_Gameplay_Gameplay_CS = `
struct Struct_Gameplay_CSCB_std140_0
{
    @align(16) MouseState_0 : vec4<f32>,
    @align(16) MouseStateLastFrame_0 : vec4<f32>,
    @align(16) Reset_0 : u32,
    @align(4) iFrame_0 : i32,
    @align(8) iTime_0 : f32,
    @align(4) _padding0_0 : f32,
};

@binding(4) @group(0) var<uniform> _Gameplay_CSCB : Struct_Gameplay_CSCB_std140_0;
@binding(1) @group(0) var<storage, read_write> gameState : array<vec4<f32>>;

@binding(3) @group(0) var gameBoardReadOnly : texture_storage_2d</*(gameBoardReadOnly_format)*/, read>;

@binding(2) @group(0) var output : texture_2d<f32>;

@binding(0) @group(0) var gameBoard : texture_storage_2d</*(gameBoard_format)*/, write>;

fn GetCellData_0( cell_0 : vec2<f32>) -> vec4<f32>
{
    var _S1 : f32 = cell_0.x;
    var _S2 : bool;
    if(_S1 >= 0.0f)
    {
        _S2 = (cell_0.y) >= 0.0f;
    }
    else
    {
        _S2 = false;
    }
    if(_S2)
    {
        _S2 = _S1 <= 15.0f;
    }
    else
    {
        _S2 = false;
    }
    if(_S2)
    {
        _S2 = (cell_0.y) <= 15.0f;
    }
    else
    {
        _S2 = false;
    }
    if(_S2)
    {
        var _S3 : vec4<f32> = (textureLoad((gameBoardReadOnly), (vec2<i32>(vec2<u32>(cell_0)))));
        return _S3;
    }
    else
    {
        return vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

fn PixelToCell_0( fragCoord_0 : vec2<f32>,  uv_0 : ptr<function, vec2<f32>>,  cell_1 : ptr<function, vec2<f32>>,  cellFract_0 : ptr<function, vec2<f32>>)
{
    var w_0 : u32;
    var h_0 : u32;
    {var dim = textureDimensions((output));((w_0)) = dim.x;((h_0)) = dim.y;};
    var _S4 : f32 = f32(w_0);
    var _S5 : f32 = f32(h_0);
    var aspectRatio_0 : f32 = _S4 / _S5;
    (*uv_0) = fragCoord_0.xy / vec2<f32>(_S4, _S5) * vec2<f32>(1.20000004768371582f) - vec2<f32>(0.10000000149011612f, 0.10000000149011612f);
    (*uv_0)[i32(0)] = (*uv_0)[i32(0)] * aspectRatio_0;
    const _S6 : vec2<f32> = vec2<f32>(16.0f, 16.0f);
    (*cell_1) = floor((*uv_0) * _S6);
    (*cellFract_0) = fract((*uv_0) * _S6);
    return;
}

fn rand_0( co_0 : vec2<f32>) -> f32
{
    return fract(sin(dot(co_0.xy, vec2<f32>(12.98980045318603516f, 78.233001708984375f))) * 43758.546875f);
}

@compute
@workgroup_size(8, 8, 1)
fn gameplaycs(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S7 : u32 = DTid_0.x;
    var _S8 : bool;
    if(f32(_S7) >= 16.0f)
    {
        _S8 = true;
    }
    else
    {
        _S8 = f32(DTid_0.y) >= 16.0f;
    }
    if(_S8)
    {
        return;
    }
    var _S9 : vec2<f32> = vec2<f32>(DTid_0.xy);
    var gameStateReadIndex_0 : u32 = u32(_Gameplay_CSCB.iFrame_0) % u32(2);
    var gameStateWriteIndex_0 : u32 = (gameStateReadIndex_0 + u32(1)) % u32(2);
    var state_0 : vec4<f32> = gameState[gameStateReadIndex_0];
    var _S10 : vec2<u32> = vec2<u32>(_S9);
    var _S11 : vec4<f32> = (textureLoad((gameBoardReadOnly), (vec2<i32>(_S10))));
    var cellData_0 : vec4<f32> = _S11;
    var cellCheck_0 : vec2<f32> = state_0.zw;
    var cellDataUL_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(-1.0f, 1.0f));
    var cellDataU_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(0.0f, 1.0f));
    var cellDataUR_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(1.0f, 1.0f));
    var cellDataR_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(1.0f, 0.0f));
    var cellDataDR_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(1.0f, -1.0f));
    var cellDataD_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(0.0f, -1.0f));
    var cellDataDL_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(-1.0f, -1.0f));
    var cellDataL_0 : vec4<f32> = GetCellData_0(_S9 + vec2<f32>(-1.0f, 0.0f));
    var mouseUv_0 : vec2<f32>;
    var mouseCell_0 : vec2<f32>;
    var mouseCellFract_0 : vec2<f32>;
    PixelToCell_0(_Gameplay_CSCB.MouseState_0.xy, &(mouseUv_0), &(mouseCell_0), &(mouseCellFract_0));
    if((_Gameplay_CSCB.iFrame_0) == i32(0))
    {
        _S8 = true;
    }
    else
    {
        _S8 = bool(_Gameplay_CSCB.Reset_0);
    }
    if(_S8)
    {
        state_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    }
    if((state_0.x) < 0.10000000149011612f)
    {
        state_0[i32(0)] = 0.10000000149011612f;
        state_0[i32(1)] = 0.0f;
        const _S12 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
        state_0.z = _S12.x;
        state_0.w = _S12.y;
        cellData_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
        if((rand_0(_S9 + vec2<f32>(_Gameplay_CSCB.iTime_0))) < 0.125f)
        {
            cellData_0[i32(2)] = 1.0f;
        }
    }
    else
    {
        if((state_0.x) < 0.20000000298023224f)
        {
            state_0[i32(0)] = 0.20000000298023224f;
            var _S13 : f32;
            if((cellDataUL_0.z) > 0.0f)
            {
                _S13 = 1.0f;
            }
            else
            {
                _S13 = 0.0f;
            }
            var _S14 : f32;
            if((cellDataU_0.z) > 0.0f)
            {
                _S14 = 1.0f;
            }
            else
            {
                _S14 = 0.0f;
            }
            var _S15 : f32 = _S13 + _S14;
            if((cellDataUR_0.z) > 0.0f)
            {
                _S13 = 1.0f;
            }
            else
            {
                _S13 = 0.0f;
            }
            var _S16 : f32 = _S15 + _S13;
            if((cellDataR_0.z) > 0.0f)
            {
                _S13 = 1.0f;
            }
            else
            {
                _S13 = 0.0f;
            }
            var _S17 : f32 = _S16 + _S13;
            if((cellDataDR_0.z) > 0.0f)
            {
                _S13 = 1.0f;
            }
            else
            {
                _S13 = 0.0f;
            }
            var _S18 : f32 = _S17 + _S13;
            if((cellDataD_0.z) > 0.0f)
            {
                _S13 = 1.0f;
            }
            else
            {
                _S13 = 0.0f;
            }
            var _S19 : f32 = _S18 + _S13;
            if((cellDataDL_0.z) > 0.0f)
            {
                _S13 = 1.0f;
            }
            else
            {
                _S13 = 0.0f;
            }
            var _S20 : f32 = _S19 + _S13;
            if((cellDataL_0.z) > 0.0f)
            {
                _S13 = 1.0f;
            }
            else
            {
                _S13 = 0.0f;
            }
            cellData_0[i32(1)] = (_S20 + _S13) / 255.0f;
        }
        else
        {
            if((state_0.x) < 0.30000001192092896f)
            {
                if((_Gameplay_CSCB.MouseStateLastFrame_0.w) == 1.0f)
                {
                    _S8 = (_Gameplay_CSCB.MouseState_0.w) == 0.0f;
                }
                else
                {
                    _S8 = false;
                }
                if(_S8)
                {
                    if((_S9.x) == (mouseCell_0.x))
                    {
                        _S8 = (_S9.y) == (mouseCell_0.y);
                    }
                    else
                    {
                        _S8 = false;
                    }
                    if(_S8)
                    {
                        cellData_0[i32(3)] = 1.0f - cellData_0.w;
                    }
                }
                if((_Gameplay_CSCB.MouseStateLastFrame_0.z) == 1.0f)
                {
                    _S8 = (_Gameplay_CSCB.MouseState_0.z) == 0.0f;
                }
                else
                {
                    _S8 = false;
                }
                if(_S8)
                {
                    var mouseCellData_0 : vec4<f32> = GetCellData_0(mouseCell_0);
                    if((mouseCellData_0.w) < 1.0f)
                    {
                        if((_S9.x) == (mouseCell_0.x))
                        {
                            _S8 = (_S9.y) == (mouseCell_0.y);
                        }
                        else
                        {
                            _S8 = false;
                        }
                        if(_S8)
                        {
                            cellData_0[i32(0)] = 1.0f;
                        }
                        if((mouseCellData_0.z) == 1.0f)
                        {
                            state_0[i32(0)] = 0.30000001192092896f;
                        }
                    }
                }
                if((cellData_0.x) == 0.0f)
                {
                    if((cellDataUL_0.x) == 1.0f)
                    {
                        _S8 = (cellDataUL_0.y) == 0.0f;
                    }
                    else
                    {
                        _S8 = false;
                    }
                    if(_S8)
                    {
                        _S8 = true;
                    }
                    else
                    {
                        if((cellDataU_0.x) == 1.0f)
                        {
                            _S8 = (cellDataU_0.y) == 0.0f;
                        }
                        else
                        {
                            _S8 = false;
                        }
                    }
                    if(_S8)
                    {
                        _S8 = true;
                    }
                    else
                    {
                        if((cellDataUR_0.x) == 1.0f)
                        {
                            _S8 = (cellDataUR_0.y) == 0.0f;
                        }
                        else
                        {
                            _S8 = false;
                        }
                    }
                    if(_S8)
                    {
                        _S8 = true;
                    }
                    else
                    {
                        if((cellDataR_0.x) == 1.0f)
                        {
                            _S8 = (cellDataR_0.y) == 0.0f;
                        }
                        else
                        {
                            _S8 = false;
                        }
                    }
                    if(_S8)
                    {
                        _S8 = true;
                    }
                    else
                    {
                        if((cellDataDR_0.x) == 1.0f)
                        {
                            _S8 = (cellDataDR_0.y) == 0.0f;
                        }
                        else
                        {
                            _S8 = false;
                        }
                    }
                    if(_S8)
                    {
                        _S8 = true;
                    }
                    else
                    {
                        if((cellDataD_0.x) == 1.0f)
                        {
                            _S8 = (cellDataD_0.y) == 0.0f;
                        }
                        else
                        {
                            _S8 = false;
                        }
                    }
                    if(_S8)
                    {
                        _S8 = true;
                    }
                    else
                    {
                        if((cellDataDL_0.x) == 1.0f)
                        {
                            _S8 = (cellDataDL_0.y) == 0.0f;
                        }
                        else
                        {
                            _S8 = false;
                        }
                    }
                    if(_S8)
                    {
                        _S8 = true;
                    }
                    else
                    {
                        if((cellDataL_0.x) == 1.0f)
                        {
                            _S8 = (cellDataL_0.y) == 0.0f;
                        }
                        else
                        {
                            _S8 = false;
                        }
                    }
                    if(_S8)
                    {
                        cellData_0[i32(0)] = 1.0f;
                    }
                }
                var i_0 : i32 = i32(0);
                for(;;)
                {
                    if(i_0 < i32(16))
                    {
                    }
                    else
                    {
                        break;
                    }
                    if((cellCheck_0.x) == 15.0f)
                    {
                        cellCheck_0[i32(0)] = 0.0f;
                        if((cellCheck_0.y) == 15.0f)
                        {
                            cellCheck_0[i32(1)] = 0.0f;
                            state_0[i32(0)] = 0.40000000596046448f;
                            break;
                        }
                        else
                        {
                            cellCheck_0[i32(1)] = cellCheck_0.y + 1.0f;
                        }
                    }
                    else
                    {
                        cellCheck_0[i32(0)] = cellCheck_0.x + 1.0f;
                    }
                    var cellCheckData_0 : vec4<f32> = GetCellData_0(cellCheck_0);
                    if((cellCheckData_0.x) == 0.0f)
                    {
                        _S8 = (cellCheckData_0.z) == 0.0f;
                    }
                    else
                    {
                        _S8 = false;
                    }
                    if(_S8)
                    {
                        cellCheck_0 = vec2<f32>(0.0f, 0.0f);
                        break;
                    }
                    i_0 = i_0 + i32(1);
                }
            }
            else
            {
                if(bool(_Gameplay_CSCB.Reset_0))
                {
                    state_0[i32(0)] = 0.0f;
                }
            }
        }
    }
    var _S21 : vec2<f32> = cellCheck_0;
    state_0.z = _S21.x;
    state_0.w = _S21.y;
    textureStore((gameBoard), (_S10), (cellData_0));
    if(_S7 == u32(0))
    {
        _S8 = (DTid_0.y) == u32(0);
    }
    else
    {
        _S8 = false;
    }
    if(_S8)
    {
        _S8 = (DTid_0.z) == u32(0);
    }
    else
    {
        _S8 = false;
    }
    if(_S8)
    {
        gameState[gameStateWriteIndex_0] = state_0;
    }
    return;
}

`;

// Shader code for Compute shader "Display_CS", node "Display"
static ShaderCode_Display_Display_CS = `
struct Struct_Display_CSCB_std140_0
{
    @align(16) MouseState_0 : vec4<f32>,
    @align(16) iFrame_0 : i32,
    @align(4) iTime_0 : f32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(5) @group(0) var<uniform> _Display_CSCB : Struct_Display_CSCB_std140_0;
@binding(1) @group(0) var<storage, read> gameState : array<vec4<f32>>;

@binding(2) @group(0) var output : texture_storage_2d</*(output_format)*/, write>;

@binding(4) @group(0) var _loadedTexture_0 : texture_2d<f32>;

@binding(6) @group(0) var texSampler : sampler;

@binding(0) @group(0) var gameBoard : texture_2d<f32>;

fn PixelToCell_0( fragCoord_0 : vec2<f32>,  uv_0 : ptr<function, vec2<f32>>,  cell_0 : ptr<function, vec2<f32>>,  cellFract_0 : ptr<function, vec2<f32>>)
{
    var w_0 : u32;
    var h_0 : u32;
    {var dim = textureDimensions((output));((w_0)) = dim.x;((h_0)) = dim.y;};
    var _S1 : f32 = f32(w_0);
    var _S2 : f32 = f32(h_0);
    var aspectRatio_0 : f32 = _S1 / _S2;
    (*uv_0) = fragCoord_0.xy / vec2<f32>(_S1, _S2) * vec2<f32>(1.20000004768371582f) - vec2<f32>(0.10000000149011612f, 0.10000000149011612f);
    (*uv_0)[i32(0)] = (*uv_0)[i32(0)] * aspectRatio_0;
    const _S3 : vec2<f32> = vec2<f32>(16.0f, 16.0f);
    (*cell_0) = floor((*uv_0) * _S3);
    (*cellFract_0) = fract((*uv_0) * _S3);
    return;
}

fn BackgroundPixel_0( uv_1 : vec2<f32>) -> vec3<f32>
{
    var w_1 : u32;
    var h_1 : u32;
    {var dim = textureDimensions((_loadedTexture_0));((w_1)) = dim.x;((h_1)) = dim.y;};
    var _S4 : vec2<f32> = vec2<f32>(10.0f);
    var offsetG_0 : vec2<f32> = vec2<f32>(sin(_Display_CSCB.iTime_0 * 1.53400003910064697f), cos(_Display_CSCB.iTime_0 * 1.45299994945526123f)) * _S4 / vec2<f32>(f32(w_1), f32(h_1));
    var offsetB_0 : vec2<f32> = vec2<f32>(sin(_Display_CSCB.iTime_0 * 1.75600004196166992f), cos(_Display_CSCB.iTime_0 * 1.38100004196166992f)) * _S4 / vec2<f32>(f32(w_1), f32(h_1));
    var ret_0 : vec3<f32>;
    var _S5 : vec2<f32> = uv_1 + vec2<f32>(sin(_Display_CSCB.iTime_0 * 0.60000002384185791f + uv_1.x * 5.12400007247924805f) * 0.02999999932944775f + _Display_CSCB.iTime_0 * 0.05999999865889549f, sin(_Display_CSCB.iTime_0 * 0.69999998807907104f + uv_1.y * 3.16499996185302734f) * 0.05000000074505806f + _Display_CSCB.iTime_0 * 0.03999999910593033f);
    ret_0[i32(0)] = (textureSampleLevel((_loadedTexture_0), (texSampler), (_S5), (0.0f))).x;
    ret_0[i32(1)] = (textureSampleLevel((_loadedTexture_0), (texSampler), (_S5 + offsetG_0), (0.0f))).x;
    ret_0[i32(2)] = (textureSampleLevel((_loadedTexture_0), (texSampler), (_S5 + offsetB_0), (0.0f))).z;
    return ret_0;
}

fn SDFCircle_0( coords_0 : vec2<f32>,  circle_0 : vec3<f32>) -> f32
{
    var _S6 : vec2<f32> = coords_0 - circle_0.xy;
    var _S7 : f32 = _S6.x;
    var _S8 : f32 = _S6.y;
    var _S9 : f32 = circle_0.z;
    return abs(_S7 * _S7 + _S8 * _S8) / length(vec2<f32>(_S9 * _S7, _S9 * _S8));
}

fn BombColor_0( cellFract_1 : vec2<f32>) -> vec3<f32>
{
    return vec3<f32>(clamp(1.0f - smoothstep(1.0f, 1.5f, SDFCircle_0(cellFract_1, vec3<f32>(0.5f, 0.5f, 0.15000000596046448f))) + (1.0f - smoothstep(1.0f, 1.5f, SDFCircle_0(cellFract_1, vec3<f32>(0.30000001192092896f, 0.30000001192092896f, 0.10000000149011612f)))) + (1.0f - smoothstep(1.0f, 1.5f, SDFCircle_0(cellFract_1, vec3<f32>(0.30000001192092896f, 0.69999998807907104f, 0.10000000149011612f)))) + (1.0f - smoothstep(1.0f, 1.5f, SDFCircle_0(cellFract_1, vec3<f32>(0.69999998807907104f, 0.30000001192092896f, 0.10000000149011612f)))) + (1.0f - smoothstep(1.0f, 1.5f, SDFCircle_0(cellFract_1, vec3<f32>(0.69999998807907104f, 0.69999998807907104f, 0.10000000149011612f)))), 0.0f, 1.0f));
}

fn HiddenTileColor_0( cell_1 : vec2<f32>,  cellFract_2 : vec2<f32>,  mouseCell_0 : vec2<f32>) -> vec3<f32>
{
    var _S10 : f32 = cellFract_2.x;
    var _S11 : f32 = cellFract_2.y;
    var addMedium_0 : f32 = clamp((1.0f - step(_S10, 0.10000000149011612f)) * (1.0f - step(_S11, 0.10000000149011612f)), 0.0f, 1.0f);
    var ret_1 : vec3<f32> = vec3<f32>((0.40000000596046448f + 0.40000000596046448f * addMedium_0 + 0.19999998807907104f * (clamp(step(1.0f - _S10, 0.10000000149011612f) + step(1.0f - _S11, 0.10000000149011612f), 0.0f, 1.0f) * addMedium_0)));
    var _S12 : bool;
    if((cell_1.x) == (mouseCell_0.x))
    {
        _S12 = (cell_1.y) == (mouseCell_0.y);
    }
    else
    {
        _S12 = false;
    }
    if(_S12)
    {
        ret_1[i32(2)] = 0.0f;
    }
    return ret_1;
}

fn mod_0( x_0 : f32,  y_0 : f32) -> f32
{
    return x_0 - y_0 * floor(x_0 / y_0);
}

fn CountTileColor_0( cellFract_3 : vec2<f32>,  count_0 : f32) -> vec3<f32>
{
    var color_0 : f32;
    if((mod_0(count_0, 2.0f)) == 1.0f)
    {
        color_0 = 0.60000002384185791f * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.5f, 0.5f, 0.10000000149011612f)));
    }
    else
    {
        color_0 = 0.60000002384185791f;
    }
    if(count_0 >= 2.0f)
    {
        color_0 = color_0 * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.25f, 0.25f, 0.10000000149011612f))) * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.75f, 0.75f, 0.10000000149011612f)));
    }
    if(count_0 >= 4.0f)
    {
        color_0 = color_0 * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.25f, 0.75f, 0.10000000149011612f))) * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.75f, 0.25f, 0.10000000149011612f)));
    }
    if(count_0 >= 6.0f)
    {
        color_0 = color_0 * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.25f, 0.5f, 0.10000000149011612f))) * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.75f, 0.5f, 0.10000000149011612f)));
    }
    if(count_0 >= 8.0f)
    {
        color_0 = color_0 * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.5f, 0.25f, 0.10000000149011612f))) * smoothstep(0.69999998807907104f, 1.5f, SDFCircle_0(cellFract_3, vec3<f32>(0.5f, 0.75f, 0.10000000149011612f)));
    }
    var _S13 : f32 = cellFract_3.x;
    var _S14 : bool;
    if(_S13 < 0.02500000037252903f)
    {
        _S14 = true;
    }
    else
    {
        _S14 = (cellFract_3.y) < 0.02500000037252903f;
    }
    if(_S14)
    {
        _S14 = true;
    }
    else
    {
        _S14 = (1.0f - _S13) < 0.02500000037252903f;
    }
    if(_S14)
    {
        _S14 = true;
    }
    else
    {
        _S14 = (1.0f - cellFract_3.y) < 0.02500000037252903f;
    }
    if(_S14)
    {
        color_0 = 0.40000000596046448f;
    }
    return vec3<f32>(color_0);
}

fn FlagColor_0( cell_2 : vec2<f32>,  cellFract_4 : vec2<f32>,  mouseCell_1 : vec2<f32>) -> vec3<f32>
{
    var _S15 : vec3<f32> = HiddenTileColor_0(cell_2, cellFract_4, mouseCell_1);
    var pixel_0 : vec3<f32> = _S15;
    var _S16 : vec2<f32> = _S15.xz * vec2<f32>(smoothstep(1.0f, 1.5f, SDFCircle_0(cellFract_4, vec3<f32>(0.5f, 0.5f, 0.20000000298023224f))));
    pixel_0.x = _S16.x;
    pixel_0.z = _S16.y;
    return pixel_0;
}

fn TileColor_0( cell_3 : vec2<f32>,  cellFract_5 : vec2<f32>,  cellData_0 : vec4<f32>,  mouseCell_2 : vec2<f32>,  gameOver_0 : bool) -> vec3<f32>
{
    if(gameOver_0)
    {
        if((cellData_0.z) == 1.0f)
        {
            return BombColor_0(cellFract_5);
        }
        else
        {
            if((cellData_0.x) == 0.0f)
            {
                return HiddenTileColor_0(cell_3, cellFract_5, mouseCell_2);
            }
            else
            {
                return CountTileColor_0(cellFract_5, floor(cellData_0.y * 255.0f));
            }
        }
    }
    else
    {
        if((cellData_0.x) == 0.0f)
        {
            if((cellData_0.w) == 1.0f)
            {
                return FlagColor_0(cell_3, cellFract_5, mouseCell_2);
            }
            else
            {
                return HiddenTileColor_0(cell_3, cellFract_5, mouseCell_2);
            }
        }
        else
        {
            if((cellData_0.z) == 1.0f)
            {
                return BombColor_0(cellFract_5);
            }
            else
            {
                return CountTileColor_0(cellFract_5, floor(cellData_0.y * 255.0f));
            }
        }
    }
}

@compute
@workgroup_size(8, 8, 1)
fn displaycs(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var state_0 : vec4<f32> = gameState[u32(_Display_CSCB.iFrame_0 + i32(1)) % u32(2)];
    var _S17 : vec2<u32> = DTid_0.xy;
    var uv_2 : vec2<f32>;
    var cell_4 : vec2<f32>;
    var cellFract_6 : vec2<f32>;
    PixelToCell_0(vec2<f32>(_S17), &(uv_2), &(cell_4), &(cellFract_6));
    var _S18 : bool;
    if((cell_4.x) < 0.0f)
    {
        _S18 = true;
    }
    else
    {
        _S18 = (cell_4.y) < 0.0f;
    }
    if(_S18)
    {
        _S18 = true;
    }
    else
    {
        _S18 = (cell_4.x) > 15.0f;
    }
    if(_S18)
    {
        _S18 = true;
    }
    else
    {
        _S18 = (cell_4.y) > 15.0f;
    }
    if(_S18)
    {
        textureStore((output), (_S17), (vec4<f32>(BackgroundPixel_0(uv_2), 1.0f)));
        return;
    }
    var mouseState_0 : vec4<f32> = _Display_CSCB.MouseState_0;
    var mouseUv_0 : vec2<f32>;
    var mouseCell_3 : vec2<f32>;
    var mouseCellFract_0 : vec2<f32>;
    PixelToCell_0(_Display_CSCB.MouseState_0.xy, &(mouseUv_0), &(mouseCell_3), &(mouseCellFract_0));
    var _S19 : f32;
    if((mouseState_0.z) > 0.0f)
    {
        _S19 = 1.0f;
    }
    else
    {
        _S19 = -1.0f;
    }
    var _S20 : vec2<f32> = mouseCell_3 * vec2<f32>(_S19);
    mouseCell_3 = _S20;
    var _S21 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(cell_4), u32(0)));
    var _S22 : f32 = state_0.x;
    var gameOver_1 : bool = _S22 > 0.20000000298023224f;
    var pixelColor_0 : vec3<f32> = TileColor_0(cell_4, cellFract_6, (textureLoad((gameBoard), ((_S21)).xy, ((_S21)).z)), _S20, gameOver_1);
    if(_S22 > 0.30000001192092896f)
    {
        const _S23 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
        pixelColor_0.x = _S23.x;
        pixelColor_0.z = _S23.y;
    }
    else
    {
        if(gameOver_1)
        {
            const _S24 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
            pixelColor_0.y = _S24.x;
            pixelColor_0.z = _S24.y;
        }
    }
    textureStore((output), (_S17), (vec4<f32>(pixelColor_0, 1.0f)));
    return;
}

`;

// -------------------- Private Members

// Texture GameBoard
texture_GameBoard = null;
texture_GameBoard_size = [0, 0, 0];
texture_GameBoard_format = "";
texture_GameBoard_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Buffer GameState
buffer_GameState = null;
buffer_GameState_count = 0;
buffer_GameState_stride = 0;
buffer_GameState_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Texture Gameplay_gameBoard_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Gameplay_gameBoard_ReadOnly = null;
texture_Gameplay_gameBoard_ReadOnly_size = [0, 0, 0];
texture_Gameplay_gameBoard_ReadOnly_format = "";
texture_Gameplay_gameBoard_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Display_output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Display_output_ReadOnly = null;
texture_Display_output_ReadOnly_size = [0, 0, 0];
texture_Display_output_ReadOnly_format = "";
texture_Display_output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _Gameplay_CSCB
constantBuffer__Gameplay_CSCB = null;
constantBuffer__Gameplay_CSCB_size = 48;
constantBuffer__Gameplay_CSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Gameplay
Hash_Compute_Gameplay = 0;
ShaderModule_Compute_Gameplay = null;
BindGroupLayout_Compute_Gameplay = null;
PipelineLayout_Compute_Gameplay = null;
Pipeline_Compute_Gameplay = null;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Constant buffer _Display_CSCB
constantBuffer__Display_CSCB = null;
constantBuffer__Display_CSCB_size = 32;
constantBuffer__Display_CSCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

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
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

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
variable_Reset = false;
variableDefault_Reset = false;
variableChanged_Reset = false;

// -------------------- Private Variables

variable_c_gridSize1D = 16.000000;
variableDefault_c_gridSize1D = 16.000000;
variableChanged_c_gridSize1D = false;
variable_c_gridSize2D = [ 16.000000, 16.000000 ];
variableDefault_c_gridSize2D = [ 16.000000, 16.000000 ];
variableChanged_c_gridSize2D = [ false, false ];
// -------------------- Structs

static StructOffsets__Gameplay_CSCB =
{
    MouseState_0: 0,
    MouseState_1: 4,
    MouseState_2: 8,
    MouseState_3: 12,
    MouseStateLastFrame_0: 16,
    MouseStateLastFrame_1: 20,
    MouseStateLastFrame_2: 24,
    MouseStateLastFrame_3: 28,
    Reset: 32,
    iFrame: 36,
    iTime: 40,
    _padding0: 44,
    _size: 48,
}

static StructOffsets__Display_CSCB =
{
    MouseState_0: 0,
    MouseState_1: 4,
    MouseState_2: 8,
    MouseState_3: 12,
    iFrame: 16,
    iTime: 20,
    _padding0: 24,
    _padding1: 28,
    _size: 32,
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
    // Handle (re)creation of texture GameBoard
    {
        const baseSize = [ this.variable_c_gridSize2D[0], this.variable_c_gridSize2D[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm";
        if (this.texture_GameBoard !== null && (this.texture_GameBoard_format != desiredFormat || this.texture_GameBoard_size[0] != desiredSize[0] || this.texture_GameBoard_size[1] != desiredSize[1] || this.texture_GameBoard_size[2] != desiredSize[2]))
        {
            this.texture_GameBoard.destroy();
            this.texture_GameBoard = null;
        }

        if (this.texture_GameBoard === null)
        {
            this.texture_GameBoard_size = desiredSize.slice();
            this.texture_GameBoard_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_GameBoard_format))
                viewFormats.push(this.texture_GameBoard_format);

            this.texture_GameBoard = device.createTexture({
                label: "texture DemofoxMinesweeper.GameBoard",
                size: this.texture_GameBoard_size,
                format: Shared.GetNonSRGBFormat(this.texture_GameBoard_format),
                usage: this.texture_GameBoard_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of buffer GameState
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 2) / 1) + 0;
        const desiredStride = 16;
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
                label: "buffer DemofoxMinesweeper.GameState",
                size: Shared.Align(16, this.buffer_GameState_count * this.buffer_GameState_stride),
                usage: this.buffer_GameState_usageFlags,
            });
        }
    }

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
                label: "texture DemofoxMinesweeper.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Gameplay_gameBoard_ReadOnly
    {
        const baseSize = this.texture_GameBoard_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_GameBoard_format;
        if (this.texture_Gameplay_gameBoard_ReadOnly !== null && (this.texture_Gameplay_gameBoard_ReadOnly_format != desiredFormat || this.texture_Gameplay_gameBoard_ReadOnly_size[0] != desiredSize[0] || this.texture_Gameplay_gameBoard_ReadOnly_size[1] != desiredSize[1] || this.texture_Gameplay_gameBoard_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Gameplay_gameBoard_ReadOnly.destroy();
            this.texture_Gameplay_gameBoard_ReadOnly = null;
        }

        if (this.texture_Gameplay_gameBoard_ReadOnly === null)
        {
            this.texture_Gameplay_gameBoard_ReadOnly_size = desiredSize.slice();
            this.texture_Gameplay_gameBoard_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Gameplay_gameBoard_ReadOnly_format))
                viewFormats.push(this.texture_Gameplay_gameBoard_ReadOnly_format);

            this.texture_Gameplay_gameBoard_ReadOnly = device.createTexture({
                label: "texture DemofoxMinesweeper.Gameplay_gameBoard_ReadOnly",
                size: this.texture_Gameplay_gameBoard_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Gameplay_gameBoard_ReadOnly_format),
                usage: this.texture_Gameplay_gameBoard_ReadOnly_usageFlags,
                viewFormats: viewFormats,
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
                label: "texture DemofoxMinesweeper.Display_output_ReadOnly",
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
                // gameBoard
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_GameBoard_format), viewDimension: "2d" }
            },
            {
                // gameState
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // output
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Output_format).sampleType }
            },
            {
                // gameBoardReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Gameplay_gameBoard_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _Gameplay_CSCB
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Gameplay === null || newHash !== this.Hash_Compute_Gameplay)
        {
            this.Hash_Compute_Gameplay = newHash;

            let shaderCode = class_DemofoxMinesweeper.ShaderCode_Gameplay_Gameplay_CS;
            shaderCode = shaderCode.replace("/*(gameBoard_format)*/", Shared.GetNonSRGBFormat(this.texture_GameBoard_format));
            shaderCode = shaderCode.replace("/*(gameBoardReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Gameplay_gameBoard_ReadOnly_format));

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
                        entryPoint: "gameplaycs",
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
                        entryPoint: "gameplaycs",
                    }
                }).then( handle => { this.Pipeline_Compute_Gameplay = handle; this.loadingPromises.delete("Gameplay"); } );
            }
        }
    }

    // Load texture _loadedTexture_0 from "flagstone-rubble-7669-in-architextures.png"
    if (this.texture__loadedTexture_0 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/flagstone-rubble-7669-in-architextures.png", this.texture__loadedTexture_0_usageFlags, "2d");
        this.texture__loadedTexture_0 = loadedTex.texture;
        this.texture__loadedTexture_0_size = loadedTex.size;
        this.texture__loadedTexture_0_format = "rgba8unorm";
        this.texture__loadedTexture_0_usageFlags = loadedTex.usageFlags;
    }
    // (Re)create compute shader Display
    {
        const bindGroupEntries =
        [
            {
                // gameBoard
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_GameBoard_format).sampleType }
            },
            {
                // gameState
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // output
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Output_format), viewDimension: "2d" }
            },
            {
                // outputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Display_output_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _loadedTexture_0
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
            {
                // _Display_CSCB
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

        if (this.ShaderModule_Compute_Display === null || newHash !== this.Hash_Compute_Display)
        {
            this.Hash_Compute_Display = newHash;

            let shaderCode = class_DemofoxMinesweeper.ShaderCode_Display_Display_CS;
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
                        entryPoint: "displaycs",
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
                        entryPoint: "displaycs",
                    }
                }).then( handle => { this.Pipeline_Compute_Display = handle; this.loadingPromises.delete("Display"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("DemofoxMinesweeper.GameBoard");

    encoder.popDebugGroup(); // "DemofoxMinesweeper.GameBoard"

    encoder.pushDebugGroup("DemofoxMinesweeper.GameState");

    encoder.popDebugGroup(); // "DemofoxMinesweeper.GameState"

    encoder.pushDebugGroup("DemofoxMinesweeper.Output");

    encoder.popDebugGroup(); // "DemofoxMinesweeper.Output"

    encoder.pushDebugGroup("DemofoxMinesweeper.Gameplay_gameBoard_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxMinesweeper.Gameplay_gameBoard_ReadOnly"

    encoder.pushDebugGroup("DemofoxMinesweeper.Copy_Gameplay_gameBoard");

    // Copy texture GameBoard to texture Gameplay_gameBoard_ReadOnly
    {
        const numMips = Math.min(this.texture_GameBoard.mipLevelCount, this.texture_Gameplay_gameBoard_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_GameBoard.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_GameBoard.height >> mipIndex, 1);
            let mipDepth = this.texture_GameBoard.depthOrArrayLayers;

            if (this.texture_GameBoard.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_GameBoard, mipLevel: mipIndex },
                { texture: this.texture_Gameplay_gameBoard_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "DemofoxMinesweeper.Copy_Gameplay_gameBoard"

    encoder.pushDebugGroup("DemofoxMinesweeper.Display_output_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxMinesweeper.Display_output_ReadOnly"

    encoder.pushDebugGroup("DemofoxMinesweeper._Gameplay_CSCB");

    // Create constant buffer _Gameplay_CSCB
    if (this.constantBuffer__Gameplay_CSCB === null)
    {
        this.constantBuffer__Gameplay_CSCB = device.createBuffer({
            label: "DemofoxMinesweeper._Gameplay_CSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__Gameplay_CSCB._size),
            usage: this.constantBuffer__Gameplay_CSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _Gameplay_CSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Gameplay_CSCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseState_0, this.variable_MouseState[0], true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseState_1, this.variable_MouseState[1], true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseState_2, this.variable_MouseState[2], true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseState_3, this.variable_MouseState[3], true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseStateLastFrame_0, this.variable_MouseStateLastFrame[0], true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseStateLastFrame_1, this.variable_MouseStateLastFrame[1], true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseStateLastFrame_2, this.variable_MouseStateLastFrame[2], true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.MouseStateLastFrame_3, this.variable_MouseStateLastFrame[3], true);
        view.setUint32(this.constructor.StructOffsets__Gameplay_CSCB.Reset, (this.variable_Reset === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__Gameplay_CSCB.iFrame, this.variable_iFrame, true);
        view.setFloat32(this.constructor.StructOffsets__Gameplay_CSCB.iTime, this.variable_iTime, true);
        device.queue.writeBuffer(this.constantBuffer__Gameplay_CSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxMinesweeper._Gameplay_CSCB"

    encoder.pushDebugGroup("DemofoxMinesweeper.Gameplay");

    // Run compute shader Gameplay
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Gameplay",
            layout: this.BindGroupLayout_Compute_Gameplay,
            entries: [
                {
                    // gameBoard
                    binding: 0,
                    resource: this.texture_GameBoard.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // gameState
                    binding: 1,
                    resource: { buffer: this.buffer_GameState }
                },
                {
                    // output
                    binding: 2,
                    resource: this.texture_Output.createView({ dimension: "2d", format: this.texture_Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // gameBoardReadOnly
                    binding: 3,
                    resource: this.texture_Gameplay_gameBoard_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _Gameplay_CSCB
                    binding: 4,
                    resource: { buffer: this.constantBuffer__Gameplay_CSCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_GameBoard_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
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

    encoder.popDebugGroup(); // "DemofoxMinesweeper.Gameplay"

    encoder.pushDebugGroup("DemofoxMinesweeper.Copy_Display_output");

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

    encoder.popDebugGroup(); // "DemofoxMinesweeper.Copy_Display_output"

    encoder.pushDebugGroup("DemofoxMinesweeper._loadedTexture_0");

    encoder.popDebugGroup(); // "DemofoxMinesweeper._loadedTexture_0"

    encoder.pushDebugGroup("DemofoxMinesweeper._Display_CSCB");

    // Create constant buffer _Display_CSCB
    if (this.constantBuffer__Display_CSCB === null)
    {
        this.constantBuffer__Display_CSCB = device.createBuffer({
            label: "DemofoxMinesweeper._Display_CSCB",
            size: Shared.Align(16, this.constructor.StructOffsets__Display_CSCB._size),
            usage: this.constantBuffer__Display_CSCB_usageFlags,
        });
    }

    // Upload values to constant buffer _Display_CSCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__Display_CSCB._size));
        const view = new DataView(bufferCPU);
        view.setFloat32(this.constructor.StructOffsets__Display_CSCB.MouseState_0, this.variable_MouseState[0], true);
        view.setFloat32(this.constructor.StructOffsets__Display_CSCB.MouseState_1, this.variable_MouseState[1], true);
        view.setFloat32(this.constructor.StructOffsets__Display_CSCB.MouseState_2, this.variable_MouseState[2], true);
        view.setFloat32(this.constructor.StructOffsets__Display_CSCB.MouseState_3, this.variable_MouseState[3], true);
        view.setInt32(this.constructor.StructOffsets__Display_CSCB.iFrame, this.variable_iFrame, true);
        view.setFloat32(this.constructor.StructOffsets__Display_CSCB.iTime, this.variable_iTime, true);
        device.queue.writeBuffer(this.constantBuffer__Display_CSCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxMinesweeper._Display_CSCB"

    encoder.pushDebugGroup("DemofoxMinesweeper.Display");

    // Run compute shader Display
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Display",
            layout: this.BindGroupLayout_Compute_Display,
            entries: [
                {
                    // gameBoard
                    binding: 0,
                    resource: this.texture_GameBoard.createView({ dimension: "2d", format: this.texture_GameBoard_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // gameState
                    binding: 1,
                    resource: { buffer: this.buffer_GameState }
                },
                {
                    // output
                    binding: 2,
                    resource: this.texture_Output.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // outputReadOnly
                    binding: 3,
                    resource: this.texture_Display_output_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 4,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _Display_CSCB
                    binding: 5,
                    resource: { buffer: this.constantBuffer__Display_CSCB }
                },
                {
                    // texSampler
                    binding: 6,
                    resource: device.createSampler({
                        label: "Sampler texSampler",
                        addressModeU: "repeat",
                        addressModeV: "repeat",
                        addressModeW: "repeat",
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

        if (this.Pipeline_Compute_Display !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Display);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxMinesweeper.Display"

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

var DemofoxMinesweeper = new class_DemofoxMinesweeper;

export default DemofoxMinesweeper;
