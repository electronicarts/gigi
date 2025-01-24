///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "TextureCache.h"

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "GigiViewerDX12/stb/stb_image.h"

#define TINYEXR_IMPLEMENTATION
#include "GigiViewerDX12/tinyexr/tinyexr.h"

#include "GigiViewerDX12/f16.h"
#include "DirectXTex.h"

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

		Texture failTexture = newTexture;

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

						switch (exr_image.num_channels)
						{
							case 1: newTexture.format = DXGI_FORMAT_R32_FLOAT; break;
							case 2: newTexture.format = DXGI_FORMAT_R32G32_FLOAT; break;
							case 3: newTexture.format = DXGI_FORMAT_R32G32B32_FLOAT; break;
							case 4: newTexture.format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
							default:
							{
								m_cache[fileName] = failTexture;
								return m_cache[fileName];
							}
						}

						// find out what order to get the channels in
						int channelType = -1;
						std::vector<int> channelOrder(exr_image.num_channels);
						for (int i = 0; i < exr_image.num_channels; ++i)
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
							newTexture.images.resize(1);
							Image& newImage = newTexture.images[0];

							newImage.pixels.resize(newTexture.width * newTexture.height * exr_image.num_channels * sizeof(float));

							if (exr_image.images)
							{
								switch (channelType)
								{
									case TINYEXR_PIXELTYPE_HALF:
									{
										uint16_t** channelPtrs = (uint16_t**)exr_image.images;
										std::vector<uint16_t*> src(exr_image.num_channels);
										for (int i = 0; i < exr_image.num_channels; ++i)
											src[i] = channelPtrs[i];

										float* dest = (float*)newImage.pixels.data();
										for (int iy = 0; iy < newTexture.height; ++iy)
										{
											for (int ix = 0; ix < newTexture.width; ++ix)
											{
												for (int channel = 0; channel < exr_image.num_channels; ++channel)
												{
													dest[channelOrder[channel]] = f16tof32(*src[channel]);
													src[channel]++;
												}
												dest += exr_image.num_channels;
											}
										}
										break;
									}
									case TINYEXR_PIXELTYPE_FLOAT:
									{
										float** channelPtrs = (float**)exr_image.images;
										std::vector<float*> src(exr_image.num_channels);
										for (int i = 0; i < exr_image.num_channels; ++i)
											src[i] = channelPtrs[i];

										float* dest = (float*)newImage.pixels.data();
										for (int iy = 0; iy < newTexture.height; ++iy)
										{
											for (int ix = 0; ix < newTexture.width; ++ix)
											{
												for (int channel = 0; channel < exr_image.num_channels; ++channel)
												{
													dest[channelOrder[channel]] = *src[channel];
													src[channel]++;
												}
												dest += exr_image.num_channels;
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
			int numChannels = 0;
			float* rawPixels = stbi_loadf_from_memory((const unsigned char*)fileData.GetBytes(), (int)fileData.GetSize(), &newTexture.width, &newTexture.height, &numChannels, 0);
			if (rawPixels)
			{
				switch (numChannels)
				{
					case 1: newTexture.format = DXGI_FORMAT_R32_FLOAT; break;
					case 2: newTexture.format = DXGI_FORMAT_R32G32_FLOAT; break;
					case 3: newTexture.format = DXGI_FORMAT_R32G32B32_FLOAT; break;
					case 4: newTexture.format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
					default:
					{
						m_cache[fileName] = failTexture;
						return m_cache[fileName];
					}
				}

				newTexture.images.resize(1);
				Image& newImage = newTexture.images[0];

				newImage.pixels.resize(newTexture.width * newTexture.height * numChannels * sizeof(float));
				memcpy(newImage.pixels.data(), rawPixels, newImage.pixels.size());
				stbi_image_free(rawPixels);
			}
		}
		else if (p.extension().string() == ".dds")
		{
			DirectX::TexMetadata metaData;
			DirectX::ScratchImage scratchImage;

			HRESULT hr = DirectX::LoadFromDDSMemory((const uint8_t*)fileData.GetBytes(), fileData.GetSize(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metaData, scratchImage);
			if (SUCCEEDED(hr))
			{
				// Get the image properties
				newTexture.width = (int)metaData.width;
				newTexture.height = (int)metaData.height;
				newTexture.depth = metaData.IsVolumemap() ? (int)metaData.depth : (int)metaData.arraySize;
				newTexture.mips = (int)metaData.mipLevels;
				newTexture.format = metaData.format;

				newTexture.images.resize(scratchImage.GetImageCount());

				int arrayCount = (int)metaData.arraySize;
				int mipCount = (int)metaData.mipLevels;
				int depthCount = (int)metaData.depth;

				int newImageIndex = -1;
				for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
				{
					for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
					{
						for (int z = 0; z < depthCount; ++z)
						{
							newImageIndex++;
							Image& newImage = newTexture.images[newImageIndex];

							const DirectX::Image* oldImage = scratchImage.GetImage(mipIndex, arrayIndex, z);

							newImage.pixels.resize(oldImage->slicePitch);
							memcpy(newImage.pixels.data(), oldImage->pixels, oldImage->slicePitch);
						}

						if (metaData.IsVolumemap())
							depthCount = std::max(depthCount / 2, 1);
					}
				}

				// Release the scratch image
				scratchImage.Release();
			}
		}
		else
		{
			int numChannels = 0;
			unsigned char* rawPixels = stbi_load_from_memory((const unsigned char*)fileData.GetBytes(), (int)fileData.GetSize(), &newTexture.width, &newTexture.height, &numChannels, 0);
			if (rawPixels)
			{
				// There are no 3 channel texture formats, but 3 channel image files are common.
				// Promote it to 4 channels and fill alpha with 255 (1.0)
				if (numChannels == 3)
				{
					numChannels = 4;
					newTexture.format = DXGI_FORMAT_R8G8B8A8_UNORM;

					newTexture.images.resize(1);
					Image& newImage = newTexture.images[0];

					newImage.pixels.resize(newTexture.width* newTexture.height * numChannels);

					for (size_t i = 0; i < newTexture.width * newTexture.height; ++i)
					{
						newImage.pixels[i * 4 + 0] = rawPixels[i * 3 + 0];
						newImage.pixels[i * 4 + 1] = rawPixels[i * 3 + 1];
						newImage.pixels[i * 4 + 2] = rawPixels[i * 3 + 2];
						newImage.pixels[i * 4 + 3] = 255;
					}
				}
				else
				{
					switch (numChannels)
					{
						case 1: newTexture.format = DXGI_FORMAT_R8_UNORM; break;
						case 2: newTexture.format = DXGI_FORMAT_R8G8_UNORM; break;
						case 4: newTexture.format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
						default:
						{
							m_cache[fileName] = failTexture;
							return m_cache[fileName];
						}
					}

					newTexture.images.resize(1);
					Image& newImage = newTexture.images[0];

					newImage.pixels.resize(newTexture.width * newTexture.height * numChannels);
					memcpy(newImage.pixels.data(), rawPixels, newImage.pixels.size());
				}

				stbi_image_free(rawPixels);
			}
		}

		m_cache[fileName] = newTexture;
	}

	// return the entry for this file
	return m_cache[fileName];
}
