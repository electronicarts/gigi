// Strides technique, shader Float16RW
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	//if (DTid.x < 32)
		//return;

	float16_t value = buff[DTid.x];
	value += 0.1f;
	buff2[DTid.x] = float16_t(value);
	//buff2[DTid.x] = half(float(DTid.x) + 0.1f);
}

/*
Shader Resources:
	Buffer buff (as SRV)
	Buffer buff2 (as UAV)
*/
