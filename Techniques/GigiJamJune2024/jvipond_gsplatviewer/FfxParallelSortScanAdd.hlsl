// Unnamed technique, shader FfxParallelSortScanAdd
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
#define SCAN_SCRATCH
#include "GpuSortFidelityFx.hlsli"

/*$(_compute:FfxParallelSortScanAdd)*/(uint LocalID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    // When doing adds, we need to access data differently because reduce
    // has a more specialized access pattern to match optimized count
    // Access needs to be done similarly to reduce
    // Figure out what bin data we are reducing
    uint BinID = GroupID / FfxNumReduceThreadgroupPerBin();
    uint BinOffset = BinID * FfxNumThreadGroups();

    // Get the base index for this thread group
    uint BaseIndex = (GroupID % FfxNumReduceThreadgroupPerBin()) * FFX_PARALLELSORT_ELEMENTS_PER_THREAD * FFX_PARALLELSORT_THREADGROUP_SIZE;

    ffxParallelSortScanPrefix(FfxNumThreadGroups(), LocalID, GroupID, BinOffset, BaseIndex, true);
}

/*
Shader Resources:
	Buffer rw_scan_source (as UAV)
	Buffer rw_scan_dest (as UAV)
	Buffer rw_scan_scratch (as UAV)
*/
