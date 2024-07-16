#pragma once

#include <cmath>
#include "SRGB.h"

namespace DX12Utils
{
	// adapted from https://github.com/TheRealMJP/BakingLab

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

	inline unsigned char SRGBToLinear(unsigned char color)
	{
		float f = float(color) / 255.0f;
		f = SRGBToLinear(f);
		f *= 255.0f;
		if (f < 0.0f)
			f = 0.0f;
		if (f > 255.0f)
			f = 255.0f;
		return (unsigned char)f;
	}

	inline unsigned char LinearTosRGB(unsigned char color)
	{
		float f = float(color) / 255.0f;
		f = LinearTosRGB(f);
		f *= 255.0f;
		if (f < 0.0f)
			f = 0.0f;
		if (f > 255.0f)
			f = 255.0f;
		return (unsigned char)f;
	}

}; // namespace DX12Utils