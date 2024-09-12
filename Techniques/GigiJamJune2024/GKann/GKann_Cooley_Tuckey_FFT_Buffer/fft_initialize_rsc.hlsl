// Unnamed technique, shader fft_initialize_rsc
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

/*$(_compute:fftInitializeRsc)*/(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = InputTexture[DTid.xy];
    color = min(color, 15);
    FFTBufferRG[DTid.x + DTid.y * N] = color.rg;
    FFTBufferB[DTid.x + DTid.y * N] = float2(color.b, 0.0);
}

/*
Shader Resources:
	Texture InputTexture (as SRV)
	Buffer FFTBuffer (as UAV)
*/
