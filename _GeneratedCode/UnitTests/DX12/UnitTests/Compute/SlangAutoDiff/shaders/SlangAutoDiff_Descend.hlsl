#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


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

#line 89 "core"
struct DiffPair_float_0
{
    float primal_0;
    float differential_0;
};


#line 1 "token paste"
void _d_exp_0(inout DiffPair_float_0 dpx_0, float dOut_0)
{

#line 886 "diff.meta.slang"
    float _S1 = exp(dpx_0.primal_0) * dOut_0;

#line 886
    dpx_0.primal_0 = dpx_0.primal_0;

#line 886
    dpx_0.differential_0 = _S1;

#line 860
    return;
}


#line 1 "token paste"
DiffPair_float_0 _d_exp_1(DiffPair_float_0 dpx_1)
{

#line 1
    DiffPair_float_0 _S2 = { exp(dpx_1.primal_0), exp(dpx_1.primal_0) * dpx_1.differential_0 };

#line 829 "diff.meta.slang"
    return _S2;
}


#line 829
float s_bwd_exp_0(float _S3)
{

#line 829
    return exp(_S3);
}


#line 829
void s_bwd_exp_1(inout DiffPair_float_0 _S4, float _S5)
{

#line 829
    _d_exp_0(_S4, _S5);

#line 829
    return;
}


#line 1 "token paste"
DiffPair_float_0 _d_sqrt_0(DiffPair_float_0 dpx_2)
{

#line 1
    DiffPair_float_0 _S6 = { sqrt(dpx_2.primal_0), 0.5 / sqrt(max(0.00000010000000116861, dpx_2.primal_0)) * dpx_2.differential_0 };

#line 829 "diff.meta.slang"
    return _S6;
}


#line 829
float s_bwd_sqrt_0(float _S7)
{

#line 829
    return sqrt(_S7);
}


#line 19 "SlangAutoDiff_Descend.hlsl"
float GetHeightAtPos_0(float x_0, float y_0, float2 gaussPos_0, float2 gaussSigma_0)
{


    float _S8 = gaussSigma_0.x;

#line 23
    float XOverSigma_0 = x_0 / _S8;

#line 30
    float _S9 = gaussSigma_0.y;

#line 30
    float XOverSigma_1 = y_0 / _S9;

#line 35
    return exp(-0.5 * XOverSigma_0 * XOverSigma_0) / (_S8 * sqrt(6.28318548202514648438)) * (exp(-0.5 * XOverSigma_1 * XOverSigma_1) / (_S9 * sqrt(6.28318548202514648438)));
}


#line 70
void s_bwd_GetHeightAtPos_0(inout DiffPair_float_0 dpx_3, inout DiffPair_float_0 dpy_0, float2 gaussPos_1, float2 gaussSigma_1, float _s_dOut_0)
{

#line 19
    float _S10 = gaussSigma_1.x;

#line 19
    float _S11 = gaussSigma_1.y;

#line 19
    float XOverSigma_2 = dpx_3.primal_0 / _S10;

#line 19
    float _S12 = -0.5 * XOverSigma_2;

#line 19
    float _S13 = _S12 * XOverSigma_2;

#line 19
    float _S14 = _S10 * s_bwd_sqrt_0(6.28318548202514648438);

#line 19
    float _S15 = _S14 * _S14;

#line 19
    float XOverSigma_3 = dpy_0.primal_0 / _S11;

#line 19
    float _S16 = -0.5 * XOverSigma_3;

#line 19
    float _S17 = _S16 * XOverSigma_3;

#line 19
    float _S18 = _S11 * s_bwd_sqrt_0(6.28318548202514648438);

#line 19
    float s_diff_gaussX_T_0 = s_bwd_exp_0(_S17) / _S18 * _s_dOut_0;

#line 19
    float _S19 = _S18 * (s_bwd_exp_0(_S13) / _S14 * _s_dOut_0 / (_S18 * _S18));

#line 19
    DiffPair_float_0 _S20;

#line 19
    _S20.primal_0 = _S17;

#line 19
    _S20.differential_0 = 0.0;

#line 19
    s_bwd_exp_1(_S20, _S19);

#line 19
    float _S21 = (_S16 * _S20.differential_0 + -0.5 * (XOverSigma_3 * _S20.differential_0)) / _S11;

#line 19
    float _S22 = _S14 * (s_diff_gaussX_T_0 / _S15);

#line 19
    DiffPair_float_0 _S23;

#line 19
    _S23.primal_0 = _S13;

#line 19
    _S23.differential_0 = 0.0;

#line 19
    s_bwd_exp_1(_S23, _S22);

#line 19
    float _S24 = (_S12 * _S23.differential_0 + -0.5 * (XOverSigma_2 * _S23.differential_0)) / _S10;

#line 19
    dpy_0.primal_0 = dpy_0.primal_0;

#line 19
    dpy_0.differential_0 = _S21;

#line 19
    dpx_3.primal_0 = dpx_3.primal_0;

#line 19
    dpx_3.differential_0 = _S24;

#line 19
    return;
}


