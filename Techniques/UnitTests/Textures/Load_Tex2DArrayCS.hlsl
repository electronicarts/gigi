// Unnamed technique, shader Load_Tex2DArrayCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// Figure out which texture to read, and where
	uint2 px = DTid.xy;

	int imageIndex = px.y / 8;
	px.y = px.y % 8;

	int arrayIndex = px.x / 16;
	px.x = px.x % 16;

	int wh = 0;
	int mipIndex = 0;
	if (px.x < 8)
	{
		wh = 8;
		mipIndex = 0;
	}
	else if (px.x < 12)
	{
		wh = 4;
		mipIndex = 1;
		px.x -= 8;
	}
	else if (px.x < 14)
	{
		wh = 2;
		mipIndex = 2;
		px.x -= 12;
	}
	else if (px.x < 15)
	{
		wh = 1;
		mipIndex = 3;
		px.x -= 14;
	}
	else
	{
		wh = 0;
		mipIndex = 4;
		px.x -= 15;
	}

	// Out of bounds pixels are magenta
	if(px.x >= wh || px.y >= wh)
	{
		Output[DTid.xy] = float4(1.0f, 0.0f, 1.0f, 1.0f);
		return;
	}

	// read, write, exit
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	switch (imageIndex)
	{
		case 0: color = BC4U.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 1: color = BC4U_NoMips.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 2: color = BC5U.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 3: color = BC5U_NoMips.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 4: color = BC6U.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 5: color = BC6U_NoMips.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 6: color = BC7sRGB.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 7: color = BC7sRGB_NoMips.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 8: color = BinaryF32.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 9: color = BinaryF32_NoMips.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 10: color = BinaryU8.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 11: color = BinaryU8_NoMips.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 12: color = /*$(Image2DArray:Tex2DArray_BC4U_8x8x4_mips.dds:BC4_Unorm:float4:false:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 13: color = /*$(Image2DArray:Tex2DArray_BC4U_8x8x4_nomips.dds:BC4_Unorm:float4:false:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 14: color = /*$(Image2DArray:Tex2DArray_BC5U_8x8x4_mips.dds:BC5_Unorm:float4:true:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 15: color = /*$(Image2DArray:Tex2DArray_BC5U_8x8x4_nomips.dds:BC5_Unorm:float4:true:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 16: color = /*$(Image2DArray:Tex2DArray_BC6U_8x8x4_mips.dds:BC6_UF16:float4:false:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 17: color = /*$(Image2DArray:Tex2DArray_BC6U_8x8x4_nomips.dds:BC6_UF16:float4:false:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 18: color = /*$(Image2DArray:Tex2DArray_BC7sRGB_8x8x4_mips.dds:BC7_Unorm_sRGB:float4:true:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
		case 19: color = /*$(Image2DArray:Tex2DArray_BC7sRGB_8x8x4_nomips.dds:BC7_Unorm_sRGB:float4:true:false)*/.Load(uint4(px, arrayIndex, mipIndex)); break;
	}
	Output[DTid.xy] = color;

	/*
	A single array index has 4 mips:
	8x8
	4x4
	2x2
	1x1
	we can fit that in 16x8.

	there are 4 indices per image, so we can put them horizontally to make it be 64x8.

	There are 8 imported images and 4 non imported images, so we can put them vertically to make it be 64*96.
	*/
}

/*
Shader Resources:
	Texture BC6U (as SRV)
	Texture BC6U_NoMips (as SRV)
	Texture BC7sRGB (as SRV)
	Texture BC7sRGB_NoMips (as SRV)
	Texture BinaryF32 (as SRV)
	Texture BinaryF32_NoMips (as SRV)
	Texture BinaryU8 (as SRV)
	Texture BinaryU8_NoMips (as SRV)
	Texture Output (as UAV)
*/
