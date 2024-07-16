

struct ViewMode
{
    static const int Normal = 0;
    static const int UV = 1;
    static const int Solid = 2;
};

struct Struct__VertexShaderCB
{
    float4x4 ViewProjMtx;
};

ConstantBuffer<Struct__VertexShaderCB> _VertexShaderCB : register(b0);


struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	uint instanceId : SV_InstanceID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float2 uv         : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	float3 offset = float3((float(input.instanceId)-2.0f) * 2.0f, 0.0f, 0.0f);
	ret.position = mul(float4(input.position + offset, 1.0f), _VertexShaderCB.ViewProjMtx);

	ret.normal = input.normal;
	ret.uv = input.uv;

	return ret;
}
