#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI 0.31830988618379067153776752674503

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float NoH, float alpha)
{
    float alpha2 = alpha * alpha;
    float d = (NoH * NoH) * (alpha2 - 1.0) + 1.0; // d : temporary denominator
    return alpha2 * M_INV_PI / (d * d);
}

float G1_GGX_Schlick(float NoV, float alpha)
{
    float k = alpha / 2.0;
    return max(NoV, 0.001) / (NoV * (1.0 - k) + k);
}

float G_Smith(float NoV, float NoL, float alpha)
{
    return G1_GGX_Schlick(NoL, alpha) * G1_GGX_Schlick(NoV, alpha);
}

// Note: applies both diffuse and specular
float3 MicrofacetBRDF(float3 L, float3 V, float3 N, float metallic, float roughness, float3 baseColor, float specularlevel)
{
    float3 H = normalize(V + L); // half vector

    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);

    float3 f0 = 0.16 * (specularlevel * specularlevel); // Disney's specualr reflectance parameterization for dialectric materials
    f0 = lerp(f0, baseColor, metallic);

    float alpha = roughness * roughness; // alpha a = roughness^2 : From Disney parameterization to be perceptually linear

    float3 F = fresnelSchlick(VoH, f0); // fresnel term
    float D = D_GGX(NoH, alpha); // normal distribution function
    float G = G_Smith(NoV, NoL, alpha); // geometric shadowing function matching GGX NDF

    float3 specular = (F * D * G) / (4.0 * max(NoV, 0.001) * max(NoL, 0.001));

    float3 rhoD = baseColor;
    rhoD *= 1.0 - F; // Reduce diffuse based on energy lost to fresnel specular increase but no F0 adjustemnt
    rhoD *= (1.0 - metallic);

    float3 diffuse = rhoD * M_INV_PI;

    return diffuse + specular;
}

void GetDiffuseVsSpecular(in float3 rayDir, in float3 baseColor, in float metallic, in float3 normal, out float pSpec, out float pDiff, out float3 kDiffuse)
{
    pDiff = 1.0f;
    pSpec = 0.0f;
    kDiffuse = baseColor;

    float3 V = -rayDir;
    float NoV = saturate(dot(normal, V));
    if (NoV <= 0.0f)
        return;

    // Fresnel
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    float3 F = fresnelSchlick(NoV, F0);

    // Diffuse energy term
    kDiffuse = (1.0f - F) * (1.0f - metallic) * baseColor;

    // Lobe probabilities
    float wSpec = dot(F, float3(0.2126f, 0.7152f, 0.0722f));
    float wDiff = dot(kDiffuse, float3(0.2126f, 0.7152f, 0.0722f));
    pSpec = clamp(wSpec / max(wSpec + wDiff, 1e-6f), 0.05f, 0.95f);
    pDiff = 1.0f - pSpec;
}

void BuildOrthonormalBasis(float3 n, out float3 t, out float3 b)
{
    // Frisvad basis construction (stable, branch-light)
    float s = (n.z >= 0.0f) ? 1.0f : -1.0f;
    float a = -1.0f / (s + n.z);
    float xy = n.x * n.y * a;
    t = float3(1.0f + s * n.x * n.x * a, s * xy, -s * n.x);
    b = float3(xy, s + n.y * n.y * a, -n.y);
}

float3 SampleGGXHalfVector(float2 u, float alpha, float3 N)
{
    float a2 = alpha * alpha;
    float phi = M_PI * 2.0f * u.x;

    // GGX NDF sampling (isotropic)
    float cosTheta = sqrt((1.0f - u.y) / (1.0f + (a2 - 1.0f) * u.y));
    float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));

    float3 Hlocal = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    float3 T, B;
    BuildOrthonormalBasis(N, T, B);
    return normalize(T * Hlocal.x + B * Hlocal.y + N * Hlocal.z);
}
