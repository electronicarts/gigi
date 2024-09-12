// FFT1024 (2d) technique, shader ComplexTranspose
/*$(ShaderResources)*/

#define NUMTHREADS (/*$(NumThreads)*/.x * /*$(NumThreads)*/.y)
#define NUMTHREADS_Y (/*$(NumThreads)*/.y)

#define THEADBLOCKS_PER_N (/*$(Variable:N)*/ / NUMTHREADS)

// Working memory.
groupshared Struct_Complex A[32][32];
groupshared Struct_Complex B[32][32];

uint2 conflictFreeIndex(uint2 loc)
{
    return uint2(loc.x ^ loc.y, loc.y);
}

void readTileIntoA(uint2 srcTileIndex, uint2 groupThreadId)
{
    const uint tlOffset = srcTileIndex.x * 32 + srcTileIndex.y * 32 * /*$(Variable:N)*/;
    
    for (int i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        const uint2 virtPos = uint2(groupThreadId.x, groupThreadId.y + i *NUMTHREADS_Y);
        const uint2 cfi = conflictFreeIndex(virtPos);
        A[cfi.y][cfi.x] = ComplexSquareMatrix[tlOffset + (virtPos.y * /*$(Variable:N)*/) + virtPos.x];
    }
}

void writeTileFromA(uint2 dstTileIndex, uint2 groupThreadId)
{
    const uint tlOffset = dstTileIndex.x * 32 + dstTileIndex.y * 32 * /*$(Variable:N)*/;
    
    for (int i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        const uint2 virtPos = uint2(groupThreadId.x, groupThreadId.y + i *NUMTHREADS_Y);
        const uint2 cfi = conflictFreeIndex(virtPos.yx); // TRANSPOSE!
        ComplexSquareMatrix[tlOffset + (virtPos.y * /*$(Variable:N)*/) + virtPos.x] = A[cfi.y][cfi.x];
    }
}

void readTileIntoB(uint2 srcTileIndex, uint2 groupThreadId)
{
    const uint tlOffset = srcTileIndex.x * 32 + srcTileIndex.y * 32 * /*$(Variable:N)*/;
    
    for (int i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        const uint2 virtPos = uint2(groupThreadId.x, groupThreadId.y + i *NUMTHREADS_Y);
        const uint2 cfi = conflictFreeIndex(virtPos);
        B[cfi.y][cfi.x] = ComplexSquareMatrix[tlOffset + (virtPos.y * /*$(Variable:N)*/) + virtPos.x];
    }
}

void writeTileFromB(uint2 dstTileIndex, uint2 groupThreadId)
{
    const uint tlOffset = dstTileIndex.x * 32 + dstTileIndex.y * 32 * /*$(Variable:N)*/;
    
    for (int i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        const uint2 virtPos = uint2(groupThreadId.x, groupThreadId.y + i *NUMTHREADS_Y);
        const uint2 cfi = conflictFreeIndex(virtPos.yx); // TRANSPOSE!
        ComplexSquareMatrix[tlOffset + (virtPos.y * /*$(Variable:N)*/) + virtPos.x] = B[cfi.y][cfi.x];
    }
}

uint calculateLowerTriangularRow(uint tileIndex)
{
    // https://en.wikipedia.org/wiki/Triangular_number#Triangular_roots_and_tests_for_triangular_numbers
    return 1 + uint((sqrt(1 + 8.f * tileIndex) - 1) / 2);
}

uint calculateLowerTriangularRowStart(uint row)
{
    return ((row - 1) * (row)) >> 1;
}

/*$(_compute:csMain)*/(uint DGid : SV_GroupID, uint2 groupThreadId : SV_GroupThreadID)
{
    // 1024 x 1024 complex values
    // break into 32x32 tiles of 32x32
    // there is room in memory for 2 tiles
    
    // There are 32 main diagonal tiles => We have memory to do 2 at a time => 16 dispatches
    // For lower triangular there are (32*32 - 32) / 2 = 496 tiles (each with a pair in the Upper) => 496 dispatches
    // Total dispatches: 16 + 496 = 512

    if (DGid < 16)
    {
        const uint diagTile = 2*DGid;
        readTileIntoA(uint2(diagTile,diagTile), groupThreadId);
        readTileIntoB(uint2(diagTile+1,diagTile+1), groupThreadId);
        // enforce that the writes to shared memory have occured
        GroupMemoryBarrierWithGroupSync();
        writeTileFromA(uint2(diagTile,diagTile), groupThreadId); // Same destination
        writeTileFromB(uint2(diagTile+1,diagTile+1), groupThreadId); // Same destination
    }
    else
    {
        const uint tileId = (DGid - 16); //[0-495]
        const uint tileRow = calculateLowerTriangularRow(tileId);
        const uint tileCol = tileId - calculateLowerTriangularRowStart(tileRow);

        readTileIntoA(uint2(tileCol,tileRow), groupThreadId);
        readTileIntoB(uint2(tileRow,tileCol), groupThreadId); // Read Opposite
        // enforce that the writes to shared memory have occured
        GroupMemoryBarrierWithGroupSync();
        writeTileFromA(uint2(tileRow,tileCol), groupThreadId); // Swap destination
        writeTileFromB(uint2(tileCol,tileRow), groupThreadId); // Swap destination
    }
}

/*
Shader Resources:
    Buffer ComplexSquareMatrix (as UAV)
*/
