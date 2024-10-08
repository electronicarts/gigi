// Unnamed technique, shader GaussBlurCS
/*$(ShaderResources)*/

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

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	int2 px = DTid.xy;
	int2 maxPx;
	Input.GetDimensions(maxPx.x, maxPx.y);
	maxPx -= int2(1,1);

	// initialize values
	float weight = 0.0f;
	float3 color = float3(0.0f, 0.0f, 0.0f);

	// loop horizontally or vertically, as appropriate
	for (int index = -/*$(Variable:Radius)*/; index <= /*$(Variable:Radius)*/; ++index)
	{
		int2 offset = (BLURH) ? int2(index, 0) : int2(0, index);
		int2 readPx = clamp(px + offset, int2(0, 0), maxPx);
		color += Input[readPx].rgb;
		weight += 1.0f;
	}

	// normalize blur
	color /= weight;

	if (/*$(Variable:sRGB)*/)
		color = LinearToSRGB(color);

	Output[px] = float4(color, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
