// Unnamed technique, shader ManualUpscale
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;

    uint2 destDims;
    dest.GetDimensions(destDims.x, destDims.y);

    float2 uv = (float2(px) + 0.5f) / float2(destDims);

    float ddx = 1.0f / float(destDims.x);
    float ddy = 1.0f / float(destDims.y);

	float4 color = source.SampleGrad(linearWrapSampler, uv, float2(ddx, 0.0f), float2(0.0f, ddy));

	dest[px] = color;
}

/*
Shader Resources:
	Texture source (as SRV)
	Texture dest (as UAV)
Shader Samplers:
	linearWrapSampler filter: MinMagMipLinear addressmode: Wrap
*/
