// Unnamed technique, shader ClearBuffersCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;
    LinearZAndNormal[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    PrevLinearZAndNormal[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    FilteredPastFbo[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    PrevReprojFbo_0[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    PrevReprojFbo_1[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    PrevReprojFbo_2[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);    
    CurReprojFbo_0[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    CurReprojFbo_1[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    CurReprojFbo_2[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    PingPongFbo_0[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    PingPongFbo_1[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    Final_Fbo[px] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}
