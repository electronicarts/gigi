// RTAO technique, shader GBufferPS
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
	float4 tangent    : TEXCOORD2;
	float3 albedo     : TEXCOORD3;
	float2 uv         : TEXCOORD4;
	uint   instanceId : TEXCOORD5;
};

struct PSOutput
{
    uint4  gbuffer0    : SV_Target0;  // RGB = normal, A = instanceId
    uint4  gbuffer1    : SV_Target1;  // RGB = albedo, A = 1 (is A used to know where empty sky is?)
    uint4  gbuffer2    : SV_Target2;  // RGB = tangent, A = bitangent direction
    uint4  gbuffer3    : SV_Target3;  // RG = uv. BA = unused.	
};

PSOutput psmain(PSInput input, bool isFrontFacing : SV_IsFrontFace)
{
	PSOutput ret = (PSOutput)0;

    float3 normal = normalize(input.normal) * (isFrontFacing ? 1.0f : -1.0f);
    float3 tangent = normalize(input.tangent.xyz);

    ret.gbuffer0 = uint4(255.0f * (normal * 0.5f + 0.5f), input.instanceId);
    ret.gbuffer1 = uint4(255.0f * input.albedo, 1);
    ret.gbuffer2 = uint4(255.0f * (tangent * 0.5f + 0.5f), input.tangent.w == 0.0f ? 0 : 1);
    ret.gbuffer3 = uint4(255.0f * input.uv, 0, 0);

	return ret;
}
