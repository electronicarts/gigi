// Unnamed technique, shader Blur


struct Struct__FilterSub_Iteration_3_BlurCB
{
    uint FilterSub_Iteration_3_sRGB;
    int __loopIndexValue_3;
    float2 _padding0;
};

Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);
ConstantBuffer<Struct__FilterSub_Iteration_3_BlurCB> _FilterSub_Iteration_3_BlurCB : register(b0);

#line 2


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
#line 26
void main(uint3 DTid : SV_DispatchThreadID)
{
	int2 px = int2(DTid.xy);

	int2 dims;
	Input.GetDimensions(dims.x, dims.y);

	int radius = _FilterSub_Iteration_3_BlurCB.__loopIndexValue_3 + 1;
	float3 ret = float3(0.0f, 0.0f, 0.0f);
	for (int iy = -1; iy <= 1; ++iy)
	{
		for (int ix = -1; ix <= 1; ++ix)
		{
			int2 readpx = (px + int2(ix, iy) * radius + dims) % dims;
			ret += Input[readpx].rgb;
		}
	}
	ret /= 9.0f;

	if (_FilterSub_Iteration_3_BlurCB.FilterSub_Iteration_3_sRGB)
		ret = LinearToSRGB(ret);

	Output[px] = float4(ret, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
