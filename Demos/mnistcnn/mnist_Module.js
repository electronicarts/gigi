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

class class_mnist
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Draw", node "Draw"
static ShaderCode_Draw_Draw = `
struct Struct_DrawExtents_std430_0
{
    @align(8) MinX_0 : u32,
    @align(4) MaxX_0 : u32,
    @align(8) MinY_0 : u32,
    @align(4) MaxY_0 : u32,
    @align(8) PixelCount_0 : u32,
    @align(4) _padding0_0 : f32,
    @align(8) PixelLocationSum_0 : vec2<u32>,
};

@binding(1) @group(0) var<storage, read_write> DrawExtents : array<Struct_DrawExtents_std430_0>;

struct Struct_DrawCB_std140_0
{
    @align(16) Clear_0 : u32,
    @align(4) _padding0_1 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
    @align(16) MouseState_0 : vec4<f32>,
    @align(16) MouseStateLastFrame_0 : vec4<f32>,
    @align(16) PenSize_0 : f32,
    @align(4) UseImportedImage_0 : u32,
    @align(8) iFrame_0 : i32,
    @align(4) _padding3_0 : f32,
};

@binding(3) @group(0) var<uniform> _DrawCB : Struct_DrawCB_std140_0;
@binding(0) @group(0) var Canvas : texture_storage_2d</*(Canvas_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn Draw(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : bool;
    if((DTid_0.x) == u32(0))
    {
        _S1 = (DTid_0.y) == u32(0);
    }
    else
    {
        _S1 = false;
    }
    if(_S1)
    {
        DrawExtents[i32(0)].MinX_0 = u32(256);
        DrawExtents[i32(0)].MinY_0 = u32(256);
        DrawExtents[i32(0)].MaxX_0 = u32(0);
        DrawExtents[i32(0)].MaxY_0 = u32(0);
        DrawExtents[i32(0)].PixelCount_0 = u32(0);
        DrawExtents[i32(0)].PixelLocationSum_0 = vec2<u32>(u32(0), u32(0));
    }
    if((_DrawCB.Clear_0) != u32(0))
    {
        _S1 = true;
    }
    else
    {
        _S1 = (_DrawCB.iFrame_0) == i32(0);
    }
    if(_S1)
    {
        textureStore((Canvas), (DTid_0.xy), vec4<f32>((0.0f), 0, 0, 1));
        return;
    }
    if(bool(_DrawCB.UseImportedImage_0))
    {
        return;
    }
    var _S2 : vec4<f32> = _DrawCB.MouseStateLastFrame_0;
    var mouseLastFrame_0 : vec4<f32> = _DrawCB.MouseStateLastFrame_0;
    var _S3 : vec4<f32> = _DrawCB.MouseState_0;
    var mouse_0 : vec4<f32> = _DrawCB.MouseState_0;
    var _S4 : vec2<f32> = vec2<f32>(30.0f);
    var _S5 : vec2<f32> = _S2.xy - _S4;
    mouseLastFrame_0.x = _S5.x;
    mouseLastFrame_0.y = _S5.y;
    var _S6 : vec2<f32> = _S3.xy - _S4;
    mouse_0.x = _S6.x;
    mouse_0.y = _S6.y;
    if((mouseLastFrame_0.z) != 0.0f)
    {
        _S1 = (mouse_0.z) != 0.0f;
    }
    else
    {
        _S1 = false;
    }
    if(_S1)
    {
        _S1 = true;
    }
    else
    {
        if((mouseLastFrame_0.w) != 0.0f)
        {
            _S1 = (mouse_0.w) != 0.0f;
        }
        else
        {
            _S1 = false;
        }
    }
    if(_S1)
    {
        if((mouseLastFrame_0.x) != (mouse_0.x))
        {
            _S1 = true;
        }
        else
        {
            _S1 = (mouseLastFrame_0.y) != (mouse_0.y);
        }
    }
    else
    {
        _S1 = false;
    }
    if(_S1)
    {
        var A_0 : vec2<f32> = mouseLastFrame_0.xy;
        var _S7 : vec2<f32> = mouse_0.xy - A_0;
        var AB_0 : vec2<f32> = normalize(_S7);
        var _S8 : vec2<u32> = DTid_0.xy;
        var _S9 : vec2<f32> = vec2<f32>(_S8);
        var closestPointToPixel_0 : vec2<f32> = _S9 - (A_0 + AB_0 * vec2<f32>(clamp(dot(AB_0, _S9 - A_0), 0.0f, length(_S7))));
        if((dot(closestPointToPixel_0, closestPointToPixel_0)) < (_DrawCB.PenSize_0 * _DrawCB.PenSize_0))
        {
            textureStore((Canvas), (_S8), vec4<f32>((mouse_0.z), 0, 0, 1));
        }
    }
    return;
}

`;

// Shader code for Compute shader "Shrink", node "Shrink"
static ShaderCode_Shrink_Shrink = `
struct Struct_ShrinkCB_std140_0
{
    @align(16) NormalizeDrawing_0 : u32,
    @align(4) UseImportedImage_0 : u32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(5) @group(0) var<uniform> _ShrinkCB : Struct_ShrinkCB_std140_0;
@binding(3) @group(0) var ImportedImage : texture_2d<f32>;

@binding(2) @group(0) var NNInput : texture_storage_2d</*(NNInput_format)*/, write>;

struct Struct_DrawExtents_std430_0
{
    @align(8) MinX_0 : u32,
    @align(4) MaxX_0 : u32,
    @align(8) MinY_0 : u32,
    @align(4) MaxY_0 : u32,
    @align(8) PixelCount_0 : u32,
    @align(4) _padding0_1 : f32,
    @align(8) PixelLocationSum_0 : vec2<u32>,
};

@binding(1) @group(0) var<storage, read> DrawExtents : array<Struct_DrawExtents_std430_0>;

@binding(0) @group(0) var Canvas : texture_2d<f32>;

fn ShrinkNormalize_0( pixelPos_0 : vec2<u32>)
{
    var _S1 : u32 = DrawExtents[i32(0)].MinY_0;
    var drawMin_0 : vec2<u32> = vec2<u32>(DrawExtents[i32(0)].MinX_0, DrawExtents[i32(0)].MinY_0);
    var _S2 : u32 = DrawExtents[i32(0)].MaxY_0;
    var drawMax_0 : vec2<u32> = vec2<u32>(DrawExtents[i32(0)].MaxX_0, DrawExtents[i32(0)].MaxY_0);
    var _S3 : bool;
    if((DrawExtents[i32(0)].MaxX_0) < (DrawExtents[i32(0)].MinX_0))
    {
        _S3 = true;
    }
    else
    {
        _S3 = _S2 < _S1;
    }
    if(_S3)
    {
        textureStore((NNInput), (pixelPos_0), vec4<f32>((0.0f), 0, 0, 1));
        return;
    }
    var drawSize_0 : vec2<u32> = drawMax_0 - drawMin_0;
    var normalizedImageSize_0 : vec2<u32> = vec2<u32>(u32(20), u32(20));
    var offset_0 : vec2<i32> = vec2<i32>(i32(4), i32(4));
    var _S4 : u32 = drawSize_0.x;
    var _S5 : u32 = drawSize_0.y;
    var drawnAspectRatio_0 : f32 = f32(_S4) / f32(_S5);
    if(_S4 > _S5)
    {
        normalizedImageSize_0[i32(1)] = u32(f32(normalizedImageSize_0.y) / drawnAspectRatio_0);
        offset_0[i32(1)] = offset_0[i32(1)] + i32((u32(20) - normalizedImageSize_0.y) / u32(2));
    }
    else
    {
        normalizedImageSize_0[i32(0)] = u32(f32(normalizedImageSize_0.x) * drawnAspectRatio_0);
        offset_0[i32(0)] = offset_0[i32(0)] + i32((u32(20) - normalizedImageSize_0.x) / u32(2));
    }
    var _S6 : vec2<f32> = vec2<f32>(vec2<i32>(drawMin_0));
    var _S7 : vec2<i32> = vec2<i32>(pixelPos_0);
    var _S8 : vec2<f32> = vec2<f32>(drawSize_0);
    var _S9 : vec2<u32> = vec2<u32>(_S6 + vec2<f32>(_S7 - offset_0) * _S8 / vec2<f32>(normalizedImageSize_0));
    var _S10 : vec2<u32> = vec2<u32>(_S6 + vec2<f32>(_S7 - offset_0 + vec2<i32>(i32(1), i32(1))) * _S8 / vec2<f32>(normalizedImageSize_0));
    var _S11 : u32 = _S10.x;
    var _S12 : u32 = _S9.x;
    var _S13 : u32 = _S10.y;
    var _S14 : u32 = _S9.y;
    var count_0 : f32 = f32((_S11 - _S12) * (_S13 - _S14));
    var iy_0 : u32 = _S14;
    var output_0 : f32 = 0.0f;
    for(;;)
    {
        if(iy_0 < _S13)
        {
        }
        else
        {
            break;
        }
        var ix_0 : u32 = _S12;
        for(;;)
        {
            if(ix_0 < _S11)
            {
            }
            else
            {
                break;
            }
            var _S15 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(ix_0, iy_0), u32(0)));
            var output_1 : f32 = output_0 + (textureLoad((Canvas), ((_S15)).xy, ((_S15)).z).x) / count_0;
            ix_0 = ix_0 + u32(1);
            output_0 = output_1;
        }
        iy_0 = iy_0 + u32(1);
    }
    textureStore((NNInput), (pixelPos_0), vec4<f32>((output_0), 0, 0, 1));
    return;
}

fn ShrinkFn_0( pixelPos_1 : vec2<u32>)
{
    const _S16 : vec2<f32> = vec2<f32>(vec2<u32>(u32(256), u32(256)));
    const _S17 : vec2<f32> = vec2<f32>(vec2<u32>(u32(28), u32(28)));
    var _S18 : vec2<u32> = vec2<u32>(vec2<f32>(pixelPos_1) * _S16 / _S17);
    var _S19 : vec2<u32> = vec2<u32>(vec2<f32>(pixelPos_1 + vec2<u32>(u32(1), u32(1))) * _S16 / _S17);
    var _S20 : u32 = _S19.x;
    var _S21 : u32 = _S18.x;
    var _S22 : u32 = _S19.y;
    var _S23 : u32 = _S18.y;
    var count_1 : f32 = f32((_S20 - _S21) * (_S22 - _S23));
    var iy_1 : u32 = _S23;
    var output_2 : f32 = 0.0f;
    for(;;)
    {
        if(iy_1 < _S22)
        {
        }
        else
        {
            break;
        }
        var ix_1 : u32 = _S21;
        for(;;)
        {
            if(ix_1 < _S20)
            {
            }
            else
            {
                break;
            }
            var _S24 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(ix_1, iy_1), u32(0)));
            var output_3 : f32 = output_2 + (textureLoad((Canvas), ((_S24)).xy, ((_S24)).z).x) / count_1;
            ix_1 = ix_1 + u32(1);
            output_2 = output_3;
        }
        iy_1 = iy_1 + u32(1);
    }
    textureStore((NNInput), (pixelPos_1), vec4<f32>((output_2), 0, 0, 1));
    return;
}

@compute
@workgroup_size(8, 8, 1)
fn Shrink(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    if(bool(_ShrinkCB.UseImportedImage_0))
    {
        var _S25 : vec2<u32> = DTid_0.xy;
        var _S26 : vec3<i32> = vec3<i32>(vec3<u32>(_S25, u32(0)));
        textureStore((NNInput), (_S25), vec4<f32>(((textureLoad((ImportedImage), ((_S26)).xy, ((_S26)).z).x)), 0, 0, 1));
    }
    else
    {
        if(bool(_ShrinkCB.NormalizeDrawing_0))
        {
            ShrinkNormalize_0(DTid_0.xy);
        }
        else
        {
            ShrinkFn_0(DTid_0.xy);
        }
    }
    return;
}

`;

