// Write a solid color wherever the depth buffer is not the default value/*$(ShaderResources)*/
/*$(_compute:Main)*/(uint3 DTid : SV_DispatchThreadID)
{
    if(Depth.Load(int3(DTid.xy, 0)).x == /*$(Variable:depthClearValue)*/)
        Output[DTid.xy] = 0;
    else
        Output[DTid.xy] = 255;
}
