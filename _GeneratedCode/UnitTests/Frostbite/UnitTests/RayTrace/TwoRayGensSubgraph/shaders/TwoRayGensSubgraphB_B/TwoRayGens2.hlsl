// TwoRayGens technique

#include <Systems/Render/Raytrace/RaytraceBase.hlsl>

// TODO: need to make ABI for PS support
FB_ABI_BEGIN(TwoRayGensSubgraph_B_TwoRayGens2_Abi)
FB_ABI_END

struct Struct__B_TwoRayGens2CB
{
    float3 cameraPos;
    float _padding0;
    float4x4 clipToWorld;
    float depthNearPlane;
    float3 _padding1;
};

RWTexture2D<float4> g_texture : register(u0);
RaytracingAccelerationStructure g_scene : FB_RT_SCENE_REGISTER(t0);
Texture2D<float4> g_blueChannel : register(t0);
cbuffer cb0 : register(b0)
{
	Struct__B_TwoRayGens2CB _B_TwoRayGens2CB;
};


struct Payload
{
	bool hit;
	float blueChannelMultiplier;
};

#define TraceARay(AccelerationStructure, RayFlags, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex, Ray, payload) \
	fb::rt::traceRay<TwoRayGensSubgraph_B_TwoRayGens2_Abi, FB_RT_FLAGS_UseSharedStack | FB_RT_FLAGS_DefaultStackLds, RayFlags> \
		(AccelerationStructure, Ray, payload, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex);

FB_DEFINE_RAYGEN_SHADER(RayGen2, 8, 8, 1)
{
	uint2 px = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, _B_TwoRayGens2CB.depthNearPlane, 1), _B_TwoRayGens2CB.clipToWorld);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = _B_TwoRayGens2CB.cameraPos;
	ray.Direction = normalize(world.xyz - ray.Origin);
	ray.TMin = 0;
	ray.TMax = 1000.0f;

	Payload payload = (Payload)0;

	int missShaderIndex = (px.y < dimensions.y / 2) ? 1 : 1;

	TraceARay(g_scene, // Scene (TLAS) buffer
		RAY_FLAG_FORCE_OPAQUE, // Ray flags
		0xFF, // Ray mask
		0,
		0,
		missShaderIndex,
		ray,
		payload);

	float4 color = g_texture[px];
	color.a = 1.0f;
	color.g = payload.hit ? 1.0f : 0.0f;

	uint2 blueChannelDims;
	g_blueChannel.GetDimensions(blueChannelDims.x, blueChannelDims.y);
	color.b = dot(g_blueChannel[px % blueChannelDims].rgb, float3(0.3f, 0.59f, 0.11f)) * payload.blueChannelMultiplier;

	g_texture[px] = color;
}

FB_DEFINE_MISS_SHADER(TwoRayGensSubgraph_B_TwoRayGens2_Abi, Miss2A, defaultLsrt, Payload, payload)
{
	payload.hit = false;
	payload.blueChannelMultiplier = 0.25f;
}

FB_DEFINE_MISS_SHADER(TwoRayGensSubgraph_B_TwoRayGens2_Abi, Miss2B, defaultLsrt, Payload, payload)
{
	payload.hit = false;
	payload.blueChannelMultiplier = 1.0f;
}

FB_DEFINE_CLOSEST_HIT_SHADER(TwoRayGensSubgraph_B_TwoRayGens2_Abi, ClosestHit2, defaultLsrt, Payload, payload)
{
	payload.hit = true;
	payload.blueChannelMultiplier = 0.0f;
}

/*
Shader Resources:
	Texture g_texture (as UAV)
	Buffer g_scene (as RTScene)
	Texture g_blueChannel (as SRV)
*/
