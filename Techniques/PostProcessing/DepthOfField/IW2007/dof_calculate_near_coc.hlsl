// Unnamed technique, shader CalculateNearCoC
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	//28-2
	uint2 px = DTid.xy;
	float4 shrunk = shrunkSampler[px];
	float4 blurred = blurredSampler[px];
	float3 color = shrunk.rgb;
	float coc = 2.0f * max(blurred.a, shrunk.a) - shrunk.a;
	Output[px] = float4(color, coc);
}

/*
Shader Resources:
	Texture shrunkSampler (as SRV)
	Texture blurredSampler (as SRV)
	Texture Output (as UAV)
*/
