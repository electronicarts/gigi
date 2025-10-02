
#include "../RayTracing/RayTracingCommon.hlsl"
#include "../RayTracing/RayTracingGlobalResources.hlsl"
// Texture2DRW_RGS technique, shader RWRGS


RWTexture2D<float4> nodeTexture : register(u0, space1);
RWTexture2D<float4> importedTexture : register(u1, space1);
Texture2D<float4> importedColor : register(t0, space0);
Texture2D<float4> _loadedTexture_0 : register(t1, space0);


struct Payload
{
	bool hit;
};

[shader("raygeneration")]
void rgsmain()
{
	uint2 px = DispatchRaysIndex().xy;

	float3 loadedTexturePx = _loadedTexture_0[px].rgb;
	float3 importedTexturePx = importedTexture[px].rgb;
	float3 importedColorPx = importedColor[px].rgb;

	nodeTexture[px] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[px] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}

[shader("closesthit")]
void chsmain(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
    payload.hit = true;
}

[shader("miss")]
void missmain(inout Payload payload : SV_RayPayload)
{
    payload.hit = false;
}
