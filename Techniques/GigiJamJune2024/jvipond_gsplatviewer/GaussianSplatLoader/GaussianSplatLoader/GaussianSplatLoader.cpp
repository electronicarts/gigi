#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>

#define GLM_FORCE_QUAT_DATA_XYZW
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"
#include "glm/gtc/type_precision.hpp"
#include "glm/gtc/packing.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

static constexpr std::uint32_t kTextureWidth = 2048;
static constexpr std::uint32_t kChunkSize = 256;

enum class VectorFormat
{
	Float32 = 0,
	Norm16 = 1,
	Norm11 = 2,
	Norm6 = 3,
};

std::size_t getVectorSize(VectorFormat fmt)
{
	switch (fmt)
	{
		case VectorFormat::Float32: return 12;
		case VectorFormat::Norm16: return 6;
		case VectorFormat::Norm11: return 4;
		case VectorFormat::Norm6: return 2;
	};

	__assume(0);
}

enum class ColorFormat
{
	Float32x4 = 0,
	Float16x4 = 1,
	Norm8x4 = 2,
};

std::size_t getColorSize(ColorFormat fmt)
{
	switch (fmt)
	{
		case ColorFormat::Float32x4: return 16;
		case ColorFormat::Float16x4: return 8;
		case ColorFormat::Norm8x4: return 4;
	};

	__assume(0);
}

std::size_t getOtherSize(VectorFormat scaleFmt)
{
	return 4 + getVectorSize(scaleFmt);
}

struct GaussianSplat
{
    glm::vec3 pos;
    glm::vec3 n;
	glm::vec3 dc0;
    glm::vec3 sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8, sh9, shA, shB, shC, shD, shE, shF;
    float opacity;
    glm::vec3 scale;
    glm::quat rot;
};

struct SHTableItemFloat32
{
	glm::vec3 sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8, sh9, shA, shB, shC, shD, shE, shF;
	glm::vec3 shPadding; // pad to multiple of 16 bytes
};

struct hvec3
{
	hvec3(const glm::vec3 v) : x(glm::packHalf1x16(v.x)), y(glm::packHalf1x16(v.y)), z(glm::packHalf1x16(v.z)) {}
	hvec3() : x(0), y(0), z(0) {}
	std::uint16_t x, y, z;
};

struct SHTableItemFloat16
{
	hvec3 sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8, sh9, shA, shB, shC, shD, shE, shF;
	hvec3 shPadding; // pad to multiple of 16 bytes
};

struct SHTableItemNorm11
{
	std::uint32_t sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8, sh9, shA, shB, shC, shD, shE, shF;
};

struct SHTableItemNorm6
{
	std::uint16_t sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8, sh9, shA, shB, shC, shD, shE, shF;
	std::uint16_t shPadding; // pad to multiple of 4 bytes
};

enum class ShFormat
{
	Float32 = 0,
	Float16 = 1,
	Norm11 = 2,
	Norm6 = 3,
};

std::size_t getShSize(ShFormat fmt)
{
	switch (fmt)
	{
		case ShFormat::Float32: return sizeof(SHTableItemFloat32);
		case ShFormat::Float16: return sizeof(SHTableItemFloat16);
		case ShFormat::Norm11: return sizeof(SHTableItemNorm11);
		case ShFormat::Norm6: return sizeof(SHTableItemNorm6);
	};

	__assume(0);
}

struct ChunkInfo
{
	std::uint32_t colR, colG, colB, colA;
	glm::vec2 posX, posY, posZ;
	std::uint32_t sclX, sclY, sclZ;
	std::uint32_t shR, shG, shB;
};

enum class Quality
{
	Medium = 0,
	High = 1,
	VeryHigh = 2,
};

void loadPly(const std::filesystem::path& filename, std::vector<GaussianSplat>& splats)
{
    std::ifstream infile(filename, std::ios_base::binary);

    if (!infile.good())
    {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return;
	}

	std::string buff;
	std::getline(infile, buff);
	std::getline(infile, buff);

	std::string dummy;
	std::getline(infile, buff);
	std::stringstream ss(buff);
	int count;
	ss >> dummy >> dummy >> count;

	while (std::getline(infile, buff))
		if (buff.compare("end_header") == 0)
			break;
	
	splats.resize(count);
	infile.read((char*)splats.data(), count * sizeof(GaussianSplat));
}

