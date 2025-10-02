



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
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return ret;
}
