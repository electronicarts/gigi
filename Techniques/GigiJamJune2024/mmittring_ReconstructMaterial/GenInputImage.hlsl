// Unnamed technique, shader GenInputImage
/*$(ShaderResources)*/

#include "common.hlsl"



/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	static const uint2 c_Size = uint2/*$(Variable:Size)*/;

	// 0..1
	float2 uv = DTid.xy / (float2)c_Size;

	GBuffer gbuffer = scene(uv);

	// from https://www.shadertoy.com/view/XlKSDR

	float3 lightDirection = float3(-0.6f, -0.7f, 0.7f);

	float3 shading = BRDF(uv, gbuffer, lightDirection, float3(0, 0, -1));

	// shading
	Data[DTid.xy] = float4(shading, 1);

	// visualize stateA
//	Data[DTid.xy] = float4(gbuffer.baseColor, gbuffer.linearRoughness);
	// visualize stateB
//	Data[DTid.xy] = float4(gbuffer.normal * 0.5f + 0.5f, gbuffer.metallic);

	// visualize depth
//	Data[DTid.xy] = float4(gbuffer.depth, gbuffer.depth, gbuffer.depth, 1);
	// visualize normal
//	Data[DTid.xy] = float4(gbuffer.normal * 0.5f + 0.5f, 1);
	// visualize baseColor
//	Data[DTid.xy] = float4(gbuffer.baseColor, 1); // nice colors
	// visualize metallic
//	Data[DTid.xy] = float4(gbuffer.metallic.rrr, 1); // black
	// visualize linearRoughness
//	Data[DTid.xy] = float4(gbuffer.linearRoughness.rrr, 1); // multipl grey levels
}

/*
Shader Resources:
	Texture Data (as UAV)
*/
