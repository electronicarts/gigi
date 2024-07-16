///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "UploadBufferTracker.h"

#include <d3d12.h>

UploadBufferTracker::Buffer* UploadBufferTracker::GetBuffer(ID3D12Device* device, size_t size, bool forConstantBuffer)
{
    size_t unalignedSize = size;

    // align size to the proper size, for constant buffers
    if (forConstantBuffer)
        size = ALIGN(256, size);

    Buffer* uploadBuffer = nullptr;

    // recycle one if there is one waiting
    auto it = std::find_if(free.begin(), free.end(), [&](const UploadBufferTracker::Buffer* buffer) { return buffer->size == size; });
    if (it != free.end())
    {
        uploadBuffer = *it;
        free.erase(it);
    }
    // otherwise create a new one
    else
    {
        uploadBuffer = new UploadBufferTracker::Buffer;
        uploadBuffer->size = size;
        uploadBuffer->unalignedSize = unalignedSize;

        D3D12_HEAP_PROPERTIES heapDesc = {};
        heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
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
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = device->CreateCommittedResource(&heapDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer->buffer));
        if (FAILED(hr))
            return nullptr;

        uploadBuffer->buffer->SetName(L"UploadBufferTracker");
    }

    // remember that this is in use
    inUse.push_back(uploadBuffer);

    return uploadBuffer;
}
