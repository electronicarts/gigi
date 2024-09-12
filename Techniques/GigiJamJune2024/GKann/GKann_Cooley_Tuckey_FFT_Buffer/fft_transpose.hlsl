// FFT1024 technique, shader ComplexSquareTranspose
/*$(ShaderResources)*/

#include "fft_consts.hlsl"

groupshared float2 X[2][32][32];

// Integer approximation to derive Y coordinates of triangular rows for linear index values
uint calculateLowerTriangularRow(uint tileIndex)
{
	return 1 + uint( (sqrt(1 + 8.f*tileIndex) - 1) / 2 );
}

// This is the triangular numbers formula with x = (x-1)
// Used to derive X coordinates of trinagular rows from linear index values
uint calculateLowerTriangularRowStart(uint row)
{
	return ((row - 1) * (row)) >> 1;
}

/*$(_compute:fftTranspose)*/(uint3 workgroupId : SV_GroupID, uint3 threadId : SV_GroupThreadID)
{
	// 2048 * 2048 complex values
	// break into 64x64 tiles of 32x32
	// Each tile fits in 1024 float2 array of groupshared memory
	// Each of the 32x32 threads gets to tranpose 1 value each

	// Diagonal tiles:
	// 64 main-diagonal tiles = 64 dispatches
	// Each group handles 1 tile

	// Non-diagonal tiles:
	// 64x64 total tiles = 4096 tiles - 64 diagonal = 4032 non-diagonal tiles
	// Each group handles 2 tiles = 4032 / 2 = 2016 dispatches

	// Total dispatches = 64 + 2016 = 2080

	uint workgroupIndex = workgroupId.x;
	if (workgroupIndex < 64) // Diagonal (just transpose locally)
	{
		// Copy values from the storage buffer into workgroup memory
		{
			uint2 grId = uint2(workgroupIndex * 32 + threadId.x, workgroupIndex * 32 + threadId.y);
			uint swizThreadIdX = threadId.y ^ threadId.x; // Avoids bank conflicts
			uint swizThreadId = threadId.y * 32 + swizThreadIdX;
			uint2 swId = uint2(swizThreadId % 32, swizThreadId / 32);
			X[0][swId.y][swId.x] = FFTBufferOrigin[grId.x + grId.y * N];
		}

		GroupMemoryBarrierWithGroupSync();

		// Copy values transposed from workgroup memory
		{
			uint2 grId = uint2(workgroupIndex * 32 + threadId.x, workgroupIndex * 32 + threadId.y);
			uint swizThreadIdY = threadId.y ^ threadId.x; // Avoids bank conflicts
			uint transThreadId = threadId.x * 32 + swizThreadIdY;
			uint2 swId = uint2(transThreadId % 32, transThreadId / 32);
			FFTBufferTarget[grId.x + grId.y * N] = X[0][swId.y][swId.x];
		}
	}
	else // Non-diagonal (transpose two diagonally mirrored tiles and swap)
	{
		// Triangular indices require especial treatment
		// LINEAR ID | X  D I M 
		// - - - - - | - - - - -
		// 0 - - - - | 0 - - - -
		// 1 2 - - - | 0 1 - - -
		// 3 4 5 - - | 0 1 2 - -
		// 6 7 8 9 - | 0 1 2 3 -
		// ----------x
		// - - - - - | Y
		// 1 - - - - | 
		// 2 2 - - - | D
		// 3 3 3 - - | I
		// 4 4 4 4 - | M

		// The Y dimensional index correlates to the n-th triangular number. By solving the triangular root with an integer division
		// we can effectively get the (n)th triangular root for any linear id that falls in our Y dimension coordinates (offset by 1).
		// Similarly, we can compute the X dimension coordinates by removing the (n-1)th triangular number from it.
		// https://en.wikipedia.org/wiki/Triangular_number#Triangular_roots_and_tests_for_triangular_numbers

		// Triangular indices mirrored along the diagonal (just swapping X and Y)
		// LINEAR ID | D I M   X
		// - 0 1 3 6 | - 1 2 3 4
		// - - 2 4 7 | - - 2 3 4
		// - - - 5 8 | - - - 3 4
		// - - - - 9 | - - - - 4
		// - - - - - | - - - - -
		// ----------x
		// - 0 0 0 0 | D
		// - - 1 1 1 | I
		// - - - 2 2 | M
		// - - - - 3 |  
		// - - - - - | Y

		uint tileId = (workgroupIndex - 64);
		uint tileRow = calculateLowerTriangularRow(tileId);
		uint tileCol = tileId - calculateLowerTriangularRowStart(tileRow);

		// Copy first tile values from the storage buffer into first tile workgroup memory
		{
			uint2 grId = uint2(tileCol * 32 + threadId.x, tileRow * 32 + threadId.y);
			uint swizThreadIdX = threadId.y ^ threadId.x; // Avoids bank conflicts
			uint swizThreadId = threadId.y * 32 + swizThreadIdX;
			uint2 swId = uint2(swizThreadId % 32, swizThreadId / 32);
			X[0][swId.y][swId.x] = FFTBufferOrigin[grId.x + grId.y * N];
		}

		// Copy second tile values from the storage buffer into second tile workgroup memory
		{
			uint2 grId = uint2(tileRow * 32 + threadId.x, tileCol * 32 + threadId.y);
			uint swizThreadIdX = threadId.y ^ threadId.x; // Avoids bank conflicts
			uint swizThreadId = threadId.y * 32 + swizThreadIdX;
			uint2 swId = uint2(swizThreadId % 32, swizThreadId / 32);
			X[1][swId.y][swId.x] = FFTBufferOrigin[grId.x + grId.y * N];
		}

		GroupMemoryBarrierWithGroupSync();

		// Copy second tile values (transposed) from it's workgroup memory to first tile storage buffer
		{
			uint2 grId = uint2(tileCol * 32 + threadId.x, tileRow * 32 + threadId.y);
			uint swizThreadIdY = threadId.y ^ threadId.x; // Avoids bank conflicts
			uint transThreadId = threadId.x * 32 + swizThreadIdY;
			uint2 swId = uint2(transThreadId % 32, transThreadId / 32);
			FFTBufferTarget[grId.x + grId.y * N] = X[1][swId.y][swId.x];
		}

		// Copy first tile values (transposed) from it's workgroup memory to second tile storage buffer
		{
			uint2 grId = uint2(tileRow * 32 + threadId.x, tileCol * 32 + threadId.y);
			uint swizThreadIdY = threadId.y ^ threadId.x; // Avoids bank conflicts
			uint transThreadId = threadId.x * 32 + swizThreadIdY;
			uint2 swId = uint2(transThreadId % 32, transThreadId / 32);
			FFTBufferTarget[grId.x + grId.y * N] = X[0][swId.y][swId.x];
		}
	}
}

/*
Shader Resources:
	Buffer FFTBufferOrigin (as SRV)
	Buffer FFTBufferTarget (as UAV)
*/