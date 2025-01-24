#include "../public/technique.h"
#include "DX12Utils/dxutils.h"
#include "DX12Utils/DelayedReleaseTracker.h"
#include "DX12Utils/HeapAllocationTracker.h"
#include "DX12Utils/TextureCache.h"

#include <vector>
#include <chrono>

namespace Mips_DrawCall
{
    static std::vector<Context*> s_allContexts;

    static DX12Utils::Heap                  s_srvHeap;
    static DX12Utils::Heap                  s_rtvHeap;
    static DX12Utils::Heap                  s_dsvHeap;
    static DX12Utils::UploadBufferTracker   s_ubTracker;
    static DX12Utils::DelayedReleaseTracker s_delayedRelease;
    static DX12Utils::HeapAllocationTracker s_heapAllocationTrackerRTV;
    static DX12Utils::HeapAllocationTracker s_heapAllocationTrackerDSV;

    TLogFn Context::LogFn = [] (LogLevel level, const char* msg, ...) {};
    TPerfEventBeginFn Context::PerfEventBeginFn = [] (const char* name, ID3D12GraphicsCommandList* commandList, int index) {};
    TPerfEventEndFn Context::PerfEventEndFn = [] (ID3D12GraphicsCommandList* commandList) {};

    std::wstring Context::s_techniqueLocation = L"./";
    static unsigned int s_timerIndex = 0;

    ID3D12CommandSignature* ContextInternal::s_commandSignatureDispatch = nullptr;

    template <typename T>
    T Pow2GE(const T& A)
    {
        float f = std::log2(float(A));
        f = std::ceilf(f);
        return (T)std::pow(2.0f, f);
    }

