// Unnamed technique, shader ModelViewerPathTraceCS
/*$(ShaderResources)*/

#include "sRGB.hlsli"
#include "PCG.hlsli"
#include "tonemap.hlsl"
#include "PBR.hlsli"

#define USE_DIFFUSE_COSINE_WEIGHTED_HEMISPHERE_SAMPLING 1

/*$(Embed:_material.hlsli)*/
/*$(Embed:materialsOverride.hlsli)*/

struct RayHit
{
    float dist;
    Struct_Materials material;
    float3 normal;
};

float3 BarycentricInterpolate(float3 barycentrics, float3 v0, float3 v1, float3 v2)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

float2 BarycentricInterpolate(float3 barycentrics, float2 v0, float2 v1, float2 v2)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

bool IsShadowed(float3 origin, float3 dir, float maxT)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(dir);
    ray.TMin = 0.0f;
    ray.TMax = maxT;

    RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
             RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
             RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;

    q.TraceRayInline(Scene, 0, 255, ray);
    q.Proceed();

    return q.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
}

float3 EstimateDirectLighting(float3 P, float3 N, float3 V, Struct_Materials mat, inout uint rng)
{
    float3 direct = 0.0f;

    uint lightCount, lightStride;
    Lights.GetDimensions(lightCount, lightStride);

    // boost the contribution of the lights to account for them not getting selected every frame, if we are doing a subset of the lights every frame.
    uint lightSelectionCount = /*$(Variable:DirectLightCountPT)*/;
    if (lightSelectionCount == 0 || lightSelectionCount > lightCount)
        lightSelectionCount = lightCount;
    float lightBoost = float(lightCount) / float(lightSelectionCount);

    for (uint lightSelectionIndex = 0; lightSelectionIndex < lightSelectionCount; ++lightSelectionIndex)
    {
        // Selection with replacement because it is cheap and easy to implement.
        // We may choose the same light multiple times, but it'll converge... eventually.
        uint i = HashPCG(rng) % lightCount;

        float3 L = 0.0f;
        float attenuation = 1.0f;
        float maxT = 1000.0f;

        // Point light
        if (Lights[i].PosDir.w == 1.0f)
        {
            float3 toLight = Lights[i].PosDir.xyz - P;
            float distSq = max(dot(toLight, toLight), 1e-6f);
            float dist = sqrt(distSq);
            L = toLight / dist;
            maxT = dist - 1e-3f;

            if (Lights[i].Range > 0.0f)
            {
                if (dist >= Lights[i].Range) continue;
                float rangeFade = saturate(1.0f - dist / Lights[i].Range);
                attenuation *= rangeFade * rangeFade;
            }

            attenuation *= 1.0f / distSq;
        }
        // Directional light
        else
        {
            L = normalize(Lights[i].PosDir.xyz);
            maxT = 1000.0f;
        }

        float NoL = saturate(dot(N, L));
        if (NoL <= 0.0f) continue;

        if (IsShadowed(P + N * /*$(Variable:RayPosNormalNudgePT)*/, L, maxT))
            continue;

        float3 Li = Lights[i].ColorIntensity.xyz * Lights[i].ColorIntensity.w * attenuation * lightBoost;

        float3 f = MicrofacetBRDF(L, V, N, mat.metallic, mat.roughness, mat.baseColor.rgb, 0.5f);

        direct += Li * f * NoL;
    }

    return direct;
}

