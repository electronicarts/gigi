// Unnamed technique, shader PS
/*$(ShaderResources)*/

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float2 UV         : TEXCOORD0;
	float3 normal     : TEXCOORD1;
	int    materialID : TEXCOORD2;
	float  depth      : TEXCOORD3;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
	float4 linearDepth : SV_Target1;
};

float4 ReadAlbedo(float2 UV, in Texture2D<float4> albedo, in Texture2D<float> transparency)
{
	float4 ret;
	ret.rgb = albedo.Sample(linearWrapSampler, UV).rgb;
	ret.a = transparency.Sample(linearWrapSampler, UV);
	return ret;
}

float4 ReadAlbedo(float2 UV, in Texture2D<float4> albedo)
{
	float4 ret;
	ret.rgb = albedo.Sample(linearWrapSampler, UV).rgb;
	ret.a = 1.0f;
	return ret;
}


PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.linearDepth = input.depth;

	float4 albedo = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Textures should be in "..\..\OBJAssets\sponza\", but it doesn't copy them over correctly when doing that.
	switch(input.materialID)
	{
		case 0: // leaf
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_thorn_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/, /*$(Image2D:textures\sponza_thorn_mask.png:R8_UNorm:float:false:true)*/); break;

		case 1: // vase_round
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\vase_round.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 2: // Material__57
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\vase_plant.png:RGBA8_UNorm_SRGB:float4:true:true)*/, /*$(Image2D:textures\vase_plant_mask.png:R8_UNorm:float:false:true)*/); break;

		case 3: // Material__298
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\background.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 4: // bricks
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\spnza_bricks_a_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 5: // arch
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_arch_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 6: // ceiling
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_ceiling_a_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 7: // column_a
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_column_a_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 8: // floor
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_floor_a_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 9: // column_c
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_column_c_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 10: // details
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_details_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 11: // column_b
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_column_b_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 12: // Material__47
		albedo = float4(0.0f, 0.0f, 0.0f, 0.0f); break;

		case 13: // flagpole
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_flagpole_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 14: // fabric_e
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_fabric_green_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 15: // fabric_d
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_fabric_blue_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 16: // fabric_a
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_fabric_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 17: // fabric_g
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_curtain_blue_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 18: // fabric_c
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_curtain_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 19: // fabric_f
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_curtain_green_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 20: // chain
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\chain_texture.png:RGBA8_UNorm_SRGB:float4:true:true)*/, /*$(Image2D:textures\chain_texture_mask.png:R8_UNorm:float:false:true)*/); break;

		case 21: // vase_hanging
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\vase_hanging.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 22: // vase
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\vase_dif.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 23: // Material__25
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\lion.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		case 24: // roof
		albedo = ReadAlbedo(input.UV, /*$(Image2D:textures\sponza_roof_diff.png:RGBA8_UNorm_SRGB:float4:true:true)*/); break;

		default: albedo = float4(1.0f, 0.0f, 1.0f, 1.0f); break;
	}

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

	ret.colorTarget = float4(color, albedo.a);

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