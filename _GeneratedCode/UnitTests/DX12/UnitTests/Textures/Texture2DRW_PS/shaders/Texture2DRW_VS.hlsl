// Texture2DRW_PS technique, shader RWVS



#line 2


struct VSInput
{
	uint vertexId  : SV_VertexID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
};

VSOutput vsmain(VSInput input)
{
	VSOutput ret = (VSOutput)0;
	switch(input.vertexId)
	{
		case 0: ret.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); break;
		case 1: ret.position = float4( 3.0f, -1.0f, 0.0f, 1.0f); break;
		case 2: ret.position = float4(-1.0f,  3.0f, 0.0f, 1.0f); break;
	}
	return ret;
}
