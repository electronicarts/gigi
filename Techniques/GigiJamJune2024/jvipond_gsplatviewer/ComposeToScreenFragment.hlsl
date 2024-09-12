// Unnamed technique, shader ComposeToScreenFragment
/*$(ShaderResources)*/

inline half3 GammaToLinearSpace (half3 sRGB)
{
    // Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return sRGB * (sRGB * (sRGB * 0.305306011h + 0.682171111h) + 0.012522878h);

    // Precise version, useful for debugging.
    //return half3(GammaToLinearSpaceExact(sRGB.r), GammaToLinearSpaceExact(sRGB.g), GammaToLinearSpaceExact(sRGB.b));
}

struct v2f
{
    float4 vertex : SV_POSITION;
};

half4 frag (v2f i) : SV_Target
{
    half4 col = ColorTarget.Load(int3(i.vertex.xy, 0));
    col.rgb = GammaToLinearSpace(col.rgb);
    col.a = saturate(col.a * 1.5);
    return col;
}

/*
Shader Resources:
	Texture ColorTarget (as SRV)
*/
