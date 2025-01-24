// Texture2DRW_PS technique, shader RWPS
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput psmain(PSInput input)
{
	float3 loadedTexturePx = /*$(Image2D:../cabinsmall.png:RGBA8_Unorm:float4:false)*/[input.position.xy].rgb;
	float3 importedTexturePx = importedTexture[input.position.xy].rgb;
	float3 importedColorPx = importedColor[input.position.xy].rgb;

	nodeTexture[input.position.xy] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[input.position.xy] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);

	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(1.0f, 0.5f, 0.25f, 1.0f);
	return ret;
}
