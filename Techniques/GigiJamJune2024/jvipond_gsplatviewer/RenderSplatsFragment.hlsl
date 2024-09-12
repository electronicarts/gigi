// Unnamed technique, shader RenderSplatVertex
/*$(ShaderResources)*/

struct v2f // AKA PSInput
{
    half4 col : COLOR0;
    float2 pos : TEXCOORD0;
    float4 vertex : SV_POSITION;
};

half4 frag(v2f i) : SV_Target
{
    float power = -dot(i.pos, i.pos);
    half alpha = exp(power);
    if (i.col.a >= 0)
    {
        alpha = saturate(alpha * i.col.a);
    }
    else
    {
		// "selected" splat: magenta outline, increase opacity, magenta tint
        half3 selectedColor = half3(1, 0, 1);
        if (alpha > 7.0 / 255.0)
        {
            if (alpha < 10.0 / 255.0)
            {
                alpha = 1;
                i.col.rgb = selectedColor;
            }
            alpha = saturate(alpha + 0.3);
        }
        i.col.rgb = lerp(i.col.rgb, selectedColor, 0.5);
    }
	
    if (alpha < 1.0 / 255.0)
        discard;

    return half4(i.col.rgb * alpha, alpha);
}

/*
Shader Resources:
	Buffer _OrderBuffer (as SRV)
	Buffer _SplatViewData (as SRV)
*/
