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
    // mode: 0 = depth pass, 1 = motion-vector pass
    // mode: 2 = color copy (hudless / scene color)
    int32_t mode;
    // depth comparison threshold (in same depth units as depth buffer)
    float depthThreshold;
    int32_t _pad0[2];
};

static const char* s_CopyResizeTextureHLSL = R"<<<<<(

struct CBStruct
{
    int2 imagePosition;
    int2 imageSize;
    int2 imageClipMin;
    int2 imageClipMax;
    int mode;              // 0 = depth, 1 = motion vectors, 2 = color copy
    float depthThreshold;  // depth compare threshold
    int2 _pad0;
};

SamplerState linearClampSampler : register(s0);
Texture2D<float4> Input0 : register(t0);
Texture2D<float> InputDepth : register(t1);
RWTexture2D<float4> Output : register(u0);
ConstantBuffer<CBStruct> CB : register(b0);

void HandleZoomingIn(int2 px)
{
    int2 relativePx = px - CB.imagePosition;
    float2 inputUV = (float2(relativePx) + float2(0.5f, 0.5f)) / float2(CB.imageSize);
    float4 v = Input0.SampleLevel(linearClampSampler, inputUV, 0);
    float d = InputDepth.SampleLevel(linearClampSampler, inputUV, 0);

    // Select output based on mode, no branch
    float4 outDepth = float4(d, d, d, 1.0f);
    float4 outMV    = float4(v.xy, 0.0f, 1.0f);
    float4 outColor = float4(v.rgb, 1.0f);
    if (CB.mode == 0)
        Output[px] = outDepth;
    else if (CB.mode == 1)
        Output[px] = outMV;
    else
        Output[px] = outColor;
}

void HandleZoomingOut(int2 px, uint2 InputDims)
{
    int2 relativePx = px - CB.imagePosition;
    float2 inputUVStart = (float2(relativePx) + float2(0.0f, 0.0f)) / float2(CB.imageSize);
    float2 inputUVEnd   = (float2(relativePx) + float2(1.0f, 1.0f)) / float2(CB.imageSize);

    int2 inputPXStart = int2(inputUVStart * float2(InputDims));
    int2 inputPXEnd   = int2(inputUVEnd * float2(InputDims));
    inputPXStart = max(inputPXStart, int2(0, 0));
    inputPXEnd   = min(inputPXEnd, int2(int(InputDims.x) - 1, int(InputDims.y) - 1)); // <-- fixed

    float3 accumColor = float3(0.0f, 0.0f, 0.0f);
    float2 accumMV    = float2(0.0f, 0.0f);
    float totalWeight = 0.0f;

    float2 centerUV = (inputUVStart + inputUVEnd) * 0.5f;
    float centerDepth = InputDepth.SampleLevel(linearClampSampler, centerUV, 0);

    int mode = CB.mode;
    for (int iy = inputPXStart.y; iy <= inputPXEnd.y; ++iy)
    {
        for (int ix = inputPXStart.x; ix <= inputPXEnd.x; ++ix)
        {
            float weight = 1.0f;
            float sampleDepth = InputDepth.Load(int3(ix, iy, 0));

            if (mode == 0)
            {
                // accumulate depth as greyscale
                accumColor += float3(sampleDepth, sampleDepth, sampleDepth);
                totalWeight += weight;
            }
            else if (mode == 1)
            {
                // motion vectors: depth-aware inliers
                float inlier = (abs(sampleDepth - centerDepth) <= CB.depthThreshold) ? 1.0f : 0.0f;
                float4 mv = Input0.Load(int3(ix, iy, 0));
                accumMV += mv.xy * weight * inlier;
                totalWeight += weight * inlier;
            }
            else
            {
                // color copy: average color values (no depth gating)
                float4 col = Input0.Load(int3(ix, iy, 0));
                accumColor += col.rgb * weight;
                totalWeight += weight;
            }
        }
    }

    // Fallback: sample center
    float dCenter = InputDepth.SampleLevel(linearClampSampler, centerUV, 0);
    float4 outDepth = float4(dCenter, dCenter, dCenter, 1.0f);
    float4 sampled = Input0.SampleLevel(linearClampSampler, centerUV, 0);
    float4 outMV    = float4(sampled.xy, 0.0f, 1.0f);
    float4 outColor = float4(sampled.rgb, 1.0f);

    if (totalWeight <= 0.0f)
    {
        if (mode == 0)
            Output[px] = outDepth;
        else if (mode == 1)
            Output[px] = outMV;
        else
            Output[px] = outColor;
        return;
    }

    if (mode == 0)
    {
        Output[px] = float4(accumColor / totalWeight, 1.0f);
    }
    else if (mode == 1)
    {
        Output[px] = float4(accumMV / totalWeight, 0.0f, 1.0f);
    }
    else
    {
        Output[px] = float4(accumColor / totalWeight, 1.0f);
    }
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int2 px = (int2)DTid.xy;

    uint2 pxStart = clamp(CB.imagePosition, CB.imageClipMin, CB.imageClipMax);
    uint2 pxEnd   = clamp(CB.imagePosition + CB.imageSize, CB.imageClipMin, CB.imageClipMax);

    if (px.x < pxStart.x || px.y < pxStart.y || px.x >= pxEnd.x || px.y >= pxEnd.y)
    {
        if (CB.mode != 2)
            Output[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    uint2 InputDims;
    Input0.GetDimensions(InputDims.x, InputDims.y);

    if (InputDims.x <= CB.imageSize.x)
        HandleZoomingIn(px);
    else
        HandleZoomingOut(px, InputDims);
}

)<<<<<";
