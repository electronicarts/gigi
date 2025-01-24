///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define NOMINMAX
#include <d3d12.h>

#include <vector>

namespace ImageReadback
{
	bool GetDecodedPixel(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int x, int y, int z, int mipIndex, std::vector<unsigned char>& pixel, DXGI_FORMAT& pixelFormat);

	bool GetDecodedPixelsRectangle(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int x1, int x2, int y1, int y2, int z, int mipIndex, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat);

	bool GetDecodedImage(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat);

	bool GetDecodedImageSlice(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int chosenZIndex, int chosenMipIndex, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat, int& width, int& height);

	bool GetDecodedImageSliceAllDepths(ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int chosenZIndex, int chosenMipIndex, std::vector<unsigned char>& pixels, DXGI_FORMAT& pixelFormat, int& width, int& height, int& depth);

	DXGI_FORMAT GetDecodedFormat(DXGI_FORMAT format);
};
