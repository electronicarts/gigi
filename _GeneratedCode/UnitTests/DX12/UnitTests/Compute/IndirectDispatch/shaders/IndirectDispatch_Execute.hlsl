

RWTexture2D<float4> Render_Target : register(u0);

#line 1


[numthreads(1, 1, 1)]
#line 3
void DoIndirectDispatch(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;
    float3 color = Render_Target[px].rgb;
    color += float3(0.8f, 0.5f, 0.2f);
    Render_Target[px] = float4(color, 1.0f);
}