std::tuple<glm::vec3, glm::vec3> calculateBounds(const std::vector<GaussianSplat>& splats)
{
	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(std::numeric_limits<float>::lowest());
	for (const auto& splat : splats)
	{
		min = glm::min(min, splat.pos);
		max = glm::max(max, splat.pos);
	}
	return { min, max };
}

std::uint64_t mortonPart1By2(std::uint64_t x)
{
	x &= 0x1fffff;
	x = (x ^ (x << 32)) & 0x1f00000000ffffUL;
	x = (x ^ (x << 16)) & 0x1f0000ff0000ffUL;
	x = (x ^ (x << 8)) & 0x100f00f00f00f00fUL;
	x = (x ^ (x << 4)) & 0x10c30c30c30c30c3UL;
	x = (x ^ (x << 2)) & 0x1249249249249249UL;
	return x;
}

std::uint64_t mortonEncode(const glm::uvec3& v)
{
	return (mortonPart1By2(v.z) << 2) | (mortonPart1By2(v.y) << 1) | mortonPart1By2(v.x);
}

void reorderMorton(std::vector<GaussianSplat>& splats, const glm::vec3& boundsMin, const glm::vec3& boundsMax)
{
	static constinit float scaler = static_cast<float>((1 << 21) - 1);
	const glm::vec3 invBoundsSize = 1.0f / (boundsMax - boundsMin);
	const std::size_t numSplats = splats.size();
	std::vector<std::tuple<std::uint64_t, std::size_t>> order(numSplats);
	for (std::size_t i = 0; i < numSplats; i++)
	{
		const GaussianSplat& splat = splats[i];
		const glm::vec3 pos = (splat.pos - boundsMin) * invBoundsSize * scaler;
		const glm::uvec3 ipos = glm::uvec3(pos);
		const std::uint64_t code = mortonEncode(ipos);
		order[i] = { code, i };
	}

	std::sort(order.begin(), order.end(),
		[](const auto& a, const auto& b)
		{
			const auto& [code1, index1] = a;
			const auto& [code2, index2] = b;
			if (code1 < code2) return true;
			if (code1 > code2) return false;
			return index1 < index2;
		});

	std::vector<GaussianSplat> copy = splats;
	for (std::size_t i = 0; i < numSplats; i++)
		splats[i] = copy[std::get<1>(order[i])];
}

std::uint64_t encodeFloat3ToNorm16(const glm::vec3& v) // 48 bits: 16.16.16
{
	return (std::uint64_t)(v.x * 65535.5f) | ((std::uint64_t)(v.y * 65535.5f) << 16) | ((std::uint64_t)(v.z * 65535.5f) << 32);
}

std::uint32_t encodeFloat3ToNorm11(const glm::vec3& v) // 32 bits: 11.10.11
{
	return (std::uint32_t)(v.x * 2047.5f) | ((std::uint32_t)(v.y * 1023.5f) << 11) | ((std::uint32_t)(v.z * 2047.5f) << 21);
}

std::uint16_t encodeFloat3ToNorm655(const glm::vec3& v) // 16 bits: 6.5.5
{
	return (std::uint16_t)((std::uint32_t)(v.x * 63.5f) | ((std::uint32_t)(v.y * 31.5f) << 6) | ((std::uint32_t)(v.z * 31.5f) << 11));
}

std::uint16_t encodeFloat3ToNorm565(const glm::vec3& v) // 16 bits: 5.6.5
{
	return (std::uint16_t)((std::uint32_t)(v.x * 31.5f) | ((std::uint32_t)(v.y * 63.5f) << 5) | ((std::uint32_t)(v.z * 31.5f) << 11));
}

std::uint32_t encodeQuatToNorm10(const glm::quat& q) // 32 bits: 10.10.10.2
{
	return (std::uint32_t)(q.x * 1023.5f) | ((std::uint32_t)(q.y * 1023.5f) << 10) | ((std::uint32_t)(q.z * 1023.5f) << 20) | ((std::uint32_t)(q.w * 3.5f) << 30);
}

