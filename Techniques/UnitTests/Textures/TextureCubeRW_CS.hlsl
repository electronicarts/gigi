// Texture2DArrayRW technique, shader RWCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	static const float c_pi = 3.14159265359f;

	float2 uv = float2(px) / 64.0f;
	uv *= float2(c_pi, 2.0f * c_pi);

	float3 dir1 = float3(
		sin(uv.x)*cos(uv.y),
		sin(uv.x)*sin(uv.y),
		cos(uv.x)
	);

	uv.x += 0.5f;
	uv.y += 0.3f;

	float3 dir2 = float3(
		sin(uv.x)*cos(uv.y),
		sin(uv.x)*sin(uv.y),
		cos(uv.x)
	);	

	float3 loadedTexturePx = /*$(ImageCube:cube%s.png:RGBA8_Unorm:float4:false)*/.SampleLevel(LinearWrap, dir1, 0).rgb;
	float3 importedTexturePx = importedTexture[uint3(px, 1)].rgb;
	float3 importedColorPx = importedColor.SampleLevel(LinearWrap, dir2, 0).rgb;

	nodeTexture[uint3(px, 0)] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[uint3(px, 2)] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}
