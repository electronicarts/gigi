// Unnamed technique, shader BakeSpecularIBL
/*$(ShaderResources)*/

#include "PCG.hlsli"
#include "CubeMap.hlsli"
#include "PBR.hlsli"

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint rng = HashInit(DTid);

    uint3 outputDims;
    Output.GetDimensions(outputDims.x, outputDims.y, outputDims.z);

    float2 uv = (float2(DTid.xy) + float2(0.5f, 0.5f)) / float2(outputDims.xy);
    uv = uv * 2.0f - 1.0f;
    uv.y = -uv.y;

    float3 dir = CubeUVFaceToDir(uv, DTid.z);

    float2 offset = float2(RandomFloat01(rng), RandomFloat01(rng));

    float roughness = float(/*$(Variable:LevelToBake)*/) / float(/*$(Variable:SpecularBakeLevelCountMinus1)*/);
    roughness = saturate(roughness);
    float alpha = max(roughness * roughness, 1e-4f);

    uint inputFaceWidth;
    uint inputFaceHeight;
    uint inputMipCount;
    Input.GetDimensions(0, inputFaceWidth, inputFaceHeight, inputMipCount);

    float texelSolidAngle = 4.0f * c_pi / (6.0f * float(inputFaceWidth) * float(inputFaceHeight));

    // For prefiltered env maps, we bake for the reflection direction itself.
    float3 N = dir;
    float3 V = N;

    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;
    for (uint index = 0; index < /*$(Variable:NumSamples)*/; ++index)
    {
        float2 halton = frac(Halton(index) + offset);

        float3 H = SampleGGXHalfVector(halton, alpha, N);
        float3 L = reflect(-V, H);

        float NoL = saturate(dot(N, L));
        if (NoL <= 0.0f)
            continue;

        float NoH = saturate(dot(N, H));
        float VoH = saturate(dot(V, H));

        // GGX half-vector importance-sampling PDF.
        float D = D_GGX(NoH, alpha);
        float PDF = max((D * NoH) / max(4.0f * VoH, 1e-6f), 1e-6f);

        #if 0
        float sampleSolidAngle = 1.0f / (float(/*$(Variable:NumSamples)*/) * PDF);
        float sourceMip = (roughness <= 0.0f) ? 0.0f : 0.5f * log2(sampleSolidAngle / texelSolidAngle);
        sourceMip = clamp(sourceMip, 0.0f, max(float(inputMipCount) - 1.0f, 0.0f));
        #else
        float sourceMip = 0.0f;
        #endif

        float3 lightIn = Input.SampleLevel(LinearWrapSampler, L, sourceMip).rgb;
        float3 sample = lightIn * NoL;
        irradiance += sample;
        totalWeight += NoL;
    }

    // Avoid black texels when all Monte Carlo samples are rejected for this output texel.
    if (totalWeight > 1e-6f)
        irradiance /= totalWeight;
    else
        irradiance = Input.SampleLevel(LinearWrapSampler, dir, 0.0f).rgb;

    Output[DTid] = float4(irradiance, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
Shader Samplers:
	LinearWrapSampler filter: MinMagMipLinear addressmode: Wrap
*/
