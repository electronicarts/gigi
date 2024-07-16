// Barrier technique, shader After
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	Output[DTid.xy] = float4(Output[DTid.xy].rgb * 2.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
