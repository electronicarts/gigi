// SimpleRaster technique, shader VertexShader


struct Struct__QuadVSCB
{
    float4x4 ViewProjMtx;
};

ConstantBuffer<Struct__QuadVSCB> _QuadVSCB : register(b0);

#line 2


struct VSInput
{
	uint vertexID: SV_VertexID;
	uint instanceId : SV_InstanceID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	uint shadingRate  : SV_ShadingRate;
};

// from d3d12.h
static const uint D3D12_SHADING_RATE_1X1 = 0;
static const uint D3D12_SHADING_RATE_1X2 = 0x1;
static const uint D3D12_SHADING_RATE_2X1 = 0x4;
static const uint D3D12_SHADING_RATE_2X2 = 0x5;
static const uint D3D12_SHADING_RATE_2X4 = 0x6;
static const uint D3D12_SHADING_RATE_4X2 = 0x9;
static const uint D3D12_SHADING_RATE_4X4 = 0xa;

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	float3 position = float3(0.0f, 0.0f, 0.0f);
	switch(input.vertexID)
	{
		// first triangle, couter clock wise
		case 0: position = float3(0.0f, 0.0f, 0.0f); ret.shadingRate = D3D12_SHADING_RATE_1X1; break;
		case 1: position = float3(1.0f, 0.0f, 0.0f); ret.shadingRate = D3D12_SHADING_RATE_1X1; break;
		case 2: position = float3(0.0f, 1.0f, 0.0f); ret.shadingRate = D3D12_SHADING_RATE_1X1; break;

		// second triangle, clock wise
		case 3: position = float3(0.0f, 1.0f, 0.0f); ret.shadingRate = D3D12_SHADING_RATE_4X4; break;
		case 4: position = float3(1.0f, 1.0f, 0.0f); ret.shadingRate = D3D12_SHADING_RATE_4X4; break;
		case 5: position = float3(1.0f, 0.0f, 0.0f); ret.shadingRate = D3D12_SHADING_RATE_4X4; break;
	}

	position += float3(-0.6f, -0.3f, 0.0f);
	position.xy *= 2.0f;

	ret.position = mul(float4(position, 1.0f), _QuadVSCB.ViewProjMtx);

	return ret;
}
