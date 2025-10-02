// Texture2DArrayRW technique, shader RWCS


SamplerState LinearWrap : register(s0);
RWTexture2DArray<float4> nodeTexture : register(u0);
RWTexture2DArray<float4> importedTexture : register(u1);
TextureCube<float4> importedColor : register(t0);
TextureCube<float4> _loadedTexture_0 : register(t1);


[numthreads(8, 8, 1)]
void csmain(uint3 DTid : SV_DispatchThreadID)
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

    float3 loadedTexturePx = _loadedTexture_0.SampleLevel(LinearWrap, dir1, 0).rgb;
    float3 importedTexturePx = importedTexture[uint3(px, 1)].rgb;
    float3 importedColorPx = importedColor.SampleLevel(LinearWrap, dir2, 0).rgb;

	nodeTexture[uint3(px, 0)] = float4((loadedTexturePx * importedTexturePx) * importedColorPx, 1.0f);
	importedTexture[uint3(px, 2)] = float4((loadedTexturePx + importedTexturePx) / 2.0f * importedColorPx, 1.0f);
}
