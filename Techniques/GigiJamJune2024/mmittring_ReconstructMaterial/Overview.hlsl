// SlangAutoDiff technique, shader ComputeGradient
/*$(ShaderResources)*/

#define SLANG
#include "common.hlsl"





[shader("compute")]
/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	static const uint2 c_Size = uint2/*$(Variable:Size)*/;

	// a tile is Size x Size
	int2 tilePos = DTid.xy / c_Size.x;

	int2 tileLocalPos = DTid.xy % c_Size.x;

	// 0..1
	float2 uv = tileLocalPos / (float2)c_Size;
	
	// G-Buffer

	// float4(albedo color 0..1, roughness 0..1)
	float4 stateA = saturate(StateA[tileLocalPos.xy]);
	// float4(normal mapped to 0..1, metallic 0..1)
	float4 stateB = saturate(StateB[tileLocalPos.xy]);
	
	GBuffer targetgbuffer = scene(uv);
	float4 targetStateA = float4(targetgbuffer.baseColor, targetgbuffer.linearRoughness);
	float4 targetStateB = float4(targetgbuffer.normal * 0.5f + 0.5f, targetgbuffer.metallic);

	GBuffer gbuffer;
	gbuffer.depth = 0.5f;
	gbuffer.normal = normalize(stateB.xyz * 2 - 1);
	gbuffer.baseColor = stateA.xyz;
	gbuffer.metallic = stateB.w;
	gbuffer.linearRoughness = stateA.w;
	float3 shadedResult = BRDF(uv, gbuffer, float3(-0.6f, -0.7f, 0.7f), float3(0, 0, -1));
	
	float3 targetColor = BRDF(uv, targetgbuffer, float3(-0.6f, -0.7f, 0.7f), float3(0, 0, -1));
//	float3 targetColor = InputImage[tileLocalPos.xy].rgb;

	float4 output = float4(0, 0, 0, 0);

	if (tilePos.x == 0)
		output = targetStateA;
	else if(tilePos.x == 1)
		output = targetStateB;
	if (tilePos.x == 2)
		output = float4(targetColor, 0);
	else if (tilePos.x == 3)
		output = stateA;
	else if(tilePos.x == 4)
		output = stateB;
	else if (tilePos.x == 5)
		output = float4(shadedResult, 0);

	if(tilePos.y == 0)
		OverviewImage[DTid.xy] = float4(output.xyz, 1);
	else if(tilePos.y == 1)
		OverviewImage[DTid.xy] = float4(output.www, 1);
}

/*
Shader Resources:
	Texture InputImage (as SRV)
	Texture OutputGradient (as UAV)
	Texture StateA (as UAV)
	Texture StateB (as UAV)
	Texture OverviewImage (as UAV)
*/
