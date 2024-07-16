// Unnamed technique, shader csmain
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	tex[DTid.xy] = float4(0.25f, 0.75f, 1.0f, 4.0f);
}
