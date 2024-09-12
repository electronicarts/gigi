// 2D FFT With Texture technique, shader RealTextureCombiner
/*$(ShaderResources)*/

/*$(_compute:csMain)*/(uint3 DTid : SV_DispatchThreadID)
{
    const uint2 px = DTid.xy;
    const uint inOffset = px.x + px.y * /*$(Variable:N)*/ * /*$(Variable:N2)*/;

    const float r = Red[inOffset].Re;
    const float g = Green[inOffset].Re;
    const float b = Blue[inOffset].Re;

    const uint2 opx = DTid.yx;
    OutTexture[opx] = float4(r,g,b,1.0);
}

/*
Shader Resources:
    Buffer Red (as SRV)
    Buffer Green (as SRV)
    Buffer Blue (as SRV)
    Texture OutTexture (as UAV)
*/
