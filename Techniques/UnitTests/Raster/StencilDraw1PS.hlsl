// Stencil technique, shader Draw1PS
/*$(ShaderResources)*/

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

PSOutput Draw1PS(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(input.normal * 0.5f + 0.5f, 1.0f);
	return ret;
}
