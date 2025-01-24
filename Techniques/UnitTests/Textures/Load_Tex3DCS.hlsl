// Unnamed technique, shader Load_Tex3DCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// Figure out which texture to read, and where
	uint2 px = DTid.xy;

	int imageIndex = px.y / 8;
	px.y = px.y % 8;

	int slicewh = 0;
	int sliceCount = 0;
	int mipIndex = 0;
	if (px.x < 32)
	{
		px.x -= 0;
		slicewh = 8;
		sliceCount = 4;
		mipIndex = 0;
	}
	else if (px.x < 40)
	{
		px.x -= 32;
		slicewh = 4;
		sliceCount = 2;
		mipIndex = 1;
	}
	else if (px.x < 42)
	{
		px.x -= 40;
		slicewh = 2;
		sliceCount = 1;
		mipIndex = 2;
	}
	else if (px.x < 43)
	{
		px.x -= 42;
		slicewh = 1;
		sliceCount = 1;
		mipIndex = 3;
	}

	int sliceIndex = px.x / slicewh;
	px.x = px.x % slicewh;

	// Out of bounds pixels are magenta
	if(px.x >= slicewh || px.y >= slicewh)
	{
		Output[DTid.xy] = float4(1.0f, 0.0f, 1.0f, 1.0f);
		return;
	}

	// read, write, exit
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	switch (imageIndex)
	{
		case 0: color = BC4U.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 1: color = BC4U_NoMips.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 2: color = BC5U.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 3: color = BC5U_NoMips.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 4: color = BC6U.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 5: color = BC6U_NoMips.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 6: color = BC7sRGB.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 7: color = BC7sRGB_NoMips.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 8: color = BinaryF32.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 9: color = BinaryF32_NoMips.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 10: color = BinaryU8.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 11: color = BinaryU8_NoMips.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 12: color = /*$(Image3D:Tex3D_BC4U_8x8x4_mips.dds:BC4_Unorm:float4:false:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 13: color = /*$(Image3D:Tex3D_BC4U_8x8x4_nomips.dds:BC4_Unorm:float4:false:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 14: color = /*$(Image3D:Tex3D_BC5U_8x8x4_mips.dds:BC5_Unorm:float4:false:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 15: color = /*$(Image3D:Tex3D_BC5U_8x8x4_nomips.dds:BC5_Unorm:float4:false:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 16: color = /*$(Image3D:Tex3D_BC6U_8x8x4_mips.dds:BC6_UF16:float4:false:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 17: color = /*$(Image3D:Tex3D_BC6U_8x8x4_nomips.dds:BC6_UF16:float4:false:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 18: color = /*$(Image3D:Tex3D_BC7sRGB_8x8x4_mips.dds:BC7_Unorm_sRGB:float4:true:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
		case 19: color = /*$(Image3D:Tex3D_BC7sRGB_8x8x4_nomips.dds:BC7_Unorm_sRGB:float4:true:false)*/.Load(uint4(px, sliceIndex, mipIndex)); break;
	}
	Output[DTid.xy] = color;

	/*
	A single image has 4 mips:
	8x8x4
	4x4x2
	2x2x1
	1x1x1

	We can lay out the slices horizontally to have these 4:
	32x8
	8x4
	2x1
	1x1

	That can fit in a 43x8

	There are 8 imported images and 4 non imported images, so we can put them vertically to make it be 43*96.
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
