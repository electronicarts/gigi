// Unnamed technique, shader ClearCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    Output[DTid.xy] = float4(0.5f, 0.5f, 0.5f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
