/*$(ShaderResources)*/

/*$(_compute:DoIndirectDispatch)*/(uint3 DTid : SV_DispatchThreadID)
{
    //DispatchIndex - This is replaced with the index of the dispatch for each dispatch argument in the dispatch buffer. Otherwise, the value is 0. 
    uint dispatchIndex = /*$(DispatchIndex)*/;
    uint offset = 0;
    float3 color = float3(0.8f, 0.5f, 0.2f);
    if(dispatchIndex == 1)
    {
        offset += 3;
        color = float3(0.5f, 0.8f, 0.2f);
    }
    else if(dispatchIndex == 2)
    {
        offset += 1;
        color = float3(0.5f, 0.2f, 0.8f);
    }
    uint2 px = DTid.xy + uint2(offset, offset);
    /*$(RWTextureW:Render_Target)*/[px] = float4(color, 1.0f);
}