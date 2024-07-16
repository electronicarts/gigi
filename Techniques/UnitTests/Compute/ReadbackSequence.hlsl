// ReadbackSequence technique, shader ReadbackSequenceCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	float3 c = float3(0.0f, 0.0f, 0.0f);

	if (/*$(Variable:frameIndex)*/ & 1)
		c.b = 1.0f;

	if (/*$(Variable:frameIndex)*/ & 2)
		c.g = 1.0f;

	if (/*$(Variable:frameIndex)*/ & 4)
		c.r = 1.0f;

	//float frameIndex = float(/*$(Variable:frameIndex)*/ % 256) / 255.0f;

	output[DTid.xy] = float4(c, 1.0f);
}
