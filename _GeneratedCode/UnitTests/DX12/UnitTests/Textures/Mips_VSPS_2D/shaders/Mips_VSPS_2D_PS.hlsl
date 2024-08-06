// Unnamed technique, shader PS


RWTexture2D<float4> MipTexPS : register(u0);

#line 2


struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput main(PSInput input)
{
	uint2 dimensions;
	MipTexPS.GetDimensions(dimensions.x, dimensions.y);
	MipTexPS[uint2(input.position.xy) % dimensions] = float4(0.8f, 0.6f, 0.4f, 0.2f);

	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(sin(input.position.x / 10.0f)*0.5f + 0.5f, 0.2f, 0.2f, 1.0f);
	return ret;
}
