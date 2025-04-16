// Unnamed technique, shader Blur
/*$(ShaderResources)*/

#include "sRGB.hlsli"

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	int2 px = int2(DTid.xy);

	int2 dims;
	Input.GetDimensions(dims.x, dims.y);

	int radius = /*$(Variable:LoopIndex)*/ + 1;
	float3 ret = float3(0.0f, 0.0f, 0.0f);
	for (int iy = -1; iy <= 1; ++iy)
	{
		for (int ix = -1; ix <= 1; ++ix)
		{
			int2 readpx = (px + int2(ix, iy) * radius + dims) % dims;
			ret += Input[readpx].rgb;
		}
	}
	ret /= 9.0f;

	if (/*$(Variable:sRGB)*/)
		ret = LinearToSRGB(ret);

	Output[px] = float4(ret, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
