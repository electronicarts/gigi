///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>

enum class ResourceType
{
    Buffer,
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube,
    Texture2DMS,
    RTScene
};

ID3D12Resource* CreateTexture(ID3D12Device* device, unsigned int size[3], unsigned int numMips, DXGI_FORMAT format, unsigned int sampleCount, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, ResourceType textureType, const char* debugName);

ID3D12Resource* CreateBuffer(ID3D12Device* device, unsigned int size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_HEAP_TYPE heapType, const char* debugName);
