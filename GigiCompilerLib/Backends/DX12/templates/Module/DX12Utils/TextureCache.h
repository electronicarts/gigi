/*$(CopyrightHeader)*/#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "SRGB.h"

namespace DX12Utils
{

class TextureCache
{
public:
	enum class Type
	{
		U8,
		F32
	};

	struct Texture
	{
		bool Valid()
		{
			return pixels.size() > 0;
		}

		int SizeOfPixel() const
		{
			return channels * SizeOfChannel();
		}

		int SizeOfChannel() const
		{
			return (type == Type::F32) ? 4 : 1;
		}

		int width = 0;
		int height = 0;
		int channels = 0;
		Type type = Type::U8;
		std::vector<unsigned char> pixels;
		std::string fileName;
	};

	static Texture& Get(const char* fileName);

	static Texture GetAs(const char* fileName, bool sourceIsSRGB, Type desiredType, bool desiredSRGB, int desiredChannels = 0);

	static bool Remove(const char* fileName)
	{
		if (s_cache.count(fileName) == 0)
			return false;

		s_cache.erase(fileName);
		return true;
	}

	static void ClearCache()
	{
		std::unordered_map<std::string, Texture> empty;
		std::swap(s_cache, empty);
	}

private:
	TextureCache() {} // This is a static class

	static Texture s_invalidTexture;

private:
	static std::unordered_map<std::string, Texture> s_cache;
};

}; // namespace DX12Utils