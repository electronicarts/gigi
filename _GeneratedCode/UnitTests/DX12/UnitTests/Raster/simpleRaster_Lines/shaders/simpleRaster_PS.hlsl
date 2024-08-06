// SimpleRaster technique, shader PixelShader



#line 2


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
