// Strides technique, shader Float4RW
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x >= 4)
		return;

	float4 value = buff[DTid.x];
	value *= 2.0f;
	buff2[DTid.x] = value;
}

/*
Shader Resources:
	Buffer buff (as SRV)
	Buffer buff (as UAV)
*/
