// Unnamed technique, shader SaveCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float2 uv = (float2(px) + 0.5f) / float2(/*$(Variable:RenderSize)*/);
	ColorSDR[px] = float4(uv, 0.0f, 1.0f);
	ColorHDR[px] = float4(uv * 3.0f - 1.0f, 0.0f, 1.0f);
}

/*
Shader Resources:
	Texture ColorSDR (as UAV)
	Texture ColorHDR (as UAV)
*/
