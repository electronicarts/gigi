// Unnamed technique, shader FfxParallelSortScatter
/*$(ShaderResources)*/

uint FfxNumKeys() { return /*$(Variable:_SplatCount)*/; }
int FfxNumBlocksPerThreadGroup() { return /*$(Variable:numBlocksPerThreadGroup)*/; }
uint FfxNumThreadGroups() { return /*$(Variable:numThreadGroups)*/; }
uint FfxNumThreadGroupsWithAdditionalBlocks() { return /*$(Variable:numThreadGroupsWithAdditionalBlocks)*/; }
uint FfxNumReduceThreadgroupPerBin() { return /*$(Variable:numReduceThreadgroupPerBin)*/; }
uint FfxNumScanValues() { return /*$(Variable:numScanValues)*/; }
uint FfxShiftBit() { return /*$(Variable:shift)*/; }

#define SOURCE_KEYS
#define DEST_KEYS
#define SUM_TABLE
#define SOURCE_PAYLOADS
#define DEST_PAYLOADS
#include "GpuSortFidelityFx.hlsli"

/*$(_compute:FfxParallelSortScatter)*/(uint LocalID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    ffxParallelSortScatterUInt(LocalID, GroupID, FfxShiftBit());
}

/*
Shader Resources:
	Buffer rw_source_keys (as UAV)
	Buffer rw_dest_keys (as UAV)
	Buffer rw_sum_table (as UAV)
	Buffer rw_source_payloads (as UAV)
	Buffer rw_dest_payloads (as UAV)
*/
