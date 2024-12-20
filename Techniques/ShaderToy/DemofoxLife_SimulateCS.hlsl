// Unnamed technique, shader Simulate
/*$(ShaderResources)*/

// Adapted from a great webGPU tutorial: https://codelabs.developers.google.com/your-first-webgpu-app#0

uint wang_hash_init(uint3 seed)
{
	return uint(seed.x * uint(1973) + seed.y * uint(9277) + seed.z * uint(26699)) | uint(1);
}

uint wang_hash_uint(inout uint seed)
{
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}

float wang_hash_float01(inout uint state)
{
	return float(wang_hash_uint(state) & 0x00FFFFFF) / float(0x01000000);
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	const int2 px = int2(DTid.xy);
	const int simReadIndex = /*$(Variable:FrameIndex)*/ % 2;
	const int simWriteIndex = (/*$(Variable:FrameIndex)*/ + 1) % 2;
	const int2 gridSize = /*$(Variable:GridSize)*/;

	// Handle randomization
	if (/*$(Variable:Randomize)*/)
	{
		uint rng = wang_hash_init(uint3(DTid.xy, asuint(/*$(Variable:iTime)*/)));
		uint writeValue = (wang_hash_float01(rng) < /*$(Variable:RandomDensity)*/) ? 255 : 0;
		GridState[uint3(px, 0)] = writeValue;
		GridState[uint3(px, 1)] = writeValue;
		return;
	}

	// Handle drawing with mouse
	const float4 mouseState = /*$(Variable:MouseState)*/;
	{
		if (mouseState.z != 0.0f || mouseState.w != 0.0f)
		{
			uint writeValue = (mouseState.z != 0.0f) ? 255 : 0;

			float2 uv = mouseState.xy / float2(/*$(Variable:RenderSize)*/);
			int2 cellIndex = int2(uv * float2(gridSize));

			if (px.x == cellIndex.x && px.y == cellIndex.y)
			{
				if (/*$(Variable:Pause)*/)
				{
					GridState[uint3(px, 0)] = writeValue;
					GridState[uint3(px, 1)] = writeValue;
				}
				else
				{
					GridState[uint3(px, simWriteIndex)] = writeValue;
				}
				return;
			}
		}
	}

	// Handle clearing
	if (/*$(Variable:FrameIndex)*/ < 1 || /*$(Variable:Clear)*/)
	{
		GridState[uint3(px, 0)] = 0;
		GridState[uint3(px, 1)] = 0;
		return;
	}

	// See if we should simulate
	bool simulate = true;
	if (/*$(Variable:Pause)*/)
	{
		if (!/*$(Variable:SingleStep)*/)
			simulate = false;
	}
	else if (/*$(Variable:FrameIndex)*/ % /*$(Variable:SimPeriod)*/ != 0)
	{
		simulate = false;
	}

	// Do a pass through if not simulating
	if (!simulate)
	{
		GridState[uint3(px, simWriteIndex)]  = GridState[uint3(px, simReadIndex)];
		return;
	}

	// Count neighbors
	int neighborCount = 0;
	for (int iy = -1; iy <= 1; ++iy)
	{
		for (int ix = -1; ix <= 1; ++ix)
		{
			if (ix == 0 && iy == 0)
				continue;

			int2 readpx = px + int2(ix, iy);
			int2 readpxWrapped = (readpx + gridSize) % gridSize;
			if (!/*$(Variable:WrapAround)*/ && (readpx.x != readpxWrapped.x || readpx.y != readpxWrapped.y))
				continue;

			if (GridState[uint3(readpxWrapped, simReadIndex)] == 255)
				neighborCount++;
		}
	}

	switch (neighborCount)
	{
		// active cells with 2 neighbors stay active
		case 2: GridState[uint3(px, simWriteIndex)] = GridState[uint3(px, simReadIndex)]; break;

		// Cells with 3 neighbors become or stay active
		case 3: GridState[uint3(px, simWriteIndex)] = 255; break;

		// Cells with < 2 or > 3 neighbors become inactive
		default: GridState[uint3(px, simWriteIndex)] = 0; break;
	}
}

/*
Shader Resources:
	Texture GridState (as UAV)
*/
