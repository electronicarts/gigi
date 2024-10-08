// Unnamed technique, shader NearBorderCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	
	float4 PixelColor = NearFieldColorCoC[px];// Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, tapUV, 0); 
	
	if (PixelColor.w == 0) // Only fill the empty areas around near field
	{
		PixelColor = 0;
		float Weight = 0;
		int RADIUS_TAPS = 1;
		for (int u = -RADIUS_TAPS; u <= RADIUS_TAPS; ++u)
		{
			for (int v = -RADIUS_TAPS; v <= RADIUS_TAPS; ++v)
			{
				float4 tapValue = NearFieldColorCoC[px + int2(u,v)];//Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, tapUV + float2(u,v) * PostprocessInput0Size.zw, 0); 
				float tapWeight = tapValue.w == 0.0f? 0.0f : 1.0f;
				PixelColor += tapWeight * tapValue;
				Weight += tapWeight;
			}
		}
		PixelColor /= (Weight + 0.0000001f);
		PixelColor.w = 0;
	}
	
	float4 OutColor = PixelColor;

	NearFieldColorCoCBorder[px] = OutColor;
}

/*
Shader Resources:
	Texture NearFieldColorCoC (as SRV)
	Texture NearFieldColorCoCBorder (as UAV)
*/
