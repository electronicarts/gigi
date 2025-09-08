///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <d3d12.h>

#define ALIGN(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

struct UploadBufferTracker
{
    struct Buffer
    {
        ID3D12Resource* buffer = nullptr;
        size_t unalignedSize = 0;
        size_t size = 0;
        size_t age = 0;
    };

    void OnNewFrame(int maxFramesInFlight)
    {
        // advance the age of each in use buffer. Put them in the free list when it's safe to do so
        inUse.erase(
            std::remove_if(inUse.begin(), inUse.end(),
                [maxFramesInFlight, this](Buffer* buffer)
                {
                    buffer->age++;
                    if (buffer->age >= maxFramesInFlight)
                    {
                        buffer->age = 0;
                        free.push_back(buffer);
                        return true;
                    }
                    return false;
                }
            ),
            inUse.end()
        );
    }

    void Release()
    {
        for (Buffer* b : inUse)
            b->buffer->Release();
        inUse.clear();

        for (Buffer* b : free)
            b->buffer->Release();
        free.clear();
    }

    Buffer* GetBuffer(ID3D12Device* device, size_t size, bool forConstantBuffer);

    template <typename T>
    Buffer* GetBufferT(ID3D12Device* device, bool forConstantBuffer, const T& srcData)
    {
        return GetBufferT(device, forConstantBuffer, &srcData, 1);
    }

    template <typename T>
    Buffer* GetBufferT(ID3D12Device* device, bool forConstantBuffer, const T* srcData, size_t count)
    {
        Buffer* ret = GetBuffer(device, sizeof(T) * count, forConstantBuffer);
        T* data = nullptr;
        D3D12_RANGE  readRange = { 0, 0 };
        ret->buffer->Map(0, &readRange, reinterpret_cast<void**>(&data));
        memcpy(data, srcData, sizeof(T) * count);
        ret->buffer->Unmap(0, nullptr);
        return ret;
    }

    std::vector<Buffer*> inUse;
    std::vector<Buffer*> free;

    size_t getInUseSize()
    {
        return inUse.size();
    }

    size_t getFreeSize()
    {
        return free.size();

    }

};
