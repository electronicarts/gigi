// Globe technique, shader GlobePS
/*$(ShaderResources)*/

#include "PBR.hlsl"

struct PSInput // AKA VSOutputGlobe
{
	float4 position          : SV_POSITION;
	float2 uv                : TEXCOORD0;
	float3 worldPos          : TEXCOORD1;
	float3 normal            : TEXCOORD2;
	float4 positionThisFrame : TEXCOORD3;
	float4 positionLastFrame : TEXCOORD4;
};

struct PSOutput
{
	float4 color    : SV_Target0;
	float2 velocity : SV_Target1;
};

PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;

	uint2 dims = /*$(Variable:RenderSize)*/;

	float3 albedo = /*$(Image:lroc_color_poles_2k.png:RGBA8_Unorm:float4:true)*/.SampleLevel(LinearWrap, input.uv, 0).rgb;

	const float metallic = /*$(Variable:metallic)*/;
	const float roughness = /*$(Variable:roughness)*/;
	const float3 lightDir = normalize(/*$(Variable:lightDir)*/);
	const float3 lightColor = /*$(Variable:lightColor)*/;
	const float lightIntensity = /*$(Variable:lightIntensity)*/;
	const float3 lightAmbient = /*$(Variable:lightAmbient)*/;

	// Get reverse view direction (from world to camera)
	float3 V = normalize(/*$(Variable:CameraPos)*/ - input.worldPos);

	// calculate lighting
	float3 light;
	if (/*$(Variable:LightingMode)*/ == LightingModes::PBR)
		light = DirectionalLight(input.worldPos, input.normal, V, lightDir, lightColor * lightIntensity, albedo, metallic, roughness, c_F0);
	else
		light = albedo * clamp(dot(input.normal, lightDir), 0.0f, 1.0f);
	ret.color = float4(light + albedo * lightAmbient, 1.0f);

	// calculate velocity
	float2 newLastPositionPx = input.positionLastFrame.xy / input.positionLastFrame.w - /*$(Variable:CameraJitterLastFrame)*/;
	float2 newThisPositionPx = input.positionThisFrame.xy / input.positionThisFrame.w - /*$(Variable:CameraJitter)*/;
	ret.velocity = (newThisPositionPx - newLastPositionPx) * float2(1.0f, -1.0f);

	return ret;
}
