///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS // for stbi_image_write

#include "ImageSave.h"

#include "ImageReadback.h"

#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "GigiAssert.h"
#include "Schemas/Types.h"
#include "DX12Utils/Utils.h"
#include "GigiCompilerLib/Backends/Shared.h"

#include "tinyexr/tinyexr.h"

#include "DirectXTex.h"
#include <d3dx12/d3dx12.h>

namespace ImageSave
{
    template <typename LAMBDA>
    static bool ProcessResource(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options, const char* extension, const LAMBDA& lambda)
    {
        // Get the pixel data
        DXGI_FORMAT decodedFormat;
        std::vector<unsigned char> decodedPixels;
        if (!ImageReadback::GetDecodedImage(device, readbackResource, resourceOriginalDesc, decodedPixels, decodedFormat))
            return false;
        DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);

        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSubResources = is3D
            ? resourceOriginalDesc.MipLevels
            : resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        D3D12_RESOURCE_DESC decodedDesc = resourceOriginalDesc;
        decodedDesc.Format = decodedFormat;
        device->GetCopyableFootprints(&decodedDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        int arrayCount = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
        int mipCount = resourceOriginalDesc.MipLevels;

        // for each array index
        unsigned char* src = decodedPixels.data();
        for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
        {
            // for each mip index
            for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            {
                int zCount = 1;
                if (is3D)
                {
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    zCount = layouts[subresourceIndex].Footprint.Depth;
                }

                for (int zIndex = 0; zIndex < zCount; ++zIndex)
                {
                    // Make the filename
                    std::filesystem::path p(fileName);
                    if (options.saveAll)
                    {
                        static const char* c_cubeMapNames[] =
                        {
                            "Right",
                            "Left",
                            "Up",
                            "Down",
                            "Front",
                            "Back"
                        };

                        char numberedExtension[256];
                        if (is3D)
                            sprintf_s(numberedExtension, ".mip%i.slice%i%s", mipIndex, zIndex, extension);
                        else if (options.isCubeMap)
                            sprintf_s(numberedExtension, ".%s.mip%i%s", c_cubeMapNames[arrayIndex], mipIndex, extension);
                        else
                            sprintf_s(numberedExtension, ".index%i.mip%i%s", arrayIndex, mipIndex, extension);
                        p.replace_extension(numberedExtension);
                    }
                    else
                    {
                        p.replace_extension(extension);
                    }

                    // Get the information about this subresource
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];

                    // Figure out if we should save this one
                    bool shouldSave = options.saveAll;
                    if (is3D)
                        shouldSave |= (mipIndex == options.mipIndex && zIndex == options.zIndex);
                    else
                        shouldSave |= (mipIndex == options.mipIndex && arrayIndex == options.zIndex);

                    // save the image if we should
                    if (shouldSave)
                    {
                        if (!lambda(p.string().c_str(), p.wstring().c_str(), layout.Footprint.Width, layout.Footprint.Height, decodedFormatInfo.channelCount, decodedFormatInfo, options, src))
                            return false;

                        if (!options.saveAll)
                            return true;
                    }

                    // Move past this subresource
                    src += layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel;
                }
            }
        }

