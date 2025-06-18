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

// Shader code for Compute shader "Hidden_Layer", node "Hidden_Layer"
static ShaderCode_Hidden_Layer_Hidden_Layer = `
@binding(1) @group(0) var<storage, read> NNWeights : array<f32>;

@binding(0) @group(0) var NNInput : texture_2d<f32>;

@binding(2) @group(0) var<storage, read_write> HiddenLayerActivations : array<f32>;

@compute
@workgroup_size(64, 1, 1)
fn HiddenLayer(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var hiddenNeuronIndex_0 : i32 = i32(DTid_0.x);
    var weightsBeginIndex_0 : i32 = hiddenNeuronIndex_0 * i32(785);
    var _S1 : f32 = NNWeights[weightsBeginIndex_0 + i32(784)];
    var inputNeuronIndex_0 : i32 = i32(0);
    var output_0 : f32 = _S1;
    for(;;)
    {
        if(inputNeuronIndex_0 < i32(784))
        {
        }
        else
        {
            break;
        }
        var _S2 : u32 = u32(inputNeuronIndex_0);
        var _S3 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(_S2 % u32(28), _S2 / u32(28)), u32(0)));
        var output_1 : f32 = output_0 + (textureLoad((NNInput), ((_S3)).xy, ((_S3)).z).x) * NNWeights[weightsBeginIndex_0 + inputNeuronIndex_0];
        inputNeuronIndex_0 = inputNeuronIndex_0 + i32(1);
        output_0 = output_1;
    }
    HiddenLayerActivations[hiddenNeuronIndex_0] = 1.0f / (1.0f + exp(- output_0));
    return;
}

`;

// Shader code for Compute shader "Output_Layer", node "Output_Layer"
static ShaderCode_Output_Layer_Output_Layer = `
@binding(0) @group(0) var<storage, read> NNWeights : array<f32>;

@binding(1) @group(0) var<storage, read> HiddenLayerActivations : array<f32>;

@binding(2) @group(0) var<storage, read_write> OutputLayerActivations : array<f32>;

@compute
@workgroup_size(64, 1, 1)
fn OutputLayer(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var outputNeuronIndex_0 : i32 = i32(DTid_0.x);
    var weightsBeginIndex_0 : i32 = i32(23550) + outputNeuronIndex_0 * i32(31);
    var _S1 : f32 = NNWeights[weightsBeginIndex_0 + i32(30)];
    var hiddenNeuronIndex_0 : i32 = i32(0);
    var output_0 : f32 = _S1;
    for(;;)
    {
        if(hiddenNeuronIndex_0 < i32(30))
        {
        }
        else
        {
            break;
        }
        var output_1 : f32 = output_0 + HiddenLayerActivations[hiddenNeuronIndex_0] * NNWeights[weightsBeginIndex_0 + hiddenNeuronIndex_0];
        hiddenNeuronIndex_0 = hiddenNeuronIndex_0 + i32(1);
        output_0 = output_1;
    }
    OutputLayerActivations[outputNeuronIndex_0] = 1.0f / (1.0f + exp(- output_0));
    return;
}

`;

