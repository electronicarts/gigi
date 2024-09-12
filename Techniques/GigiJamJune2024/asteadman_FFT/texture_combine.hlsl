// 2D FFT With Texture technique, shader RealTextureCombiner
/*$(ShaderResources)*/

/*$(_compute:csMain)*/(uint3 DTid : SV_DispatchThreadID)
{
    const uint2 px = DTid.xy;
    const uint inOffset = px.x + px.y * /*$(Variable:N)*/;

    const float r = RedGreen[inOffset].Re;
    const float g = RedGreen[inOffset].Im;
    const float b = Blue[inOffset].Re;

    const uint2 opx = DTid.yx;
    OutTexture[opx] = float4(r,g,b,1.0);
}

/*
Shader Resources:
    Buffer RedGreen (as SRV)
    Buffer Blue (as SRV)
    Texture OutTexture (as UAV)
*/
