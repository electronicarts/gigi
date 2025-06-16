// SubGraphTest technique, shader Swap Colors
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	/*$(RWTextureW:Output)*/[DTid.xy].rgba = /*$(RWTextureR:Output)*/[DTid.xy].gbra;
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
