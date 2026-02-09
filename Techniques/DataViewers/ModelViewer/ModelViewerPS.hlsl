// Unnamed technique, shader ModelViewerPS
/*$(ShaderResources)*/

/*$(Embed:_material.hlsli)*/

// Define some constants
#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI 0.31830988618379067153776752674503

struct PSInput // AKA VSOutput
{
	float4 Position   : SV_POSITION;
	float4 Color      : TEXCOORD0;
	float3 Normal     : NORMAL;
	float3 WorldPos   : POSITION;
	int MaterialID    : TEXCOORD1;
	float2 UV0	      : TEXCOORD2;
    float2 UV1	      : TEXCOORD3;
    float2 UV2	      : TEXCOORD4;
    float2 UV3	      : TEXCOORD5;
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

    // Material Properties
    float3 matNormal = float3(0.0f, 0.0f, 1.0f);
    float matOcclusion = 1.0f;
    Struct_Materials material = Material_MaterialBuffer(Materials, input.MaterialID, input.UV0, input.UV1, input.UV2, input.UV3, matNormal, matOcclusion, ddx(input.UV0), ddy(input.UV0), ddx(input.UV1), ddy(input.UV1), ddx(input.UV2), ddy(input.UV2), ddx(input.UV3), ddy(input.UV3));

    if (/*$(Variable:ViewMode)*/ == ViewModes::MaterialOcclusion)
    {
        ret.colorTargetF32 = float4(matOcclusion.xxx, 1.0f);
        ret.colorTargetU8sRGB = float4(matOcclusion.xxx, 1.0f);
    }
    else if (/*$(Variable:ViewMode)*/ == ViewModes::MaterialColor)
    {
        ret.colorTargetF32 = float4(material.baseColor.rgb, 1.0f);
        ret.colorTargetU8sRGB = float4(material.baseColor.rgb, 1.0f);
    }
    else if (/*$(Variable:ViewMode)*/ == ViewModes::MaterialEmissive)
    {
        ret.colorTargetF32 = float4(material.emissive.rgb, 1.0f);
        ret.colorTargetU8sRGB = float4(material.emissive.rgb, 1.0f);
    }
    else if (/*$(Variable:ViewMode)*/ == ViewModes::MaterialMetallic)
    {
        ret.colorTargetF32 = float4(material.metallic.xxx, 1.0f);
        ret.colorTargetU8sRGB = float4(material.metallic.xxx, 1.0f);
    }
    else if (/*$(Variable:ViewMode)*/ == ViewModes::MaterialRoughness)
    {
        ret.colorTargetF32 = float4(material.roughness.xxx, 1.0f);
        ret.colorTargetU8sRGB = float4(material.roughness.xxx, 1.0f);
    }
    else if (/*$(Variable:ViewMode)*/ == ViewModes::MaterialNormal)
    {
        ret.colorTargetF32 = float4(/*$(Variable:RemapRanges)*/ ? (matNormal + 1.0f) / 2.0f : matNormal, 1.0f);
		ret.colorTargetU8sRGB = float4(/*$(Variable:RemapRanges)*/ ? (matNormal + 1.0f) / 2.0f : matNormal, 1.0f);
    }
    else if (/*$(Variable:ViewMode)*/ == ViewModes::Shaded)
	{
		float3 cameraPosition = /*$(Variable:CameraPos)*/;
		float3 viewDirection = normalize(cameraPosition - input.WorldPos);

        float3 basecolor = material.baseColor.rgb * input.Color.rgb;
		float roughness = material.roughness;
		float specularlevel = 0.5f;
		float3 normal = input.Normal;
		float metalic = material.metallic;

        float3 radiance = float3(0.0f, 0.0f, 0.0f);

		// If only one light, and it is black, do default lighting
        uint2 lightsCountStride;
        Lights.GetDimensions(lightsCountStride.x, lightsCountStride.y);
        if (/*$(Variable:ForceDefaultLights)*/ || (lightsCountStride.x == 1 && Lights[0].ColorIntensity.w <= 0.0f))
        {
            // Lighting Properties
            float3 lightDirection;
            if (/*$(Variable:LightIsLocal)*/)
                lightDirection = mul(float4(0.0f, 0.5f, -0.5f, 0.0f), /*$(Variable:InvViewMtx)*/).xyz;
            else
                lightDirection = normalize(/*$(Variable:DirectionalLightDir)*/);
            float lightIntensity = /*$(Variable:DirectionalLightPower)*/;
            float3 lightColor = /*$(Variable:DirectionalLightColor)*/;
			float3 lightPerpendicularIrradiance = lightIntensity * lightColor;

			// Directional Lighting
			float3 irradiance = max(dot(lightDirection, normal), 0.0) * lightPerpendicularIrradiance; 
			float3 brdf = microfacetBRDF(lightDirection, viewDirection, input.Normal, metalic, roughness, basecolor, specularlevel);
			radiance = irradiance * brdf;
        }
		// Otherwise, use the lights given
        else
        {
            for (uint lightIndex = 0; lightIndex < lightsCountStride.x; ++lightIndex)
            {
				// Positional light
				float3 lightDirection = float3(0.0f, 0.0f, 0.0f);
                if (Lights[0].PosDir.w == 1.0f)
                {
					float3 lightPos = Lights[lightIndex].PosDir.xyz;
					lightDirection = normalize(lightPos - input.WorldPos);
                    float distance = length(lightPos - input.WorldPos);
                    if (Lights[lightIndex].Range > 0.0f && distance >= Lights[lightIndex].Range)
					{
						continue;
					}
                }
                // Directional light
                else
                {
					lightDirection = normalize(Lights[lightIndex].PosDir.xyz);
				}

                float3 lightPerpendicularIrradiance = Lights[lightIndex].ColorIntensity.w * Lights[lightIndex].ColorIntensity.xyz;

                // Blender has power in watts. We don't use physically accurate units here, so just going to tone things down a bit.
                lightPerpendicularIrradiance /= 100.0f;

                float3 irradiance = max(dot(lightDirection, normal), 0.0) * lightPerpendicularIrradiance;
                float3 brdf = microfacetBRDF(lightDirection, viewDirection, input.Normal, metalic, roughness, basecolor, specularlevel);
                radiance = irradiance * brdf;
            }
		}

        // Ambient and emissive lighting
        radiance *= matOcclusion;
        radiance += /*$(Variable:AmbientColor)*/ * /*$(Variable:AmbientPower)*/;
        radiance += material.emissive;

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
