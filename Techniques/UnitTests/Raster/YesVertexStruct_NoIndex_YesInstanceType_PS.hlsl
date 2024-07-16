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

PSOutput PSMain(PSInput input)
{
	PSOutput ret = (PSOutput)0;

	switch(/*$(Variable:viewMode)*/)
	{
		case ViewMode::Normal: ret.colorTarget = float4(input.normal * 0.5f + 0.5f, 1.0f); break;
		case ViewMode::UV: ret.colorTarget = float4(input.uv, 0.0f, 1.0f); break;
		case ViewMode::Solid: ret.colorTarget = float4(0.0f, 1.0f, 0.0f, 1.0f); break;
	}

	return ret;
}
