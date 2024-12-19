// Unnamed technique, shader TextureViewerCS
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
	uint2 px = DTid.xy;
	float4 color = source[px];

	// TODO: you can modify the color here
	//color.r = 1.0f;

	if ((bool)/*$(Variable:sRGB)*/)
		color.rgb = LinearToSRGB(color.rgb);
	dest[px] = color;
}

/*
Shader Resources:
	Texture source (as SRV)
	Texture dest (as UAV)
*/
