// Unnamed technique, shader MakeLineList
/*$(ShaderResources)*/

static const float c_pi = 3.14159265359f;

uint DecodeTrinary(inout uint symbol)
{
	uint value = symbol % 3;
	symbol /= 3;
	return value;
}

int DecodeSignedTrinary(inout uint symbol)
{
	uint value = symbol % 3;
	symbol /= 3;
	switch (value)
	{
		case 0: return 0;
		case 1: return 1;
		case 2: return -1;
	}
	return 0;
}

bool DecodeBinary(inout uint symbol)
{
	uint value = symbol % 2;
	symbol /= 2;
	return value != 0;
}

// // https://blog.demofox.org/2013/10/12/converting-to-and-from-polar-spherical-coordinates-made-easy/
float3 SphericalToCartesian(float2 rads)
{
	float cx = cos(rads.x);
	float sx = sin(rads.x);
	float cy = cos(rads.y);
	float sy = sin(rads.y);

	return float3(cx * cy, sx * cy, sy);
}

void Rot(inout float3 XAxis, inout float3 YAxis, inout float3 ZAxis, float3x3 mat, bool absolute)
{
	float3x3 result = absolute ? mul(float3x3(XAxis, YAxis, ZAxis), mat) : mul(mat, float3x3(XAxis, YAxis, ZAxis));
	XAxis = result[0];
	YAxis = result[1];
	ZAxis = result[2];
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	// initialize
	uint vertexIndex = 0;
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float3 cursorPos = /*$(Variable:CursorPos)*/;
	float3 XAxis = normalize(/*$(Variable:CursorXAxis)*/);
	float3 YAxis = normalize(/*$(Variable:CursorYAxis)*/);
	float3 ZAxis = cross(XAxis, YAxis);

	float3 BBMin = float3(0.0f, 0.0f, 0.0f);
	float3 BBMax = float3(0.0f, 0.0f, 0.0f);
	bool firstDraw = true;

	uint cursorStateStackIndex = 0;
	bool cursorStateStackOverflow = false;

	const float cosTheta = cos(/*$(Variable:CursorRotationDegrees)*/ * c_pi / 180.0f);
	const float sinTheta = sin(/*$(Variable:CursorRotationDegrees)*/ * c_pi / 180.0f);

	const float3x3 rotX = float3x3(
		float3(1.0f, 0.0f, 0.0f),
		float3(0.0f, cosTheta, -sinTheta),
		float3(0.0f, sinTheta, cosTheta)
	);

	const float3x3 rotY = float3x3(
		float3(cosTheta, 0.0f, sinTheta),
		float3(0.0f, 1.0f, 0.0f),
		float3(-sinTheta, 0.0f, cosTheta)
	);

	const float3x3 rotZ = float3x3(
		float3(cosTheta, -sinTheta, 0.0f),
		float3(sinTheta, cosTheta, 0.0f),
		float3(0.0f, 0.0f, 1.0f)
	);

	uint rulesSize;
	Rules.GetDimensions(rulesSize);

	for (uint srcSymbolIndex = 0; srcSymbolIndex < GlobalState[0].SymbolsCount; ++srcSymbolIndex)
	{
		// Get the command for this symbol
		uint command = ~0;
		{
			uint symbol = (Symbols[srcSymbolIndex]) % /*$(Variable:RulesMax)*/;
			uint ruleIndex = RulesIndex[symbol];
			if (ruleIndex < rulesSize)
				command = Rules[ruleIndex];
		}

		// If the command is out of range of the bitpacking, nothing to draw.
		if (command >= 324)
		{
			// push cursor state
			if (command == 324)
			{
				if (cursorStateStackIndex < /*$(Variable:CursorStateStackDepth)*/)
				{
					CursorStateStack[cursorStateStackIndex].Position = cursorPos;
					CursorStateStack[cursorStateStackIndex].XAxis = XAxis;
					CursorStateStack[cursorStateStackIndex].YAxis = YAxis;
					cursorStateStackIndex++;
				}
				else
					cursorStateStackOverflow = true;
			}
			// pop cursor state
			else if (command == 325)
			{
				if (cursorStateStackIndex > 0)
				{
					cursorStateStackIndex--;
					cursorPos = CursorStateStack[cursorStateStackIndex].Position;
					XAxis = CursorStateStack[cursorStateStackIndex].XAxis;
					YAxis = CursorStateStack[cursorStateStackIndex].YAxis;
					ZAxis = cross(XAxis, YAxis);
				}
				else
					cursorStateStackOverflow = true;
			}

			continue;
		}

		// Decode the command
		uint cursorMoveAxis = DecodeTrinary(command);
		float cursorMoveDistance = (float)DecodeSignedTrinary(command);
		uint cursorRotationAxis = DecodeTrinary(command);
		int cursorRotationDistance = DecodeSignedTrinary(command);
		bool absolute = DecodeBinary(command);
		bool draw = DecodeBinary(command);

		// plot the first point of the line if we are drawing
		if (draw)
		{
			BBMin = firstDraw ? cursorPos : min(BBMin, cursorPos);
			BBMax = firstDraw ? cursorPos : max(BBMax, cursorPos);
			firstDraw = false;

			LineList[vertexIndex].Position = cursorPos;
			LineList[vertexIndex].Color = color;
			vertexIndex++;
		}

		// Do cursor rotation
		if (cursorRotationDistance != 0)
		{
			bool pos = cursorRotationDistance > 0;
			switch (cursorRotationAxis)
			{
				case 0: Rot(XAxis, YAxis, ZAxis, pos ? rotX : transpose(rotX), absolute); break;
				case 1: Rot(XAxis, YAxis, ZAxis, pos ? rotY : transpose(rotY), absolute); break;
				case 2: Rot(XAxis, YAxis, ZAxis, pos ? rotZ : transpose(rotZ), absolute); break;
			}
		}

		// Do absolute offset if we should
		if (absolute)
		{
			switch(cursorMoveAxis)
			{
				case 0: cursorPos.x += cursorMoveDistance; break;
				case 1: cursorPos.y += cursorMoveDistance; break;
				case 2: cursorPos.z += cursorMoveDistance; break;
			}
		}
		// Else do relative offset
		else
		{
			switch(cursorMoveAxis)
			{
				case 0: cursorPos += XAxis * cursorMoveDistance; break;
				case 1: cursorPos += YAxis * cursorMoveDistance; break;
				case 2: cursorPos += ZAxis * cursorMoveDistance; break;
			}
		}

		// plot the second point of the line if we are drawing
		if (draw)
		{
			BBMin = min(BBMin, cursorPos);
			BBMax = max(BBMax, cursorPos);

			LineList[vertexIndex].Position = cursorPos;
			LineList[vertexIndex].Color = color;
			vertexIndex++;
		}

		if (srcSymbolIndex == 0)
		{
			/*
			GlobalState[0].Debug1 = float4(XAxis, 0.0f);
			GlobalState[0].Debug2 = float4(YAxis, 0.0f);
			GlobalState[0].Debug3 = float4(ZAxis, 0.0f);
			GlobalState[0].Debug4 = float4(0.0f, 0.0f, 0.0f, 0.0f);

			float2 cursorRotF = float2(cursorRot) * c_pi / 16.0f;

			float3 xAxis = SphericalToCartesian(cursorRotF);
			float3 yAxis = SphericalToCartesian(cursorRotF + float2(c_pi / 2.0f, 0.0f));
			float3 zAxis = SphericalToCartesian(cursorRotF + float2(0.0f, c_pi / 2.0f));

			GlobalState[0].Debug1 = float4(xAxis, 0.0f);
			GlobalState[0].Debug2 = float4(yAxis, 0.0f);
			GlobalState[0].Debug3 = float4(zAxis, 0.0f);
			GlobalState[0].Debug4 = float4(cursorRot, 0.0f, 0.0f);
			*/

			//GlobalState[0].Debug1 = float4(offsetX, offsetY, offsetZ, 0.0f);
			//GlobalState[0].Debug2 = float4(rotateX, rotateY, absolute, draw);

			//GlobalState[0].Debug1 = float4(oldLastPoint, 0.0f);
			//GlobalState[0].Debug2 = float4(lastPoint, 0.0f);
		}
	}

	// store the bounding box so we can constrain the lines to a box
	GlobalState[0].LinesBBMin = BBMin;
	GlobalState[0].LinesBBMax = BBMax;

	// write out how many vertices were made
	GlobalState[0].LineListCount = vertexIndex;

	// log whether we had problems with the cursor state stack or not
	GlobalState[0].CursorStateStackOverflow = cursorStateStackOverflow || (cursorStateStackIndex != 0);

	// store the cursor's ending state in case we want to see it
	GlobalState[0].CursorEndPosition = cursorPos;
	GlobalState[0].CursorEndXAxis = XAxis;
	GlobalState[0].CursorEndYAxis = YAxis;
	GlobalState[0].CursorEndZAxis = ZAxis;
}

/*
Shader Resources:
	Buffer GlobalState (as UAV)
	Buffer Symbols (as SRV)
	Buffer LineList (as UAV)
*/
