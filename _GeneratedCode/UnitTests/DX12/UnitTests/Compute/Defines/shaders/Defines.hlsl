// Unnamed technique, shader WriteOutputCS


RWTexture2D<float4> Output : register(u0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
    Output[DTid.xy] = float4(RED, GREEN, 1.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
