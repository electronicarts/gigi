// Unnamed technique, shader apply_frequency_effect
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

float sinc(float x) {
	return (abs(x) < 1e-4) ? (1.0 - x * x / 6.0) : (sin(x) / x);
}

#define EFFECT_SELECTION /*$(Variable:effectSelection)*/
static const float EFFECT_MULTIPLIER = /*$(Variable:effectMultiplier)*/;

/*$(_compute:applyBlurEffects)*/(uint3 DTid : SV_DispatchThreadID)
{
    float t = 0.5 - 0.5 * cos(EFFECT_MULTIPLIER * 3.0);
    float2 f2 = float2(int2(DTid.xy + N / 2) % N - N / 2) * t;
    float f = length(f2);
    float scale = 1.0;

    // Circular bokeh
    if (EFFECT_SELECTION == 0)
    {
        scale = sinc(f);
    }

    // Gaussian blur
    if (EFFECT_SELECTION == 1)
    {
        scale = exp(-0.2 * f * f);
    }

    // Box blur
    if (EFFECT_SELECTION == 2)
    {
        scale = sinc(f2.x) * sinc(f2.y);
    }

    FFTBuffer[DTid.x + DTid.y * N] *= scale;
}

/*
Shader Resources:
	Texture FFTBuffer (as UAV)
*/
