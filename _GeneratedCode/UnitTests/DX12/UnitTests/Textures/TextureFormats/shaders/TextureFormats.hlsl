// Unnamed technique, shader csmain


RWTexture2D<float4> tex : register(u0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	tex[DTid.xy] = float4(0.25f, 0.75f, 1.0f, 4.0f);
}
