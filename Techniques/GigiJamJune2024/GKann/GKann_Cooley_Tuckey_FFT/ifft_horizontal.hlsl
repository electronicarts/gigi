// Unnamed technique, shader ifft_horizontal
/*$(ShaderResources)*/

#include "fft_core.hlsl"

/*$(_compute:ifftHorizontal)*/(uint local_index : SV_GroupIndex, uint3 workgroup_id : SV_GroupID)
{
	fft(local_index, workgroup_id.x, 0, true);
}

/*
Shader Resources:
	Buffer FFTBuffer (as UAV)
*/
