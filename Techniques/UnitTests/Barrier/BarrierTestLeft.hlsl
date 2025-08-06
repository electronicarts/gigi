// Barrier technique, shader Left
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	/*$(RWTextureW:Output)*/[DTid.xy] = float4(0.5f, 0.0f, 0.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
