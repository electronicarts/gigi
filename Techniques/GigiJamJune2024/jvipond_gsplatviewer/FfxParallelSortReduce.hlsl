// Unnamed technique, shader FfxParallelSortReduce
/*$(ShaderResources)*/

uint FfxNumKeys() { return /*$(Variable:_SplatCount)*/; }
int FfxNumBlocksPerThreadGroup() { return /*$(Variable:numBlocksPerThreadGroup)*/; }
uint FfxNumThreadGroups() { return /*$(Variable:numThreadGroups)*/; }
uint FfxNumThreadGroupsWithAdditionalBlocks() { return /*$(Variable:numThreadGroupsWithAdditionalBlocks)*/; }
uint FfxNumReduceThreadgroupPerBin() { return /*$(Variable:numReduceThreadgroupPerBin)*/; }
uint FfxNumScanValues() { return /*$(Variable:numScanValues)*/; }
uint FfxShiftBit() { return /*$(Variable:shift)*/; }

#define REDUCE_TABLE
#define SUM_TABLE
#include "GpuSortFidelityFx.hlsli"

/*$(_compute:FfxParallelSortReduce)*/(uint LocalID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    ffxParallelSortReduceCount(LocalID, GroupID);
}

/*
Shader Resources:
	Buffer rw_sum_table (as UAV)
	Buffer rw_reduce_table (as UAV)
*/
