// Unnamed technique, shader bloom_effect
/*$(ShaderResources)*/

#include "fft_core.hlsl"

static const float EFFECT_MULTIPLIER = /*$(Variable:effectMultiplier)*/;

/*$(_compute:applyBloomEffect)*/(uint3 DTid : SV_DispatchThreadID)
{
	for (uint i = 0; i < N_CHANNELS; i++) {
		uint3 uv = uint3(DTid.x, DTid.y, i);
		float2 kernelValue = BloomKernel[uv];
		float2 imageValue = FFTBuffer[uv];
        FFTBuffer[uv] = imageValue + mul(imageValue, kernelValue * EFFECT_MULTIPLIER);
    }
}

/*
Shader Resources:
	Texture FFTBuffer (as UAV)
	Texture BloomKernel (as SRV)
*/
