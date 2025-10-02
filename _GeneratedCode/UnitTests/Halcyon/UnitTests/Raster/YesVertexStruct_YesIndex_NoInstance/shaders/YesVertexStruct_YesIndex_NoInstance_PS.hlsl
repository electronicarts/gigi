

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

ConstantBuffer<Struct__PixelShaderCB> _PixelShaderCB : register(b0);


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
