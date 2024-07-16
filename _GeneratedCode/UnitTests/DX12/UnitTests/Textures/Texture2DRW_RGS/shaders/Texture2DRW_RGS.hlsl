// Texture2DRW_RGS technique, shader RWRGS


RWTexture2D<float4> nodeTexture : register(u0);
RWTexture2D<float4> importedTexture : register(u1);
Texture2D<float4> importedColor : register(t0);
Texture2D<float4> _loadedTexture_0 : register(t1);


struct Payload
{
	bool hit;
};

[shader("raygeneration")]
void rgsmain()
{
	uint2 px = DispatchRaysIndex().xy;

	float3 loadedTexturePx = _loadedTexture_0[px].rgb;
	float3 importedTexturePx = importedTexture[px].rgb;
	float3 importedColorPx = importedColor[px].rgb;

	nodeTexture[px] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[px] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}
