// Unnamed technique, shader ClearBufferCS
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

    TheBuffer[DTid.x] = 0;
}

/*
Shader Resources:
	Buffer TheBuffer (as UAV)
*/
