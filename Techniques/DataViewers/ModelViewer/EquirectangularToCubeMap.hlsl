// Unnamed technique, shader EquirectangularToCubeMap
/*$(ShaderResources)*/

#include "CubeMap.hlsli"

float3 SkyboxColor(float2 uv)
{
    return Input.SampleLevel(LinearWrapSampler, uv, 0).rgb;
    //return float3(0.5f, 0.5f, 0.5f);
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint3 outputDims;
    Output.GetDimensions(outputDims.x, outputDims.y, outputDims.z);

    float2 uv = (float2(DTid.xy) + float2(0.5f, 0.5f)) / float2(outputDims.xy);
    uv = uv * 2.0f - 1.0f;
    uv.y = -uv.y;

    float3 dir = CubeUVFaceToDir(uv, DTid.z);

    float3 color = SkyboxColor(SampleEquirectangularMap(dir));

    color = min(color, float3(/*$(Variable:MaxIBLValue)*/, /*$(Variable:MaxIBLValue)*/, /*$(Variable:MaxIBLValue)*/));

    Output[uint3(DTid.xyz)] = float4(color, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
