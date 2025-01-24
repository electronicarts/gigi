///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ImageReadback.h"

#include <d3dx12/d3dx12.h>
#include "DirectXTex.h"

#include "GigiAssert.h"
#include "Schemas/Types.h"
#include "DX12Utils/Utils.h"
#include "GigiCompilerLib/Backends/Shared.h"

namespace ImageReadback
{
    static bool DecodeBCnPixel(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int x, int y, int z, int mipIndex, std::vector<unsigned char>& pixel, DXGI_FORMAT& pixelFormat)
    {
        // Calculate the subresource that we care about
        z = std::max(0, std::min(resourceOriginalDesc.DepthOrArraySize - 1, z));
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, is3D ? 0 : z, 0, resourceOriginalDesc.MipLevels, is3D ? 1 : resourceOriginalDesc.DepthOrArraySize);

        // Get information about the subresources
        unsigned int numSubResources = is3D
            ? resourceOriginalDesc.MipLevels
            : resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        std::vector<unsigned int> numRows(numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        device->GetCopyableFootprints(&resourceOriginalDesc, 0, numSubResources, 0, layouts, numRows.data(), nullptr, nullptr);

        // Ensure params are in range
        const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];
        x = std::max<UINT>(0, std::min<UINT>(layout.Footprint.Width - 1, x));
        y = std::max<UINT>(0, std::min<UINT>(layout.Footprint.Height - 1, y));
        if (is3D)
            z = std::max<UINT>(0, std::min<UINT>(layout.Footprint.Depth - 1, z));
        else
            z = 0;

        // Calculate where the block is that we care about, so we can decompress just a single block
        size_t blockOffset = 0;
        size_t blockSize = 0;
        {
            size_t rowPitch = layout.Footprint.RowPitch;
            size_t slicePitch = layout.Footprint.RowPitch * numRows[subresourceIndex];

            blockSize = DirectX::BytesPerBlock(resourceOriginalDesc.Format);
            size_t blockRowSize = rowPitch;
            size_t blockOffsetX = x / 4;
            size_t blockOffsetY = y / 4;
            blockOffset = layout.Offset + z * slicePitch + blockOffsetY * blockRowSize + blockOffsetX * blockSize;
        }

        // set up compressed image data
        DirectX::Image compressedImages;
        compressedImages.width = 4;
        compressedImages.height = 4;
        compressedImages.format = resourceOriginalDesc.Format;
        HRESULT hr = DirectX::ComputePitch(compressedImages.format, compressedImages.width, compressedImages.height, compressedImages.rowPitch, compressedImages.slicePitch);
        if (FAILED(hr))
            return false;

        // Map the block we care about
        D3D12_RANGE readRange;
        readRange.Begin = blockOffset;
        readRange.End = blockOffset + blockSize;
        uint8_t* readbackData = nullptr;
        hr = readbackResource->Map(0, &readRange, (void**)&readbackData);
        if (FAILED(hr))
            return false;

        // Decompress the block
        compressedImages.pixels = &readbackData[blockOffset];
        DirectX::ScratchImage decompressedImages;
        hr = DirectX::Decompress(compressedImages, DXGI_FORMAT_UNKNOWN, decompressedImages);

        // Unmap
        D3D12_RANGE writeRange;
        writeRange.Begin = 1;
        writeRange.End = 0;
        readbackResource->Unmap(0, &writeRange);
        if (FAILED(hr))
            return false;

