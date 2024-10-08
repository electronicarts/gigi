// Unnamed technique, shader GaussBlur
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// unlisted code from article
	// Kernel calculated at http://demofox.org/gauss.html
	// Sigma 1.0, Support 0.995
	static const int c_kernelSize = 5;
	static const float c_kernel[c_kernelSize] = {0.3829f, 0.2417f, 0.0606f, 0.0060f, 0.0002f};

	int2 px = int2(DTid.xy);
	uint2 dims;
	DownsampledCoC.GetDimensions(dims.x, dims.y);

	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float weight = 0.0f;
	for (int iy = -c_kernelSize; iy <= c_kernelSize; ++iy)
	{
		for (int ix = -c_kernelSize; ix <= c_kernelSize; ++ix)
		{
			int2 readpx = px + int2(ix, iy);
			if (readpx.x >= 0 && readpx.y >= 0 && readpx.x < dims.x && readpx.y < dims.y)
			{
				//uint2 readpx = uint2(px + int2(ix, iy) + int2(dims)) % dims;
				sum += DownsampledCoC[readpx];
				weight += 1.0f;
			}
		}
	}

	BlurredDownsampledCoC[px] = sum / weight;
}

/*
Shader Resources:
	Texture DownsampledCoC (as SRV)
	Texture BlurredDownsampledCoC (as UAV)
*/
