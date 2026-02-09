// Box Blur Shader/*$(ShaderResources)*/

/*$(Embed:boxblur_embed.hlsl)*/

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

float3 SRGBToLinear(in float3 sRGBCol)
{
	float3 linearRGBLo = sRGBCol / 12.92;
	float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
	float3 linearRGB;
	linearRGB.r = sRGBCol.r <= 0.04045 ? linearRGBLo.r : linearRGBHi.r;
	linearRGB.g = sRGBCol.g <= 0.04045 ? linearRGBLo.g : linearRGBHi.g;
	linearRGB.b = sRGBCol.b <= 0.04045 ? linearRGBLo.b : linearRGBHi.b;
	return linearRGB;
}

/*$(_compute:BlurH)*/(uint3 DTid : SV_DispatchThreadID)
{
    int radius = GetRadius();
    uint w, h;
    Input.GetDimensions(w, h);

    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = -radius; i <= radius; i++)
        result += Input.Load(int3((DTid.xy + int2(i, 0)) % int2(w, h), 0)) / (float(radius) * 2.0f + 1.0f);

    if (/*$(Variable:sRGB)*/)
        Output[DTid.xy] = float4(LinearToSRGB(result.rgb), result.a);
    else
        Output[DTid.xy] = result;
}

/*$(_compute:BlurV)*/(uint3 DTid : SV_DispatchThreadID)
{
    int radius = GetRadius();
    uint w, h;
    Input.GetDimensions(w, h);

    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = -radius; i <= radius; i++)
        result += Input.Load(int3((DTid.xy + int2(0, i)) % int2(w, h), 0)) / (float(radius) * 2.0f + 1.0f);

    if (/*$(Variable:sRGB)*/)
        Output[DTid.xy] = float4(LinearToSRGB(result.rgb), result.a);
    else
        Output[DTid.xy] = result;
}
