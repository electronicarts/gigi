// FFT1024 technique, shader fft1024
/*$(ShaderResources)*/

static const float PI = 3.14159265f;
#ifdef INVERSE
    static const float INV = -1.f;
    static const float OUTSCALE = (1.f / /*$(Variable:N)*/);
#else
    static const float INV = 1.f;
    static const float OUTSCALE = 1.f;
#endif 

groupshared float Re[64][64];
groupshared float Im[64][64];

uint2 transposeIndex(uint2 loc)
{
    //return loc;
    return uint2(loc.y, loc.x);
}

// offset each row by 1 column, with an extra offset for topright and bottom left tiles. Allows for conflict free 32-value reads of any given row and any given column with a stride of 1 or 2.
uint2 conflictFreeIndex(uint2 loc)
{
    const uint2 tile = uint2(loc.x >> 5, loc.y >> 5);
    const uint2 tileOffset = uint2((loc.x + loc.y) & 0x1f, loc.y&0x1f);

    return uint2((tile.x << 5) + ((tileOffset.x + (tile.x ^ tile.y)) & 0x1f), (tile.y << 5) + ((tileOffset.y + (tile.x ^ tile.y)) & 0x1f));
}

float4 fft2(float2 cmplxA, float2 cmplxB)
{
    return float4( cmplxA + cmplxB, cmplxA - cmplxB );
}

float2 fft32(float2 cmplxIn)
{
    uint li = WaveGetLaneIndex();
    float re = cmplxIn.x;
    float im = cmplxIn.y;
    
    // first RADIX-4
    {
        const float a_r = WaveReadLaneAt(re, (li / 4));
        const float a_i = WaveReadLaneAt(im, (li / 4));
        const float b_r = WaveReadLaneAt(re, (li / 4) + 8);
        const float b_i = WaveReadLaneAt(im, (li / 4) + 8);
        const float c_r = WaveReadLaneAt(re, (li / 4) + 16);
        const float c_i = WaveReadLaneAt(im, (li / 4) + 16);
        const float d_r = WaveReadLaneAt(re, (li / 4) + 24);
        const float d_i = WaveReadLaneAt(im, (li / 4) + 24);
        const float f1 = (li & 0x01) ? -1.f : 1.f;
        const float f2 = (li & 0x02) ? -1.f : 1.f;
    
        const float ac_r = a_r + f1 * c_r;
        const float ac_i = a_i + f1 * c_i;
    
        const float bd_r = (li & 0x01) ? INV * (-b_i + d_i) : (b_r + d_r);
        const float bd_i = (li & 0x01) ? INV * (b_r - d_r) : (b_i + d_i);
        
        const float theta_p = 2.f * PI * (li & 0x03) * (li / 4) / 32;
        const float wp_r = cos(theta_p);
        const float wp_i = -INV * sin(theta_p);
    
        const float y_r = ac_r + f1 * f2 * bd_r;
        const float y_i = ac_i + f1 * f2 * bd_i;
    
        re = y_r * wp_r - y_i * wp_i;
        im = y_i * wp_r + wp_i * y_r;
    }

    // second RADIX-4
    {
        const float a_r = WaveReadLaneAt(re, (li & 0x03) + 4 * (li / 16));
        const float a_i = WaveReadLaneAt(im, (li & 0x03) + 4 * (li / 16));
        const float b_r = WaveReadLaneAt(re, (li & 0x03) + 4 * (li / 16) + 8);
        const float b_i = WaveReadLaneAt(im, (li & 0x03) + 4 * (li / 16) + 8);
        const float c_r = WaveReadLaneAt(re, (li & 0x03) + 4 * (li / 16) + 16);
        const float c_i = WaveReadLaneAt(im, (li & 0x03) + 4 * (li / 16) + 16);
        const float d_r = WaveReadLaneAt(re, (li & 0x03) + 4 * (li / 16) + 24);
        const float d_i = WaveReadLaneAt(im, (li & 0x03) + 4 * (li / 16) + 24);
        const float f1 = ((li / 4) & 0x01) ? -1.f : 1.f;
        const float f2 = ((li / 4) & 0x02) ? -1.f : 1.f;
        
        const float ac_r = a_r + f1 * c_r;
        const float ac_i = a_i + f1 * c_i;
        
        const float bd_r = ((li / 4) & 0x01) ? INV * (-b_i + d_i) : (b_r + d_r);
        const float bd_i = ((li / 4) & 0x01) ? INV * (b_r - d_r) : (b_i + d_i);
        
        const float theta_p = 2.f * PI * ((li / 4) & 0x03) * (li / 16) / 8;
        const float wp_r = cos(theta_p);
        const float wp_i = -INV * sin(theta_p);
    
        const float y_r = ac_r + f1 * f2 * bd_r;
        const float y_i = ac_i + f1 * f2 * bd_i;
    
        re = y_r * wp_r - y_i * wp_i;
        im = y_i * wp_r + wp_i * y_r;
    }
    
    // RADIX-2
    {
        const float a_r = WaveReadLaneAt(re, (li & 0x0f));
        const float a_i = WaveReadLaneAt(im, (li & 0x0f));
        const float b_r = WaveReadLaneAt(re, (li & 0x0f) + 16);
        const float b_i = WaveReadLaneAt(im, (li & 0x0f) + 16);
        const float f1 = ((li / 16) & 0x01) ? -1.f : 1.f;
        
        re = a_r + f1 * b_r;
        im = a_i + f1 * b_i;
    }
    return float2(re, im);
}

