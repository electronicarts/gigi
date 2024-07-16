// Stencil technique, shader Draw2PS
/*$(ShaderResources)*/

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
	float3 color = /*$(Image2D:cabinsmall.png:RGBA8_Unorm:float4:false)*/[input.position.xy].rgb;
	ret.colorTarget = float4(color, 1.0f);
	return ret;
}
