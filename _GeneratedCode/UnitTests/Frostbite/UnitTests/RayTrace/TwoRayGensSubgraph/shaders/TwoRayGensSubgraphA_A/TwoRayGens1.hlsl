
#include <Systems/Render/Raytrace/RaytraceBase.hlsl>

// TODO: need to make ABI for PS support
FB_ABI_BEGIN(TwoRayGensSubgraph_A_TwoRayGens1_Abi)
FB_ABI_END
// TwoRayGens technique


struct Struct_VertexBufferSimple
{
    float3 Position;
};

struct Struct__A_TwoRayGens1CB
{
    float3 cameraPos;
    float _padding0;
    float4x4 clipToWorld;
    float depthNearPlane;
    float3 _padding1;
};

RWTexture2D<float4> g_texture : register(u0);
RaytracingAccelerationStructure g_scene : FB_RT_SCENE_REGISTER(t0);
cbuffer cb0 : register(b0)
{
	Struct__A_TwoRayGens1CB _A_TwoRayGens1CB;
};


struct Payload
{
	bool hit;
};

#define TraceARay(AccelerationStructure, RayFlags, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex, Ray, payload) \
	fb::rt::traceRay<TwoRayGensSubgraph_A_TwoRayGens1_Abi, FB_RT_FLAGS_UseSharedStack | FB_RT_FLAGS_DefaultStackLds, RayFlags> \
		(AccelerationStructure, Ray, payload, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex);

FB_DEFINE_RAYGEN_SHADER(RayGen1, 8, 8, 1)
{
	uint2 px = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, _A_TwoRayGens1CB.depthNearPlane, 1), _A_TwoRayGens1CB.clipToWorld);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = _A_TwoRayGens1CB.cameraPos;
	ray.Direction = normalize(world.xyz - ray.Origin);
	ray.TMin = 0;
	ray.TMax = 1000.0f;

	Payload payload = (Payload)0;

	TraceARay(g_scene, // Scene (TLAS) buffer
		RAY_FLAG_FORCE_OPAQUE, // Ray flags
		0xFF, // Ray mask
		0,
		0,
		1,
		ray,
		payload);

	float4 color = g_texture[px];
	color.a = 1.0f;
	color.r = payload.hit ? 1.0f : 0.0f;
	g_texture[px] = color;
}

FB_DEFINE_MISS_SHADER(TwoRayGensSubgraph_A_TwoRayGens1_Abi, Miss1, defaultLsrt, Payload, payload)
{
	payload.hit = false;
}

FB_DEFINE_CLOSEST_HIT_SHADER(TwoRayGensSubgraph_A_TwoRayGens1_Abi, ClosestHit1, defaultLsrt, Payload, payload)
{
	payload.hit = true;
}

/*
Shader Resources:
	Texture g_texture (as UAV)
	Buffer g_scene (as RTScene)
*/
