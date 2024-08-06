// AnyHit technique, shader raygen


struct Struct__RayGenCB
{
    float3 CameraPos;
    float _padding0;
    float4x4 InvViewProjMtx;
    float depthNearPlane;
    float3 _padding1;
};

RWTexture2D<float4> output : register(u0);
RaytracingAccelerationStructure scene : register(t0);
ConstantBuffer<Struct__RayGenCB> _RayGenCB : register(b0);

#line 2


struct Payload
{
	bool hit;
	float3 color;
	uint2 px;
};

[shader("raygeneration")]
#line 11
void RayGen()
{
	uint2 px = DispatchRaysIndex().xy;
	uint2 dimensions = DispatchRaysDimensions().xy;

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, _RayGenCB.depthNearPlane, 1), _RayGenCB.InvViewProjMtx);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = _RayGenCB.CameraPos;
	ray.TMin = 0;
	ray.TMax = 1000.0f;
	ray.Direction = normalize(world.xyz - ray.Origin);

	Payload payload = (Payload)0;
	payload.px = px;

	TraceRay(scene, // Scene (TLAS) buffer
		0, // Ray flags
		0xFF, // Ray mask
		0,
		0,
		0,
		ray,
		payload);

	output[px] = float4(payload.hit ? payload.color : float3(0.2f, 0.2f, 0.2f), 1.0f);
}

[shader("miss")]
#line 43
void Miss(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
}

[shader("closesthit")]
#line 48
void ClosestHit(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	payload.hit = true;
}

[shader("anyhit")]
#line 53
void AnyHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
	// Screen space checkerboard
	if ((payload.px.x/4 + payload.px.y/4) % 2 == 0)
		IgnoreHit();
}

/*
Shader Resources:
	Texture output (as UAV)
	Buffer scene (as RTScene)
	Buffer vertexBuffer (as SRV)
*/
