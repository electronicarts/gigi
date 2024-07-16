// Intersection technique
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
	float3 normal;
};

struct HitAttributes
{
	float3 normal;
};

/*$(_raygeneration:RayGen)*/
{
	uint2 px = DispatchRaysIndex().xy;
	float2 dimensions = float2(DispatchRaysDimensions().xy);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, 0.0f, 1), /*$(Variable:InvViewProjMtx)*/);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = /*$(Variable:CameraPos)*/;
	ray.Direction = normalize(world.xyz - ray.Origin);
	ray.TMin = 0;
	ray.TMax = 1000.0f;

	Payload payload = (Payload)0;
	payload.hit = true;

	/*$(RayTraceFn)*/(Scene, // Scene (TLAS) buffer
		0, // Ray flags
		0xFF, // Ray mask
		/*$(RTHitGroupIndex:HitGroup0)*/,
		0,
		/*$(RTMissIndex:Miss)*/,
		ray,
		payload);

	if (payload.hit)
		Output[px] = float4(payload.normal * 0.5f + 0.5f, 1.0f);
	else
		Output[px] = float4(0.2f, 0.2f, 0.2f, 1.0f);
}

/*$(_miss:Miss)*/
{
	payload.hit = false;
}

float TestSphereTrace(in float3 rayPos, in float3 rayDir, in float4 sphere, out float3 normal)
{
	//get the vector from the center of this sphere to where the ray begins.
	float3 m = rayPos - sphere.xyz;

	//get the dot product of the above vector and the ray's vector
	float b = dot(m, rayDir);

	float c = dot(m, m) - sphere.w * sphere.w;

	//exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
	if(c > 0.0 && b > 0.0)
		return -1.0f;

	//calculate discriminant
	float discr = b * b - c;

	//a negative discriminant corresponds to ray missing sphere
	if(discr < 0.0)
		return -1.0f;
    
	//ray now found to intersect sphere, compute smallest t value of intersection
    bool fromInside = false;
	float dist = -b - sqrt(discr);
    if (dist < 0.0f)
    {
        fromInside = true;
        dist = -b + sqrt(discr);
    }

	normal = normalize((rayPos+rayDir*dist) - sphere.xyz) * (fromInside ? -1.0f : 1.0f);

	return dist;
}

/*$(_closesthit:ClosestHit:HitAttributes)*/
{
	payload.normal = intersection.normal;
}

/*$(_anyhit:AnyHit:HitAttributes)*/
{
	if (attr.normal.y > 0.5f && attr.normal.y < 0.6f)
		IgnoreHit();
}

/*$(_intersection:Intersection)*/
{
	HitAttributes attr;

	uint primIndex = PrimitiveIndex();

	float3 AABBMin = float3(AABBs[primIndex*6+0], AABBs[primIndex*6+1], AABBs[primIndex*6+2]);
	float3 AABBMax = float3(AABBs[primIndex*6+3], AABBs[primIndex*6+4], AABBs[primIndex*6+5]);

	float3 AABBMid = (AABBMin + AABBMax) / 2.0f;
	float3 AABBSize = AABBMax - AABBMin;
	float AABBMinSize = min(AABBSize.x, min(AABBSize.y, AABBSize.z));

	float3 rayPos = ObjectRayOrigin() - AABBMid;
	float3 rayDir = ObjectRayDirection();

	float hitT = TestSphereTrace(rayPos, rayDir, float4(0.0f, 0.0f, 0.0f, AABBMinSize / 2.0f), attr.normal);

	if (hitT > 0.0f)
	{
		ReportHit(hitT, 0, attr);
	}
	else
	{
		// uncomment this to see the bounds of the boxes. useful for debugging
		//attr.normal = float3(1.0f, 0.0f, 0.0f);
		//ReportHit(0.0f, 0, attr);
	}
}

/*
Shader Resources:
	Buffer Scene (as RTScene)
	Buffer AABBs (as SRV)
	Texture Output (as UAV)
*/
