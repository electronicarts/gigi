// Expands the color written by outline_solid.hlsl/*$(ShaderResources)*/
/*$(_compute:Main)*/(uint3 DTid : SV_DispatchThreadID)
{
    int radius = /*$(Variable:expansionRadius)*/;
    float output = 0.0f;
    for (int iy = -radius; iy <= radius; ++iy)
        for (int ix = -radius; ix <= radius; ++ix)
            output = max(output, Input.Load(int3(int2(DTid.xy) + int2(ix,iy), 0))).r;

    Output[DTid.xy] = float4(output, 0.0f, 0.0f, 0.0f);
}
