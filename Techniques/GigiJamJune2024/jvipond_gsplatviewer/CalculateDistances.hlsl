// Unnamed technique, shader CalculateDistances
/*$(ShaderResources)*/

#define VECTOR_FMT_32F 0
#define VECTOR_FMT_16 1
#define VECTOR_FMT_11 2
#define VECTOR_FMT_6 3

half3 DecodePacked_6_5_5(uint enc)
{
	return half3(
        (enc & 63) / 63.0,
        ((enc >> 6) & 31) / 31.0,
        ((enc >> 11) & 31) / 31.0);
}

half3 DecodePacked_5_6_5(uint enc)
{
	return half3(
        (enc & 31) / 31.0,
        ((enc >> 5) & 63) / 63.0,
        ((enc >> 11) & 31) / 31.0);
}

half3 DecodePacked_11_10_11(uint enc)
{
	return half3(
        (enc & 2047) / 2047.0,
        ((enc >> 11) & 1023) / 1023.0,
        ((enc >> 21) & 2047) / 2047.0);
}

float3 DecodePacked_16_16_16(uint2 enc)
{
	return float3(
        (enc.x & 65535) / 65535.0,
        ((enc.x >> 16) & 65535) / 65535.0,
        (enc.y & 65535) / 65535.0);
}

float4 DecodePacked_10_10_10_2(uint enc)
{
	return float4(
        (enc & 1023) / 1023.0,
        ((enc >> 10) & 1023) / 1023.0,
        ((enc >> 20) & 1023) / 1023.0,
        ((enc >> 30) & 3) / 3.0);
}
uint EncodeQuatToNorm10(float4 v) // 32 bits: 10.10.10.2
{
	return (uint) (v.x * 1023.5f) | ((uint) (v.y * 1023.5f) << 10) | ((uint) (v.z * 1023.5f) << 20) | ((uint) (v.w * 3.5f) << 30);
}

float3 LoadAndDecodeVector(ByteAddressBuffer dataBuffer, uint addrU, uint fmt)
{
	uint addrA = addrU & ~0x3;

	uint val0 = dataBuffer.Load(addrA);

	float3 res = 0;
	if (fmt == VECTOR_FMT_32F)
	{
		uint val1 = dataBuffer.Load(addrA + 4);
		uint val2 = dataBuffer.Load(addrA + 8);
		if (addrU != addrA)
		{
			uint val3 = dataBuffer.Load(addrA + 12);
			val0 = (val0 >> 16) | ((val1 & 0xFFFF) << 16);
			val1 = (val1 >> 16) | ((val2 & 0xFFFF) << 16);
			val2 = (val2 >> 16) | ((val3 & 0xFFFF) << 16);
		}
		res = float3(asfloat(val0), asfloat(val1), asfloat(val2));
	}
	else if (fmt == VECTOR_FMT_16)
	{
		uint val1 = dataBuffer.Load(addrA + 4);
		if (addrU != addrA)
		{
			val0 = (val0 >> 16) | ((val1 & 0xFFFF) << 16);
			val1 >>= 16;
		}
		res = DecodePacked_16_16_16(uint2(val0, val1));
	}
	else if (fmt == VECTOR_FMT_11)
	{
		uint val1 = dataBuffer.Load(addrA + 4);
		if (addrU != addrA)
		{
			val0 = (val0 >> 16) | ((val1 & 0xFFFF) << 16);
		}
		res = DecodePacked_11_10_11(val0);
	}
	else if (fmt == VECTOR_FMT_6)
	{
		if (addrU != addrA)
			val0 >>= 16;
		res = DecodePacked_6_5_5(val0);
	}
	return res;
}

float3 LoadSplatPosValue(uint index)
{
	uint fmt = /*$(Variable:_SplatFormat)*/ & 0xFF;
	uint stride = 0;
	if (fmt == VECTOR_FMT_32F)
		stride = 12;
	else if (fmt == VECTOR_FMT_16)
		stride = 6;
	else if (fmt == VECTOR_FMT_11)
		stride = 4;
	else if (fmt == VECTOR_FMT_6)
		stride = 2;
	return LoadAndDecodeVector(_SplatPos, index * stride, fmt);
}

static const uint kChunkSize = 256;

float3 LoadSplatPos(uint idx)
{
	float3 pos = LoadSplatPosValue(idx);
	uint chunkIdx = idx / kChunkSize;
	if (chunkIdx < /*$(Variable:_SplatChunkCount)*/)
	{
		Struct_GaussianSplatChunkInfo chunk = _SplatChunks[chunkIdx];
		float3 posMin = float3(chunk.posX.x, chunk.posY.x, chunk.posZ.x);
		float3 posMax = float3(chunk.posX.y, chunk.posY.y, chunk.posZ.y);
		pos = lerp(posMin, posMax, pos);
	}
	return pos;
}

// radix sort etc. friendly, see http://stereopsis.com/radix.html
uint FloatToSortableUint(float f)
{
	uint fu = asuint(f);
	uint mask = -((int) (fu >> 31)) | 0x80000000;
	return fu ^ mask;
}

/*$(_compute:)*/CSCalcDistances(uint3 DTid : SV_DispatchThreadID)
{
	uint idx = DTid.x;
	if (idx >= /*$(Variable:_SplatCount)*/)
		return;

	uint origIdx = _SplatSortKeys[idx];

	float3 pos = LoadSplatPos(origIdx);
	float4x4 mv = mul(/*$(Variable:_MatrixObjectToWorld)*/, /*$(Variable:ViewMtx)*/);
	pos = mul(float4(pos.xyz, 1), mv).xyz;

	_SplatSortDistances[idx] = FloatToSortableUint(pos.z);
}

/*
Shader Resources:
	Buffer _SplatSortDistances (as UAV)
	Buffer _SplatSortKeys (as SRV)
	Buffer _SplatPos (as SRV)
	Buffer _SplatChunks (as SRV)
*/
