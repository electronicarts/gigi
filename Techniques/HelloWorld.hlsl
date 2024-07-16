// HelloWorld technique, shader Hello World
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	uint2 dims;
	Output.GetDimensions(dims.x, dims.y);

	float timeInSeconds = /*$(Variable:iTime)*/;

	float2 uv = float2(px) / float2(dims);
	uv = clamp((uv + float2(cos(timeInSeconds*0.5f), sin(timeInSeconds*0.3f))), 0.0f, 1.0f);

	Output[px] = float4(uv, sin(timeInSeconds)*0.5f + 0.5f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
