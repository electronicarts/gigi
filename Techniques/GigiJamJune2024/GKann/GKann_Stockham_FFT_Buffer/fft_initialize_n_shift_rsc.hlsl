// Unnamed technique, shader fftInitializeAndShiftRsc
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

/*$(_compute:fftInitializeAndShiftRsc)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 uv = (DTid.xy + N / 2) % N;
    float4 color = InputTexture[uv];
    color = min(color, 15);
    FFTBufferRG[DTid.x + DTid.y * N] = color.rg;
    FFTBufferB[DTid.x + DTid.y * N] = float2(color.b, 0.0);
}

/*
Shader Resources:
	Texture InputTexture (as SRV)
	Buffer FFTBuffer (as UAV)
*/
