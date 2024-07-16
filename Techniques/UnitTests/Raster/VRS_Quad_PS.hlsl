// SimpleRaster technique, shader PixelShader
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput PSMain(PSInput input)
{
	float shade = sin(input.position.x * 200.0f) * 0.5f + 0.5f;
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(shade, 0.0f, 0.0f, 0.75f);
	return ret;
}