#line 19
void s_bwd_GetHeightAtPos_1(inout DiffPair_float_0 _S25, inout DiffPair_float_0 _S26, float2 _S27, float2 _S28, float _S29)
{

#line 19
    s_bwd_GetHeightAtPos_0(_S25, _S26, _S27, _S28, _S29);

#line 19
    return;
}


#line 19
DiffPair_float_0 s_fwd_GetHeightAtPos_0(DiffPair_float_0 dpx_4, DiffPair_float_0 dpy_1, float2 gaussPos_2, float2 gaussSigma_2)
{

#line 19
    float _S30 = gaussSigma_2.x;

#line 19
    float XOverSigma_4 = dpx_4.primal_0 / _S30;

#line 19
    float s_diff_XOverSigma_0 = dpx_4.differential_0 / _S30;

#line 19
    float _S31 = -0.5 * XOverSigma_4;

#line 19
    DiffPair_float_0 _S32 = { _S31 * XOverSigma_4, s_diff_XOverSigma_0 * -0.5 * XOverSigma_4 + s_diff_XOverSigma_0 * _S31 };

#line 19
    DiffPair_float_0 _S33 = _d_exp_1(_S32);

#line 19
    DiffPair_float_0 _S34 = { 6.28318548202514648438, 0.0 };

#line 19
    DiffPair_float_0 _S35 = _d_sqrt_0(_S34);

#line 19
    float _S36 = _S30 * _S35.primal_0;

#line 19
    float gaussX_0 = _S33.primal_0 / _S36;

#line 19
    float _S37 = gaussSigma_2.y;

#line 19
    float XOverSigma_5 = dpy_1.primal_0 / _S37;

#line 19
    float s_diff_XOverSigma_1 = dpy_1.differential_0 / _S37;

#line 19
    float _S38 = -0.5 * XOverSigma_5;

#line 19
    DiffPair_float_0 _S39 = { _S38 * XOverSigma_5, s_diff_XOverSigma_1 * -0.5 * XOverSigma_5 + s_diff_XOverSigma_1 * _S38 };

#line 19
    DiffPair_float_0 _S40 = _d_exp_1(_S39);

#line 19
    DiffPair_float_0 _S41 = _d_sqrt_0(_S34);

#line 19
    float _S42 = _S37 * _S41.primal_0;

#line 19
    float gaussY_0 = _S40.primal_0 / _S42;

#line 19
    DiffPair_float_0 _S43 = { gaussX_0 * gaussY_0, (_S33.differential_0 * _S36 - _S33.primal_0 * (_S35.differential_0 * _S30)) / (_S36 * _S36) * gaussY_0 + (_S40.differential_0 * _S42 - _S40.primal_0 * (_S41.differential_0 * _S37)) / (_S42 * _S42) * gaussX_0 };

#line 19
    return _S43;
}


