// Texture2DRW_PS technique, shader RWPS


RWTexture2D<float4> nodeTexture : register(u0);
RWTexture2D<float4> importedTexture : register(u1);
Texture2D<float4> importedColor : register(t0);
Texture2D<float4> _loadedTexture_0 : register(t1);

#line 2


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
	float3 loadedTexturePx = _loadedTexture_0[input.position.xy].rgb;
	float3 importedTexturePx = importedTexture[input.position.xy].rgb;
	float3 importedColorPx = importedColor[input.position.xy].rgb;

	nodeTexture[input.position.xy] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[input.position.xy] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);

	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(1.0f, 0.5f, 0.25f, 1.0f);
	return ret;
}
