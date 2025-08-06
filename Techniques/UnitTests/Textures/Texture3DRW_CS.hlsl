// Texture3DRW technique, shader RWCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	float3 loadedTexturePx = /*$(Image3D:ImageB%i.png:RGBA8_Unorm:float4:false)*/[DTid.xyz].rgb;
	float3 importedTexturePx = /*$(RWTextureR:importedTexture)*/[DTid.xyz].rgb;
	float3 importedColorPx = /*$(RWTextureR:importedColor)*/[DTid.xyz].rgb;

	nodeTexture[DTid.xyz] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[DTid.xyz] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}
