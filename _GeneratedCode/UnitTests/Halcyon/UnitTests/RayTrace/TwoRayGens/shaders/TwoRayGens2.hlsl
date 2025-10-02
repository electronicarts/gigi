
#include "../RayTracing/RayTracingCommon.hlsl"
#include "../RayTracing/RayTracingGlobalResources.hlsl"
// TwoRayGens technique


struct Struct__TwoRayGens2CB
{
    float3 cameraPos;
    float _padding0;
    float4x4 clipToWorld;
    float depthNearPlane;
    float3 _padding1;
};

RWTexture2D<float4> g_texture : register(u0, space1);
RaytracingAccelerationStructure g_scene : register(t0, space0);
Texture2D<float4> g_blueChannel : register(t1, space0);
ConstantBuffer<Struct__TwoRayGens2CB> _TwoRayGens2CB : register(b0, space0);


struct Payload
{
	bool hit;
	float blueChannelMultiplier;
};

[shader("raygeneration")]
void RayGen2()
{
	uint2 px = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, _TwoRayGens2CB.depthNearPlane, 1), _TwoRayGens2CB.clipToWorld);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = _TwoRayGens2CB.cameraPos;
	ray.Direction = normalize(world.xyz - ray.Origin);
	ray.TMin = 0;
	ray.TMax = 1000.0f;

	Payload payload = (Payload)0;

	int missShaderIndex = (px.y < dimensions.y / 2) ? 0 : 1;

	TraceRay(g_scene, // Scene (TLAS) buffer
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

[shader("miss")]
void Miss2A(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
	payload.blueChannelMultiplier = 0.25f;
}

[shader("miss")]
void Miss2B(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
	payload.blueChannelMultiplier = 1.0f;
}

[shader("closesthit")]
void ClosestHit2(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
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
