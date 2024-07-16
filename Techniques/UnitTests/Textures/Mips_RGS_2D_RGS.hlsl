// Unnamed technique, shader RayGen
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
};

/*$(_raygeneration:main)*/
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

	/*$(RayTraceFn)*/(Scene, // Scene (TLAS) buffer
		RAY_FLAG_FORCE_OPAQUE, // Ray flags
		0xFF, // Ray mask
		/*$(RTHitGroupIndex:EmptyHG)*/,
		/*$(RTHitGroupCount)*/,
		/*$(RTMissIndex:Miss)*/,
		ray,
		payload);

	uint2 OutputDims;
	Output.GetDimensions(OutputDims.x, OutputDims.y);
	if (px.x < OutputDims.x && px.y < OutputDims.y)
		Output[px] = float4(0.2, 0.4, 0.6, 0.8);
}

/*$(_miss:Miss)*/
{
	payload.hit = true;
}

/*
Shader Resources:
	Texture Mip1 (as UAV)
	Buffer Scene (as RTScene)
*/
