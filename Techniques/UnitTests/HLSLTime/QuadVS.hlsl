/*$(ShaderResources)*/

struct VSInput
{
	uint vertexId : SV_VertexID;
};

struct VSOutput
{
	float4 csPos : SV_POSITION;
};

VSOutput mainVS(VSInput input)
{
	VSOutput output = (VSOutput)0;
	uint id = input.vertexId % 6;

	float2 uv = float2(0, 0);
	if(id == 1) uv = float2(1, 0);
	if(id == 2 || id == 3) uv = float2(1, 1);
	if(id == 4) uv = float2(0, 1);

	// x:-1 / 1, y:-1 / 1
	float2 xy = uv * 2.0f - 1.0f;

	output.csPos = float4(xy, 0.5f, 1.0f);

	return output;
}