// Shader code for Compute shader "Presentation", node "Presentation"
static ShaderCode_Presentation_Presentation = `
struct Struct_PresentationCB_std140_0
{
    @align(16) Conv1OutputScale_0 : i32,
    @align(4) Conv2OutputScale_0 : i32,
    @align(8) MaxPool1OutputScale_0 : i32,
    @align(4) MaxPool2OutputScale_0 : i32,
    @align(16) MouseState_0 : vec4<f32>,
    @align(16) PenSize_0 : f32,
    @align(4) UseImportedImage_0 : u32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(20) @group(0) var<uniform> _PresentationCB : Struct_PresentationCB_std140_0;
@binding(0) @group(0) var DrawCanvas : texture_2d<f32>;

@binding(1) @group(0) var NNInput : texture_2d<f32>;

@binding(8) @group(0) var PresentationCanvas : texture_storage_2d</*(PresentationCanvas_format)*/, write>;

@binding(6) @group(0) var<storage, read> MaxValues : array<u32>;

@binding(2) @group(0) var Conv1Output : texture_2d_array<f32>;

@binding(3) @group(0) var MaxPool1Output : texture_2d_array<f32>;

@binding(4) @group(0) var Conv2Output : texture_2d_array<f32>;

@binding(5) @group(0) var MaxPool2Output : texture_2d_array<f32>;

@binding(7) @group(0) var<storage, read> LinearOutput : array<f32>;

@binding(10) @group(0) var _loadedTexture_0 : texture_2d<f32>;

@binding(11) @group(0) var _loadedTexture_1 : texture_2d<f32>;

@binding(12) @group(0) var _loadedTexture_2 : texture_2d<f32>;

@binding(13) @group(0) var _loadedTexture_3 : texture_2d<f32>;

@binding(14) @group(0) var _loadedTexture_4 : texture_2d<f32>;

@binding(15) @group(0) var _loadedTexture_5 : texture_2d<f32>;

@binding(16) @group(0) var _loadedTexture_6 : texture_2d<f32>;

@binding(17) @group(0) var _loadedTexture_7 : texture_2d<f32>;

@binding(18) @group(0) var _loadedTexture_8 : texture_2d<f32>;

@binding(19) @group(0) var _loadedTexture_9 : texture_2d<f32>;

fn DecodeAtomicInt_0( i_0 : u32) -> f32
{
    if(((i_0 >> (u32(31)))) == u32(0))
    {
        return (bitcast<f32>((~i_0)));
    }
    else
    {
        return (bitcast<f32>(((i_0 & (u32(2147483647))))));
    }
}

@compute
@workgroup_size(8, 8, 1)
fn Presentation(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    const c_inputPanelPos_0 : vec2<i32> = vec2<i32>(i32(302), i32(30));
    const c_conv1OutputPos_0 : vec2<i32> = vec2<i32>(i32(346), i32(30));
    var c_conv1OutputScale_0 : i32 = _PresentationCB.Conv1OutputScale_0;
    var _S1 : i32 = i32(24) * _PresentationCB.Conv1OutputScale_0;
    var _S2 : i32 = _S1 + i32(3);
    var _S3 : i32 = i32(2) * _S2 - i32(3);
    var _S4 : i32 = i32(346) + _S1 + i32(6) + i32(10);
    var c_maxPool1OutputPos_0 : vec2<i32> = vec2<i32>(_S4, i32(30));
    var c_maxPool1OutputScale_0 : i32 = _PresentationCB.MaxPool1OutputScale_0;
    var _S5 : i32 = i32(12) * _PresentationCB.MaxPool1OutputScale_0;
    var _S6 : i32 = _S5 + i32(3);
    var _S7 : i32 = i32(2) * _S6 - i32(3);
    var _S8 : i32 = _S4 + _S5 + i32(6) + i32(10);
    var c_conv2OutputPos_0 : vec2<i32> = vec2<i32>(_S8, i32(30));
    var c_conv2OutputScale_0 : i32 = _PresentationCB.Conv2OutputScale_0;
    var _S9 : i32 = i32(10) * _PresentationCB.Conv2OutputScale_0;
    var _S10 : i32 = _S9 + i32(3);
    var _S11 : i32 = i32(4) * _S10 - i32(3);
    var _S12 : i32 = _S8 + _S9 + i32(6) + i32(10);
    var c_maxPool2OutputPos_0 : vec2<i32> = vec2<i32>(_S12, i32(30));
    var c_maxPool2OutputScale_0 : i32 = _PresentationCB.MaxPool2OutputScale_0;
    var _S13 : i32 = i32(5) * _PresentationCB.MaxPool2OutputScale_0;
    var _S14 : i32 = _S13 + i32(3);
    var _S15 : i32 = i32(4) * _S14 - i32(3);
    var _S16 : i32 = _S12 + _S13 + i32(6) + i32(10);
    var c_linearOutputPos_0 : vec2<i32> = vec2<i32>(_S16, i32(30));
    var _S17 : i32 = _S16 + i32(28) + i32(6);
    var c_linearOutputLabelsPos_0 : vec2<i32> = vec2<i32>(_S17, i32(30));
    var c_winnerLabelPos_0 : vec2<i32> = vec2<i32>(_S17 + i32(28) + i32(6), i32(30));
    const c_borderColor_0 : vec4<f32> = vec4<f32>(0.80000001192092896f, 0.80000001192092896f, 0.0f, 1.0f);
    const c_backgroundColor_0 : vec4<f32> = vec4<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f, 1.0f);
    var _S18 : vec2<u32> = DTid_0.xy;
    var _S19 : vec2<i32> = vec2<i32>(_S18);
    var relPos_0 : vec2<i32> = _S19 - vec2<i32>(i32(30), i32(30));
    var _S20 : i32 = relPos_0.x;
    var _S21 : bool;
    if(_S20 >= i32(0))
    {
        _S21 = (relPos_0.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S20 < i32(256);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_0.y) < i32(256);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var mouse_0 : vec4<f32> = _PresentationCB.MouseState_0;
        var color_0 : vec3<f32>;
        if(bool(u32(!bool(_PresentationCB.UseImportedImage_0))))
        {
            var _S22 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_0), u32(0)));
            color_0 = vec3<f32>(0.0f, (textureLoad((DrawCanvas), ((_S22)).xy, ((_S22)).z).x), 0.0f);
        }
        else
        {
            var _S23 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(vec2<i32>(vec2<f32>(relPos_0) * vec2<f32>(vec2<u32>(u32(28), u32(28))) / vec2<f32>(vec2<u32>(u32(256), u32(256))))), u32(0)));
            var _S24 : f32 = (textureLoad((NNInput), ((_S23)).xy, ((_S23)).z).x);
            color_0 = vec3<f32>(_S24, _S24, 0.0f);
        }
        if((length(mouse_0.xy - vec2<f32>(_S18))) < (_PresentationCB.PenSize_0))
        {
            color_0 = mix(color_0, vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(0.15000000596046448f, 0.15000000596046448f, 0.15000000596046448f));
        }
        textureStore((PresentationCanvas), (_S18), (vec4<f32>(color_0, 1.0f)));
        return;
    }
    if(_S20 >= i32(-3))
    {
        _S21 = (relPos_0.y) >= i32(-3);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S20 < i32(259);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_0.y) < i32(259);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
        return;
    }
    var relPos_1 : vec2<i32> = _S19 - c_inputPanelPos_0;
    var _S25 : i32 = relPos_1.x;
    if(_S25 >= i32(0))
    {
        _S21 = (relPos_1.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S25 < i32(28);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_1.y) < i32(28);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var _S26 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_1), u32(0)));
        textureStore((PresentationCanvas), (_S18), (vec4<f32>(vec3<f32>((textureLoad((NNInput), ((_S26)).xy, ((_S26)).z).x)), 1.0f)));
        return;
    }
    if(_S25 >= i32(-3))
    {
        _S21 = (relPos_1.y) >= i32(-3);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S25 < i32(31);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_1.y) < i32(31);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
        return;
    }
    var relPos_2 : vec2<i32> = _S19 - c_conv1OutputPos_0;
    var _S27 : i32 = relPos_2.x;
    if(_S27 >= i32(0))
    {
        _S21 = (relPos_2.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S27 < _S1;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_2.y) < _S3;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var _S28 : i32 = relPos_2.y;
        var _S29 : i32 = _S28 % _S2;
        if(_S29 >= _S1)
        {
            textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
            return;
        }
        var readPx_0 : vec3<u32> = vec3<u32>(vec2<u32>(relPos_2.xy), u32(0));
        var zValue_0 : i32 = _S28 / _S2;
        readPx_0[i32(2)] = u32(zValue_0);
        readPx_0[i32(1)] = readPx_0[i32(1)] - u32(_S2 * zValue_0);
        var _S30 : vec2<u32> = readPx_0.xy / vec2<u32>(u32(c_conv1OutputScale_0));
        readPx_0.x = _S30.x;
        readPx_0.y = _S30.y;
        var minValue_0 : f32 = DecodeAtomicInt_0(MaxValues[i32(0)]);
        var _S31 : vec4<i32> = vec4<i32>(vec4<u32>(readPx_0, u32(0)));
        textureStore((PresentationCanvas), (_S18), (vec4<f32>(vec3<f32>((((textureLoad((Conv1Output), ((_S31)).xy, i32(((_S31)).z), ((_S31)).w).x) - minValue_0) / (DecodeAtomicInt_0(MaxValues[i32(1)]) - minValue_0))), 1.0f)));
        return;
    }
    if(_S27 >= i32(-3))
    {
        _S21 = (relPos_2.y) >= i32(-3);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S27 < _S2;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_2.y) < (_S3 + i32(3));
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
        return;
    }
    var relPos_3 : vec2<i32> = _S19 - c_maxPool1OutputPos_0;
    var _S32 : i32 = relPos_3.x;
    if(_S32 >= i32(0))
    {
        _S21 = (relPos_3.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S32 < _S5;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_3.y) < _S7;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var _S33 : i32 = relPos_3.y;
        var _S34 : i32 = _S33 % _S6;
        if(_S34 >= _S5)
        {
            textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
            return;
        }
        var readPx_1 : vec3<u32> = vec3<u32>(vec2<u32>(relPos_3.xy), u32(0));
        var zValue_1 : i32 = _S33 / _S6;
        readPx_1[i32(2)] = u32(zValue_1);
        readPx_1[i32(1)] = readPx_1[i32(1)] - u32(_S6 * zValue_1);
        var _S35 : vec2<u32> = readPx_1.xy / vec2<u32>(u32(c_maxPool1OutputScale_0));
        readPx_1.x = _S35.x;
        readPx_1.y = _S35.y;
        var minValue_1 : f32 = DecodeAtomicInt_0(MaxValues[i32(2)]);
        var _S36 : vec4<i32> = vec4<i32>(vec4<u32>(readPx_1, u32(0)));
        textureStore((PresentationCanvas), (_S18), (vec4<f32>(vec3<f32>((((textureLoad((MaxPool1Output), ((_S36)).xy, i32(((_S36)).z), ((_S36)).w).x) - minValue_1) / (DecodeAtomicInt_0(MaxValues[i32(3)]) - minValue_1))), 1.0f)));
        return;
    }
    if(_S32 >= i32(-3))
    {
        _S21 = (relPos_3.y) >= i32(-3);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S32 < _S6;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_3.y) < (_S7 + i32(3));
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
        return;
    }
    var relPos_4 : vec2<i32> = _S19 - c_conv2OutputPos_0;
    var _S37 : i32 = relPos_4.x;
    if(_S37 >= i32(0))
    {
        _S21 = (relPos_4.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S37 < _S9;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_4.y) < _S11;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var _S38 : i32 = relPos_4.y;
        var _S39 : i32 = _S38 % _S10;
        if(_S39 >= _S9)
        {
            textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
            return;
        }
        var readPx_2 : vec3<u32> = vec3<u32>(vec2<u32>(relPos_4.xy), u32(0));
        var zValue_2 : i32 = _S38 / _S10;
        readPx_2[i32(2)] = u32(zValue_2);
        readPx_2[i32(1)] = readPx_2[i32(1)] - u32(_S10 * zValue_2);
        var _S40 : vec2<u32> = readPx_2.xy / vec2<u32>(u32(c_conv2OutputScale_0));
        readPx_2.x = _S40.x;
        readPx_2.y = _S40.y;
        var minValue_2 : f32 = DecodeAtomicInt_0(MaxValues[i32(4)]);
        var _S41 : vec4<i32> = vec4<i32>(vec4<u32>(readPx_2, u32(0)));
        textureStore((PresentationCanvas), (_S18), (vec4<f32>(vec3<f32>((((textureLoad((Conv2Output), ((_S41)).xy, i32(((_S41)).z), ((_S41)).w).x) - minValue_2) / (DecodeAtomicInt_0(MaxValues[i32(5)]) - minValue_2))), 1.0f)));
        return;
    }
    if(_S37 >= i32(-3))
    {
        _S21 = (relPos_4.y) >= i32(-3);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S37 < _S10;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_4.y) < (_S11 + i32(3));
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
        return;
    }
    var relPos_5 : vec2<i32> = _S19 - c_maxPool2OutputPos_0;
    var _S42 : i32 = relPos_5.x;
    if(_S42 >= i32(0))
    {
        _S21 = (relPos_5.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S42 < _S13;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_5.y) < _S15;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var _S43 : i32 = relPos_5.y;
        var _S44 : i32 = _S43 % _S14;
        if(_S44 >= _S13)
        {
            textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
            return;
        }
        var readPx_3 : vec3<u32> = vec3<u32>(vec2<u32>(relPos_5.xy), u32(0));
        var zValue_3 : i32 = _S43 / _S14;
        readPx_3[i32(2)] = u32(zValue_3);
        readPx_3[i32(1)] = readPx_3[i32(1)] - u32(_S14 * zValue_3);
        var _S45 : vec2<u32> = readPx_3.xy / vec2<u32>(u32(c_maxPool2OutputScale_0));
        readPx_3.x = _S45.x;
        readPx_3.y = _S45.y;
        var minValue_3 : f32 = DecodeAtomicInt_0(MaxValues[i32(6)]);
        var _S46 : vec4<i32> = vec4<i32>(vec4<u32>(readPx_3, u32(0)));
        textureStore((PresentationCanvas), (_S18), (vec4<f32>(vec3<f32>((((textureLoad((MaxPool2Output), ((_S46)).xy, i32(((_S46)).z), ((_S46)).w).x) - minValue_3) / (DecodeAtomicInt_0(MaxValues[i32(7)]) - minValue_3))), 1.0f)));
        return;
    }
    if(_S42 >= i32(-3))
    {
        _S21 = (relPos_5.y) >= i32(-3);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S42 < _S14;
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_5.y) < (_S15 + i32(3));
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
        return;
    }
    var _S47 : vec2<i32> = _S19 - c_linearOutputPos_0;
    var relPos_6 : vec2<i32> = _S47;
    if((_S47.x) >= i32(0))
    {
        _S21 = (relPos_6.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_6.x) < i32(28);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_6.y) < i32(307);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        if((relPos_6.y % i32(31)) >= i32(28))
        {
            textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
            return;
        }
        relPos_6[i32(1)] = relPos_6[i32(1)] / i32(31);
        var minValue_4 : f32 = DecodeAtomicInt_0(MaxValues[i32(8)]);
        textureStore((PresentationCanvas), (_S18), (vec4<f32>(vec3<f32>(((LinearOutput[relPos_6.y] - minValue_4) / (DecodeAtomicInt_0(MaxValues[i32(9)]) - minValue_4))), 1.0f)));
        return;
    }
    if((relPos_6.x) >= i32(-3))
    {
        _S21 = (relPos_6.y) >= i32(-3);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_6.x) < i32(31);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_6.y) < i32(310);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        textureStore((PresentationCanvas), (_S18), (c_borderColor_0));
        return;
    }
    var _S48 : vec2<i32> = _S19 - c_linearOutputLabelsPos_0;
    var relPos_7 : vec2<i32> = _S48;
    if((_S48.x) >= i32(0))
    {
        _S21 = (relPos_7.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_7.x) < i32(28);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_7.y) < i32(307);
    }
    else
    {
        _S21 = false;
    }
    var maxValue_0 : f32;
    if(_S21)
    {
        if((relPos_7.y % i32(31)) < i32(28))
        {
            var index_0 : i32 = relPos_7.y / i32(31);
            relPos_7[i32(1)] = relPos_7.y % i32(31);
            switch(index_0)
            {
            case i32(0), :
                {
                    var _S49 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_0), ((_S49)).xy, ((_S49)).z).x);
                    break;
                }
            case i32(1), :
                {
                    var _S50 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_1), ((_S50)).xy, ((_S50)).z).x);
                    break;
                }
            case i32(2), :
                {
                    var _S51 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_2), ((_S51)).xy, ((_S51)).z).x);
                    break;
                }
            case i32(3), :
                {
                    var _S52 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_3), ((_S52)).xy, ((_S52)).z).x);
                    break;
                }
            case i32(4), :
                {
                    var _S53 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_4), ((_S53)).xy, ((_S53)).z).x);
                    break;
                }
            case i32(5), :
                {
                    var _S54 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_5), ((_S54)).xy, ((_S54)).z).x);
                    break;
                }
            case i32(6), :
                {
                    var _S55 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_6), ((_S55)).xy, ((_S55)).z).x);
                    break;
                }
            case i32(7), :
                {
                    var _S56 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_7), ((_S56)).xy, ((_S56)).z).x);
                    break;
                }
            case i32(8), :
                {
                    var _S57 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_8), ((_S57)).xy, ((_S57)).z).x);
                    break;
                }
            case i32(9), :
                {
                    var _S58 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_7), u32(0)));
                    maxValue_0 = (textureLoad((_loadedTexture_9), ((_S58)).xy, ((_S58)).z).x);
                    break;
                }
            case default, :
                {
                    maxValue_0 = 0.0f;
                    break;
                }
            }
            if(maxValue_0 > 0.0f)
            {
                var minValue_5 : f32 = DecodeAtomicInt_0(MaxValues[i32(8)]);
                textureStore((PresentationCanvas), (_S18), (vec4<f32>(mix(vec3<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f), mix(vec3<f32>(0.40000000596046448f, 0.0f, 0.0f), vec3<f32>(1.0f, 1.0f, 0.0f), vec3<f32>(((LinearOutput[index_0] - minValue_5) / (DecodeAtomicInt_0(MaxValues[i32(9)]) - minValue_5)))), vec3<f32>(maxValue_0)), 1.0f)));
                return;
            }
        }
    }
    var relPos_8 : vec2<i32> = _S19 - c_winnerLabelPos_0;
    var _S59 : i32 = relPos_8.x;
    if(_S59 >= i32(0))
    {
        _S21 = (relPos_8.y) >= i32(0);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = _S59 < i32(28);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        _S21 = (relPos_8.y) < i32(307);
    }
    else
    {
        _S21 = false;
    }
    if(_S21)
    {
        var _S60 : i32 = relPos_8.y;
        if((_S60 % i32(31)) < i32(28))
        {
            maxValue_0 = 0.0f;
            var i_1 : i32 = i32(0);
            for(;;)
            {
                if(i_1 < i32(10))
                {
                }
                else
                {
                    break;
                }
                var iValue_0 : f32 = LinearOutput[i_1];
                if(iValue_0 > maxValue_0)
                {
                    maxValue_0 = iValue_0;
                }
                i_1 = i_1 + i32(1);
            }
            if(LinearOutput[_S60 / i32(31)] >= maxValue_0)
            {
                textureStore((PresentationCanvas), (_S18), (vec4<f32>(0.0f, 1.0f, 0.0f, 1.0f)));
                return;
            }
        }
    }
    textureStore((PresentationCanvas), (_S18), (c_backgroundColor_0));
    return;
}

`;

