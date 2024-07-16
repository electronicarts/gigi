// Unnamed technique, shader VS
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
};

VSOutput vsmain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), /*$(Variable:ViewProjMtx)*/);
	return ret;
}