void emitEncodedVector(const glm::vec3& v, std::byte* out, VectorFormat fmt)
{
	switch (fmt)
	{
		case VectorFormat::Float32:
		{
			*(float*)out = v.x;
			*(float*)(out + 4) = v.y;
			*(float*)(out + 8) = v.z;
			break;
		}
		case VectorFormat::Norm16:
		{
			const std::uint64_t enc = encodeFloat3ToNorm16(glm::saturate(v));
			*(std::uint32_t*)out = (std::uint32_t)enc;
			*(std::uint16_t*)(out + 4) = (std::uint16_t)(enc >> 32);
			break;
		}
		case VectorFormat::Norm11:
		{
			const std::uint32_t enc = encodeFloat3ToNorm11(glm::saturate(v));
			*(std::uint32_t*)out = enc;
			break;
		}
		case VectorFormat::Norm6:
		{
			const std::uint16_t enc = encodeFloat3ToNorm655(glm::saturate(v));
			*(std::uint16_t*)out = enc;
			break;
		}
	}
}

float sign(float x)
{
	if (x > 0.0f) return 1.0f;
	if (x < 0.0f) return -1.0f;
	return 0.0f;
}

float squareCentered01(float x)
{
	x -= 0.5f;
	x *= x * sign(x);
	return x * 2.0f + 0.5f;
}

