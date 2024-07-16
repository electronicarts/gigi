/*$(ShaderResources)*/
static const uint RaytracingInstanceMaskAll			 = 0xFF;
static const float c_pi = 3.14159265359f;
static const float c_twopi = 2.0f * c_pi;

struct Payload
{
	float result;
};

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

[shader("raygeneration")]
void RTAORayGen()
{
	uint2 px = DispatchRaysIndex().xy;

	float depth = g_depth[px];
	if (depth == /*$(Variable:depthClearValue)*/)
	{
		g_texture[px] = 1.0f;
		return;
	}

	// Calculate screen position of this pixel
	float2 screenPos = (float2(px)+0.5f) / DispatchRaysDimensions().xy * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	// Calculate world position of the pixel, at the depth location in the depth buffer
	float4 world = mul(float4(screenPos, depth, 1), /*$(Variable:InvViewProjMtx)*/);
	world.xyz /= world.w;

	// Get tangent frame
	float3 wsnormal = normalize(2.0f * (float3(g_gbuffer[uint3(px,0)].xyz) / 255.0f) - 1.0f);
	float4 tan = g_gbuffer[uint3(px,2)];
	float3 wstangent = 2.0f * (float3(tan.xyz) / 255.0f) - 1.0f;
	wstangent = normalize(wstangent - wsnormal * dot(wsnormal, wstangent));
	float3 wsbitangent = normalize(cross(wsnormal, wstangent));
	if (tan.w == 0.0f)
		wsbitangent *= -1.0f;

	// Get the vector from the texture
	uint2 noiseDims;
	g_noiseTexture.GetDimensions(noiseDims.x, noiseDims.y);
	float3 dir = g_noiseTexture[px % noiseDims].rgb;

	if (/*$(Variable:SourceIsUnorm)*/)
		dir = dir * 2.0f - 1.0f;

	dir = normalize(dir);

	// Convert the direction from world space to tangent space
	if (/*$(Variable:ApplyInTangentSpace)*/)
	{
		float3x3 TBN = float3x3(
			wstangent,
			wsbitangent,
			wsnormal
		);

		dir = mul(dir, TBN);
	}

	if (dot(dir, wsnormal) < 0.0f)
		dir *= -1.0f;

	// make ray desc
	RayDesc ray;
	ray.Origin = world.xyz;
	ray.TMin = /*$(Variable:rayMin)*/;
	ray.TMax = /*$(Variable:rayMax)*/;
	ray.Direction = dir;

	Payload payload = (Payload)0;

	// Shoot the ray and get the result in payload
	TraceRay(g_scene,
		RAY_FLAG_FORCE_OPAQUE,
		RaytracingInstanceMaskAll,
		/*$(RTHitGroupIndex:HitGroup0)*/,
		0,
		/*$(RTMissIndex:RTAOMiss)*/,
		ray,
		payload);

	// Write out AO
	g_texture[px] = LinearToSRGB(payload.result.xxx).x;
}

[shader("miss")]
void RTAOMiss(inout Payload payload : SV_RayPayload)
{
	payload.result = 1.0f;
}

[shader("closesthit")]
void RTAOClosestHit(inout Payload payload : SV_RayPayload,
				 BuiltInTriangleIntersectionAttributes intersection : SV_IntersectionAttributes)
{
	if(/*$(Variable:distanceIsShade)*/)
	{
		float percentTime = RayTCurrent() / /*$(Variable:rayMax)*/;
		payload.result = pow(percentTime, /*$(Variable:AOPower)*/);
	}
	else
	{
		payload.result = 0.0f;
	}
}
