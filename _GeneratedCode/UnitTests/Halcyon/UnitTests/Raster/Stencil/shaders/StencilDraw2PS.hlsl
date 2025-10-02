// Stencil technique, shader Draw2PS


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