// This function always twiddles
// Assumes gtid = 32 wide in x
void fft64_rows(uint rowOffset, uint2 gtid)
{
    uint li = WaveGetLaneIndex();
    
    float2 cmplxA, cmplxB;
    {
        const uint2 cfiEven = conflictFreeIndex(uint2((li << 1), rowOffset + gtid.y));
        const uint2 cfiOdd = conflictFreeIndex(uint2((li << 1) + 1, rowOffset + gtid.y));
        
        const float2 cmplxEvenCol = float2(Re[cfiEven.y][cfiEven.x], Im[cfiEven.y][cfiEven.x]);
        const float2 cmplxOddCol = float2(Re[cfiOdd.y][cfiOdd.x], Im[cfiOdd.y][cfiOdd.x]);
        
        // Individually fft-32 the two rows
        const float2 freqA = fft32(cmplxEvenCol); // first half of fft-64
        float2 freqB = fft32(cmplxOddCol); // second half of fft-64
        
        // second "row" (B) needs a twiddle
        const float2 twiddleB = float2(cos(2.f * PI * li / (64)), -INV * sin(2.f * PI * li / (64)));
        freqB = float2(freqB.x * twiddleB.x - freqB.y * twiddleB.y, freqB.y * twiddleB.x + freqB.x * twiddleB.y);
    
        // Finally Element wise fft-2 the 2 "rows"
        const float4 cmplxAB = fft2(freqA, freqB);
        cmplxA = cmplxAB.xy;
        cmplxB = cmplxAB.zw;
    }

    const uint2 virtIndexA = uint2(li, rowOffset + gtid.y);
    const uint2 virtIndexB = uint2(li+32, rowOffset + gtid.y);
    const float2 twiddleA = float2(cos(2.f * PI * virtIndexA.x * virtIndexA.y / (4096)), -INV * sin(2.f * PI * virtIndexA.x * virtIndexA.y / (4096)));
    const float2 twiddleB = float2(cos(2.f * PI * virtIndexB.x * virtIndexB.y / (4096)), -INV * sin(2.f * PI * virtIndexB.x * virtIndexB.y / (4096)));
    const uint2 cfiA = conflictFreeIndex(virtIndexA);
    const uint2 cfiB = conflictFreeIndex(virtIndexB);
    
    Re[cfiA.y][cfiA.x] = cmplxA.x * twiddleA.x - cmplxA.y * twiddleA.y;
    Im[cfiA.y][cfiA.x] = cmplxA.y * twiddleA.x + cmplxA.x * twiddleA.y;
    
    Re[cfiB.y][cfiB.x] = cmplxB.x * twiddleB.x - cmplxB.y * twiddleB.y;
    Im[cfiB.y][cfiB.x] = cmplxB.y * twiddleB.x + cmplxB.x * twiddleB.y;
}

