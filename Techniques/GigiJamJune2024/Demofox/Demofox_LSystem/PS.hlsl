// Unnamed technique, shader PS
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float4 color      : TEXCOORD0;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput main(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = input.color;
	return ret;
}
