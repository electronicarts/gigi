///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

// Portions of this software were based on https://fgiesen.wordpress.com/2012/03/28/half-to-float-done-quic/
inline float half_to_float_fast5(uint16_t h_)
{
    typedef unsigned int uint;

    union FP32
    {
        uint u;
        float f;
        struct
        {
            uint Mantissa : 23;
            uint Exponent : 8;
            uint Sign : 1;
        };
    };

    union FP16
    {
        unsigned short u;
        struct
        {
            uint Mantissa : 10;
            uint Exponent : 5;
            uint Sign : 1;
        };
    };

    FP16 h;
    h.u = h_;

    static const FP32 magic = { (254 - 15) << 23 };
    static const FP32 was_infnan = { (127 + 16) << 23 };
    FP32 o;

    o.u = (h.u & 0x7fff) << 13;     // exponent/mantissa bits
    o.f *= magic.f;                 // exponent adjust
    if (o.f >= was_infnan.f)        // make sure Inf/NaN survive
        o.u |= 255 << 23;
    o.u |= (h.u & 0x8000) << 16;    // sign bit
    return o.f;
}

// Portions of this software were based on https://gist.github.com/rygorous/2156668
inline uint16_t float_to_half_fast(float f_)
{
    typedef unsigned int uint;

    union FP32
    {
        uint u;
        float f;
        struct
        {
            uint Mantissa : 23;
            uint Exponent : 8;
            uint Sign : 1;
        };
    };

    union FP16
    {
        unsigned short u;
        struct
        {
            uint Mantissa : 10;
            uint Exponent : 5;
            uint Sign : 1;
        };
    };

    FP32 f;
    f.f = f_;

    FP16 o = { 0 };

    // Based on ISPC reference code (with minor modifications)
    if (f.Exponent == 255) // Inf or NaN (all exponent bits set)
    {
        o.Exponent = 31;
        o.Mantissa = f.Mantissa ? 0x200 : 0; // NaN->qNaN and Inf->Inf
    }
    else // Normalized number
    {
        // Exponent unbias the single, then bias the halfp
        int newexp = f.Exponent - 127 + 15;
        if (newexp >= 31) // Overflow, return signed infinity
            o.Exponent = 31;
        else if (newexp <= 0) // Underflow
        {
            if ((14 - newexp) <= 24) // Mantissa might be non-zero
            {
                uint mant = f.Mantissa | 0x800000; // Hidden 1 bit
                o.Mantissa = mant >> (14 - newexp);
                if ((mant >> (13 - newexp)) & 1) // Check for rounding
                    o.u++; // Round, might overflow into exp bit, but this is OK
            }
        }
        else
        {
            o.Exponent = newexp;
            o.Mantissa = f.Mantissa >> 13;
            if (f.Mantissa & 0x1000) // Check for rounding
                o.u++; // Round, might overflow to inf, this is OK
        }
    }

    o.Sign = f.Sign;
    return o.u;
}

inline float f16tof32(uint16_t f16)
{
    return half_to_float_fast5(f16);
}

inline uint16_t f32tof16(float f32)
{
    return float_to_half_fast(f32);
}

// float3 -> 111110
// The standard 32-bit HDR color format.  Each float has a 5-bit exponent and no sign bit.
inline unsigned int f32toR11G11B10(float rgb[3])
{
    unsigned int r = ((f32tof16(rgb[0]) + 8) >> 4) & 0x000007FF;
    unsigned int g = ((f32tof16(rgb[1]) + 8) << 7) & 0x003FF800;
    unsigned int b = ((f32tof16(rgb[2]) + 16) << 17) & 0xFFC00000;
    return r | g | b;
}

// 111110 -> float3 
inline void R11G11B10tof32(uint32_t rgb, float rgbOut[3])
{
    rgbOut[0] = f16tof32((rgb << 4) & 0x7FF0);
    rgbOut[1] = f16tof32((rgb >> 7) & 0x7FF0);
    rgbOut[2] = f16tof32((rgb >> 17) & 0x7FE0);
}