///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Keep synchronized with shader below
struct CopyResizeTexture_CBStruct
{
    int32_t imagePosition[2];
    int32_t imageSize[2];
    int32_t imageClipMin[2];
    int32_t imageClipMax[2];
};

static const char* s_CopyResizeTextureHLSL = R"<<<<<(

// Keep synchronized with C++ above
struct CBStruct
{
    int2 imagePosition;
    int2 imageSize;
    int2 imageClipMin;
    int2 imageClipMax;
};

SamplerState linearClampSampler : register(s0);
Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);
ConstantBuffer<CBStruct> CB : register(b0);

void HandleZoomingIn(int2 px)
{
    // Zooming in, or no zooming. bilinear sample.
    int2 relativePx = px - CB.imagePosition;
    float2 inputUV = (float2(relativePx) + float2(0.5f, 0.5f)) / float2(CB.imageSize);
    Output[px] = Input.SampleLevel(linearClampSampler, inputUV, 0);
}

void HandleZoomingOut(int2 px, uint2 InputDims)
{
    // Zooming out. integrate / average
    int2 relativePx = px - CB.imagePosition;

    float2 inputUVStart = (float2(relativePx) + float2(0.0f, 0.0f)) / float2(CB.imageSize);
    float2 inputUVEnd = (float2(relativePx) + float2(1.0f, 1.0f)) / float2(CB.imageSize);

    // TODO: could make fractional pixel bounds and weight the values by coverage to get a more accurate result

    int2 inputPXStart = int2(inputUVStart * float2(InputDims));
    int2 inputPXEnd = int2(inputUVStart * float2(InputDims));

    float3 result = float3(0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;

    for (int iy = inputPXStart.y; iy <= inputPXEnd.y; ++iy)
    {
        for (int ix = inputPXStart.x; ix <= inputPXEnd.x; ++ix)
        {
            float weight = 1.0f;
            result += Input[uint2(ix, iy)].rgb * weight;
            totalWeight += weight;
        }
    }
    result /= totalWeight;

    Output[px] = float4(result, 1.0f);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int2 px = (int2)DTid.xy;

    // Write black if out of bounds
    {
        uint2 pxStart = clamp(CB.imagePosition, CB.imageClipMin, CB.imageClipMax);
        uint2 pxEnd = clamp(CB.imagePosition + CB.imageSize, CB.imageClipMin, CB.imageClipMax);

        if (px.x < pxStart.x || px.y < pxStart.y || px.x >= pxEnd.x || px.y >= pxEnd.y)
        {
            Output[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
            return;
        }
    }

    uint2 InputDims;
    Input.GetDimensions(InputDims.x, InputDims.y);

    if (InputDims.x <= CB.imageSize.x)
        HandleZoomingIn(px);
    else
        HandleZoomingOut(px, InputDims);
}

)<<<<<";
