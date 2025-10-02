
#include "../RayTracing/RayTracingCommon.hlsl"
#include "../RayTracing/RayTracingGlobalResources.hlsl"
// AnyHit technique, shader raygen


struct Struct_VertexBuffer
{
    float3 position;
    float3 normal;
    float2 uv;
    uint materialId;
};

struct Struct__RayGenCB
{
    float3 CameraPos;
    float _padding0;
    float4x4 InvViewProjMtx;
    float depthNearPlane;
    float3 _padding1;
};

RWTexture2D<float4> output : register(u0, space1);
RaytracingAccelerationStructure scene : register(t0, space0);
StructuredBuffer<Struct_VertexBuffer> vertexBuffer : register(t1, space0);
ConstantBuffer<Struct__RayGenCB> _RayGenCB : register(b0, space0);


struct Payload
{
	bool hit;
	float3 color;
	uint2 px;
};

[shader("raygeneration")]
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
void Miss(inout Payload payload : SV_RayPayload)
{
	payload.hit = false;
}

[shader("closesthit")]
void ClosestHit(inout Payload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	payload.hit = true;
}

[shader("anyhit")]
void AnyHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
	float3 hitLocation = ObjectRayOrigin() + ObjectRayDirection() * RayTCurrent();

	// Screen space checkerboard
	//if ((payload.px.x/4 + payload.px.y/4) % 2 == 0)
	//	IgnoreHit();

	// UV space checkerboard
	float2 uv0 = vertexBuffer[PrimitiveIndex()*3 + 0].uv;
	float2 uv1 = vertexBuffer[PrimitiveIndex()*3 + 1].uv;
	float2 uv2 = vertexBuffer[PrimitiveIndex()*3 + 2].uv;
	float2 uv = uv0 + attr.barycentrics.x * (uv1 - uv0) + attr.barycentrics.y * (uv2 - uv0);
	uint2 uvInt = uint2(uv * 64.0f);
	if ((uvInt.x/4 + uvInt.y/4) % 2 == 0)
		IgnoreHit();

	// use the normal for the color
	float3 normal0 = vertexBuffer[PrimitiveIndex()*3 + 0].normal;
	float3 normal1 = vertexBuffer[PrimitiveIndex()*3 + 1].normal;
	float3 normal2 = vertexBuffer[PrimitiveIndex()*3 + 2].normal;
	float3 normal = normal0 + attr.barycentrics.x * (normal1 - normal0) + attr.barycentrics.y * (normal2 - normal0);
	normal = normalize(normal);
	payload.color = normal * 0.5f + 0.5f;
}

/*
Shader Resources:
	Texture output (as UAV)
	Buffer scene (as RTScene)
	Buffer vertexBuffer (as SRV)
*/