RayHit RayVsScene(float3 rayPos, float3 rayDir)
{
    RayHit ret = (RayHit)0;
    ret.dist = -1.0f;

    // Describe the ray to shoot
    RayDesc ray;
    ray.Origin = rayPos;
    ray.TMin = 0;
    ray.TMax = 1000.0f;
    ray.Direction = normalize(rayDir);

    // Do a ray query to see if the ray hit anything
    RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
             RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES > rayQuery;

    rayQuery.TraceRayInline(
        Scene,
        0,
        255,
        ray
    );

    rayQuery.Proceed();

    if (rayQuery.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        return ret;

    ret.dist = rayQuery.CommittedRayT();
    
    // Get the barycentric coordinates of where the ray intersected the triangle
    float3 barycentrics;
    barycentrics.yz = rayQuery.CommittedTriangleBarycentrics();
    barycentrics.x = 1.0f - (barycentrics.y + barycentrics.z);

    // Get the normal
    ret.normal = BarycentricInterpolate(barycentrics,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 0].Normal,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 1].Normal,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 2].Normal);
    ret.normal = normalize(ret.normal);

    // Get UVs
    float2 uv0 = BarycentricInterpolate(barycentrics,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 0].UV0,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 1].UV0,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 2].UV0);
    float2 uv1 = BarycentricInterpolate(barycentrics,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 0].UV1,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 1].UV1,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 2].UV1);
    float2 uv2 = BarycentricInterpolate(barycentrics,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 0].UV2,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 1].UV2,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 2].UV2);
    float2 uv3 = BarycentricInterpolate(barycentrics,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 0].UV3,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 1].UV3,
                    VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 2].UV3);

    float3 matNormal = float3(0.0f, 0.0f, 0.0f);
    float matOcclusion = 0.0f;

    int materialID = VertexBuffer[rayQuery.CommittedPrimitiveIndex() * 3 + 0].MaterialID;

    ret.material = Material_MaterialBuffer_Level(Materials, materialID, uv0, uv1, uv2, uv3, matNormal, matOcclusion, /*$(Variable:MipLevelPT)*/);
    ApplyMaterialOverride(ret.material);

    return ret;
}

