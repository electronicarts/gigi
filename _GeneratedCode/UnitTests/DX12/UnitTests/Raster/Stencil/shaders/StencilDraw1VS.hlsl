// Stencil technique, shader Draw1VS


struct Struct__Draw1VSCB
{
    float4x4 ViewProjMtx;
};

ConstantBuffer<Struct__Draw1VSCB> _Draw1VSCB : register(b0);

#line 2


struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float2 uv         : TEXCOORD1;
};

VSOutput Draw1VS(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	ret.position = mul(float4(input.position, 1.0f), _Draw1VSCB.ViewProjMtx);

	ret.normal = input.normal;
	ret.uv = input.uv;

	return ret;
}
