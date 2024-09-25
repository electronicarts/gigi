// Unnamed technique, shader simpleAssertYES_cs
/*$(ShaderResources)*/

/*$(_compute:main_cs)*/(uint3 DTid : SV_DispatchThreadID)
{
    float v0 = 1.1;
    float v1 = 1.2;
    float v2 = 1.3;
    float v3 = 1.4;
    float v4 = 1.5;
    float v5 = 1.6;
    /*$(Assert: 1>4, "v0:%.1f v1:%.1f v2:%.1f v3:%.1f v4:%.1f v5:%.1f", v0, v1, v2, v3, v4, v5 )*/
}
