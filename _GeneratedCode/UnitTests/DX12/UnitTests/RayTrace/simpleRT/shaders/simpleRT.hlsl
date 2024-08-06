

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
RaytracingAccelerationStructure g_scene : register(t0);
ConstantBuffer<Struct__SimpleRTRayGenCB> _SimpleRTRayGenCB : register(b0);

#line 1

struct Payload
{
	float3 RGB;
};

[shader("raygeneration")]
#line 7
void RayGen()
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

	TraceRay(g_scene,
		RAY_FLAG_FORCE_OPAQUE,
		0xFF,
		0,
		0,
		0,
		ray,
		payload);

	g_texture[px] = float4(payload.RGB, 1.0f);
}

[shader("miss")]
#line 38
void Miss(inout Payload payload : SV_RayPayload)
{
	payload.RGB = _SimpleRTRayGenCB.missColor;
}

[shader("closesthit")]
#line 43
void ClosestHit(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	payload.RGB = _SimpleRTRayGenCB.hitColor;
}