// Unnamed technique, shader ModelViewerPS
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 Position   : SV_POSITION;
	float4 Color      : TEXCOORD0;
};

struct PSOutput
{
	float4 colorTargetF32 : SV_Target0;
	float4 colorTargetU8sRGB : SV_Target1;
};

PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTargetF32 = input.Color;
	ret.colorTargetU8sRGB = input.Color;
	return ret;
}
