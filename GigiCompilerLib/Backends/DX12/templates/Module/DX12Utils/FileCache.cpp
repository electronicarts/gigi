/*$(CopyrightHeader)*/#include "FileCache.h"

#include <filesystem>

std::unordered_map<std::string, DX12Utils::FileCache::File> DX12Utils::FileCache::s_cache;

namespace DX12Utils
{

FileCache::File& FileCache::Get(const char* fileName)
{
	// normalize the string by making it canonical and making it lower case
	std::string s = std::filesystem::weakly_canonical(fileName).string();
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });

	// If we don't have an entry for this file, create one
	// Add an extra null character at the end for text files.
	// We'll report the size as size-1 to account for this.
	if (s_cache.count(s) == 0)
	{
		File newFile;

		FILE* file = nullptr;
		fopen_s(&file, s.c_str(), "rb");
		if (file)
		{
			fseek(file, 0, SEEK_END);
			newFile.bytes.resize(ftell(file) + 1);
			fseek(file, 0, SEEK_SET);
			fread(newFile.bytes.data(), 1, newFile.bytes.size() - 1, file);
			newFile.bytes[newFile.bytes.size() - 1] = 0;
			fclose(file);
		}

		s_cache[s] = newFile;
	}

	// return the entry for this file
	return s_cache[s];
}

}; //namespace DX12Utils
