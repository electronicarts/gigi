// Texture2DRW technique, shader RWCS


RWTexture2D<float4> nodeTexture : register(u0);
RWTexture2D<float4> importedTexture : register(u1);
Texture2D<float4> importedColor : register(t0);
Texture2D<float4> _loadedTexture_0 : register(t1);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	float3 loadedTexturePx = _loadedTexture_0[DTid.xy].rgb;
	float3 importedTexturePx = importedTexture[DTid.xy].rgb;
	float3 importedColorPx = importedColor[DTid.xy].rgb;

	nodeTexture[DTid.xy] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[DTid.xy] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}
