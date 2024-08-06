// SubGraphTest technique, shader Swap Colors


RWTexture2D<float4> Output : register(u0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	Output[DTid.xy].rgba = Output[DTid.xy].gbra;
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
