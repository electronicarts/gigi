/*$(ShaderResources)*/

/*$(_compute:DoIndirectDispatch)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;
    float3 color = /*$(RWTextureR:Render_Target)*/[px].rgb;
    color += float3(0.8f, 0.5f, 0.2f);
    /*$(RWTextureW:Render_Target)*/[px] = float4(color, 1.0f);
}