#line 40
[shader("compute")][numthreads(1, 1, 1)]
void csmain(uint3 DTid_0 : SV_DISPATCHTHREADID)
{

#line 42
    float _S44 = Data_0[0U];

#line 42
    float _S45 = Data_0[1U];

#line 42
    float2 ballPos_0 = float2(_S44, _S45);
    float2 _S46 = float2(0.0, 0.0);

#line 113
    float2 _S47 = float2(0.00100000004749745131, 0.00100000004749745131);

#line 113
    float2 _S48 = float2(0.99000000953674316406, 0.99000000953674316406);

#line 113
    int i_0 = int(0);

#line 113
    float2 ballPosGradient_0 = _S46;

#line 113
    for(;;)
    {

#line 45
        if(i_0 < _DescendCB_0.NumGaussians_0)
        {
        }
        else
        {

#line 45
            break;
        }

        int _S49 = int(4) + i_0 * int(5);

#line 48
        float _S50 = Data_0[uint(_S49)];

#line 48
        float _S51 = Data_0[uint(_S49 + int(1))];

#line 48
        float2 gaussPos_3 = float2(_S50, _S51);
        float gaussAngle_0 = Data_0[uint(_S49 + int(2))];
        float _S52 = Data_0[uint(_S49 + int(3))];

#line 50
        float _S53 = Data_0[uint(_S49 + int(4))];

#line 50
        float2 gaussSigma_3 = float2(_S52, _S53);


        float2 relativePos_0 = ballPos_0 - gaussPos_3;
        float _S54 = - gaussAngle_0;

#line 54
        float cosTheta_0 = cos(_S54);
        float sinTheta_0 = sin(_S54);

        float _S55 = relativePos_0.x;

#line 57
        float _S56 = relativePos_0.y;

#line 57
        float _S57 = _S55 * cosTheta_0 - _S56 * sinTheta_0;
        float _S58 = _S55 * sinTheta_0 + _S56 * cosTheta_0;



        float2 dFLocal_0 = _S46;


        if(bool(_DescendCB_0.UseBackwardAD_0))
        {

            float height_0 = GetHeightAtPos_0(_S57, _S58, gaussPos_3, gaussSigma_3);

            DiffPair_float_0 ballPosX_0;

#line 70
            ballPosX_0.primal_0 = _S57;

#line 70
            ballPosX_0.differential_0 = 0.0;
            DiffPair_float_0 ballPosY_0;

#line 71
            ballPosY_0.primal_0 = _S58;

#line 71
            ballPosY_0.differential_0 = 0.0;
            s_bwd_GetHeightAtPos_1(ballPosX_0, ballPosY_0, gaussPos_3, gaussSigma_3, height_0);

            dFLocal_0 = float2(ballPosX_0.differential_0, ballPosY_0.differential_0);

#line 65
        }
        else
        {

#line 65
            DiffPair_float_0 _S59 = { _S57, 1.0 };

#line 65
            DiffPair_float_0 _S60 = { _S58, 0.0 };

#line 84
            dFLocal_0[int(0)] = s_fwd_GetHeightAtPos_0(_S59, _S60, gaussPos_3, gaussSigma_3).differential_0;

#line 84
            DiffPair_float_0 _S61 = { _S57, 0.0 };

#line 84
            DiffPair_float_0 _S62 = { _S58, 1.0 };

#line 92
            dFLocal_0[int(1)] = dFLocal_0.y + s_fwd_GetHeightAtPos_0(_S61, _S62, gaussPos_3, gaussSigma_3).differential_0;

#line 65
        }

#line 97
        float cosNegTheta_0 = cos(gaussAngle_0);
        float sinNegTheta_0 = sin(gaussAngle_0);

#line 103
        float2 ballPosGradient_1 = ballPosGradient_0 + float2(dFLocal_0.x * cosNegTheta_0 - dFLocal_0.y * sinNegTheta_0, dFLocal_0.x * sinNegTheta_0 + dFLocal_0.y * cosNegTheta_0);

#line 45
        i_0 = i_0 + int(1);

#line 45
        ballPosGradient_0 = ballPosGradient_1;

#line 45
    }

#line 107
    float2 adjust_0 = - ballPosGradient_0 * _DescendCB_0.LearningRate_0;

#line 107
    float2 adjust_1;
    if(length(adjust_0) > _DescendCB_0.MaximumStepSize_0)
    {

#line 108
        adjust_1 = normalize(adjust_0) * _DescendCB_0.MaximumStepSize_0;

#line 108
    }
    else
    {

#line 108
        adjust_1 = adjust_0;

#line 108
    }

#line 113
    float2 ballPos_1 = clamp(ballPos_0 + adjust_1, _S47, _S48);


    Data_0[0U] = ballPos_1.x;
    Data_0[1U] = ballPos_1.y;


    Data_0[2U] = ballPosGradient_0.x;
    Data_0[3U] = ballPosGradient_0.y;
    return;
}

