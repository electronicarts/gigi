// Unnamed technique, shader Mip1CS


RWTexture2D<float4> Input : register(u0);
RWTexture2D<float4> Output : register(u1);


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
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

    float3 result =
		SRGBToLinear(Input[px * 2 + uint2(0, 0)].rgb) +
		SRGBToLinear(Input[px * 2 + uint2(1, 0)].rgb) +
		SRGBToLinear(Input[px * 2 + uint2(1, 1)].rgb) +
		SRGBToLinear(Input[px * 2 + uint2(0,1)].rgb);
	
	result /= 4.0f;

	Output[px] = float4(LinearToSRGB(result), 1.0f);
}

/*
Shader Resources:
	Texture Input (as UAV)
	Texture Output (as UAV)
*/
