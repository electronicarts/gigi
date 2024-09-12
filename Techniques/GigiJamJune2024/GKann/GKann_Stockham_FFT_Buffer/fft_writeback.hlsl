// Unnamed technique, shader fft_writeback
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

/*$(_compute:fftWriteback)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint id = DTid.x + DTid.y * N;
	float4 outputCol = float4(FFTBufferRG[id].x, FFTBufferRG[id].y, FFTBufferB[id].x, 1.0);
	OutputTexture[DTid.xy] = outputCol;
}

/*
Shader Resources:
	Buffer FFTBufferR (as SRV)
	Buffer FFTBufferG (as SRV)
	Buffer FFTBufferB (as SRV)
	Texture OutputTexture (as UAV)
*/
