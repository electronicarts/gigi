///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <unordered_map>
#include <string>

class FileCache
{
public:
	struct File
	{
		bool Valid()
		{
			return bytes.size() > 0;
		}

		size_t GetSize() const
		{
			// Note that we add an extra null character on the end for text files, so size is really size-1
			size_t ret = bytes.size();
			if (ret > 0)
				ret--;
			return ret;
		}

		const char* GetBytes() const
		{
			return bytes.data();
		}

	private:
		friend class FileCache;

		// Hidden away because we add an extra character for a null terminator for text files
		std::vector<char> bytes;
	};

	File& Get(const char* fileName);

	bool Remove(const char* fileName)
	{
		if (m_cache.count(fileName) == 0)
			return false;

		m_cache.erase(fileName);
		return true;
	}

	void ClearCache()
	{
		std::unordered_map<std::string, File> empty;
		std::swap(m_cache, empty);
	}

	std::unordered_map<std::string, File> getCache()
	{
		return m_cache;
	}

private:
	std::unordered_map<std::string, File> m_cache;
};