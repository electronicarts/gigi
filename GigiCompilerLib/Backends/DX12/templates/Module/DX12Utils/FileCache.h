/*$(CopyrightHeader)*/#pragma once

#include <vector>
#include <unordered_map>
#include <string>

namespace DX12Utils
{

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

	static File& Get(const char* fileName);

	static bool Remove(const char* fileName)
	{
		if (s_cache.count(fileName) == 0)
			return false;

		s_cache.erase(fileName);
		return true;
	}

	static void ClearCache()
	{
		std::unordered_map<std::string, File> empty;
		std::swap(s_cache, empty);
	}

private:
	FileCache() {} // This is a static class

private:
	static std::unordered_map<std::string, File> s_cache;
};

}; //namespace DX12Utils
