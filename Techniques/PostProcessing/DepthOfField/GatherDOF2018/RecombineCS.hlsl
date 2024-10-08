// Unnamed technique, shader RecombineCS
/*$(ShaderResources)*/

#include "Common.hlsli"

// Blend in-focus / far / near layers on the top of each others
float3 GetCombinedLayerColor(uint2 px, float2 uv, in ViewStruct View)
{
	// Set FadePower to 1.0f in the next line to be physically correct,
	// but some in-focus silhouette might become visible at the border of the
	// near field, where the gradient smoothly fades alpha to 0. [0 -> 0.5 -> 1]
	// Limitation of screen-space effect: we don't have access to the actual color occluded by near field meshes.
	// This makes the near-field fading gradient more aggressive [0 -> 1 -> 1] to hide any sharp silhouette.
	float FadePower = 2.0f; 
#if 1 // Optimized version using early-out to avoid unnecessary texture fetches -> ~25% speed-up in reference scene.
	
	float4 NearColor = /*$(Variable:DoNearField)*/ ? NearField.SampleLevel(linearClampSampler, uv, 0) : float4(0.0f, 0.0f, 0.0f, 0.0f);// Texture2DSampleLevel(PostprocessInput2, PostprocessInput2Sampler, uv, 0);
	
	NearColor.w = saturate(NearColor.w * FadePower);
	
	// Pure near field, early exit
	if (NearColor.w == 1.0f) 
	{
		return NearColor.rgb;
	}
	
	float4 FarColor = /*$(Variable:DoFarField)*/ ? FarField.SampleLevel(linearClampSampler, uv, 0) : float4(0.0f, 0.0f, 0.0f, 0.0f);// Texture2DSampleLevel(PostprocessInput1, PostprocessInput1Sampler, uv, 0);
	
	// Original CoC to guarantee crisp edge of in-focus over far-field
	//float2 PixelPosCenter = SvPosition.xy;
	//float2 FullResUV = PixelPosCenter * PostprocessInput0Size.zw;
	float SceneDepth = Depth[px];// CalcSceneDepth(FullResUV);
	
	bool isInFocus = (SceneDepth >= View.DepthOfFieldFocalDistance) &&
						(SceneDepth < View.DepthOfFieldFocalDistance + View.DepthOfFieldFocalRegion);
	
	if (!isInFocus) {
		// Pure far field without any bleeding from near field, early exit
		if (FarColor.w == 1.0f && NearColor.w == 0.0f )
		{
			return FarColor.rgb;
		}
		
		// Blending only between far and near
		if (FarColor.w == 1.0f) {
			return float3( NearColor.w * (NearColor.rgb) + (1.0f - NearColor.w) * FarColor.rgb );
		}
	} else {
		// Pixel was originally in focus, background should never bleed onto it
		FarColor.w = 0;
	}
	
	// Worst case: 3 layer merge
	float3 FocusColor = Color[px].rgb;// Texture2DSample(PostprocessInput0, PostprocessInput0Sampler, uv).rgb;
	
	float3 Result = FarColor.w * FarColor.rgb + (1.0f - FarColor.w) * FocusColor;
	Result =  NearColor.w * (NearColor.rgb) + (1.0f - NearColor.w) * Result;
	
	return Result;
	
#else // Original generic version
	
	float3 FocusColor = Texture2DSample(PostprocessInput0, PostprocessInput0Sampler, uv).rgb;
	float4 FarColor = Texture2DSampleLevel(PostprocessInput1, PostprocessInput1Sampler, uv, 0);
	float4 NearColor = Texture2DSampleLevel(PostprocessInput2, PostprocessInput2Sampler, uv, 0);
	
	// Original CoC to guarantee crisp edge of in-focus over far-field
	float2 PixelPosCenter = SvPosition.xy;
	float2 FullResUV = PixelPosCenter * PostprocessInput0Size.zw;
	float SceneDepth = CalcSceneDepth(FullResUV);
	
	bool isInFocus = (SceneDepth >= View.DepthOfFieldFocalDistance) &&
						(SceneDepth < View.DepthOfFieldFocalDistance + View.DepthOfFieldFocalRegion);
	if (isInFocus) FarColor.w = 0;
	
	// Alpha composite far field on the top of the original scene.
	float3 Result = FarColor.w * FarColor.rgb + (1.0f - FarColor.w) * FocusColor;
	
	// Alpha composite on the near field
	if (NearColor.w > 0) {
		float blendFactor = saturate(NearColor.w * FadePower);
		Result =  blendFactor * (NearColor.rgb) + (1.0f - blendFactor) * Result;
	}
	
	return Result;
#endif		
	
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	ViewStruct View = (ViewStruct)0;
	View.DepthOfFieldFocalLength = /*$(Variable:FocalLength)*/;
	View.DepthOfFieldFocalDistance = /*$(Variable:FocalDistance)*/;
	View.DepthOfFieldFocalRegion = /*$(Variable:FocalRegion)*/;
	View.DepthOfFieldNearTransitionRegion = /*$(Variable:NearTransitionRegion)*/;
	View.DepthOfFieldFarTransitionRegion = /*$(Variable:FarTransitionRegion)*/;
	View.DepthOfFieldScale = /*$(Variable:Scale)*/;
	uint2 px = DTid.xy;

	uint2 OutputSize;
	Output.GetDimensions(OutputSize.x, OutputSize.y);
	float2 uv = (float2(px) + float2(0.5f, 0.5f)) / float2(OutputSize);

	/*
#if RECOMBINE_METHOD == 2 // Separate translucency
	// SceneColor in full res
	float2 PixelPosCenter = SvPosition.xy;
	float2 FullResUV = PixelPosCenter * PostprocessInput0Size.zw;
	float4 SeparateTranslucency = UpsampleSeparateTranslucency(SvPosition.xy, FullResUV, PostprocessInput3, PostprocessInput3Size.zw);
#endif	
*/
	float3 RecombinedLayersColor = GetCombinedLayerColor(px, uv, View);

/*	
#if RECOMBINE_METHOD == 2
		// Separate translucency as premultiplied alpha
		RecombinedLayersColor.rgb = RecombinedLayersColor.rgb * SeparateTranslucency.a + SeparateTranslucency.rgb;
#endif	
*/

	if (/*$(Variable:sRGB)*/)
		RecombinedLayersColor = LinearToSRGB(RecombinedLayersColor);

	Output[px] = float4(RecombinedLayersColor, 1.0f);
}

/*
Shader Resources:
	Texture Color (as SRV)
	Texture FarField (as SRV)
	Texture NearField (as SRV)
	Texture Depth (as SRV)
	Texture Output (as UAV)
*/
