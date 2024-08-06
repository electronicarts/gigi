// Stencil technique, shader Draw2VS



#line 2


struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
};

// Full Screen Triangle
VSOutput Draw2VS(uint id : SV_VertexID)
{
	VSOutput ret = (VSOutput)0;
	float2 uv = float2((id << 1) & 2, id & 2);
	ret.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return ret;
}
