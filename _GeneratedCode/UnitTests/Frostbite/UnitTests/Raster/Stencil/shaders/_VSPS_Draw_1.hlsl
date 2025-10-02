//VS Source: StencilDraw1VS.hlsl
//PS Source: StencilDraw1PS.hlsl

#if FB_VS// Stencil technique, shader Draw1VS


struct Struct__Draw1VSCB
{
    float4x4 ViewProjMtx;
};

cbuffer cb0 : register(b0)
{
	Struct__Draw1VSCB _Draw1VSCB;
};


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

#endif // FB_VS

#if FB_PS// Stencil technique, shader Draw1PS




struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float2 uv         : TEXCOORD1;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput Draw1PS(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(input.normal * 0.5f + 0.5f, 1.0f);
	return ret;
}

#endif // FB_PS