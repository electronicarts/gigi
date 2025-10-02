//#pragma pack_matrix(row_major)
//#ifdef SLANG_HLSL_ENABLE_NVAPI
//#include "nvHLSLExtns.h"
//#endif
//
//#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
//#pragma warning(disable : 3557)
//#endif


#line 3 "boxblur.hlsl"
struct Struct_BoxBlur_0CB_0
{
    int radius_0;
    uint sRGB_0;
    float2 _padding0_0;
};



struct SLANG_ParameterGroup_cb0_0
{
    Struct_BoxBlur_0CB_0 _BoxBlur_0CB_0;
};


#line 12
cbuffer cb0_0 : register(b0)
{
    SLANG_ParameterGroup_cb0_0 cb0_0;
}

#line 10
Texture2D<float4 > Input_0 : register(t0);


#line 11
RWTexture2D<float4 > Output_0 : register(u0);


#line 18
float3 LinearToSRGB_0(float3 linearCol_0)
{
    float3 sRGBLo_0 = linearCol_0 * 12.92000007629394531f;
    float3 sRGBHi_0 = pow(abs(linearCol_0), float3(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * 1.0549999475479126f - 0.05499999970197678f;
    float3 sRGB_1;

#line 22
    float _S1;
    if((linearCol_0.x) <= 0.00313080009073019f)
    {

#line 23
        _S1 = sRGBLo_0.x;

#line 23
    }
    else
    {

#line 23
        _S1 = sRGBHi_0.x;

#line 23
    }

#line 23
    sRGB_1[int(0)] = _S1;
    if((linearCol_0.y) <= 0.00313080009073019f)
    {

#line 24
        _S1 = sRGBLo_0.y;

#line 24
    }
    else
    {

#line 24
        _S1 = sRGBHi_0.y;

#line 24
    }

#line 24
    sRGB_1[int(1)] = _S1;
    if((linearCol_0.z) <= 0.00313080009073019f)
    {

#line 25
        _S1 = sRGBLo_0.z;

#line 25
    }
    else
    {

#line 25
        _S1 = sRGBHi_0.z;

#line 25
    }

#line 25
    sRGB_1[int(2)] = _S1;
    return sRGB_1;
}


#line 41
[shader("compute")][numthreads(8, 8, 1)]
void BlurH(uint3 DTid_0 : SV_DispatchThreadID)
{

#line 41
    uint3 _S2 = DTid_0;

    int radius_1 = cb0_0._BoxBlur_0CB_0.radius_0;
    uint w_0;

#line 44
    uint h_0;
    Input_0.GetDimensions(w_0, h_0);

    float4 _S3 = float4(0.0f, 0.0f, 0.0f, 0.0f);

#line 47
    int i_0 = - radius_1;

#line 47
    float4 result_0 = _S3;

    for(;;)
    {

#line 49
        if(i_0 <= radius_1)
        {
        }
        else
        {

#line 49
            break;
        }

#line 50
        uint2 _S4 = (_S2.xy + uint2(int2(i_0, int(0)))) % uint2(int2(int(w_0), int(h_0)));

#line 50
        float4 result_1 = result_0 + Input_0.Load(int3(int2(_S4), int(0))) / (float(radius_1) * 2.0f + 1.0f);

#line 49
        i_0 = i_0 + int(1);

#line 49
        result_0 = result_1;

#line 49
    }


    if(bool(cb0_0._BoxBlur_0CB_0.sRGB_0))
    {

#line 53
        Output_0[_S2.xy] = float4(LinearToSRGB_0(result_0.xyz), result_0.w);

#line 52
    }
    else
    {
        Output_0[_S2.xy] = result_0;

#line 52
    }



    return;
}

