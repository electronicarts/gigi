/*$(ShaderResources)*/

/*$(_compute:FillIndirectDispatch)*/(uint3 DTid : SV_DispatchThreadID)
{
    IndirectDispatchCount[0].x = 1;
    IndirectDispatchCount[0].y = 1;
    IndirectDispatchCount[0].z = 1;
    IndirectDispatchCount[0].w = 2;

    IndirectDispatchCount[1].x = 5;
    IndirectDispatchCount[1].y = 5;
    IndirectDispatchCount[1].z = 1;
    IndirectDispatchCount[1].w = 42;

    IndirectDispatchCount[2].x = 2;
    IndirectDispatchCount[2].y = 2;
    IndirectDispatchCount[2].z = 1;
    IndirectDispatchCount[2].w = 435;

    IndirectDispatchCountBuffer[0] = 2;
}