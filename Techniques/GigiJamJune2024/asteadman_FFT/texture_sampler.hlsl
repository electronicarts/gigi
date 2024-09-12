// Unnamed technique, shader TextureSampler
/*$(ShaderResources)*/

/*$(_compute:csMain)*/(uint3 DTid : SV_DispatchThreadID)
{
    const uint2 px = DTid.yx;
    const float3 rgb = InputTexture[px].rgb;

    const uint outOffsetTransposed = px.y + px.x * /*$(Variable:N)*/;
    Struct_Complex RG = { rgb.r, rgb.g };
    OutputComplexRG[outOffsetTransposed] = RG;
    OutputRealB[outOffsetTransposed] = rgb.b;
}

/*
Shader Resources:
    Texture InputTexture (as Count)
    Buffer OutputComplexRG (as UAV)
    Buffer OutputRealB (as UAV)
*/
