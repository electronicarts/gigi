// TwoRayGens technique
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
};

/*$(_raygeneration:RayGen1)*/
{
	uint2 px = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, /*$(Variable:depthNearPlane)*/, 1), /*$(Variable:clipToWorld)*/);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = /*$(Variable:cameraPos)*/;
	ray.Direction = normalize(world.xyz - ray.Origin);
	ray.TMin = 0;
	ray.TMax = 1000.0f;

	Payload payload = (Payload)0;

	/*$(RayTraceFn)*/(g_scene, // Scene (TLAS) buffer
		RAY_FLAG_FORCE_OPAQUE, // Ray flags
		0xFF, // Ray mask
		/*$(RTHitGroupIndex:HitGroup1)*/,
		0,
		/*$(RTMissIndex:TwoRayGensMiss1)*/,
		ray,
		payload);

	float4 color = g_texture[px];
	color.a = 1.0f;
	color.r = payload.hit ? 1.0f : 0.0f;
	g_texture[px] = color;
}

/*$(_miss:Miss1)*/
{
	payload.hit = false;
}

/*$(_closesthit:ClosestHit1)*/
{
	payload.hit = true;
}

/*
Shader Resources:
	Texture g_texture (as UAV)
	Buffer g_scene (as RTScene)
*/
