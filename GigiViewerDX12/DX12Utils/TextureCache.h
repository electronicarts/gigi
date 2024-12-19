///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "FileCache.h"

enum DXGI_FORMAT;

class TextureCache
{
public:

	// A texture may contain multiple images
	struct Image
	{
		std::vector<unsigned char> pixels;
	};

	struct Texture
	{
		int width = 0;
		int height = 0;
		int depth = 1;  // Or array count
		int mips = 1;
		DXGI_FORMAT format = (DXGI_FORMAT)0; // DXGI_FORMAT_UNKNOWN
		std::string fileName;

		// Layout of image types:
		// 2D - depth is 1.  images.size() == mips.  mip 0 then mip 1 then mip 2 etc.
		// 3D - depth is variable.  all slize of depth 0 as mip 0, then all slices of depth 1 as mip 1, etc. each mip has half as many slices of depth.
		std::vector<Image> images;

		bool Valid() const
		{
			return images.size() > 0;
		}

		size_t PixelCount() const
		{
			size_t ret = 0;
			for (const Image& image : images)
				ret += image.pixels.size();
			return ret;
		}

		// Meant to be used with volume textures
		void Ensure3D()
		{
			if (images.size() == mips)
				return;

			int mipDepth = depth;
			int destTextureIndex = 0;
			int srcTextureIndex = 0;
			for (int mipIndex = 0; mipIndex < mips; ++mipIndex)
			{
				for (int iz = 0; iz < mipDepth; ++iz)
				{
					if (srcTextureIndex != 0)
					{
						Image& src = images[srcTextureIndex];
						Image& dest = images[destTextureIndex];

						size_t copyOffset = dest.pixels.size();
						size_t copySize = src.pixels.size();

						dest.pixels.resize(copyOffset + copySize);

						memcpy(&dest.pixels[copyOffset], &src.pixels[0], copySize);

						src.pixels.clear();
					}

					srcTextureIndex++;
				}

				mipDepth /= 2;
				if (mipDepth < 1)
					mipDepth = 1;
				destTextureIndex++;
			}

			images.resize(mips);
		}

		// Meant to be used with 2d texture arrays (aka not volume textures)
		template <typename LAMBDA>
		bool SplitByIndex(const LAMBDA& lambda) const
		{
			if (depth == 1)
			{
				lambda(*this);
				return true;
			}

			Texture newTexture;
			newTexture.width = width;
			newTexture.height = height;
			newTexture.mips = mips;
			newTexture.format = format;
			newTexture.fileName = fileName;

			newTexture.depth = 1;
			newTexture.images.resize(newTexture.mips);

			int arrayCount = depth;
			int mipCount = mips;
			int depthCount = 1;

			for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
			{
				for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
				{
					int srcImageIndex = mipIndex + arrayIndex * mipCount;
					if (srcImageIndex >= images.size())
						return false;
					newTexture.images[mipIndex] = images[srcImageIndex];
				}

				lambda(newTexture);
			}

			return true;
		}
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
