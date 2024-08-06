// Unnamed technique, shader VS


RWTexture2D<float4> MipTexVS : register(u0);

#line 2


struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
};

VSOutput main(uint id : SV_VertexID)
{
	uint2 dimensions;
	MipTexVS.GetDimensions(dimensions.x, dimensions.y);
	uint2 writePos = uint2(id % dimensions.x, (id / dimensions.x) % dimensions.y);
	MipTexVS[writePos] = float4(0.0f, 0.25f, 0.5f, 0.75f);

	// Full Screen Triangle
	VSOutput ret = (VSOutput)0;
	float2 uv = float2((id << 1) & 2, id & 2);
	ret.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return ret;
}
