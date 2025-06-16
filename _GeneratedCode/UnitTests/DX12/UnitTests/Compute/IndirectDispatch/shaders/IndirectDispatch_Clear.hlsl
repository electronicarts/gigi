// IndirectDispatch technique, shader Clear Render Target


RWTexture2D<float4> Render_Target : register(u0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	Render_Target[DTid.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

/*
Shader Resources:
	Texture Render Target (as UAV)
*/
