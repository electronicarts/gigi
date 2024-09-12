// Demofox_ChaosGame_3D technique, shader CSCombine
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float firstEye = dot(Output[px].rgb, float3(0.3f, 0.59f, 0.11f));
	float secondEye = dot(SecondEye[px].rgb, float3(0.3f, 0.59f, 0.11f));
	Output[px] = float4(firstEye, secondEye, secondEye, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
	Texture SecondEye (as SRV)
*/
