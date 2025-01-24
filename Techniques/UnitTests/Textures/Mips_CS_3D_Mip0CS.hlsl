// Unnamed technique, shader Mip0CS
/*$(ShaderResources)*/

static const float c_goldenRatioConugate = 0.61803398874f;

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
	uint3 px = DTid.xyz;

	uint2 dims;
	/*$(Image2D:../cabinsmall.png:RGBA8_Unorm_sRGB:float4:true:true)*/.GetDimensions(dims.x, dims.y);

	uint2 readOffset = uint2(
		uint(c_goldenRatioConugate * float(dims.x)),
		uint(c_goldenRatioConugate * float(dims.y))
	);

	uint2 readPos = (px.xy + readOffset * px.z) % dims;

	Output[px] = float4(LinearToSRGB(/*$(Image2D:../cabinsmall.png:RGBA8_Unorm_sRGB:float4:true:true)*/[readPos].rgb), 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
