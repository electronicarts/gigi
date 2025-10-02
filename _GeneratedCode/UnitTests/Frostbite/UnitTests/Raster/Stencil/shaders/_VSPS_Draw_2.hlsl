//VS Source: StencilDraw2VS.hlsl
//PS Source: StencilDraw2PS.hlsl

#if FB_VS// Stencil technique, shader Draw2VS




struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
};

// Full Screen Triangle
VSOutput Draw2VS(uint id : SV_VertexID)
{
	VSOutput ret = (VSOutput)0;
	float2 uv = float2((id << 1) & 2, id & 2);
	ret.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return ret;
}

#endif // FB_VS

#if FB_PS// Stencil technique, shader Draw2PS


Texture2D<float4> _loadedTexture_0 : register(t0);


struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput Draw2PS(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	float3 color = _loadedTexture_0[input.position.xy].rgb;
	ret.colorTarget = float4(color, 1.0f);
	return ret;
}

#endif // FB_PS