// Unnamed technique, shader NearHaloCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	
	float PixelColor = NearMaxCocTilemap_1_8[px];//Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, tapUV, 0); 
	
	if (PixelColor == 0) // Only fill the empty areas around near field
	{
		PixelColor = 0;
		float Weight = 0;
		int RADIUS_TAPS = 4; // 8x8 taps, but shouldn't be heavy at such low resolution
		for (int u = -RADIUS_TAPS; u <= RADIUS_TAPS; ++u)
		{
			for (int v = -RADIUS_TAPS; v <= RADIUS_TAPS; ++v)
			{
				float tapValue = NearMaxCocTilemap_1_8[int2(px) + int2(u, v)];//Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, tapUV + float2(u,v) * PostprocessInput0Size.zw, 0); 
				float tapWeight = tapValue == 0.0f? 0.0f : 1.0f;
				PixelColor += tapWeight * tapValue;
				Weight += tapWeight;
			}
		}
		PixelColor /= (Weight + 0.000001f);
	}
	
	float4 OutColor = PixelColor;

	NearMaxCocTilemap_1_8_Halo[px] = OutColor;
}

/*
Shader Resources:
	Texture NearMaxCocTilemap_1_8 (as SRV)
	Texture NearMaxCocTilemap_1_8_Halo (as UAV)
*/
