

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

ConstantBuffer<Struct__PixelShaderCB> _PixelShaderCB : register(b0);


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
