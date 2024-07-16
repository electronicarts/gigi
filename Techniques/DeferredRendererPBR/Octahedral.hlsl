//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#ifndef __OCTAHEDRAL__
#define __OCTAHEDRAL__

// Octahedral mapping for normal vectors, "fast" version
// See http://jcgt.org/published/0003/02/01/

// Returns ±1
float2 signNotZero(float2 v)
{
	//return select(v > 0.0f, float(1).xx, float(-1).xx);
	return float2(v.x > 0.0f ? 1.0f : -1.0f, v.y > 0.0f ? 1.0f : -1.0f);
}

// Assumes normalized input. Output is on [-1, 1] for each component.
float2 packOctahedralSnorm(float3 v)
{
	float2 p = v.xy * (1.0f / (abs(v.x) + abs(v.y) + abs(v.z)));
	return (v.z <= 0.0f) ? ((1.0f - abs(p.yx)) * signNotZero(p)) : p;
}

// Input is [-1, 1] for each component. Output is normalized.
float3 unpackOctahedralSnorm(float2 e)
{
	float3 v = float3(e.xy, 1.0f - abs(e.x) - abs(e.y));
	if (v.z < 0.0f) v.xy = (1.0f - abs(v.yx)) * signNotZero(v.xy);
	return normalize(v);
}

// Assumes normalized input. Output is on [0, 1] for each component.
float2 packOctahedralUnorm(float3 v)
{
	return packOctahedralSnorm(v) * 0.5f + 0.5f;
}

// Input is [0, 1] for each component. Output is normalized.
float3 unpackOctahedralUnorm(float2 e)
{
	return unpackOctahedralSnorm(e * 2.0f - 1.0f);
}

#endif