// Shader code for Compute shader "Presentation", node "Presentation"
static ShaderCode_Presentation_Presentation = `
struct Struct_PresentationCB_std140_0
{
    @align(16) MouseState_0 : vec4<f32>,
    @align(16) PenSize_0 : f32,
    @align(4) UseImportedImage_0 : u32,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(17) @group(0) var<uniform> _PresentationCB : Struct_PresentationCB_std140_0;
@binding(0) @group(0) var DrawCanvas : texture_2d<f32>;

@binding(1) @group(0) var NNInput : texture_2d<f32>;

@binding(4) @group(0) var PresentationCanvas : texture_storage_2d</*(PresentationCanvas_format)*/, write>;

@binding(2) @group(0) var<storage, read> HiddenLayerActivations : array<f32>;

@binding(3) @group(0) var<storage, read> OutputLayerActivations : array<f32>;

@binding(6) @group(0) var _loadedTexture_0 : texture_2d<f32>;

@binding(7) @group(0) var _loadedTexture_1 : texture_2d<f32>;

@binding(8) @group(0) var _loadedTexture_2 : texture_2d<f32>;

@binding(9) @group(0) var _loadedTexture_3 : texture_2d<f32>;

@binding(10) @group(0) var _loadedTexture_4 : texture_2d<f32>;

@binding(11) @group(0) var _loadedTexture_5 : texture_2d<f32>;

@binding(12) @group(0) var _loadedTexture_6 : texture_2d<f32>;

@binding(13) @group(0) var _loadedTexture_7 : texture_2d<f32>;

@binding(14) @group(0) var _loadedTexture_8 : texture_2d<f32>;

@binding(15) @group(0) var _loadedTexture_9 : texture_2d<f32>;

@binding(16) @group(0) var _loadedTexture_10 : texture_2d<f32>;

@compute
@workgroup_size(8, 8, 1)
fn Presentation(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    const c_inputPanelPos_0 : vec2<i32> = vec2<i32>(i32(302), i32(30));
    const c_hiddenPanelPos_0 : vec2<i32> = vec2<i32>(i32(346), i32(30));
    const c_outputPanelPos_0 : vec2<i32> = vec2<i32>(i32(390), i32(30));
    const c_outputLabelsPos_0 : vec2<i32> = vec2<i32>(i32(424), i32(30));
    const c_instructionsPos_0 : vec2<i32> = vec2<i32>(i32(30), i32(322));
    const c_borderColor_0 : vec4<f32> = vec4<f32>(0.80000001192092896f, 0.80000001192092896f, 0.0f, 1.0f);
    const c_backgroundColor_0 : vec4<f32> = vec4<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f, 1.0f);
    var _S1 : vec2<u32> = DTid_0.xy;
    var _S2 : vec2<i32> = vec2<i32>(_S1);
    var relPos_0 : vec2<i32> = _S2 - vec2<i32>(i32(30), i32(30));
    var _S3 : i32 = relPos_0.x;
    var _S4 : bool;
    if(_S3 >= i32(0))
    {
        _S4 = (relPos_0.y) >= i32(0);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = _S3 < i32(256);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_0.y) < i32(256);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        var mouse_0 : vec4<f32> = _PresentationCB.MouseState_0;
        var color_0 : vec3<f32>;
        if(bool(u32(!bool(_PresentationCB.UseImportedImage_0))))
        {
            var _S5 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_0), u32(0)));
            color_0 = vec3<f32>(0.0f, (textureLoad((DrawCanvas), ((_S5)).xy, ((_S5)).z).x), 0.0f);
        }
        else
        {
            var _S6 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(vec2<i32>(vec2<f32>(relPos_0) * vec2<f32>(vec2<u32>(u32(28), u32(28))) / vec2<f32>(vec2<u32>(u32(256), u32(256))))), u32(0)));
            var _S7 : f32 = (textureLoad((NNInput), ((_S6)).xy, ((_S6)).z).x);
            color_0 = vec3<f32>(_S7, _S7, 0.0f);
        }
        if((length(mouse_0.xy - vec2<f32>(_S1))) < (_PresentationCB.PenSize_0))
        {
            color_0 = mix(color_0, vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(0.15000000596046448f, 0.15000000596046448f, 0.15000000596046448f));
        }
        else
        {
        }
        textureStore((PresentationCanvas), (_S1), (vec4<f32>(color_0, 1.0f)));
        return;
    }
    if(_S3 >= i32(-3))
    {
        _S4 = (relPos_0.y) >= i32(-3);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = _S3 < i32(259);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_0.y) < i32(259);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        textureStore((PresentationCanvas), (_S1), (c_borderColor_0));
        return;
    }
    var relPos_1 : vec2<i32> = _S2 - c_inputPanelPos_0;
    var _S8 : i32 = relPos_1.x;
    if(_S8 >= i32(0))
    {
        _S4 = (relPos_1.y) >= i32(0);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = _S8 < i32(28);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_1.y) < i32(28);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        var _S9 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_1), u32(0)));
        textureStore((PresentationCanvas), (_S1), (vec4<f32>(vec3<f32>((textureLoad((NNInput), ((_S9)).xy, ((_S9)).z).x)), 1.0f)));
        return;
    }
    if(_S8 >= i32(-3))
    {
        _S4 = (relPos_1.y) >= i32(-3);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = _S8 < i32(31);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_1.y) < i32(31);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        textureStore((PresentationCanvas), (_S1), (c_borderColor_0));
        return;
    }
    var _S10 : vec2<i32> = _S2 - c_hiddenPanelPos_0;
    var relPos_2 : vec2<i32> = _S10;
    if((_S10.x) >= i32(0))
    {
        _S4 = (relPos_2.y) >= i32(0);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_2.x) < i32(28);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_2.y) < i32(927);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        if((relPos_2.y % i32(31)) >= i32(28))
        {
            textureStore((PresentationCanvas), (_S1), (c_borderColor_0));
            return;
        }
        relPos_2[i32(1)] = relPos_2[i32(1)] / i32(31);
        textureStore((PresentationCanvas), (_S1), (vec4<f32>(vec3<f32>(HiddenLayerActivations[relPos_2.y]), 1.0f)));
        return;
    }
    if((relPos_2.x) >= i32(-3))
    {
        _S4 = (relPos_2.y) >= i32(-3);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_2.x) < i32(31);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_2.y) < i32(930);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        textureStore((PresentationCanvas), (_S1), (c_borderColor_0));
        return;
    }
    var _S11 : vec2<i32> = _S2 - c_outputPanelPos_0;
    var relPos_3 : vec2<i32> = _S11;
    if((_S11.x) >= i32(0))
    {
        _S4 = (relPos_3.y) >= i32(0);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_3.x) < i32(28);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_3.y) < i32(307);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        if((relPos_3.y % i32(31)) >= i32(28))
        {
            textureStore((PresentationCanvas), (_S1), (c_borderColor_0));
            return;
        }
        relPos_3[i32(1)] = relPos_3[i32(1)] / i32(31);
        textureStore((PresentationCanvas), (_S1), (vec4<f32>(vec3<f32>(OutputLayerActivations[relPos_3.y]), 1.0f)));
        return;
    }
    if((relPos_3.x) >= i32(-3))
    {
        _S4 = (relPos_3.y) >= i32(-3);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_3.x) < i32(31);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_3.y) < i32(310);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        textureStore((PresentationCanvas), (_S1), (c_borderColor_0));
        return;
    }
    var _S12 : vec2<i32> = _S2 - c_outputLabelsPos_0;
    var relPos_4 : vec2<i32> = _S12;
    if((_S12.x) >= i32(0))
    {
        _S4 = (relPos_4.y) >= i32(0);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_4.x) < i32(28);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_4.y) < i32(307);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        if((relPos_4.y % i32(31)) < i32(28))
        {
            var index_0 : i32 = relPos_4.y / i32(31);
            relPos_4[i32(1)] = relPos_4.y % i32(31);
            var alpha_0 : f32;
            switch(index_0)
            {
            case i32(0), :
                {
                    var _S13 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_0), ((_S13)).xy, ((_S13)).z).x);
                    break;
                }
            case i32(1), :
                {
                    var _S14 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_1), ((_S14)).xy, ((_S14)).z).x);
                    break;
                }
            case i32(2), :
                {
                    var _S15 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_2), ((_S15)).xy, ((_S15)).z).x);
                    break;
                }
            case i32(3), :
                {
                    var _S16 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_3), ((_S16)).xy, ((_S16)).z).x);
                    break;
                }
            case i32(4), :
                {
                    var _S17 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_4), ((_S17)).xy, ((_S17)).z).x);
                    break;
                }
            case i32(5), :
                {
                    var _S18 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_5), ((_S18)).xy, ((_S18)).z).x);
                    break;
                }
            case i32(6), :
                {
                    var _S19 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_6), ((_S19)).xy, ((_S19)).z).x);
                    break;
                }
            case i32(7), :
                {
                    var _S20 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_7), ((_S20)).xy, ((_S20)).z).x);
                    break;
                }
            case i32(8), :
                {
                    var _S21 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_8), ((_S21)).xy, ((_S21)).z).x);
                    break;
                }
            case i32(9), :
                {
                    var _S22 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_4), u32(0)));
                    alpha_0 = (textureLoad((_loadedTexture_9), ((_S22)).xy, ((_S22)).z).x);
                    break;
                }
            case default, :
                {
                    alpha_0 = 0.0f;
                    break;
                }
            }
            if(alpha_0 > 0.0f)
            {
                textureStore((PresentationCanvas), (_S1), (vec4<f32>(mix(vec3<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f), mix(vec3<f32>(0.40000000596046448f, 0.0f, 0.0f), vec3<f32>(1.0f, 1.0f, 0.0f), vec3<f32>(OutputLayerActivations[index_0])), vec3<f32>(alpha_0)), 1.0f)));
                return;
            }
        }
    }
    var relPos_5 : vec2<i32> = _S2 - c_instructionsPos_0;
    var _S23 : i32 = relPos_5.x;
    if(_S23 >= i32(0))
    {
        _S4 = (relPos_5.y) >= i32(0);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = _S23 < i32(290);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_5.y) < i32(85);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        var _S24 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(relPos_5.xy), u32(0)));
        textureStore((PresentationCanvas), (_S1), (vec4<f32>(vec3<f32>((textureLoad((_loadedTexture_10), ((_S24)).xy, ((_S24)).z).x)), 1.0f)));
        return;
    }
    if(_S23 >= i32(-3))
    {
        _S4 = (relPos_5.y) >= i32(-3);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = _S23 < i32(293);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        _S4 = (relPos_5.y) < i32(88);
    }
    else
    {
        _S4 = false;
    }
    if(_S4)
    {
        textureStore((PresentationCanvas), (_S1), (c_borderColor_0));
        return;
    }
    textureStore((PresentationCanvas), (_S1), (c_backgroundColor_0));
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

// Buffer Hidden_Layer_Activations
buffer_Hidden_Layer_Activations = null;
buffer_Hidden_Layer_Activations_count = 0;
buffer_Hidden_Layer_Activations_stride = 0;
buffer_Hidden_Layer_Activations_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer Output_Layer_Activations
buffer_Output_Layer_Activations = null;
buffer_Output_Layer_Activations_count = 0;
buffer_Output_Layer_Activations_stride = 0;
buffer_Output_Layer_Activations_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer Draw_Extents
buffer_Draw_Extents = null;
buffer_Draw_Extents_count = 0;
buffer_Draw_Extents_stride = 0;
buffer_Draw_Extents_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

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

// Compute Shader Hidden_Layer
Hash_Compute_Hidden_Layer = 0;
ShaderModule_Compute_Hidden_Layer = null;
BindGroupLayout_Compute_Hidden_Layer = null;
PipelineLayout_Compute_Hidden_Layer = null;
Pipeline_Compute_Hidden_Layer = null;

// Compute Shader Output_Layer
Hash_Compute_Output_Layer = 0;
ShaderModule_Compute_Output_Layer = null;
BindGroupLayout_Compute_Output_Layer = null;
PipelineLayout_Compute_Output_Layer = null;
Pipeline_Compute_Output_Layer = null;

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

// Texture _loadedTexture_10
texture__loadedTexture_10 = null;
texture__loadedTexture_10_size = [0, 0, 0];
texture__loadedTexture_10_format = "";
texture__loadedTexture_10_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Constant buffer _PresentationCB
constantBuffer__PresentationCB = null;
constantBuffer__PresentationCB_size = 32;
constantBuffer__PresentationCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Presentation
Hash_Compute_Presentation = 0;
ShaderModule_Compute_Presentation = null;
BindGroupLayout_Compute_Presentation = null;
PipelineLayout_Compute_Presentation = null;
Pipeline_Compute_Presentation = null;

// -------------------- Imported Members

// Buffer NN_Weights
buffer_NN_Weights = null;
buffer_NN_Weights_count = 0;
buffer_NN_Weights_stride = 0;
buffer_NN_Weights_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Texture Imported_Image
texture_Imported_Image = null;
texture_Imported_Image_size = [0, 0, 0];
texture_Imported_Image_format = "";
texture_Imported_Image_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

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

// -------------------- Private Variables

variable_c_numInputNeurons = 784;
variableDefault_c_numInputNeurons = 784;
variableChanged_c_numInputNeurons = false;
variable_c_numHiddenNeurons = 30;
variableDefault_c_numHiddenNeurons = 30;
variableChanged_c_numHiddenNeurons = false;
variable_c_numOutputNeurons = 10;
variableDefault_c_numOutputNeurons = 10;
variableChanged_c_numOutputNeurons = false;
variable_c_numHiddenWeights = 23550;  // (c_numInputNeurons + 1) * c_numHiddenNeurons
variableDefault_c_numHiddenWeights = 23550;  // (c_numInputNeurons + 1) * c_numHiddenNeurons
variableChanged_c_numHiddenWeights = false;
variable_c_numOutputWeights = 310;  // (c_numHiddenNeurons + 1) * c_numOutputNeurons
variableDefault_c_numOutputWeights = 310;  // (c_numHiddenNeurons + 1) * c_numOutputNeurons
variableChanged_c_numOutputWeights = false;
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
    MouseState_0: 0,
    MouseState_1: 4,
    MouseState_2: 8,
    MouseState_3: 12,
    PenSize: 16,
    UseImportedImage: 20,
    _padding0: 24,
    _padding1: 28,
    _size: 32,
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
    // Validate buffer NN_Weights
    if (this.buffer_NN_Weights === null)
    {
        Shared.LogError("Imported resource buffer_NN_Weights was not provided");
        return false;
    }

    // Validate texture Imported_Image
    if (this.texture_Imported_Image === null)
    {
        Shared.LogError("Imported resource texture_Imported_Image was not provided");
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

    // Handle (re)creation of buffer Hidden_Layer_Activations
    {
        const baseCount = this.variable_c_numHiddenNeurons;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = 4;
        if (this.buffer_Hidden_Layer_Activations !== null && (this.buffer_Hidden_Layer_Activations_count != desiredCount || this.buffer_Hidden_Layer_Activations_stride != desiredStride))
        {
            this.buffer_Hidden_Layer_Activations.destroy();
            this.buffer_Hidden_Layer_Activations = null;
        }

        if (this.buffer_Hidden_Layer_Activations === null)
        {
            this.buffer_Hidden_Layer_Activations_count = desiredCount;
            this.buffer_Hidden_Layer_Activations_stride = desiredStride;
            this.buffer_Hidden_Layer_Activations = device.createBuffer({
                label: "buffer mnist.Hidden_Layer_Activations",
                size: Shared.Align(16, this.buffer_Hidden_Layer_Activations_count * this.buffer_Hidden_Layer_Activations_stride),
                usage: this.buffer_Hidden_Layer_Activations_usageFlags,
            });
        }
    }

    // Handle (re)creation of buffer Output_Layer_Activations
    {
        const baseCount = this.variable_c_numOutputNeurons;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = 4;
        if (this.buffer_Output_Layer_Activations !== null && (this.buffer_Output_Layer_Activations_count != desiredCount || this.buffer_Output_Layer_Activations_stride != desiredStride))
        {
            this.buffer_Output_Layer_Activations.destroy();
            this.buffer_Output_Layer_Activations = null;
        }

        if (this.buffer_Output_Layer_Activations === null)
        {
            this.buffer_Output_Layer_Activations_count = desiredCount;
            this.buffer_Output_Layer_Activations_stride = desiredStride;
            this.buffer_Output_Layer_Activations = device.createBuffer({
                label: "buffer mnist.Output_Layer_Activations",
                size: Shared.Align(16, this.buffer_Output_Layer_Activations_count * this.buffer_Output_Layer_Activations_stride),
                usage: this.buffer_Output_Layer_Activations_usageFlags,
            });
        }
    }

    // Handle (re)creation of texture Presentation_Canvas
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 480) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 990) / 1) + 0,
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

    // (Re)create compute shader Hidden_Layer
    {
        const bindGroupEntries =
        [
            {
                // NNInput
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture_NN_Input_format).sampleType }
            },
            {
                // NNWeights
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // HiddenLayerActivations
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Hidden_Layer === null || newHash !== this.Hash_Compute_Hidden_Layer)
        {
            this.Hash_Compute_Hidden_Layer = newHash;

            let shaderCode = class_mnist.ShaderCode_Hidden_Layer_Hidden_Layer;

            this.ShaderModule_Compute_Hidden_Layer = device.createShaderModule({ code: shaderCode, label: "Compute Shader Hidden_Layer"});
            this.BindGroupLayout_Compute_Hidden_Layer = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Hidden_Layer",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Hidden_Layer = device.createPipelineLayout({
                label: "Compute Pipeline Layout Hidden_Layer",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Hidden_Layer],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Hidden_Layer = device.createComputePipeline({
                    label: "Compute Pipeline Hidden_Layer",
                    layout: this.PipelineLayout_Compute_Hidden_Layer,
                    compute: {
                        module: this.ShaderModule_Compute_Hidden_Layer,
                        entryPoint: "HiddenLayer",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Hidden_Layer");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Hidden_Layer",
                    layout: this.PipelineLayout_Compute_Hidden_Layer,
                    compute: {
                        module: this.ShaderModule_Compute_Hidden_Layer,
                        entryPoint: "HiddenLayer",
                    }
                }).then( handle => { this.Pipeline_Compute_Hidden_Layer = handle; this.loadingPromises.delete("Hidden_Layer"); } );
            }
        }
    }

    // (Re)create compute shader Output_Layer
    {
        const bindGroupEntries =
        [
            {
                // NNWeights
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // HiddenLayerActivations
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // OutputLayerActivations
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Output_Layer === null || newHash !== this.Hash_Compute_Output_Layer)
        {
            this.Hash_Compute_Output_Layer = newHash;

            let shaderCode = class_mnist.ShaderCode_Output_Layer_Output_Layer;

            this.ShaderModule_Compute_Output_Layer = device.createShaderModule({ code: shaderCode, label: "Compute Shader Output_Layer"});
            this.BindGroupLayout_Compute_Output_Layer = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Output_Layer",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Output_Layer = device.createPipelineLayout({
                label: "Compute Pipeline Layout Output_Layer",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Output_Layer],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Output_Layer = device.createComputePipeline({
                    label: "Compute Pipeline Output_Layer",
                    layout: this.PipelineLayout_Compute_Output_Layer,
                    compute: {
                        module: this.ShaderModule_Compute_Output_Layer,
                        entryPoint: "OutputLayer",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Output_Layer");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Output_Layer",
                    layout: this.PipelineLayout_Compute_Output_Layer,
                    compute: {
                        module: this.ShaderModule_Compute_Output_Layer,
                        entryPoint: "OutputLayer",
                    }
                }).then( handle => { this.Pipeline_Compute_Output_Layer = handle; this.loadingPromises.delete("Output_Layer"); } );
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
    // Load texture _loadedTexture_10 from "instructions.png"
    if (this.texture__loadedTexture_10 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/instructions.png", this.texture__loadedTexture_10_usageFlags, "2d");
        this.texture__loadedTexture_10 = loadedTex.texture;
        this.texture__loadedTexture_10_size = loadedTex.size;
        this.texture__loadedTexture_10_format = "rgba8unorm-srgb";
        this.texture__loadedTexture_10_usageFlags = loadedTex.usageFlags;
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
                // HiddenLayerActivations
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // OutputLayerActivations
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "read-only-storage" }
            },
            {
                // PresentationCanvas
                binding: 4,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Presentation_Canvas_format), viewDimension: "2d" }
            },
            {
                // PresentationCanvasReadOnly
                binding: 5,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Presentation_PresentationCanvas_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _loadedTexture_0
                binding: 6,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
            {
                // _loadedTexture_1
                binding: 7,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_1_format).sampleType }
            },
            {
                // _loadedTexture_2
                binding: 8,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_2_format).sampleType }
            },
            {
                // _loadedTexture_3
                binding: 9,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_3_format).sampleType }
            },
            {
                // _loadedTexture_4
                binding: 10,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_4_format).sampleType }
            },
            {
                // _loadedTexture_5
                binding: 11,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_5_format).sampleType }
            },
            {
                // _loadedTexture_6
                binding: 12,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_6_format).sampleType }
            },
            {
                // _loadedTexture_7
                binding: 13,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_7_format).sampleType }
            },
            {
                // _loadedTexture_8
                binding: 14,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_8_format).sampleType }
            },
            {
                // _loadedTexture_9
                binding: 15,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_9_format).sampleType }
            },
            {
                // _loadedTexture_10
                binding: 16,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_10_format).sampleType }
            },
            {
                // _PresentationCB
                binding: 17,
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
    encoder.pushDebugGroup("mnist.NN_Weights");

    encoder.popDebugGroup(); // "mnist.NN_Weights"

    encoder.pushDebugGroup("mnist.Drawing_Canvas");

    encoder.popDebugGroup(); // "mnist.Drawing_Canvas"

    encoder.pushDebugGroup("mnist.NN_Input");

    encoder.popDebugGroup(); // "mnist.NN_Input"

    encoder.pushDebugGroup("mnist.Hidden_Layer_Activations");

    encoder.popDebugGroup(); // "mnist.Hidden_Layer_Activations"

    encoder.pushDebugGroup("mnist.Output_Layer_Activations");

    encoder.popDebugGroup(); // "mnist.Output_Layer_Activations"

    encoder.pushDebugGroup("mnist.Presentation_Canvas");

    encoder.popDebugGroup(); // "mnist.Presentation_Canvas"

    encoder.pushDebugGroup("mnist.Imported_Image");

    encoder.popDebugGroup(); // "mnist.Imported_Image"

    encoder.pushDebugGroup("mnist.Draw_Extents");

    encoder.popDebugGroup(); // "mnist.Draw_Extents"

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

    encoder.pushDebugGroup("mnist.Hidden_Layer");

    // Run compute shader Hidden_Layer
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Hidden_Layer",
            layout: this.BindGroupLayout_Compute_Hidden_Layer,
            entries: [
                {
                    // NNInput
                    binding: 0,
                    resource: this.texture_NN_Input.createView({ dimension: "2d", format: this.texture_NN_Input_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // NNWeights
                    binding: 1,
                    resource: { buffer: this.buffer_NN_Weights }
                },
                {
                    // HiddenLayerActivations
                    binding: 2,
                    resource: { buffer: this.buffer_Hidden_Layer_Activations }
                },
            ]
        });

        // Calculate dispatch size

        const baseDispatchSize = [ this.variable_c_numHiddenNeurons, 1, 1 ];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 64 - 1) / 64),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Hidden_Layer !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Hidden_Layer);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Hidden_Layer"

    encoder.pushDebugGroup("mnist.Output_Layer");

    // Run compute shader Output_Layer
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Output_Layer",
            layout: this.BindGroupLayout_Compute_Output_Layer,
            entries: [
                {
                    // NNWeights
                    binding: 0,
                    resource: { buffer: this.buffer_NN_Weights }
                },
                {
                    // HiddenLayerActivations
                    binding: 1,
                    resource: { buffer: this.buffer_Hidden_Layer_Activations }
                },
                {
                    // OutputLayerActivations
                    binding: 2,
                    resource: { buffer: this.buffer_Output_Layer_Activations }
                },
            ]
        });

        // Calculate dispatch size

        const baseDispatchSize = [ this.variable_c_numOutputNeurons, 1, 1 ];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 64 - 1) / 64),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Output_Layer !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Output_Layer);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "mnist.Output_Layer"

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

    encoder.pushDebugGroup("mnist._loadedTexture_10");

    encoder.popDebugGroup(); // "mnist._loadedTexture_10"

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
                    // HiddenLayerActivations
                    binding: 2,
                    resource: { buffer: this.buffer_Hidden_Layer_Activations }
                },
                {
                    // OutputLayerActivations
                    binding: 3,
                    resource: { buffer: this.buffer_Output_Layer_Activations }
                },
                {
                    // PresentationCanvas
                    binding: 4,
                    resource: this.texture_Presentation_Canvas.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // PresentationCanvasReadOnly
                    binding: 5,
                    resource: this.texture_Presentation_PresentationCanvas_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 6,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_1
                    binding: 7,
                    resource: this.texture__loadedTexture_1.createView({ dimension: "2d", format: this.texture__loadedTexture_1_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_2
                    binding: 8,
                    resource: this.texture__loadedTexture_2.createView({ dimension: "2d", format: this.texture__loadedTexture_2_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_3
                    binding: 9,
                    resource: this.texture__loadedTexture_3.createView({ dimension: "2d", format: this.texture__loadedTexture_3_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_4
                    binding: 10,
                    resource: this.texture__loadedTexture_4.createView({ dimension: "2d", format: this.texture__loadedTexture_4_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_5
                    binding: 11,
                    resource: this.texture__loadedTexture_5.createView({ dimension: "2d", format: this.texture__loadedTexture_5_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_6
                    binding: 12,
                    resource: this.texture__loadedTexture_6.createView({ dimension: "2d", format: this.texture__loadedTexture_6_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_7
                    binding: 13,
                    resource: this.texture__loadedTexture_7.createView({ dimension: "2d", format: this.texture__loadedTexture_7_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_8
                    binding: 14,
                    resource: this.texture__loadedTexture_8.createView({ dimension: "2d", format: this.texture__loadedTexture_8_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_9
                    binding: 15,
                    resource: this.texture__loadedTexture_9.createView({ dimension: "2d", format: this.texture__loadedTexture_9_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _loadedTexture_10
                    binding: 16,
                    resource: this.texture__loadedTexture_10.createView({ dimension: "2d", format: this.texture__loadedTexture_10_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // _PresentationCB
                    binding: 17,
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
