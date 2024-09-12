// Unnamed technique, shader bloom_effect
/*$(ShaderResources)*/

#include "fft_core.hlsl"

static const float EFFECT_MULTIPLIER = /*$(Variable:effectMultiplier)*/;

/*$(_compute:applyBloomEffect)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint uv = DTid.x + DTid.y * N;
	float2 kernelValue = BloomKernel[uv];
	float2 imageValue = FFTBuffer[uv];
	FFTBuffer[uv] = imageValue + mul(imageValue, kernelValue * EFFECT_MULTIPLIER / N);
}

/*
Shader Resources:
	Texture FFTBuffer (as UAV)
	Texture BloomKernel (as SRV)
*/
