/*$(ShaderResources)*/

struct VSInput
{
	uint vertexID: SV_VertexID;
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
	int colorIndex = input.vertexID / 3;
	int vertIndex = input.vertexID % 3;

	if (colorIndex > 15)
	{
		ret.position = float4(-2,-2,0,0);
		ret.color = float3(0,0,0);
	}
	else
	{
		ret.color.r = colorIndex & 1;
		ret.color.g = (colorIndex >> 1) & 1;
		ret.color.b = (colorIndex >> 2) & 1;

		if (vertIndex == 1)
			position.x += 0.1f;
		else if (vertIndex == 2)
			position.y += 0.1f;
		position.x += 0.5f * ((colorIndex & 1) ? 1 : -1);
		colorIndex /= 2;
		position.y += 0.5f * ((colorIndex & 1) ? 1 : -1);
		colorIndex /=2;
		if (colorIndex > 0)
		{
			position.x += 0.25f * ((colorIndex & 1) ? 1 : -1);
			colorIndex /= 2;
			position.y += 0.25f * ((colorIndex & 1) ? 1 : -1);
			colorIndex /=2;
		}
		ret.position = float4(position,1);
	}

	return ret;
}
