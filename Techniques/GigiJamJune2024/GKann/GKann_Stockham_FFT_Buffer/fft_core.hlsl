
#include "fft_consts.hlsl"

groupshared float2 X[2][N];

float2 cmul(float2 x, float2 y) {
    return float2(x.x * y.x - x.y * y.y, x.x * y.y + x.y * y.x);
}

float2 cis(float x) {
    return float2(cos(x), sin(x));
}

// Perform an FFT over one axis of the image
void fft(uint local_index, uint workgroup_index, bool inverse)
{
    // Copy a row/column of values from the storage buffer into workgroup memory
    for (uint i = 0; i < N / WORKGROUP_SIZE; i++) {
        uint j = local_index + i * WORKGROUP_SIZE;
        uint2 p = uint2(j, workgroup_index);
        X[0][j] = FFTBuffer[p.x + p.y * N];
    }

    GroupMemoryBarrierWithGroupSync();
    float inv = inverse ? 1.0 : -1.0;
    uint flip = 0;

    // Do the radix-4 passes if chosen
    for (uint it = 0; RADIX == 4 && it < LOG4_N; it++) {
        uint n = N >> (it * 2);
        uint s = 1 << (it * 2);
        uint n0 = 0;
        uint n1 = n / 4;
        uint n2 = n / 2;
        uint n3 = n1 + n2;
        float theta0 = inv * 2.0 * PI / n;

        for (uint i = 0; i < N / WORKGROUP_SIZE / 4; i++) {
            uint j = local_index + i * WORKGROUP_SIZE;
            uint p = j / s;
            uint q = j % s;
            float2 w1p = cis(p * theta0 * 1);
            float2 w2p = cis(p * theta0 * 2);
            float2 w3p = cis(p * theta0 * 3);
            float2 a = X[flip][q + s*(p + n0)];
            float2 b = X[flip][q + s*(p + n1)];
            float2 c = X[flip][q + s*(p + n2)];
            float2 d = X[flip][q + s*(p + n3)];
            float2  apc =    a + c;
            float2  amc =    a - c;
            float2  bpd =    b + d;
            float2 jbmd = inv * cmul(float2(0, 1), (b - d));
            X[1 - flip][q + s*(4*p + 0)] =      apc +  bpd;
            X[1 - flip][q + s*(4*p + 1)] = cmul(w1p, (amc + jbmd));
            X[1 - flip][q + s*(4*p + 2)] = cmul(w2p, (apc -  bpd));
            X[1 - flip][q + s*(4*p + 3)] = cmul(w3p, (amc - jbmd));
        }

        flip = (flip + 1) % 2;
        GroupMemoryBarrierWithGroupSync();
    }


    // Do the radix-2 passes if chosen or if there is a leftover pass
    // required from doing the radix-4 passes (i.e. if N isn't a power of 4)
    for (uint it = (RADIX == 4 ? 2 * LOG4_N : 0); it < LOG2_N; it++) {
        uint n = N >> it;
        uint s = 1 << it;

        if (n == 2) {
            for (uint i = 0; i < N / WORKGROUP_SIZE / 2; i++) {
                uint j = local_index + i * WORKGROUP_SIZE;
                uint q = j % s;
                float2 a = X[flip][q + 0];
                float2 b = X[flip][q + s];
                X[1 - flip][q + 0] = a + b;
                X[1 - flip][q + s] = a - b;
            }
        }
        else {
            uint m = n / 2;
            float theta0 = inv * 2.0 * PI / n;
            for (uint i = 0; i < N / WORKGROUP_SIZE / 2; i++) {
                uint j = local_index + i * WORKGROUP_SIZE;
                uint p = j / s;
                uint q = j % s;
                float2 wp = cis(p * theta0);
                float2 a = X[flip][q + s*(p + 0)];
                float2 b = X[flip][q + s*(p + m)];
                X[1 - flip][q + s*(2*p + 0)] =  a + b;
                X[1 - flip][q + s*(2*p + 1)] = cmul((a - b), wp);
            }
        }

        flip = (flip + 1) % 2;
        GroupMemoryBarrierWithGroupSync();
    }

    // Copy the FFT'd workgroup memory back into the storage buffer
    for (uint i = 0; i < N / WORKGROUP_SIZE; i++) {
        uint j = local_index + i * WORKGROUP_SIZE;
        uint2 p = uint2(j, workgroup_index);
        FFTBuffer[p.x + p.y * N] = X[flip][j] / (inverse ? N : 1.0);
    }
}