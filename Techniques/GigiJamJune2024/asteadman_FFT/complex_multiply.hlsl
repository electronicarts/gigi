// 2D FFT With Texture technique, shader ComplexMultiply
/*$(ShaderResources)*/

/*$(_compute:csMain)*/(uint3 DGid : SV_GroupID, uint threadId : SV_GroupIndex)
{
    int i;

    for (i = 0; i < 4; ++i)
    {
        const int srcIndex = (DGid.x * /*$(Variable:N)*/) + i * 256 + threadId;
        const Struct_Complex A = ComplexInOut[srcIndex];
        const Struct_Complex B = ComplexRef[srcIndex];

        const Struct_Complex C = { A.Re * B.Re - A.Im * B.Im, A.Im * B.Re + A.Re * B.Im };
        
        ComplexInOut[srcIndex] = C;
    }
}

/*
Shader Resources:
    Buffer ComplexInOut (as UAV)
    Buffer ComplexRef (as SRV)
*/
