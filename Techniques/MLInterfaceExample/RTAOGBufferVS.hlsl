// RTAO technique, shader GBufferVS
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
	float4 tangent    : TANGENT;
	float3 albedo     : COLOR;
	float2 uv         : TEXCOORD;
	uint instanceId   : SV_InstanceID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
	float4 tangent    : TEXCOORD2;
	float3 albedo     : TEXCOORD3;
	float2 uv         : TEXCOORD4;
	uint   instanceId : TEXCOORD5;
};

VSOutput vsmain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), /*$(Variable:ViewProjMtx)*/);
	ret.normal = input.normal;
	ret.tangent = input.tangent;
	ret.albedo = input.albedo;
	ret.uv = input.uv;
	ret.instanceId = input.instanceId;
	return ret;
}
