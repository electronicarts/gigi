///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Assumes:
// * firstPos is a float3.
// * you add "stride" bytes toget to the next position.
// * count is the number of times you do this. (the total number of positions)
bool SaveAsBVH(const char* fileName, const unsigned char* firstPos, size_t stride, size_t count);
