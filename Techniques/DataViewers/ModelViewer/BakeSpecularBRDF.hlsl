// Unnamed technique, shader BakeSpecularBRDF
/*$(ShaderResources)*/

#include "PCG.hlsli"
#include "PBR.hlsli"

float2 IntegrateBRDF(float NoV, float roughness, float2 offset)
{
    float3 N = float3(0.0f, 0.0f, 1.0f);
    float3 V = float3(sqrt(saturate(1.0f - NoV * NoV)), 0.0f, NoV);

    float alpha = max(roughness * roughness, 1e-4f);

    float A = 0.0f;
    float B = 0.0f;

    for (uint index = 0; index < /*$(Variable:NumSpecularBRDFSamples)*/; ++index)
    {
        float2 Xi = frac(Halton(index) + offset);

        float3 H = SampleGGXHalfVector(Xi, alpha, N);
        float3 L = reflect(-V, H);

        float NoL = saturate(L.z);
        float NoH = saturate(H.z);
        float VoH = saturate(dot(V, H));

        if (NoL > 0.0f)
        {
            float G = G_Smith(NoV, NoL, alpha);
            float GVis = (G * VoH) / max(NoH * NoV, 1e-6f);
            float Fc = pow(1.0f - VoH, 5.0f);

            A += (1.0f - Fc) * GVis;
            B += Fc * GVis;
        }
    }

    float sampleCount = max(float(/*$(Variable:NumSpecularBRDFSamples)*/), 1.0f);
    return float2(A, B) / sampleCount;
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 outputDims;
    Output.GetDimensions(outputDims.x, outputDims.y);

    float2 uv = (float2(DTid.xy) + 0.5f) / float2(outputDims.xy);

    uint rng = HashInit(DTid);
    float2 offset = float2(RandomFloat01(rng), RandomFloat01(rng));

    float NoV = saturate(uv.x);
    float roughness = saturate(uv.y);

    float2 integrated = IntegrateBRDF(NoV, roughness, offset);

    Output[DTid.xy] = float4(integrated, 0.0f, 1.0f);
}

/*
Shader Resources:
    Texture Output (as UAV)
*/
