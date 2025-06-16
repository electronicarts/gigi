// Unnamed technique, shader ClearBufferCS


RWStructuredBuffer<uint> TheBuffer : register(u0);

#line 2


[numthreads(64, 1, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
    // This crashes in slang right now. (2025.7.1)
    // https://github.com/shader-slang/slang/issues/6842
    /*
    uint count = 0;
    uint stride = 0;
    TheBuffer.GetDimensions(count, stride);
    */
    uint count = 4;
    if (DTid.x >= count)
        return;

    TheBuffer[DTid.x] = 0;
}

/*
Shader Resources:
	Buffer TheBuffer (as UAV)
*/
