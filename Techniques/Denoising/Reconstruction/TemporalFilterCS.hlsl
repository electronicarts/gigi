// Unnamed technique, shader TemporalFilterCS
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

/*$(_compute:TemporalFilterCS)*/(uint3 DTid : SV_DispatchThreadID)
{
	float3 accum = Accum[DTid.xy].rgb;
	float3 source = Source[DTid.xy].rgb;
	accum = lerp(accum, source, /*$(Variable:TemporalAlpha)*/);
	Accum[DTid.xy] = float4(accum, 1.0f);
}

/*
Shader Resources:
	Texture Source (as SRV)
	Texture Accum (as UAV)
*/
