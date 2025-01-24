// Unnamed technique, shader PS
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(0.0f, 0.4f, 0.0f, 1.0f);
	return ret;
}
