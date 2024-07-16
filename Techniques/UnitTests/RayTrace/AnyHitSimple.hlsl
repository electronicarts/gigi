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
