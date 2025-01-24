///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Portions of this software are based on https://github.com/TheRealMJP/BakingLab

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

inline double SRGBToLinear(double color)
{
    double x = color / 12.92;
    double y = std::pow((color + 0.055) / 1.055, 2.4);
    return color <= 0.04045 ? x : y;
}

inline double LinearTosRGB(double color)
{
    double x = color * 12.92;
    double y = std::pow(color, 1.0 / 2.4) * 1.055 - 0.055;
    return color < 0.0031308 ? x : y;
}
