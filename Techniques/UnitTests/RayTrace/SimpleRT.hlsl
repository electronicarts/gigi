/*$(ShaderResources)*/
struct Payload
{
	float3 RGB;
};

/*$(_raygeneration:RayGen)*/
{
	uint2 px = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, /*$(Variable:depthNearPlane)*/, 1), /*$(Variable:clipToWorld)*/);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = /*$(Variable:cameraPos)*/;
	ray.TMin = 0;
	ray.TMax = 1000.0f;
	ray.Direction = normalize(world.xyz - ray.Origin);

	Payload payload = (Payload)0;

	/*$(RayTraceFn)*/(g_scene,
		RAY_FLAG_FORCE_OPAQUE,
		0xFF,
		/*$(RTHitGroupIndex:HitGroup0)*/,
		0,
		/*$(RTMissIndex:SimpleRTMiss)*/,
		ray,
		payload);

	/*$(RWTextureW:g_texture)*/[px] = float4(payload.RGB, 1.0f);
}

/*$(_miss:Miss)*/
{
	payload.RGB = /*$(Variable:missColor)*/;
}

/*$(_closesthit:ClosestHit)*/
{
	payload.RGB = /*$(Variable:hitColor)*/;
}