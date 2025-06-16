// Unnamed technique, shader WriteOutputCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    Output[DTid.xy] = float4(RED, GREEN, 1.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
