// Unnamed technique, shader BlurFarCS
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

	uint2 FarFieldColorCoCSize;
	FarFieldColorCoC.GetDimensions(FarFieldColorCoCSize.x, FarFieldColorCoCSize.y);

	float2 UVAndScreenPos = (float2(px) + float2(0.5f, 0.5f)) / float2(FarFieldColorCoCSize);

	float4 PixelColor = FarFieldColorCoC[px];
	float PixelCoC = PixelColor.w;

	float3 ResultColor = 0;
	float Weight = 0;
	
	int TAP_COUNT = BLUR_TAP_COUNT; // Higher means less noise and make floodfilling easier after

	// Multiplying by PixelCoC guarantees a smooth evolution of the blur radius
	// especially visible on plane (like the floor) where CoC slowly grows with the distance.
	// This makes all the difference between a natural bokeh and some noticeable
	// in-focus and out-of-focus layer blending
	float radius = KernelSize.x * PixelCoC;

	if (PixelCoC > 0) { // Ignore any pixel not belonging to far field
	
		// Weighted average of the texture samples inside the bokeh pattern
		// High radius and low sample count can create "gaps" which are fixed later (floodfill).
		for (int u = 0; u < TAP_COUNT; ++u)
		{
			for (int v = 0; v < TAP_COUNT; ++v)
			{
				float2 uv = float2(u, v) / (TAP_COUNT - 1); // map to [0, 1]
				uv = SquareToPolygonMapping( uv, KernelSize ) / float2(FarFieldColorCoCSize); // map to bokeh shape, then to texel size
				uv = UVAndScreenPos.xy + radius * uv;

				float4 tapColor = FarFieldColorCoC.SampleLevel(linearClampSampler, uv, 0); //Texture2DSampleLevel(PostprocessInput0, PostprocessInput0Sampler, uv, 0);
				
				// Weighted by CoC. Gives more influence to taps with a CoC higher than us.
				float TapWeight = tapColor.w * saturate(1.0f - (PixelCoC - tapColor.w)); 
				
				ResultColor +=  tapColor.xyz * TapWeight; 
				Weight += TapWeight;
			}
		}
		if (Weight > 0) ResultColor /= Weight;
		Weight = Weight / TAP_COUNT / TAP_COUNT;
	}
	
	Weight = saturate(Weight * 10); // From CoC 0.1, completely rely on the far field layer and stop lerping with in-focus layer
	float4 OutColor = float4(ResultColor, Weight);

	BlurredFarFieldColorAlpha[px] = OutColor;
}

/*
Shader Resources:
	Texture FarFieldColorCoC (as SRV)
	Texture BlurredFarFieldColorAlpha (as UAV)
*/
