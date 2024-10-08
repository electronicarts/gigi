// Unnamed technique, shader BrightLight
/*$(ShaderResources)*/

#define FLT_MAX		3.402823466e+38

// returns -1.0 on miss, else returns time to hit
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

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	uint2 dims;
	Color.GetDimensions(dims.x, dims.y);

	// Calculate a ray for this pixel
	float2 screenPos = float2(px) / float2(dims) * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, /*$(Variable:DepthNearPlane)*/, 1), /*$(Variable:InvViewProjMtx)*/);
	world.xyz /= world.w;

	float3 rayPos = /*$(Variable:CameraPos)*/;
	float3 rayDir = normalize(world.xyz - /*$(Variable:CameraPos)*/);

	// Shoot the ray at the spheres and compare hit time against depth buffer
	float sphereHitT = FLT_MAX;
	for (uint i = 0; i < /*$(Variable:SphereCount)*/; ++i)
	{
		float percent = (float(i) + 0.5f) / float(/*$(Variable:SphereCount)*/);
		float3 spherePosition = lerp(/*$(Variable:SpheresPositionStart)*/, /*$(Variable:SpheresPositionEnd)*/, percent);

		float3 normal;
		float hitT = TestSphereTrace(rayPos, rayDir, float4(spherePosition, /*$(Variable:SphereRadius)*/), normal);
		if (hitT >= 0.0f)
			sphereHitT = min(hitT, sphereHitT);
	}

	// If no sphere hit, or none hit before the depth buffer value, nothing to do.
	if (LinearDepth[px] < sphereHitT)
		return;

	// draw the sphere hit to the color buffer and linear depth buffer
	LinearDepth[px] = sphereHitT;

	Color[px] = float4(/*$(Variable:SphereColor)*/*/*$(Variable:SphereBrightness)*/, 1.0f);
}

/*
Shader Resources:
	Texture Color (as UAV)
	Texture LinearDepth (as UAV)
*/
