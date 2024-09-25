// Unnamed technique, shader simpleAssertYES_cs
/*$(ShaderResources)*/

/*$(_compute:main_cs)*/(uint3 DTid : SV_DispatchThreadID)
{
    /*$(Assert:1>4)*/
    uint dim, stride;
    stub_uav.GetDimensions(dim, stride);
}
