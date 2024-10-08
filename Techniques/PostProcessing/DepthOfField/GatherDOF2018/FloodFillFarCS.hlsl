// Unnamed technique, shader FloodFillFarCS
/*$(ShaderResources)*/

#include "Common.hlsli"

#define FLOODFILL_TAP_COUNT /*$(Variable:FloodFillTapCount)*/

// .x : size of the bokeh blur radius in texel space
// .y : rotation in radius to apply to the bokeh shape
// .z : Number of edge of the polygon (number of blades). 0: circle. 4: square, 6: hexagon...
#define KernelSize /*$(Variable:KernelSize)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	uint2 ColorCocSize;
	ColorCoc.GetDimensions(ColorCocSize.x, ColorCocSize.y);

	float2 UVAndScreenPos = (float2(px) + float2(0.5f, 0.5f)) / float2(ColorCocSize);

	#if COC_TILEMAP	

	// pass through if this is disabled
	if (!/*$(Variable:DoNearFieldFloodFill)*/)
	{
		BlurredFieldColorAlpha[px] = ColorCoc[px];
		return;
	}

	#else

	// pass through if this is disabled
	if (!/*$(Variable:DoFarFieldFloodFill)*/)
	{
		BlurredFieldColorAlpha[px] = ColorCoc[px];
		return;
	}

	#endif

	float4 PixelColor = ColorCoc[px];
	float PixelCoC = PixelColor.w;	

#if COC_TILEMAP	
	// Use max CoC tilemap instead of the original pixel CoC for near field
	PixelCoC = MaxCoCTileMap[px/4].r;
#endif

	float4 ResultColor = PixelColor;
	
	float radius = PixelCoC * 1.75f * KernelSize.x  / 15.0f;
	
	int TAP_COUNT = FLOODFILL_TAP_COUNT; // Higher count improves the floodfill
	
	if (PixelCoC > 0) { // Early out if we're outside the field
	
		// McIntosh12 http://ivizlab.sfu.ca/papers/cgf2012.pdf
		// Keep the maximum of all the samples
		for (int u = 0; u < TAP_COUNT; ++u)
		{
			for (int v = 0; v < TAP_COUNT; ++v)
			{
				float2 uv = float2(u, v) / (TAP_COUNT - 1); // map to [0, 1]
				uv = SquareToPolygonMapping( uv, KernelSize ) / float2(ColorCocSize); // map to bokeh shape, then to texel size
				uv = UVAndScreenPos.xy + radius * uv;

				float4 tapColor = ColorCoc.SampleLevel(linearClampSampler, uv, 0);//Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, uv, 0);
				ResultColor = max(ResultColor, tapColor);
			}
		}
	}
#if COC_TILEMAP		
	float4 OutColor = ResultColor;
#else
	// do not touch alpha of far field
	float4 OutColor = float4(ResultColor.xyz, PixelCoC);
#endif

	BlurredFieldColorAlpha[px] = OutColor;
}

/*
Shader Resources:
	Texture ColorCoc (as SRV)
	Texture MaxCoCTileMap (as SRV)
	Texture BlurredFieldColorAlpha (as UAV)
*/
