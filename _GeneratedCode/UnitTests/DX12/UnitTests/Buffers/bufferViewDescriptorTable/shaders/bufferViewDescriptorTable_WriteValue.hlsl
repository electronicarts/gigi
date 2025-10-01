// BufferViewDescriptorTable technique, shader WriteValue


RWStructuredBuffer<float> TheBuffer : register(u0);

#line 2


[numthreads(64, 1, 1)]
#line 4
void main(uint3 DTid : SV_DispatchThreadID)
{
	TheBuffer[DTid.x] = WRITE_VALUE;
}

/*
Shader Resources:
	Buffer TheBuffer (as UAV)
*/
