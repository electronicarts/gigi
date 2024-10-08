// Unnamed technique, shader BlurNearCoC
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// 28-3
	uint2 px = DTid.xy;

	uint2 dims;
	CalculatedNearCoC.GetDimensions(dims.x, dims.y);

	float4 texCoords = (float2(px) + float2(0.5f, 0.5f)).xxyy + float4(-0.5f, 0.5f, -0.5f, 0.5f);
	texCoords /= float4(dims.xxyy);

	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	color += CalculatedNearCoC.SampleLevel(linearClampSampler, texCoords.xz, 0);
	color += CalculatedNearCoC.SampleLevel(linearClampSampler, texCoords.yz, 0);
	color += CalculatedNearCoC.SampleLevel(linearClampSampler, texCoords.xw, 0);
	color += CalculatedNearCoC.SampleLevel(linearClampSampler, texCoords.yw, 0);

	Output[px] = color / 4.0f;

	// TODO: maybe the input is the full sized color image, even though the output isnt?
	// The instructions seem to contradict each other a bit
}

/*
Shader Resources:
	Texture CalculatedNearCoC (as SRV)
	Texture Output (as UAV)
*/
