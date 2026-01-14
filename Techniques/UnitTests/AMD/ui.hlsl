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

// Front-to-back compositing for the UI-only layer
void BlendUI(inout float3 layer, inout float a, float3 color, float alpha)
{
    float added = alpha * (1.0f - a);
    layer += color * added;
    a += added;
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    int2 dims;
    Color.GetDimensions(dims.x, dims.y);

    int2 px = DTid.xy;
    float2 uv = (float2(px - dims / 2) + 0.5f) / float2(dims);

    uv.x *= float(dims.x) / float(dims.y);

    // Preserve existing Color sampling
    float3 baseColor = Color[px].rgb;
    float3 outColor = baseColor;

    // UI-only accumulation
    float3 uiLayer = float3(0.0f, 0.0f, 0.0f);
    float uiAlpha = 0.0f;

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
        // accumulate UI-only layer
        BlendUI(uiLayer, uiAlpha, color, alpha);
        // keep original behavior on Color
        outColor = lerp(outColor, color, alpha);

        // Outline
        dist = SDFCircle(uv, float2(-0.55f, 0.375f), 0.125f);
        alpha = smoothstep(c_UIFade, 0.0f, dist);
        color = float3(1.0f, 1.0f, 1.0f) * 0.15f;
        BlendUI(uiLayer, uiAlpha, color, alpha);
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
        BlendUI(uiLayer, uiAlpha, color, alpha);
        outColor = lerp(outColor, color, alpha);

        // Outline
        dist = SDFCircle(uv, float2(0.55f, 0.375f), 0.125f);
        alpha = smoothstep(c_UIFade, 0.0f, dist);
        color = float3(1.0f, 1.0f, 1.0f) * 0.15f;
        BlendUI(uiLayer, uiAlpha, color, alpha);
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
            BlendUI(uiLayer, uiAlpha, color, alpha);
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(-0.047f, 0.047f), float2(0.047f, 0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 0.0f;
            BlendUI(uiLayer, uiAlpha, color, alpha);
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(-0.047f, 0.047f), float2(-0.047f, -0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 0.0f;
            BlendUI(uiLayer, uiAlpha, color, alpha);
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(-0.047f, -0.047f), float2(0.047f, -0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 1.0f;
            BlendUI(uiLayer, uiAlpha, color, alpha);
            outColor = lerp(outColor, color, alpha);

            dist = SDFLineSegment(repUV, float2(0.047f, 0.047f), float2(0.047f, -0.047f));
            alpha = smoothstep(c_UIFade, 0.0f, dist);
            color = float3(1.0f, 1.0f, 1.0f) * 1.0f;
            BlendUI(uiLayer, uiAlpha, color, alpha);
            outColor = lerp(outColor, color, alpha);
        }
    }

    // Write back the original color target (unchanged behavior)
    Color[px] = float4(outColor, 1.0);

    // Write UI-only texture: color + alpha of the UI elements (transparent background)
    UITexture[px] = float4(uiLayer, uiAlpha);
}

/*
Shader Resources:
	Texture Color (as UAV)
    Texture UITexture (as UAV)
*/
