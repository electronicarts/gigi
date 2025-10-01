// BufferViewDescriptorTable technique, shader Init
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	TheBuffer[DTid.x] = 0.0f;
}

/*
Shader Resources:
	Buffer TheBuffer (as UAV)
*/
