// Barrier technique, shader Right


RWTexture2D<float4> Output : register(u0);


[numthreads(8, 8, 1)]
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	Output.GetDimensions(dims.x, dims.y);
	Output[DTid.xy + uint2(dims.x / 2, 0)] = float4(0.0f, 0.5f, 0.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
