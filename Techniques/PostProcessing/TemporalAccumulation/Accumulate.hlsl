// Unnamed technique, shader TemporalAccum
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	float alpha = (/*$(Variable:Reset)*/ || !/*$(Variable:Enabled)*/)
		? 1.0f
		: /*$(Variable:Alpha)*/;

	uint2 px = DTid.xy;

	float4 oldValue = Accum[px];
	float4 newValue = Input[px];

	Accum[px] = lerp(oldValue, newValue, alpha);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Accum (as UAV)
*/
