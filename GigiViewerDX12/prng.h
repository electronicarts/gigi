///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

inline float halton(int index, float base)
{
    float result = 0.0f;
    float f = 1.0f / base;
    float i = float(index);

    for (int x = 0; x < 8; x++)
    {
        if (i <= 0.0f)
            break;

        result += f * std::fmodf(i, base);
        i = std::floor(i / base);
        f = f / base;
    }

    return result;
}

inline uint32_t wang_hash_init(uint32_t seed0, uint32_t seed1, uint32_t seed2)
{
    return uint32_t(seed0 * uint32_t(1973) + seed1 * uint32_t(9277) + seed2 * uint32_t(26699)) | uint32_t(1);
}

inline uint32_t wang_hash_uint(uint32_t& seed)
{
    seed = uint32_t(seed ^ uint32_t(61)) ^ uint32_t(seed >> uint32_t(16));
    seed *= uint32_t(9);
    seed = seed ^ (seed >> 4);
    seed *= uint32_t(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

inline float wang_hash_float01(uint32_t& state)
{
    return float(wang_hash_uint(state) & 0x00FFFFFF) / float(0x01000000);
}
