// Unnamed technique, shader WriteColorCS


RWTexture2D<float4> Color : register(u0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= (150) && DTid.x <= (200))
        Color[DTid.xy] = float4(0.2f, 0.8f, 0.2f, 1.0f);
}

/*
Shader Resources:
	Texture Color (as UAV)
*/
