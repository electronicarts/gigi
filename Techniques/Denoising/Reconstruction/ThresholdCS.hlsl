// Unnamed technique, shader ThresholdCS
/*$(ShaderResources)*/

uint wang_hash_init(uint3 seed)
{
	return uint(seed.x * uint(1973) + seed.y * uint(9277) + seed.z * uint(26699)) | uint(1);
}

uint wang_hash_uint(inout uint seed)
{
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}
float wang_hash_float01(inout uint state)
{
	return float(wang_hash_uint(state) & 0x00FFFFFF) / float(0x01000000);
}

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

float SampleSpatioTemporalTexture(in Texture2DArray<float> t, uint2 px, int frameIndex)
{
	uint width, height, depth;
	t.GetDimensions(width, height, depth);
	return t[uint3(px % uint2(width, height), frameIndex % depth)];
}

/*$(_compute:ThresholdCS)*/(uint3 DTid : SV_DispatchThreadID)
{
	int frameIndex = /*$(Variable:Animate)*/ ? /*$(Variable:frameIndex)*/ : 0;

	float4 source = Source[DTid.xy];

	float randomValue = 0.0f;
	switch(/*$(Variable:NoiseType)*/)
	{
		case ENoiseType::White:
		{
			uint rng = wang_hash_init(uint3(DTid.xy, frameIndex));
			randomValue = wang_hash_float01(rng);
			break;
		}
		case ENoiseType::Blue: randomValue = SampleSpatioTemporalTexture(/*$(Image2DArray:real_uniform_gauss1_0_exp0101_separate05_%i.png:R8_Unorm:float:false)*/, DTid.xy, frameIndex); break;
		case ENoiseType::Binomial: randomValue = SampleSpatioTemporalTexture(/*$(Image2DArray:real_uniform_binomial3x3_exp0101_separate05_%i.png:R8_Unorm:float:false)*/, DTid.xy, frameIndex); break;
	}

	if (randomValue < /*$(Variable:Density)*/ || /*$(Variable:Density)*/ >= 1.0f)
		Dest[DTid.xy] = float4(LinearToSRGB(source.rgb), source.a);
	else
		Dest[DTid.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

/*
Shader Resources:
	Texture Source (as SRV)
	Texture Dest (as UAV)
*/
