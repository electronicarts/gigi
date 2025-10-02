

struct Struct__VertexShaderCB
{
    float4x4 ViewProjMtx;
};

ConstantBuffer<Struct__VertexShaderCB> _VertexShaderCB : register(b0);


struct VSInput
{
	uint vertexID: SV_VertexID;
	uint instanceId : SV_InstanceID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 color      : TEXCOORD;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	float3 position = float3(0.0f, 0.0f, 0.0f);
	switch(input.vertexID)
	{
		// first triangle, couter clock wise
		case 0: position = float3(0.0f, 0.0f, 0.0f); break;
		case 1: position = float3(1.0f, 0.0f, 0.0f); break;
		case 2: position = float3(0.0f, 1.0f, 0.0f); break;

		// second triangle, clock wise
		case 3: position = float3(0.0f, 1.0f, 0.0f); break;
		case 4: position = float3(1.0f, 1.0f, 0.0f); break;
		case 5: position = float3(1.0f, 0.0f, 0.0f); break;
	}

	float3 offset = float3(0.0f, 0.0f, float(input.instanceId));

	ret.position = mul(float4(position + offset, 1.0f), _VertexShaderCB.ViewProjMtx);

	ret.color = float3(1.0f, float(input.instanceId) / 4.0f, 0.0f);

	return ret;
}