// Shader code for Compute shader "CalculateExtents", node "CalculateExtents"
static ShaderCode_CalculateExtents_CalculateExtents = `
@binding(0) @group(0) var Canvas : texture_2d<f32>;

struct Struct_DrawExtents_std430_0
{
    @align(8) MinX_0 : u32,
    @align(4) MaxX_0 : u32,
    @align(8) MinY_0 : u32,
    @align(4) MaxY_0 : u32,
    @align(8) PixelCount_0 : u32,
    @align(4) _padding0_0 : f32,
    @align(8) PixelLocationSum_0 : vec2<u32>,
};

@binding(1) @group(0) var<storage, read_write> DrawExtents : array<Struct_DrawExtents_std430_0>;

@compute
@workgroup_size(8, 8, 1)
fn CalculateExtents(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : bool;
    if((DTid_0.x) != u32(0))
    {
        _S1 = true;
    }
    else
    {
        _S1 = (DTid_0.y) != u32(0);
    }
    if(_S1)
    {
        return;
    }
    var canvasDims_0 : vec2<u32>;
    var _S2 : u32 = canvasDims_0[i32(0)];
    var _S3 : u32 = canvasDims_0[i32(1)];
    {var dim = textureDimensions((Canvas));((_S2)) = dim.x;((_S3)) = dim.y;};
    canvasDims_0[i32(0)] = _S2;
    canvasDims_0[i32(1)] = _S3;
    var _S4 : u32 = DrawExtents[i32(0)].MinX_0;
    var _S5 : u32 = DrawExtents[i32(0)].MaxX_0;
    var _S6 : u32 = DrawExtents[i32(0)].MinY_0;
    var _S7 : u32 = DrawExtents[i32(0)].MaxY_0;
    var _S8 : u32 = DrawExtents[i32(0)].PixelCount_0;
    var PixelLocationSum_1 : vec2<u32> = DrawExtents[i32(0)].PixelLocationSum_0;
    var minX_0 : u32 = _S4;
    var maxX_0 : u32 = _S5;
    var minY_0 : u32 = _S6;
    var maxY_0 : u32 = _S7;
    var iy_0 : u32 = u32(0);
    var pixelcount_0 : u32 = _S8;
    for(;;)
    {
        if(iy_0 < (canvasDims_0.y))
        {
        }
        else
        {
            break;
        }
        var minX_1 : u32 = minX_0;
        var maxX_1 : u32 = maxX_0;
        var minY_1 : u32 = minY_0;
        var maxY_1 : u32 = maxY_0;
        var ix_0 : u32 = u32(0);
        var pixelcount_1 : u32 = pixelcount_0;
        for(;;)
        {
            if(ix_0 < (canvasDims_0.x))
            {
            }
            else
            {
                break;
            }
            var _S9 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(ix_0, iy_0), u32(0)));
            if((textureLoad((Canvas), ((_S9)).xy, ((_S9)).z).x) == 0.0f)
            {
                ix_0 = ix_0 + u32(1);
                continue;
            }
            var _S10 : u32 = min(minX_1, ix_0);
            var _S11 : u32 = max(maxX_1, ix_0);
            var _S12 : u32 = min(minY_1, iy_0);
            var _S13 : u32 = max(maxY_1, iy_0);
            var _S14 : u32 = pixelcount_1 + u32(1);
            PixelLocationSum_1[i32(0)] = PixelLocationSum_1[i32(0)] + ix_0;
            PixelLocationSum_1[i32(1)] = PixelLocationSum_1[i32(1)] + iy_0;
            minX_1 = _S10;
            maxX_1 = _S11;
            minY_1 = _S12;
            maxY_1 = _S13;
            pixelcount_1 = _S14;
            ix_0 = ix_0 + u32(1);
        }
        var iy_1 : u32 = iy_0 + u32(1);
        minX_0 = minX_1;
        maxX_0 = maxX_1;
        minY_0 = minY_1;
        maxY_0 = maxY_1;
        iy_0 = iy_1;
        pixelcount_0 = pixelcount_1;
    }
    DrawExtents[i32(0)].MinX_0 = minX_0;
    DrawExtents[i32(0)].MaxX_0 = maxX_0;
    DrawExtents[i32(0)].MinY_0 = minY_0;
    DrawExtents[i32(0)].MaxY_0 = maxY_0;
    DrawExtents[i32(0)].PixelCount_0 = pixelcount_0;
    DrawExtents[i32(0)].PixelLocationSum_0 = PixelLocationSum_1;
    return;
}

`;

// Shader code for Compute shader "Conv1", node "Conv1"
static ShaderCode_Conv1_Conv1 = `
@binding(0) @group(0) var Input : texture_2d<f32>;

@binding(1) @group(0) var<storage, read> Weights : array<f32>;

@binding(2) @group(0) var<storage, read> Bias : array<f32>;

@binding(4) @group(0) var<storage, read_write> MaxValues : array<atomic<u32>>;

@binding(3) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var ix_0 : u32;
    var _S1 : u32 = DTid_0.z;
    var _S2 : i32 = i32(_S1 * u32(25));
    var readPx_0 : vec2<u32> = vec2<u32>(u32(0), u32(0));
    var iy_0 : u32 = u32(0);
    var result_0 : f32 = 0.0f;
    for(;;)
    {
        if(iy_0 < u32(5))
        {
        }
        else
        {
            break;
        }
        readPx_0[i32(1)] = DTid_0.y + iy_0;
        ix_0 = u32(0);
        for(;;)
        {
            if(ix_0 < u32(5))
            {
            }
            else
            {
                break;
            }
            readPx_0[i32(0)] = DTid_0.x + ix_0;
            var _S3 : vec3<i32> = vec3<i32>(vec3<u32>(readPx_0, u32(0)));
            var result_1 : f32 = result_0 + (textureLoad((Input), ((_S3)).xy, ((_S3)).z).x) * Weights[u32(_S2) + iy_0 * u32(5) + ix_0];
            ix_0 = ix_0 + u32(1);
            result_0 = result_1;
        }
        iy_0 = iy_0 + u32(1);
    }
    var result_2 : f32 = max(result_0 + Bias[_S1], 0.0f);
    var uvalue_0 : u32 = (bitcast<u32>((result_2)));
    if(((uvalue_0 >> (u32(31)))) == u32(0))
    {
        ix_0 = (uvalue_0 | (u32(2147483648)));
    }
    else
    {
        ix_0 = ~uvalue_0;
    }
    var _S4 : u32 = atomicMin(&(MaxValues[i32(0)]), ix_0);
    var _S5 : u32 = atomicMax(&(MaxValues[i32(1)]), ix_0);
    var _S6 : vec3<u32> = DTid_0.xyz;
    textureStore((Output), ((_S6)).xy, i32(((_S6)).z), vec4<f32>((result_2), 0, 0, 1));
    return;
}

`;

// Shader code for Compute shader "MaxPool1", node "MaxPool1"
static ShaderCode_MaxPool1_MaxPool1 = `
@binding(0) @group(0) var Input : texture_2d_array<f32>;

@binding(1) @group(0) var<storage, read_write> MaxValues : array<atomic<u32>>;

@binding(2) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : vec2<u32> = DTid_0.xy * vec2<u32>(u32(2));
    var _S2 : u32 = DTid_0.z;
    var _S3 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1, _S2), u32(0)));
    var _S4 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1 + vec2<u32>(u32(0), u32(1)), _S2), u32(0)));
    var _S5 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1 + vec2<u32>(u32(1), u32(0)), _S2), u32(0)));
    var _S6 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1 + vec2<u32>(u32(1), u32(1)), _S2), u32(0)));
    var result_0 : f32 = max(max((textureLoad((Input), ((_S3)).xy, i32(((_S3)).z), ((_S3)).w).x), (textureLoad((Input), ((_S4)).xy, i32(((_S4)).z), ((_S4)).w).x)), max((textureLoad((Input), ((_S5)).xy, i32(((_S5)).z), ((_S5)).w).x), (textureLoad((Input), ((_S6)).xy, i32(((_S6)).z), ((_S6)).w).x)));
    var uvalue_0 : u32 = (bitcast<u32>((result_0)));
    var uvalue_1 : u32;
    if(((uvalue_0 >> (u32(31)))) == u32(0))
    {
        uvalue_1 = (uvalue_0 | (u32(2147483648)));
    }
    else
    {
        uvalue_1 = ~uvalue_0;
    }
    var _S7 : u32 = atomicMin(&(MaxValues[i32(2)]), uvalue_1);
    var _S8 : u32 = atomicMax(&(MaxValues[i32(3)]), uvalue_1);
    textureStore((Output), ((DTid_0)).xy, i32(((DTid_0)).z), vec4<f32>((result_0), 0, 0, 1));
    return;
}

`;

// Shader code for Compute shader "Conv2", node "Conv2"
static ShaderCode_Conv2_Conv2 = `
@binding(0) @group(0) var Input : texture_2d_array<f32>;

@binding(2) @group(0) var<storage, read> Weights : array<f32>;

@binding(3) @group(0) var<storage, read> Bias : array<f32>;

@binding(1) @group(0) var<storage, read_write> MaxValues : array<atomic<u32>>;

@binding(4) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var iy_0 : u32;
    var _S1 : u32 = DTid_0.z;
    var _S2 : i32 = i32(_S1 * u32(18));
    var readPx_0 : vec3<u32> = vec3<u32>(u32(0), u32(0), u32(0));
    var iz_0 : u32 = u32(0);
    var result_0 : f32 = 0.0f;
    for(;;)
    {
        if(iz_0 < u32(2))
        {
        }
        else
        {
            break;
        }
        readPx_0[i32(2)] = iz_0;
        iy_0 = u32(0);
        for(;;)
        {
            if(iy_0 < u32(3))
            {
            }
            else
            {
                break;
            }
            readPx_0[i32(1)] = DTid_0.y + iy_0;
            var ix_0 : u32 = u32(0);
            var result_1 : f32 = result_0;
            for(;;)
            {
                if(ix_0 < u32(3))
                {
                }
                else
                {
                    break;
                }
                readPx_0[i32(0)] = DTid_0.x + ix_0;
                var _S3 : vec4<i32> = vec4<i32>(vec4<u32>(readPx_0, u32(0)));
                var result_2 : f32 = result_1 + (textureLoad((Input), ((_S3)).xy, i32(((_S3)).z), ((_S3)).w).x) * Weights[u32(_S2) + iz_0 * u32(9) + iy_0 * u32(3) + ix_0];
                ix_0 = ix_0 + u32(1);
                result_1 = result_2;
            }
            iy_0 = iy_0 + u32(1);
            result_0 = result_1;
        }
        iz_0 = iz_0 + u32(1);
    }
    var result_3 : f32 = 1.0f / (1.0f + exp(- (result_0 + Bias[_S1])));
    var uvalue_0 : u32 = (bitcast<u32>((result_3)));
    if(((uvalue_0 >> (u32(31)))) == u32(0))
    {
        iy_0 = (uvalue_0 | (u32(2147483648)));
    }
    else
    {
        iy_0 = ~uvalue_0;
    }
    var _S4 : u32 = atomicMin(&(MaxValues[i32(4)]), iy_0);
    var _S5 : u32 = atomicMax(&(MaxValues[i32(5)]), iy_0);
    var _S6 : vec3<u32> = DTid_0.xyz;
    textureStore((Output), ((_S6)).xy, i32(((_S6)).z), vec4<f32>((result_3), 0, 0, 1));
    return;
}

`;

// Shader code for Compute shader "MaxPool2", node "MaxPool2"
static ShaderCode_MaxPool2_MaxPool2 = `
@binding(0) @group(0) var Input : texture_2d_array<f32>;

@binding(1) @group(0) var<storage, read_write> MaxValues : array<atomic<u32>>;

@binding(2) @group(0) var Output : texture_storage_2d_array</*(Output_format)*/, write>;

@compute
@workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : vec2<u32> = DTid_0.xy * vec2<u32>(u32(2));
    var _S2 : u32 = DTid_0.z;
    var _S3 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1, _S2), u32(0)));
    var _S4 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1 + vec2<u32>(u32(0), u32(1)), _S2), u32(0)));
    var _S5 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1 + vec2<u32>(u32(1), u32(0)), _S2), u32(0)));
    var _S6 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(_S1 + vec2<u32>(u32(1), u32(1)), _S2), u32(0)));
    var result_0 : f32 = max(max((textureLoad((Input), ((_S3)).xy, i32(((_S3)).z), ((_S3)).w).x), (textureLoad((Input), ((_S4)).xy, i32(((_S4)).z), ((_S4)).w).x)), max((textureLoad((Input), ((_S5)).xy, i32(((_S5)).z), ((_S5)).w).x), (textureLoad((Input), ((_S6)).xy, i32(((_S6)).z), ((_S6)).w).x)));
    var uvalue_0 : u32 = (bitcast<u32>((result_0)));
    var uvalue_1 : u32;
    if(((uvalue_0 >> (u32(31)))) == u32(0))
    {
        uvalue_1 = (uvalue_0 | (u32(2147483648)));
    }
    else
    {
        uvalue_1 = ~uvalue_0;
    }
    var _S7 : u32 = atomicMin(&(MaxValues[i32(6)]), uvalue_1);
    var _S8 : u32 = atomicMax(&(MaxValues[i32(7)]), uvalue_1);
    textureStore((Output), ((DTid_0)).xy, i32(((DTid_0)).z), vec4<f32>((result_0), 0, 0, 1));
    return;
}

`;

// Shader code for Compute shader "Linear", node "Linear"
static ShaderCode_Linear_Linear = `
@binding(3) @group(0) var<storage, read> Bias : array<f32>;

@binding(0) @group(0) var Input : texture_2d_array<f32>;

@binding(2) @group(0) var<storage, read> Weights : array<f32>;

@binding(4) @group(0) var<storage, read_write> Output : array<f32>;

@binding(1) @group(0) var<storage, read_write> MaxValues : array<atomic<u32>>;

@compute
@workgroup_size(64, 1, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var iy_0 : u32;
    var _S1 : u32 = DTid_0.x;
    var _S2 : i32 = i32(_S1 * u32(100));
    var _S3 : f32 = Bias[_S1];
    var iz_0 : u32 = u32(0);
    var weightIndex_0 : i32 = _S2;
    var result_0 : f32 = _S3;
    for(;;)
    {
        if(iz_0 < u32(4))
        {
        }
        else
        {
            break;
        }
        iy_0 = u32(0);
        for(;;)
        {
            if(iy_0 < u32(5))
            {
            }
            else
            {
                break;
            }
            var ix_0 : u32 = u32(0);
            var weightIndex_1 : i32 = weightIndex_0;
            var result_1 : f32 = result_0;
            for(;;)
            {
                if(ix_0 < u32(5))
                {
                }
                else
                {
                    break;
                }
                var _S4 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(ix_0, iy_0, iz_0), u32(0)));
                var result_2 : f32 = result_1 + (textureLoad((Input), ((_S4)).xy, i32(((_S4)).z), ((_S4)).w).x) * Weights[weightIndex_1];
                var _S5 : i32 = weightIndex_1 + i32(1);
                ix_0 = ix_0 + u32(1);
                weightIndex_1 = _S5;
                result_1 = result_2;
            }
            iy_0 = iy_0 + u32(1);
            weightIndex_0 = weightIndex_1;
            result_0 = result_1;
        }
        iz_0 = iz_0 + u32(1);
    }
    Output[_S1] = result_0;
    var uvalue_0 : u32 = (bitcast<u32>((result_0)));
    if(((uvalue_0 >> (u32(31)))) == u32(0))
    {
        iy_0 = (uvalue_0 | (u32(2147483648)));
    }
    else
    {
        iy_0 = ~uvalue_0;
    }
    var _S6 : u32 = atomicMin(&(MaxValues[i32(8)]), iy_0);
    var _S7 : u32 = atomicMax(&(MaxValues[i32(9)]), iy_0);
    return;
}

`;

