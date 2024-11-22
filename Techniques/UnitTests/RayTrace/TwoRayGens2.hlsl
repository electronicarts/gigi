// TwoRayGens technique
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
	float blueChannelMultiplier;
};

/*$(_raygeneration:RayGen2)*/
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

	int missShaderIndex = (px.y < dimensions.y / 2) ? /*$(RTMissIndex:TwoRayGensMiss2A)*/ : /*$(RTMissIndex:TwoRayGensMiss2B)*/;

	/*$(RayTraceFn)*/(g_scene, // Scene (TLAS) buffer
		RAY_FLAG_FORCE_OPAQUE, // Ray flags
		0xFF, // Ray mask
		/*$(RTHitGroupIndex:HitGroup2)*/,
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

/*$(_miss:Miss2A)*/
{
	payload.hit = false;
	payload.blueChannelMultiplier = 0.25f;
}

/*$(_miss:Miss2B)*/
{
	payload.hit = false;
	payload.blueChannelMultiplier = 1.0f;
}

/*$(_closesthit:ClosestHit2)*/
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
