// Demofox_ChaosGame technique, shader CSInit
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	Points[DTid.x].Position = float3(0.0f, 0.0f, 0.0f);
}
