// Unnamed technique, shader DOF
/*$(ShaderResources)*/

// Implementation of "Bokeh depth of field in a single pass"
// https://blog.voxagon.se/2018/05/04/bokeh-depth-of-field-in-single-pass.html

static const float GOLDEN_ANGLE = 2.39996323; 

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

float getBlurSize(float depth, float focusPoint, float focusScale)
{
	float coc = clamp((1.0 / focusPoint - 1.0 / depth)*focusScale, -1.0, 1.0);
	return abs(coc) * /*$(Variable:MaxBlurSize)*/;
}

float3 depthOfField(uint2 texCoord, float focusPoint, float focusScale)
{
	uint2 ColorSize;
	Color.GetDimensions(ColorSize.x, ColorSize.y);

	float centerDepth = LinearDepth[texCoord].r;
	float centerSize = getBlurSize(centerDepth, focusPoint, focusScale);
	float3 color = Color[texCoord].rgb;
	float tot = 1.0;
	float radius = /*$(Variable:RadiusScale)*/;
	for (float ang = 0.0; radius</*$(Variable:MaxBlurSize)*/; ang += GOLDEN_ANGLE)
	{
		uint2 tc = uint2(texCoord + float2(cos(ang), sin(ang)) * radius + float2(0.5f, 0.5f)); // NOTE: added a half pixel to make floor into round
		tc = min(tc, ColorSize - uint2(1,1));
		float3 sampleColor = Color[tc].rgb;
		float sampleDepth = LinearDepth[tc].r;
		float sampleSize = getBlurSize(sampleDepth, focusPoint, focusScale);
		if (sampleDepth > centerDepth)
			sampleSize = clamp(sampleSize, 0.0, centerSize*2.0);
		float m = smoothstep(radius-0.5, radius+0.5, sampleSize);
		color += lerp(color/tot, sampleColor, m);
		tot += 1.0;   radius += /*$(Variable:RadiusScale)*//radius;
	}

	return color /= tot;
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = uint2(DTid.xy);

	float3 output = depthOfField(px, /*$(Variable:FocalLength)*/, /*$(Variable:FocusScale)*/);

	if (/*$(Variable:sRGB)*/)
		output = LinearToSRGB(output);

	Output[px] = float4(output, 1.0f);

}

/*
Shader Resources:
	Texture Color (as SRV)
	Texture LinearDepth (as SRV)
	Texture output (as UAV)
*/