float3 GetColorForRay(float3 initialRayPos, float3 initialRayDir, inout uint rng, float2 cpRotation)
{
    float3 ret = float3(0.0f, 0.0f, 0.0f);
    float3 throughput = float3(1.0f, 1.0f, 1.0f);
    float3 rayPos = initialRayPos;
    float3 rayDir = initialRayDir;

    for (int bounceIndex = 0; bounceIndex <= /*$(Variable:NumBouncesPT)*/; ++bounceIndex)
    {
        RayHit hitInfo = RayVsScene(rayPos, rayDir);

        // Make all surfaces double sided
        if (dot(rayDir, hitInfo.normal) > 0.0f)
            hitInfo.normal = -hitInfo.normal;

        // If ray missed, apply the sky as emissive and break out of the loop
        if (hitInfo.dist < 0.0f)
        {
            float3 emissive = SkyboxCubeMap.SampleLevel(LinearWrapSampler, rayDir, /*$(Variable:MipLevelPT)*/).rgb;
            ret += emissive * throughput;
            break;
        }

        // Alpha test
        if (hitInfo.material.baseColor.a <= 0.1f)
        {
            rayPos = (rayPos + rayDir * hitInfo.dist) + rayDir * /*$(Variable:RayPosNormalNudgePT)*/;
            continue;
        }

        // Apply emissive
        ret += hitInfo.material.emissive * throughput;

        // Apply direct lighting
        if (/*$(Variable:DirectLightingMultiplier)*/ > 0.0f)
            ret += EstimateDirectLighting(rayPos + rayDir * hitInfo.dist, hitInfo.normal, -rayDir, hitInfo.material, rng) * throughput * /*$(Variable:DirectLightingMultiplier)*/;

        // update the ray position
        rayPos = (rayPos + rayDir * hitInfo.dist) + hitInfo.normal * /*$(Variable:RayPosNormalNudgePT)*/;

        // Calculate the chances to choose between diffuse and specular reflection
        float pSpec = 0.0f;
        float pDiff = 1.0f;
        float3 kDiffuse = hitInfo.material.baseColor.rgb;
        GetDiffuseVsSpecular(rayDir, hitInfo.material.baseColor.rgb, hitInfo.material.metallic, hitInfo.normal, pSpec, pDiff, kDiffuse);

        float3 BRDF = float3(1.0f, 1.0f, 1.0f);
        float PDF = 1.0f;
        float cosTheta = 0.0f;

        // Specular reflection
        if (RandomFloat01(rng) < pSpec)
        {
            float3 N = hitInfo.normal;
            float3 V = normalize(-rayDir);
            float NoV = saturate(dot(N, V));

            float roughness = clamp(hitInfo.material.roughness, 0.04f, 1.0f);
            float alpha = roughness * roughness;

            float2 halton = frac(Halton(bounceIndex) + cpRotation);
            float3 H = SampleGGXHalfVector(halton, alpha, N);

            float VoH = saturate(dot(V, H));
            rayDir = reflect(-V, H); // L
            float NoL = saturate(dot(N, rayDir));
            float NoH = saturate(dot(N, H));

            float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), hitInfo.material.baseColor.rgb, hitInfo.material.metallic);
            float3 F = fresnelSchlick(VoH, F0);
            float D = D_GGX(NoH, alpha);
            float G = G_Smith(NoV, NoL, alpha);

            BRDF = (F * D * G) / max(4.0f * NoV * NoL, 1e-6f);

            float pdfSpec = max((D * NoH) / max(4.0f * VoH, 1e-6f), 1e-6f);
            PDF = pdfSpec * pSpec;
            cosTheta = NoL;
        }
        // Diffuse reflection
        else
        {
            BRDF = kDiffuse / c_pi;
            float2 halton = frac(Halton(bounceIndex) + cpRotation);
            float3 unitVec = RandomUnitVector(halton);
            #if USE_DIFFUSE_COSINE_WEIGHTED_HEMISPHERE_SAMPLING == 1
                float3 hemiVec = hitInfo.normal + unitVec;
                float hemiVecLenSq = dot(hemiVec, hemiVec);
                rayDir = (hemiVecLenSq > 1e-8f) ? (hemiVec * rsqrt(hemiVecLenSq)) : hitInfo.normal;
                cosTheta = saturate(dot(rayDir, hitInfo.normal));
                PDF = max(cosTheta / c_pi, 1e-6f);
            #else
                rayDir = (dot(unitVec, hitInfo.normal) < 0) ? -unitVec : unitVec;
                cosTheta = saturate(dot(rayDir, hitInfo.normal));
                PDF = 1.0f / (2.0f * c_pi);
            #endif

            PDF *= pDiff;
        }

        // update throughput
        throughput *= BRDF * cosTheta / PDF;

        // Russian Roulette
        // As the throughput gets smaller, the ray is more likely to get terminated early.
        // Survivors have their value boosted to make up for fewer samples being in the average.
        if (bounceIndex >= /*$(Variable:RRMinBouncesRT)*/)
        {
            float p = clamp(max(throughput.r, max(throughput.g, throughput.b)), 0.05f, 0.95f);
            if (RandomFloat01(rng) > p)
                break;

            // Add the energy we 'lose' by randomly terminating paths
            throughput *= 1.0f / p;
        }
    }

    return ret;
}

