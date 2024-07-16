// Unnamed technique, shader csmain


RWTexture2D<float4> tex : register(u0);


[numthreads(8, 8, 1)]
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	tex[DTid.xy] = float4(0.25f, 0.75f, 1.0f, 4.0f);
}
