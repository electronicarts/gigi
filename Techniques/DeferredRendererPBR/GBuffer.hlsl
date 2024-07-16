#include "Octahedral.hlsl"

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

struct GBuffer
{
    float3 albedo;
    float3 normal;
    float roughness;
    float metallic;
    float ao;
};

void EncodeGBuffer(in GBuffer gbuffer, out float4 renderTarget0, out float4 renderTarget1)
{
	renderTarget0 = float4(LinearToSRGB(gbuffer.albedo), gbuffer.roughness);
	renderTarget1 = float4(packOctahedralUnorm(gbuffer.normal), gbuffer.metallic, gbuffer.ao);
}

GBuffer DecodeGBuffer(in float4 renderTarget0, in float4 renderTarget1)
{
    GBuffer ret;
    ret.albedo = SRGBToLinear(renderTarget0.rgb);
    ret.roughness = renderTarget0.a;
    ret.normal = normalize(unpackOctahedralUnorm(renderTarget1.rg));
    ret.metallic = renderTarget1.b;
    ret.ao = renderTarget1.a;
    return ret;
}