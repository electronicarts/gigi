// SimpleRaster technique, shader PixelShader
/*$(ShaderResources)*/

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

	float4 texLinear = MipTex.Sample(LinearWrap, float3(input.UV * 2.0f, 0.0f));
	float4 texPoint = MipTex.Sample(PointWrap, float3(input.UV * 2.0f, 0.0f));

	ret.colorTarget = /*$(Variable:UseMips)*/
		? float4(texLinear.rgb, 1.0f)
		: float4(texPoint.rgb, 1.0f)
	;

	//ret.colorTarget = float4(input.UV, 0.0f, 1.0f);

	return ret;
}
