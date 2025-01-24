// Unnamed technique, shader CSMakeOutput
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

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	uint4 dims;
	/*$(Image3D:..\\skyboxes\\Test_%s.png:RGBA8_Unorm_sRGB:float4:true:true)*/.GetDimensions(0, dims.x, dims.y, dims.z, dims.w);

	uint2 readPos = px % dims.xy;

	Output[px] = float4(LinearToSRGB(/*$(Image3D:..\\skyboxes\\Test_%s.png:RGBA8_Unorm_sRGB:float4:true:true)*/[uint3(readPos, 0)].rgb), 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
