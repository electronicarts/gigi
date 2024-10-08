// Unnamed technique, shader DownSampleCoC
/*$(ShaderResources)*/

#define dofEqWorld (/*$(Variable:EqNear)*/ / /*$(Variable:DepthScale)*/)

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	//28-1
	uint2 px = DTid.xy;

	uint2 dims;
	Color.GetDimensions(dims.x, dims.y);

	/*
	 ___ ___ ___ ___
	|   |   |   |   |
	|___|___|___|___|
	|   |   |   |   |      ___
	|___|___|___|___|  => |   |
	|   |   |   |   |     |___|
	|___|___|___|___|
	|   |   |   |   |
	|___|___|___|___|

	*/

	// We want to average a 4x4 block of pixels from color.
	// We could do 16 reads, but instead, we can do 4 bilinear taps at the center of each 4x4 cell.
	// We add 0.5 when converting from integer pixel coordinates to UVs to get to the center of the cell, where the pixel data is.
	// We add another 0.5 because we actually want to go the center of the 4x4 block
	float2 uv00 = (float2(px * 4 + uint2(0, 0)) + float2(0.5f, 0.5f) + float2(0.5f, 0.5f)) / float2(dims);
	float2 uv10 = (float2(px * 4 + uint2(2, 0)) + float2(0.5f, 0.5f) + float2(0.5f, 0.5f)) / float2(dims);
	float2 uv01 = (float2(px * 4 + uint2(0, 2)) + float2(0.5f, 0.5f) + float2(0.5f, 0.5f)) / float2(dims);
	float2 uv11 = (float2(px * 4 + uint2(2, 2)) + float2(0.5f, 0.5f) + float2(0.5f, 0.5f)) / float2(dims);

	float3 color = Color.SampleLevel(linearWrapSampler, uv00, 0).rgb + Color.SampleLevel(linearWrapSampler, uv10, 0).rgb + Color.SampleLevel(linearWrapSampler, uv01, 0).rgb + Color.SampleLevel(linearWrapSampler, uv11, 0).rgb;
	color /= 4.0f;

	// We want to take the minimum CoC value of every depth value in the 4x4 block 
	float4 depth;
	depth[0] = LinearDepth[px * 4 + uint2(0, 0)].r;
	depth[1] = LinearDepth[px * 4 + uint2(1, 0)].r;
	depth[2] = LinearDepth[px * 4 + uint2(2, 0)].r;
	depth[3] = LinearDepth[px * 4 + uint2(3, 0)].r;
	float4 sceneCoc = saturate(dofEqWorld.x * depth + dofEqWorld.y);
	float4 curCoc = sceneCoc;
	float4 coc = curCoc;

	depth[0] = LinearDepth[px * 4 + uint2(0, 1)].r;
	depth[1] = LinearDepth[px * 4 + uint2(1, 1)].r;
	depth[2] = LinearDepth[px * 4 + uint2(2, 1)].r;
	depth[3] = LinearDepth[px * 4 + uint2(3, 1)].r;
	sceneCoc = saturate(dofEqWorld.x * depth + dofEqWorld.y);
	curCoc = sceneCoc;
	coc = max(coc, curCoc);

	depth[0] = LinearDepth[px * 4 + uint2(0, 2)].r;
	depth[1] = LinearDepth[px * 4 + uint2(1, 2)].r;
	depth[2] = LinearDepth[px * 4 + uint2(2, 2)].r;
	depth[3] = LinearDepth[px * 4 + uint2(3, 2)].r;
	sceneCoc = saturate(dofEqWorld.x * depth + dofEqWorld.y);
	curCoc = sceneCoc;
	coc = max(coc, curCoc);

	depth[0] = LinearDepth[px * 4 + uint2(0, 3)].r;
	depth[1] = LinearDepth[px * 4 + uint2(1, 3)].r;
	depth[2] = LinearDepth[px * 4 + uint2(2, 3)].r;
	depth[3] = LinearDepth[px * 4 + uint2(3, 3)].r;
	sceneCoc = saturate(dofEqWorld.x * depth + dofEqWorld.y);
	curCoc = sceneCoc;
	coc = max(coc, curCoc);

	float maxCoc = max(max(coc[0], coc[1]), max(coc[2], coc[3]));

	DownsampledCoC[px] = float4(color, maxCoc);
}

/*
Shader Resources:
	Texture Color (as SRV)
	Texture LinearDepth (as SRV)
	Texture DownsampledCoC (as UAV)
*/
