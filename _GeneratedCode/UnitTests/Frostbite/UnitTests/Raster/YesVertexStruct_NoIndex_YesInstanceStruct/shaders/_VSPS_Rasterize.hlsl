//VS Source: YesVertexStruct_NoIndex_YesInstanceStruct_VS.hlsl
//PS Source: YesVertexStruct_NoIndex_YesInstanceStruct_PS.hlsl

#if FB_VS

struct ViewMode
{
    static const int Normal = 0;
    static const int UV = 1;
    static const int Solid = 2;
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
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 offset : TEXCOORD1;
	float scale : TEXCOORD2;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float2 uv         : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	ret.position = mul(float4(input.position * input.scale + input.offset, 1.0f), _VertexShaderCB.ViewProjMtx);

	ret.normal = input.normal;
	ret.uv = input.uv;

	return ret;
}

#endif // FB_VS

#if FB_PS

struct ViewMode
{
    static const int Normal = 0;
    static const int UV = 1;
    static const int Solid = 2;
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
	float3 normal     : TEXCOORD0;
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
		case ViewMode::Normal: ret.colorTarget = float4(input.normal * 0.5f + 0.5f, 1.0f); break;
		case ViewMode::UV: ret.colorTarget = float4(input.uv, 0.0f, 1.0f); break;
		case ViewMode::Solid: ret.colorTarget = float4(0.0f, 1.0f, 0.0f, 1.0f); break;
	}

	return ret;
}

#endif // FB_PS