// Unnamed technique, shader Write
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// COLOR is assumed to be a #define, set on the compute shader node
	Color[DTid.xy] = COLOR;
	buff[0] = COLOR;
}

/*
Shader Resources:
	Texture Color (as UAV)
	Buffer buff (as UAV)
*/
