// Composes the expanded color from outline_expand.hlsl back into the color target to draw the outlines/*$(ShaderResources)*/
/*$(_compute:Main)*/(uint3 DTid : SV_DispatchThreadID)
{
    // if the depth is not the defualt depth, leave the output color alone
    if(Depth.Load(int3(DTid.xy, 0)).x != /*$(Variable:depthClearValue)*/)
        return;

    // set the color of the output to be the outline color, if there is outline color there.
    float outline = OutlineBuffer.Load(int3(DTid.xy, 0)).r;
    if (outline == 0.0f)
        return;

    float3 outColor = lerp(Output[DTid.xy].xyz, /*$(Variable:outlineColor)*/, outline);

    Output[DTid.xy] = float4(outColor, 1.0f);
}
