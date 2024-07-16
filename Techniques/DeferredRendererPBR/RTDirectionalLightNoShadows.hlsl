// RTPointLight technique, shader NoShadows
/*$(ShaderResources)*/

#include "GBuffer.hlsl"
#include "PBR.hlsl"

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// If light is off, nothing to do
	if (!/*$(Variable:Enabled)*/)
		return;
			
	uint2 dims;
	Output.GetDimensions(dims.x, dims.y);

	// If this is empty sky, no lighting to do	
	uint2 px = DTid.xy;
	float depth = Depth[px];
	if (depth == /*$(Variable:DepthClearValue)*/)
	{
		return;
	}

	// Becode gbuffer
	GBuffer gbuffer = DecodeGBuffer(Albedo_Roughness[px], Normal_Metallic_AO[px]);

	// Get world position of pixel
	float2 screenPos = (float2(px)+0.5f) / float2(dims) * 2.0 - 1.0;
	screenPos.y = -screenPos.y;
	float4 world = mul(float4(screenPos, depth, 1), /*$(Variable:InvViewProjMtx)*/);
	world /= world.w;

	// Get reverse view direction (from world to camera)
	float3 V = normalize(/*$(Variable:CameraPos)*/ - world.xyz);

	float3 litColor = Output[px].rgb;
	litColor += DirectionalLight(world.xyz, gbuffer.normal, V, -/*$(Variable:Direction)*/, /*$(Variable:Color)*/ * /*$(Variable:Intensity)*/, gbuffer.albedo, gbuffer.metallic, gbuffer.roughness, c_F0) * gbuffer.ao;
	Output[px] = float4(litColor, 1.0f);
}
/*
Shader Resources:
	Texture Depth (as SRV)
	Texture Albedo_Roughness (as SRV)
	Texture Normal_Metallic_AO (as SRV)
	Texture Output (as UAV)
*/
