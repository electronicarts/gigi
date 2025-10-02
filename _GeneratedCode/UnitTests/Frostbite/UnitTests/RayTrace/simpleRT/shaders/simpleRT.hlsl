
#include <Systems/Render/Raytrace/RaytraceBase.hlsl>

// TODO: need to make ABI for PS support
FB_ABI_BEGIN(simpleRT_SimpleRTRayGen_Abi)
FB_ABI_END


struct Struct_VertexBuffer
{
    float3 Color;
    float3 Position;
};

struct Struct__SimpleRTRayGenCB
{
    float3 cameraPos;
    float _padding0;
    float4x4 clipToWorld;
    float depthNearPlane;
    float3 hitColor;
    float3 missColor;
    float _padding1;
};

RWTexture2D<float4> g_texture : register(u0);
RaytracingAccelerationStructure g_scene : FB_RT_SCENE_REGISTER(t0);
cbuffer cb0 : register(b0)
{
	Struct__SimpleRTRayGenCB _SimpleRTRayGenCB;
};

struct Payload
{
	float3 RGB;
};

#define TraceARay(AccelerationStructure, RayFlags, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex, Ray, payload) \
	fb::rt::traceRay<simpleRT_SimpleRTRayGen_Abi, FB_RT_FLAGS_UseSharedStack | FB_RT_FLAGS_DefaultStackLds, RayFlags> \
		(AccelerationStructure, Ray, payload, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex, MissShaderIndex);

FB_DEFINE_RAYGEN_SHADER(RayGen, 8, 8, 1)
{
	uint2 px = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, _SimpleRTRayGenCB.depthNearPlane, 1), _SimpleRTRayGenCB.clipToWorld);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = _SimpleRTRayGenCB.cameraPos;
	ray.TMin = 0;
	ray.TMax = 1000.0f;
	ray.Direction = normalize(world.xyz - ray.Origin);

	Payload payload = (Payload)0;

	TraceARay(g_scene,
		RAY_FLAG_FORCE_OPAQUE,
		0xFF,
		0,
		0,
		0,
		ray,
		payload);

	g_texture[px] = float4(payload.RGB, 1.0f);
}

FB_DEFINE_MISS_SHADER(simpleRT_SimpleRTRayGen_Abi, Miss, defaultLsrt, Payload, payload)
{
	payload.RGB = _SimpleRTRayGenCB.missColor;
}

FB_DEFINE_CLOSEST_HIT_SHADER(simpleRT_SimpleRTRayGen_Abi, ClosestHit, defaultLsrt, Payload, payload)
{
	payload.RGB = _SimpleRTRayGenCB.hitColor;
}