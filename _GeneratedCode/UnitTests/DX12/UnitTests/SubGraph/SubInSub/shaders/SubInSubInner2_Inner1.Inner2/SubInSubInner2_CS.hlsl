// SubInSubInner2 technique, shader Inner2CS


struct Struct__Inner1_Inner2_Inner2CS_0CB
{
    float4 Inner1_Inner1Mult;
};

Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);
ConstantBuffer<Struct__Inner1_Inner2_Inner2CS_0CB> _Inner1_Inner2_Inner2CS_0CB : register(b0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	Output[px].rgba = Input[px].gbra * _Inner1_Inner2_Inner2CS_0CB.Inner1_Inner1Mult;
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