        // Copy the specific pixel we care about and return success
        const DirectX::Image* decompressedImage = decompressedImages.GetImage(0, 0, 0);
        pixelFormat = decompressedImages.GetMetadata().format;
        size_t pixelBytes = DirectX::BitsPerPixel(pixelFormat) / 8;
        pixel.resize(pixelBytes);
        memcpy(pixel.data(), &decompressedImage->pixels[(y % 4) * decompressedImage->rowPitch + (x % 4) * pixelBytes], pixelBytes);
        return true;
    }

    static bool DecodeBCnImage(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat)
    {
        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSubResources = is3D
            ? resourceOriginalDesc.MipLevels
            : resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        std::vector<unsigned char> layoutsMemSrc((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        std::vector<unsigned int> numRowsAllSrc(numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layoutsSrc = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMemSrc.data();
        device->GetCopyableFootprints(&resourceOriginalDesc, 0, numSubResources, 0, layoutsSrc, numRowsAllSrc.data(), nullptr, nullptr);

        pixelFormat = GetDecodedFormat(resourceOriginalDesc.Format);
        DXGI_FORMAT_Info pixelFormatInfo = Get_DXGI_FORMAT_Info(pixelFormat);

        std::vector<unsigned char> layoutsMemDest((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layoutsDest = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMemDest.data();
        D3D12_RESOURCE_DESC decompressedDesc = resourceOriginalDesc;
        decompressedDesc.Format = pixelFormat;
        device->GetCopyableFootprints(&decompressedDesc, 0, numSubResources, 0, layoutsDest, nullptr, nullptr, nullptr);

        // Allocate memory for decoded image
        {
            // Count how much is needed
            size_t decompressedPixelCount = 0;
            for (size_t subResourceIndex = 0; subResourceIndex < numSubResources; ++subResourceIndex)
            {
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layoutDest = layoutsDest[subResourceIndex];
                decompressedPixelCount += layoutDest.Footprint.Width * layoutDest.Footprint.Height * layoutDest.Footprint.Depth;
            }

            // Allocate
            pixels.resize(decompressedPixelCount * pixelFormatInfo.bytesPerPixel);
        }

        // Map the blocks
        uint8_t* readbackData = nullptr;
        HRESULT hr = readbackResource->Map(0, nullptr, (void**)&readbackData);
        if (FAILED(hr))
            return false;

        // Decode each subresource
        {
            DirectX::Image compressedImages;
            compressedImages.format = resourceOriginalDesc.Format;

            // For each subresource
            unsigned char* dest = pixels.data();
            for (size_t subResourceIndex = 0; subResourceIndex < numSubResources; ++subResourceIndex)
            {
                // BC images may not match width / height of the decoded format due to the requirement of each block being 4x4.
                // We can detect this when layoutSrc w/h doesn't match layoutDest w/h
                // Dest w/h will be <= src w/h
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layoutSrc = layoutsSrc[subResourceIndex];
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layoutDest = layoutsDest[subResourceIndex];
                unsigned int numRowsSrc = numRowsAllSrc[subResourceIndex];

                compressedImages.width = layoutSrc.Footprint.Width;
                compressedImages.height = layoutSrc.Footprint.Height;
                compressedImages.rowPitch = layoutSrc.Footprint.RowPitch;
                compressedImages.slicePitch = layoutSrc.Footprint.RowPitch * numRowsSrc;

                // For each slice
                for (unsigned int z = 0; z < layoutSrc.Footprint.Depth; ++z)
                {
                    compressedImages.pixels = &readbackData[layoutSrc.Offset + z * compressedImages.slicePitch];

                    DirectX::ScratchImage decompressedImages;
                    hr = DirectX::Decompress(compressedImages, pixelFormat, decompressedImages);
                    if (FAILED(hr))
                    {
                        readbackResource->Unmap(0, nullptr);
                        return false;
                    }

                    // Copy the decompressed data out, but only as much as we need, since we don't need to round to 4x4 blocks
                    const DirectX::Image* decompressedImage = decompressedImages.GetImage(0, 0, 0);
                    const unsigned char* src = decompressedImage->pixels;

                    for (size_t iy = 0; iy < layoutDest.Footprint.Height; ++iy)
                    {
                        memcpy(dest, src, layoutDest.Footprint.Width * pixelFormatInfo.bytesPerPixel);
                        src += decompressedImage->rowPitch;
                        dest += layoutDest.Footprint.Width * pixelFormatInfo.bytesPerPixel;
                    }
                }
            }
        }

        // Unmap and return success
        D3D12_RANGE writeRange;
        writeRange.Begin = 1;
        writeRange.End = 0;
        readbackResource->Unmap(0, &writeRange);
        return true;
    }

    bool GetDecodedPixel(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int x, int y, int z, int mipIndex, std::vector<unsigned char>& pixel, DXGI_FORMAT& pixelFormat)
    {
        if (mipIndex < 0 || mipIndex >= resourceOriginalDesc.MipLevels)
            return false;

        // If compressed, decode it
        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(resourceOriginalDesc.Format);
        if (formatInfo.isCompressed)
            return DecodeBCnPixel(device, readbackResource, resourceOriginalDesc, x, y, z, mipIndex, pixel, pixelFormat);

        pixelFormat = resourceOriginalDesc.Format;

        // Calculate the subresource that we care about
        z = std::max(0, std::min(resourceOriginalDesc.DepthOrArraySize - 1, z));
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, is3D ? 0 : z, 0, resourceOriginalDesc.MipLevels, is3D ? 1 : resourceOriginalDesc.DepthOrArraySize);

        // Get information about the subresources
        unsigned int numSubResources = is3D
            ? resourceOriginalDesc.MipLevels
            : resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        device->GetCopyableFootprints(&resourceOriginalDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        // Ensure params are in range
        const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];
        if (x < 0 || y < 0 || z < 0 || x >= (int)layout.Footprint.Width || y >= (int)layout.Footprint.Height)
            return false;

        if (is3D && z >= (int)layout.Footprint.Depth)
            return false;

        // Map the memory
        D3D12_RANGE readRange;
        readRange.Begin = layout.Offset + y * layout.Footprint.RowPitch + x * formatInfo.bytesPerPixel + z * layout.Footprint.Height * layout.Footprint.RowPitch;
        readRange.End = readRange.Begin + formatInfo.bytesPerPixel;
        unsigned char* readbackData = nullptr;
        HRESULT hr = readbackResource->Map(0, &readRange, (void**)&readbackData);
        if (FAILED(hr))
            return false;

        // read back the single pixel we care about
        pixel.resize(formatInfo.bytesPerPixel);
        memcpy(pixel.data(), &readbackData[readRange.Begin], formatInfo.bytesPerPixel);

        // unmap and return success
        D3D12_RANGE writeRange;
        writeRange.Begin = 1;
        writeRange.End = 0;
        readbackResource->Unmap(0, &writeRange);
        return true;
    }

    bool GetDecodedPixelsRectangle(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int x1, int x2, int y1, int y2, int z, int mipIndex, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat)
    {
        // If this function is ever too slow, we can re-write it to not call into GetDecodedPixel for each individual pixel
        pixels.clear();
        std::vector<unsigned char> pixel;

        if (x1 > x2)
            std::swap(x1, x2);

        if (y1 > y2)
            std::swap(y1, y2);

        for (int iy = y1; iy < y2; ++iy)
        {
            for (int ix = x1; ix < x2; ++ix)
            {
                if (GetDecodedPixel(device, readbackResource, resourceOriginalDesc, ix, iy, z, mipIndex, pixel, pixelFormat))
                    pixels.insert(pixels.end(), pixel.begin(), pixel.end());
            }
        }

        return pixels.size() > 0;
    }

    bool GetDecodedImage(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat)
    {
        // If compressed, decode it
        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(resourceOriginalDesc.Format);
        if (formatInfo.isCompressed)
            return DecodeBCnImage(device, readbackResource, resourceOriginalDesc, pixels, pixelFormat);

        pixelFormat = resourceOriginalDesc.Format;

        // Get the information about each sub resource
        unsigned int numSubResources = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
            ? resourceOriginalDesc.MipLevels
            : resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        device->GetCopyableFootprints(&resourceOriginalDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        // Map the memory
        unsigned char* readbackData = nullptr;
        HRESULT hr = readbackResource->Map(0, nullptr, (void**)&readbackData);
        if (FAILED(hr))
            return false;

        // Get the pixel data of each resource
        pixels.resize(0);
        size_t destOffset = 0;
        for (unsigned int subResourceIndex = 0; subResourceIndex < numSubResources; ++subResourceIndex)
        {
            const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subResourceIndex];

            int unalignedPitch = layout.Footprint.Width * formatInfo.bytesPerPixel;

            pixels.resize(pixels.size() + unalignedPitch * layout.Footprint.Height * layout.Footprint.Depth);

            for (unsigned int iz = 0; iz < layout.Footprint.Depth; ++iz)
            {
                for (unsigned int iy = 0; iy < layout.Footprint.Height; ++iy)
                {
                    const unsigned char* src = &readbackData[layout.Offset + iy * layout.Footprint.RowPitch + iz * layout.Footprint.Height * layout.Footprint.RowPitch];
                    unsigned char* dest = &pixels[destOffset];
                    memcpy(dest, src, unalignedPitch);
                    destOffset += unalignedPitch;
                }
            }
        }

        // unmap and return success
        D3D12_RANGE writeRange;
        writeRange.Begin = 1;
        writeRange.End = 0;
        readbackResource->Unmap(0, &writeRange);
        return true;
    }

    bool GetDecodedImageSlice(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int chosenZIndex, int chosenMipIndex, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat, int& width, int& height)
    {
        // TODO: in the future, this should only decode the specific slice asked for. Lots less processing, memory allocation, and memory copies.
        // Decode the full image
        std::vector<unsigned char> decodedPixels;
        if (!GetDecodedImage(device, readbackResource, resourceOriginalDesc, decodedPixels, pixelFormat))
            return false;

        // Get information about the subresources
        bool is3D = (resourceOriginalDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        unsigned int numSubResources = is3D
            ? resourceOriginalDesc.MipLevels
            : resourceOriginalDesc.MipLevels * resourceOriginalDesc.DepthOrArraySize;
        std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
        D3D12_RESOURCE_DESC decodedDesc = resourceOriginalDesc;
        decodedDesc.Format = pixelFormat;
        device->GetCopyableFootprints(&decodedDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

        // Gather image information
        int arrayCount = is3D ? 1 : resourceOriginalDesc.DepthOrArraySize;
        int mipCount = resourceOriginalDesc.MipLevels;
        DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(pixelFormat);

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
                    // Get the information about this subresource
                    unsigned int subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceOriginalDesc.MipLevels, arrayCount);
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresourceIndex];

                    // Figure out if we should save this one
                    bool thisIsTheOne = false;
                    if (is3D)
                        thisIsTheOne = (mipIndex == chosenMipIndex && zIndex == chosenZIndex);
                    else
                        thisIsTheOne = (mipIndex == chosenMipIndex && arrayIndex == chosenZIndex);

                    // If this is the slice we want, send it back
                    if (thisIsTheOne)
                    {
                        width = layout.Footprint.Width;
                        height = layout.Footprint.Height;

                        pixels.resize(width * height * decodedFormatInfo.bytesPerPixel);
                        memcpy(pixels.data(), src, pixels.size());

                        return true;
                    }

                    // Move past this subresource
                    src += layout.Footprint.Width * layout.Footprint.Height * decodedFormatInfo.bytesPerPixel;
                }
            }
        }

        return false;
    }

    bool GetDecodedImageSliceAllDepths(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int chosenZIndex, int chosenMipIndex, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat, int& width, int& height, int& depth)
    {
        if (resourceOriginalDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            depth = 1;
            return GetDecodedImageSlice(device, readbackResource, resourceOriginalDesc, chosenZIndex, chosenMipIndex, pixels, pixelFormat, width, height);
        }

        depth = resourceOriginalDesc.DepthOrArraySize;
        for (int i = 0; i < chosenMipIndex; ++i)
            depth = std::max(depth / 2, 1);

        pixels.resize(0);
        std::vector<unsigned char> pixelsSlice;
        for (int i = 0; i < depth; ++i)
        {
            if (!GetDecodedImageSlice(device, readbackResource, resourceOriginalDesc, i, chosenMipIndex, pixelsSlice, pixelFormat, width, height))
                return false;

            size_t copyOffset = pixels.size();
            size_t copySize = pixelsSlice.size();
            pixels.resize(copyOffset + copySize);
            memcpy(&pixels[copyOffset], pixelsSlice.data(), copySize);
        }

        return true;
    }

    DXGI_FORMAT GetDecodedFormat(DXGI_FORMAT format)
    {
        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(format);
        return formatInfo.isCompressed ? formatInfo.decompressedFormat : format;
    }

}; // namespace ImageReadback
