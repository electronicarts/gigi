// Unnamed technique, shader PushPullCS
/*$(ShaderResources)*/

// Adapted from https://www.shadertoy.com/view/XsfyRr
// Paper here: https://www.scitepress.org/papers/2009/17726/17726.pdf

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

float4 SampleTexture(in Texture2D<float4> t, in float2 px)
{
	uint2 dims;
	t.GetDimensions(dims.x, dims.y);
	dims /= 2;
	float2 uv = (px + float2(0.5f, 0.5f)) / float2(dims);
	return t.SampleLevel(linearWrapSampler, uv, 0);
}

/*$(_compute:DownSample_1_2)*/(uint3 DTid : SV_DispatchThreadID)
{
	float4 c0 = SampleTexture(Input, float2(DTid.xy) + float2(-0.5f,  0.5f));
	float4 c1 = SampleTexture(Input, float2(DTid.xy) + float2( 0.5f, -0.5f));
	float4 c2 = SampleTexture(Input, float2(DTid.xy) + float2( 0.5f,  0.5f));
	float4 c3 = SampleTexture(Input, float2(DTid.xy) + float2(-0.5f, -0.5f));

	float4 c = float4(0.0f, 0.0f, 0.0f, c0.a + c1.a + c2.a + c3.a);
	if (c.a > 0.0f)
	{
		c.rgb += c0.rgb * c0.a;
		c.rgb += c1.rgb * c1.a;
		c.rgb += c2.rgb * c2.a;
		c.rgb += c3.rgb * c3.a;
		c.rgb /= c.a;
	}

	Output[DTid.xy] = float4(LinearToSRGB(c.rgb), c.a);
}

/*$(_compute:DownSample_2_2)*/(uint3 DTid : SV_DispatchThreadID)
{
	float4 c0 = SampleTexture(Input, float2(DTid.xy) + float2(-0.5f,  0.5f));
	float4 c1 = SampleTexture(Input, float2(DTid.xy) + float2( 0.5f, -0.5f));
	float4 c2 = SampleTexture(Input, float2(DTid.xy) + float2( 0.5f,  0.5f));
	float4 c3 = SampleTexture(Input, float2(DTid.xy) + float2(-0.5f, -0.5f));

	float4 w = float4( min(1.0, c0.a), min(1.0, c1.a), min(1.0, c2.a), min(1.0, c3.a) );

	float4 c = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float sumw = w.x + w.y + w.z + w.w;
	if (sumw > 0.0f)
	{
		c.rgb += c0.rgb * w.x;
		c.rgb += c1.rgb * w.y;
		c.rgb += c2.rgb * w.z;
		c.rgb += c3.rgb * w.w;
		c.a = sumw;
		c.rgb /= c.a;
	}

	Output[DTid.xy] = float4(LinearToSRGB(c.rgb), c.a);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
