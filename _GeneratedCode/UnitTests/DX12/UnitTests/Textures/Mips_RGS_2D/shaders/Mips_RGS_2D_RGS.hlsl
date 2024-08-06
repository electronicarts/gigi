// Unnamed technique, shader RayGen


RWTexture2D<float4> Output : register(u0);
RaytracingAccelerationStructure Scene : register(t0);

#line 2


struct Payload
{
	bool hit;
};

[shader("raygeneration")]
#line 9
void main()
{
	uint2 px = DispatchRaysIndex().xy;
	uint2 dims = DispatchRaysDimensions().xy;

	RayDesc ray;
	//ray.Origin =
	//ray.Direction =
	ray.TMin = 0;
	ray.TMax = 1000.0f;

	Payload payload = (Payload)0;
	payload.hit = false;

	TraceRay(Scene, // Scene (TLAS) buffer
		RAY_FLAG_FORCE_OPAQUE, // Ray flags
		0xFF, // Ray mask
		0,
		1,
		0,
		ray,
		payload);

	uint2 OutputDims;
	Output.GetDimensions(OutputDims.x, OutputDims.y);
	if (px.x < OutputDims.x && px.y < OutputDims.y)
		Output[px] = float4(0.2, 0.4, 0.6, 0.8);
}

[shader("miss")]
#line 38
void Miss(inout Payload payload : SV_RayPayload)
{
	payload.hit = true;
}

/*
Shader Resources:
	Texture Mip1 (as UAV)
	Buffer Scene (as RTScene)
*/
