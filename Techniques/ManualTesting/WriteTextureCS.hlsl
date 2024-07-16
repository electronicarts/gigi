// TextureTypes technique, shader WriteTextureCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	TextureF.GetDimensions(dims.x, dims.y);
	float2 uv = float2(DTid.xy) / float2(dims);

	int4 outputColorU = uint4(uint2(uv*4294967295.0f), 168, 4294967295);
	TextureU[DTid.xy] = outputColorU;

	int4 outputColorI = int4(int2((uv-0.5f)*196.0f), 168, 196);
	TextureI[DTid.xy] = outputColorI;

	float4 outputColorF = float4(uv - 0.5f, 0.75f, 0.9f);
	//float4 outputColorF = float4(uv*0.5f + 0.25f, 0.75f, 0.9f);
	TextureF[DTid.xy] = outputColorF;
}

/*
Shader Resources:
	Texture Texture (as UAV)
*/
