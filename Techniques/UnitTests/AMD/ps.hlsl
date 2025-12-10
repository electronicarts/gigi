// Unnamed technique, shader PS
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float2 UV         : TEXCOORD0;
	float3 normal     : TEXCOORD1;
	int    materialID : TEXCOORD2;
	float  depth      : TEXCOORD3;
	#if OPAQUE_PASS == 1
		float4 positionLastFrame : TEXCOORD4;
		float4 positionThisFrame : TEXCOORD5;
	#endif
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
    float4 linearDepth : SV_Target1;
	#if OPAQUE_PASS == 1
    	float2 motionVectors : SV_Target2;
	#endif
};

float4 ReadAlbedo(float2 UV, in Texture2D<float4> albedo, in Texture2D<float> transparency)
{
	float4 ret;
    ret.rgb = albedo.SampleBias(linearWrapSampler, UV, /*$(Variable:LODBias)*/).rgb;
    ret.a = transparency.SampleBias(linearWrapSampler, UV, /*$(Variable:LODBias)*/);
	return ret;
}

float4 ReadAlbedo(float2 UV, in Texture2D<float4> albedo)
{
    float4 ret;
    ret.rgb = albedo.SampleBias(linearWrapSampler, UV, /*$(Variable:LODBias)*/).rgb;
	ret.a = 1.0f;
	return ret;
}

float checkers(in float2 p)
{
    float2 q = floor(p);
    return (q.x + q.y) % 2.0f;
}

PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.linearDepth = input.depth;

	float4 albedo = float4(1.0f, 0.0f, 1.0f, checkers(input.UV * 50.0f) >= 1.0f ? 1.0f : 0.5f);

	#if OPAQUE_PASS == 1
		if (albedo.a < 1.0f)
			discard;
	#else
		if (albedo.a >= 1.0f)
			discard;
	#endif

	float3 lightDir = normalize(float3(0.2f, 1.0f, 0.3f));
	float3 lightColor = float3(1.0f, 1.0f, 1.0f);
	float3 ambientLightColor = float3(0.1f, 0.1f, 0.1f);

	float3 color = albedo * (max(dot(input.normal, lightDir), 0.0f) * lightColor + ambientLightColor);

    float preExposure = pow(2.0f, /*$(Variable:PreExposureFStops)*/);
    color *= preExposure;

	ret.colorTarget = float4(color, albedo.a);

	#if OPAQUE_PASS == 1
        ret.motionVectors = (input.positionLastFrame.xy / input.positionLastFrame.w) - (input.positionThisFrame.xy / input.positionThisFrame.w);
        ret.motionVectors *= float2(0.5f, -0.5f);
	#endif

	return ret;
}

/*
Info about obj format: https://paulbourke.net/dataformats/mtl/

Obj notes:
map_ka
map_kd
map_ks
map_ke
map_d
map_bump

* bump (MASTER_Interior_01_Floor_Tile_Hexagonal_BLENDSHADER)
* d - transparency
* illum - illumination model
* ka - ambient color
* kd - diffuse color
* ks - specular color
* ke - emissive color
* Ni - refraction index
* Ns - specular exponent
* Tr - transparency (1-d)
* Tf - transmission color
*/