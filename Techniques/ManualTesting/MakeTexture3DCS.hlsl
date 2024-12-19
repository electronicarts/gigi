// Unnamed technique, shader MakeTexture3DCS
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
	// Mip 0
	{
		uint3 renderSize = max(/*$(Variable:RenderSize)*/ / uint3(1,1,1), uint3(1,1,1));
		float3 uvw = (float3(DTid) + 0.5f) / float3(renderSize);
		TexF32Mip0[DTid] = float4(uvw, 1.0f);
		TexF32_NoMips[DTid] = float4(uvw, 1.0f);
		TexU8Mip0[DTid] = float4(uvw, 1.0f);
		TexU8_NoMips[DTid] = float4(uvw, 1.0f);
		TexU8_sRGBMip0[DTid] = float4(LinearToSRGB(uvw), 1.0f);
		TexU8_sRGB_NoMips[DTid] = float4(LinearToSRGB(uvw), 1.0f);
	}

	// Mip1
	{
		uint3 renderSize = max(/*$(Variable:RenderSize)*/ / uint3(2,2,2), uint3(1,1,1));
		float3 uvw = (float3(DTid) + 0.5f) / float3(renderSize);
		TexF32Mip1[DTid] = float4(uvw, 1.0f);
		TexU8Mip1[DTid] = float4(uvw, 1.0f);
		TexU8_sRGBMip1[DTid] = float4(LinearToSRGB(uvw), 1.0f);
	}

	// Mip2
	{
		uint3 renderSize = max(/*$(Variable:RenderSize)*/ / uint3(4,4,4), uint3(1,1,1));
		float3 uvw = (float3(DTid) + 0.5f) / float3(renderSize);
		TexF32Mip2[DTid] = float4(uvw, 1.0f);
		TexU8Mip2[DTid] = float4(uvw, 1.0f);
		TexU8_sRGBMip2[DTid] = float4(LinearToSRGB(uvw), 1.0f);
	}

	// Mip3
	{
		uint3 renderSize = max(/*$(Variable:RenderSize)*/ / uint3(8,8,8), uint3(1,1,1));
		float3 uvw = (float3(DTid) + 0.5f) / float3(renderSize);
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
	Texture TexF32_NoMips (as UAV)
	Texture TexU8Mip0 (as UAV)
	Texture TexU8Mip1 (as UAV)
	Texture TexU8Mip2 (as UAV)
	Texture TexU8Mip3 (as UAV)
	Texture TexU8_NoMips (as UAV)
	Texture TexU8_sRGBMip0 (as UAV)
	Texture TexU8_sRGBMip1 (as UAV)
	Texture TexU8_sRGBMip2 (as UAV)
	Texture TexU8_sRGBMip3 (as UAV)
	Texture TexU8_sRGB_NoMips (as UAV)
*/
