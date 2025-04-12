#pragma pack_matrix(row_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 12 "SlangAutoDiff_Descend.hlsl"
RWBuffer<float > Data_0 : register(u0);


#line 4
struct Struct_DescendCB_0
{
    float LearningRate_0;
    float MaximumStepSize_0;
    int NumGaussians_0;
    uint UseBackwardAD_0;
};


cbuffer _DescendCB_0 : register(b0)
{
    Struct_DescendCB_0 _DescendCB_0;
}

#line 7818 "hlsl.meta.slang"
struct DiffPair_float_0
{
    float primal_0;
    float differential_0;
};


#line 1 "token paste"
void _d_exp_0(inout DiffPair_float_0 dpx_0, float dOut_0)
{

#line 1868 "diff.meta.slang"
    float _S1 = exp(dpx_0.primal_0) * dOut_0;

#line 1868
    dpx_0.primal_0 = dpx_0.primal_0;

#line 1868
    dpx_0.differential_0 = _S1;



    return;
}


#line 1 "token paste"
DiffPair_float_0 _d_exp_1(DiffPair_float_0 dpx_1)
{

#line 1841 "diff.meta.slang"
    float _S2 = exp(dpx_1.primal_0);

#line 1841
    DiffPair_float_0 _S3 = { _S2, _S2 * dpx_1.differential_0 };

#line 1841
    return _S3;
}


#line 1 "token paste"
DiffPair_float_0 _d_sqrt_0(DiffPair_float_0 dpx_2)
{

#line 1838 "diff.meta.slang"
    DiffPair_float_0 _S4 = { sqrt(dpx_2.primal_0), 0.5f / sqrt(max(1.00000001168609742e-07f, dpx_2.primal_0)) * dpx_2.differential_0 };


    return _S4;
}


#line 7 "SlangAutoDiff_Descend.hlsl"
float GetHeightAtPos_0(float x_0, float y_0, float2 gaussPos_0, float2 gaussSigma_0)
{


    float _S5 = gaussSigma_0.x;

#line 11
    float XOverSigma_0 = x_0 / _S5;

    float _S6 = sqrt(6.28318548202514648f);

#line 18
    float _S7 = gaussSigma_0.y;

#line 18
    float XOverSigma_1 = y_0 / _S7;

#line 23
    return exp(-0.5f * XOverSigma_0 * XOverSigma_0) / (_S5 * _S6) * (exp(-0.5f * XOverSigma_1 * XOverSigma_1) / (_S7 * _S6));
}


#line 57
float s_primal_ctx_exp_0(float _S8)
{

#line 57
    return exp(_S8);
}


#line 57
float s_primal_ctx_sqrt_0(float _S9)
{

#line 57
    return sqrt(_S9);
}


#line 57
void s_bwd_prop_exp_0(inout DiffPair_float_0 _S10, float _S11)
{

#line 57
    _d_exp_0(_S10, _S11);

#line 57
    return;
}


#line 7
void s_bwd_prop_GetHeightAtPos_0(inout DiffPair_float_0 dpx_3, inout DiffPair_float_0 dpy_0, float2 gaussPos_1, float2 gaussSigma_1, float _s_dOut_0)
{


    float _S12 = gaussSigma_1.x;

#line 11
    float XOverSigma_2 = dpx_3.primal_0 / _S12;
    float _S13 = -0.5f * XOverSigma_2;

#line 12
    float _S14 = _S13 * XOverSigma_2;

#line 12
    float _S15 = s_primal_ctx_sqrt_0(6.28318548202514648f);
    float _S16 = _S12 * _S15;

#line 13
    float _S17 = _S16 * _S16;

#line 18
    float _S18 = gaussSigma_1.y;

#line 18
    float XOverSigma_3 = dpy_0.primal_0 / _S18;
    float _S19 = -0.5f * XOverSigma_3;

#line 19
    float _S20 = _S19 * XOverSigma_3;
    float _S21 = _S18 * _S15;


    float s_diff_gaussX_T_0 = s_primal_ctx_exp_0(_S20) / _S21 * _s_dOut_0;

#line 20
    float _S22 = _S21 * (s_primal_ctx_exp_0(_S14) / _S16 * _s_dOut_0 / (_S21 * _S21));

#line 19
    DiffPair_float_0 _S23;

#line 19
    _S23.primal_0 = _S20;

#line 19
    _S23.differential_0 = 0.0f;

#line 19
    s_bwd_prop_exp_0(_S23, _S22);

#line 18
    float _S24 = (_S19 * _S23.differential_0 + -0.5f * (XOverSigma_3 * _S23.differential_0)) / _S18;

#line 13
    float _S25 = _S16 * (s_diff_gaussX_T_0 / _S17);

#line 12
    DiffPair_float_0 _S26;

#line 12
    _S26.primal_0 = _S14;

#line 12
    _S26.differential_0 = 0.0f;

#line 12
    s_bwd_prop_exp_0(_S26, _S25);

#line 11
    float _S27 = (_S13 * _S26.differential_0 + -0.5f * (XOverSigma_2 * _S26.differential_0)) / _S12;

#line 11
    dpy_0.primal_0 = dpy_0.primal_0;

#line 11
    dpy_0.differential_0 = _S24;

#line 11
    dpx_3.primal_0 = dpx_3.primal_0;

#line 11
    dpx_3.differential_0 = _S27;

#line 7
    return;
}


