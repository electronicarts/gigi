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
#include "tinyobjloader/tiny_obj_loader.h"
#include "FlattenedVertex.h"
// clang-format on

class ObjCache
{
public:
	struct OBJData
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, error;

		std::vector<FlattenedVertex> flattenedVertices;

		bool valid = false;
	};

	OBJData& Get(FileCache& fileCache, const char* fileName);

	bool Remove(const char* fileName)
	{
		if (m_cache.count(fileName) == 0)
			return false;

		m_cache.erase(fileName);
		return true;
	}

	void ClearCache()
	{
		std::unordered_map<std::string, OBJData> empty;
		std::swap(m_cache, empty);
	}

	std::unordered_map<std::string, OBJData> getCache()
	{
		return m_cache;
	}

private:
	std::unordered_map<std::string, OBJData> m_cache;
};