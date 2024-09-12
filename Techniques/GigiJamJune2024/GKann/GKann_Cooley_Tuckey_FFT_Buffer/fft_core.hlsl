
#include "fft_consts.hlsl"

groupshared float2 X[N];

float2 cmul(float2 x, float2 y) {
    return float2(x.x * y.x - x.y * y.y, x.x * y.y + x.y * y.x);
}

float2 cis(float x) {
    return float2(cos(x), sin(x));
}

uint2 set_component(uint2 v, uint i, uint x) {
    uint2 u = v;
    u[i] = x;
    return u;
}

uint reverse_digits_base_2(uint x, uint n) {
    return reversebits(x) >> (32 - n);
}

uint reverse_digits_base_4(uint x, uint n) {
    uint v = x;
    uint y = 0;
    
    for (uint i = 0; i < n; i++) {
        y = (y << 2) | (v & 3);
        v >>= 2;
    }
    
    return y;
}

// Perform an FFT over one axis of the image
void fft(uint local_index, uint workgroup_index, bool inverse)
{
    // Copy a row/column of values from the storage buffer into workgroup
    // memory, doing a bit-reversal permutation in the process
    for (uint i = 0; i < N / WORKGROUP_SIZE; i++) {
        uint j = local_index + i * WORKGROUP_SIZE;
        uint2 p = uint2(j, workgroup_index);
        uint k;

        if (RADIX == 2) {
            k = reverse_digits_base_2(j, LOG2_N);
        } else {
            k = reverse_digits_base_4(j >> (LOG2_N & 1), LOG4_N);
            k |= (j & (LOG2_N & 1)) << (LOG2_N - 1);
        }

        X[k] = FFTBuffer[p.x + p.y * N];
    }

    GroupMemoryBarrierWithGroupSync();
    float d = inverse ? 1.0 : -1.0;

    // Do the radix-4 passes if chosen
    for (uint p = 0; RADIX == 4 && p < LOG4_N; p++) {
        uint s = 1 << (2 * p);

        for (uint i = 0; i < N / WORKGROUP_SIZE / 4; i++) {
            uint j = local_index + i * WORKGROUP_SIZE;
            uint k = j & (s - 1);
            float t = d * 2.0 * PI / float(s * 4) * float(k);
            uint k0 = ((j >> (2 * p)) << (2 * p + 2)) + k;
            uint k1 = k0 + 1 * s;
            uint k2 = k0 + 2 * s;
            uint k3 = k0 + 3 * s;
            float2 x0 = X[k0];
            float2 x1 = cmul(cis(t), X[k1]);
            float2 x2 = cmul(cis(t * 2.0), X[k2]);
            float2 x3 = cmul(cis(t * 3.0), X[k3]);
            X[k0] = x0 + x1 + x2 + x3;
            X[k1] = x0 + d * float2(-x1.y, x1.x) - x2 + d * float2(x3.y, -x3.x);
            X[k2] = x0 - x1 + x2 - x3;
            X[k3] = x0 + d * float2(x1.y, -x1.x) - x2 + d * float2(-x3.y, x3.x);
        }

        GroupMemoryBarrierWithGroupSync();
    }

    // Do the radix-2 passes if chosen or if there is a leftover pass
    // required from doing the radix-4 passes (i.e. if N isn't a power of 4)
    for (uint p = (RADIX == 4 ? 2 * LOG4_N : 0); p < LOG2_N; p++) {
        uint s = 1 << p;

        for (uint i = 0; i < N / WORKGROUP_SIZE / 2; i++) {
            uint j = local_index + i * WORKGROUP_SIZE;
            uint k = j & (s - 1);
            uint k0 = ((j >> p) << (p + 1)) + k;
            uint k1 = k0 + s;
            float2 x0 = X[k0];
            float2 x1 = cmul(cis(d * 2.0 * PI / float(s * 2) * float(k)), X[k1]);
            X[k0] = x0 + x1;
            X[k1] = x0 - x1;
        }

        GroupMemoryBarrierWithGroupSync();
    }

    // Copy the FFT'd workgroup memory back into the storage buffer
    for (uint i = 0; i < N / WORKGROUP_SIZE; i++) {
        uint j = local_index + i * WORKGROUP_SIZE;
        uint2 p = uint2(j, workgroup_index);
        FFTBuffer[p.x + p.y * N] = X[j] / (inverse ? N : 1.0);
    }
}