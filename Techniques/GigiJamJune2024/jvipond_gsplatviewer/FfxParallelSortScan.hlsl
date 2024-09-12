// Unnamed technique, shader FfxParallelSortScan
/*$(ShaderResources)*/

uint FfxNumKeys() { return /*$(Variable:_SplatCount)*/; }
int FfxNumBlocksPerThreadGroup() { return /*$(Variable:numBlocksPerThreadGroup)*/; }
uint FfxNumThreadGroups() { return /*$(Variable:numThreadGroups)*/; }
uint FfxNumThreadGroupsWithAdditionalBlocks() { return /*$(Variable:numThreadGroupsWithAdditionalBlocks)*/; }
uint FfxNumReduceThreadgroupPerBin() { return /*$(Variable:numReduceThreadgroupPerBin)*/; }
uint FfxNumScanValues() { return /*$(Variable:numScanValues)*/; }
uint FfxShiftBit() { return /*$(Variable:shift)*/; }

#define SCAN_SOURCE
#define SCAN_DEST
#include "GpuSortFidelityFx.hlsli"

/*$(_compute:FfxParallelSortScan)*/(uint LocalID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    uint BaseIndex = FFX_PARALLELSORT_ELEMENTS_PER_THREAD * FFX_PARALLELSORT_THREADGROUP_SIZE * GroupID;
    ffxParallelSortScanPrefix(FfxNumScanValues(), LocalID, GroupID, 0, BaseIndex, false);
}

/*
Shader Resources:
	Buffer rw_scan_source (as UAV)
	Buffer rw_scan_dest (as UAV)
*/
