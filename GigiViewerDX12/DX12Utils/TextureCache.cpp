///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "TextureCache.h"

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

#include <f16.h>

TextureCache::Texture& TextureCache::Get(FileCache& fileCache, const char* fileName_)
{
	// normalize the string by making it canonical and making it lower case
	std::filesystem::path p = std::filesystem::weakly_canonical(fileName_);
	std::string s = p.string();
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	const char* fileName = s.c_str();

	// If we don't have an entry for this file, create one
	if (m_cache.count(fileName) == 0)
	{
		FileCache::File fileData = fileCache.Get(s.c_str());

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
						int channelType = -1;
						std::vector<int> channelOrder(newTexture.channels);
						for (int i = 0; i < newTexture.channels; ++i)
						{
							if (i == 0)
							{
								channelType = exr_header.channels[i].pixel_type;
							}
							else if (channelType != exr_header.channels[i].pixel_type)
							{
								channelType = -1;
								break;
							}

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
						if (channelType == TINYEXR_PIXELTYPE_FLOAT || channelType == TINYEXR_PIXELTYPE_HALF)
						{
							newTexture.pixels.resize(newTexture.width * newTexture.height * newTexture.channels * sizeof(float));

							if (exr_image.images)
							{
								switch (channelType)
								{
									case TINYEXR_PIXELTYPE_HALF:
									{
										uint16_t** channelPtrs = (uint16_t**)exr_image.images;
										std::vector<uint16_t*> src(newTexture.channels);
										for (int i = 0; i < newTexture.channels; ++i)
											src[i] = channelPtrs[i];

										float* dest = (float*)newTexture.pixels.data();
										for (int iy = 0; iy < newTexture.height; ++iy)
										{
											for (int ix = 0; ix < newTexture.width; ++ix)
											{
												for (int channel = 0; channel < newTexture.channels; ++channel)
												{
													dest[channelOrder[channel]] = f16tof32(*src[channel]);
													src[channel]++;
												}
												dest += newTexture.channels;
											}
										}
										break;
									}
									case TINYEXR_PIXELTYPE_FLOAT:
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
										break;
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

		m_cache[fileName] = newTexture;
	}

	// return the entry for this file
	return m_cache[fileName];
}