// Box Blur Shader

struct Struct__BoxBlurCB
{
    int radius;
    uint sRGB;
    float2 _padding0;
};

Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);
ConstantBuffer<Struct__BoxBlurCB> _BoxBlurCB : register(b0);

#line 1


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

[numthreads(8, 8, 1)]
#line 25
void BlurH(uint3 DTid : SV_DispatchThreadID)
{
    int radius = _BoxBlurCB.radius;
    uint w, h;
    Input.GetDimensions(w, h);

    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = -radius; i <= radius; i++)
        result += Input.Load(int3((DTid.xy + int2(i, 0)) % int2(w, h), 0)) / (float(radius) * 2.0f + 1.0f);

    if (_BoxBlurCB.sRGB)
        Output[DTid.xy] = float4(LinearToSRGB(result.rgb), result.a);
    else
        Output[DTid.xy] = result;
}

[numthreads(8, 8, 1)]
#line 42
void BlurV(uint3 DTid : SV_DispatchThreadID)
{
    int radius = _BoxBlurCB.radius;
    uint w, h;
    Input.GetDimensions(w, h);

    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = -radius; i <= radius; i++)
        result += Input.Load(int3((DTid.xy + int2(0, i)) % int2(w, h), 0)) / (float(radius) * 2.0f + 1.0f);

    if (_BoxBlurCB.sRGB)
        Output[DTid.xy] = float4(LinearToSRGB(result.rgb), result.a);
    else
        Output[DTid.xy] = result;
}
