// Unnamed technique, shader NearBlur
/*$(ShaderResources)*/

#include "Common.hlsli"

#define BLUR_TAP_COUNT /*$(Variable:BlurTapCount)*/

// .x : size of the bokeh blur radius in texel space
// .y : rotation in radius to apply to the bokeh shape
// .z : Number of edge of the polygon (number of blades). 0: circle. 4: square, 6: hexagon...
#define KernelSize /*$(Variable:KernelSize)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	uint2 NearFieldColorCoCBorderSize;
	NearFieldColorCoCBorder.GetDimensions(NearFieldColorCoCBorderSize.x, NearFieldColorCoCBorderSize.y);

	float2 UVAndScreenPos = (float2(px) + float2(0.5f, 0.5f)) / float2(NearFieldColorCoCBorderSize);

	float4 PixelColor = NearFieldColorCoCBorder[px];// Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, UVAndScreenPos.xy, 0);
	float PixelCoC = NearmaxCoCTilemap_1_8_Halo[px/4];// Texture2DSampleLevel(PostprocessInput1, PostprocessInput1Sampler, UVAndScreenPos.xy, 0); 

	float3 ResultColor = 0;
	float Weight = 0;
	int TAP_COUNT = BLUR_TAP_COUNT; // Higher means less noise and make floodfilling easier after
	
	float radius = KernelSize.x * 0.7f * PixelCoC; 
	
	if (PixelCoC > 0) { // Early exit based on MaxCoC tilemap
		for (int u = 0; u < TAP_COUNT; ++u)
		{
			for (int v = 0; v < TAP_COUNT; ++v)
			{
				float2 uv = float2(u, v) / (TAP_COUNT - 1); // map to [0, 1]
				uv = SquareToPolygonMapping( uv, KernelSize ) / float2(NearFieldColorCoCBorderSize); // map to bokeh shape, then to texel size
				uv = UVAndScreenPos.xy + radius * uv;

				float4 tapColor = NearFieldColorCoCBorder.SampleLevel(linearClampSampler, uv, 0);//  Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, uv, 0);
				float TapWeight = saturate(tapColor.w * 10.0f); // From CoC 0.1 rely only on the near field and stop lerping with in-focus area 
				
				ResultColor +=  tapColor.xyz * TapWeight; 
				Weight += TapWeight;
			}
		}
		
		ResultColor /= (Weight + 0.0000001f);
		Weight /= (TAP_COUNT * TAP_COUNT);
	}
	float4 OutColor = float4(ResultColor, Weight);	

	NearFieldColorCoCBorderBlurred[px] = OutColor;
}

/*
Shader Resources:
	Texture NearFieldColorCoCBorder (as SRV)
	Count NearmaxCoCTilemap_1_8_Halo (as Count)
	Texture NearFieldColorCoCBorderBlurred (as UAV)
*/
