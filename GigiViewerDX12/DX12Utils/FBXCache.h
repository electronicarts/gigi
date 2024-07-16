///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include <vector>
#include <unordered_map>
#include <string>
#include <array>

#include "FileCache.h"
#include "FlattenedVertex.h"
// clang-format on

class FBXCache
{
public:

	struct FBXData
	{
		std::string warn, error;
		std::vector<FlattenedVertex> flattenedVertices;
		bool valid = false;
	};

	FBXData& Get(FileCache& fileCache, const char* fileName);

	bool Remove(const char* fileName)
	{
		if (m_cache.count(fileName) == 0)
			return false;

		m_cache.erase(fileName);
		return true;
	}

	void ClearCache()
	{
		std::unordered_map<std::string, FBXData> empty;
		std::swap(m_cache, empty);
	}

	std::unordered_map<std::string, FBXData> getCache()
	{
		return m_cache;
	}

private:
	std::unordered_map<std::string, FBXData> m_cache;
};