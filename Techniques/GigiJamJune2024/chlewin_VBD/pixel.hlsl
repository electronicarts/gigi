/*$(ShaderResources)*/

#include "simpleRenderCommon.hlsl"

PSOutput psMain(VSOutput input)
{
	PSOutput ret = (PSOutput)0;

	float3 meshColor = float3(1,1,1);

	float3 lightDir = -normalize(/*$(Variable:lightDir)*/);
	float3 normal = -normalize(input.normal);

	float nDotL = saturate(dot(lightDir, normal));

	ret.color = float4(nDotL * /*$(Variable:lightColor)*/ * meshColor, 1);

	float skyVisibility = saturate(dot(normal, float3(0, 1, 0)));
	float groundVisibility = saturate(dot(normal, float3(0, -1, 0)));

	ret.color.xyz += lerp(0.5*/*$(Variable:equatorColor)*/, /*$(Variable:skyColor)*/, skyVisibility)*meshColor;
	ret.color.xyz += lerp(0.5*/*$(Variable:equatorColor)*/, /*$(Variable:groundColor)*/, groundVisibility)*meshColor;

	return ret;
}

