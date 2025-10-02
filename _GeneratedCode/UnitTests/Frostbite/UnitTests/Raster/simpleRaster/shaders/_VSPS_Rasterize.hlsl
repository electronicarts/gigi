//VS Source: simpleRaster_VS.hlsl
//PS Source: simpleRaster_PS.hlsl

#if FB_VS// SimpleRaster technique, shader VertexShader


struct Struct__VertexShaderCB
{
    float4x4 ViewProjMtx;
};

cbuffer cb0 : register(b0)
{
	Struct__VertexShaderCB _VertexShaderCB;
};


struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), _VertexShaderCB.ViewProjMtx);
	ret.normal = input.normal;
	return ret;
}

#endif // FB_VS

#if FB_PS// SimpleRaster technique, shader PixelShader




struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput PSMain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(input.normal * 0.5f + 0.5f, 1.0f);
	return ret;
}

#endif // FB_PS