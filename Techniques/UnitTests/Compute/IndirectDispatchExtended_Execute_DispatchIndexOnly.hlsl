/*$(ShaderResources)*/

/*$(_compute:DoIndirectDispatch)*/(uint3 DTid : SV_DispatchThreadID)
{
    //DispatchIndex - This is replaced with the index of the dispatch for each dispatch argument in the dispatch buffer. Otherwise, the value is 0. 
    uint dispatchIndex = /*$(DispatchIndex)*/;
    uint2 offset = uint2(0, 3);
    float3 color = float3(0.8f, 0.5f, 0.2f);
    if(dispatchIndex == 0)
    {
        offset = uint2(3, 0);
        color = float3(0.3f, 0.7f, 0.8f);
    }
    uint2 px = DTid.xy + offset;
    /*$(RWTextureW:Render_Target)*/[px] = float4(color, 1.0f);
}