float2 GetRayCPRotation(uint3 pxRay, inout uint rng)
{
    // Blue noise is limited in number of unique samples by the number of pixels in the image, which can make visible patterns.
    // Also, blue noise only matters at low sample counts.
    // So, at higher sample counts, just use white noise.
    if (/*$(Variable:FrameIndex)*/ > 50)
        return float2(RandomFloat01(rng), RandomFloat01(rng));

    uint3 dims;
    /*$(Image2DArray:Assets/Noise/vector2_uniform_gauss1_0_exp0101_separate05_%i.png:RG8_UNorm:float2:false:false)*/.GetDimensions(dims.x, dims.y, dims.z);

    // Every time we go through the full blue noise texture, do a different 2d shift (white noise)
    uint rng2 = HashInit(uint3(pxRay.xy, pxRay.z / dims.z));
    pxRay.x += HashPCG(rng2);
    pxRay.y += HashPCG(rng2);

    // tile the blue noise texture across the screen over space and time
    uint3 readLocation = pxRay % dims;
    return /*$(Image2DArray:Assets/Noise/vector2_uniform_gauss1_0_exp0101_separate05_%i.png:RG8_UNorm:float2:false:false)*/[readLocation];
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    // Get the dimensions of the render target, so we know what percentage of the screen in X and Y this pixel is at.
    uint2 px = DTid.xy;
    uint w, h;
    Output.GetDimensions(w, h);
    float2 dimensions = float2(w, h);

    uint rng = HashInit(uint3(DTid.xy, /*$(Variable:FrameIndex)*/));

    // Calculate this pixel's location in screen space
    float2 screenPos = (float2(px) + 0.5f) / dimensions * 2.0 - 1.0;
    screenPos.y = -screenPos.y;

    // Calculate this pixel in world space at a specific depth away from the camera.
    // This gives us the target to shoot the ray at.
    // Note: this jittered view is the source of non determinism in this path tracer. If we jittered in this shader instead, it would be deterministic
    float4 world = mul(float4(screenPos, 0.99f, 1), /*$(Variable:InvJitteredViewProjMtx)*/);
    world.xyz /= world.w;

    // Shoot N rays and average them
    float3 color = float3(0.0f, 0.0f, 0.0f);
    float3 rayPos = /*$(Variable:CameraPos)*/;
    float3 rayDir = normalize(world.xyz - rayPos);
    for (int localRayIndex = 0; localRayIndex < /*$(Variable:RaysPerFramePT)*/; ++localRayIndex)
    {
        uint globalRayIndex = /*$(Variable:FrameIndex)*/ * /*$(Variable:RaysPerFramePT)*/ + localRayIndex;

        // Cranley Patterson rotation using either blue noise or white noise
        float2 cpRotation;
        if (true)
        {
            uint3 readLocation;
            readLocation.xy = px;
            readLocation.z = globalRayIndex;
            cpRotation = GetRayCPRotation(readLocation, rng);
        }
        else
        {
            cpRotation = float2(RandomFloat01(rng), RandomFloat01(rng));
        }

        float3 rayColor = GetColorForRay(rayPos, rayDir, rng, cpRotation);
        color = lerp(color, rayColor, 1.0f / (localRayIndex + 1));
    }

    // Accumulate the pixel colors over frames
    float4 old = AccumPT[px].rgba;
    float3 prevColor = old.rgb;
    float sampleCount = old.a;

    if (sampleCount > 0.0f)
        color = lerp(prevColor, color, 1.0f / (sampleCount + 1.0f));

    AccumPT[px] = float4(color, sampleCount + 1.0f);

    // apply exposure
    color *= pow(2.0f, /*$(Variable:ExposureFStops)*/);

    // Do tonemapping
	switch(/*$(Variable:ToneMapper)*/)
	{
		// Do nothing, only apply exposure
		case ToneMappingOperation::None: break;

		// https://64.github.io/tonemapping/
		case ToneMappingOperation::Reinhard:
		{
			color = color / (1.0f + color);
			break;
		}
		case ToneMappingOperation::ACES_Luminance:
		{
			// The * 0.6f is to undo the exposure baked in, per the author's instructions
			color = ACESFilm(color * 0.6f);
			break;
		}
		case ToneMappingOperation::ACES:
		{
			color = ACESFitted(color);
			break;
		}
	}

    // output results
    Output[px] = float4(LinearToSRGB(color), 1.0f);
}

/*
Shader Resources:
	Buffer Scene (as RTScene)
	Texture AccumPT (as UAV)
    Texture SkyboxCubeMap (as SRV)
	Buffer Lights (as SRV)
	Buffer Materials (as SRV)
	Buffer VertexBuffer (as SRV)
	Texture Output (as UAV)
Shader Samplers:
	LinearWrapSampler filter: MinMagMipLinear addressmode: Wrap
*/

