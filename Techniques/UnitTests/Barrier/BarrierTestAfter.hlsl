// Barrier technique, shader After
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	/*$(RWTextureW:Output)*/[DTid.xy] = float4(/*$(RWTextureR:Output)*/[DTid.xy].rgb * 2.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
