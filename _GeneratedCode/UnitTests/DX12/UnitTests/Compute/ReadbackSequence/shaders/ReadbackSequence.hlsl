// ReadbackSequence technique, shader ReadbackSequenceCS


struct Struct__ReadbackSequenceCSCB
{
    int frameIndex;
    float3 _padding0;
};

RWTexture2D<float4> output : register(u0);
ConstantBuffer<Struct__ReadbackSequenceCSCB> _ReadbackSequenceCSCB : register(b0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	float3 c = float3(0.0f, 0.0f, 0.0f);

	if (_ReadbackSequenceCSCB.frameIndex & 1)
		c.b = 1.0f;

	if (_ReadbackSequenceCSCB.frameIndex & 2)
		c.g = 1.0f;

	if (_ReadbackSequenceCSCB.frameIndex & 4)
		c.r = 1.0f;

	//float frameIndex = float(_ReadbackSequenceCSCB.frameIndex % 256) / 255.0f;

	output[DTid.xy] = float4(c, 1.0f);
}
