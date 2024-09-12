// Unnamed technique, shader fft_initialize_rsc
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

/*$(_compute:fftInitializeRsc)*/(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = InputTexture[DTid.xy];

    if (N_CHANNELS == 1) {
        color.r = dot(float3(0.2126, 0.7152, 0.0722), color.rgb);
    }

    for (uint i = 0; i < N_CHANNELS; i++) {
        FFTBuffer[int3(DTid.x, DTid.y, i)] = float2(min(color[i], 15), 0.0);
    }
}

/*
Shader Resources:
	Texture InputTexture (as SRV)
	Buffer FFTBuffer (as UAV)
*/
