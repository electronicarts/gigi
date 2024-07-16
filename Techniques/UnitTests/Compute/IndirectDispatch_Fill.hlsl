/*$(ShaderResources)*/

/*$(_compute:FillIndirectDispatchCount)*/(uint3 DTid : SV_DispatchThreadID)
{
    IndirectDispatchCount[0].x = 5;
    IndirectDispatchCount[0].y = 4;
    IndirectDispatchCount[0].z = 3;
    IndirectDispatchCount[0].w = 2;

    IndirectDispatchCount[1].x = /*$(Variable:Dispatch Count 1)*/.x;
    IndirectDispatchCount[1].y = /*$(Variable:Dispatch Count 1)*/.y;
    IndirectDispatchCount[1].z = /*$(Variable:Dispatch Count 1)*/.z;
    IndirectDispatchCount[1].w = 42;

    IndirectDispatchCount[2].x = /*$(Variable:Dispatch Count 2)*/.x;
    IndirectDispatchCount[2].y = /*$(Variable:Dispatch Count 2)*/.y;
    IndirectDispatchCount[2].z = /*$(Variable:Dispatch Count 2)*/.z;
    IndirectDispatchCount[2].w = 435;
}