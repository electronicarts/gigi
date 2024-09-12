// Unnamed technique, shader TextureSampler
/*$(ShaderResources)*/

/*$(_compute:csMain)*/(uint3 DTid : SV_DispatchThreadID)
{
    const uint2 px = DTid.yx;
    const float3 rgb = InputTexture[px].rgb;

    const uint outOffsetTransposed = px.y + px.x * /*$(Variable:N)*/ * /*$(Variable:N2)*/;
    OutputRealR[outOffsetTransposed] = rgb.r;
    OutputRealG[outOffsetTransposed] = rgb.g;
    OutputRealB[outOffsetTransposed] = rgb.b;
}

/*
Shader Resources:
    Texture InputTexture (as Count)
    Buffer OutputRealR (as UAV)
    Buffer OutputRealG (as UAV)
    Buffer OutputRealB (as UAV)
*/
