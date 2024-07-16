// SlangAutoDiff technique, shader Render
/*$(ShaderResources)*/

static const float c_pi = 3.14159265359f;

float GetHeightAtPos(float x, float y, float2 gaussPos, float2 gaussSigma)
{
	float gaussX;
	{
		float XOverSigma = x / gaussSigma.x;
		float numerator = exp(-0.5 * XOverSigma * XOverSigma);
		gaussX = numerator / (gaussSigma.x * sqrt(2.0f * c_pi));
	}

	float gaussY;
	{
		float XOverSigma = y / gaussSigma.y;
		float numerator = exp(-0.5 * XOverSigma * XOverSigma);
		gaussY = numerator / (gaussSigma.y * sqrt(2.0f * c_pi));
	}

	return gaussX * gaussY;
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// get pixel location and UV
	uint2 dims;
	Output.GetDimensions(dims.x, dims.y);
	uint2 px = DTid.xy;
	float2 uv = (float2(px) + 0.5f) / float2(dims);

	// calculate the height at this pixel
	float height = 0.0f;
	float maxHeight = 0.0f;
	for (int i = 0; i < /*$(Variable:NumGaussians)*/; ++i)
	{
		// Get the data from the buffer
		float2 gaussPos = float2(Data[4+i*5+0], Data[4+i*5+1]);
		float gaussAngle = Data[4+i*5+2];
		float2 gaussSigma = float2(Data[4+i*5+3], Data[4+i*5+4]);

		// Put the pixel in the coordinate system of this gaussian
		float2 pxRelativePos = uv - gaussPos;
		const float cosTheta = cos(-gaussAngle);
		const float sinTheta = sin(-gaussAngle);
		float2 pos = float2(
			pxRelativePos.x * cosTheta - pxRelativePos.y * sinTheta,
			pxRelativePos.x * sinTheta + pxRelativePos.y * cosTheta
		);

		// Find the height of the hill of this gaussian and add it to the height of the other gaussians so far
		height += GetHeightAtPos(pos.x, pos.y, gaussPos, gaussSigma);

		// Also keep track of the maximum height possible, to normalize the height a bit later
		maxHeight += GetHeightAtPos(0.0f, 0.0f, gaussPos, gaussSigma);
	}

	// draw the ball
	float ball = 0.0f;
	{
		float ballSizeInPixels = 5.0f;

		float2 ballPos = float2(Data[0], Data[1]);
		float ballDist = length(ballPos - uv);

		float ballSize = ballSizeInPixels / float(dims.x);

		ball = smoothstep(ballSize, 0.0f, ballDist);
	}

	// Write the normalized height into the red channel, and the ball into the green channel
	float color = height / maxHeight;
	if (/*$(Variable:QuantizeDisplay)*/)
		color = floor(color * 64.0f + 0.5f) / 64.0f;
	Output[px] = float4(color, ball, 0.0f, 1.0f);
}

/*
Shader Resources:
	Buffer Data (as SRV)
	Texture Output (as UAV)
*/
