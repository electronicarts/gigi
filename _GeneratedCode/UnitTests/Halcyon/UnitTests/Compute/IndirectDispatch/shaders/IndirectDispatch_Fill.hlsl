

struct Struct__Fill_Indirect_Dispatch_Count_0CB
{
    uint3 Dispatch_Count_1;
    float _padding0;
    uint3 Dispatch_Count_2;
    float _padding1;
};

RWStructuredBuffer<uint4> IndirectDispatchCount : register(u0);
ConstantBuffer<Struct__Fill_Indirect_Dispatch_Count_0CB> _Fill_Indirect_Dispatch_Count_0CB : register(b0);


[numthreads(1, 1, 1)]
void FillIndirectDispatchCount(uint3 DTid : SV_DispatchThreadID)
{
    IndirectDispatchCount[0].x = 5;
    IndirectDispatchCount[0].y = 4;
    IndirectDispatchCount[0].z = 3;
    IndirectDispatchCount[0].w = 2;

    IndirectDispatchCount[1].x = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1.x;
    IndirectDispatchCount[1].y = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1.y;
    IndirectDispatchCount[1].z = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1.z;
    IndirectDispatchCount[1].w = 42;

    IndirectDispatchCount[2].x = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2.x;
    IndirectDispatchCount[2].y = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2.y;
    IndirectDispatchCount[2].z = _Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2.z;
    IndirectDispatchCount[2].w = 435;
}