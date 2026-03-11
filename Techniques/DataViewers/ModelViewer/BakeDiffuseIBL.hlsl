// Unnamed technique, shader BakeDiffuseIBL
/*$(ShaderResources)*/

#include "PCG.hlsli"
#include "CubeMap.hlsli"

#define USE_DIFFUSE_COSINE_WEIGHTED_HEMISPHERE_SAMPLING 1

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

    // Lambertian diffuse BRDF value
    float BRDF = 1.0f / c_pi;

    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    for (uint index = 0; index < /*$(Variable:NumDiffuseIBLSamples)*/; ++index)
    {
        float2 halton = frac(Halton(index) + offset);
        float3 unitVec = RandomUnitVector(halton);

        #if USE_DIFFUSE_COSINE_WEIGHTED_HEMISPHERE_SAMPLING == 1
            float3 hemiVec = dir + unitVec;
            float hemiVecLenSq = dot(hemiVec, hemiVec);
            float3 sampleDir = (hemiVecLenSq > 1e-8f) ? (hemiVec * rsqrt(hemiVecLenSq)) : dir;
            float cosTheta = saturate(dot(sampleDir, dir));
            float PDF = max(cosTheta / c_pi, 1e-6f);
        #else
            float3 sampleDir = (dot(dir, unitVec) < 0) ? -unitVec : unitVec;
            float cosTheta = saturate(dot(sampleDir, dir));
            float PDF = 1.0f / (2.0f * c_pi);
        #endif

        float3 lightIn = Input.SampleLevel(LinearWrapSampler, sampleDir, 0).rgb;
        float3 sample = lightIn * BRDF * cosTheta / PDF;
        irradiance = lerp(irradiance, sample, 1.0f / (index + 1));
    }

    Output[DTid] = float4(irradiance, 1.0f);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
