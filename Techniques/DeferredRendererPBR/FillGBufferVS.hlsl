// Unnamed technique, shader Fill GBuffer VS
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
	float4 tangent    : TANGENT;
	float2 uv         : TEXCOORD0;
	int  materialID   : TEXCOORD1;
	int  shapeID      : TEXCOORD2;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float4 tangent    : TEXCOORD1;
	float2 uv         : TEXCOORD2;
	float3 viewPos    : TEXCOORD3;
	float3 worldPos   : TEXCOORD4;
	int    materialID : TEXCOORD6;
	int    shapeID    : TEXCOORD7;
};

VSOutput vsmain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), /*$(Variable:ViewProjMtx)*/);
	ret.normal = input.normal;
	ret.tangent = input.tangent;
	ret.uv = input.uv;
	ret.materialID = input.materialID;
	ret.shapeID = input.shapeID;

	float4 viewPos = mul(float4(input.position, 1.0f), /*$(Variable:ViewMtx)*/);
	ret.viewPos = viewPos.xyz / viewPos.w;
	ret.worldPos = input.position;

	return ret;
}
