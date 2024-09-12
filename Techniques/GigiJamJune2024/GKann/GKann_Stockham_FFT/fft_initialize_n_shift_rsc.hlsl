// Unnamed technique, shader fftInitializeAndShiftRsc
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

/*$(_compute:fftInitializeAndShiftRsc)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 uv = (DTid.xy + N / 2) % N;
    float4 color = InputTexture[uv.xy];

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