// Shader code for Compute shader "InitMaxValues", node "InitMaxValues"
static ShaderCode_InitMaxValues_InitMaxValues = `
@binding(0) @group(0) var<storage, read_write> MaxValues : array<u32>;

@compute
@workgroup_size(64, 1, 1)
fn main(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : u32 = DTid_0.x * u32(2);
    MaxValues[_S1] = u32(4294967295);
    MaxValues[_S1 + u32(1)] = u32(0);
    return;
}

`;

// -------------------- Struct Vertex Buffer Attributes

// Vertex buffer attributes for struct DrawExtents
static StructVertexBufferAttributes_DrawExtents =
[
    // MinX
    {
        format: "uint32",
        offset: 0,
        shaderLocation: 0,
    },
    // MaxX
    {
        format: "uint32",
        offset: 4,
        shaderLocation: 1,
    },
    // MinY
    {
        format: "uint32",
        offset: 8,
        shaderLocation: 2,
    },
    // MaxY
    {
        format: "uint32",
        offset: 12,
        shaderLocation: 3,
    },
    // PixelCount
    {
        format: "uint32",
        offset: 16,
        shaderLocation: 4,
    },
    // PixelLocationSum
    {
        format: "uint32x2",
        offset: 24,
        shaderLocation: 5,
    },
];

// Vertex buffer attributes for struct DrawExtents_Unpadded
static StructVertexBufferAttributes_DrawExtents_Unpadded =
[
    // MinX
    {
        format: "uint32",
        offset: 0,
        shaderLocation: 0,
    },
    // MaxX
    {
        format: "uint32",
        offset: 4,
        shaderLocation: 1,
    },
    // MinY
    {
        format: "uint32",
        offset: 8,
        shaderLocation: 2,
    },
    // MaxY
    {
        format: "uint32",
        offset: 12,
        shaderLocation: 3,
    },
    // PixelCount
    {
        format: "uint32",
        offset: 16,
        shaderLocation: 4,
    },
    // PixelLocationSum
    {
        format: "uint32x2",
        offset: 20,
        shaderLocation: 5,
    },
];

// -------------------- Private Members

// Texture Drawing_Canvas
texture_Drawing_Canvas = null;
texture_Drawing_Canvas_size = [0, 0, 0];
texture_Drawing_Canvas_format = "";
texture_Drawing_Canvas_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture NN_Input
texture_NN_Input = null;
texture_NN_Input_size = [0, 0, 0];
texture_NN_Input_format = "";
texture_NN_Input_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Buffer Draw_Extents
buffer_Draw_Extents = null;
buffer_Draw_Extents_count = 0;
buffer_Draw_Extents_stride = 0;
buffer_Draw_Extents_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Texture Conv1Output
texture_Conv1Output = null;
texture_Conv1Output_size = [0, 0, 0];
texture_Conv1Output_format = "";
texture_Conv1Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture MaxPool1Output
texture_MaxPool1Output = null;
texture_MaxPool1Output_size = [0, 0, 0];
texture_MaxPool1Output_format = "";
texture_MaxPool1Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture Conv2Output
texture_Conv2Output = null;
texture_Conv2Output_size = [0, 0, 0];
texture_Conv2Output_format = "";
texture_Conv2Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture MaxPool2Output
texture_MaxPool2Output = null;
texture_MaxPool2Output_size = [0, 0, 0];
texture_MaxPool2Output_format = "";
texture_MaxPool2Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Buffer MaxValues
buffer_MaxValues = null;
buffer_MaxValues_count = 0;
buffer_MaxValues_stride = 0;
buffer_MaxValues_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer LinearOutput
buffer_LinearOutput = null;
buffer_LinearOutput_count = 0;
buffer_LinearOutput_stride = 0;
buffer_LinearOutput_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Compute Shader InitMaxValues
Hash_Compute_InitMaxValues = 0;
ShaderModule_Compute_InitMaxValues = null;
BindGroupLayout_Compute_InitMaxValues = null;
PipelineLayout_Compute_InitMaxValues = null;
Pipeline_Compute_InitMaxValues = null;

// Texture Draw_Canvas_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Draw_Canvas_ReadOnly = null;
texture_Draw_Canvas_ReadOnly_size = [0, 0, 0];
texture_Draw_Canvas_ReadOnly_format = "";
texture_Draw_Canvas_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Shrink_NNInput_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Shrink_NNInput_ReadOnly = null;
texture_Shrink_NNInput_ReadOnly_size = [0, 0, 0];
texture_Shrink_NNInput_ReadOnly_format = "";
texture_Shrink_NNInput_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Presentation_PresentationCanvas_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Presentation_PresentationCanvas_ReadOnly = null;
texture_Presentation_PresentationCanvas_ReadOnly_size = [0, 0, 0];
texture_Presentation_PresentationCanvas_ReadOnly_format = "";
texture_Presentation_PresentationCanvas_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Conv1_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Conv1_Output_ReadOnly = null;
texture_Conv1_Output_ReadOnly_size = [0, 0, 0];
texture_Conv1_Output_ReadOnly_format = "";
texture_Conv1_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture MaxPool1_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_MaxPool1_Output_ReadOnly = null;
texture_MaxPool1_Output_ReadOnly_size = [0, 0, 0];
texture_MaxPool1_Output_ReadOnly_format = "";
texture_MaxPool1_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Conv2_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Conv2_Output_ReadOnly = null;
texture_Conv2_Output_ReadOnly_size = [0, 0, 0];
texture_Conv2_Output_ReadOnly_format = "";
texture_Conv2_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture MaxPool2_Output_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_MaxPool2_Output_ReadOnly = null;
texture_MaxPool2_Output_ReadOnly_size = [0, 0, 0];
texture_MaxPool2_Output_ReadOnly_format = "";
texture_MaxPool2_Output_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _DrawCB
constantBuffer__DrawCB = null;
constantBuffer__DrawCB_size = 64;
constantBuffer__DrawCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Draw
Hash_Compute_Draw = 0;
ShaderModule_Compute_Draw = null;
BindGroupLayout_Compute_Draw = null;
PipelineLayout_Compute_Draw = null;
Pipeline_Compute_Draw = null;

// Compute Shader CalculateExtents
Hash_Compute_CalculateExtents = 0;
ShaderModule_Compute_CalculateExtents = null;
BindGroupLayout_Compute_CalculateExtents = null;
PipelineLayout_Compute_CalculateExtents = null;
Pipeline_Compute_CalculateExtents = null;

// Constant buffer _ShrinkCB
constantBuffer__ShrinkCB = null;
constantBuffer__ShrinkCB_size = 16;
constantBuffer__ShrinkCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Shrink
Hash_Compute_Shrink = 0;
ShaderModule_Compute_Shrink = null;
BindGroupLayout_Compute_Shrink = null;
PipelineLayout_Compute_Shrink = null;
Pipeline_Compute_Shrink = null;

// Compute Shader Conv1
Hash_Compute_Conv1 = 0;
ShaderModule_Compute_Conv1 = null;
BindGroupLayout_Compute_Conv1 = null;
PipelineLayout_Compute_Conv1 = null;
Pipeline_Compute_Conv1 = null;

// Compute Shader MaxPool1
Hash_Compute_MaxPool1 = 0;
ShaderModule_Compute_MaxPool1 = null;
BindGroupLayout_Compute_MaxPool1 = null;
PipelineLayout_Compute_MaxPool1 = null;
Pipeline_Compute_MaxPool1 = null;

// Compute Shader Conv2
Hash_Compute_Conv2 = 0;
ShaderModule_Compute_Conv2 = null;
BindGroupLayout_Compute_Conv2 = null;
PipelineLayout_Compute_Conv2 = null;
Pipeline_Compute_Conv2 = null;

// Compute Shader MaxPool2
Hash_Compute_MaxPool2 = 0;
ShaderModule_Compute_MaxPool2 = null;
BindGroupLayout_Compute_MaxPool2 = null;
PipelineLayout_Compute_MaxPool2 = null;
Pipeline_Compute_MaxPool2 = null;

// Compute Shader Linear
Hash_Compute_Linear = 0;
ShaderModule_Compute_Linear = null;
BindGroupLayout_Compute_Linear = null;
PipelineLayout_Compute_Linear = null;
Pipeline_Compute_Linear = null;

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

// Texture _loadedTexture_3
texture__loadedTexture_3 = null;
texture__loadedTexture_3_size = [0, 0, 0];
texture__loadedTexture_3_format = "";
texture__loadedTexture_3_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_4
texture__loadedTexture_4 = null;
texture__loadedTexture_4_size = [0, 0, 0];
texture__loadedTexture_4_format = "";
texture__loadedTexture_4_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_5
texture__loadedTexture_5 = null;
texture__loadedTexture_5_size = [0, 0, 0];
texture__loadedTexture_5_format = "";
texture__loadedTexture_5_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_6
texture__loadedTexture_6 = null;
texture__loadedTexture_6_size = [0, 0, 0];
texture__loadedTexture_6_format = "";
texture__loadedTexture_6_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_7
texture__loadedTexture_7 = null;
texture__loadedTexture_7_size = [0, 0, 0];
texture__loadedTexture_7_format = "";
texture__loadedTexture_7_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_8
texture__loadedTexture_8 = null;
texture__loadedTexture_8_size = [0, 0, 0];
texture__loadedTexture_8_format = "";
texture__loadedTexture_8_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture _loadedTexture_9
texture__loadedTexture_9 = null;
texture__loadedTexture_9_size = [0, 0, 0];
texture__loadedTexture_9_format = "";
texture__loadedTexture_9_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Constant buffer _PresentationCB
constantBuffer__PresentationCB = null;
constantBuffer__PresentationCB_size = 48;
constantBuffer__PresentationCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Presentation
Hash_Compute_Presentation = 0;
ShaderModule_Compute_Presentation = null;
BindGroupLayout_Compute_Presentation = null;
PipelineLayout_Compute_Presentation = null;
Pipeline_Compute_Presentation = null;

// -------------------- Imported Members

// Texture Imported_Image
texture_Imported_Image = null;
texture_Imported_Image_size = [0, 0, 0];
texture_Imported_Image_format = "";
texture_Imported_Image_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Buffer Conv1Weights
buffer_Conv1Weights = null;
buffer_Conv1Weights_count = 0;
buffer_Conv1Weights_stride = 0;
buffer_Conv1Weights_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer Conv1Bias
buffer_Conv1Bias = null;
buffer_Conv1Bias_count = 0;
buffer_Conv1Bias_stride = 0;
buffer_Conv1Bias_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer Conv2Weights
buffer_Conv2Weights = null;
buffer_Conv2Weights_count = 0;
buffer_Conv2Weights_stride = 0;
buffer_Conv2Weights_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer Conv2Bias
buffer_Conv2Bias = null;
buffer_Conv2Bias_count = 0;
buffer_Conv2Bias_stride = 0;
buffer_Conv2Bias_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer LinearWeights
buffer_LinearWeights = null;
buffer_LinearWeights_count = 0;
buffer_LinearWeights_stride = 0;
buffer_LinearWeights_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer LinearBias
buffer_LinearBias = null;
buffer_LinearBias_count = 0;
buffer_LinearBias_stride = 0;
buffer_LinearBias_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// -------------------- Exported Members

// Texture Presentation_Canvas
texture_Presentation_Canvas = null;
texture_Presentation_Canvas_size = [0, 0, 0];
texture_Presentation_Canvas_format = "";
texture_Presentation_Canvas_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_Clear = false;
variableDefault_Clear = false;
variableChanged_Clear = false;
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
variable_PenSize = 10.000000;
variableDefault_PenSize = 10.000000;
variableChanged_PenSize = false;
variable_UseImportedImage = false;
variableDefault_UseImportedImage = false;
variableChanged_UseImportedImage = false;
variable_NormalizeDrawing = true;  // MNIST normalization: shrink image to 20x20 and put center of mass in the middle of a 28x28 image
variableDefault_NormalizeDrawing = true;  // MNIST normalization: shrink image to 20x20 and put center of mass in the middle of a 28x28 image
variableChanged_NormalizeDrawing = false;
variable_Conv1OutputScale = 4;  // Scale Conv1 output. 24x24x2 unscaled.
variableDefault_Conv1OutputScale = 4;  // Scale Conv1 output. 24x24x2 unscaled.
variableChanged_Conv1OutputScale = false;
variable_MaxPool1OutputScale = 8;  // Scale MaxPool1 output. 12x12x2 unscaled.
variableDefault_MaxPool1OutputScale = 8;  // Scale MaxPool1 output. 12x12x2 unscaled.
variableChanged_MaxPool1OutputScale = false;
variable_Conv2OutputScale = 8;  // Scale Conv2 output. 10x10x4 unscaled.
variableDefault_Conv2OutputScale = 8;  // Scale Conv2 output. 10x10x4 unscaled.
variableChanged_Conv2OutputScale = false;
variable_MaxPool2OutputScale = 16;  // Scale MaxPool2 output. 5x5x4 unscaled.
variableDefault_MaxPool2OutputScale = 16;  // Scale MaxPool2 output. 5x5x4 unscaled.
variableChanged_MaxPool2OutputScale = false;

// -------------------- Private Variables

variable_c_NNInputImageSize = [ 28, 28 ];
variableDefault_c_NNInputImageSize = [ 28, 28 ];
variableChanged_c_NNInputImageSize = [ false, false ];
variable_c_drawingCanvasSize = [ 256, 256 ];
variableDefault_c_drawingCanvasSize = [ 256, 256 ];
variableChanged_c_drawingCanvasSize = [ false, false ];
variable_initialized = false;
variableDefault_initialized = false;
variableChanged_initialized = false;
// -------------------- Structs

static StructOffsets_DrawExtents =
{
    MinX: 0,
    MaxX: 4,
    MinY: 8,
    MaxY: 12,
    PixelCount: 16,
    _padding0: 20,
    PixelLocationSum_0: 24,
    PixelLocationSum_1: 28,
    _size: 32,
}

static StructOffsets__DrawCB =
{
    Clear: 0,
    _padding0: 4,
    _padding1: 8,
    _padding2: 12,
    MouseState_0: 16,
    MouseState_1: 20,
    MouseState_2: 24,
    MouseState_3: 28,
    MouseStateLastFrame_0: 32,
    MouseStateLastFrame_1: 36,
    MouseStateLastFrame_2: 40,
    MouseStateLastFrame_3: 44,
    PenSize: 48,
    UseImportedImage: 52,
    iFrame: 56,
    _padding3: 60,
    _size: 64,
}

static StructOffsets__ShrinkCB =
{
    NormalizeDrawing: 0,
    UseImportedImage: 4,
    _padding0: 8,
    _padding1: 12,
    _size: 16,
}

static StructOffsets__PresentationCB =
{
    Conv1OutputScale: 0,
    Conv2OutputScale: 4,
    MaxPool1OutputScale: 8,
    MaxPool2OutputScale: 12,
    MouseState_0: 16,
    MouseState_1: 20,
    MouseState_2: 24,
    MouseState_3: 28,
    PenSize: 32,
    UseImportedImage: 36,
    _padding0: 40,
    _padding1: 44,
    _size: 48,
}

