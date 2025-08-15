// SlangAutoDiff technique, shader Descend
/*$(ShaderResources)*/

static const float c_pi = 3.14159265359f;

[Differentiable]
float GetHeightAtPos(float x, float y, no_diff float2 gaussPos, no_diff float2 gaussSigma)
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

[shader("compute")]
/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	float2 ballPos = float2(Data[0], Data[1]);
	float2 ballPosGradient = float2(0.0f, 0.0f);

	for (int i = 0; i < /*$(Variable:NumGaussians)*/; ++i)
	{
		// Get the gaussian data from the buffer
		float2 gaussPos = float2(Data[4+i*5+0], Data[4+i*5+1]);
		float gaussAngle = Data[4+i*5+2];
		float2 gaussSigma = float2(Data[4+i*5+3], Data[4+i*5+4]);

		// Put the ball in the coordinate system of this gaussian
		float2 relativePos = ballPos - gaussPos;
		const float cosTheta = cos(-gaussAngle);
		const float sinTheta = sin(-gaussAngle);
		float2 pos = float2(
			relativePos.x * cosTheta - relativePos.y * sinTheta,
			relativePos.x * sinTheta + relativePos.y * cosTheta
		);

		// Get local derivatives
		float2 dFLocal = float2(0.0f, 0.0f);

		// Backward mode automatic differentiation (AD) - AKA Backpropagation
		if ((bool)/*$(Variable:UseBackwardAD)*/)
		{
			// get local dFdX and dFdy
			float height = GetHeightAtPos(pos.x, pos.y, gaussPos, gaussSigma);

			var ballPosX = diffPair(pos.x);
			var ballPosY = diffPair(pos.y);
			bwd_diff(GetHeightAtPos)(ballPosX, ballPosY, gaussPos, gaussSigma, height);

			dFLocal = float2(ballPosX.d, ballPosY.d);
		}
		// Forward mode automatic differentiation (AD) - AKA Dual numbers
		else
		{
			// get local dFdX
			{
				var ballPosX = diffPair(pos.x, 1.0f);
				var ballPosY = diffPair(pos.y, 0.0f);
				let result = fwd_diff(GetHeightAtPos)(ballPosX, ballPosY, gaussPos, gaussSigma);
				dFLocal.x = result.d;
			}

			// get local dFdy
			{
				var ballPosX = diffPair(pos.x, 0.0f);
				var ballPosY = diffPair(pos.y, 1.0f);
				let result = fwd_diff(GetHeightAtPos)(ballPosX, ballPosY, gaussPos, gaussSigma);
				dFLocal.y += result.d;
			}
		}

		// put gradient into global space and add it into the total gradient
		const float cosNegTheta = cos(gaussAngle);
		const float sinNegTheta = sin(gaussAngle);
		float2 dFGlobal = float2(
			dFLocal.x * cosNegTheta - dFLocal.y * sinNegTheta,
			dFLocal.x * sinNegTheta + dFLocal.y * cosNegTheta
		);
		ballPosGradient += dFGlobal;
	}

	// Adjust the ball based on the gradient
	float2 adjust = - ballPosGradient * /*$(Variable:LearningRate)*/;
	if (length(adjust) > /*$(Variable:MaximumStepSize)*/)
		adjust = normalize(adjust) * /*$(Variable:MaximumStepSize)*/;
	ballPos += adjust;

	// make sure the ball doesn't go out of bounds
	// SCREEN_MIN and SCREEN_MAX are defines just to exercise node / shader defines.
	ballPos = clamp(ballPos, float2(SCREEN_MIN, SCREEN_MIN), float2(SCREEN_MAX, SCREEN_MAX));

	// set the ball pos
	Data[0] = ballPos.x;
	Data[1] = ballPos.y;

	// Set the gradient
	Data[2] = ballPosGradient.x;
	Data[3] = ballPosGradient.y;
}

/*
Shader Resources:
	Buffer Data (as UAV)
*/
