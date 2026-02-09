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
// clang-format on

class PLYCache
{
public:

	enum class FieldType
	{
		unknown,
		i8,
		u8,
		i16,
		u16,
		i32,
		u32,
		f32,
		f64,
	};

	// property structure for E.g. vertices and indices
	struct Property
	{
		std::string name;

		// The type of data stored
		FieldType type = FieldType::unknown;

		// Only used if a list
		bool isList = false;
		FieldType listSizeType = FieldType::unknown;
	};

	// A description of data, and a bunch of data
	struct ElementGroup
	{
		std::string name;
		std::vector<Property> properties;
		unsigned int count = 0;
		std::vector<unsigned char> data;
		size_t propertiesSizeBytes = 0; // sum of non list property sizes
	};

	struct PLYData
	{
		std::string warn, error;
		std::vector<ElementGroup> elementGroups;
		bool valid = false;
	};

	PLYData& Get(FileCache& fileCache, const char* fileName);

	PLYData GetFlattened(FileCache& fileCache, const char* fileName);

	bool Remove(const char* fileName)
	{
		if (m_cache.count(fileName) == 0)
			return false;

		m_cache.erase(fileName);
		return true;
	}

	void ClearCache()
	{
		std::unordered_map<std::string, PLYData> empty;
		std::swap(m_cache, empty);
	}

	std::unordered_map<std::string, PLYData> getCache()
	{
		return m_cache;
	}

	// utils
public:
	template <typename T>
	static const unsigned char* ReadFromBinaryAndCastTo(const unsigned char* srcPointer, PLYCache::FieldType type, T& out)
	{
		switch (type)
		{
			case PLYCache::FieldType::i8:
			{
				int8_t val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				break;
			}
			case PLYCache::FieldType::u8:
			{
				uint8_t val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				out = (T)val;
				break;
			}
			case PLYCache::FieldType::i16:
			{
				int16_t val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				out = (T)val;
				break;
			}
			case PLYCache::FieldType::u16:
			{
				uint16_t val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				out = (T)val;
				break;
			}
			case PLYCache::FieldType::i32:
			{
				int32_t val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				out = (T)val;
				break;
			}
			case PLYCache::FieldType::u32:
			{
				uint32_t val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				out = (T)val;
				break;
			}
			case PLYCache::FieldType::f32:
			{
				float val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				out = (T)val;
				break;
			}
			case PLYCache::FieldType::f64:
			{
				double val;
				memcpy(&val, srcPointer, sizeof(val));
				srcPointer += sizeof(val);
				out = (T)val;
				break;
			}
			default: return nullptr;
		}

		return srcPointer;
	}

private:
	std::unordered_map<std::string, PLYData> m_cache;
};