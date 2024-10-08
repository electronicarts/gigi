// Unnamed technique, shader ApplyDOF
/*$(ShaderResources)*/

#define dofEqFar float3(/*$(Variable:EqFar)*/ / /*$(Variable:DepthScale)*/, /*$(Variable:FarToNearRadiusRatio)*/)

#define d0 /*$(Variable:fadeDistances)*/.x
#define d1 /*$(Variable:fadeDistances)*/.y
#define d2 /*$(Variable:fadeDistances)*/.z

float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

float3 GetSmallBlurSample(uint2 px)
{
	static const float weight = 4.0f / 17.0f;
	float3 sum = float3(0.0f, 0.0f, 0.0f);

	uint2 dims;
	Color.GetDimensions(dims.x, dims.y);

	// Unblurred sample done by alpha blending
	float2 uv = (float2(px) + float2(0.5f, 0.5f) + float2(+0.5f, -1.5f)) / float2(dims);
	sum += weight * Color.SampleLevel(linearWrapSampler, uv, 0);

	uv = (float2(px) + float2(0.5f, 0.5f) + float2(-1.5f, -0.5f)) / float2(dims);
	sum += weight * Color.SampleLevel(linearWrapSampler, uv, 0);

	uv = (float2(px) + float2(0.5f, 0.5f) + float2(-0.5f, +1.5f)) / float2(dims);
	sum += weight * Color.SampleLevel(linearWrapSampler, uv, 0);

	uv = (float2(px) + float2(0.5f, 0.5f) + float2(+1.5f, +0.5f)) / float2(dims);
	sum += weight * Color.SampleLevel(linearWrapSampler, uv, 0);

	return sum;
}

float4 InterpolateDof(float3 small, float3 med, float3 large, float t)
{
	// Efficiently calculate the cross-blend weights for each sample.
	// Let the unblurred sample to small blur fade happen over distance
	// d0, the small to medium blur over distance d1, and the medium to
	// large blur over distance d2, where d0 + d1 + d2 = 1.
	// dofLerpScale = float4( -1 / d0, -1 / d1, -1 / d2, 1 / d2 );
	// dofLerpBias = float4( 1, (1 – d2) / d1, 1 / d2, (d2 – 1) / d2 );

	float4 dofLerpScale = float4( -1.0f / d0, -1.0f / d1, -1.0f / d2, 1.0f / d2 );
	float4 dofLerpBias = float4( 1.0f, (1.0f - d2) / d1, 1.0f / d2, (d2 - 1.0f) / d2 );

	float4 weights = saturate(t * dofLerpScale + dofLerpBias);
	weights.yz = min(weights.yz, 1 - weights.xy);

	// Unblurred sample with weight "weights.x" done by alpha blending
	float3 color = weights.y * small + weights.z * med + weights.w * large;
	float alpha = dot(weights.yzw, half3(16.0 / 17, 1.0, 1.0));
	return float4(color, alpha);
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	//28-4
	uint2 px = DTid.xy;
	float3 small = GetSmallBlurSample(px);
	float4 med = SmallBlur[px/4];
	float3 large = LargeBlur[px/4].rgb;
	float nearCoc = med.a;
	float depth = LinearDepth[px].r;
	float coc = 0.0f;
	if (depth == 0.0f)
	{
		coc = nearCoc; // We don't want to blur the sky
	}
	else
	{
		// dofEqFar.x and dofEqFar.y specify the linear ramp to convert
		// to depth for the distant out-of-focus region.
		// dofEqFar.z is the ratio of the far to the near blur radius.
		float farCoc = saturate(dofEqFar.x * depth + dofEqFar.y);
		coc = max(nearCoc, farCoc * dofEqFar.z);
	}
	float4 output = InterpolateDof(small, med.rgb, large, coc);

	if (/*$(Variable:sRGB)*/)
		output.rgb = LinearToSRGB(output.rgb);

	Output[px] = float4(output.rgb, 1.0f);
}

/*
Shader Resources:
	Texture Color (as SRV)
	Texture LinearDepth (as SRV)
	Texture LargeBlur (as SRV)
	Texture SmallBlur (as SRV)
	Texture Output (as UAV)
Shader Samplers:
	linearWrapSampler filter: MinMagMipLinear addressmode: Wrap
*/
