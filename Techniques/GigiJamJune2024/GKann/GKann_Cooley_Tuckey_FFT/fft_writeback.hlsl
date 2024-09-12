// Unnamed technique, shader fft_writeback
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

/*$(_compute:fftWriteback)*/(uint3 DTid : SV_DispatchThreadID)
{
	float3 outputCol;
	for (uint i = 0; i < N_CHANNELS; i++) {
		outputCol[i] = FFTBuffer[uint3(DTid.x, DTid.y, i)];
    }

	OutputTexture[DTid.xy] = float4(outputCol, 1.0);
}

/*
Shader Resources:
	Buffer FFTBuffer (as SRV)
	Texture OutputTexture (as UAV)
*/