    bool CreateShared(ID3D12Device* device)
    {

        // Create heaps
        if (c_numSRVDescriptors > 0 && !CreateHeap(s_srvHeap, device, c_numSRVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, Context::LogFn))
            return false;

        if (c_numRTVDescriptors > 0 && !CreateHeap(s_rtvHeap, device, c_numRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, Context::LogFn))
            return false;

        if (c_numDSVDescriptors > 0 && !CreateHeap(s_dsvHeap, device, c_numDSVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, Context::LogFn))
            return false;

        s_heapAllocationTrackerRTV.Init(s_rtvHeap.m_heap, c_numRTVDescriptors, (int)device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
        s_heapAllocationTrackerDSV.Init(s_dsvHeap.m_heap, c_numDSVDescriptors, (int)device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

        // create indirect dispatch command
        {
            D3D12_INDIRECT_ARGUMENT_DESC dispatchArg = {};
            dispatchArg.Type						 = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

            D3D12_COMMAND_SIGNATURE_DESC dispatchDesc = {};
            dispatchDesc.ByteStride					  = sizeof(uint32_t) * 3;
            dispatchDesc.NumArgumentDescs			  = 1;
            dispatchDesc.pArgumentDescs				  = &dispatchArg;
            dispatchDesc.NodeMask					  = 0x0;

            device->CreateCommandSignature(
                &dispatchDesc,
                nullptr,
                IID_PPV_ARGS(&ContextInternal::s_commandSignatureDispatch));
        }

        return true;
    }

    void DestroyShared()
    {

        // Clear out heap trackers
        s_heapAllocationTrackerRTV.Release();
        s_heapAllocationTrackerDSV.Release();

        // Destroy Heaps
        DestroyHeap(s_srvHeap);
        DestroyHeap(s_rtvHeap);
        DestroyHeap(s_dsvHeap);

        // Destroy any upload buffers
        s_ubTracker.Release();

        // Finish any delayed release
        s_delayedRelease.Release();

        // Destroy indirect dispatch command
        if (ContextInternal::s_commandSignatureDispatch)
        {
            ContextInternal::s_commandSignatureDispatch->Release();
            ContextInternal::s_commandSignatureDispatch = nullptr;
        }
    }

    Context* CreateContext(ID3D12Device* device)
    {
        if (s_allContexts.size() == 0)
        {
            if (!CreateShared(device))
                return nullptr;
        }

        Context* ret = new Context;
        s_allContexts.push_back(ret);
        return ret;
    }

    void DestroyContext(Context* context)
    {
        s_allContexts.erase(std::remove(s_allContexts.begin(), s_allContexts.end(), context), s_allContexts.end());
        delete context;
        if (s_allContexts.size() == 0)
            DestroyShared();
    }

    ID3D12Resource* Context::GetPrimaryOutputTexture()
    {
        return nullptr;
    }

    D3D12_RESOURCE_STATES Context::GetPrimaryOutputTextureState()
    {
        return D3D12_RESOURCE_STATE_COMMON;
    }

    void OnNewFrame(int framesInFlight)
    {
        s_delayedRelease.OnNewFrame(framesInFlight);
        s_ubTracker.OnNewFrame(framesInFlight);
        s_heapAllocationTrackerRTV.OnNewFrame(framesInFlight);
        s_heapAllocationTrackerDSV.OnNewFrame(framesInFlight);
    }

    int Context::GetContextCount()
    {
        return (int)s_allContexts.size();
    }

    Context* Context::GetContext(int index)
    {
        if (index >= 0 && index < GetContextCount())
            return s_allContexts[index];
        else
            return nullptr;
    }

    ID3D12Resource* Context::CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const void* data, size_t size, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Make a buffer and have the context manage it
        ID3D12Resource* ret = DX12Utils::CreateBuffer(
            device,
            (unsigned int)size,
            flags,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_HEAP_TYPE_DEFAULT,
            debugName,
            Context::LogFn
        );
        AddManagedResource(ret);

        // Copy the data to the buffer if we should
        if (data != nullptr && size > 0)
            UploadBufferData(device, commandList, ret, D3D12_RESOURCE_STATE_COPY_DEST, data, (unsigned int)size);

        // Do a resource transition if we should
        if (desiredState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = ret;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = desiredState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // return the resource
        return ret;
    }

    ID3D12Resource* Context::CreateManagedTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, const void* initialData, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Create a texture
        ID3D12Resource* ret = DX12Utils::CreateTexture(device, size, numMips, format, flags, D3D12_RESOURCE_STATE_COPY_DEST, resourceType, debugName, Context::LogFn);
        AddManagedResource(ret);

        // copy initial data in, if we should
        if (initialData != nullptr)
        {
            DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
            UploadTextureData(device, commandList, ret, D3D12_RESOURCE_STATE_COPY_DEST, initialData, size[0] * formatInfo.bytesPerPixel);
        }

        // Put the resource into the desired state
        if (desiredState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = ret;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = desiredState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        return ret;
    }

    ID3D12Resource* Context::CreateManagedTextureAndClear(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, void* clearValue, size_t clearValueSize, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Make sure the clear value is the correct size
        DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
        if (clearValue != nullptr && clearValueSize > 0 && clearValueSize != formatInfo.bytesPerPixel)
            return nullptr;

        // Copy data into the resource
        std::vector<unsigned char> expandedClearValue;
        void* initialData = nullptr;
        if (clearValue != nullptr && clearValueSize > 0)
        {
            expandedClearValue.resize(size[0] * size[1] * size[2] * formatInfo.bytesPerPixel);
            unsigned char* dest = expandedClearValue.data();
            for (size_t i = 0; i < size[0] * size[1] * size[2]; ++i)
            {
                memcpy(dest, clearValue, formatInfo.bytesPerPixel);
                dest += formatInfo.bytesPerPixel;
            }
            initialData = expandedClearValue.data();
        }

        // make and return the texture
        return CreateManagedTexture(device, commandList, flags, format, size, numMips, resourceType, initialData, debugName, desiredState);
    }

    ID3D12Resource* Context::CreateManagedTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, DX12Utils::ResourceType resourceType, const char* fileName, bool sourceIsSRGB, unsigned int size[3], const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Get the desired channel type
        DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
        DX12Utils::TextureCache::Type desiredChannelType = DX12Utils::TextureCache::Type::U8;
        if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
            desiredChannelType = DX12Utils::TextureCache::Type::U8;
        else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)
            desiredChannelType = DX12Utils::TextureCache::Type::F32;
        else
            return nullptr;

        if (resourceType == DX12Utils::ResourceType::Texture2D)
        {
            // Load the texture and convert as necessary
            DX12Utils::TextureCache::Texture texture = DX12Utils::TextureCache::GetAs(fileName, sourceIsSRGB, desiredChannelType, formatInfo.sRGB, formatInfo.channelCount);
            if (!texture.Valid())
                return nullptr;

            // store off image properties
            size[0] = texture.width;
            size[1] = texture.height;
            size[2] = 1;

            // make and return the texture
            return CreateManagedTexture(device, commandList, flags, format, size, 1, resourceType, texture.pixels.data(), debugName, desiredState);
        }
        else if (resourceType == DX12Utils::ResourceType::Texture2DArray ||
                 resourceType == DX12Utils::ResourceType::Texture3D ||
                 resourceType == DX12Utils::ResourceType::TextureCube)
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

            bool useCubeMapNames = (resourceType == DX12Utils::ResourceType::TextureCube && strstr(fileName, "%s") != nullptr);
            bool hasPercentI = strstr(fileName, "%i") != nullptr;
            if (!useCubeMapNames && !hasPercentI)
                return nullptr;

            std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;

            // Load multiple textures
            int textureIndex = -1;
            while (1)
            {
                textureIndex++;
                char indexedFileName[1024];

                if (useCubeMapNames)
                    sprintf_s(indexedFileName, fileName, c_cubeMapNames[textureIndex]);
                else
                    sprintf_s(indexedFileName, fileName, textureIndex);

                // Load the texture and convert as necessary
                DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, sourceIsSRGB, desiredChannelType, formatInfo.sRGB, formatInfo.channelCount);
                if (!loadedTextureSlice.Valid())
                {
                    if (textureIndex == 0)
                        return nullptr;
                    break;
                }

                // make sure the textures are the same size
                if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))
                    return nullptr;

                loadedTextureSlices.push_back(loadedTextureSlice);
            }

            // store the texture size
            size[0] = loadedTextureSlices[0].width;
            size[1] = loadedTextureSlices[0].height;
            size[2] = (unsigned int)loadedTextureSlices.size();

            // gather up all pixels into a contiguous chunk of memory
            std::vector<unsigned char> allPixels;
            for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)
                allPixels.insert(allPixels.end(), texture.pixels.begin(), texture.pixels.end());

            // make and return the texture
            return CreateManagedTexture(device, commandList, flags, format, size, 1, resourceType, allPixels.data(), debugName, desiredState);
        }
        else
            return nullptr;
    }

    void Context::UploadTextureData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* texture, D3D12_RESOURCE_STATES textureState, const void* data, unsigned int unalignedPitch)
    {
        // Get information about the texture
        int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);
        D3D12_RESOURCE_DESC textureDesc = texture->GetDesc();

        // transition the resource to copy dest if it isn't already
        if (textureState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = texture;
            barrier.Transition.StateBefore = textureState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // 3d textures do a single copy because it's a single sub resource.
        if (textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            // Get the upload buffer
            DX12Utils::UploadBufferTracker::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * textureDesc.Height * textureDesc.DepthOrArraySize, Context::LogFn, false);

            // Map, copy, unmap
            {
                unsigned char* dest = nullptr;
                D3D12_RANGE readRange = { 0, 0 };
                HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);
                if (FAILED(hr))
                {
                    Context::LogFn(LogLevel::Error, "Could not map upload buffer.");
                }
                else
                {
                    const unsigned char* src = (const unsigned char*)data;
                    for (int iz = 0; iz < textureDesc.DepthOrArraySize; ++iz)
                    {
                        for (int iy = 0; iy < (int)textureDesc.Height; ++iy)
                        {
                            memcpy(dest, src, unalignedPitch);
                            src += unalignedPitch;
                            dest += alignedPitch;
                        }
                    }

                    uploadBuffer->buffer->Unmap(0, nullptr);
                }
            }

            // copy the upload buffer into the texture
            {
                unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;
                device->GetCopyableFootprints(&textureDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

                D3D12_TEXTURE_COPY_LOCATION src = {};
                src.pResource = uploadBuffer->buffer;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = *layout;

                D3D12_TEXTURE_COPY_LOCATION dest = {};
                dest.pResource = texture;
                dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dest.SubresourceIndex = 0;

                commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
            }
        }
        // 2d array textures do a copy for each slice
        else if (textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            for (int iz = 0; iz < textureDesc.DepthOrArraySize; ++iz)
            {
                // Get the upload buffer
                DX12Utils::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * textureDesc.Height, Context::LogFn, false);

                // Map, copy, unmap
                {
                    unsigned char* dest = nullptr;
                    D3D12_RANGE readRange = { 0, 0 };
                    HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);
                    if (FAILED(hr))
                    {
                        Context::LogFn(LogLevel::Error, "Could not map upload buffer.");
                    }
                    else
                    {
                        const unsigned char* src = &((const unsigned char*)data)[unalignedPitch * textureDesc.Height * iz];
                        for (int iy = 0; iy < (int)textureDesc.Height; ++iy)
                        {
                            memcpy(dest, src, unalignedPitch);
                            src += unalignedPitch;
                            dest += alignedPitch;
                        }

                        uploadBuffer->buffer->Unmap(0, nullptr);
                    }
                }

                 // copy the upload buffer into the texture
                 {
                     unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
                     D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;
                     device->GetCopyableFootprints(&textureDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

                     D3D12_TEXTURE_COPY_LOCATION src = {};
                     src.pResource = uploadBuffer->buffer;
                     src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                     src.PlacedFootprint = *layout;

                     D3D12_TEXTURE_COPY_LOCATION dest = {};
                     dest.pResource = texture;
                     dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                     dest.SubresourceIndex = iz;

                     commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
                 }
            }
        }
        else
        {
            Context::LogFn(LogLevel::Error, "Unhandled texture dimension.");
        }

        // transition the resource back to what it was
        if (textureState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = texture;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = textureState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }
    }

    void Context::UploadBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* buffer, D3D12_RESOURCE_STATES bufferState, const void* data, unsigned int dataSize)
    {
        // Get the upload buffer
        DX12Utils::UploadBufferTracker::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, dataSize, Context::LogFn, false);

        // copy cpu data to the upload buffer
        {
            void* start = nullptr;
            HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&start));
            if(hr)
            {
                Context::LogFn(LogLevel::Error, "Could not map upload buffer");
                return;
            }

            memcpy(start, data, dataSize);

            uploadBuffer->buffer->Unmap(0, nullptr);
        }

        // transition the resource to copy dest if it isn't already
        if (bufferState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = buffer;
            barrier.Transition.StateBefore = bufferState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // copy the resource
        commandList->CopyResource(buffer, uploadBuffer->buffer);

        // transition the resource back to what it was
        if (bufferState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = buffer;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = bufferState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }
    }

    int Context::GetRTV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_RTV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName)
    {
        // Make the key
        DX12Utils::SubResourceHeapAllocationInfo key;
        key.resource = resource;
        key.arrayIndex = arrayIndex;
        key.mipIndex = mipIndex;

        // If it already exists, use it
        auto it = m_internal.m_RTVCache.find(key);
        if (it != m_internal.m_RTVCache.end())
            return it->second;

        // Allocate an RTV index
        int rtvIndex = -1;
        if (!s_heapAllocationTrackerRTV.Allocate(rtvIndex, debugName))
            return -1;

        // Create the RTV
        if (!DX12Utils::CreateRTV(device, resource, s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), format, dimension, arrayIndex, mipIndex))
        {
            s_heapAllocationTrackerRTV.Free(rtvIndex);
            return -1;
        }

        // store the result
        m_internal.m_RTVCache[key] = rtvIndex;
        return rtvIndex;
    }

    int Context::GetDSV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_DSV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName)
    {
	    // Make the key
        DX12Utils::SubResourceHeapAllocationInfo key;
        key.resource = resource;
        key.arrayIndex = arrayIndex;
        key.mipIndex = mipIndex;

	    // If it already exists, use it
	    auto it = m_internal.m_DSVCache.find(key);
	    if (it != m_internal.m_DSVCache.end())
            return it->second;

        // Allocate a DSV index
        int dsvIndex = -1;
        if (!s_heapAllocationTrackerDSV.Allocate(dsvIndex, debugName))
            return -1;

        // Create the DSV
        if (!DX12Utils::CreateDSV(device, resource, s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), format, dimension, arrayIndex, mipIndex))
        {
            s_heapAllocationTrackerDSV.Free(dsvIndex);
            return -1;
        }

        // store the result
        m_internal.m_DSVCache[key] = dsvIndex;
        return dsvIndex;
    }

    const ProfileEntry* Context::ReadbackProfileData(ID3D12CommandQueue* commandQueue, int& numItems)
    {
        numItems = 0;

        if (!m_profile || !m_internal.m_TimestampReadbackBuffer)
            return nullptr;

        uint64_t GPUFrequency;
        commandQueue->GetTimestampFrequency(&GPUFrequency);
        double GPUTickDelta = 1.0 / static_cast<double>(GPUFrequency);

        D3D12_RANGE range;
        range.Begin = 0;
        range.End = ((10 + 1) * 2) * sizeof(uint64_t);

        uint64_t* timeStampBuffer = nullptr;
        m_internal.m_TimestampReadbackBuffer->Map(0, &range, (void**)&timeStampBuffer);

        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_0
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_1
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_2
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_3
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_4
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_5
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_6
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_7
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Rasterize_Mip_8
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: Draw_Sphere
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+1] - timeStampBuffer[0])); numItems++; // GPU total

        D3D12_RANGE emptyRange = {};
        m_internal.m_TimestampReadbackBuffer->Unmap(0, &emptyRange);

        return m_profileData;
    }

    Context::~Context()
    {
        for (const auto& pair : m_internal.m_RTVCache)
            s_heapAllocationTrackerRTV.Free(pair.second);
        m_internal.m_RTVCache.clear();

        for (const auto& pair : m_internal.m_DSVCache)
            s_heapAllocationTrackerDSV.Free(pair.second);
        m_internal.m_DSVCache.clear();

        for (ID3D12Resource* resource : m_internal.m_managedResources)
            resource->Release();
        m_internal.m_managedResources.clear();

        if(m_internal.m_TimestampQueryHeap)
        {
            m_internal.m_TimestampQueryHeap->Release();
            m_internal.m_TimestampQueryHeap = nullptr;
        }

        if(m_internal.m_TimestampReadbackBuffer)
        {
            m_internal.m_TimestampReadbackBuffer->Release();
            m_internal.m_TimestampReadbackBuffer = nullptr;
        }

        if(m_internal.texture_Color_Buffer)
        {
            s_delayedRelease.Add(m_internal.texture_Color_Buffer);
            m_internal.texture_Color_Buffer = nullptr;
        }

        if(m_internal.texture_Depth_Buffer)
        {
            s_delayedRelease.Add(m_internal.texture_Depth_Buffer);
            m_internal.texture_Depth_Buffer = nullptr;
        }

        if(m_output.texture_Final_Color_Buffer)
        {
            s_delayedRelease.Add(m_output.texture_Final_Color_Buffer);
            m_output.texture_Final_Color_Buffer = nullptr;
        }

        if(m_output.texture_Final_Depth_Buffer)
        {
            s_delayedRelease.Add(m_output.texture_Final_Depth_Buffer);
            m_output.texture_Final_Depth_Buffer = nullptr;
        }

        // _VertexShaderCB
        if (m_internal.constantBuffer__VertexShaderCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__VertexShaderCB);
            m_internal.constantBuffer__VertexShaderCB = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_0_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_0_pso);
            m_internal.drawCall_Rasterize_Mip_0_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_0_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_0_rootSig);
            m_internal.drawCall_Rasterize_Mip_0_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_1_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_1_pso);
            m_internal.drawCall_Rasterize_Mip_1_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_1_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_1_rootSig);
            m_internal.drawCall_Rasterize_Mip_1_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_2_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_2_pso);
            m_internal.drawCall_Rasterize_Mip_2_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_2_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_2_rootSig);
            m_internal.drawCall_Rasterize_Mip_2_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_3_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_3_pso);
            m_internal.drawCall_Rasterize_Mip_3_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_3_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_3_rootSig);
            m_internal.drawCall_Rasterize_Mip_3_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_4_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_4_pso);
            m_internal.drawCall_Rasterize_Mip_4_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_4_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_4_rootSig);
            m_internal.drawCall_Rasterize_Mip_4_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_5_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_5_pso);
            m_internal.drawCall_Rasterize_Mip_5_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_5_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_5_rootSig);
            m_internal.drawCall_Rasterize_Mip_5_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_6_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_6_pso);
            m_internal.drawCall_Rasterize_Mip_6_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_6_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_6_rootSig);
            m_internal.drawCall_Rasterize_Mip_6_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_7_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_7_pso);
            m_internal.drawCall_Rasterize_Mip_7_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_7_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_7_rootSig);
            m_internal.drawCall_Rasterize_Mip_7_rootSig = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_8_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_8_pso);
            m_internal.drawCall_Rasterize_Mip_8_pso = nullptr;
        }

        if(m_internal.drawCall_Rasterize_Mip_8_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_8_rootSig);
            m_internal.drawCall_Rasterize_Mip_8_rootSig = nullptr;
        }

        // _VertexShaderSphereCB
        if (m_internal.constantBuffer__VertexShaderSphereCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__VertexShaderSphereCB);
            m_internal.constantBuffer__VertexShaderSphereCB = nullptr;
        }

        // _PixelShaderSphereCB
        if (m_internal.constantBuffer__PixelShaderSphereCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__PixelShaderSphereCB);
            m_internal.constantBuffer__PixelShaderSphereCB = nullptr;
        }

        if(m_internal.drawCall_Draw_Sphere_pso)
        {
            s_delayedRelease.Add(m_internal.drawCall_Draw_Sphere_pso);
            m_internal.drawCall_Draw_Sphere_pso = nullptr;
        }

        if(m_internal.drawCall_Draw_Sphere_rootSig)
        {
            s_delayedRelease.Add(m_internal.drawCall_Draw_Sphere_rootSig);
            m_internal.drawCall_Draw_Sphere_rootSig = nullptr;
        }
    }

    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        // reset the timer index
        s_timerIndex = 0;

        ScopedPerfEvent scopedPerf("Mips_DrawCall", commandList, 19);

        std::chrono::high_resolution_clock::time_point startPointCPUTechnique;
        if(context->m_profile)
        {
            startPointCPUTechnique = std::chrono::high_resolution_clock::now();
            if(context->m_internal.m_TimestampQueryHeap == nullptr)
            {
                D3D12_QUERY_HEAP_DESC QueryHeapDesc;
                QueryHeapDesc.Count = (10+1) * 2;
                QueryHeapDesc.NodeMask = 1;
                QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&context->m_internal.m_TimestampQueryHeap));
                if (c_debugNames)
                    context->m_internal.m_TimestampQueryHeap->SetName(L"Mips_DrawCall Time Stamp Query Heap");

                context->m_internal.m_TimestampReadbackBuffer = DX12Utils::CreateBuffer(device, sizeof(uint64_t) * (10+1) * 2, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (c_debugNames ? L"Mips_DrawCall Time Stamp Query Heap" : nullptr), nullptr);
            }
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
        }

        if (!context->m_input.buffer_VertexBuffer)
        {
            Context::LogFn(LogLevel::Error, "Mips_DrawCall: Imported buffer \"VertexBuffer\" is null.\n");
            return;
        }

        if (context->m_input.buffer_VertexBuffer_vertexInputLayout.size() == 0)
        {
            Context::LogFn(LogLevel::Error, "Mips_DrawCall: Imported buffer \"VertexBuffer\" is used as a vertex buffer but no vertexInputLayout was given.\n");
            return;
        }

        if (!context->m_input.buffer_SphereVB)
        {
            Context::LogFn(LogLevel::Error, "Mips_DrawCall: Imported buffer \"SphereVB\" is null.\n");
            return;
        }

        if (context->m_input.buffer_SphereVB_vertexInputLayout.size() == 0)
        {
            Context::LogFn(LogLevel::Error, "Mips_DrawCall: Imported buffer \"SphereVB\" is used as a vertex buffer but no vertexInputLayout was given.\n");
            return;
        }

        // Make sure internally owned resources are created and are the right size and format
        context->EnsureResourcesCreated(device, commandList);

        // set the heaps
        ID3D12DescriptorHeap* heaps[] =
        {
            s_srvHeap.m_heap,
        };
        commandList->SetDescriptorHeaps(_countof(heaps), heaps);

        // Make sure imported resources are in the correct state
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[2];

            if(context->m_input.buffer_VertexBuffer_state != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_VertexBuffer;
                barriers[barrierCount].Transition.StateBefore = context->m_input.buffer_VertexBuffer_state;
                barriers[barrierCount].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(context->m_input.buffer_SphereVB_state != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_SphereVB;
                barriers[barrierCount].Transition.StateBefore = context->m_input.buffer_SphereVB_state;
                barriers[barrierCount].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        // Shader Constants: _VertexShaderCB
        {
            context->m_internal.constantBuffer__VertexShaderCB_cpu.ViewProjMtx = context->m_input.variable_ViewProjMtx;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__VertexShaderCB, context->m_internal.constantBuffer__VertexShaderCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_internal.texture_Color_Buffer;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, barriers);
        }

        // Draw Call: Rasterize_Mip_0
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_0", commandList, 0);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_0_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_0_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 0, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 0, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = context->m_internal.texture_Color_Buffer_size[0];
            int renderHeight = context->m_internal.texture_Color_Buffer_size[1];

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 0, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 0, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_0";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_1
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_1", commandList, 4);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_1_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_1_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 1, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 1, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 2, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 2, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 1, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 1, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_1";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_2
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_2", commandList, 5);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_2_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_2_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 2, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 2, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 4, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 4, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 2, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 2, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_2";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_3
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_3", commandList, 6);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_3_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_3_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 3, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 3, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 8, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 8, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 3, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 3, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_3";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_4
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_4", commandList, 7);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_4_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_4_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 4, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 4, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 16, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 16, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 4, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 4, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_4";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_5
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_5", commandList, 8);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_5_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_5_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 5, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 5, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 32, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 32, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 5, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 5, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_5";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_6
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_6", commandList, 9);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_6_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_6_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 6, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 6, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 64, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 64, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 6, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 6, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_6";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_7
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_7", commandList, 10);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_7_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_7_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 7, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 7, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 128, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 128, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 7, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 7, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_7";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Draw Call: Rasterize_Mip_8
        {
            ScopedPerfEvent scopedPerf("Draw Call: Rasterize_Mip_8", commandList, 11);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Rasterize_Mip_8_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Rasterize_Mip_8_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_VertexBuffer_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_VertexBuffer->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_VertexBuffer_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_VertexBuffer_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_VertexBuffer_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_VertexBuffer_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Color_Buffer
            {
                 float clearValues[4] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 8, "Mips_DrawCall.Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 8, "Mips_DrawCall.Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = max(context->m_internal.texture_Color_Buffer_size[0] / 256, 1);
            int renderHeight = max(context->m_internal.texture_Color_Buffer_size[1] / 256, 1);

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY, 0, 8, "Mips_DrawCall.Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_internal.texture_Depth_Buffer, context->m_internal.texture_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY, 0, 8, "Mips_DrawCall.Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Rasterize_Mip_8";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _VertexShaderSphereCB
        {
            context->m_internal.constantBuffer__VertexShaderSphereCB_cpu.ViewProjMtx = context->m_input.variable_ViewProjMtx;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__VertexShaderSphereCB, context->m_internal.constantBuffer__VertexShaderSphereCB_cpu, Context::LogFn);
        }

        // Shader Constants: _PixelShaderSphereCB
        {
            context->m_internal.constantBuffer__PixelShaderSphereCB_cpu.UseMips = context->m_input.variable_UseMips;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__PixelShaderSphereCB, context->m_internal.constantBuffer__PixelShaderSphereCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_internal.texture_Color_Buffer;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, barriers);
        }

        // Draw Call: Draw_Sphere
        {
            ScopedPerfEvent scopedPerf("Draw Call: Draw_Sphere", commandList, 14);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_Draw_Sphere_rootSig);
            commandList->SetPipelineState(context->m_internal.drawCall_Draw_Sphere_pso);

            DX12Utils::ResourceDescriptor descriptorsVS[] =
            {
                { context->m_internal.constantBuffer__VertexShaderSphereCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, 1, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(0, descriptorTableVS);

            DX12Utils::ResourceDescriptor descriptorsPS[] =
            {
                { context->m_internal.texture_Color_Buffer, context->m_internal.texture_Color_Buffer_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.constantBuffer__PixelShaderSphereCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 },
            };
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTablePS = GetDescriptorTable(device, s_srvHeap, descriptorsPS, 2, Context::LogFn);
            commandList->SetGraphicsRootDescriptorTable(1, descriptorTablePS);

            // Vertex Buffer
            int vertexCountPerInstance = context->m_input.buffer_SphereVB_count;

            D3D12_VERTEX_BUFFER_VIEW vbView;
            vbView.BufferLocation = context->m_input.buffer_SphereVB->GetGPUVirtualAddress();
            vbView.StrideInBytes = (context->m_input.buffer_SphereVB_format == DXGI_FORMAT_UNKNOWN)
                ? context->m_input.buffer_SphereVB_stride
                : DX12Utils::Get_DXGI_FORMAT_Info(context->m_input.buffer_SphereVB_format, Context::LogFn).bytesPerPixel;
            vbView.SizeInBytes = vbView.StrideInBytes * context->m_input.buffer_SphereVB_count;

            commandList->IASetVertexBuffers(0, 1, &vbView);

            // Index Buffer
            int indexCountPerInstance = -1;

            // Instance Buffer
            int instanceCount = 1;

            // Clear Final_Color_Buffer
            {
                 float clearValues[4] = { 0.500000f, 0.500000f, 0.500000f, 1.000000f };
                 int rtvIndex = context->GetRTV(device, context->m_output.texture_Final_Color_Buffer, context->m_output.texture_Final_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2D, 0, 0, "Mips_DrawCall.Final_Color_Buffer");
                 if (rtvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get RTV for Mips_DrawCall.Final_Color_Buffer");
                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), clearValues, 0, nullptr);
            }

            // Clear Final_Depth_Buffer
            {
                 D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                 int dsvIndex = context->GetDSV(device, context->m_output.texture_Final_Depth_Buffer, context->m_output.texture_Final_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2D, 0, 0, "Mips_DrawCall.Final_Depth_Buffer");
                 if (dsvIndex == -1)
                     Context::LogFn(LogLevel::Error, "Could not get DSV for Mips_DrawCall.Final_Depth_Buffer");
                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), clearFlags, 0.000000f, 0, 0, nullptr);
            }

            int renderWidth = context->m_output.texture_Final_Color_Buffer_size[0];
            int renderHeight = context->m_output.texture_Final_Color_Buffer_size[1];

            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] =
            {
                s_heapAllocationTrackerRTV.GetCPUHandle(context->GetRTV(device, context->m_output.texture_Final_Color_Buffer, context->m_output.texture_Final_Color_Buffer_format, D3D12_RTV_DIMENSION_TEXTURE2D, 0, 0, "Mips_DrawCall.Final_Color_Buffer"))
            };

            int colorTargetHandleCount = _countof(colorTargetHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->GetDSV(device, context->m_output.texture_Final_Depth_Buffer, context->m_output.texture_Final_Depth_Buffer_format, D3D12_DSV_DIMENSION_TEXTURE2D, 0, 0, "Mips_DrawCall.Final_Depth_Buffer"));
            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->OMSetStencilRef(0);

            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);

            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Draw_Sphere";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Make sure imported resources are put back in the state they were given to us in
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[2];

            if(context->m_input.buffer_VertexBuffer_state != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_VertexBuffer;
                barriers[barrierCount].Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                barriers[barrierCount].Transition.StateAfter = context->m_input.buffer_VertexBuffer_state;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(context->m_input.buffer_SphereVB_state != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_SphereVB;
                barriers[barrierCount].Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                barriers[barrierCount].Transition.StateAfter = context->m_input.buffer_SphereVB_state;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        if(context->m_profile)
        {
            context->m_profileData[(s_timerIndex-1)/2].m_label = "Total";
            context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPUTechnique).count();
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            commandList->ResolveQueryData(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, s_timerIndex, context->m_internal.m_TimestampReadbackBuffer, 0);
        }
    }

    void Context::EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        bool dirty = false;

        // Color_Buffer
        {
            unsigned int baseSize[3] = { 1, 1, 1 };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 400) / 1 + 0,
                ((baseSize[1] + 0) * 400) / 1 + 0,
                ((baseSize[2] + 0) * 2) / 1 + 0
            };

            unsigned int desiredNumMips = 1;
            {
                int maxSize = max(desiredSize[0], desiredSize[1]);
                while (maxSize > 1)
                {
                    maxSize /= 2;
                    desiredNumMips++;
                }
            }

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            if(!m_internal.texture_Color_Buffer ||
               m_internal.texture_Color_Buffer_size[0] != desiredSize[0] ||
               m_internal.texture_Color_Buffer_size[1] != desiredSize[1] ||
               m_internal.texture_Color_Buffer_size[2] != desiredSize[2] ||
               m_internal.texture_Color_Buffer_numMips != desiredNumMips ||
               m_internal.texture_Color_Buffer_format != desiredFormat)
            {
                dirty = true;
                if(m_internal.texture_Color_Buffer)
                    s_delayedRelease.Add(m_internal.texture_Color_Buffer);

                m_internal.texture_Color_Buffer = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_internal.texture_Color_Buffer_flags, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, DX12Utils::ResourceType::Texture2DArray, (c_debugNames ? L"Color_Buffer" : nullptr), Context::LogFn);
                m_internal.texture_Color_Buffer_size[0] = desiredSize[0];
                m_internal.texture_Color_Buffer_size[1] = desiredSize[1];
                m_internal.texture_Color_Buffer_size[2] = desiredSize[2];
                m_internal.texture_Color_Buffer_numMips = desiredNumMips;
                m_internal.texture_Color_Buffer_format = desiredFormat;
            }
        }

        // Depth_Buffer
        {
            unsigned int baseSize[3] = {
                m_internal.texture_Color_Buffer_size[0],
                m_internal.texture_Color_Buffer_size[1],
                m_internal.texture_Color_Buffer_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            unsigned int desiredNumMips = 1;
            {
                int maxSize = max(desiredSize[0], desiredSize[1]);
                while (maxSize > 1)
                {
                    maxSize /= 2;
                    desiredNumMips++;
                }
            }

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_D32_FLOAT;

            if(!m_internal.texture_Depth_Buffer ||
               m_internal.texture_Depth_Buffer_size[0] != desiredSize[0] ||
               m_internal.texture_Depth_Buffer_size[1] != desiredSize[1] ||
               m_internal.texture_Depth_Buffer_size[2] != desiredSize[2] ||
               m_internal.texture_Depth_Buffer_numMips != desiredNumMips ||
               m_internal.texture_Depth_Buffer_format != desiredFormat)
            {
                dirty = true;
                if(m_internal.texture_Depth_Buffer)
                    s_delayedRelease.Add(m_internal.texture_Depth_Buffer);

                m_internal.texture_Depth_Buffer = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_internal.texture_Depth_Buffer_flags, D3D12_RESOURCE_STATE_DEPTH_WRITE, DX12Utils::ResourceType::Texture2DArray, (c_debugNames ? L"Depth_Buffer" : nullptr), Context::LogFn);
                m_internal.texture_Depth_Buffer_size[0] = desiredSize[0];
                m_internal.texture_Depth_Buffer_size[1] = desiredSize[1];
                m_internal.texture_Depth_Buffer_size[2] = desiredSize[2];
                m_internal.texture_Depth_Buffer_numMips = desiredNumMips;
                m_internal.texture_Depth_Buffer_format = desiredFormat;
            }
        }

        // Final_Color_Buffer
        {

            unsigned int baseSize[3] = { (unsigned int)m_input.variable_RenderSize[0], (unsigned int)m_input.variable_RenderSize[1], 1 };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            unsigned int desiredNumMips = 1;
            {
                int maxSize = max(desiredSize[0], desiredSize[1]);
                while (maxSize > 1)
                {
                    maxSize /= 2;
                    desiredNumMips++;
                }
            }

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            if(!m_output.texture_Final_Color_Buffer ||
               m_output.texture_Final_Color_Buffer_size[0] != desiredSize[0] ||
               m_output.texture_Final_Color_Buffer_size[1] != desiredSize[1] ||
               m_output.texture_Final_Color_Buffer_size[2] != desiredSize[2] ||
               m_output.texture_Final_Color_Buffer_numMips != desiredNumMips ||
               m_output.texture_Final_Color_Buffer_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_Final_Color_Buffer)
                    s_delayedRelease.Add(m_output.texture_Final_Color_Buffer);

                m_output.texture_Final_Color_Buffer = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_Final_Color_Buffer_flags, D3D12_RESOURCE_STATE_RENDER_TARGET, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"Final_Color_Buffer" : nullptr), Context::LogFn);
                m_output.texture_Final_Color_Buffer_size[0] = desiredSize[0];
                m_output.texture_Final_Color_Buffer_size[1] = desiredSize[1];
                m_output.texture_Final_Color_Buffer_size[2] = desiredSize[2];
                m_output.texture_Final_Color_Buffer_numMips = desiredNumMips;
                m_output.texture_Final_Color_Buffer_format = desiredFormat;
            }
        }

        // Final_Depth_Buffer
        {

            unsigned int baseSize[3] = { (unsigned int)m_input.variable_RenderSize[0], (unsigned int)m_input.variable_RenderSize[1], 1 };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            unsigned int desiredNumMips = 1;
            {
                int maxSize = max(desiredSize[0], desiredSize[1]);
                while (maxSize > 1)
                {
                    maxSize /= 2;
                    desiredNumMips++;
                }
            }

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_D32_FLOAT;

            if(!m_output.texture_Final_Depth_Buffer ||
               m_output.texture_Final_Depth_Buffer_size[0] != desiredSize[0] ||
               m_output.texture_Final_Depth_Buffer_size[1] != desiredSize[1] ||
               m_output.texture_Final_Depth_Buffer_size[2] != desiredSize[2] ||
               m_output.texture_Final_Depth_Buffer_numMips != desiredNumMips ||
               m_output.texture_Final_Depth_Buffer_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_Final_Depth_Buffer)
                    s_delayedRelease.Add(m_output.texture_Final_Depth_Buffer);

                m_output.texture_Final_Depth_Buffer = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_Final_Depth_Buffer_flags, D3D12_RESOURCE_STATE_DEPTH_WRITE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"Final_Depth_Buffer" : nullptr), Context::LogFn);
                m_output.texture_Final_Depth_Buffer_size[0] = desiredSize[0];
                m_output.texture_Final_Depth_Buffer_size[1] = desiredSize[1];
                m_output.texture_Final_Depth_Buffer_size[2] = desiredSize[2];
                m_output.texture_Final_Depth_Buffer_numMips = desiredNumMips;
                m_output.texture_Final_Depth_Buffer_format = desiredFormat;
            }
        }

        // _VertexShaderCB
        if (m_internal.constantBuffer__VertexShaderCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__VertexShaderCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_VertexShaderCB" : nullptr), Context::LogFn);
        }

        // _VertexShaderSphereCB
        if (m_internal.constantBuffer__VertexShaderSphereCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__VertexShaderSphereCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_VertexShaderSphereCB" : nullptr), Context::LogFn);
        }

        // _PixelShaderSphereCB
        if (m_internal.constantBuffer__PixelShaderSphereCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__PixelShaderSphereCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_PixelShaderSphereCB" : nullptr), Context::LogFn);
        }
        EnsureDrawCallPSOsCreated(device, dirty);
    }

    bool Context::EnsureDrawCallPSOsCreated(ID3D12Device* device, bool dirty)
    {

        // Draw Call: Rasterize_Mip_0
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_0_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_0_pso);
                m_internal.drawCall_Rasterize_Mip_0_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_0_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_0_rootSig);
                m_internal.drawCall_Rasterize_Mip_0_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_0_pso || !m_internal.drawCall_Rasterize_Mip_0_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_0_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_0_rootSig->SetName(L"Rasterize_Mip_0");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_0_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_0_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_0");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_0_pso->SetName(L"Rasterize_Mip_0");
        }

        // Draw Call: Rasterize_Mip_1
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_1_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_1_pso);
                m_internal.drawCall_Rasterize_Mip_1_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_1_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_1_rootSig);
                m_internal.drawCall_Rasterize_Mip_1_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_1_pso || !m_internal.drawCall_Rasterize_Mip_1_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_1_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_1_rootSig->SetName(L"Rasterize_Mip_1");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_1_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_1_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_1");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_1_pso->SetName(L"Rasterize_Mip_1");
        }

        // Draw Call: Rasterize_Mip_2
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_2_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_2_pso);
                m_internal.drawCall_Rasterize_Mip_2_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_2_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_2_rootSig);
                m_internal.drawCall_Rasterize_Mip_2_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_2_pso || !m_internal.drawCall_Rasterize_Mip_2_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_2_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_2_rootSig->SetName(L"Rasterize_Mip_2");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_2_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_2_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_2");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_2_pso->SetName(L"Rasterize_Mip_2");
        }

        // Draw Call: Rasterize_Mip_3
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_3_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_3_pso);
                m_internal.drawCall_Rasterize_Mip_3_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_3_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_3_rootSig);
                m_internal.drawCall_Rasterize_Mip_3_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_3_pso || !m_internal.drawCall_Rasterize_Mip_3_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_3_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_3_rootSig->SetName(L"Rasterize_Mip_3");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_3_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_3_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_3");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_3_pso->SetName(L"Rasterize_Mip_3");
        }

        // Draw Call: Rasterize_Mip_4
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_4_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_4_pso);
                m_internal.drawCall_Rasterize_Mip_4_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_4_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_4_rootSig);
                m_internal.drawCall_Rasterize_Mip_4_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_4_pso || !m_internal.drawCall_Rasterize_Mip_4_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_4_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_4_rootSig->SetName(L"Rasterize_Mip_4");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_4_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_4_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_4");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_4_pso->SetName(L"Rasterize_Mip_4");
        }

        // Draw Call: Rasterize_Mip_5
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_5_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_5_pso);
                m_internal.drawCall_Rasterize_Mip_5_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_5_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_5_rootSig);
                m_internal.drawCall_Rasterize_Mip_5_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_5_pso || !m_internal.drawCall_Rasterize_Mip_5_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_5_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_5_rootSig->SetName(L"Rasterize_Mip_5");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_5_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_5_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_5");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_5_pso->SetName(L"Rasterize_Mip_5");
        }

        // Draw Call: Rasterize_Mip_6
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_6_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_6_pso);
                m_internal.drawCall_Rasterize_Mip_6_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_6_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_6_rootSig);
                m_internal.drawCall_Rasterize_Mip_6_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_6_pso || !m_internal.drawCall_Rasterize_Mip_6_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_6_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_6_rootSig->SetName(L"Rasterize_Mip_6");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_6_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_6_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_6");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_6_pso->SetName(L"Rasterize_Mip_6");
        }

        // Draw Call: Rasterize_Mip_7
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_7_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_7_pso);
                m_internal.drawCall_Rasterize_Mip_7_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_7_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_7_rootSig);
                m_internal.drawCall_Rasterize_Mip_7_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_7_pso || !m_internal.drawCall_Rasterize_Mip_7_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_7_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_7_rootSig->SetName(L"Rasterize_Mip_7");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_7_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_7_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_7");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_7_pso->SetName(L"Rasterize_Mip_7");
        }

        // Draw Call: Rasterize_Mip_8
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Rasterize_Mip_8_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_8_pso);
                m_internal.drawCall_Rasterize_Mip_8_pso = nullptr;
            }
            if (m_internal.drawCall_Rasterize_Mip_8_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Rasterize_Mip_8_rootSig);
                m_internal.drawCall_Rasterize_Mip_8_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Rasterize_Mip_8_pso || !m_internal.drawCall_Rasterize_Mip_8_rootSig)
        {

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParams[1];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 1;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 0;
            rootDesc.pStaticSamplers = nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_8_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_8_rootSig->SetName(L"Rasterize_Mip_8");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCall_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_VertexBuffer_vertexInputLayout.begin(), m_input.buffer_VertexBuffer_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Rasterize_Mip_8_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_internal.texture_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_internal.texture_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Rasterize_Mip_8_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Rasterize_Mip_8");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Rasterize_Mip_8_pso->SetName(L"Rasterize_Mip_8");
        }

        // Draw Call: Draw_Sphere
        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed.
        {
            if (m_internal.drawCall_Draw_Sphere_pso)
            {
                s_delayedRelease.Add(m_internal.drawCall_Draw_Sphere_pso);
                m_internal.drawCall_Draw_Sphere_pso = nullptr;
            }
            if (m_internal.drawCall_Draw_Sphere_rootSig)
            {
                s_delayedRelease.Add(m_internal.drawCall_Draw_Sphere_rootSig);
                m_internal.drawCall_Draw_Sphere_rootSig = nullptr;
            }
        }
        if(!m_internal.drawCall_Draw_Sphere_pso || !m_internal.drawCall_Draw_Sphere_rootSig)
        {
            D3D12_STATIC_SAMPLER_DESC samplers[2];

            // LinearWrap (PS)
            samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplers[0].MipLODBias  = 0;
            samplers[0].MaxAnisotropy  = 0;
            samplers[0].ComparisonFunc  = D3D12_COMPARISON_FUNC_NEVER;
            samplers[0].BorderColor  = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            samplers[0].MinLOD = 0.0f;
            samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
            samplers[0].ShaderRegister = 0;
            samplers[0].RegisterSpace = 1;
            samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            // PointWrap (PS)
            samplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplers[1].MipLODBias  = 0;
            samplers[1].MaxAnisotropy  = 0;
            samplers[1].ComparisonFunc  = D3D12_COMPARISON_FUNC_NEVER;
            samplers[1].BorderColor  = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            samplers[1].MinLOD = 0.0f;
            samplers[1].MaxLOD = D3D12_FLOAT32_MAX;
            samplers[1].ShaderRegister = 1;
            samplers[1].RegisterSpace = 1;
            samplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_DESCRIPTOR_RANGE rangesVertex[1];

            // _VertexShaderSphereCB
            rangesVertex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesVertex[0].NumDescriptors = 1;
            rangesVertex[0].BaseShaderRegister = 0;
            rangesVertex[0].RegisterSpace = 0;
            rangesVertex[0].OffsetInDescriptorsFromTableStart = 0;

            D3D12_DESCRIPTOR_RANGE rangesPixel[2];

            // MipTex
            rangesPixel[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            rangesPixel[0].NumDescriptors = 1;
            rangesPixel[0].BaseShaderRegister = 0;
            rangesPixel[0].RegisterSpace = 0;
            rangesPixel[0].OffsetInDescriptorsFromTableStart = 0;

            // _PixelShaderSphereCB
            rangesPixel[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rangesPixel[1].NumDescriptors = 1;
            rangesPixel[1].BaseShaderRegister = 0;
            rangesPixel[1].RegisterSpace = 0;
            rangesPixel[1].OffsetInDescriptorsFromTableStart = 1;

            D3D12_ROOT_PARAMETER rootParams[2];

            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
            rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[0].DescriptorTable.pDescriptorRanges = rangesVertex;

            rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            rootParams[1].DescriptorTable.NumDescriptorRanges = 2;
            rootParams[1].DescriptorTable.pDescriptorRanges = rangesPixel;

            // Root desc
            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = 2;
            rootDesc.pParameters = rootParams;
            rootDesc.NumStaticSamplers = 2;
            rootDesc.pStaticSamplers = samplers;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not serialize root signature : %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            char* sigptr = (char*)sig->GetBufferPointer();
            auto sigsize = sig->GetBufferSize();

            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_Draw_Sphere_rootSig));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    Context::LogFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            if (c_debugNames)
                m_internal.drawCall_Draw_Sphere_rootSig->SetName(L"Draw_Sphere");

            D3D_SHADER_MACRO* definesVS = nullptr;

            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCallSphere_VS.hlsl", "VSMain", "vs_6_1", definesVS, c_debugShaders, Context::LogFn);
            if (byteCodeVS.size() == 0)
                return false;

            D3D_SHADER_MACRO* definesPS = nullptr;

            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/Mips_DrawCallSphere_PS.hlsl", "PSMain", "ps_6_1", definesPS, c_debugShaders, Context::LogFn);
            if (byteCodePS.size() == 0)
                return false;

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;

            // Vertex buffer vertex input layout
            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_SphereVB_vertexInputLayout.begin(), m_input.buffer_SphereVB_vertexInputLayout.end());

            // Make the PSO desc
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
            psoDesc.VS.pShaderBytecode = byteCodeVS.data();
            psoDesc.VS.BytecodeLength = byteCodeVS.size();
            psoDesc.PS.pShaderBytecode = byteCodePS.data();
            psoDesc.PS.BytecodeLength = byteCodePS.size();
            psoDesc.SampleDesc.Count = 1;
            psoDesc.pRootSignature = m_internal.drawCall_Draw_Sphere_rootSig;
            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            psoDesc.RasterizerState.DepthClipEnable = TRUE;
            psoDesc.RasterizerState.MultisampleEnable = FALSE;
            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            psoDesc.RasterizerState.ForcedSampleCount = 0;
            psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
            psoDesc.BlendState.IndependentBlendEnable = FALSE;

            psoDesc.BlendState.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                TRUE, FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 7
            };

            psoDesc.BlendState.RenderTarget[1] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[2] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[3] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[4] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[5] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[6] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.BlendState.RenderTarget[7] = D3D12_RENDER_TARGET_BLEND_DESC
            {
                FALSE, FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP, 15
            };

            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            psoDesc.RTVFormats[0] = m_output.texture_Final_Color_Buffer_format;
            psoDesc.NumRenderTargets = 1;

            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(m_output.texture_Final_Depth_Buffer_format);
            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))
            {
                psoDesc.DepthStencilState.StencilEnable = TRUE;
                psoDesc.DepthStencilState.StencilReadMask = 255;
                psoDesc.DepthStencilState.StencilWriteMask = 255;
                psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            }

            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_Draw_Sphere_pso));

            if (FAILED(hr))
            {
                Context::LogFn(LogLevel::Error, "Could not create PSO for Draw_Sphere");
                return false;
            }

            // name the PSO for debuggers
            if (c_debugNames)
                m_internal.drawCall_Draw_Sphere_pso->SetName(L"Draw_Sphere");
        }
        return true;
    }
};
