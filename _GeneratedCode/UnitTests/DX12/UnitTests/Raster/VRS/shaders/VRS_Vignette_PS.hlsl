// VRS technique, shader VignettePS


Texture2D<float> Depth : register(t0);

#line 2


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
	float alpha = 1.0f;
	if (Depth[uint2(input.position.xy)] != 0.0f)
	{
		alpha = 0.0f;
	}

	float2 uv = float2(input.position.xy) / 512.0f;

	float r = sin(uv.x*50.0f)* 0.5f + 0.5f;
	float g = sin(uv.y*50.0f)* 0.5f + 0.5f;

	PSOutput ret = (PSOutput)0;
	//ret.colorTarget = float4(r, g, 0.0f, alpha);
	ret.colorTarget = float4(0.0f, 0.0f, r, alpha);
	return ret;
}
