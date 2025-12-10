// Unnamed technique, shader Blur


struct Struct__FilterSub_Iteration_4_Blur_0CB
{
    uint FilterSub_Iteration_4_sRGB;
    int __loopIndexValue_4;
    float2 _padding0;
};

Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);
ConstantBuffer<Struct__FilterSub_Iteration_4_Blur_0CB> _FilterSub_Iteration_4_Blur_0CB : register(b0);

#line 2


#include "sRGB.hlsli"

[numthreads(8, 8, 1)]
#line 6
void main(uint3 DTid : SV_DispatchThreadID)
{
	int2 px = int2(DTid.xy);

	int2 dims;
	Input.GetDimensions(dims.x, dims.y);

	int radius = _FilterSub_Iteration_4_Blur_0CB.__loopIndexValue_4 + 1;
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

	if ((bool)_FilterSub_Iteration_4_Blur_0CB.FilterSub_Iteration_4_sRGB)
		ret = LinearToSRGB(ret);

	Output[px] = float4(ret, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
