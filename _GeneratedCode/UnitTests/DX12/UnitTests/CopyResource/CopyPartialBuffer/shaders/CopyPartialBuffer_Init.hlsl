// Unnamed technique, shader FillInput


RWStructuredBuffer<float> Input : register(u0);
RWStructuredBuffer<float> Output : register(u1);

#line 2


[numthreads(64, 1, 1)]
#line 4
void main(uint3 DTid : SV_DispatchThreadID)
{
	Input[DTid.x] = float(DTid.x);
	Output[DTid.x] = 0.0f;
}

/*
Shader Resources:
	Buffer Input (as UAV)
	Buffer Output (as UAV)
*/
