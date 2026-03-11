// Unnamed technique, shader ModelViewerPS
/*$(ShaderResources)*/

#include "tonemap.hlsl"
#include "PBR.hlsli"

/*$(Embed:_material.hlsli)*/
/*$(Embed:materialsOverride.hlsli)*/

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

float3 ApplyDisplayTransform(float3 color)
{
    color *= pow(2.0f, /*$(Variable:ExposureFStops)*/);

    switch(/*$(Variable:ToneMapper)*/)
    {
        case ToneMappingOperation::None: break;

        case ToneMappingOperation::Reinhard:
        {
            color = color / (1.0f + color);
            break;
        }

        case ToneMappingOperation::ACES_Luminance:
        {
            color = ACESFilm(color * 0.6f);
            break;
        }

        case ToneMappingOperation::ACES:
        {
            color = ACESFitted(color);
            break;
        }
    }

    return color;
}

PSOutput psmain(PSInput input)
{
    PSOutput ret = (PSOutput)0;

    // Material Properties
    float3 matNormal = float3(0.0f, 0.0f, 1.0f);
    float matOcclusion = 1.0f;
    Struct_Materials material = Material_MaterialBuffer(Materials, input.MaterialID, input.UV0, input.UV1, input.UV2, input.UV3, matNormal, matOcclusion, ddx(input.UV0), ddy(input.UV0), ddx(input.UV1), ddy(input.UV1), ddx(input.UV2), ddy(input.UV2), ddx(input.UV3), ddy(input.UV3));
    ApplyMaterialOverride(material);

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
		float roughness = clamp(material.roughness, 0.04f, 1.0f);
		float specularlevel = 0.5f;
		float3 normal = normalize(input.Normal);
		float metalic = material.metallic;

        float3 radiance = float3(0.0f, 0.0f, 0.0f);

        // Diffuse IBL term, same basic model as PathTracer raster PS.
        float3 viewDir = normalize(/*$(Variable:CameraPos)*/ - input.WorldPos);
        float NoV = saturate(dot(normal, viewDir));
        float3 irradianceIBL = DiffuseIBL.SampleLevel(LinearWrapSampler, normal, 0).rgb;
        float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), basecolor, metalic);
        float3 F = fresnelSchlick(NoV, F0);
        float3 kD = (1.0f - F) * (1.0f - metalic);
        radiance += kD * basecolor * irradianceIBL * matOcclusion;

        // Specular IBL from prefiltered environment map + BRDF integration LUT.
        float3 R = reflect(-viewDir, normal);
        uint specWidth, specHeight, specMipCount;
        SpecularIBL.GetDimensions(0, specWidth, specHeight, specMipCount);
        float specMip = saturate(roughness) * max(float(specMipCount) - 1.0f, 0.0f);
        float3 specularIBL = SpecularIBL.SampleLevel(LinearWrapSampler, R, specMip).rgb;
        float2 envBRDF = SpecularBRDFLUT.SampleLevel(LinearClampSampler, float2(NoV, saturate(roughness)), 0).rg;
        float3 specular = specularIBL * (F0 * envBRDF.x + envBRDF.y);
        radiance += specular * matOcclusion;

        // Direct lighting
        if (/*$(Variable:DirectLightingMultiplier)*/ > 0.0f)
        {
            uint2 lightsCountStride;
            Lights.GetDimensions(lightsCountStride.x, lightsCountStride.y);

            for (uint lightIndex = 0; lightIndex < lightsCountStride.x; ++lightIndex)
            {
                float3 lightDirection = float3(0.0f, 0.0f, 0.0f);
                float attenuation = 1.0f;

                // Positional light
                if (Lights[lightIndex].PosDir.w == 1.0f)
                {
                    float3 toLight = Lights[lightIndex].PosDir.xyz - input.WorldPos;
                    float distanceSq = max(dot(toLight, toLight), 1e-6f);
                    float distance = sqrt(distanceSq);
                    lightDirection = toLight / distance;

                    if (Lights[lightIndex].Range > 0.0f && distance >= Lights[lightIndex].Range)
                    {
                        continue;
                    }

                    if (Lights[lightIndex].Range > 0.0f)
                    {
                        float rangeFade = saturate(1.0f - distance / Lights[lightIndex].Range);
                        attenuation *= rangeFade * rangeFade;
                    }

                    attenuation *= 1.0f / distanceSq;
                }
                // Directional light
                else
                {
                    lightDirection = normalize(Lights[lightIndex].PosDir.xyz);
                }

                float3 lightPerpendicularIrradiance = Lights[lightIndex].ColorIntensity.w * Lights[lightIndex].ColorIntensity.xyz;

                float NoL = saturate(dot(lightDirection, normal));
                if (NoL <= 0.0f)
                    continue;

                float3 irradiance = NoL * lightPerpendicularIrradiance * attenuation;
                float3 brdf = MicrofacetBRDF(lightDirection, viewDirection, input.Normal, metalic, roughness, basecolor, specularlevel);
                radiance += irradiance * brdf * /*$(Variable:DirectLightingMultiplier)*/;
            }
		}

        // Ambient and emissive lighting
        radiance *= matOcclusion;
        radiance += material.emissive;

        float3 displayColor = ApplyDisplayTransform(radiance);

        ret.colorTargetF32 = float4(radiance,1.0f);
        ret.colorTargetU8sRGB = float4(displayColor, 1.0f);
	}
	else
	{
		ret.colorTargetF32 = input.Color;
		ret.colorTargetU8sRGB = input.Color;
	}
	return ret;
}
