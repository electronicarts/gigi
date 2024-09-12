/*$(ShaderResources)*/

#include "RenderCommon.hlsli"
#include "BattleConstants.hlsli"

#define ART_WIREFRAME 1

struct PSOutput
{
	float4 color    : SV_Target0;
};

float3 ConvertColor(float r, float g, float b)
{
	return pow(float3(r, g, b) / 255.0f, 2.0f);
}

PSOutput psmain(VertexData input)
{
	const float iTime = /*$(Variable:iTime)*/;
	PSOutput result = (PSOutput)0;

	float3 BackgroundColor = ConvertColor(29, 30, 24);

	#if ART_WIREFRAME
	float Desaturation = 0.05;
	float3 TeamColors[4] = {
		lerp(ConvertColor(0, 255, 0), 1, Desaturation), // Green
		lerp(ConvertColor(255, 125, 0), 1, Desaturation), // Orange
		lerp(ConvertColor(255, 0, 0), 1, Desaturation), // Red
		lerp(ConvertColor(255, 255, 0), 1, Desaturation), // Yellow
	};

	#else
	float3 TeamColors[4] = {
		ConvertColor(98, 131, 149), // Red
		ConvertColor(238, 108, 77), // Blue
		ConvertColor(105, 153, 93), // Green
		ConvertColor(254, 239, 221), // White
	};
	#endif

	const Struct_EntityState state = EntityStates[input.entityidx];

	float3 color = TeamColors[state.Team].rgb;
	float opacity = 1.0f;

	#if ART_WIREFRAME
	float wireWidth = 1.0;
	float3 d = fwidth(input.barycentric);
	float3 f = smoothstep(0, d * wireWidth, input.barycentric) ;
	float edgeFactor = min(min(f.x, f.y), f.z);
 	opacity = 1 - edgeFactor;
	#endif

	if (input.meshPart == 0) // Ship
	{
		float TimeSinceSpawn = iTime - state.SpawnTime;
		opacity *= saturate(TimeSinceSpawn / RespawnFadeIn);
	}
	if (input.meshPart == 1) // Laser
	{
		float TimeSinceLastShot = iTime - state.LastLaserFireTime;
		opacity *= saturate(1 - TimeSinceLastShot / LaserFadeTime) * LaserBaseOpacity;
	}
	if (input.meshPart == 2) // Explosion
	{
		color = lerp(color, 1, 0.2);
		opacity *= pow(saturate(1 - (iTime - state.DeathTime) / ExplosionFadeTime), 1);
	}

	result.color = float4(color, opacity);
	
	return result;
}