// Texture2DRW_RGS technique, shader RWRGS
/*$(ShaderResources)*/

struct Payload
{
	bool hit;
};

/*$(_raygeneration:rgsmain)*/
{
	uint2 px = DispatchRaysIndex().xy;

	float3 loadedTexturePx = /*$(Image2D:../cabinsmall.png:RGBA8_Unorm:float4:false)*/[px].rgb;
	float3 importedTexturePx = /*$(RWTextureR:importedTexture)*/[px].rgb;
	float3 importedColorPx = /*$(RWTextureR:importedColor)*/[px].rgb;

	nodeTexture[px] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[px] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}

/*$(_closesthit:chsmain)*/
{
    payload.hit = true;
}

/*$(_miss:missmain)*/
{
    payload.hit = false;
}
