// SimpleRaster technique, shader PixelShader
/*$(ShaderResources)*/
#include "MeshAmplificationLines_Shared.hlsli"

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput PSMain(ProcessedVertex input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(input.color, 1.0f);
	return ret;
}