        return true;
    }

    bool SaveAsPng(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        return ProcessResource(fileName, device, readbackResource, resourceOriginalDesc, options, ".png",
            [] (const char* fileName, const wchar_t* fileNameW, int width, int height, int channelCount, const DXGI_FORMAT_Info& decodedFormatInfo, const Options& options, const unsigned char* pixels)
            {
                // Verify the pixel data is appropriate for this file type
                if (decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_uint8_t)
                    return false;

                return stbi_write_png(fileName, width, height, channelCount, pixels, 0) != 0;
            }
        );
    }

    bool SaveAsCSV(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        return ProcessResource(fileName, device, readbackResource, resourceOriginalDesc, options, ".csv",
            [](const char* fileName, const wchar_t* fileNameW, int width, int height, int channelCount, const DXGI_FORMAT_Info& decodedFormatInfo, const Options& options, const unsigned char* pixels)
            {
                FILE* file = nullptr;
                fopen_s(&file, fileName, "wb");

                static const char c_channelNames[] = { 'R', 'G', 'B', 'A' };
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                    fprintf(file, "%s\"%c\"", (i > 0 ? "," : ""), c_channelNames[i]);
                fprintf(file, "\n");

                switch (decodedFormatInfo.channelType)
                {
                    case DXGI_FORMAT_Info::ChannelType::_uint8_t:
                    {
                        const unsigned char* src = pixels;
                        for (int i = 0; i < width * height; ++i)
                        {
                            for (int c = 0; c < decodedFormatInfo.channelCount; ++c)
                            {
                                fprintf(file, "%s\"%u\"", (c > 0 ? "," : ""), (unsigned int)*src);
                                src++;
                            }
                            fprintf(file, "\n");
                        }
                        break;
                    }
                    case DXGI_FORMAT_Info::ChannelType::_float:
                    {
                        for (int i = 0; i < width * height; ++i)
                        {
                            const float* src = (float*)pixels;
                            for (int c = 0; c < decodedFormatInfo.channelCount; ++c)
                            {
                                fprintf(file, "%s\"%f\"", (c > 0 ? "," : ""), *src);
                                src++;
                            }
                            fprintf(file, "\n");
                        }
                        break;
                    }
                    default:
                    {
                        return false;
                    }
                }

                fclose(file);

                return true;
            }
        );
    }

    bool SaveAsHDR(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        return ProcessResource(fileName, device, readbackResource, resourceOriginalDesc, options, ".hdr",
            [](const char* fileName, const wchar_t* fileNameW, int width, int height, int channelCount, const DXGI_FORMAT_Info& decodedFormatInfo, const Options& options, const unsigned char* pixels)
            {
                // Verify the pixel data is appropriate for this file type
                if (decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_float)
                    return false;

                return stbi_write_hdr(fileName, width, height, channelCount, (float*)pixels) != 0;
            }
        );
    }

    bool SaveAsEXR(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        return ProcessResource(fileName, device, readbackResource, resourceOriginalDesc, options, ".exr",
            [](const char* fileName, const wchar_t* fileNameW, int width, int height, int channelCount, const DXGI_FORMAT_Info& decodedFormatInfo, const Options& options, const unsigned char* pixels_)
            {
                // Verify the pixel data is appropriate for this file type
                if (decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_float)
                    return false;

                EXRHeader header;
                InitEXRHeader(&header);

                EXRImage image;
                InitEXRImage(&image);

                int numChannels = channelCount;
                const float* pixels = (const float*)pixels_;

                image.num_channels = numChannels;

                std::vector<std::vector<float>> images;
                images.resize(numChannels);
                for (std::vector<float>& v : images)
                    v.resize(width * height);

                // Split interleaved channels into separate channels
                for (int i = 0; i < width * height; i++)
                {
                    for (int j = 0; j < numChannels; ++j)
                        images[j][i] = pixels[numChannels * i + j];
                }

                // put the RGBA channels into ABGR because most EXR viewers expect this
                std::vector<float*> image_ptr(numChannels);
                for (int i = 0; i < numChannels; ++i)
                    image_ptr[i] = images[numChannels - i - 1].data();

                image.images = (unsigned char**)image_ptr.data();
                image.width = width;
                image.height = height;

                // Must be (A)BGR order, since most of EXR viewers expect this channel order.
                const char* chanelNames[4] = { "R", "G", "B", "A" };
                header.num_channels = numChannels;
                header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
                for (int i = 0; i < numChannels; ++i)
                {
                    char buffer[256];
                    const char* channelName;
                    if (i < 4)
                    {
                        channelName = chanelNames[numChannels - i - 1];
                    }
                    else
                    {
                        sprintf(buffer, "X%i", i - 4);
                        channelName = buffer;
                    }
                    strncpy(header.channels[i].name, channelName, 255); header.channels[0].name[strlen(channelName)] = '\0';
                }

                header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
                header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
                for (int i = 0; i < header.num_channels; i++) {
                    header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
                    header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;// TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
                }

                const char* err = NULL; // or nullptr in C++11 or later.
                int ret = SaveEXRImageToFile(&image, &header, fileName, &err);
                if (ret != TINYEXR_SUCCESS) {
                    //fprintf(stderr, "Save EXR err: %s\n", err);
                    FreeEXRErrorMessage(err); // free's buffer for an error message
                    return false;
                }
                //printf("Saved exr file. [ %s ] \n", outfilename);

                //free(pixels);

                free(header.channels);
                free(header.pixel_types);
                free(header.requested_pixel_types);

                return true;
            }
        );
    }

    bool SaveAsDDS_BC4(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        // Get the pixel data
        DXGI_FORMAT decodedFormat;
        std::vector<unsigned char> decodedPixels;
        if (!ImageReadback::GetDecodedImage(device, readbackResource, resourceOriginalDesc, decodedPixels, decodedFormat))
            return false;

        // Verify the pixel data is appropriate for this file type
        DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
        if (decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_uint8_t)
            return false;

        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSlices = resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        unsigned int numSubResources = is3D ? resourceOriginalDesc.MipLevels : numSlices;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        D3D12_RESOURCE_DESC decodedDesc = resourceOriginalDesc;
        decodedDesc.Format = decodedFormat;
        device->GetCopyableFootprints(&decodedDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        int arrayCount = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
        int mipCount = resourceOriginalDesc.MipLevels;

        // for each array index
        std::vector<DirectX::Image> images(options.saveAll ? numSlices : 1);
        unsigned char* src = decodedPixels.data();
        int imageIndex = 0;
        for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
        {
            // for each mip index
            for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            {
                int zCount = 1;
                if (is3D)
                {
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    zCount = layouts[subresourceIndex].Footprint.Depth;
                }

                for (int zIndex = 0; zIndex < zCount; ++zIndex)
                {
                    // Get the information about this subresource
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];

                    // Figure out if we should save this one
                    bool shouldSave = options.saveAll;
                    if (is3D)
                        shouldSave |= (mipIndex == options.mipIndex && zIndex == options.zIndex);
                    else
                        shouldSave |= (mipIndex == options.mipIndex && arrayIndex == options.zIndex);

                    // save the image if we should
                    if (shouldSave)
                    {
                        DirectX::Image& image = images[imageIndex];
                        imageIndex++;

                        image.width = layout.Footprint.Width;
                        image.height = layout.Footprint.Height;
                        image.format = decodedFormatInfo.format;
                        image.rowPitch = decodedFormatInfo.bytesPerPixel * image.width;
                        image.slicePitch = image.rowPitch * image.height;
                        image.pixels = src;
                    }

                    // Move past this subresource
                    src += layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel;
                }
            }
        }

        // Being lazy about calculating how many 2d images a 3d texture with mips needs.
        if (imageIndex < images.size())
            images.resize(imageIndex);

        // make meta data
        DirectX::TexMetadata metaData;
        if (options.saveAll)
        {
            metaData.width = resourceOriginalDesc.Width;
            metaData.height = resourceOriginalDesc.Height;
            metaData.depth = is3D ? resourceOriginalDesc.DepthOrArraySize : 1;
            metaData.arraySize = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
            metaData.mipLevels = resourceOriginalDesc.MipLevels;
        }
        else
        {
            metaData.width = images[0].width;
            metaData.height = images[0].height;
            metaData.depth = 1;
            metaData.arraySize = 1;
            metaData.mipLevels = 1;
        }
        metaData.miscFlags = 0;
        metaData.miscFlags2 = 0;
        metaData.format = decodedFormatInfo.format;
        metaData.dimension = is3D ? DirectX::TEX_DIMENSION_TEXTURE3D : DirectX::TEX_DIMENSION_TEXTURE2D;

        // Compress
        DirectX::ScratchImage compressedImages;
        HRESULT hr = DirectX::Compress(images.data(), images.size(), metaData, options.bc45.isSigned ? DXGI_FORMAT_BC4_SNORM : DXGI_FORMAT_BC4_UNORM, DirectX::TEX_COMPRESS_PARALLEL, DirectX::TEX_THRESHOLD_DEFAULT, compressedImages);
        if (FAILED(hr))
        {
            return false;
        }

        // Save to disk
        std::filesystem::path p(fileName);
        p.replace_extension(".dds");
        hr = DirectX::SaveToDDSFile(compressedImages.GetImages(), compressedImages.GetImageCount(), compressedImages.GetMetadata(), DirectX::DDS_FLAGS_NONE, p.wstring().c_str());
        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }

    bool SaveAsDDS_BC5(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        // Get the pixel data
        DXGI_FORMAT decodedFormat;
        std::vector<unsigned char> decodedPixels;
        if (!ImageReadback::GetDecodedImage(device, readbackResource, resourceOriginalDesc, decodedPixels, decodedFormat))
            return false;

        // Verify the pixel data is appropriate for this file type
        DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
        if (decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_uint8_t)
            return false;

        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSlices = resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        unsigned int numSubResources = is3D ? resourceOriginalDesc.MipLevels : numSlices;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        D3D12_RESOURCE_DESC decodedDesc = resourceOriginalDesc;
        decodedDesc.Format = decodedFormat;
        device->GetCopyableFootprints(&decodedDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        int arrayCount = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
        int mipCount = resourceOriginalDesc.MipLevels;

        // for each array index
        std::vector<DirectX::Image> images(options.saveAll ? numSlices : 1);
        unsigned char* src = decodedPixels.data();
        int imageIndex = 0;
        for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
        {
            // for each mip index
            for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            {
                int zCount = 1;
                if (is3D)
                {
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    zCount = layouts[subresourceIndex].Footprint.Depth;
                }

                for (int zIndex = 0; zIndex < zCount; ++zIndex)
                {
                    // Get the information about this subresource
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];

                    // Figure out if we should save this one
                    bool shouldSave = options.saveAll;
                    if (is3D)
                        shouldSave |= (mipIndex == options.mipIndex && zIndex == options.zIndex);
                    else
                        shouldSave |= (mipIndex == options.mipIndex && arrayIndex == options.zIndex);

                    // save the image if we should
                    if (shouldSave)
                    {
                        DirectX::Image& image = images[imageIndex];
                        imageIndex++;

                        image.width = layout.Footprint.Width;
                        image.height = layout.Footprint.Height;
                        image.format = decodedFormatInfo.format;
                        image.rowPitch = decodedFormatInfo.bytesPerPixel * image.width;
                        image.slicePitch = image.rowPitch * image.height;
                        image.pixels = src;
                    }

                    // Move past this subresource
                    src += layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel;
                }
            }
        }

        // Being lazy about calculating how many 2d images a 3d texture with mips needs.
        if (imageIndex < images.size())
            images.resize(imageIndex);

        // make meta data
        DirectX::TexMetadata metaData;
        if (options.saveAll)
        {
            metaData.width = resourceOriginalDesc.Width;
            metaData.height = resourceOriginalDesc.Height;
            metaData.depth = is3D ? resourceOriginalDesc.DepthOrArraySize : 1;
            metaData.arraySize = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
            metaData.mipLevels = resourceOriginalDesc.MipLevels;
        }
        else
        {
            metaData.width = images[0].width;
            metaData.height = images[0].height;
            metaData.depth = 1;
            metaData.arraySize = 1;
            metaData.mipLevels = 1;
        }
        metaData.miscFlags = 0;
        metaData.miscFlags2 = 0;
        metaData.format = decodedFormatInfo.format;
        metaData.dimension = is3D ? DirectX::TEX_DIMENSION_TEXTURE3D : DirectX::TEX_DIMENSION_TEXTURE2D;

        // Compress
        DirectX::ScratchImage compressedImages;
        HRESULT hr = DirectX::Compress(images.data(), images.size(), metaData, options.bc45.isSigned ? DXGI_FORMAT_BC5_SNORM : DXGI_FORMAT_BC5_UNORM, DirectX::TEX_COMPRESS_PARALLEL, DirectX::TEX_THRESHOLD_DEFAULT, compressedImages);
        if (FAILED(hr))
        {
            return false;
        }

        // Save to disk
        std::filesystem::path p(fileName);
        p.replace_extension(".dds");
        hr = DirectX::SaveToDDSFile(compressedImages.GetImages(), compressedImages.GetImageCount(), compressedImages.GetMetadata(), DirectX::DDS_FLAGS_NONE, p.wstring().c_str());
        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }

    bool SaveAsDDS_BC6(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        // Get the pixel data
        DXGI_FORMAT decodedFormat;
        std::vector<unsigned char> decodedPixels;
        if (!ImageReadback::GetDecodedImage(device, readbackResource, resourceOriginalDesc, decodedPixels, decodedFormat))
            return false;

        // Verify the pixel data is appropriate for this file type
        DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
        if (decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_float)
            return false;

        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSlices = resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        unsigned int numSubResources = is3D ? resourceOriginalDesc.MipLevels : numSlices;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        D3D12_RESOURCE_DESC decodedDesc = resourceOriginalDesc;
        decodedDesc.Format = decodedFormat;
        device->GetCopyableFootprints(&decodedDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        int arrayCount = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
        int mipCount = resourceOriginalDesc.MipLevels;

        // for each array index
        std::vector<DirectX::Image> images(options.saveAll ? numSlices : 1);
        unsigned char* src = decodedPixels.data();
        int imageIndex = 0;
        for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
        {
            // for each mip index
            for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            {
                int zCount = 1;
                if (is3D)
                {
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    zCount = layouts[subresourceIndex].Footprint.Depth;
                }

                for (int zIndex = 0; zIndex < zCount; ++zIndex)
                {
                    // Get the information about this subresource
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];

                    // Figure out if we should save this one
                    bool shouldSave = options.saveAll;
                    if (is3D)
                        shouldSave |= (mipIndex == options.mipIndex && zIndex == options.zIndex);
                    else
                        shouldSave |= (mipIndex == options.mipIndex && arrayIndex == options.zIndex);

                    // save the image if we should
                    if (shouldSave)
                    {
                        DirectX::Image& image = images[imageIndex];
                        imageIndex++;

                        image.width = layout.Footprint.Width;
                        image.height = layout.Footprint.Height;
                        image.format = decodedFormatInfo.format;
                        image.rowPitch = decodedFormatInfo.bytesPerPixel * image.width;
                        image.slicePitch = image.rowPitch * image.height;
                        image.pixels = src;
                    }

                    // Move past this subresource
                    src += layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel;
                }
            }
        }

        // Being lazy about calculating how many 2d images a 3d texture with mips needs.
        if (imageIndex < images.size())
            images.resize(imageIndex);

        // make meta data
        DirectX::TexMetadata metaData;
        if (options.saveAll)
        {
            metaData.width = resourceOriginalDesc.Width;
            metaData.height = resourceOriginalDesc.Height;
            metaData.depth = is3D ? resourceOriginalDesc.DepthOrArraySize : 1;
            metaData.arraySize = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
            metaData.mipLevels = resourceOriginalDesc.MipLevels;
        }
        else
        {
            metaData.width = images[0].width;
            metaData.height = images[0].height;
            metaData.depth = 1;
            metaData.arraySize = 1;
            metaData.mipLevels = 1;
        }
        metaData.miscFlags = 0;
        metaData.miscFlags2 = 0;
        metaData.format = decodedFormatInfo.format;
        metaData.dimension = is3D ? DirectX::TEX_DIMENSION_TEXTURE3D : DirectX::TEX_DIMENSION_TEXTURE2D;

        // Compress
        DirectX::ScratchImage compressedImages;
        HRESULT hr = DirectX::Compress(images.data(), images.size(), metaData, options.bc6.isSigned ? DXGI_FORMAT_BC6H_SF16 : DXGI_FORMAT_BC6H_UF16, DirectX::TEX_COMPRESS_PARALLEL, DirectX::TEX_THRESHOLD_DEFAULT, compressedImages);
        if (FAILED(hr))
        {
            return false;
        }

        // Save to disk
        std::filesystem::path p(fileName);
        p.replace_extension(".dds");
        hr = DirectX::SaveToDDSFile(compressedImages.GetImages(), compressedImages.GetImageCount(), compressedImages.GetMetadata(), DirectX::DDS_FLAGS_NONE, p.wstring().c_str());
        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }

    bool SaveAsDDS_BC7(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        // Get the pixel data
        DXGI_FORMAT decodedFormat;
        std::vector<unsigned char> decodedPixels;
        if (!ImageReadback::GetDecodedImage(device, readbackResource, resourceOriginalDesc, decodedPixels, decodedFormat))
            return false;

        // Verify the pixel data is appropriate for this file type
        DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
        if (decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_uint8_t)
            return false;

        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSlices = resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        unsigned int numSubResources = is3D ? resourceOriginalDesc.MipLevels : numSlices;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        D3D12_RESOURCE_DESC decodedDesc = resourceOriginalDesc;
        decodedDesc.Format = decodedFormat;
        device->GetCopyableFootprints(&decodedDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        int arrayCount = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
        int mipCount = resourceOriginalDesc.MipLevels;

        // for each array index
        std::vector<DirectX::Image> images(options.saveAll ? numSlices : 1);
        unsigned char* src = decodedPixels.data();
        int imageIndex = 0;
        for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
        {
            // for each mip index
            for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            {
                int zCount = 1;
                if (is3D)
                {
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    zCount = layouts[subresourceIndex].Footprint.Depth;
                }

                for (int zIndex = 0; zIndex < zCount; ++zIndex)
                {
                    // Get the information about this subresource
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];

                    // Figure out if we should save this one
                    bool shouldSave = options.saveAll;
                    if (is3D)
                        shouldSave |= (mipIndex == options.mipIndex && zIndex == options.zIndex);
                    else
                        shouldSave |= (mipIndex == options.mipIndex && arrayIndex == options.zIndex);

                    // save the image if we should
                    if (shouldSave)
                    {
                        DirectX::Image& image = images[imageIndex];
                        imageIndex++;

                        image.width = layout.Footprint.Width;
                        image.height = layout.Footprint.Height;
                        image.format = decodedFormatInfo.format;
                        image.rowPitch = decodedFormatInfo.bytesPerPixel * image.width;
                        image.slicePitch = image.rowPitch * image.height;
                        image.pixels = src;
                    }

                    // Move past this subresource
                    src += layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel;
                }
            }
        }

        // Being lazy about calculating how many 2d images a 3d texture with mips needs.
        if (imageIndex < images.size())
            images.resize(imageIndex);

        // make meta data
        DirectX::TexMetadata metaData;
        if (options.saveAll)
        {
            metaData.width = resourceOriginalDesc.Width;
            metaData.height = resourceOriginalDesc.Height;
            metaData.depth = is3D ? resourceOriginalDesc.DepthOrArraySize : 1;
            metaData.arraySize = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
            metaData.mipLevels = resourceOriginalDesc.MipLevels;
        }
        else
        {
            metaData.width = images[0].width;
            metaData.height = images[0].height;
            metaData.depth = 1;
            metaData.arraySize = 1;
            metaData.mipLevels = 1;
        }
        metaData.miscFlags = 0;
        metaData.miscFlags2 = 0;
        metaData.format = decodedFormatInfo.format;
        metaData.dimension = is3D ? DirectX::TEX_DIMENSION_TEXTURE3D : DirectX::TEX_DIMENSION_TEXTURE2D;

        // Compress
        DirectX::ScratchImage compressedImages;
        HRESULT hr = DirectX::Compress(images.data(), images.size(), metaData, options.bc7.sRGB ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM, DirectX::TEX_COMPRESS_PARALLEL, DirectX::TEX_THRESHOLD_DEFAULT, compressedImages);
        if (FAILED(hr))
        {
            return false;
        }

        // Save to disk
        std::filesystem::path p(fileName);
        p.replace_extension(".dds");
        hr = DirectX::SaveToDDSFile(compressedImages.GetImages(), compressedImages.GetImageCount(), compressedImages.GetMetadata(), DirectX::DDS_FLAGS_NONE, p.wstring().c_str());
        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }

    bool SaveAsBinary(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options)
    {
        // Get the pixel data
        DXGI_FORMAT decodedFormat;
        std::vector<unsigned char> decodedPixels;
        if (!ImageReadback::GetDecodedImage(device, readbackResource, resourceOriginalDesc, decodedPixels, decodedFormat))
            return false;

        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSubResources = is3D
            ? resourceOriginalDesc.MipLevels
            : resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        D3D12_RESOURCE_DESC decodedDesc = resourceOriginalDesc;
        decodedDesc.Format = decodedFormat;
        device->GetCopyableFootprints(&decodedDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        int arrayCount = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
        int mipCount = resourceOriginalDesc.MipLevels;

        // open file
        std::filesystem::path p(fileName);
        p.replace_extension(".bin");
        FILE* file = nullptr;
        fopen_s(&file, p.string().c_str(), "wb");
        if (!file)
            return false;

        // for each array index
        DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
        unsigned char* src = decodedPixels.data();
        for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
        {
            // for each mip index
            for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            {
                int zCount = 1;
                if (is3D)
                {
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    zCount = layouts[subresourceIndex].Footprint.Depth;
                }

                for (int zIndex = 0; zIndex < zCount; ++zIndex)
                {
                    // Get the information about this subresource
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];

                    // Figure out if we should save this one
                    bool shouldSave = options.saveAll;
                    if (is3D)
                        shouldSave |= (mipIndex == options.mipIndex && zIndex == options.zIndex);
                    else
                        shouldSave |= (mipIndex == options.mipIndex && arrayIndex == options.zIndex);

                    // save the image if we should
                    if (shouldSave)
                        fwrite(src, 1, layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel, file);

                    // Move past this subresource
                    src += layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel;
                }
            }
        }

        fclose(file);
        return true;
    }

}; // namespace ImageSave
