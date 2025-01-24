// Unnamed technique, shader ModelViewerPS
/*$(ShaderResources)*/

// Define some constants
#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI 0.31830988618379067153776752674503

struct PSInput // AKA VSOutput
{
	float4 Position   : SV_POSITION;
	float4 Color      : TEXCOORD0;
	float3 Normal     : NORMAL;
	float3 WorldPos   : POSITION;
};

struct PSOutput
{
	float4 colorTargetF32 : SV_Target0;
	float4 colorTargetU8sRGB : SV_Target1;
};

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

float G1_GGX_Schlick(float NoV, float alpha) {
  float k = alpha / 2.0;
  return max(NoV, 0.001) / (NoV * (1.0 - k) + k);
}

float G_Smith(float NoV, float NoL, float alpha) {
  return G1_GGX_Schlick(NoL, alpha) * G1_GGX_Schlick(NoV, alpha);
}

float3 microfacetBRDF(float3 L, float3 V, float3 N, float metallic, float roughness, float3 baseColor, float specularlevel) 
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

PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	if (/*$(Variable:ViewMode)*/ == ViewModes::PlasticShaded)
	{
		float3 cameraPosition = /*$(Variable:CameraPos)*/;
		float3 viewDirection = normalize(cameraPosition - input.WorldPos);

		// Material Properties
		float3 basecolor = float3(0.8f, 0.8f, 0.8f);
		// float3 basecolor = /*$(Variable:ViewerColor)*/;
		float roughness = 0.4f;
		float specularlevel = 0.5f;
		float3 normal = input.Normal;
		float metalic = 0.0f;

		// Lighting Properties
		float3 cameraSpaceLightDirection = normalize(float3(0.0f, 0.5f, -0.5f));
		float3 lightDirection = mul(cameraSpaceLightDirection, /*$(Variable:InvViewMtx)*/);
		float lightIntensity = 1.0f;
		float3 lightColor = float3(1.0f, 1.0f, 1.0f);
		float3 lightPerpendicularIrradiance = lightIntensity * lightColor;

		// Directional Lighting
		float3 irradiance = max(dot(lightDirection, normal), 0.0) * lightPerpendicularIrradiance; 
		float3 brdf = microfacetBRDF(lightDirection, viewDirection, input.Normal, metalic, roughness, basecolor, specularlevel);
		float3 radiance = irradiance * brdf;

		// Ambient lighting
		radiance += float3(0.02f, 0.02f, 0.02f);

		ret.colorTargetF32 = float4(radiance,1.0f);
		ret.colorTargetU8sRGB = float4(radiance,1.0f);


	}
	else
	{
		ret.colorTargetF32 = input.Color;
		ret.colorTargetU8sRGB = input.Color;
	}
	return ret;
}
