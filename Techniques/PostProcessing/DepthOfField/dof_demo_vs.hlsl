// Unnamed technique, shader VS
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
	float2 UV         : TEXCOORD0;
	int    materialID : TEXCOORD1;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float2 UV         : TEXCOORD0;
	float3 normal     : TEXCOORD1;
	int    materialID : TEXCOORD2;
	float  depth      : TEXCOORD3;
};

VSOutput vsmain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), /*$(Variable:ViewProjMtx)*/);
	ret.UV = input.UV;
	ret.normal = input.normal;
	ret.materialID = input.materialID;

	float4 cameraPos = mul(float4(input.position, 1.0f), /*$(Variable:ViewMtx)*/);
	ret.depth = cameraPos.z / cameraPos.w;

	return ret;
}
