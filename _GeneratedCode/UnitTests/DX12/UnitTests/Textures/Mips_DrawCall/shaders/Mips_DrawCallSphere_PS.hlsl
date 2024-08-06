// SimpleRaster technique, shader PixelShader


struct Struct__PixelShaderSphereCB
{
    uint UseMips;
    float3 _padding0;
};

SamplerState LinearWrap : register(s0, space1);
SamplerState PointWrap : register(s1, space1);
Texture2D<float4> MipTex : register(t0);
ConstantBuffer<Struct__PixelShaderSphereCB> _PixelShaderSphereCB : register(b0);

#line 2


struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float2 UV         : TEXCOORD0;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput PSMain(PSInput input)
{
	PSOutput ret = (PSOutput)0;

	float4 texLinear = MipTex.Sample(LinearWrap, input.UV * 2.0f);
	float4 texPoint = MipTex.Sample(PointWrap, input.UV * 2.0f);

	ret.colorTarget = _PixelShaderSphereCB.UseMips
		? float4(texLinear.rgb, 1.0f)
		: float4(texPoint.rgb, 1.0f)
	;

	//ret.colorTarget = float4(input.UV, 0.0f, 1.0f);

	return ret;
}
