// Unnamed technique, shader Mip0CS


RWTexture2DArray<float4> Output : register(u0);
Texture2D<float4> _loadedTexture_0 : register(t0);

#line 2


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

[numthreads(8, 8, 1)]
#line 15
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	uint2 dims;
	_loadedTexture_0.GetDimensions(dims.x, dims.y);

	// Output1
	{
		uint2 readPos = px % dims;
		Output[uint3(px,0)] = float4(LinearToSRGB(_loadedTexture_0[readPos].rgb), 1.0f);
	}

	// Output2
	{
		uint2 readPos = (dims/2 + px) % dims;
		Output[uint3(px,1)] = float4(LinearToSRGB(_loadedTexture_0[readPos].rgb), 1.0f);
	}
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
