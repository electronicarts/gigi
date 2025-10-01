// Strides technique, shader Init
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	buff.Store(DTid.x * 4, asuint(float(DTid.x)));
	buff2.Store(DTid.x * 4, asuint(0.0f));
}

/*
Shader Resources:
	Buffer buff (as UAV)
	Buffer buff2 (as UAV)
*/
