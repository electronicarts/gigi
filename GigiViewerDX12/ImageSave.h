///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define NOMINMAX
#include <d3d12.h>

namespace ImageSave
{
	struct Options
	{
		int zIndex = 0;			// Which array index or volume slice is being viewed.
		int mipIndex = 0;		// Which mip index is being viewed currently
		bool saveAll = false;	// If true, saves all slices and mips, else just saves the current one chosen.
		bool isCubeMap = false;	// If true, uses cube map face names instead of numbered array indices.

		struct BC45
		{
			bool isSigned = false;
		};
		BC45 bc45;

		struct BC6
		{
			bool isSigned = false;
		};
		BC6 bc6;

		struct BC7
		{
			bool sRGB = true;
		};
		BC7 bc7;
	};

	bool SaveAsPng(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsCSV(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsEXR(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsHDR(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsDDS_BC4(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsDDS_BC5(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsDDS_BC6(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsDDS_BC7(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
	bool SaveAsBinary(const char* fileName, ID3D12Device2* device, ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, const Options& options);
};
