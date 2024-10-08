// Unnamed technique, shader NormalizeCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	float maxMagnitude = asfloat(MaxMagnitude[0]);
	uint2 px = DTid.xy;
	float value = DFTMagnitude[px];
	if (/*$(Variable:LogSpaceMagnitude)*/)
		DFTMagnitude[px] = log(1.0f + value * 255.0f);
	else
		DFTMagnitude[px] = value / maxMagnitude;
}

/*
Shader Resources:
	Texture DFTMagnitude (as UAV)
	Buffer MaxMagnitude (as SRV)
*/