#line 7
void s_bwd_GetHeightAtPos_0(inout DiffPair_float_0 _S28, inout DiffPair_float_0 _S29, float2 _S30, float2 _S31, float _S32)
{

#line 7
    s_bwd_prop_GetHeightAtPos_0(_S28, _S29, _S30, _S31, _S32);

#line 7
    return;
}


#line 7
DiffPair_float_0 s_fwd_GetHeightAtPos_0(DiffPair_float_0 dpx_4, DiffPair_float_0 dpy_1, float2 gaussPos_2, float2 gaussSigma_2)
{


    float _S33 = gaussSigma_2.x;

#line 11
    float XOverSigma_4 = dpx_4.primal_0 / _S33;

#line 11
    float s_diff_XOverSigma_0 = dpx_4.differential_0 / _S33;
    float _S34 = -0.5f * XOverSigma_4;

#line 12
    DiffPair_float_0 _S35 = { _S34 * XOverSigma_4, s_diff_XOverSigma_0 * -0.5f * XOverSigma_4 + s_diff_XOverSigma_0 * _S34 };

#line 12
    DiffPair_float_0 _S36 = _d_exp_1(_S35);

#line 12
    DiffPair_float_0 _S37 = { 6.28318548202514648f, 0.0f };
    DiffPair_float_0 _S38 = _d_sqrt_0(_S37);

#line 13
    float _S39 = _S33 * _S38.primal_0;

#line 13
    float gaussX_0 = _S36.primal_0 / _S39;

#line 18
    float _S40 = gaussSigma_2.y;

#line 18
    float XOverSigma_5 = dpy_1.primal_0 / _S40;

#line 18
    float s_diff_XOverSigma_1 = dpy_1.differential_0 / _S40;
    float _S41 = -0.5f * XOverSigma_5;

#line 19
    DiffPair_float_0 _S42 = { _S41 * XOverSigma_5, s_diff_XOverSigma_1 * -0.5f * XOverSigma_5 + s_diff_XOverSigma_1 * _S41 };

#line 19
    DiffPair_float_0 _S43 = _d_exp_1(_S42);
    float _S44 = _S40 * _S38.primal_0;

#line 20
    float gaussY_0 = _S43.primal_0 / _S44;

#line 20
    DiffPair_float_0 _S45 = { gaussX_0 * gaussY_0, (_S36.differential_0 * _S39 - _S36.primal_0 * (_S38.differential_0 * _S33)) / (_S39 * _S39) * gaussY_0 + (_S43.differential_0 * _S44 - _S43.primal_0 * (_S38.differential_0 * _S40)) / (_S44 * _S44) * gaussX_0 };


    return _S45;
}


