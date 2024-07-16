// SimpleRaster technique, shader VertexShader
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
	float2 UV         : TEXCOORD0;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float2 UV         : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), /*$(Variable:ViewProjMtx)*/);
	ret.UV = input.UV;
	return ret;
}
