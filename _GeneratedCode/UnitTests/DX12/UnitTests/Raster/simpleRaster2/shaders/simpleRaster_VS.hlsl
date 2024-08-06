// SimpleRaster technique, shader VertexShader


struct Struct__VertexShaderCB
{
    float4x4 ViewProjMtx;
};

ConstantBuffer<Struct__VertexShaderCB> _VertexShaderCB : register(b0);

#line 2


struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position, 1.0f), _VertexShaderCB.ViewProjMtx);
	ret.normal = input.normal;
	return ret;
}
