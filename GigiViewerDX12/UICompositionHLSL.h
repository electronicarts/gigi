///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Keep synchronized with shader below
struct UIComposition_CBStruct
{
    int32_t imagePosition[2];
    int32_t imageSize[2];
    int32_t imageClipMin[2];
    int32_t imageClipMax[2];
};

static const char* s_UICompositionGraphicsHLSL = R"(
struct CBStruct
{
    int2 imagePosition;
    int2 imageSize;
    int2 imageClipMin;
    int2 imageClipMax;
};

// Full screen triangle vertex shader
struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSOutput VSMain(uint id : SV_VertexID)
{
    VSOutput output;
    output.uv = float2((id << 1) & 2, id & 2);
    output.pos = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}

SamplerState linearClampSampler : register(s0);
Texture2D<float4> sceneColor : register(t0);     // Scene
Texture2D<float4> uiTexture : register(t1);      // UI
ConstantBuffer<CBStruct> CB : register(b0);

// Pixel Shader
float4 PSMain(VSOutput input) : SV_Target
{
    int2 px = int2(input.pos.xy);
    
    // ... [Logic similar to existing CS but using input.uv or px] ...
    
    // For a graphics pass, you typically sample sceneColor using UVs or Load using pixel coords
    // If you want exact absolute positioning matching the compute shader:
    
    // Load scene color
    float4 scenePixel = sceneColor.Load(int3(px, 0));
    
    // Check if this pixel is within the UI region
    int2 relativePx = px - CB.imagePosition;
    bool inUIRegion = (px.x >= CB.imageClipMin[0] && px.x < CB.imageClipMax[0] &&
                       px.y >= CB.imageClipMin[1] && px.y < CB.imageClipMax[1] &&
                       relativePx.x >= 0 && relativePx.x < CB.imageSize[0] &&
                       relativePx.y >= 0 && relativePx.y < CB.imageSize[1]);
    
    if (!inUIRegion)
    {
        return scenePixel;
    }
    
    // Sample UI texture
    float2 uiUV = (float2(relativePx) + float2(0.5f, 0.5f)) / float2(CB.imageSize);
    float4 uiPixel = uiTexture.SampleLevel(linearClampSampler, uiUV, 0);
    
    float4 result = float4(
        lerp(scenePixel.rgb, uiPixel.rgb, uiPixel.a),
        1.0f
    );
    
    return result;
}
)";