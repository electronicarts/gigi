/*$(CopyrightHeader)*/#include "TextureCache.h"
#include "FileCache.h"

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

std::unordered_map<std::string, DX12Utils::TextureCache::Texture> DX12Utils::TextureCache::s_cache;

namespace DX12Utils
{

TextureCache::Texture TextureCache::s_invalidTexture;

TextureCache::Texture& TextureCache::Get(const char* fileName_)
{
	// normalize the string by making it canonical and making it lower case
	std::filesystem::path p = std::filesystem::weakly_canonical(fileName_);
	std::string s = p.string();
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	const char* fileName = s.c_str();

	// If we don't have an entry for this file, create one
	if (s_cache.count(fileName) == 0)
	{
		FileCache::File fileData = FileCache::Get(s.c_str());

		Texture newTexture;
		newTexture.fileName = fileName;

		if (p.extension().string() == ".exr")
		{
			// 1. Read EXR version.
			EXRVersion exr_version;
			if (ParseEXRVersionFromMemory(&exr_version, (const unsigned char*)fileData.GetBytes(), fileData.GetSize()) >= 0 && !exr_version.multipart)
			{
				// 2. Read EXR header
				EXRHeader exr_header;
				InitEXRHeader(&exr_header);

				const char* err = nullptr;
				if (ParseEXRHeaderFromMemory(&exr_header, &exr_version, (const unsigned char*)fileData.GetBytes(), fileData.GetSize(), &err) >= 0)
				{
					EXRImage exr_image;
					InitEXRImage(&exr_image);

					if (LoadEXRImageFromMemory(&exr_image, &exr_header, (const unsigned char*)fileData.GetBytes(), fileData.GetSize(), &err) >= 0)
					{
						newTexture.width = exr_image.width;
						newTexture.height = exr_image.height;
						newTexture.channels = exr_image.num_channels;
						newTexture.type = Type::F32;

						// find out what order to get the channels in
						bool validChannelTypes = true;
						std::vector<int> channelOrder(newTexture.channels);
						for (int i = 0; i < newTexture.channels; ++i)
						{
							if (exr_header.channels[i].pixel_type != TINYEXR_PIXELTYPE_FLOAT) // only supporting floating point texture types for now
								validChannelTypes = false;

							if (!_stricmp(exr_header.channels[i].name, "R"))
								channelOrder[i] = 0;
							else if (!_stricmp(exr_header.channels[i].name, "G"))
								channelOrder[i] = 1;
							else if (!_stricmp(exr_header.channels[i].name, "B"))
								channelOrder[i] = 2;
							else if (!_stricmp(exr_header.channels[i].name, "A"))
								channelOrder[i] = 3;
							else
								channelOrder[i] = i;
						}

						// 3. Access image data
						// `exr_image.images` will be filled when EXR is scanline format.
						// `exr_image.tiled` will be filled when EXR is tiled format.
						if (validChannelTypes)
						{
							newTexture.pixels.resize(newTexture.width * newTexture.height * newTexture.channels * sizeof(float));

							if (exr_image.images)
							{
								float** channelPtrs = (float**)exr_image.images;
								std::vector<float*> src(newTexture.channels);
								for (int i = 0; i < newTexture.channels; ++i)
									src[i] = channelPtrs[i];

								float* dest = (float*)newTexture.pixels.data();
								for (int iy = 0; iy < newTexture.height; ++iy)
								{
									for (int ix = 0; ix < newTexture.width; ++ix)
									{
										for (int channel = 0; channel < newTexture.channels; ++channel)
										{
											dest[channelOrder[channel]] = *src[channel];
											src[channel]++;
										}
										dest += newTexture.channels;
									}
								}
							}

							// i don't have a tiled exr image to test, so not writing that code yet
						}
						else
						{
							newTexture.width = 0;
							newTexture.height = 0;
							newTexture.channels = 0;
						}

						// 4. Free image data
						FreeEXRImage(&exr_image);
						FreeEXRHeader(&exr_header);
					}
					else
					{
						FreeEXRErrorMessage(err);
					}
				}
				else
				{
					FreeEXRErrorMessage(err);
				}
			}
		}
		else if (p.extension().string() == ".hdr")
		{
			float* rawPixels = stbi_loadf_from_memory((const unsigned char*)fileData.GetBytes(), (int)fileData.GetSize(), &newTexture.width, &newTexture.height, &newTexture.channels, 0);
			if (rawPixels)
			{
				newTexture.type = Type::F32;
				newTexture.pixels.resize(newTexture.width * newTexture.height * newTexture.channels * sizeof(float));
				memcpy(newTexture.pixels.data(), rawPixels, newTexture.pixels.size());
				stbi_image_free(rawPixels);
			}
		}
		else
		{
			unsigned char* rawPixels = stbi_load_from_memory((const unsigned char*)fileData.GetBytes(), (int)fileData.GetSize(), &newTexture.width, &newTexture.height, &newTexture.channels, 0);
			if (rawPixels)
			{
				newTexture.pixels.resize(newTexture.width * newTexture.height * newTexture.channels);
				memcpy(newTexture.pixels.data(), rawPixels, newTexture.pixels.size());
				stbi_image_free(rawPixels);
			}
		}

		s_cache[fileName] = newTexture;
	}

	// return the entry for this file
	return s_cache[fileName];
}

TextureCache::Texture TextureCache::GetAs(const char* fileName, bool sourceIsSRGB, Type desiredType, bool desiredSRGB, int desiredChannels)
{
	// Get the texture
	TextureCache::Texture ret = Get(fileName);
	if (!ret.Valid())
		return ret;

	// Convert type if we should
	if (ret.type != desiredType)
	{
		TextureCache::Texture convertedTex;
		convertedTex.width = ret.width;
		convertedTex.height = ret.height;
		convertedTex.channels = ret.channels;
		convertedTex.type = desiredType;
		convertedTex.pixels.resize(convertedTex.width * convertedTex.height * convertedTex.SizeOfPixel());
		convertedTex.fileName = ret.fileName;

		const size_t numValues = convertedTex.width * convertedTex.height * convertedTex.channels;

		switch (convertedTex.type)
		{
			case Type::F32:
			{
				const unsigned char* src = ret.pixels.data();
				float* dest = (float*)convertedTex.pixels.data();
				for (size_t i = 0; i < numValues; ++i)
					dest[i] = float(src[i]) / 255.0f;
				break;
			}
			case Type::U8:
			{
				const float* src = (const float*)ret.pixels.data();
				unsigned char* dest = convertedTex.pixels.data();
				for (size_t i = 0; i < numValues; ++i)
					dest[i] = (unsigned char)std::max(std::min(src[i] * 255.0f, 255.0f), 0.0f);
				break;
			}
		}
		ret = convertedTex;
	}

	// Convert sRGB if we should
	if (sourceIsSRGB != desiredSRGB)
	{
		// Only makes sense to do conversion in floating point. Can be re-visited later if needed.
		if (ret.type != Type::F32)
			return s_invalidTexture;

		TextureCache::Texture convertedTex = ret;

		const size_t numPixels = convertedTex.width * convertedTex.height;
		const size_t numChannels = convertedTex.channels;
		const float* src = (const float*)ret.pixels.data();
		float* dest = (float*)convertedTex.pixels.data();

		// linear to sRGB
		if (desiredSRGB)
		{
			for (size_t i = 0; i < numPixels; ++i)
			{
				for (size_t c = 0; c < numChannels; ++c)
				{
					if (c < 3)
						*dest = LinearTosRGB(*src);
					src++;
					dest++;
				}
			}
		}
		// sRGB to linear
		else
		{
			for (size_t i = 0; i < numPixels; ++i)
			{
				for (size_t c = 0; c < numChannels; ++c)
				{
					if (c < 3)
						*dest = SRGBToLinear(*src);
					src++;
					dest++;
				}
			}
		}
		ret = convertedTex;
	}

	// Convert the number of channels if we should
	if (desiredChannels > 0 && ret.channels != desiredChannels)
	{
		const size_t numPixels = ret.width * ret.height;
		const size_t numSrcChannels = ret.channels;

		TextureCache::Texture convertedTex;
		convertedTex.width = ret.width;
		convertedTex.height = ret.height;
		convertedTex.channels = desiredChannels;
		convertedTex.type = ret.type;
		convertedTex.pixels.resize(convertedTex.width * convertedTex.height * convertedTex.SizeOfPixel());
		convertedTex.fileName = ret.fileName;

		// If we are adding channels
		size_t memcpySizeSrc = 0;
		std::vector<unsigned char> extraChannels;
		if (desiredChannels > numSrcChannels)
		{
			// Copy the whole source pixel
			memcpySizeSrc = ret.SizeOfPixel();

			// Also copy extra channel data
			for (size_t c = numSrcChannels; c < desiredChannels; ++c)
			{
				// Alpha channel gets a 255 or 1.0
				if (c == 3)
				{
					if (ret.type == Type::F32)
					{
						size_t offset = extraChannels.size();
						extraChannels.resize(offset + 4);
						float f = 1.0f;
						memcpy(&extraChannels[offset], &f, 4);
					}
					else
					{
						extraChannels.push_back(255);
					}
				}
				// All others get a 0
				else
				{
					if (ret.type == Type::F32)
					{
						size_t offset = extraChannels.size();
						extraChannels.resize(offset + 4);
						float f = 0.0f;
						memcpy(&extraChannels[offset], &f, 4);
					}
					else
					{
						extraChannels.push_back(0);
					}
				}
			}
		}
		// else if we are removing channels
		else
		{
			// Copy only part of the source pixel
			memcpySizeSrc = convertedTex.SizeOfPixel();
		}

		// Copy over the channels we want to keep, and any extra channel data
		const unsigned char* src = ret.pixels.data();
		unsigned char* dest = convertedTex.pixels.data();
		for (size_t i = 0; i < numPixels; ++i)
		{
			// Copy from the source pixel to the dest pixel
			memcpy(dest, src, memcpySizeSrc);
			src += ret.SizeOfPixel();
			dest += memcpySizeSrc;

			// Copy any extra channel data
			memcpy(dest, extraChannels.data(), extraChannels.size());
			dest += extraChannels.size();
		}

		ret = convertedTex;
	}

	return ret;
}

}; // namespace DX12Utils
