// Unnamed technique, shader Show
/*$(ShaderResources)*/

#include "SRGB.hlsli"

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

    uint2 dims1, dims2;
	Image1.GetDimensions(dims1.x, dims1.y);
	Image2.GetDimensions(dims2.x, dims2.y);

    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (px.x < dims1.x)
	{
		if (px.y < dims1.y)
            color = Image1[px];
	}
    else
    {
		if (px.y < dims2.y)
			color = Image2[px - uint2(dims1.x, 0)];
	}

    if (/*$(Variable:LinearToSRGB)*/)
		color.rgb = LinearToSRGB(color.rgb);

	Output[px] = color;
}

/*
Shader Resources:
	Texture Image1 (as SRV)
	Texture Image2 (as SRV)
	Texture Output (as UAV)
*/
