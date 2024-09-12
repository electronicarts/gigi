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

groupshared float Re[32][32];
groupshared float Im[32][32];

#define NUMTHREADS (/*$(NumThreads)*/.x * /*$(NumThreads)*/.y)
#define NUMTHREADS_X (/*$(NumThreads)*/.x)
#define NUMTHREADS_Y (/*$(NumThreads)*/.y)

#define THEADBLOCKS_PER_N (/*$(Variable:N)*/ / NUMTHREADS)

uint2 transposeIndex(uint2 loc)
{
    return loc.yx;
}

// offset each row by 1 column. Allows for conflict free reads of any given row, and any given column.
// Each row will be spread accross all 32 banks. Any one column is spread accross all 32 banks.
uint2 conflictFreeIndex(uint2 loc)
{
    return uint2((loc.x ^ loc.y), loc.y);
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
void fft32_rows_with_twiddle(uint2 virtPos)
{
    const uint2 cfi = conflictFreeIndex(virtPos);
    
    float re = Re[cfi.y][cfi.x];
    float im = Im[cfi.y][cfi.x];
    
    float2 X = fft32(float2(re,im));

    const float2 twiddle = float2(cos(2.f * PI * virtPos.x * virtPos.y / (1024)), -INV * sin(2.f * PI * virtPos.x * virtPos.y / (1024)));
    
    // Write back to shared memory /w twiddle
    Re[cfi.y][cfi.x] = X.x * twiddle.x - X.y * twiddle.y;
    Im[cfi.y][cfi.x] = X.y * twiddle.x + X.x * twiddle.y;
}

void fft32_cols(uint2 virtPos)
{
    const uint2 cfi = conflictFreeIndex(virtPos.yx); // SWAP

    float re = Re[cfi.y][cfi.x];
    float im = Im[cfi.y][cfi.x];
    
    float2 X = fft32(float2(re, im));
    
    // Write Back to shared memory (same position as read)
    Re[cfi.y][cfi.x] = X.x;
    Im[cfi.y][cfi.x] = X.y;
}

/*$(_compute:csMain)*/(uint DGid : SV_GroupID, uint threadId : SV_GroupIndex, uint2 groupThreadId : SV_GroupThreadID)
{
    int i;

    // copy to groupshared
    for (i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        const uint2 cfi = conflictFreeIndex(transposeIndex(uint2(groupThreadId.x, groupThreadId.y + i * NUMTHREADS_Y)));
        const int srcIndex = (DGid * /*$(Variable:N)*/) + i * NUMTHREADS + threadId;

        Re[cfi.y][cfi.x] = ComplexInput[srcIndex].Re;
        Im[cfi.y][cfi.x] = ComplexInput[srcIndex].Im;
    }
    // enforce that the writes to shared memory have occured
    GroupMemoryBarrierWithGroupSync();

    // FFT the rows /w twiddle
    [loop]
    for (i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        fft32_rows_with_twiddle(uint2(groupThreadId.x, groupThreadId.y + i*NUMTHREADS_Y));
    }
    // enforce that the writes to shared memory have occured
    GroupMemoryBarrierWithGroupSync();

    // FFT the cols (no twiddle)
    [loop]
    for (i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        fft32_cols(uint2(groupThreadId.x, groupThreadId.y + i*NUMTHREADS_Y));
    }
    // enforce that the writes to shared memory have occured
    GroupMemoryBarrierWithGroupSync();

    for (i = 0; i < THEADBLOCKS_PER_N; ++i)
    {
        const uint2 cfi = conflictFreeIndex(uint2(groupThreadId.x, groupThreadId.y + i * NUMTHREADS_Y));
        // copy from groupshared
        const float re = Re[cfi.y][cfi.x] * OUTSCALE;
        const float im = Im[cfi.y][cfi.x] * OUTSCALE;
        const Struct_Complex cmplx = { re, im };

        const int dstIndex = (DGid * /*$(Variable:N)*/) + i * NUMTHREADS + threadId;
        ComplexOutput[dstIndex] = cmplx;
    }
}

/*
Shader Resources:
    Buffer ComplexInput (as SRV)
    Buffer ComplexOutput (as UAV)
*/
