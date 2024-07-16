// Unnamed technique, shader PS
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

PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	//ret.colorTarget = float4(input.normal * 0.5f + 0.5f, 1.0f);
	ret.colorTarget = float4(input.uv, 0.0f, 1.0f);
	return ret;
}
