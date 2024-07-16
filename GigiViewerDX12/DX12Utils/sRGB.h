///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// adapted from https://github.com/TheRealMJP/BakingLab

#include <cmath>

inline float SRGBToLinear(float color)
{
    float x = color / 12.92f;
    float y = std::pow((color + 0.055f) / 1.055f, 2.4f);
    return color <= 0.04045f ? x : y;
}

inline float LinearTosRGB(float color)
{
    float x = color * 12.92f;
    float y = std::pow(color, 1.0f / 2.4f) * 1.055f - 0.055f;
    return color < 0.0031308f ? x : y;
}
