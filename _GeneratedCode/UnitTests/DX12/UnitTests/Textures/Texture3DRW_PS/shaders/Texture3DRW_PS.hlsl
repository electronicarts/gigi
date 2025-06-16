// Texture3DRW_CS technique, shader RWPS


RWTexture3D<float4> nodeTexture : register(u0);
RWTexture3D<float4> importedTexture : register(u1);
Texture3D<float4> importedColor : register(t0);
Texture3D<float4> _loadedTexture_0 : register(t1);

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
    float3 loadedTexturePx = _loadedTexture_0[uint3(input.position.xy, 0)].rgb;
    float3 importedTexturePx = importedTexture[uint3(input.position.xy, 1)].rgb;
    float3 importedColorPx = importedColor[uint3(input.position.xy, 2)].rgb;

	nodeTexture[uint3(input.position.xy, 0)] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[uint3(input.position.xy, 2)] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);

	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(1.0f, 0.5f, 0.25f, 1.0f);
	return ret;
}
