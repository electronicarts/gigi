// Blurs the color written by outline_expand.hlsl/*$(ShaderResources)*/
/*$(_compute:Main)*/(uint3 DTid : SV_DispatchThreadID)
{
    int radius = /*$(Variable:blurRadius)*/;
    float output = 0.0f;
    float weight = 0.0f;
    for (int iy = -radius; iy <= radius; ++iy)
    {
        for (int ix = -radius; ix <= radius; ++ix)
        {
            output += Input.Load(int3(int2(DTid.xy) + int2(ix,iy), 0)).r;
            weight += 1.0f;
        }
    }

    output /= weight;

    Output[DTid.xy] = float4(output, 0.0f, 0.0f, 0.0f);
}
