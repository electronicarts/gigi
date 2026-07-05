// IndirectDispatch technique, shader Clear Render Target
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	/*$(RWTextureW:Render_Target)*/[DTid.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

/*
Shader Resources:
	Texture Render Target (as UAV)
*/
