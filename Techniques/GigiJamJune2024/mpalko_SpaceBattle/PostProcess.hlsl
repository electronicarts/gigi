/*$(ShaderResources)*/

float BlurWeight(float normalizedDistance, float numSamples)
{
    // return 1.0f / numSamples;
    float oneMinusDistance = saturate(1.0f - normalizedDistance);
    //return exp(-normalizedDistance * 12);
    float offset = 0.06;
    float falloff = 5;
    float invSquare = 1.0f / pow(normalizedDistance * falloff + offset, 2);
    float lerpDown = 1 - normalizedDistance;
    return invSquare / numSamples * falloff * lerpDown * 2;
}

/*$(_compute:csBlur)*/(uint3 DTid : SV_DispatchThreadID)
{
    int radius = /*$(Variable:BlurRadius)*/;
    uint w, h;
    Input.GetDimensions(w, h);

    float3 result = float3(0.0f, 0.0f, 0.0f);

    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {   
            float distance = length(float2(x,y)) / float(radius);

            result += Input.Load(int3((DTid.xy + int2(x, y)) % int2(w, h), 0)).rgb * BlurWeight(saturate(distance), pow(float(radius) * 2.0f + 1.0f, 2));
        }
    }

    Output[DTid.xy] = float4(result, 1);
}

// ============= ACES BEGIN
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

float3 SRGBToLinear(in float3 sRGBCol)
{
	float3 linearRGBLo = sRGBCol / 12.92;
	float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
	float3 linearRGB;
	linearRGB.r = sRGBCol.r <= 0.04045 ? linearRGBLo.r : linearRGBHi.r;
	linearRGB.g = sRGBCol.g <= 0.04045 ? linearRGBLo.g : linearRGBHi.g;
	linearRGB.b = sRGBCol.b <= 0.04045 ? linearRGBLo.b : linearRGBHi.b;
	return linearRGB;
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

// ============= ACES END

/*$(_compute:csPost)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;

    float3 colorIn = Input[px].rgb;

    float3 colorOut = ACESFitted(colorIn);

    colorOut = pow(colorOut, 1.0f/2.2f);

    Output[px] = float4(colorOut, 1);
}