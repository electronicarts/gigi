// SimpleRaster technique, shader VertexShader


struct Struct__VertexShaderSphereCB
{
    float4x4 ViewProjMtx;
};

ConstantBuffer<Struct__VertexShaderSphereCB> _VertexShaderSphereCB : register(b0);

#line 2


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
	ret.position = mul(float4(input.position, 1.0f), _VertexShaderSphereCB.ViewProjMtx);
	ret.UV = input.UV;
	return ret;
}
