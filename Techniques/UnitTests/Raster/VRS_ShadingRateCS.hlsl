// VRS technique, shader ShadingRateCS
/*$(ShaderResources)*/

// from d3d12.h
static const uint D3D12_SHADING_RATE_1X1 = 0;
static const uint D3D12_SHADING_RATE_1X2 = 0x1;
static const uint D3D12_SHADING_RATE_2X1 = 0x4;
static const uint D3D12_SHADING_RATE_2X2 = 0x5;
static const uint D3D12_SHADING_RATE_2X4 = 0x6;
static const uint D3D12_SHADING_RATE_4X2 = 0x9;
static const uint D3D12_SHADING_RATE_4X4 = 0xa;

/*$(_compute:ShadingRateCSMain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	ShadingRate.GetDimensions(dims.x, dims.y);
	float2 uv = (float2(DTid.xy) / float2(dims)) - 0.5f;
	float distuv = length(uv);

	uint shadingRate;
	if (distuv < 0.2f)
		shadingRate = D3D12_SHADING_RATE_1X1;
	else if (distuv < 0.4f)
		shadingRate = D3D12_SHADING_RATE_2X2;
	else
		shadingRate = D3D12_SHADING_RATE_4X4;
	
	ShadingRate[DTid.xy] = shadingRate;
}

/*
Shader Resources:
	Texture ShadingRate (as UAV)
*/