void fft64_cols(uint colOffset, uint2 gtid)
{
    uint li = WaveGetLaneIndex();
    
    float2 cmplxA, cmplxB;
    {
        const uint2 cfiEven = conflictFreeIndex(uint2(colOffset + gtid.y, (li << 1)));
        const uint2 cfiOdd = conflictFreeIndex(uint2(colOffset + gtid.y, (li << 1) + 1));
        
        const float2 cmplxEvenCol = float2(Re[cfiEven.y][cfiEven.x], Im[cfiEven.y][cfiEven.x]);
        const float2 cmplxOddCol = float2(Re[cfiOdd.y][cfiOdd.x], Im[cfiOdd.y][cfiOdd.x]);
        
        // Individually fft-32 the two "rows"
        const float2 freqA = fft32(cmplxEvenCol); // first half of fft-64
        float2 freqB = fft32(cmplxOddCol); // second half of fft-64
        
        // second "row" (B) needs a twiddle
        const float2 twiddleB = float2(cos(2.f * PI * li / (64)), -INV * sin(2.f * PI * li / (64)));
        freqB = float2(freqB.x * twiddleB.x - freqB.y * twiddleB.y, freqB.y * twiddleB.x + freqB.x * twiddleB.y);
    
        // Finally Element wise fft-2 the 2 "rows"
        const float4 cmplxAB = fft2(freqA, freqB);
        cmplxA = cmplxAB.xy;
        cmplxB = cmplxAB.zw;
    }
    
    const uint2 cfiA = conflictFreeIndex(uint2(colOffset + gtid.y, li));
    const uint2 cfiB = conflictFreeIndex(uint2(colOffset + gtid.y, li + 32));
    
    Re[cfiA.y][cfiA.x] = cmplxA.x;
    Im[cfiA.y][cfiA.x] = cmplxA.y;
    
    Re[cfiB.y][cfiB.x] = cmplxB.x;
    Im[cfiB.y][cfiB.x] = cmplxB.y;
}

/*$(_compute:csMain)*/(uint DGid : SV_GroupID, uint threadId : SV_GroupIndex, uint2 groupThreadId : SV_GroupThreadID)
{
    int i;

    // copy to groupshared
    for (i = 0; i < 8; ++i)
    {
        const uint2 cfi = conflictFreeIndex(transposeIndex(uint2(groupThreadId.x + ((groupThreadId.y&0x01)<<5), (groupThreadId.y>>1) + (i<<3))));
        const int srcIndex = (DGid * /*$(Variable:N)*/) + i * /*$(NumThreads)*/.x * /*$(NumThreads)*/.y + threadId;

        Re[cfi.y][cfi.x] = RealInput[srcIndex];
        Im[cfi.y][cfi.x] = 0;
    }
    // enforce that the writes to shared memory have occured
    GroupMemoryBarrierWithGroupSync();

    // FFT the rows /w twiddle
    for (i = 0; i < 4; ++i)
    {
        fft64_rows(i*16, groupThreadId);
    }
    // enforce that the writes to shared memory have occured
    GroupMemoryBarrierWithGroupSync();

    // FFT the cols (no twiddle)
    for (i = 0; i < 4; ++i)
    {
        fft64_cols(i*16, groupThreadId);
    }
    // enforce that the writes to shared memory have occured
    GroupMemoryBarrierWithGroupSync();

    for (i = 0; i < 8; ++i)
    {
        const uint2 cfi = conflictFreeIndex(uint2(groupThreadId.x + ((groupThreadId.y&0x01)<<5), (groupThreadId.y>>1) + (i<<3)));
        // copy from groupshared
        const int dstIndex = (DGid * /*$(Variable:N)*/) + i * /*$(NumThreads)*/.x * /*$(NumThreads)*/.y + threadId;

        const Struct_Complex outVal = {Re[cfi.y][cfi.x] * OUTSCALE, Im[cfi.y][cfi.x] * OUTSCALE};
        ComplexOutput[dstIndex] = outVal;
    }
}

/*
Shader Resources:
    Buffer RealInput (as SRV)
    Buffer ComplexOutput (as UAV)
*/
