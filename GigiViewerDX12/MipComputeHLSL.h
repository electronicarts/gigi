///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2026 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Keep synchronized with shader below
struct MipCompute_CBStruct
{
    uint32_t isSRGBFormat; // 0/1
    uint32_t _pad[3];
};

static const char* s_MipComputeHLSL = R"<<<<<(

struct CBStruct
{
    uint  isSRGBFormat; // 0/1
    uint3 _pad;
};

ConstantBuffer<CBStruct> CB : register(b0);

RWTexture2D<float4> input2D : register(u0);
RWTexture2D<float4> output2D : register(u1);

RWTexture3D<float4> input3D : register(u0);
RWTexture3D<float4> output3D : register(u1);

uint3 MipDims(uint3 base, uint mip)
{
    return max(base >> mip, 1);
}

uint2 Clamp2(uint2 v, uint2 maxv) { return min(v, maxv); }
uint3 Clamp3(uint3 v, uint3 maxv) { return min(v, maxv); }

float3 LinearToSRGB(float3 linearCol)
{
    float3 sRGBLo = linearCol * 12.92;
    float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
    float3 sRGB;
    sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
    sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
    sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
    return sRGB;
}

float3 SRGBToLinear(in float3 sRGBCol)
{
    float3 linearRGBLo = sRGBCol / 12.92;
    float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
    float3 linearRGB;
    linearRGB.r = sRGBCol.r <= 0.04045 ? linearRGBLo.r : linearRGBHi.r;
    linearRGB.g = sRGBCol.g <= 0.04045 ? linearRGBLo.g : linearRGBHi.g;
    linearRGB.b = sRGBCol.b <= 0.04045 ? linearRGBLo.b : linearRGBHi.b;
    return linearRGB;
}

[numthreads(8, 8, 1)]
void main2D(uint3 DTid : SV_DispatchThreadID)
{
    uint2 srcDims, dstDims;
    input2D.GetDimensions(srcDims.x, srcDims.y);
    output2D.GetDimensions(dstDims.x, dstDims.y);

    if (DTid.x >= dstDims.x || DTid.y >= dstDims.y) return;

    uint2 srcBase = DTid.xy * 2;
    uint2 maxSrc  = (srcDims.x > 0 && srcDims.y > 0) ? (srcDims - 1) : uint2(0,0);

    uint2 p00 = Clamp2(srcBase + uint2(0,0), maxSrc);
    uint2 p10 = Clamp2(srcBase + uint2(1,0), maxSrc);
    uint2 p11 = Clamp2(srcBase + uint2(1,1), maxSrc);
    uint2 p01 = Clamp2(srcBase + uint2(0,1), maxSrc);

    float4 s00 = input2D[p00];
    float4 s10 = input2D[p10];
    float4 s11 = input2D[p11];
    float4 s01 = input2D[p01];

    if(CB.isSRGBFormat)
    {
        s00.rgb = SRGBToLinear(s00.rgb);
        s10.rgb = SRGBToLinear(s10.rgb);
        s11.rgb = SRGBToLinear(s11.rgb);
        s01.rgb = SRGBToLinear(s01.rgb);
    }

    // Blend premultiplied alpha
    s00.rgb *= s00.a;
    s10.rgb *= s10.a;
    s11.rgb *= s11.a;
    s01.rgb *= s01.a;

    float4 result =
        s00 + s10 +
        s11 + s01;

    result *= .25f; // /= 4.0f;

    // un-premultiply alpha
    if (result.a > 0.0f)
        result.rgb /= result.a;

    if(CB.isSRGBFormat)
        result.rgb = LinearToSRGB(result.rgb);

    output2D[DTid.xy] = result;
}

[numthreads(8, 8, 1)]
void main3D(uint3 DTid : SV_DispatchThreadID)
{
    uint3 srcDims, dstDims;
    input3D.GetDimensions(srcDims.x, srcDims.y, srcDims.z);
    output3D.GetDimensions(dstDims.x, dstDims.y, dstDims.z);

    if (DTid.x >= dstDims.x || DTid.y >= dstDims.y || DTid.z >= dstDims.z) return;

    uint3 srcBase = DTid.xyz * 2;
    uint3 maxSrc  = (srcDims.x > 0 && srcDims.y > 0 && srcDims.z > 0) ? (srcDims - 1) : uint3(0,0,0);

    uint3 p000 = Clamp3(srcBase + uint3(0,0,0), maxSrc);
    uint3 p100 = Clamp3(srcBase + uint3(1,0,0), maxSrc);
    uint3 p010 = Clamp3(srcBase + uint3(0,1,0), maxSrc);
    uint3 p110 = Clamp3(srcBase + uint3(1,1,0), maxSrc);
    uint3 p001 = Clamp3(srcBase + uint3(0,0,1), maxSrc);
    uint3 p101 = Clamp3(srcBase + uint3(1,0,1), maxSrc);
    uint3 p011 = Clamp3(srcBase + uint3(0,1,1), maxSrc);
    uint3 p111 = Clamp3(srcBase + uint3(1,1,1), maxSrc);

    float4 s000 = input3D[p000];
    float4 s100 = input3D[p100];
    float4 s010 = input3D[p010];
    float4 s110 = input3D[p110];
    float4 s001 = input3D[p001];
    float4 s101 = input3D[p101];
    float4 s011 = input3D[p011];
    float4 s111 = input3D[p111];

    if(CB.isSRGBFormat)
    {
        s000.rgb = SRGBToLinear(s000.rgb);
        s100.rgb = SRGBToLinear(s100.rgb);
        s110.rgb = SRGBToLinear(s110.rgb);
        s010.rgb = SRGBToLinear(s010.rgb);
        s001.rgb = SRGBToLinear(s001.rgb);
        s101.rgb = SRGBToLinear(s101.rgb);
        s111.rgb = SRGBToLinear(s111.rgb);
        s011.rgb = SRGBToLinear(s011.rgb);
    }

    // Blend premultiplied alpha
    s000.rgb *= s000.a;
    s100.rgb *= s100.a;
    s110.rgb *= s110.a;
    s010.rgb *= s010.a;
    s001.rgb *= s001.a;
    s101.rgb *= s101.a;
    s111.rgb *= s111.a;
    s011.rgb *= s011.a;

    float4 result =
        s000 + s100 + s010 + s110 +
        s001 + s101 + s011 + s111;

    result *= 0.125f; // /= 8.0f;

    // un-premultiply alpha
    if (result.a > 0.0f)
        result.rgb /= result.a;

    if(CB.isSRGBFormat)
        result.rgb = LinearToSRGB(result.rgb);

    output3D[DTid.xyz] = result;
}

)<<<<<";