std::size_t createChunkData(std::vector<GaussianSplat>& splats, const std::filesystem::path& outputPath)
{
	const std::size_t numSplats = splats.size();
	const std::size_t numChunks = (numSplats + kChunkSize - 1) / kChunkSize;
	std::vector<ChunkInfo> chunks(numChunks);

	for (std::size_t chunkIndex = 0; chunkIndex < numChunks; chunkIndex++)
	{
		glm::vec3 chunkMinpos = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 chunkMinscl = glm::vec3(std::numeric_limits<float>::max());
		glm::vec4 chunkMincol = glm::vec4(std::numeric_limits<float>::max());
		glm::vec3 chunkMinshs = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 chunkMaxpos = glm::vec3(std::numeric_limits<float>::lowest());
		glm::vec3 chunkMaxscl = glm::vec3(std::numeric_limits<float>::lowest());
		glm::vec4 chunkMaxcol = glm::vec4(std::numeric_limits<float>::lowest());
		glm::vec3 chunkMaxshs = glm::vec3(std::numeric_limits<float>::lowest());

		const std::size_t splatBegin = std::min(chunkIndex * kChunkSize, numSplats);
		const std::size_t splatEnd = std::min((chunkIndex + 1) * kChunkSize, numSplats);

		for (std::size_t splatIndex = splatBegin; splatIndex < splatEnd; splatIndex++)
		{
			GaussianSplat& splat = splats[splatIndex];

			splat.scale = glm::pow(splat.scale, glm::vec3(1.0f / 8.0f));
			// transform opacity to be more unformly distributed
			splat.opacity = squareCentered01(splat.opacity);

			chunkMinpos = glm::min(chunkMinpos, splat.pos);
			chunkMinscl = glm::min(chunkMinscl, splat.scale);
			chunkMincol = glm::min(chunkMincol, glm::vec4(splat.dc0, splat.opacity));
			chunkMinshs = glm::min(chunkMinshs, splat.sh1);
			chunkMinshs = glm::min(chunkMinshs, splat.sh2);
			chunkMinshs = glm::min(chunkMinshs, splat.sh3);
			chunkMinshs = glm::min(chunkMinshs, splat.sh4);
			chunkMinshs = glm::min(chunkMinshs, splat.sh5);
			chunkMinshs = glm::min(chunkMinshs, splat.sh6);
			chunkMinshs = glm::min(chunkMinshs, splat.sh7);
			chunkMinshs = glm::min(chunkMinshs, splat.sh8);
			chunkMinshs = glm::min(chunkMinshs, splat.sh9);
			chunkMinshs = glm::min(chunkMinshs, splat.shA);
			chunkMinshs = glm::min(chunkMinshs, splat.shB);
			chunkMinshs = glm::min(chunkMinshs, splat.shC);
			chunkMinshs = glm::min(chunkMinshs, splat.shD);
			chunkMinshs = glm::min(chunkMinshs, splat.shE);
			chunkMinshs = glm::min(chunkMinshs, splat.shF);

			chunkMaxpos = glm::max(chunkMaxpos, splat.pos);
			chunkMaxscl = glm::max(chunkMaxscl, splat.scale);
			chunkMaxcol = glm::max(chunkMaxcol, glm::vec4(splat.dc0, splat.opacity));
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh1);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh2);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh3);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh4);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh5);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh6);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh7);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh8);
			chunkMaxshs = glm::max(chunkMaxshs, splat.sh9);
			chunkMaxshs = glm::max(chunkMaxshs, splat.shA);
			chunkMaxshs = glm::max(chunkMaxshs, splat.shB);
			chunkMaxshs = glm::max(chunkMaxshs, splat.shC);
			chunkMaxshs = glm::max(chunkMaxshs, splat.shD);
			chunkMaxshs = glm::max(chunkMaxshs, splat.shE);
			chunkMaxshs = glm::max(chunkMaxshs, splat.shF);
		}

		chunkMaxpos = glm::max(chunkMaxpos, chunkMinpos + 1.0e-5f);
		chunkMaxscl = glm::max(chunkMaxscl, chunkMinscl + 1.0e-5f);
		chunkMaxcol = glm::max(chunkMaxcol, chunkMincol + 1.0e-5f);
		chunkMaxshs = glm::max(chunkMaxshs, chunkMinshs + 1.0e-5f);

		// store chunk info
		ChunkInfo& info = chunks[chunkIndex];
		info.posX = glm::vec2(chunkMinpos.x, chunkMaxpos.x);
		info.posY = glm::vec2(chunkMinpos.y, chunkMaxpos.y);
		info.posZ = glm::vec2(chunkMinpos.z, chunkMaxpos.z);
		info.sclX = glm::packHalf1x16(chunkMinscl.x) | (glm::packHalf1x16(chunkMaxscl.x) << 16);
		info.sclY = glm::packHalf1x16(chunkMinscl.y) | (glm::packHalf1x16(chunkMaxscl.y) << 16);
		info.sclZ = glm::packHalf1x16(chunkMinscl.z) | (glm::packHalf1x16(chunkMaxscl.z) << 16);
		info.colR = glm::packHalf1x16(chunkMincol.x) | (glm::packHalf1x16(chunkMaxcol.x) << 16);
		info.colG = glm::packHalf1x16(chunkMincol.y) | (glm::packHalf1x16(chunkMaxcol.y) << 16);
		info.colB = glm::packHalf1x16(chunkMincol.z) | (glm::packHalf1x16(chunkMaxcol.z) << 16);
		info.colA = glm::packHalf1x16(chunkMincol.w) | (glm::packHalf1x16(chunkMaxcol.w) << 16);
		info.shR = glm::packHalf1x16(chunkMinshs.x) | (glm::packHalf1x16(chunkMaxshs.x) << 16);
		info.shG = glm::packHalf1x16(chunkMinshs.y) | (glm::packHalf1x16(chunkMaxshs.y) << 16);
		info.shB = glm::packHalf1x16(chunkMinshs.z) | (glm::packHalf1x16(chunkMaxshs.z) << 16);

		for (std::size_t splatIndex = splatBegin; splatIndex < splatEnd; splatIndex++)
		{
			GaussianSplat& splat = splats[splatIndex];
			splat.pos = (splat.pos - chunkMinpos) / (chunkMaxpos - chunkMinpos);
			splat.scale = (splat.scale - chunkMinscl) / (chunkMaxscl - chunkMinscl);
			splat.dc0 = (splat.dc0 - glm::vec3(chunkMincol)) / (glm::vec3(chunkMaxcol) - glm::vec3(chunkMincol));
			splat.opacity = (splat.opacity - chunkMincol.w) / (chunkMaxcol.w - chunkMincol.w);
			splat.sh1 = (splat.sh1 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh2 = (splat.sh2 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh3 = (splat.sh3 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh4 = (splat.sh4 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh5 = (splat.sh5 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh6 = (splat.sh6 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh7 = (splat.sh7 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh8 = (splat.sh8 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.sh9 = (splat.sh9 - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.shA = (splat.shA - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.shB = (splat.shB - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.shC = (splat.shC - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.shD = (splat.shD - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.shE = (splat.shE - chunkMinshs) / (chunkMaxshs - chunkMinshs);
			splat.shF = (splat.shF - chunkMinshs) / (chunkMaxshs - chunkMinshs);
		}
	}

	std::fstream f(outputPath, std::ios::binary | std::ios::out | std::ios::trunc);
	f.write((char*)chunks.data(), chunks.size() * sizeof(ChunkInfo));
	f.close();

	return numChunks;
}

void createPositionData(const std::vector<GaussianSplat>& splats, const std::filesystem::path& outputPath, VectorFormat fmt)
{
	const std::size_t vectorSize = getVectorSize(fmt);
	std::vector<std::byte> output(splats.size() * vectorSize);
	for (std::size_t i = 0; i < splats.size(); i++)
		emitEncodedVector(splats[i].pos, output.data() + i * vectorSize, fmt);

	std::fstream f(outputPath, std::ios::binary | std::ios::out | std::ios::trunc);
	f.write((char*)output.data(), output.size());
	f.close();
}

void createOtherData(const std::vector<GaussianSplat>& splats, const std::filesystem::path& outputPath, VectorFormat scaleFmt)
{
	const std::size_t otherSize = getOtherSize(scaleFmt);
	std::vector<std::byte> output(splats.size() * otherSize);
	for (std::size_t i = 0; i < splats.size(); i++)
	{
		std::byte* out = output.data() + i * otherSize;
		const std::uint32_t enc = encodeQuatToNorm10(splats[i].rot);
		*(std::uint32_t*)out = enc;
		out += sizeof(enc);
		emitEncodedVector(splats[i].scale, out, scaleFmt);
	}

	std::fstream f(outputPath, std::ios::binary | std::ios::out | std::ios::trunc);
	f.write((char*)output.data(), output.size());
	f.close();
}

std::tuple<std::uint32_t, std::uint32_t> calcTextureSize(std::uint32_t splatCount)
{
	std::uint32_t height = std::max(1u, (splatCount + kTextureWidth - 1) / kTextureWidth);
	static constexpr std::uint32_t blockHeight = 16;
	height = (height + blockHeight - 1) / blockHeight * blockHeight;
	return { kTextureWidth, height };
}

glm::uvec2 decodeMorton2D_16x16(std::uint32_t t)
{
	t = (t & 0xFF) | ((t & 0xFE) << 7); // -EAFBGCHEAFBGCHD
	t &= 0x5555;                        // -E-F-G-H-A-B-C-D
	t = (t ^ (t >> 1)) & 0x3333;        // --EF--GH--AB--CD
	t = (t ^ (t >> 2)) & 0x0f0f;        // ----EFGH----ABCD
	return { t & 0xF, t >> 8 };			// --------EFGHABCD
}

std::uint32_t splatIndexToTextureIndex(std::uint32_t i)
{
	const glm::uvec2 xy = decodeMorton2D_16x16(i);
	const std::uint32_t width = kTextureWidth / 16;
	i >>= 8;
	const std::uint32_t x = (i % width) * 16 + xy.x;
	const std::uint32_t y = (i / width) * 16 + xy.y;
	return y * kTextureWidth + x;
}

std::tuple<std::size_t, std::size_t> createColorData(const std::vector<GaussianSplat>& splats, const std::filesystem::path& outputPath, ColorFormat fmt)
{
	const std::size_t numSplats = splats.size();
	const auto [width, height] = calcTextureSize(static_cast<uint32_t>(numSplats));
	const std::size_t textureSize = width * height;
	std::vector<glm::vec4> data(textureSize);

	for (std::size_t i = 0; i < numSplats; i++)
	{
		const GaussianSplat& splat = splats[i];
		const size_t textureIndex = splatIndexToTextureIndex(static_cast<uint32_t>(i));
		data[textureIndex] = glm::vec4(splat.dc0, splat.opacity);
	}

	const std::size_t colorSize = getColorSize(fmt);
	std::size_t dstSize = width * height * colorSize;
	std::vector<std::byte> output(dstSize);

	std::byte* dst = output.data();
	
	for (std::size_t y = 0; y < height; y++)
	{
		std::size_t srcIndex = y * width;
		for (std::size_t x = 0; x < width; x++)
		{
			glm::vec4 pix = data[srcIndex];

			switch (fmt)
			{
			case ColorFormat::Float32x4:
				*(glm::vec4*)dst = pix;
				break;
			case ColorFormat::Float16x4:
			{
				const std::uint64_t enc = glm::packHalf4x16(pix);
				*(std::uint64_t*)dst = enc;
				break;
			}
			case ColorFormat::Norm8x4:
			{
				pix = glm::saturate(pix);
				std::uint32_t enc = (std::uint32_t)(pix.x * 255.5f) | ((std::uint32_t)(pix.y * 255.5f) << 8) | ((std::uint32_t)(pix.z * 255.5f) << 16) | ((std::uint32_t)(pix.w * 255.5f) << 24);
				*(std::uint32_t*)dst = enc;
				break;
			}
			}

			srcIndex++;
			dst += colorSize;
		}
	}

	std::fstream f(outputPath, std::ios::binary | std::ios::out | std::ios::trunc);
	f.write((char*)output.data(), output.size());
	f.close();

	return { width, height };
}

void createShData(const std::vector<GaussianSplat>& splats, const std::filesystem::path& outputPath, ShFormat fmt)
{
	const std::size_t numSplats = splats.size();
	const std::size_t shSize = getShSize(fmt);
	std::vector<std::byte> output(numSplats * shSize);
	std::byte* dst = output.data();

	for (std::size_t i = 0; i < numSplats; i++)
	{
		const GaussianSplat& splat = splats[i];
		switch (fmt)
		{
			case ShFormat::Float32:
			{
				SHTableItemFloat32 res;
				res.sh1 = splat.sh1;
				res.sh2 = splat.sh2;
				res.sh3 = splat.sh3;
				res.sh4 = splat.sh4;
				res.sh5 = splat.sh5;
				res.sh6 = splat.sh6;
				res.sh7 = splat.sh7;
				res.sh8 = splat.sh8;
				res.sh9 = splat.sh9;
				res.shA = splat.shA;
				res.shB = splat.shB;
				res.shC = splat.shC;
				res.shD = splat.shD;
				res.shE = splat.shE;
				res.shF = splat.shF;
				res.shPadding = glm::vec3(0.0f);
				((SHTableItemFloat32*)dst)[i] = res;
				break;
			}
			case ShFormat::Float16:
			{
				SHTableItemFloat16 res;
				res.sh1 = hvec3(splat.sh1);
				res.sh2 = hvec3(splat.sh2);
				res.sh3 = hvec3(splat.sh3);
				res.sh4 = hvec3(splat.sh4);
				res.sh5 = hvec3(splat.sh5);
				res.sh6 = hvec3(splat.sh6);
				res.sh7 = hvec3(splat.sh7);
				res.sh8 = hvec3(splat.sh8);
				res.sh9 = hvec3(splat.sh9);
				res.shA = hvec3(splat.shA);
				res.shB = hvec3(splat.shB);
				res.shC = hvec3(splat.shC);
				res.shD = hvec3(splat.shD);
				res.shE = hvec3(splat.shE);
				res.shF = hvec3(splat.shF);
				res.shPadding = hvec3(glm::vec3(0.0f));
				((SHTableItemFloat16*)dst)[i] = res;
				break;
			}
			case ShFormat::Norm11:
			{
				SHTableItemNorm11 res;
				res.sh1 = encodeFloat3ToNorm11(splat.sh1);
				res.sh2 = encodeFloat3ToNorm11(splat.sh2);
				res.sh3 = encodeFloat3ToNorm11(splat.sh3);
				res.sh4 = encodeFloat3ToNorm11(splat.sh4);
				res.sh5 = encodeFloat3ToNorm11(splat.sh5);
				res.sh6 = encodeFloat3ToNorm11(splat.sh6);
				res.sh7 = encodeFloat3ToNorm11(splat.sh7);
				res.sh8 = encodeFloat3ToNorm11(splat.sh8);
				res.sh9 = encodeFloat3ToNorm11(splat.sh9);
				res.shA = encodeFloat3ToNorm11(splat.shA);
				res.shB = encodeFloat3ToNorm11(splat.shB);
				res.shC = encodeFloat3ToNorm11(splat.shC);
				res.shD = encodeFloat3ToNorm11(splat.shD);
				res.shE = encodeFloat3ToNorm11(splat.shE);
				res.shF = encodeFloat3ToNorm11(splat.shF);
				((SHTableItemNorm11*)dst)[i] = res;
				break;
			}
			case ShFormat::Norm6:
			{
				SHTableItemNorm6 res;
				res.sh1 = encodeFloat3ToNorm565(splat.sh1);
				res.sh2 = encodeFloat3ToNorm565(splat.sh2);
				res.sh3 = encodeFloat3ToNorm565(splat.sh3);
				res.sh4 = encodeFloat3ToNorm565(splat.sh4);
				res.sh5 = encodeFloat3ToNorm565(splat.sh5);
				res.sh6 = encodeFloat3ToNorm565(splat.sh6);
				res.sh7 = encodeFloat3ToNorm565(splat.sh7);
				res.sh8 = encodeFloat3ToNorm565(splat.sh8);
				res.sh9 = encodeFloat3ToNorm565(splat.sh9);
				res.shA = encodeFloat3ToNorm565(splat.shA);
				res.shB = encodeFloat3ToNorm565(splat.shB);
				res.shC = encodeFloat3ToNorm565(splat.shC);
				res.shD = encodeFloat3ToNorm565(splat.shD);
				res.shE = encodeFloat3ToNorm565(splat.shE);
				res.shF = encodeFloat3ToNorm565(splat.shF);
				res.shPadding = 0;
				((SHTableItemNorm6*)dst)[i] = res;
				break;
			}
		}
	}

	std::fstream f(outputPath, std::ios::binary | std::ios::out | std::ios::trunc);
	f.write((char*)output.data(), output.size());
	f.close();
}

void reorderShs(std::vector<GaussianSplat>& splats)
{
	const std::size_t splatStride = sizeof(GaussianSplat) / 4;
	constexpr std::size_t shStartOffset = 9, shCount = 15;
	std::array<float, shCount * 3> tmp;
	std::size_t idx = shStartOffset;
	std::size_t numSplats = splats.size();
	float* data = (float*)splats.data();
	for (int i = 0; i < numSplats; ++i)
	{
		for (int j = 0; j < shCount; ++j)
		{
			tmp[j * 3 + 0] = data[idx + j];
			tmp[j * 3 + 1] = data[idx + j + shCount];
			tmp[j * 3 + 2] = data[idx + j + shCount * 2];
		}

		for (int j = 0; j < shCount * 3; ++j)
		{
			data[idx + j] = tmp[j];
		}

		idx += splatStride;
	}
}

glm::vec4 normalizeSwizzleRotation(const glm::float4& wxyz)
{
	glm::vec4 wxyzNorm = glm::normalize(wxyz);
	return glm::vec4(wxyzNorm.y, wxyzNorm.z, wxyzNorm.w, wxyzNorm.x);
}

glm::vec4 packSmallest3Rotation(glm::vec4 q)
{
	// find biggest component
	glm::vec4 absQ = glm::abs(q);
	int index = 0;
	float maxV = absQ.x;
	if (absQ.y > maxV)
	{
		index = 1;
		maxV = absQ.y;
	}
	if (absQ.z > maxV)
	{
		index = 2;
		maxV = absQ.z;
	}
	if (absQ.w > maxV)
	{
		index = 3;
		maxV = absQ.w;
	}

	if (index == 0) q = glm::vec4(q.y, q.z, q.w, q.x);// q.yzwx;
	if (index == 1) q = glm::vec4(q.x, q.z, q.w, q.y);// q.xzwy;
	if (index == 2) q = glm::vec4(q.x, q.y, q.w, q.z);// q.xywz;

	static const float kSQRT2 = 1.41421356237f;
	glm::vec3 three = glm::vec3(q) * (q.w >= 0.0f ? 1.0f : -1.0f); // -1/sqrt2..+1/sqrt2 range
	three = (three * kSQRT2) * 0.5f + 0.5f; // 0..1 range

	return glm::vec4(three, index / 3.0f);
}

glm::vec3 linearScale(const glm::vec3& logScale)
{
	return glm::abs(glm::exp(logScale));
}

float sigmoid(float v)
{
	return 1.0f / (1.0f + glm::exp(-v));
}

glm::vec3 sh0ToColor(const glm::vec3& dc0)
{
	const float kSH_C0 = 0.2820948f;
	return dc0 * kSH_C0 + 0.5f;
}

void linearizeData(std::vector<GaussianSplat>& splats)
{
	for (GaussianSplat& splat : splats)
	{
		const glm::quat& q = splat.rot;
		glm::float4 qq = normalizeSwizzleRotation(glm::float4(q.x, q.y, q.z, q.w));
		qq = packSmallest3Rotation(qq);
		splat.rot = glm::quat(qq.x, qq.y, qq.z, qq.w);

		// scale
		splat.scale = linearScale(splat.scale);

		// color
		splat.dc0 = sh0ToColor(splat.dc0);
		splat.opacity = sigmoid(splat.opacity);
	}
}

struct ProcessedSplatsResult
{
	std::size_t numSplats;
	std::size_t numChunks;
	std::uint32_t splatFormat;
	std::uint32_t colorFormat;
	std::size_t colorWidth;
	std::size_t colorHeight;
};

struct OutputPaths
{
	OutputPaths(const std::filesystem::path& outDir)
		: pos(outDir / "pos.bytes"),
		other(outDir / "oth.bytes"),
		chunks(outDir / "chunks.bytes"),
		sh(outDir / "sh.bytes"),
		color(outDir / "col.bytes")
	{}
	std::filesystem::path pos;
	std::filesystem::path other;
	std::filesystem::path chunks;
	std::filesystem::path sh;
	std::filesystem::path color;
};

ProcessedSplatsResult processSplats(std::vector<GaussianSplat>& splats, const OutputPaths& outputPaths, VectorFormat formatPos, VectorFormat formatScale, ColorFormat formatColor, ShFormat formatSh)
{
	reorderShs(splats);
	const auto& [min, max] = calculateBounds(splats);
	reorderMorton(splats, min, max);
	linearizeData(splats);
	const bool usingChunks = formatPos != VectorFormat::Float32 || formatScale != VectorFormat::Float32 || formatColor != ColorFormat::Float32x4 || formatSh != ShFormat::Float32;
	const std::size_t numChunks = usingChunks ? createChunkData(splats, outputPaths.chunks) : 0;
	createPositionData(splats, outputPaths.pos, formatPos);
	createOtherData(splats, outputPaths.other, formatScale);
	const auto [colorWidth, colorHeight] = createColorData(splats, outputPaths.color, formatColor);
	createShData(splats, outputPaths.sh, formatSh);

	return { splats.size(), numChunks, (std::uint32_t)formatPos | ((std::uint32_t)formatScale << 8) | ((std::uint32_t)formatSh << 16), (std::uint32_t)formatColor, colorWidth, colorHeight };
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "you need to specify a ply file" << std::endl;
		std::cerr << "example usage GaussianSplatLoader.exe example.ply" << std::endl;
		std::exit(-1);
	}

	std::vector<GaussianSplat> splats;
	std::filesystem::path path = argv[1];
	loadPly(path, splats);

	std::filesystem::path outDir = path.parent_path();
	OutputPaths outputPaths(outDir);

	const ProcessedSplatsResult r = processSplats(splats, outputPaths, VectorFormat::Float32, VectorFormat::Float32, ColorFormat::Float32x4, ShFormat::Float32);
	std::cout << "SplatCount = " << r.numSplats << std::endl;
	std::cout << "SplatChunkCount = " << r.numChunks << std::endl;
	std::cout << "SplatFormat = " << r.splatFormat << std::endl;
	std::cout << "ColorFormat = " << r.colorFormat << std::endl;
	std::cout << "ColorWidth = " << r.colorWidth << std::endl;
	std::cout << "ColorHeight = " << r.colorHeight << std::endl;

	std::cout << "PosData = " << outputPaths.pos.string() << std::endl;
	std::cout << "OtherData = " << outputPaths.other.string() << std::endl;
	std::cout << "ChunkData = " << outputPaths.chunks.string() << std::endl;
	std::cout << "ShData = " << outputPaths.sh.string() << std::endl;
	std::cout << "ColorData = " << outputPaths.color.string() << std::endl;

    return 0;
}
