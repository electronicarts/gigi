// BufferViewDescriptorTable technique, shader WriteValue
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	TheBuffer[DTid.x] = WRITE_VALUE;
}

/*
Shader Resources:
	Buffer TheBuffer (as UAV)
*/
