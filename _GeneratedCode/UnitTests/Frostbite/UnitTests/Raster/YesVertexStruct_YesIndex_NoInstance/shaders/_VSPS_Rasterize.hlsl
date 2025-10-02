//VS Source: YesVertexStruct_YesIndex_NoInstance_VS.hlsl
//PS Source: YesVertexStruct_YesIndex_NoInstance_PS.hlsl

#if FB_VS

struct ViewMode
{
    static const int UV = 0;
    static const int Solid = 1;
};

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
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	uint instanceId : SV_InstanceID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float2 uv         : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	float3 offset = float3((float(input.instanceId)-2.0f) * 0.3f, 0.0f, (float(input.instanceId)-2.0f) * 2.0f);
	ret.position = mul(float4(input.position + offset, 1.0f), _VertexShaderCB.ViewProjMtx);

	ret.uv = input.uv;

	return ret;
}

#endif // FB_VS

#if FB_PS

struct ViewMode
{
    static const int UV = 0;
    static const int Solid = 1;
};

struct Struct__PixelShaderCB
{
    int viewMode;
    float3 _padding0;
};

cbuffer cb0 : register(b0)
{
	Struct__PixelShaderCB _PixelShaderCB;
};


struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float2 uv         : TEXCOORD1;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput PSMain(PSInput input)
{
	PSOutput ret = (PSOutput)0;

	switch(_PixelShaderCB.viewMode)
	{
		case ViewMode::UV: ret.colorTarget = float4(input.uv, 0.0f, 1.0f); break;
		case ViewMode::Solid: ret.colorTarget = float4(0.0f, 1.0f, 0.0f, 1.0f); break;
	}

	return ret;
}

#endif // FB_PS