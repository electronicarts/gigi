// Unnamed technique, shader VS
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
	float2 uv         : TEXCOORD0;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float2 uv         : TEXCOORD1;
};

VSOutput vsmain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), /*$(Variable:ViewProjMtx)*/);
	ret.normal = input.normal;
	ret.uv = input.uv;
	return ret;
}
