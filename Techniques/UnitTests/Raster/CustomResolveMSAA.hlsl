
/*$(ShaderResources)*/

[numthreads(8, 8, 1)]
void mainCS(uint3 DTid : SV_DispatchThreadID)
{
	uint2 pxPos = DTid.xy;

	// hard coded for 4x MSAA
#if 0
	uint sampleId = (pxPos.x%2) + 2 * (pxPos.y%2);
	float4 ret = TextureIn.Load(pxPos / uint2(2,2), sampleId);
#else
	float4 ret = 0;
	for(uint i = 0; i < 4; ++i)
	{
		float4 value = TextureIn.Load(pxPos, i);
		ret += value;
	}
	ret /= 4;
	ret.a = 1;
#endif

	TextureOut[pxPos] = ret;
}
