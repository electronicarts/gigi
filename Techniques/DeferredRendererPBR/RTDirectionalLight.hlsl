// RTDirectionalLight technique, shader RayGen
/*$(ShaderResources)*/

#include "GBuffer.hlsl"
#include "PBR.hlsl"

struct Payload
{
	bool hit;
};

/*$(_raygeneration:RayGen)*/
{
	// If light is off, nothing to do
	if (!/*$(Variable:Enabled)*/)
		return;

	// If this is empty sky, no lighting to do	
	uint2 px = DispatchRaysIndex().xy;
	float depth = Depth[px];
	if (depth == /*$(Variable:DepthClearValue)*/)
		return;

	// Becode gbuffer
	GBuffer gbuffer = DecodeGBuffer(Albedo_Roughness[px], Normal_Metallic_AO[px]);

	// Get world position of pixel
	float2 screenPos = (float2(px)+0.5f) / float2(DispatchRaysDimensions().xy) * 2.0 - 1.0;
	screenPos.y = -screenPos.y;
	float4 world = mul(float4(screenPos, depth, 1), /*$(Variable:InvViewProjMtx)*/);
	world /= world.w;

	// bias the ray to avoid false self shadowing
	world.xyz += gbuffer.normal * /*$(Variable:RayNormalBias)*/;

	// Get reverse view direction (from world to camera)
	float3 V = normalize(/*$(Variable:CameraPos)*/ - world.xyz);

	// shoot the ray
	RayDesc ray;
	ray.Origin = world.xyz;
	ray.Direction = -normalize(/*$(Variable:Direction)*/);
	ray.TMin = /*$(Variable:RayTMin)*/;
	ray.TMax = /*$(Variable:RayTMax)*/;

	Payload payload = (Payload)0;
	payload.hit = true;

	/*$(RayTraceFn)*/(Scene, // Scene (TLAS) buffer
		RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // Ray flags
		0xFF, // Ray mask
		/*$(RTHitGroupIndex:EmptyHitGroup)*/,
		0,
		/*$(RTMissIndex:Miss)*/,
		ray,
		payload);

	if (payload.hit)
		return;

	float3 litColor = Output[px].rgb;
	litColor += DirectionalLight(world.xyz, gbuffer.normal, V, -/*$(Variable:Direction)*/, /*$(Variable:Color)*/ * /*$(Variable:Intensity)*/, gbuffer.albedo, gbuffer.metallic, gbuffer.roughness, c_F0) * gbuffer.ao;
	Output[px] = float4(litColor, 1.0f);
}

/*$(_miss:Miss)*/
{
	payload.hit = false;
}

/*
Shader Resources:
	Buffer Scene (as RTScene)
	Texture Depth (as SRV)
	Texture Albedo_Roughness (as SRV)
	Texture Normal_Metallic_AO (as SRV)
	Texture Output (as UAV)
*/
