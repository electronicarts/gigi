// PushPull technique, shader PushPullUpSample
/*$(ShaderResources)*/

// Adapted from https://www.shadertoy.com/view/XsfyRr
// Paper here: https://www.scitepress.org/papers/2009/17726/17726.pdf

static const float4 upsample_w = float4(9.0/16.0, 3.0/16.0, 3.0/16.0, 1.0/16.0);

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

/*$(_compute:UpSample_1_2)*/(uint3 DTid : SV_DispatchThreadID)
{
	float4 c0 = Level1[DTid.xy];

	uint2 level2px = DTid.xy / 2;
	float4 g1[4];
	g1[0] = Level2[level2px + uint2(0, 0)];
	g1[1] = Level2[level2px + uint2(1, 0)];
	g1[2] = Level2[level2px + uint2(0, 1)];
	g1[3] = Level2[level2px + uint2(1, 1)];

	g1[0].a *= upsample_w[0];
	g1[1].a *= upsample_w[1];
	g1[2].a *= upsample_w[2];
	g1[3].a *= upsample_w[3];

	float4 c1 = float4(
		g1[0].rgb * g1[0].a +
		g1[1].rgb * g1[1].a +
		g1[2].rgb * g1[2].a +
		g1[3].rgb * g1[3].a,
		g1[0].a + g1[1].a + g1[2].a + g1[3].a);

	if (c1.a > 0.0f)
	{
		c1.rgb /= c1.a;
		float4 result = lerp(c1, c0, min(1.0f, c0.a));
		Output[DTid.xy] = float4(LinearToSRGB(result.rgb), result.a);
	}
	else
	{
		Output[DTid.xy] = c0;
	}
}

/*$(_compute:UpSample_2_2)*/(uint3 DTid : SV_DispatchThreadID)
{
	float4 c0 = Level1[DTid.xy];

	uint2 level2px = DTid.xy / 2;
	float4 g1[4];
	g1[0] = Level2[level2px + uint2(0, 0)];
	g1[1] = Level2[level2px + uint2(1, 0)];
	g1[2] = Level2[level2px + uint2(0, 1)];
	g1[3] = Level2[level2px + uint2(1, 1)];

	g1[0].a *= upsample_w[0];
	g1[1].a *= upsample_w[1];
	g1[2].a *= upsample_w[2];
	g1[3].a *= upsample_w[3];

	float4 c1 = float4(
		g1[0].rgb * g1[0].a +
		g1[1].rgb * g1[1].a +
		g1[2].rgb * g1[2].a +
		g1[3].rgb * g1[3].a,
		g1[0].a + g1[1].a + g1[2].a + g1[3].a);

	if (c1.a > 0.0f)
	{
		c1.rgb /= c1.a;
		float4 result = lerp(c1, c0, min(1.0f, c0.a));
		Output[DTid.xy] = float4(LinearToSRGB(result.rgb), result.a);
	}
	else
	{
		Output[DTid.xy] = c0;
	}	
}

/*
Shader Resources:
	Texture Level1 (as SRV)
	Texture Level2 (as SRV)
	Texture Output (as UAV)
*/
