// SubGraphTest technique, shader Swap Colors
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	Output[DTid.xy].rgba = Output[DTid.xy].gbra;
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
