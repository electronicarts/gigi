// AnyHit technique, shader raygen
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
	float3 color;
	uint2 px;
};

/*$(_raygeneration:RayGen)*/
{
	uint2 px = DispatchRaysIndex().xy;
	uint2 dimensions = DispatchRaysDimensions().xy;

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, /*$(Variable:depthNearPlane)*/, 1), /*$(Variable:InvViewProjMtx)*/);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = /*$(Variable:CameraPos)*/;
	ray.TMin = 0;
	ray.TMax = 1000.0f;
	ray.Direction = normalize(world.xyz - ray.Origin);

	Payload payload = (Payload)0;
	payload.px = px;

	/*$(RayTraceFn)*/(scene, // Scene (TLAS) buffer
		0, // Ray flags
		0xFF, // Ray mask
		/*$(RTHitGroupIndex:HitGroup)*/,
		0,
		/*$(RTMissIndex:Miss)*/,
		ray,
		payload);

	output[px] = float4(payload.hit ? payload.color : float3(0.2f, 0.2f, 0.2f), 1.0f);
}

/*$(_miss:Miss)*/
{
	payload.hit = false;
}

/*$(_closesthit:ClosestHit)*/
{
	payload.hit = true;
}

/*$(_anyhit:AnyHit)*/
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
