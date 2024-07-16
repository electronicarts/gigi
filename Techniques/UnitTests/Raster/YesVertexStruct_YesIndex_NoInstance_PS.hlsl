/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
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
		case ViewMode::UV: ret.colorTarget = float4(input.uv, 0.0f, 1.0f); break;
		case ViewMode::Solid: ret.colorTarget = float4(0.0f, 1.0f, 0.0f, 1.0f); break;
	}

	return ret;
}
