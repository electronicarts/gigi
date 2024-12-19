// Unnamed technique, shader MakeTexture2DArrayCS
/*$(ShaderResources)*/

float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// Mip0
	{
		float3 uvw = (float3(DTid) + 0.5f) / float3(/*$(Variable:RenderSize)*/);
		TexF32Mip0[DTid] = float4(uvw, 1.0f);
		TexU8Mip0[DTid] = float4(uvw, 1.0f);
		TexU8_sRGBMip0[DTid] = float4(LinearToSRGB(uvw), 1.0f);

		SingleF32[DTid] = float4(uvw, 1.0f);
		SingleU8[DTid] = float4(uvw, 1.0f);
		SingleU8_sRGB[DTid] = float4(LinearToSRGB(uvw), 1.0f);
	}

	// Mip1
	{
		float3 uvw = (float3(DTid) + 0.5f) / float3(/*$(Variable:RenderSize)*/ / uint3(2,2,1));
		TexF32Mip1[DTid] = float4(uvw, 1.0f);
		TexU8Mip1[DTid] = float4(uvw, 1.0f);
		TexU8_sRGBMip1[DTid] = float4(LinearToSRGB(uvw), 1.0f);
	}

	// Mip2
	{
		float3 uvw = (float3(DTid) + 0.5f) / float3(/*$(Variable:RenderSize)*/ / uint3(4,4,1));
		TexF32Mip2[DTid] = float4(uvw, 1.0f);
		TexU8Mip2[DTid] = float4(uvw, 1.0f);
		TexU8_sRGBMip2[DTid] = float4(LinearToSRGB(uvw), 1.0f);
	}

	// Mip2
	{
		float3 uvw = (float3(DTid) + 0.5f) / float3(/*$(Variable:RenderSize)*/ / uint3(8,8,1));
		TexF32Mip3[DTid] = float4(uvw, 1.0f);
		TexU8Mip3[DTid] = float4(uvw, 1.0f);
		TexU8_sRGBMip3[DTid] = float4(LinearToSRGB(uvw), 1.0f);
	}
}

/*
Shader Resources:
	Texture TexF32Mip0 (as UAV)
	Texture TexF32Mip1 (as UAV)
	Texture TexF32Mip2 (as UAV)
	Texture TexF32Mip3 (as UAV)
	Texture TexU8Mip0 (as UAV)
	Texture TexU8Mip1 (as UAV)
	Texture TexU8Mip2 (as UAV)
	Texture TexU8Mip3 (as UAV)
	Texture TexU8_sRGBMip0 (as UAV)
	Texture TexU8_sRGBMip1 (as UAV)
	Texture TexU8_sRGBMip2 (as UAV)
	Texture TexU8_sRGBMip3 (as UAV)
	Texture SingleF32 (as UAV)
	Texture SingleU8 (as UAV)
	Texture SingleU8_sRGB (as UAV)
*/
