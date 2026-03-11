// Unnamed technique, shader ModelViewerResetPTCS
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    AccumPT[DTid.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

/*
Shader Resources:
	Texture AccumPT (as UAV)
*/
