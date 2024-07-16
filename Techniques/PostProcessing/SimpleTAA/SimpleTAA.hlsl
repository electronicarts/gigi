// SimpleTAA_NoMotionVectors technique, shader AccumulateCS
/*$(ShaderResources)*/

float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

float3 SRGBToLinear(in float3 sRGBCol)
{
	float3 linearRGBLo = sRGBCol / 12.92;
	float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
	float3 linearRGB;
	linearRGB.r = sRGBCol.r <= 0.04045 ? linearRGBLo.r : linearRGBHi.r;
	linearRGB.g = sRGBCol.g <= 0.04045 ? linearRGBLo.g : linearRGBHi.g;
	linearRGB.b = sRGBCol.b <= 0.04045 ? linearRGBLo.b : linearRGBHi.b;
	return linearRGB;
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	Accum.GetDimensions(dims.x, dims.y);

	uint2 px = DTid.xy;

	if (!/*$(Variable:Enabled)*/)
	{
		Accum[px] = Input[px];
		return;
	}

	if (/*$(Variable:resetHistory)*/)
	{
		Accum[px] = float4(0.0f, 0.0f, 0.0f, 1.0f);
		return;
	}

	// Get the pixel location last frame. Default to the current pixel location
	uint2 pxLastFrame = px;

	// Use motion vectors if we should
	if (/*$(Variable:UseMotionVectors)*/)
	{
		pxLastFrame = uint2(float2(px) + float2(0.5f, 0.5f) - Velocity[px].rg * float2(dims) * 0.5f);
	}	
	// Else do temporal reprojection if we should
	else if (/*$(Variable:TemporalReprojection)*/)
	{
		// If the depth buffer is empty at this pixel, take the current pixel value
		float depth = Depth[px];
		if (depth == /*$(Variable:DepthClearValue)*/)
		{
			Accum[px] = Input[px];
			return;
		}
		else
		{
			// Get the world positions of this pixel
			float2 screenPos = (float2(px)+0.5f) / float2(dims) * 2.0 - 1.0;
			screenPos.y = -screenPos.y;
			float4 worldPos = mul(float4(screenPos, depth, 1.0f), /*$(Variable:InvJitteredViewProjMtx)*/);
			worldPos /= worldPos.w;

			// get the screen position of this pixel last frame
			float4 screenPosLastFrame = mul(worldPos, /*$(Variable:JitteredViewProjMtxLastFrame)*/);
			screenPosLastFrame /= screenPosLastFrame.w;
			screenPosLastFrame.y = -screenPosLastFrame.y;
			pxLastFrame = uint2((screenPosLastFrame.xy * 0.5f + 0.5f) * float2(dims));
		}
	}

	// clamp pxLastFrame to valid values
	pxLastFrame = clamp(pxLastFrame, uint2(0,0), uint2(dims.x - 1, dims.y - 1));

	// Get the accumulated color for this pixel
	float3 oldColor = Accum[pxLastFrame].rgb;
	if (/*$(Variable:sRGB)*/)
		oldColor = SRGBToLinear(oldColor);

	// get the new color
	float3 newColor = Input[px].rgb;
	if (/*$(Variable:sRGB)*/)
		newColor = SRGBToLinear(newColor);

	// Do neighborhood color clamping if we should
	if(/*$(Variable:ColorNeighborhoodMode)*/ == ColorNeighborhoodModes::Clamp)
	{
		float3 colorMin = newColor;
		float3 colorMax = newColor;

		for (int i = -1; i <= 1; ++i)
		{
			for (int j = -1; j <= 1; ++j)
			{
				int2 readPx = clamp(int2(px) + int2(i,j), 0, int2(dims) - int2(1,1)); 
				float3 prevColor = Input[readPx].rgb;
				colorMin = min(colorMin, prevColor);
				colorMax = max(colorMax, prevColor);
			}
		}

		oldColor = clamp(oldColor, colorMin, colorMax);
	}

	// integrate
	float3 outputColor = lerp(oldColor, newColor, /*$(Variable:alpha)*/);

	// store result
	if (/*$(Variable:sRGB)*/)
		outputColor = LinearToSRGB(outputColor);
	Accum[px] = float4(outputColor, 1.0f);
}
