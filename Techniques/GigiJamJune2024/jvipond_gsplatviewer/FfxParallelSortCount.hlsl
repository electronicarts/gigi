// Unnamed technique, shader FfxParallelSortCount
/*$(ShaderResources)*/

uint FfxNumKeys() { return /*$(Variable:_SplatCount)*/; }
int FfxNumBlocksPerThreadGroup() { return /*$(Variable:numBlocksPerThreadGroup)*/; }
uint FfxNumThreadGroups() { return /*$(Variable:numThreadGroups)*/; }
uint FfxNumThreadGroupsWithAdditionalBlocks() { return /*$(Variable:numThreadGroupsWithAdditionalBlocks)*/; }
uint FfxNumReduceThreadgroupPerBin() { return /*$(Variable:numReduceThreadgroupPerBin)*/; }
uint FfxNumScanValues() { return /*$(Variable:numScanValues)*/; }
uint FfxShiftBit() { return /*$(Variable:shift)*/; }

#define SOURCE_KEYS
#define SUM_TABLE
#include "GpuSortFidelityFx.hlsli"

/*$(_compute:FfxParallelSortCount)*/(uint LocalID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
	ffxParallelSortCountUInt(LocalID, GroupID, FfxShiftBit());
}

/*
Shader Resources:
	Buffer rw_source_keys (as UAV)
	Buffer rw_sum_table (as UAV)
*/
