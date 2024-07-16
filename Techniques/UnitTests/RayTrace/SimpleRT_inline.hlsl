/*$(ShaderResources)*/

float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

/*$(_compute:SimpleRTCS)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	uint w, h;
	g_texture.GetDimensions(w, h);
	float2 dimensions = float2(w, h);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, /*$(Variable:depthNearPlane)*/, 1), /*$(Variable:clipToWorld)*/);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = /*$(Variable:cameraPos)*/;
	ray.TMin = 0;
	ray.TMax = 10000.0f;
	ray.Direction = normalize(world.xyz - ray.Origin);

	RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
		RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
		RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;

	rayQuery.TraceRayInline(
		g_scene,
		0,
		255,
		ray
	);

	rayQuery.Proceed();

	if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
	{
		g_texture[px] = float4(LinearToSRGB(g_vertexBuffer[rayQuery.CandidatePrimitiveIndex()*3].Normal*0.5f+0.5f), 1.0f);
		//g_texture[px] = float4(/*$(Variable:hitColor)*/, 1.0f);
	}
	else
		g_texture[px] = float4(LinearToSRGB(/*$(Variable:missColor)*/), 1.0f);
}
