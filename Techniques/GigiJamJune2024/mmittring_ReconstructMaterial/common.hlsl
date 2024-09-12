
#define FLT_MAX 3.402823466e+38F
#define PI 3.14159265f

#ifdef SLANG
#define DIFFERENTIABLE [Differentiable]
#define NO_DIFF no_diff
#else
#define DIFFERENTIABLE
#define NO_DIFF
#endif


struct GBuffer 
#ifdef SLANG
    : IDifferentiable
#endif
{
	// 0:near..1:far
	float depth;
	float3 normal;
	float3 baseColor;
	// 0..1
	float metallic;
	// 0..1
	float linearRoughness;
    
    [mutating]
    void init()
    {
        depth = 1.0f;
	    normal = 0;
	    baseColor = 0;
	    metallic = 0;
        linearRoughness = 0.0f;
    }
};

DIFFERENTIABLE
float pow5(float x)
{
    float x2 = x * x;
    return x2 * x2 * x;
}

// from https://www.shadertoy.com/view/XlKSDR
DIFFERENTIABLE
float D_GGX(float linearRoughness, float NoH, const float3 h) {
    // Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
    float oneMinusNoHSquared = 1.0 - NoH * NoH;
    float a = NoH * linearRoughness;
	const float epsilon = 0.01f;	// MartinM: to avoid NaN / div by 0
    float k = linearRoughness / (oneMinusNoHSquared + a * a + epsilon);
    float d = k * k * (1.0 / PI);
    return d;
}

// from https://www.shadertoy.com/view/XlKSDR
DIFFERENTIABLE
float V_SmithGGXCorrelated(float linearRoughness, float NoV, float NoL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = linearRoughness * linearRoughness;
    float GGXV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float GGXL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    return 0.5 / (GGXV + GGXL);
}

// from https://www.shadertoy.com/view/XlKSDR
DIFFERENTIABLE
float3 F_Schlick(const float3 f0, float VoH) {
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (1 - f0) * pow5(1.0 - VoH);
}

// from https://www.shadertoy.com/view/XlKSDR
DIFFERENTIABLE
float F_Schlick(float f0, float f90, float VoH) {
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

// from https://www.shadertoy.com/view/XlKSDR
DIFFERENTIABLE
float Fd_Burley(float linearRoughness, float NoV, float NoL, float LoH) {
    // Burley 2012, "Physically-Based Shading at Disney"
    float f90 = 0.5 + 2.0 * linearRoughness * LoH * LoH;
    float lightScatter = F_Schlick(1.0, f90, NoL);
    float viewScatter  = F_Schlick(1.0, f90, NoV);
    return lightScatter * viewScatter * (1.0 / PI);
}

float Fd_Lambert() {
    return 1.0 / PI;
}

// compute shading, the HDR color as material response to the incoming light
// @param lightDirection does not have to be normalized e.g. float3(-0.6f, -0.7f, 0.7f);
// @param viewDirection e.g. float3(0, 0, -1)
// @return HDR color float3(0.., 0.., 0..)
DIFFERENTIABLE
float3 BRDF(float2 uv, GBuffer gbuffer, NO_DIFF float3 lightDirection, NO_DIFF float3 viewDirection)
{
	lightDirection = normalize(lightDirection);

	float3 baseColor = gbuffer.baseColor;
	float metallic = gbuffer.metallic;
	float linearRoughness = gbuffer.linearRoughness;

	// light
	float3 lightColor = float3(0.98f, 0.92f, 0.89f) * 2.0f;
	float attenuation = 1.0f;
	
	float3 position = float3(uv, gbuffer.depth);
    float3 diffuseColor = (1.0 - metallic) * baseColor.rgb;
    float3 f0 = 0.04f * (1.0f - metallic) + baseColor.rgb * metallic;

	// from https://www.shadertoy.com/view/XlKSDR
	float3 v = normalize(-viewDirection);
    float3 n = normalize(gbuffer.normal);
    float3 h = normalize(v + lightDirection);
    float3 r = normalize(reflect(viewDirection, n));

    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = saturate(dot(n, lightDirection));
    float NoH = saturate(dot(n, h));
    float LoH = saturate(dot(lightDirection, h));

	// specular BRDF
    float D = D_GGX(linearRoughness, NoH, h);
    float V = V_SmithGGXCorrelated(linearRoughness, NoV, NoL);
    float3  F = F_Schlick(f0, LoH);
    float3 Fr = (D * V) * F;

    // diffuse BRDF
    float3 Fd = diffuseColor * Fd_Burley(linearRoughness, NoV, NoL, LoH);

    float3 color = Fd + Fr;
    color *= (attenuation * NoL) * lightColor;

	return color;
}





void sphere(inout GBuffer gbuffer, float2 pos, float3 center, float radius, float3 baseColor, float linearRoughness = 0.1f, float metallic = 0)
{
	// -1..1
	float2 norm = (pos - center.xy) / radius;

	float dist2 = dot(norm, norm);
	if (dist2 < 1)
	{
		// 0..1
		float h = sqrt(1.0f - dist2);
		float srcDepth = center.z - h * radius;
		if (srcDepth < gbuffer.depth)
		{
			gbuffer.depth = srcDepth;
			gbuffer.normal = normalize(float3(norm, h));
			gbuffer.baseColor = baseColor;
			gbuffer.metallic = metallic;
			gbuffer.linearRoughness = linearRoughness;
		}
	}
}

GBuffer scene(float2 uv)
{
	GBuffer gbuffer;

	gbuffer.depth = 1.0f;
	gbuffer.normal = 0;
	gbuffer.baseColor = 0;
	gbuffer.metallic = 0;
	gbuffer.linearRoughness = 0.0f;

	// red
	sphere(gbuffer, uv, float3(0.4f, 0.35f, 0.64f), 0.25f,	float3(1,0.3f,0.3f),	0.2f);
	// yellow metallic
	sphere(gbuffer, uv, float3(0.6f, 0.45f, 0.5f), 0.17f,	float3(1,1,0.2f),		0.45f, 1);
	// green
	sphere(gbuffer, uv, float3(0.65f, 0.75f, 0.4f), 0.3f,	float3(0.3f,1,0.3f),	0.01f);
	// white
	sphere(gbuffer, uv, float3(0.35f, 0.65f, 0.4f), 0.2f,	float3(1,1,1),			0.1f);

	return gbuffer;
}
