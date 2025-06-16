// Unnamed technique, shader BufferAtomicsCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
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

    for (uint i = 0; i < count; ++i)
    {
        uint oldValue = 0;
        InterlockedAdd(TheBuffer[i], i + 1, oldValue);
    }
}

/*
Shader Resources:
	Buffer TheBuffer (as UAV)
*/
