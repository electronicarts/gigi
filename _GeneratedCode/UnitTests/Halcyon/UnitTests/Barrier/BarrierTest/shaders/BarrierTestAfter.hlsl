// Barrier technique, shader After


RWTexture2D<float4> Output : register(u0);


[numthreads(8, 8, 1)]
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	Output[DTid.xy] = float4(Output[DTid.xy].rgb * 2.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
