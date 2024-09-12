// Unnamed technique, shader fft_horizontal
/*$(ShaderResources)*/

#include "fft_core.hlsl"

/*$(_compute:fftHorizontal)*/(uint local_index : SV_GroupIndex, uint3 workgroup_id : SV_GroupID)
{
	fft(local_index, workgroup_id.x, false);
}

/*
Shader Resources:
	Buffer FFTBuffer (as UAV)
*/
