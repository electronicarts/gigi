// Unnamed technique, shader Dummy
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    float3 doesnt_have_to_make_sense_innit = float3(/*$(Variable:a_big_float3)*/);
    doesnt_have_to_make_sense_innit += float3(/*$(Variable:something_big_too)*/.xy, /*$(Variable:a_tiny_uint16)*/);
    Test[0] = doesnt_have_to_make_sense_innit + float(/*$(Variable:z_another_uint16)*/);
}
