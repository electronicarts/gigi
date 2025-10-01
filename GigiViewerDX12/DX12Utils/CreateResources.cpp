///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "CreateResources.h"
#include "GigiCompilerLib/Utils.h"

ID3D12Resource* CreateTexture(ID3D12Device* device, unsigned int size[3], unsigned int numMips, DXGI_FORMAT format, unsigned int sampleCount, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, ResourceType textureType, const char* debugName)
{
    if (numMips == 0)
    {
        int maxSize = max(size[0], size[1]);
        numMips = 1;
        while (maxSize > 1)
        {
            maxSize /= 2;
            numMips++;
        }
    }

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = numMips;
    textureDesc.Format = format;
    textureDesc.Width = size[0];
    textureDesc.Height = size[1];
    textureDesc.Flags = flags;
    textureDesc.SampleDesc.Count = sampleCount;
    textureDesc.SampleDesc.Quality = 0;

    switch(textureType)
    {
        case ResourceType::Texture2D:
        case ResourceType::Texture2DMS:
        {
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            textureDesc.DepthOrArraySize = 1;
            break;
        }
        case ResourceType::Texture2DArray:
        {
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            textureDesc.DepthOrArraySize = size[2];
            break;
        }
        case ResourceType::Texture3D:
        {
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            textureDesc.DepthOrArraySize = size[2];
            break;
        }
        case ResourceType::TextureCube:
        {
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            textureDesc.DepthOrArraySize = 6;
            break;
        }
        default: return nullptr;
    }

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    ID3D12Resource* resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        state,
        nullptr,
        IID_PPV_ARGS(&resource));

    if (FAILED(hr))
        return nullptr;

    if (debugName)
        resource->SetName(ToWideString(debugName).c_str());

    return resource;
}

ID3D12Resource* CreateBuffer(ID3D12Device* device, unsigned int size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_HEAP_TYPE heapType, const char* debugName)
{
    D3D12_HEAP_PROPERTIES heapDesc = {};
    heapDesc.Type = heapType;
    heapDesc.CreationNodeMask = 1;
    heapDesc.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Width = size;
    resourceDesc.Flags = flags;

    ID3D12Resource* ret = nullptr;
    HRESULT hr = device->CreateCommittedResource(&heapDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, state, nullptr, IID_PPV_ARGS(&ret));
    if (FAILED(hr))
        return nullptr;

    if (debugName)
        ret->SetName(ToWideString(debugName).c_str());

    return ret;
}