///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "FileCache.h"

class TextureCache
{
public:
	enum class Type
	{
		U8,
		F32,
		BC7
	};

	struct Texture
	{
		bool Valid()
		{
			return pixels.size() > 0;
		}

		int width = 0;
		int height = 0;
		int channels = 0;
		Type type = Type::U8;
		std::vector<unsigned char> pixels;
		std::string fileName;
	};

	Texture& Get(FileCache& fileCache, const char* fileName);

	bool Remove(const char* fileName)
	{
		if (m_cache.count(fileName) == 0)
			return false;

		m_cache.erase(fileName);
		return true;
	}

	void ClearCache()
	{
		std::unordered_map<std::string, Texture> empty;
		std::swap(m_cache, empty);
	}

	std::unordered_map<std::string, Texture> getCache()
	{
		return m_cache;
	}

private:
	std::unordered_map<std::string, Texture> m_cache;
};