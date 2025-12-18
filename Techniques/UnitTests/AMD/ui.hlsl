// Unnamed technique, shader RenderUI
/*$(ShaderResources)*/

static const float c_UIFade = 0.003f;

float SDFDisk(float2 p, float2 center, float radius)
{
	return length(p - center) - radius;
}

float SDFBox(float2 p, float2 center, float2 halfSize)
{
	float2 d = abs(p - center) - halfSize;
	return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

float SDFCircle(float2 p, float2 center, float radius)
{
	return abs(length(p - center) - radius);
}

float3 CircleColor(float2 p, float2 center, float radius, float3 color, float3 highlightColor)
{
    static const float c_highlightScale = 1.0f;
    float2 relP = (p - center) / radius;
    float highlight = clamp(length(relP - float2(0.4f, -0.5f)), 0.0f, c_highlightScale) / c_highlightScale;
    return lerp(color, highlightColor, 1.0f - highlight);
}

float SDFLineSegment(float2 p, float2 a, float2 b)
{
	float2 pa = p - a;
	float2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
    return length(pa - ba * h);
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    int2 dims;
    Color.GetDimensions(dims.x, dims.y);

    int2 px = DTid.xy;
    float2 uv = (float2(px - dims / 2) + 0.5f) / float2(dims);

    uv.x *= float(dims.x) / float(dims.y);

    float3 outColor = Color[px].rgb;

    float time = float(/*$(Variable:iFrame)*/) / 60.0f;

    // Life ball
    {
        // Ball
        float dist = SDFDisk(uv, float2(-0.55f, 0.375f), 0.128f);
        float alpha = smoothstep(0.0f, -c_UIFade, dist);

        // Fullness
        float fullness = 0.6f + sin(uv.x * 10.0f + time * 4.0f) * 0.02f;
        float fillLevel = 0.375f - 0.128f * (fullness * 2.0f - 1.0f);
        if (uv.y < fillLevel)
            alpha *= smoothstep(fillLevel - c_UIFade, fillLevel, uv.y);

        // Blend color
        float3 color = CircleColor(uv, float2(-0.55f, 0.375f), 0.128f, float3(0.75f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f) * 0.8f);
        outColor = lerp(outColor, color, alpha);

        // Outline
        dist = SDFCircle(uv, float2(-0.55f, 0.375f), 0.125f);
        alpha = smoothstep(c_UIFade, 0.0f, dist);
        color = float3(1.0f, 1.0f, 1.0f) * 0.15f;
        outColor = lerp(outColor, color, alpha);
    }

    // Mana ball
    {
		// Ball
        float dist = SDFDisk(uv, float2(0.55f, 0.375f), 0.128f);
        float alpha = smoothstep(0.0f, -c_UIFade, dist);

        // Fullness
        float fullness = 0.8f + sin(uv.x * 10.0f + time * 4.0f) * 0.02f;
        float fillLevel = 0.375f - 0.128f * (fullness * 2.0f - 1.0f);
        if (uv.y < fillLevel)
            alpha *= smoothstep(fillLevel - c_UIFade, fillLevel, uv.y);

        // Blend color
        float3 color = CircleColor(uv, float2(0.55f, 0.375f), 0.128f, float3(0.0f, 0.0f, 0.75f), float3(1.0f, 1.0f, 1.0f) * 0.8f);
        outColor = lerp(outColor, color, alpha);

        // Outline
        dist = SDFCircle(uv, float2(0.55f, 0.375f), 0.125f);
        alpha = smoothstep(c_UIFade, 0.0f, dist);
        color = float3(1.0f, 1.0f, 1.0f) * 0.15f;
        outColor = lerp(outColor, color, alpha);
    }

    // Skill bar
    {
        if (uv.x > -0.4f && uv.x < 0.4f && uv.y > 0.275f && uv.y < 0.475f)
        {
            float2 repUV = uv;
            repUV.x = (repUV.x + 0.4f) % 0.1f - 0.05f;
            repUV.y = (repUV.y - 0.275f) % 0.1f - 0.05f;

            float dist = SDFBox(repUV, float2(0.0f, 0.0f), float2(0.05f, 0.05f));
            float alpha = smoothstep(0.0f, -c_UIFade, dist);
            float3 color = float3(0.5f, 0.5f, 0.5f);
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(-0.047f, 0.047f), float2(0.047f, 0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 0.0f;
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(-0.047f, 0.047f), float2(-0.047f, -0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 0.0f;
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(-0.047f, -0.047f), float2(0.047f, -0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 1.0f;
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(0.047f, 0.047f), float2(0.047f, -0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 1.0f;
            outColor = lerp(outColor, color, alpha);
        }
    }

    Color[px] = float4(outColor, 1.0);
}

/*
Shader Resources:
	Texture Color (as UAV)
*/
