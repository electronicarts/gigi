// Texture2DRW technique, shader RWCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	float3 loadedTexturePx = /*$(Image2D:../cabinsmall.png:RGBA8_Unorm:float4:false)*/[DTid.xy].rgb;
	float3 importedTexturePx = importedTexture[DTid.xy].rgb;
	float3 importedColorPx = importedColor[DTid.xy].rgb;

	nodeTexture[DTid.xy] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[DTid.xy] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}