[shader("compute")][numthreads(1, 1, 1)]
void csmain(uint3 DTid_0 : SV_DispatchThreadID)
{

#line 29
    float _S46 = Data_0.Load(int(0));

#line 29
    float _S47 = Data_0.Load(int(1));

#line 29
    float2 ballPos_0 = float2(_S46, _S47);
    float2 _S48 = float2(0.0f, 0.0f);

#line 30
    int i_0 = int(0);

#line 30
    float2 ballPosGradient_0 = _S48;

    for(;;)
    {

#line 32
        if(i_0 < (_DescendCB_0.NumGaussians_0))
        {
        }
        else
        {

#line 32
            break;
        }

        float _S49 = Data_0.Load(int(uint(int(4) + i_0 * int(5))));

#line 35
        float _S50 = Data_0.Load(int(uint(int(4) + i_0 * int(5) + int(1))));

#line 35
        float2 gaussPos_3 = float2(_S49, _S50);
        float _S51 = Data_0.Load(int(uint(int(4) + i_0 * int(5) + int(2))));
        float _S52 = Data_0.Load(int(uint(int(4) + i_0 * int(5) + int(3))));

#line 37
        float _S53 = Data_0.Load(int(uint(int(4) + i_0 * int(5) + int(4))));

#line 37
        float2 gaussSigma_3 = float2(_S52, _S53);


        float2 relativePos_0 = ballPos_0 - gaussPos_3;
        float _S54 = - _S51;

#line 41
        float cosTheta_0 = cos(_S54);
        float sinTheta_0 = sin(_S54);

        float _S55 = relativePos_0.x;

#line 44
        float _S56 = relativePos_0.y;

#line 44
        float _S57 = _S55 * cosTheta_0 - _S56 * sinTheta_0;
        float _S58 = _S55 * sinTheta_0 + _S56 * cosTheta_0;



        float2 dFLocal_0 = _S48;


        if(bool(_DescendCB_0.UseBackwardAD_0))
        {

            float height_0 = GetHeightAtPos_0(_S57, _S58, gaussPos_3, gaussSigma_3);

            DiffPair_float_0 ballPosX_0;

#line 57
            ballPosX_0.primal_0 = _S57;

#line 57
            ballPosX_0.differential_0 = 0.0f;
            DiffPair_float_0 ballPosY_0;

#line 58
            ballPosY_0.primal_0 = _S58;

#line 58
            ballPosY_0.differential_0 = 0.0f;
            s_bwd_GetHeightAtPos_0(ballPosX_0, ballPosY_0, gaussPos_3, gaussSigma_3, height_0);

            dFLocal_0 = float2(ballPosX_0.differential_0, ballPosY_0.differential_0);

#line 52
        }
        else
        {

#line 52
            DiffPair_float_0 _S59 = { _S57, 1.0f };

#line 52
            DiffPair_float_0 _S60 = { _S58, 0.0f };

#line 71
            dFLocal_0[int(0)] = s_fwd_GetHeightAtPos_0(_S59, _S60, gaussPos_3, gaussSigma_3).differential_0;

#line 71
            DiffPair_float_0 _S61 = { _S57, 0.0f };

#line 71
            DiffPair_float_0 _S62 = { _S58, 1.0f };

#line 79
            dFLocal_0[int(1)] = dFLocal_0[int(1)] + s_fwd_GetHeightAtPos_0(_S61, _S62, gaussPos_3, gaussSigma_3).differential_0;

#line 52
        }

#line 84
        float cosNegTheta_0 = cos(_S51);
        float sinNegTheta_0 = sin(_S51);

#line 90
        float2 ballPosGradient_1 = ballPosGradient_0 + float2(dFLocal_0.x * cosNegTheta_0 - dFLocal_0.y * sinNegTheta_0, dFLocal_0.x * sinNegTheta_0 + dFLocal_0.y * cosNegTheta_0);

#line 32
        i_0 = i_0 + int(1);

#line 32
        ballPosGradient_0 = ballPosGradient_1;

#line 32
    }

#line 94
    float2 adjust_0 = - ballPosGradient_0 * _DescendCB_0.LearningRate_0;

#line 94
    float2 adjust_1;
    if((length(adjust_0)) > (_DescendCB_0.MaximumStepSize_0))
    {

#line 95
        adjust_1 = normalize(adjust_0) * _DescendCB_0.MaximumStepSize_0;

#line 95
    }
    else
    {

#line 95
        adjust_1 = adjust_0;

#line 95
    }

#line 100
    float2 ballPos_1 = clamp(ballPos_0 + adjust_1, float2(0.00100000004749745f, 0.00100000004749745f), float2(0.99000000953674316f, 0.99000000953674316f));


    ((Data_0))[(0U)] = (ballPos_1.x);
    ((Data_0))[(1U)] = (ballPos_1.y);


    ((Data_0))[(2U)] = (ballPosGradient_0.x);
    ((Data_0))[(3U)] = (ballPosGradient_0.y);
    return;
}