static StructOffsets_DrawExtents_Unpadded =
{
    MinX: 0,
    MaxX: 4,
    MinY: 8,
    MaxY: 12,
    PixelCount: 16,
    PixelLocationSum_0: 20,
    PixelLocationSum_1: 24,
    _size: 28,
}


async SetVarsBefore()
{
    if (this.variable_initialized)
    {
        this.variableChanged_iFrame = true;
        this.variable_iFrame = this.variable_iFrame + 1;
    }

    if (!this.variable_initialized)
    {
        this.variableChanged_iFrame = true;
        this.variable_iFrame = 0 + 0;
    }


}

async SetVarsAfter()
{
    {
        this.variableChanged_initialized = true;
        this.variable_initialized = true || true;
    }


}

async ValidateImports()
{
    // Validate texture Imported_Image
    if (this.texture_Imported_Image === null)
    {
        Shared.LogError("Imported resource texture_Imported_Image was not provided");
        return false;
    }

    // Validate buffer Conv1Weights
    if (this.buffer_Conv1Weights === null)
    {
        Shared.LogError("Imported resource buffer_Conv1Weights was not provided");
        return false;
    }

    // Validate buffer Conv1Bias
    if (this.buffer_Conv1Bias === null)
    {
        Shared.LogError("Imported resource buffer_Conv1Bias was not provided");
        return false;
    }

    // Validate buffer Conv2Weights
    if (this.buffer_Conv2Weights === null)
    {
        Shared.LogError("Imported resource buffer_Conv2Weights was not provided");
        return false;
    }

    // Validate buffer Conv2Bias
    if (this.buffer_Conv2Bias === null)
    {
        Shared.LogError("Imported resource buffer_Conv2Bias was not provided");
        return false;
    }

    // Validate buffer LinearWeights
    if (this.buffer_LinearWeights === null)
    {
        Shared.LogError("Imported resource buffer_LinearWeights was not provided");
        return false;
    }

    // Validate buffer LinearBias
    if (this.buffer_LinearBias === null)
    {
        Shared.LogError("Imported resource buffer_LinearBias was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Drawing_Canvas
    {
        const baseSize = [ this.variable_c_drawingCanvasSize[0], this.variable_c_drawingCanvasSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "r32float";
        if (this.texture_Drawing_Canvas !== null && (this.texture_Drawing_Canvas_format != desiredFormat || this.texture_Drawing_Canvas_size[0] != desiredSize[0] || this.texture_Drawing_Canvas_size[1] != desiredSize[1] || this.texture_Drawing_Canvas_size[2] != desiredSize[2]))
        {
            this.texture_Drawing_Canvas.destroy();
            this.texture_Drawing_Canvas = null;
        }

        if (this.texture_Drawing_Canvas === null)
        {
            this.texture_Drawing_Canvas_size = desiredSize.slice();
            this.texture_Drawing_Canvas_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Drawing_Canvas_format))
                viewFormats.push(this.texture_Drawing_Canvas_format);

            this.texture_Drawing_Canvas = device.createTexture({
                label: "texture mnist.Drawing_Canvas",
                size: this.texture_Drawing_Canvas_size,
                format: Shared.GetNonSRGBFormat(this.texture_Drawing_Canvas_format),
                usage: this.texture_Drawing_Canvas_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture NN_Input
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 28) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 28) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "r32float";
        if (this.texture_NN_Input !== null && (this.texture_NN_Input_format != desiredFormat || this.texture_NN_Input_size[0] != desiredSize[0] || this.texture_NN_Input_size[1] != desiredSize[1] || this.texture_NN_Input_size[2] != desiredSize[2]))
        {
            this.texture_NN_Input.destroy();
            this.texture_NN_Input = null;
        }

        if (this.texture_NN_Input === null)
        {
            this.texture_NN_Input_size = desiredSize.slice();
            this.texture_NN_Input_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_NN_Input_format))
                viewFormats.push(this.texture_NN_Input_format);

            this.texture_NN_Input = device.createTexture({
                label: "texture mnist.NN_Input",
                size: this.texture_NN_Input_size,
                format: Shared.GetNonSRGBFormat(this.texture_NN_Input_format),
                usage: this.texture_NN_Input_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Presentation_Canvas
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 880) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 630) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_Presentation_Canvas !== null && (this.texture_Presentation_Canvas_format != desiredFormat || this.texture_Presentation_Canvas_size[0] != desiredSize[0] || this.texture_Presentation_Canvas_size[1] != desiredSize[1] || this.texture_Presentation_Canvas_size[2] != desiredSize[2]))
        {
            this.texture_Presentation_Canvas.destroy();
            this.texture_Presentation_Canvas = null;
        }

        if (this.texture_Presentation_Canvas === null)
        {
            this.texture_Presentation_Canvas_size = desiredSize.slice();
            this.texture_Presentation_Canvas_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Presentation_Canvas_format))
                viewFormats.push(this.texture_Presentation_Canvas_format);

            this.texture_Presentation_Canvas = device.createTexture({
                label: "texture mnist.Presentation_Canvas",
                size: this.texture_Presentation_Canvas_size,
                format: Shared.GetNonSRGBFormat(this.texture_Presentation_Canvas_format),
                usage: this.texture_Presentation_Canvas_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of buffer Draw_Extents
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = 32;
        if (this.buffer_Draw_Extents !== null && (this.buffer_Draw_Extents_count != desiredCount || this.buffer_Draw_Extents_stride != desiredStride))
        {
            this.buffer_Draw_Extents.destroy();
            this.buffer_Draw_Extents = null;
        }

        if (this.buffer_Draw_Extents === null)
        {
            this.buffer_Draw_Extents_count = desiredCount;
            this.buffer_Draw_Extents_stride = desiredStride;
            this.buffer_Draw_Extents = device.createBuffer({
                label: "buffer mnist.Draw_Extents",
                size: Shared.Align(16, this.buffer_Draw_Extents_count * this.buffer_Draw_Extents_stride),
                usage: this.buffer_Draw_Extents_usageFlags,
            });
        }
    }

    // Handle (re)creation of texture Conv1Output
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 24) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 24) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 2) / 1) + 0
        ];
        const desiredFormat = "r32float";
        if (this.texture_Conv1Output !== null && (this.texture_Conv1Output_format != desiredFormat || this.texture_Conv1Output_size[0] != desiredSize[0] || this.texture_Conv1Output_size[1] != desiredSize[1] || this.texture_Conv1Output_size[2] != desiredSize[2]))
        {
            this.texture_Conv1Output.destroy();
            this.texture_Conv1Output = null;
        }

        if (this.texture_Conv1Output === null)
        {
            this.texture_Conv1Output_size = desiredSize.slice();
            this.texture_Conv1Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Conv1Output_format))
                viewFormats.push(this.texture_Conv1Output_format);

            this.texture_Conv1Output = device.createTexture({
                label: "texture mnist.Conv1Output",
                size: this.texture_Conv1Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Conv1Output_format),
                usage: this.texture_Conv1Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture MaxPool1Output
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 12) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 12) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 2) / 1) + 0
        ];
        const desiredFormat = "r32float";
        if (this.texture_MaxPool1Output !== null && (this.texture_MaxPool1Output_format != desiredFormat || this.texture_MaxPool1Output_size[0] != desiredSize[0] || this.texture_MaxPool1Output_size[1] != desiredSize[1] || this.texture_MaxPool1Output_size[2] != desiredSize[2]))
        {
            this.texture_MaxPool1Output.destroy();
            this.texture_MaxPool1Output = null;
        }

        if (this.texture_MaxPool1Output === null)
        {
            this.texture_MaxPool1Output_size = desiredSize.slice();
            this.texture_MaxPool1Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_MaxPool1Output_format))
                viewFormats.push(this.texture_MaxPool1Output_format);

            this.texture_MaxPool1Output = device.createTexture({
                label: "texture mnist.MaxPool1Output",
                size: this.texture_MaxPool1Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_MaxPool1Output_format),
                usage: this.texture_MaxPool1Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Conv2Output
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 10) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 10) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 4) / 1) + 0
        ];
        const desiredFormat = "r32float";
        if (this.texture_Conv2Output !== null && (this.texture_Conv2Output_format != desiredFormat || this.texture_Conv2Output_size[0] != desiredSize[0] || this.texture_Conv2Output_size[1] != desiredSize[1] || this.texture_Conv2Output_size[2] != desiredSize[2]))
        {
            this.texture_Conv2Output.destroy();
            this.texture_Conv2Output = null;
        }

        if (this.texture_Conv2Output === null)
        {
            this.texture_Conv2Output_size = desiredSize.slice();
            this.texture_Conv2Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Conv2Output_format))
                viewFormats.push(this.texture_Conv2Output_format);

            this.texture_Conv2Output = device.createTexture({
                label: "texture mnist.Conv2Output",
                size: this.texture_Conv2Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Conv2Output_format),
                usage: this.texture_Conv2Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture MaxPool2Output
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 5) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 5) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 4) / 1) + 0
        ];
        const desiredFormat = "r32float";
        if (this.texture_MaxPool2Output !== null && (this.texture_MaxPool2Output_format != desiredFormat || this.texture_MaxPool2Output_size[0] != desiredSize[0] || this.texture_MaxPool2Output_size[1] != desiredSize[1] || this.texture_MaxPool2Output_size[2] != desiredSize[2]))
        {
            this.texture_MaxPool2Output.destroy();
            this.texture_MaxPool2Output = null;
        }

        if (this.texture_MaxPool2Output === null)
        {
            this.texture_MaxPool2Output_size = desiredSize.slice();
            this.texture_MaxPool2Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_MaxPool2Output_format))
                viewFormats.push(this.texture_MaxPool2Output_format);

            this.texture_MaxPool2Output = device.createTexture({
                label: "texture mnist.MaxPool2Output",
                size: this.texture_MaxPool2Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_MaxPool2Output_format),
                usage: this.texture_MaxPool2Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of buffer MaxValues
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 10) / 1) + 0;
        const desiredStride = 4;
        if (this.buffer_MaxValues !== null && (this.buffer_MaxValues_count != desiredCount || this.buffer_MaxValues_stride != desiredStride))
        {
            this.buffer_MaxValues.destroy();
            this.buffer_MaxValues = null;
        }

        if (this.buffer_MaxValues === null)
        {
            this.buffer_MaxValues_count = desiredCount;
            this.buffer_MaxValues_stride = desiredStride;
            this.buffer_MaxValues = device.createBuffer({
                label: "buffer mnist.MaxValues",
                size: Shared.Align(16, this.buffer_MaxValues_count * this.buffer_MaxValues_stride),
                usage: this.buffer_MaxValues_usageFlags,
            });
        }
    }

    // Handle (re)creation of buffer LinearOutput
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 10) / 1) + 0;
        const desiredStride = 4;
        if (this.buffer_LinearOutput !== null && (this.buffer_LinearOutput_count != desiredCount || this.buffer_LinearOutput_stride != desiredStride))
        {
            this.buffer_LinearOutput.destroy();
            this.buffer_LinearOutput = null;
        }

        if (this.buffer_LinearOutput === null)
        {
            this.buffer_LinearOutput_count = desiredCount;
            this.buffer_LinearOutput_stride = desiredStride;
            this.buffer_LinearOutput = device.createBuffer({
                label: "buffer mnist.LinearOutput",
                size: Shared.Align(16, this.buffer_LinearOutput_count * this.buffer_LinearOutput_stride),
                usage: this.buffer_LinearOutput_usageFlags,
            });
        }
    }

    // (Re)create compute shader InitMaxValues
    {
        const bindGroupEntries =
        [
            {
                // MaxValues
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_InitMaxValues === null || newHash !== this.Hash_Compute_InitMaxValues)
        {
            this.Hash_Compute_InitMaxValues = newHash;

            let shaderCode = class_mnist.ShaderCode_InitMaxValues_InitMaxValues;

            this.ShaderModule_Compute_InitMaxValues = device.createShaderModule({ code: shaderCode, label: "Compute Shader InitMaxValues"});
            this.BindGroupLayout_Compute_InitMaxValues = device.createBindGroupLayout({
                label: "Compute Bind Group Layout InitMaxValues",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_InitMaxValues = device.createPipelineLayout({
                label: "Compute Pipeline Layout InitMaxValues",
                bindGroupLayouts: [this.BindGroupLayout_Compute_InitMaxValues],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_InitMaxValues = device.createComputePipeline({
                    label: "Compute Pipeline InitMaxValues",
                    layout: this.PipelineLayout_Compute_InitMaxValues,
                    compute: {
                        module: this.ShaderModule_Compute_InitMaxValues,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("InitMaxValues");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline InitMaxValues",
                    layout: this.PipelineLayout_Compute_InitMaxValues,
                    compute: {
                        module: this.ShaderModule_Compute_InitMaxValues,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_InitMaxValues = handle; this.loadingPromises.delete("InitMaxValues"); } );
            }
        }
    }

    // Handle (re)creation of texture Draw_Canvas_ReadOnly
    {
        const baseSize = this.texture_Drawing_Canvas_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Drawing_Canvas_format;
        if (this.texture_Draw_Canvas_ReadOnly !== null && (this.texture_Draw_Canvas_ReadOnly_format != desiredFormat || this.texture_Draw_Canvas_ReadOnly_size[0] != desiredSize[0] || this.texture_Draw_Canvas_ReadOnly_size[1] != desiredSize[1] || this.texture_Draw_Canvas_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Draw_Canvas_ReadOnly.destroy();
            this.texture_Draw_Canvas_ReadOnly = null;
        }

        if (this.texture_Draw_Canvas_ReadOnly === null)
        {
            this.texture_Draw_Canvas_ReadOnly_size = desiredSize.slice();
            this.texture_Draw_Canvas_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Draw_Canvas_ReadOnly_format))
                viewFormats.push(this.texture_Draw_Canvas_ReadOnly_format);

            this.texture_Draw_Canvas_ReadOnly = device.createTexture({
                label: "texture mnist.Draw_Canvas_ReadOnly",
                size: this.texture_Draw_Canvas_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Draw_Canvas_ReadOnly_format),
                usage: this.texture_Draw_Canvas_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Shrink_NNInput_ReadOnly
    {
        const baseSize = this.texture_NN_Input_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_NN_Input_format;
        if (this.texture_Shrink_NNInput_ReadOnly !== null && (this.texture_Shrink_NNInput_ReadOnly_format != desiredFormat || this.texture_Shrink_NNInput_ReadOnly_size[0] != desiredSize[0] || this.texture_Shrink_NNInput_ReadOnly_size[1] != desiredSize[1] || this.texture_Shrink_NNInput_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Shrink_NNInput_ReadOnly.destroy();
            this.texture_Shrink_NNInput_ReadOnly = null;
        }

        if (this.texture_Shrink_NNInput_ReadOnly === null)
        {
            this.texture_Shrink_NNInput_ReadOnly_size = desiredSize.slice();
            this.texture_Shrink_NNInput_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Shrink_NNInput_ReadOnly_format))
                viewFormats.push(this.texture_Shrink_NNInput_ReadOnly_format);

            this.texture_Shrink_NNInput_ReadOnly = device.createTexture({
                label: "texture mnist.Shrink_NNInput_ReadOnly",
                size: this.texture_Shrink_NNInput_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Shrink_NNInput_ReadOnly_format),
                usage: this.texture_Shrink_NNInput_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Presentation_PresentationCanvas_ReadOnly
    {
        const baseSize = this.texture_Presentation_Canvas_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Presentation_Canvas_format;
        if (this.texture_Presentation_PresentationCanvas_ReadOnly !== null && (this.texture_Presentation_PresentationCanvas_ReadOnly_format != desiredFormat || this.texture_Presentation_PresentationCanvas_ReadOnly_size[0] != desiredSize[0] || this.texture_Presentation_PresentationCanvas_ReadOnly_size[1] != desiredSize[1] || this.texture_Presentation_PresentationCanvas_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Presentation_PresentationCanvas_ReadOnly.destroy();
            this.texture_Presentation_PresentationCanvas_ReadOnly = null;
        }

        if (this.texture_Presentation_PresentationCanvas_ReadOnly === null)
        {
            this.texture_Presentation_PresentationCanvas_ReadOnly_size = desiredSize.slice();
            this.texture_Presentation_PresentationCanvas_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Presentation_PresentationCanvas_ReadOnly_format))
                viewFormats.push(this.texture_Presentation_PresentationCanvas_ReadOnly_format);

            this.texture_Presentation_PresentationCanvas_ReadOnly = device.createTexture({
                label: "texture mnist.Presentation_PresentationCanvas_ReadOnly",
                size: this.texture_Presentation_PresentationCanvas_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Presentation_PresentationCanvas_ReadOnly_format),
                usage: this.texture_Presentation_PresentationCanvas_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Conv1_Output_ReadOnly
    {
        const baseSize = this.texture_Conv1Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Conv1Output_format;
        if (this.texture_Conv1_Output_ReadOnly !== null && (this.texture_Conv1_Output_ReadOnly_format != desiredFormat || this.texture_Conv1_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Conv1_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Conv1_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Conv1_Output_ReadOnly.destroy();
            this.texture_Conv1_Output_ReadOnly = null;
        }

        if (this.texture_Conv1_Output_ReadOnly === null)
        {
            this.texture_Conv1_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Conv1_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Conv1_Output_ReadOnly_format))
                viewFormats.push(this.texture_Conv1_Output_ReadOnly_format);

            this.texture_Conv1_Output_ReadOnly = device.createTexture({
                label: "texture mnist.Conv1_Output_ReadOnly",
                size: this.texture_Conv1_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Conv1_Output_ReadOnly_format),
                usage: this.texture_Conv1_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture MaxPool1_Output_ReadOnly
    {
        const baseSize = this.texture_MaxPool1Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_MaxPool1Output_format;
        if (this.texture_MaxPool1_Output_ReadOnly !== null && (this.texture_MaxPool1_Output_ReadOnly_format != desiredFormat || this.texture_MaxPool1_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_MaxPool1_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_MaxPool1_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_MaxPool1_Output_ReadOnly.destroy();
            this.texture_MaxPool1_Output_ReadOnly = null;
        }

        if (this.texture_MaxPool1_Output_ReadOnly === null)
        {
            this.texture_MaxPool1_Output_ReadOnly_size = desiredSize.slice();
            this.texture_MaxPool1_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_MaxPool1_Output_ReadOnly_format))
                viewFormats.push(this.texture_MaxPool1_Output_ReadOnly_format);

            this.texture_MaxPool1_Output_ReadOnly = device.createTexture({
                label: "texture mnist.MaxPool1_Output_ReadOnly",
                size: this.texture_MaxPool1_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_MaxPool1_Output_ReadOnly_format),
                usage: this.texture_MaxPool1_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Conv2_Output_ReadOnly
    {
        const baseSize = this.texture_Conv2Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Conv2Output_format;
        if (this.texture_Conv2_Output_ReadOnly !== null && (this.texture_Conv2_Output_ReadOnly_format != desiredFormat || this.texture_Conv2_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_Conv2_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_Conv2_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Conv2_Output_ReadOnly.destroy();
            this.texture_Conv2_Output_ReadOnly = null;
        }

        if (this.texture_Conv2_Output_ReadOnly === null)
        {
            this.texture_Conv2_Output_ReadOnly_size = desiredSize.slice();
            this.texture_Conv2_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Conv2_Output_ReadOnly_format))
                viewFormats.push(this.texture_Conv2_Output_ReadOnly_format);

            this.texture_Conv2_Output_ReadOnly = device.createTexture({
                label: "texture mnist.Conv2_Output_ReadOnly",
                size: this.texture_Conv2_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Conv2_Output_ReadOnly_format),
                usage: this.texture_Conv2_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture MaxPool2_Output_ReadOnly
    {
        const baseSize = this.texture_MaxPool2Output_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_MaxPool2Output_format;
        if (this.texture_MaxPool2_Output_ReadOnly !== null && (this.texture_MaxPool2_Output_ReadOnly_format != desiredFormat || this.texture_MaxPool2_Output_ReadOnly_size[0] != desiredSize[0] || this.texture_MaxPool2_Output_ReadOnly_size[1] != desiredSize[1] || this.texture_MaxPool2_Output_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_MaxPool2_Output_ReadOnly.destroy();
            this.texture_MaxPool2_Output_ReadOnly = null;
        }

        if (this.texture_MaxPool2_Output_ReadOnly === null)
        {
            this.texture_MaxPool2_Output_ReadOnly_size = desiredSize.slice();
            this.texture_MaxPool2_Output_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_MaxPool2_Output_ReadOnly_format))
                viewFormats.push(this.texture_MaxPool2_Output_ReadOnly_format);

            this.texture_MaxPool2_Output_ReadOnly = device.createTexture({
                label: "texture mnist.MaxPool2_Output_ReadOnly",
                size: this.texture_MaxPool2_Output_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_MaxPool2_Output_ReadOnly_format),
                usage: this.texture_MaxPool2_Output_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Draw
    {
        const bindGroupEntries =
        [
            {
                // Canvas
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Drawing_Canvas_format), viewDimension: "2d" }
            },
            {
                // DrawExtents
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // CanvasReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Draw_Canvas_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _DrawCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Draw === null || newHash !== this.Hash_Compute_Draw)
        {
            this.Hash_Compute_Draw = newHash;

            let shaderCode = class_mnist.ShaderCode_Draw_Draw;
            shaderCode = shaderCode.replace("/*(Canvas_format)*/", Shared.GetNonSRGBFormat(this.texture_Drawing_Canvas_format));
            shaderCode = shaderCode.replace("/*(CanvasReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Draw_Canvas_ReadOnly_format));

            this.ShaderModule_Compute_Draw = device.createShaderModule({ code: shaderCode, label: "Compute Shader Draw"});
            this.BindGroupLayout_Compute_Draw = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Draw",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Draw = device.createPipelineLayout({
                label: "Compute Pipeline Layout Draw",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Draw],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Draw = device.createComputePipeline({
                    label: "Compute Pipeline Draw",
                    layout: this.PipelineLayout_Compute_Draw,
                    compute: {
                        module: this.ShaderModule_Compute_Draw,
                        entryPoint: "Draw",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Draw");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Draw",
                    layout: this.PipelineLayout_Compute_Draw,
                    compute: {
                        module: this.ShaderModule_Compute_Draw,
                        entryPoint: "Draw",
                    }
                }).then( handle => { this.Pipeline_Compute_Draw = handle; this.loadingPromises.delete("Draw"); } );
            }
        }
    }

    // (Re)create compute shader CalculateExtents
    {
        const bindGroupEntries =
        [
            {
                // Canvas
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Drawing_Canvas_format).sampleType }
            },
            {
                // DrawExtents
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_CalculateExtents === null || newHash !== this.Hash_Compute_CalculateExtents)
        {
            this.Hash_Compute_CalculateExtents = newHash;

            let shaderCode = class_mnist.ShaderCode_CalculateExtents_CalculateExtents;

            this.ShaderModule_Compute_CalculateExtents = device.createShaderModule({ code: shaderCode, label: "Compute Shader CalculateExtents"});
            this.BindGroupLayout_Compute_CalculateExtents = device.createBindGroupLayout({
                label: "Compute Bind Group Layout CalculateExtents",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_CalculateExtents = device.createPipelineLayout({
                label: "Compute Pipeline Layout CalculateExtents",
                bindGroupLayouts: [this.BindGroupLayout_Compute_CalculateExtents],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_CalculateExtents = device.createComputePipeline({
                    label: "Compute Pipeline CalculateExtents",
                    layout: this.PipelineLayout_Compute_CalculateExtents,
                    compute: {
                        module: this.ShaderModule_Compute_CalculateExtents,
                        entryPoint: "CalculateExtents",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("CalculateExtents");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline CalculateExtents",
                    layout: this.PipelineLayout_Compute_CalculateExtents,
                    compute: {
                        module: this.ShaderModule_Compute_CalculateExtents,
                        entryPoint: "CalculateExtents",
                    }
                }).then( handle => { this.Pipeline_Compute_CalculateExtents = handle; this.loadingPromises.delete("CalculateExtents"); } );
            }
        }
    }

    // (Re)create compute shader Shrink
    {
        const bindGroupEntries =
        [
            {
                // Canvas
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Drawing_Canvas_format).sampleType }
            },
            {
                // DrawExtents
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // NNInput
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_NN_Input_format), viewDimension: "2d" }
            },
            {
                // ImportedImage
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Imported_Image_format).sampleType }
            },
            {
                // NNInputReadOnly
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Shrink_NNInput_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _ShrinkCB
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Shrink === null || newHash !== this.Hash_Compute_Shrink)
        {
            this.Hash_Compute_Shrink = newHash;

            let shaderCode = class_mnist.ShaderCode_Shrink_Shrink;
            shaderCode = shaderCode.replace("/*(NNInput_format)*/", Shared.GetNonSRGBFormat(this.texture_NN_Input_format));
            shaderCode = shaderCode.replace("/*(NNInputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Shrink_NNInput_ReadOnly_format));

            this.ShaderModule_Compute_Shrink = device.createShaderModule({ code: shaderCode, label: "Compute Shader Shrink"});
            this.BindGroupLayout_Compute_Shrink = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Shrink",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Shrink = device.createPipelineLayout({
                label: "Compute Pipeline Layout Shrink",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Shrink],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Shrink = device.createComputePipeline({
                    label: "Compute Pipeline Shrink",
                    layout: this.PipelineLayout_Compute_Shrink,
                    compute: {
                        module: this.ShaderModule_Compute_Shrink,
                        entryPoint: "Shrink",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Shrink");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Shrink",
                    layout: this.PipelineLayout_Compute_Shrink,
                    compute: {
                        module: this.ShaderModule_Compute_Shrink,
                        entryPoint: "Shrink",
                    }
                }).then( handle => { this.Pipeline_Compute_Shrink = handle; this.loadingPromises.delete("Shrink"); } );
            }
        }
    }

    // (Re)create compute shader Conv1
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_NN_Input_format).sampleType }
            },
            {
                // Weights
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Bias
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Output
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Conv1Output_format), viewDimension: "2d-array" }
            },
            {
                // MaxValues
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // OutputReadOnly
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Conv1_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Conv1 === null || newHash !== this.Hash_Compute_Conv1)
        {
            this.Hash_Compute_Conv1 = newHash;

            let shaderCode = class_mnist.ShaderCode_Conv1_Conv1;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Conv1Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Conv1_Output_ReadOnly_format));

            this.ShaderModule_Compute_Conv1 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Conv1"});
            this.BindGroupLayout_Compute_Conv1 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Conv1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Conv1 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Conv1",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Conv1],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Conv1 = device.createComputePipeline({
                    label: "Compute Pipeline Conv1",
                    layout: this.PipelineLayout_Compute_Conv1,
                    compute: {
                        module: this.ShaderModule_Compute_Conv1,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Conv1");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Conv1",
                    layout: this.PipelineLayout_Compute_Conv1,
                    compute: {
                        module: this.ShaderModule_Compute_Conv1,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Conv1 = handle; this.loadingPromises.delete("Conv1"); } );
            }
        }
    }

    // (Re)create compute shader MaxPool1
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_Conv1Output_format).sampleType }
            },
            {
                // MaxValues
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // Output
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_MaxPool1Output_format), viewDimension: "2d-array" }
            },
            {
                // OutputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_MaxPool1_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_MaxPool1 === null || newHash !== this.Hash_Compute_MaxPool1)
        {
            this.Hash_Compute_MaxPool1 = newHash;

            let shaderCode = class_mnist.ShaderCode_MaxPool1_MaxPool1;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_MaxPool1Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_MaxPool1_Output_ReadOnly_format));

            this.ShaderModule_Compute_MaxPool1 = device.createShaderModule({ code: shaderCode, label: "Compute Shader MaxPool1"});
            this.BindGroupLayout_Compute_MaxPool1 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout MaxPool1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_MaxPool1 = device.createPipelineLayout({
                label: "Compute Pipeline Layout MaxPool1",
                bindGroupLayouts: [this.BindGroupLayout_Compute_MaxPool1],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_MaxPool1 = device.createComputePipeline({
                    label: "Compute Pipeline MaxPool1",
                    layout: this.PipelineLayout_Compute_MaxPool1,
                    compute: {
                        module: this.ShaderModule_Compute_MaxPool1,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("MaxPool1");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline MaxPool1",
                    layout: this.PipelineLayout_Compute_MaxPool1,
                    compute: {
                        module: this.ShaderModule_Compute_MaxPool1,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_MaxPool1 = handle; this.loadingPromises.delete("MaxPool1"); } );
            }
        }
    }

    // (Re)create compute shader Conv2
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_MaxPool1Output_format).sampleType }
            },
            {
                // MaxValues
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // Weights
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Bias
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Output
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Conv2Output_format), viewDimension: "2d-array" }
            },
            {
                // OutputReadOnly
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Conv2_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Conv2 === null || newHash !== this.Hash_Compute_Conv2)
        {
            this.Hash_Compute_Conv2 = newHash;

            let shaderCode = class_mnist.ShaderCode_Conv2_Conv2;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_Conv2Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Conv2_Output_ReadOnly_format));

            this.ShaderModule_Compute_Conv2 = device.createShaderModule({ code: shaderCode, label: "Compute Shader Conv2"});
            this.BindGroupLayout_Compute_Conv2 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Conv2",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Conv2 = device.createPipelineLayout({
                label: "Compute Pipeline Layout Conv2",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Conv2],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Conv2 = device.createComputePipeline({
                    label: "Compute Pipeline Conv2",
                    layout: this.PipelineLayout_Compute_Conv2,
                    compute: {
                        module: this.ShaderModule_Compute_Conv2,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Conv2");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Conv2",
                    layout: this.PipelineLayout_Compute_Conv2,
                    compute: {
                        module: this.ShaderModule_Compute_Conv2,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Conv2 = handle; this.loadingPromises.delete("Conv2"); } );
            }
        }
    }

    // (Re)create compute shader MaxPool2
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_Conv2Output_format).sampleType }
            },
            {
                // MaxValues
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // Output
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_MaxPool2Output_format), viewDimension: "2d-array" }
            },
            {
                // OutputReadOnly
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_MaxPool2_Output_ReadOnly_format), viewDimension: "2d-array" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_MaxPool2 === null || newHash !== this.Hash_Compute_MaxPool2)
        {
            this.Hash_Compute_MaxPool2 = newHash;

            let shaderCode = class_mnist.ShaderCode_MaxPool2_MaxPool2;
            shaderCode = shaderCode.replace("/*(Output_format)*/", Shared.GetNonSRGBFormat(this.texture_MaxPool2Output_format));
            shaderCode = shaderCode.replace("/*(OutputReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_MaxPool2_Output_ReadOnly_format));

            this.ShaderModule_Compute_MaxPool2 = device.createShaderModule({ code: shaderCode, label: "Compute Shader MaxPool2"});
            this.BindGroupLayout_Compute_MaxPool2 = device.createBindGroupLayout({
                label: "Compute Bind Group Layout MaxPool2",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_MaxPool2 = device.createPipelineLayout({
                label: "Compute Pipeline Layout MaxPool2",
                bindGroupLayouts: [this.BindGroupLayout_Compute_MaxPool2],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_MaxPool2 = device.createComputePipeline({
                    label: "Compute Pipeline MaxPool2",
                    layout: this.PipelineLayout_Compute_MaxPool2,
                    compute: {
                        module: this.ShaderModule_Compute_MaxPool2,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("MaxPool2");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline MaxPool2",
                    layout: this.PipelineLayout_Compute_MaxPool2,
                    compute: {
                        module: this.ShaderModule_Compute_MaxPool2,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_MaxPool2 = handle; this.loadingPromises.delete("MaxPool2"); } );
            }
        }
    }

    // (Re)create compute shader Linear
    {
        const bindGroupEntries =
        [
            {
                // Input
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_MaxPool2Output_format).sampleType }
            },
            {
                // MaxValues
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
            {
                // Weights
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Bias
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // Output
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Linear === null || newHash !== this.Hash_Compute_Linear)
        {
            this.Hash_Compute_Linear = newHash;

            let shaderCode = class_mnist.ShaderCode_Linear_Linear;

            this.ShaderModule_Compute_Linear = device.createShaderModule({ code: shaderCode, label: "Compute Shader Linear"});
            this.BindGroupLayout_Compute_Linear = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Linear",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Linear = device.createPipelineLayout({
                label: "Compute Pipeline Layout Linear",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Linear],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Linear = device.createComputePipeline({
                    label: "Compute Pipeline Linear",
                    layout: this.PipelineLayout_Compute_Linear,
                    compute: {
                        module: this.ShaderModule_Compute_Linear,
                        entryPoint: "main",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Linear");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Linear",
                    layout: this.PipelineLayout_Compute_Linear,
                    compute: {
                        module: this.ShaderModule_Compute_Linear,
                        entryPoint: "main",
                    }
                }).then( handle => { this.Pipeline_Compute_Linear = handle; this.loadingPromises.delete("Linear"); } );
            }
        }
    }

    // Load texture _loadedTexture_0 from "0.png"
    if (this.texture__loadedTexture_0 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/0.png", this.texture__loadedTexture_0_usageFlags, "2d");
        this.texture__loadedTexture_0 = loadedTex.texture;
        this.texture__loadedTexture_0_size = loadedTex.size;
        this.texture__loadedTexture_0_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_0_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_1 from "1.png"
    if (this.texture__loadedTexture_1 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/1.png", this.texture__loadedTexture_1_usageFlags, "2d");
        this.texture__loadedTexture_1 = loadedTex.texture;
        this.texture__loadedTexture_1_size = loadedTex.size;
        this.texture__loadedTexture_1_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_1_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_2 from "2.png"
    if (this.texture__loadedTexture_2 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/2.png", this.texture__loadedTexture_2_usageFlags, "2d");
        this.texture__loadedTexture_2 = loadedTex.texture;
        this.texture__loadedTexture_2_size = loadedTex.size;
        this.texture__loadedTexture_2_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_2_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_3 from "3.png"
    if (this.texture__loadedTexture_3 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/3.png", this.texture__loadedTexture_3_usageFlags, "2d");
        this.texture__loadedTexture_3 = loadedTex.texture;
        this.texture__loadedTexture_3_size = loadedTex.size;
        this.texture__loadedTexture_3_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_3_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_4 from "4.png"
    if (this.texture__loadedTexture_4 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/4.png", this.texture__loadedTexture_4_usageFlags, "2d");
        this.texture__loadedTexture_4 = loadedTex.texture;
        this.texture__loadedTexture_4_size = loadedTex.size;
        this.texture__loadedTexture_4_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_4_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_5 from "5.png"
    if (this.texture__loadedTexture_5 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/5.png", this.texture__loadedTexture_5_usageFlags, "2d");
        this.texture__loadedTexture_5 = loadedTex.texture;
        this.texture__loadedTexture_5_size = loadedTex.size;
        this.texture__loadedTexture_5_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_5_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_6 from "6.png"
    if (this.texture__loadedTexture_6 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/6.png", this.texture__loadedTexture_6_usageFlags, "2d");
        this.texture__loadedTexture_6 = loadedTex.texture;
        this.texture__loadedTexture_6_size = loadedTex.size;
        this.texture__loadedTexture_6_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_6_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_7 from "7.png"
    if (this.texture__loadedTexture_7 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/7.png", this.texture__loadedTexture_7_usageFlags, "2d");
        this.texture__loadedTexture_7 = loadedTex.texture;
        this.texture__loadedTexture_7_size = loadedTex.size;
        this.texture__loadedTexture_7_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_7_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_8 from "8.png"
    if (this.texture__loadedTexture_8 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/8.png", this.texture__loadedTexture_8_usageFlags, "2d");
        this.texture__loadedTexture_8 = loadedTex.texture;
        this.texture__loadedTexture_8_size = loadedTex.size;
        this.texture__loadedTexture_8_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_8_usageFlags = loadedTex.usageFlags;
    }
    // Load texture _loadedTexture_9 from "9.png"
    if (this.texture__loadedTexture_9 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/9.png", this.texture__loadedTexture_9_usageFlags, "2d");
        this.texture__loadedTexture_9 = loadedTex.texture;
        this.texture__loadedTexture_9_size = loadedTex.size;
        this.texture__loadedTexture_9_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_9_usageFlags = loadedTex.usageFlags;
    }
    // (Re)create compute shader Presentation
    {
        const bindGroupEntries =
        [
            {
                // DrawCanvas
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_Drawing_Canvas_format).sampleType }
            },
            {
                // NNInput
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_NN_Input_format).sampleType }
            },
            {
                // Conv1Output
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_Conv1Output_format).sampleType }
            },
            {
                // MaxPool1Output
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_MaxPool1Output_format).sampleType }
            },
            {
                // Conv2Output
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_Conv2Output_format).sampleType }
            },
            {
                // MaxPool2Output
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_MaxPool2Output_format).sampleType }
            },
            {
                // MaxValues
                binding: 6,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // LinearOutput
                binding: 7,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // PresentationCanvas
                binding: 8,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Presentation_Canvas_format), viewDimension: "2d" }
            },
            {
                // PresentationCanvasReadOnly
                binding: 9,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Presentation_PresentationCanvas_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _loadedTexture_0
                binding: 10,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
            {
                // _loadedTexture_1
                binding: 11,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_1_format).sampleType }
            },
            {
                // _loadedTexture_2
                binding: 12,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_2_format).sampleType }
            },
            {
                // _loadedTexture_3
                binding: 13,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_3_format).sampleType }
            },
            {
                // _loadedTexture_4
                binding: 14,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_4_format).sampleType }
            },
            {
                // _loadedTexture_5
                binding: 15,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_5_format).sampleType }
            },
            {
                // _loadedTexture_6
                binding: 16,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_6_format).sampleType }
            },
            {
                // _loadedTexture_7
                binding: 17,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_7_format).sampleType }
            },
            {
                // _loadedTexture_8
                binding: 18,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_8_format).sampleType }
            },
            {
                // _loadedTexture_9
                binding: 19,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_9_format).sampleType }
            },
            {
                // _PresentationCB
                binding: 20,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Presentation === null || newHash !== this.Hash_Compute_Presentation)
        {
            this.Hash_Compute_Presentation = newHash;

            let shaderCode = class_mnist.ShaderCode_Presentation_Presentation;
            shaderCode = shaderCode.replace("/*(PresentationCanvas_format)*/", Shared.GetNonSRGBFormat(this.texture_Presentation_Canvas_format));
            shaderCode = shaderCode.replace("/*(PresentationCanvasReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Presentation_PresentationCanvas_ReadOnly_format));

            this.ShaderModule_Compute_Presentation = device.createShaderModule({ code: shaderCode, label: "Compute Shader Presentation"});
            this.BindGroupLayout_Compute_Presentation = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Presentation",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Presentation = device.createPipelineLayout({
                label: "Compute Pipeline Layout Presentation",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Presentation],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Presentation = device.createComputePipeline({
                    label: "Compute Pipeline Presentation",
                    layout: this.PipelineLayout_Compute_Presentation,
                    compute: {
                        module: this.ShaderModule_Compute_Presentation,
                        entryPoint: "Presentation",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Presentation");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Presentation",
                    layout: this.PipelineLayout_Compute_Presentation,
                    compute: {
                        module: this.ShaderModule_Compute_Presentation,
                        entryPoint: "Presentation",
                    }
                }).then( handle => { this.Pipeline_Compute_Presentation = handle; this.loadingPromises.delete("Presentation"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("mnist.Drawing_Canvas");

    encoder.popDebugGroup(); // "mnist.Drawing_Canvas"

    encoder.pushDebugGroup("mnist.NN_Input");

    encoder.popDebugGroup(); // "mnist.NN_Input"

    encoder.pushDebugGroup("mnist.Presentation_Canvas");

    encoder.popDebugGroup(); // "mnist.Presentation_Canvas"

    encoder.pushDebugGroup("mnist.Imported_Image");

    encoder.popDebugGroup(); // "mnist.Imported_Image"

    encoder.pushDebugGroup("mnist.Draw_Extents");

    encoder.popDebugGroup(); // "mnist.Draw_Extents"

    encoder.pushDebugGroup("mnist.Conv1Weights");

    encoder.popDebugGroup(); // "mnist.Conv1Weights"

    encoder.pushDebugGroup("mnist.Conv1Bias");

    encoder.popDebugGroup(); // "mnist.Conv1Bias"

    encoder.pushDebugGroup("mnist.Conv2Weights");

    encoder.popDebugGroup(); // "mnist.Conv2Weights"

    encoder.pushDebugGroup("mnist.Conv2Bias");

    encoder.popDebugGroup(); // "mnist.Conv2Bias"

    encoder.pushDebugGroup("mnist.LinearWeights");

    encoder.popDebugGroup(); // "mnist.LinearWeights"

    encoder.pushDebugGroup("mnist.LinearBias");

    encoder.popDebugGroup(); // "mnist.LinearBias"

    encoder.pushDebugGroup("mnist.Conv1Output");

    encoder.popDebugGroup(); // "mnist.Conv1Output"

    encoder.pushDebugGroup("mnist.MaxPool1Output");

    encoder.popDebugGroup(); // "mnist.MaxPool1Output"

    encoder.pushDebugGroup("mnist.Conv2Output");

    encoder.popDebugGroup(); // "mnist.Conv2Output"

    encoder.pushDebugGroup("mnist.MaxPool2Output");

    encoder.popDebugGroup(); // "mnist.MaxPool2Output"

    encoder.pushDebugGroup("mnist.MaxValues");

    encoder.popDebugGroup(); // "mnist.MaxValues"

    encoder.pushDebugGroup("mnist.LinearOutput");

    encoder.popDebugGroup(); // "mnist.LinearOutput"

    encoder.pushDebugGroup("mnist.InitMaxValues");

    // Run compute shader InitMaxValues
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group InitMaxValues",
            layout: this.BindGroupLayout_Compute_InitMaxValues,
            entries: [
                {
                    // MaxValues
                    binding: 0,
                    resource: { buffer: this.buffer_MaxValues }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = [this.buffer_MaxValues_count, 1, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 2) + 0 + 64 - 1) / 64),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_InitMaxValues !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_InitMaxValues);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.InitMaxValues"

    encoder.pushDebugGroup("mnist.Draw_Canvas_ReadOnly");

    encoder.popDebugGroup(); // "mnist.Draw_Canvas_ReadOnly"

    encoder.pushDebugGroup("mnist.Copy_Draw_Canvas");

    // Copy texture Drawing_Canvas to texture Draw_Canvas_ReadOnly
    {
        const numMips = Math.min(this.texture_Drawing_Canvas.mipLevelCount, this.texture_Draw_Canvas_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Drawing_Canvas.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Drawing_Canvas.height >> mipIndex, 1);
            let mipDepth = this.texture_Drawing_Canvas.depthOrArrayLayers;

            if (this.texture_Drawing_Canvas.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Drawing_Canvas, mipLevel: mipIndex },
                { texture: this.texture_Draw_Canvas_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "mnist.Copy_Draw_Canvas"

    encoder.pushDebugGroup("mnist.Shrink_NNInput_ReadOnly");

    encoder.popDebugGroup(); // "mnist.Shrink_NNInput_ReadOnly"

    encoder.pushDebugGroup("mnist.Copy_Shrink_NNInput");

    // Copy texture NN_Input to texture Shrink_NNInput_ReadOnly
    {
        const numMips = Math.min(this.texture_NN_Input.mipLevelCount, this.texture_Shrink_NNInput_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_NN_Input.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_NN_Input.height >> mipIndex, 1);
            let mipDepth = this.texture_NN_Input.depthOrArrayLayers;

            if (this.texture_NN_Input.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_NN_Input, mipLevel: mipIndex },
                { texture: this.texture_Shrink_NNInput_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "mnist.Copy_Shrink_NNInput"

    encoder.pushDebugGroup("mnist.Presentation_PresentationCanvas_ReadOnly");

    encoder.popDebugGroup(); // "mnist.Presentation_PresentationCanvas_ReadOnly"

    encoder.pushDebugGroup("mnist.Copy_Presentation_PresentationCanvas");

    // Copy texture Presentation_Canvas to texture Presentation_PresentationCanvas_ReadOnly
    {
        const numMips = Math.min(this.texture_Presentation_Canvas.mipLevelCount, this.texture_Presentation_PresentationCanvas_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Presentation_Canvas.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Presentation_Canvas.height >> mipIndex, 1);
            let mipDepth = this.texture_Presentation_Canvas.depthOrArrayLayers;

            if (this.texture_Presentation_Canvas.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Presentation_Canvas, mipLevel: mipIndex },
                { texture: this.texture_Presentation_PresentationCanvas_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "mnist.Copy_Presentation_PresentationCanvas"

    encoder.pushDebugGroup("mnist.Conv1_Output_ReadOnly");

    encoder.popDebugGroup(); // "mnist.Conv1_Output_ReadOnly"

    encoder.pushDebugGroup("mnist.Copy_Conv1_Output");

    // Copy texture Conv1Output to texture Conv1_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Conv1Output.mipLevelCount, this.texture_Conv1_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Conv1Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Conv1Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Conv1Output.depthOrArrayLayers;

            if (this.texture_Conv1Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Conv1Output, mipLevel: mipIndex },
                { texture: this.texture_Conv1_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "mnist.Copy_Conv1_Output"

    encoder.pushDebugGroup("mnist.MaxPool1_Output_ReadOnly");

    encoder.popDebugGroup(); // "mnist.MaxPool1_Output_ReadOnly"

    encoder.pushDebugGroup("mnist.Copy_MaxPool1_Output");

    // Copy texture MaxPool1Output to texture MaxPool1_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_MaxPool1Output.mipLevelCount, this.texture_MaxPool1_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_MaxPool1Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_MaxPool1Output.height >> mipIndex, 1);
            let mipDepth = this.texture_MaxPool1Output.depthOrArrayLayers;

            if (this.texture_MaxPool1Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_MaxPool1Output, mipLevel: mipIndex },
                { texture: this.texture_MaxPool1_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "mnist.Copy_MaxPool1_Output"

    encoder.pushDebugGroup("mnist.Conv2_Output_ReadOnly");

    encoder.popDebugGroup(); // "mnist.Conv2_Output_ReadOnly"

    encoder.pushDebugGroup("mnist.Copy_Conv2_Output");

    // Copy texture Conv2Output to texture Conv2_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_Conv2Output.mipLevelCount, this.texture_Conv2_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Conv2Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Conv2Output.height >> mipIndex, 1);
            let mipDepth = this.texture_Conv2Output.depthOrArrayLayers;

            if (this.texture_Conv2Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Conv2Output, mipLevel: mipIndex },
                { texture: this.texture_Conv2_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "mnist.Copy_Conv2_Output"

    encoder.pushDebugGroup("mnist.MaxPool2_Output_ReadOnly");

    encoder.popDebugGroup(); // "mnist.MaxPool2_Output_ReadOnly"

    encoder.pushDebugGroup("mnist.Copy_MaxPool2_Output");

    // Copy texture MaxPool2Output to texture MaxPool2_Output_ReadOnly
    {
        const numMips = Math.min(this.texture_MaxPool2Output.mipLevelCount, this.texture_MaxPool2_Output_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_MaxPool2Output.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_MaxPool2Output.height >> mipIndex, 1);
            let mipDepth = this.texture_MaxPool2Output.depthOrArrayLayers;

            if (this.texture_MaxPool2Output.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_MaxPool2Output, mipLevel: mipIndex },
                { texture: this.texture_MaxPool2_Output_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "mnist.Copy_MaxPool2_Output"

    encoder.pushDebugGroup("mnist._DrawCB");

    // Create constant buffer _DrawCB
    if (this.constantBuffer__DrawCB === null)
    {
        this.constantBuffer__DrawCB = device.createBuffer({
            label: "mnist._DrawCB",
            size: Shared.Align(16, this.constructor.StructOffsets__DrawCB._size),
            usage: this.constantBuffer__DrawCB_usageFlags,
        });
    }

    // Upload values to constant buffer _DrawCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__DrawCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__DrawCB.Clear, (this.variable_Clear === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseState_0, this.variable_MouseState[0], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseState_1, this.variable_MouseState[1], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseState_2, this.variable_MouseState[2], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseState_3, this.variable_MouseState[3], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseStateLastFrame_0, this.variable_MouseStateLastFrame[0], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseStateLastFrame_1, this.variable_MouseStateLastFrame[1], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseStateLastFrame_2, this.variable_MouseStateLastFrame[2], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.MouseStateLastFrame_3, this.variable_MouseStateLastFrame[3], true);
        view.setFloat32(this.constructor.StructOffsets__DrawCB.PenSize, this.variable_PenSize, true);
        view.setUint32(this.constructor.StructOffsets__DrawCB.UseImportedImage, (this.variable_UseImportedImage === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__DrawCB.iFrame, this.variable_iFrame, true);
        device.queue.writeBuffer(this.constantBuffer__DrawCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "mnist._DrawCB"

    encoder.pushDebugGroup("mnist.Draw");

    // Run compute shader Draw
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Draw",
            layout: this.BindGroupLayout_Compute_Draw,
            entries: [
                {
                    // Canvas
                    binding: 0,
                    resource: this.texture_Drawing_Canvas.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // DrawExtents
                    binding: 1,
                    resource: { buffer: this.buffer_Draw_Extents }
                },
                {
                    // CanvasReadOnly
                    binding: 2,
                    resource: this.texture_Draw_Canvas_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _DrawCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__DrawCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Drawing_Canvas_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Draw !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Draw);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Draw"

    encoder.pushDebugGroup("mnist.CalculateExtents");

    // Run compute shader CalculateExtents
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group CalculateExtents",
            layout: this.BindGroupLayout_Compute_CalculateExtents,
            entries: [
                {
                    // Canvas
                    binding: 0,
                    resource: this.texture_Drawing_Canvas.createView({ dimension: "2d", format: this.texture_Drawing_Canvas_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // DrawExtents
                    binding: 1,
                    resource: { buffer: this.buffer_Draw_Extents }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Drawing_Canvas_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_CalculateExtents !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_CalculateExtents);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.CalculateExtents"

    encoder.pushDebugGroup("mnist._ShrinkCB");

    // Create constant buffer _ShrinkCB
    if (this.constantBuffer__ShrinkCB === null)
    {
        this.constantBuffer__ShrinkCB = device.createBuffer({
            label: "mnist._ShrinkCB",
            size: Shared.Align(16, this.constructor.StructOffsets__ShrinkCB._size),
            usage: this.constantBuffer__ShrinkCB_usageFlags,
        });
    }

    // Upload values to constant buffer _ShrinkCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__ShrinkCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__ShrinkCB.NormalizeDrawing, (this.variable_NormalizeDrawing === true ? 1 : 0), true);
        view.setUint32(this.constructor.StructOffsets__ShrinkCB.UseImportedImage, (this.variable_UseImportedImage === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__ShrinkCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "mnist._ShrinkCB"

    encoder.pushDebugGroup("mnist.Shrink");

    // Run compute shader Shrink
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Shrink",
            layout: this.BindGroupLayout_Compute_Shrink,
            entries: [
                {
                    // Canvas
                    binding: 0,
                    resource: this.texture_Drawing_Canvas.createView({ dimension: "2d", format: this.texture_Drawing_Canvas_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // DrawExtents
                    binding: 1,
                    resource: { buffer: this.buffer_Draw_Extents }
                },
                {
                    // NNInput
                    binding: 2,
                    resource: this.texture_NN_Input.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // ImportedImage
                    binding: 3,
                    resource: this.texture_Imported_Image.createView({ dimension: "2d", format: this.texture_Imported_Image_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // NNInputReadOnly
                    binding: 4,
                    resource: this.texture_Shrink_NNInput_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _ShrinkCB
                    binding: 5,
                    resource: { buffer: this.constantBuffer__ShrinkCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_NN_Input_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Shrink !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Shrink);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Shrink"

    encoder.pushDebugGroup("mnist.Conv1");

    // Run compute shader Conv1
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Conv1",
            layout: this.BindGroupLayout_Compute_Conv1,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_NN_Input.createView({ dimension: "2d", format: this.texture_NN_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Weights
                    binding: 1,
                    resource: { buffer: this.buffer_Conv1Weights }
                },
                {
                    // Bias
                    binding: 2,
                    resource: { buffer: this.buffer_Conv1Bias }
                },
                {
                    // Output
                    binding: 3,
                    resource: this.texture_Conv1Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // MaxValues
                    binding: 4,
                    resource: { buffer: this.buffer_MaxValues }
                },
                {
                    // OutputReadOnly
                    binding: 5,
                    resource: this.texture_Conv1_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Conv1Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Conv1 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Conv1);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Conv1"

    encoder.pushDebugGroup("mnist.MaxPool1");

    // Run compute shader MaxPool1
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group MaxPool1",
            layout: this.BindGroupLayout_Compute_MaxPool1,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Conv1Output.createView({ dimension: "2d-array", format: this.texture_Conv1Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // MaxValues
                    binding: 1,
                    resource: { buffer: this.buffer_MaxValues }
                },
                {
                    // Output
                    binding: 2,
                    resource: this.texture_MaxPool1Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 3,
                    resource: this.texture_MaxPool1_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_MaxPool1Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_MaxPool1 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_MaxPool1);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.MaxPool1"

    encoder.pushDebugGroup("mnist.Conv2");

    // Run compute shader Conv2
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Conv2",
            layout: this.BindGroupLayout_Compute_Conv2,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_MaxPool1Output.createView({ dimension: "2d-array", format: this.texture_MaxPool1Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // MaxValues
                    binding: 1,
                    resource: { buffer: this.buffer_MaxValues }
                },
                {
                    // Weights
                    binding: 2,
                    resource: { buffer: this.buffer_Conv2Weights }
                },
                {
                    // Bias
                    binding: 3,
                    resource: { buffer: this.buffer_Conv2Bias }
                },
                {
                    // Output
                    binding: 4,
                    resource: this.texture_Conv2Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 5,
                    resource: this.texture_Conv2_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Conv2Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Conv2 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Conv2);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Conv2"

    encoder.pushDebugGroup("mnist.MaxPool2");

    // Run compute shader MaxPool2
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group MaxPool2",
            layout: this.BindGroupLayout_Compute_MaxPool2,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_Conv2Output.createView({ dimension: "2d-array", format: this.texture_Conv2Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // MaxValues
                    binding: 1,
                    resource: { buffer: this.buffer_MaxValues }
                },
                {
                    // Output
                    binding: 2,
                    resource: this.texture_MaxPool2Output.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // OutputReadOnly
                    binding: 3,
                    resource: this.texture_MaxPool2_Output_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_MaxPool2Output_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_MaxPool2 !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_MaxPool2);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.MaxPool2"

    encoder.pushDebugGroup("mnist.Linear");

    // Run compute shader Linear
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Linear",
            layout: this.BindGroupLayout_Compute_Linear,
            entries: [
                {
                    // Input
                    binding: 0,
                    resource: this.texture_MaxPool2Output.createView({ dimension: "2d-array", format: this.texture_MaxPool2Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // MaxValues
                    binding: 1,
                    resource: { buffer: this.buffer_MaxValues }
                },
                {
                    // Weights
                    binding: 2,
                    resource: { buffer: this.buffer_LinearWeights }
                },
                {
                    // Bias
                    binding: 3,
                    resource: { buffer: this.buffer_LinearBias }
                },
                {
                    // Output
                    binding: 4,
                    resource: { buffer: this.buffer_LinearOutput }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = [this.buffer_LinearOutput_count, 1, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 64 - 1) / 64),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Linear !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Linear);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Linear"

    encoder.pushDebugGroup("mnist._loadedTexture_0");

    encoder.popDebugGroup(); // "mnist._loadedTexture_0"

    encoder.pushDebugGroup("mnist._loadedTexture_1");

    encoder.popDebugGroup(); // "mnist._loadedTexture_1"

    encoder.pushDebugGroup("mnist._loadedTexture_2");

    encoder.popDebugGroup(); // "mnist._loadedTexture_2"

    encoder.pushDebugGroup("mnist._loadedTexture_3");

    encoder.popDebugGroup(); // "mnist._loadedTexture_3"

    encoder.pushDebugGroup("mnist._loadedTexture_4");

    encoder.popDebugGroup(); // "mnist._loadedTexture_4"

    encoder.pushDebugGroup("mnist._loadedTexture_5");

    encoder.popDebugGroup(); // "mnist._loadedTexture_5"

    encoder.pushDebugGroup("mnist._loadedTexture_6");

    encoder.popDebugGroup(); // "mnist._loadedTexture_6"

    encoder.pushDebugGroup("mnist._loadedTexture_7");

    encoder.popDebugGroup(); // "mnist._loadedTexture_7"

    encoder.pushDebugGroup("mnist._loadedTexture_8");

    encoder.popDebugGroup(); // "mnist._loadedTexture_8"

    encoder.pushDebugGroup("mnist._loadedTexture_9");

    encoder.popDebugGroup(); // "mnist._loadedTexture_9"

    encoder.pushDebugGroup("mnist._PresentationCB");

    // Create constant buffer _PresentationCB
    if (this.constantBuffer__PresentationCB === null)
    {
        this.constantBuffer__PresentationCB = device.createBuffer({
            label: "mnist._PresentationCB",
            size: Shared.Align(16, this.constructor.StructOffsets__PresentationCB._size),
            usage: this.constantBuffer__PresentationCB_usageFlags,
        });
    }

    // Upload values to constant buffer _PresentationCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__PresentationCB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__PresentationCB.Conv1OutputScale, this.variable_Conv1OutputScale, true);
        view.setInt32(this.constructor.StructOffsets__PresentationCB.Conv2OutputScale, this.variable_Conv2OutputScale, true);
        view.setInt32(this.constructor.StructOffsets__PresentationCB.MaxPool1OutputScale, this.variable_MaxPool1OutputScale, true);
        view.setInt32(this.constructor.StructOffsets__PresentationCB.MaxPool2OutputScale, this.variable_MaxPool2OutputScale, true);
        view.setFloat32(this.constructor.StructOffsets__PresentationCB.MouseState_0, this.variable_MouseState[0], true);
        view.setFloat32(this.constructor.StructOffsets__PresentationCB.MouseState_1, this.variable_MouseState[1], true);
        view.setFloat32(this.constructor.StructOffsets__PresentationCB.MouseState_2, this.variable_MouseState[2], true);
        view.setFloat32(this.constructor.StructOffsets__PresentationCB.MouseState_3, this.variable_MouseState[3], true);
        view.setFloat32(this.constructor.StructOffsets__PresentationCB.PenSize, this.variable_PenSize, true);
        view.setUint32(this.constructor.StructOffsets__PresentationCB.UseImportedImage, (this.variable_UseImportedImage === true ? 1 : 0), true);
        device.queue.writeBuffer(this.constantBuffer__PresentationCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "mnist._PresentationCB"

    encoder.pushDebugGroup("mnist.Presentation");

    // Run compute shader Presentation
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Presentation",
            layout: this.BindGroupLayout_Compute_Presentation,
            entries: [
                {
                    // DrawCanvas
                    binding: 0,
                    resource: this.texture_Drawing_Canvas.createView({ dimension: "2d", format: this.texture_Drawing_Canvas_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // NNInput
                    binding: 1,
                    resource: this.texture_NN_Input.createView({ dimension: "2d", format: this.texture_NN_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Conv1Output
                    binding: 2,
                    resource: this.texture_Conv1Output.createView({ dimension: "2d-array", format: this.texture_Conv1Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // MaxPool1Output
                    binding: 3,
                    resource: this.texture_MaxPool1Output.createView({ dimension: "2d-array", format: this.texture_MaxPool1Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Conv2Output
                    binding: 4,
                    resource: this.texture_Conv2Output.createView({ dimension: "2d-array", format: this.texture_Conv2Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // MaxPool2Output
                    binding: 5,
                    resource: this.texture_MaxPool2Output.createView({ dimension: "2d-array", format: this.texture_MaxPool2Output_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // MaxValues
                    binding: 6,
                    resource: { buffer: this.buffer_MaxValues }
                },
                {
                    // LinearOutput
                    binding: 7,
                    resource: { buffer: this.buffer_LinearOutput }
                },
                {
                    // PresentationCanvas
                    binding: 8,
                    resource: this.texture_Presentation_Canvas.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // PresentationCanvasReadOnly
                    binding: 9,
                    resource: this.texture_Presentation_PresentationCanvas_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 10,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_1
                    binding: 11,
                    resource: this.texture__loadedTexture_1.createView({ dimension: "2d", format: this.texture__loadedTexture_1_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_2
                    binding: 12,
                    resource: this.texture__loadedTexture_2.createView({ dimension: "2d", format: this.texture__loadedTexture_2_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_3
                    binding: 13,
                    resource: this.texture__loadedTexture_3.createView({ dimension: "2d", format: this.texture__loadedTexture_3_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_4
                    binding: 14,
                    resource: this.texture__loadedTexture_4.createView({ dimension: "2d", format: this.texture__loadedTexture_4_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_5
                    binding: 15,
                    resource: this.texture__loadedTexture_5.createView({ dimension: "2d", format: this.texture__loadedTexture_5_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_6
                    binding: 16,
                    resource: this.texture__loadedTexture_6.createView({ dimension: "2d", format: this.texture__loadedTexture_6_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_7
                    binding: 17,
                    resource: this.texture__loadedTexture_7.createView({ dimension: "2d", format: this.texture__loadedTexture_7_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_8
                    binding: 18,
                    resource: this.texture__loadedTexture_8.createView({ dimension: "2d", format: this.texture__loadedTexture_8_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_9
                    binding: 19,
                    resource: this.texture__loadedTexture_9.createView({ dimension: "2d", format: this.texture__loadedTexture_9_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _PresentationCB
                    binding: 20,
                    resource: { buffer: this.constantBuffer__PresentationCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Presentation_Canvas_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Presentation !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Presentation);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Presentation"

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

var mnist = new class_mnist;

export default mnist;
