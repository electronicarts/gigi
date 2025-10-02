// Texture3DRW technique, shader RWCS


RWTexture3D<float4> nodeTexture : register(u0);
RWTexture3D<float4> importedTexture : register(u1);
Texture3D<float4> importedColor : register(t0);
Texture3D<float4> _loadedTexture_0 : register(t1);


[numthreads(4, 4, 4)]
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	float3 loadedTexturePx = _loadedTexture_0[DTid.xyz].rgb;
	float3 importedTexturePx = importedTexture[DTid.xyz].rgb;
	float3 importedColorPx = importedColor[DTid.xyz].rgb;

	nodeTexture[DTid.xyz] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[DTid.xyz] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}
