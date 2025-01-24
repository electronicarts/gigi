/*$(CopyrightHeader)*/#include "dxutils.h"
#include "../public/technique.h"

#define ALIGN(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

namespace DX12Utils
{
    UploadBufferTracker::Buffer* UploadBufferTracker::GetBuffer(ID3D12Device* device, size_t size, TLogFn logFn, bool align256)
    {
        // align size to the proper size if we should
        if (align256)
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
            {
                logFn(LogLevel::Error, "Failed creating a resource to upload shader constants.");
                return nullptr;
            }
        }

        // remember that this is in use
        inUse.push_back(uploadBuffer);

        return uploadBuffer;
    }

    bool CreateHeap(Heap& heap, ID3D12Device* device, int numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, TLogFn logFn)
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = numDescriptors;
        srvHeapDesc.Type = type;
        srvHeapDesc.Flags = flags;
        if(FAILED(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&heap.m_heap))))
        {
            logFn(LogLevel::Error, "Could not create heap");
            return false;
        }

        heap.indexCount = numDescriptors;
        heap.nextIndexFree = 0;
        heap.indexSize = device->GetDescriptorHandleIncrementSize(type);
        return true;
    }

    void DestroyHeap(Heap& heap)
    {
        if(heap.m_heap)
        {
            heap.m_heap->Release();
            heap.m_heap = nullptr;
        }
    }

    ID3D12Resource* CreateTexture(ID3D12Device* device, const unsigned int size[3], unsigned int numMips, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, ResourceType textureType, LPCWSTR debugName, TLogFn logFn)
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
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;

        switch(textureType)
        {
            case ResourceType::Texture2D:
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
        {
            logFn(LogLevel::Error, "Could not create texture");
            return nullptr;
        }

        if (debugName)
            resource->SetName(debugName);

        return resource;
    }

    ID3D12Resource* CreateBuffer(ID3D12Device* device, unsigned int size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_HEAP_TYPE heapType, LPCWSTR debugName, TLogFn logFn)
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
        if(FAILED(hr))
        {
            logFn(LogLevel::Error, "Could not create buffer");
            return nullptr;
        }

        if (debugName)
            ret->SetName(debugName);

        return ret;
    }

    bool CopyConstantsCPUToGPU(UploadBufferTracker& tracker, ID3D12Device* device, ID3D12GraphicsCommandList* commandList,ID3D12Resource* resource, void* data, size_t dataSize, TLogFn logFn)
    {
        UploadBufferTracker::UploadBufferTracker::Buffer* uploadBuffer = tracker.GetBuffer(device, dataSize, logFn);

        // copy cpu data to the upload buffer
        {
            void* CBStart = nullptr;
            HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&CBStart));
            if(hr)
            {
                logFn(LogLevel::Error, "Could not map upload buffer");
                return false;
            }

            memcpy(CBStart, data, dataSize);

            uploadBuffer->buffer->Unmap(0, nullptr);
        }

        // transition the destination resource to copy dest
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = resource;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // copy the resource
        commandList->CopyResource(resource, uploadBuffer->buffer);

        // transition the destination resource to generic read
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = resource;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        return true;
    }

    bool MakeRootSig(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_RANGE* ranges,
        int rangeCount,
        D3D12_STATIC_SAMPLER_DESC* samplers,
        int samplerCount,
        ID3D12RootSignature** rootSig,
        LPCWSTR debugName,
        TLogFn logFn)
    {
        D3D12_ROOT_PARAMETER rootParams[1];

        rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParams[0].DescriptorTable.NumDescriptorRanges = rangeCount;
        rootParams[0].DescriptorTable.pDescriptorRanges = ranges;

        D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
        rootDesc.NumParameters = 1;
        rootDesc.pParameters = rootParams;
        rootDesc.NumStaticSamplers = samplerCount;
        rootDesc.pStaticSamplers = samplers;
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        ID3DBlob* sig = nullptr;
        ID3DBlob* error = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
        if (FAILED(hr))
        {
            const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
            logFn(LogLevel::Error, "Could not serialize root signature: %s", errorMsg);
            if(sig) sig->Release();
            if(error) error->Release();
            return false;
        }

        hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(rootSig));
        if (FAILED(hr))
        {
            const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
            logFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
            if(sig) sig->Release();
            if(error) error->Release();
            return false;
        }
        if(sig) sig->Release();
        if(error) error->Release();
        sig = nullptr;
        error = nullptr;

        if (debugName)
            (*rootSig)->SetName(debugName);

        return true;
    }

    static inline size_t HashCombine(size_t a, size_t b)
    {
        a ^= b + 0x9e3779b9 + (a<<6) + (a>>2);
        return a;
    }

    static inline size_t Hash(const ResourceDescriptor& v)
    {
        size_t hash1 = std::hash<size_t>()(static_cast<size_t>(reinterpret_cast<std::uintptr_t>(v.m_res)));
        size_t hash2 = std::hash<size_t>()(static_cast<size_t>(v.m_format));
        size_t hash3 = std::hash<size_t>()(static_cast<size_t>(v.m_stride));
        size_t hash4 = std::hash<size_t>()(static_cast<size_t>(v.m_count));
        size_t hash5 = std::hash<size_t>()(static_cast<size_t>(v.m_access));
        size_t hash6 = std::hash<size_t>()(static_cast<size_t>(v.m_resourceType));
        size_t hash7 = std::hash<size_t>()(static_cast<size_t>(v.m_raw));
        size_t hash8 = std::hash<size_t>()(static_cast<size_t>(v.m_UAVMipIndex));

        size_t hash12 = HashCombine(hash1, hash2);
        size_t hash34 = HashCombine(hash3, hash4);
        size_t hash56 = HashCombine(hash5, hash6);
        size_t hash78 = HashCombine(hash7, hash8);

        size_t hash1234 = HashCombine(hash12, hash34);
        size_t hash5678 = HashCombine(hash56, hash78);

        return HashCombine(hash1234, hash5678);
    }

    inline void MakeDescriptorTable(ID3D12Device* device, Heap& srvHeap, const ResourceDescriptor* descriptors, size_t count, TLogFn logFn)
    {
        size_t startIndex = srvHeap.nextIndexFree;
        srvHeap.nextIndexFree += count;
        if (srvHeap.nextIndexFree > srvHeap.indexCount)
        {
            logFn(LogLevel::Error, "Ran out of SRV descriptors, please increase c_numSRVDescriptors");
            return;
        }

        for (size_t index = 0; index < count; ++index)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = srvHeap.m_heap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += (startIndex + index) * srvHeap.indexSize;

            const ResourceDescriptor& descriptor = descriptors[index];
            if (descriptor.m_access == AccessType::SRV)
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = SRV_Safe_DXGI_FORMAT(descriptor.m_format);
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                if (descriptor.m_resourceType == ResourceType::RTScene)
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
                    srvDesc.RaytracingAccelerationStructure.Location = descriptor.m_res->GetGPUVirtualAddress();
                    device->CreateShaderResourceView(nullptr, &srvDesc, handle);
                }
                else
                {
                    switch(descriptor.m_resourceType)
                    {
                        case ResourceType::Buffer:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                            srvDesc.Buffer.FirstElement = 0;
                            srvDesc.Buffer.NumElements = descriptor.m_count;
                            srvDesc.Buffer.StructureByteStride = descriptor.m_stride;
                            srvDesc.Buffer.Flags = descriptor.m_raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
                            srvDesc.Format = descriptor.m_raw ? DXGI_FORMAT_R32_TYPELESS : descriptor.m_format;
                            break;
                        }
                        case ResourceType::Texture2D:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                            srvDesc.Texture2D.MipLevels = -1;
                            srvDesc.Texture2D.MostDetailedMip = 0;
                            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

                            DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(srvDesc.Format, logFn);
                            srvDesc.Texture2D.PlaneSlice = formatInfo.planeIndex;
                            break;
                        }
                        case ResourceType::Texture2DArray:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                            srvDesc.Texture2DArray.MipLevels = -1;
                            srvDesc.Texture2DArray.MostDetailedMip = 0;
                            srvDesc.Texture2DArray.ResourceMinLODClamp = 0;
                            srvDesc.Texture2DArray.FirstArraySlice = 0;
                            srvDesc.Texture2DArray.ArraySize = descriptor.m_count;

                            DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(srvDesc.Format, logFn);
                            srvDesc.Texture2DArray.PlaneSlice = formatInfo.planeIndex;
                            break;
                        }
                        case ResourceType::Texture3D:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                            srvDesc.Texture3D.MipLevels = -1;
                            srvDesc.Texture3D.MostDetailedMip = 0;
                            srvDesc.Texture3D.ResourceMinLODClamp = 0;
                            break;
                        }
                        case ResourceType::TextureCube:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                            srvDesc.TextureCube.MipLevels = -1;
                            srvDesc.TextureCube.MostDetailedMip = 0;
                            srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
                            break;
                        }
                    }

                    device->CreateShaderResourceView(descriptor.m_res, &srvDesc, handle);
                }
            }
            else if(descriptor.m_access == AccessType::UAV)
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = SRV_Safe_DXGI_FORMAT(descriptor.m_format);

                // Do some translation for unsupported formats
                switch(uavDesc.Format)
                {
                    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
                }

                switch(descriptor.m_resourceType)
                {
                    case ResourceType::Buffer:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                        uavDesc.Buffer.FirstElement = 0;
                        uavDesc.Buffer.NumElements = descriptor.m_count;
                        uavDesc.Buffer.StructureByteStride = descriptor.m_stride;
                        uavDesc.Buffer.CounterOffsetInBytes = 0;
                        uavDesc.Buffer.Flags = descriptor.m_raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
                        uavDesc.Format = descriptor.m_raw ? DXGI_FORMAT_R32_TYPELESS : descriptor.m_format;
                        break;
                    }
                    case ResourceType::Texture2D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                        uavDesc.Texture2D.MipSlice = descriptor.m_UAVMipIndex;

                        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(uavDesc.Format, logFn);
                        uavDesc.Texture2D.PlaneSlice = formatInfo.planeIndex;
                        break;
                    }
                    case ResourceType::Texture2DArray:
                    case ResourceType::TextureCube:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                        uavDesc.Texture2DArray.MipSlice = descriptor.m_UAVMipIndex;
                        uavDesc.Texture2DArray.FirstArraySlice = 0;
                        uavDesc.Texture2DArray.ArraySize = (descriptor.m_resourceType == ResourceType::TextureCube) ? 6 : descriptor.m_count;

                        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(uavDesc.Format, logFn);
                        uavDesc.Texture2DArray.PlaneSlice = formatInfo.planeIndex;
                        break;
                    }
                    case ResourceType::Texture3D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                        uavDesc.Texture3D.MipSlice = descriptor.m_UAVMipIndex;
                        uavDesc.Texture3D.FirstWSlice = 0;

                        int wsize = descriptor.m_count;
                        for (UINT mipIndex = 0; mipIndex < descriptor.m_UAVMipIndex; ++mipIndex)
                            wsize /= 2;
                        if (wsize < 1)
                            wsize = 1;

                        uavDesc.Texture3D.WSize = wsize;
                        break;
                    }
                }

                if (!FormatSupportedForUAV(device, uavDesc.Format))
                {
                    logFn(LogLevel::Error, "invalid format for UAV: %s", Get_DXGI_FORMAT_Info(uavDesc.Format, logFn).name);
                }

                device->CreateUnorderedAccessView(descriptor.m_res, nullptr, &uavDesc, handle);
            }
            else if(descriptor.m_access == AccessType::CBV)
            {
                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.SizeInBytes = descriptor.m_stride;
                cbvDesc.BufferLocation = descriptor.m_res->GetGPUVirtualAddress();

                device->CreateConstantBufferView(&cbvDesc, handle);
            }
            else
            {
                logFn(LogLevel::Error, "Unhandled type encountered");
            }
        }
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorTable(ID3D12Device* device, Heap& srvHeap, const ResourceDescriptor* descriptors, size_t count, TLogFn logFn)
    {
        // Get the hash of the descriptor table desired
        size_t hash = 0x1ee7beef;
        for (size_t i = 0; i < count; ++i)
            hash = HashCombine(hash, Hash(descriptors[i]));

        // if this descriptor table already exists, use it
        D3D12_GPU_DESCRIPTOR_HANDLE ret = srvHeap.m_heap->GetGPUDescriptorHandleForHeapStart();
        auto it = srvHeap.descriptorTableCache.find(hash);
        if (it != srvHeap.descriptorTableCache.end())
        {
            ret.ptr += it->second * srvHeap.indexSize;
            return ret;
        }

        // insert the descriptor table we are about to make into the cache
        srvHeap.descriptorTableCache[hash] = srvHeap.nextIndexFree;

        // make the descriptor table
        MakeDescriptorTable(device, srvHeap, descriptors, count, logFn);

        // return the newly made descriptor table
        it = srvHeap.descriptorTableCache.find(hash);
        ret.ptr += it->second * srvHeap.indexSize;
        return ret;
    }

    bool CreateTLAS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* vertexBuffer, int vertexBufferCount, bool isAABBs, D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, DXGI_FORMAT vertexPositionFormat, unsigned int vertexPositionOffset, unsigned int vertexPositionStride, ID3D12Resource*& blas, unsigned int& blasSize, ID3D12Resource*& tlas, unsigned int& tlasSize, ID3D12Resource*& scratch, ID3D12Resource*& instanceDescs, TLogFn logFn)
    {
        ID3D12Device5* dxrDevice = nullptr;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxrDevice))))
        {
            logFn(LogLevel::Error, "CreateTLAS could not get ID3D12Device5");
            return false;
        }

        ID3D12GraphicsCommandList4* dxrCommandList = nullptr;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&dxrCommandList))))
        {
            logFn(LogLevel::Error, "CreateTLAS could not get ID3D12GraphicsCommandList4");
            return false;
        }

        // Make BLAS
        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};

        if (isAABBs)
        {
            switch (vertexPositionFormat)
            {
                case DXGI_FORMAT_R32_FLOAT:
                case DXGI_FORMAT_R32G32B32_FLOAT:
                    break;
                default:
                {
                    logFn(LogLevel::Error, "Invalid format for AABBs BLAS (%s)\n", Get_DXGI_FORMAT_Info(vertexPositionFormat, logFn).name);
                    return false;
                }
            }
            DXGI_FORMAT_Info vertexFormatInfo = Get_DXGI_FORMAT_Info(vertexPositionFormat, logFn);

            // Calculate how many AABBs there are
            UINT AABBCount = (vertexBufferCount * vertexFormatInfo.channelCount) / 6;
            if (AABBCount == 0)
                return false;

            geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
            geometryDesc.AABBs.AABBCount = AABBCount;
            geometryDesc.AABBs.AABBs.StartAddress = vertexBuffer->GetGPUVirtualAddress();
            geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(float) * 6;
        }
        else
        {
            // We need 3 vertices for every triangle.
            // Round down, and fail if we don't have a single triangle.
            UINT vertexCount = 3 * (vertexBufferCount / 3);
            if (vertexCount == 0)
                return false;

            geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geometryDesc.Triangles.IndexBuffer = 0;
            geometryDesc.Triangles.IndexCount = 0;
            geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
            geometryDesc.Triangles.Transform3x4 = 0;
            geometryDesc.Triangles.VertexFormat = vertexPositionFormat;
            geometryDesc.Triangles.VertexCount = vertexCount;
            geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress() + vertexPositionOffset;
            geometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexPositionStride;
        }

        // flags
        geometryDesc.Flags = geometryFlags;

        // Get required sizes for an acceleration structure.
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
        topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelInputs.Flags = buildFlags;
        topLevelInputs.NumDescs = 1;
        topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
        dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
        if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes == 0)
            return false;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
        bottomLevelInputs.NumDescs = 1;
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelInputs.pGeometryDescs = &geometryDesc;
        dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
        if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes == 0)
            return false;

        // allocate a scratch buffer for creating our BLAS and TLAS
        scratch = CreateBuffer(
            device,
            (unsigned int)ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes)),
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_HEAP_TYPE_DEFAULT,
            L"BLAS TLAS Scratch",
            logFn
        );

        // Create buffers for TLAS and BLAS, and track them in the resource tracker
        tlasSize = (int)topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
        tlas = CreateBuffer(
            device,
            (unsigned int)ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, topLevelPrebuildInfo.ResultDataMaxSizeInBytes),
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
            D3D12_HEAP_TYPE_DEFAULT,
            L"TLAS",
            logFn
        );

        blasSize = (int)bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
        blas = CreateBuffer(
            device,
            (unsigned int)ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes),
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
            D3D12_HEAP_TYPE_DEFAULT,
            L"BLAS",
            logFn
        );

        // Create an upload buffer for the instance descs and put the data in there	
        {
            D3D12_RAYTRACING_INSTANCE_DESC instanceDesc;
            memset(&instanceDesc, 0, sizeof(instanceDesc));
            instanceDesc.Transform[0][0] = 1.0f;
            instanceDesc.Transform[1][1] = 1.0f;
            instanceDesc.Transform[2][2] = 1.0f;
            instanceDesc.InstanceID = 0;
            instanceDesc.InstanceMask = 1;
            instanceDesc.AccelerationStructure = blas->GetGPUVirtualAddress();

            instanceDescs = CreateBuffer(
                device,
                sizeof(instanceDesc),
                D3D12_RESOURCE_FLAG_NONE,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                D3D12_HEAP_TYPE_UPLOAD,
                L"TLAS Instance Descs",
                logFn
            );

            D3D12_RAYTRACING_INSTANCE_DESC* data = nullptr;
            D3D12_RANGE  readRange = { 0, 0 };
            instanceDescs->Map(0, &readRange, reinterpret_cast<void**>(&data));
            memcpy(data, &instanceDesc, sizeof(instanceDesc));
            instanceDescs->Unmap(0, nullptr);
        }

        // BLAS Desc
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
        {
            bottomLevelBuildDesc.Inputs = bottomLevelInputs;
            bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
            bottomLevelBuildDesc.DestAccelerationStructureData = blas->GetGPUVirtualAddress();
        }

        // TLAS Desc
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
        {
            topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
            topLevelBuildDesc.Inputs = topLevelInputs;
            topLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
            topLevelBuildDesc.DestAccelerationStructureData = tlas->GetGPUVirtualAddress();
        }

        // Build BLAS
        dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

        // Wait for it to be done with a UAV barrier
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.UAV.pResource = blas;
            commandList->ResourceBarrier(1, &barrier);
        }

        // Build TLAS
        dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

        // Wait for it to be done with a UAV barrier
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.UAV.pResource = tlas;
            commandList->ResourceBarrier(1, &barrier);
        }

        // Free the interfaces
        dxrCommandList->Release();
        dxrDevice->Release();

        return true;
    }

    bool CreateRTV(ID3D12Device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, DXGI_FORMAT format, D3D12_RTV_DIMENSION dimension, int arrayIndex, int mipIndex)
    {
	    // Create the RTV
	    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	    rtvDesc.Format = format;
	    switch (dimension)
	    {
		    case D3D12_RTV_DIMENSION_TEXTURE2D:
		    {
			    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			    rtvDesc.Texture2D.MipSlice = mipIndex;
			    rtvDesc.Texture2D.PlaneSlice = 0;
			    break;
		    }
		    case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
		    {
			    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			    rtvDesc.Texture2DArray.MipSlice = mipIndex;
			    rtvDesc.Texture2DArray.PlaneSlice = 0;
			    rtvDesc.Texture2DArray.ArraySize = 1;
			    rtvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
			    break;
		    }
		    case D3D12_RTV_DIMENSION_TEXTURE3D:
		    {
			    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			    rtvDesc.Texture3D.MipSlice = mipIndex;
			    rtvDesc.Texture3D.WSize = 1;
			    rtvDesc.Texture3D.FirstWSlice = arrayIndex;
                break;
		    }
		    default:
		    {
			    return false;
		    }
	    }

	    device->CreateRenderTargetView(resource, &rtvDesc, rtvHandle);
        return true;
    }

    bool CreateDSV(ID3D12Device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, DXGI_FORMAT format, D3D12_DSV_DIMENSION dimension, int arrayIndex, int mipIndex)
    {
	    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	    dsvDesc.Format = DX12Utils::DSV_Safe_DXGI_FORMAT(format);
	    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	    switch (dimension)
	    {
		    case D3D12_DSV_DIMENSION_TEXTURE2D:
		    {
			    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			    dsvDesc.Texture2D.MipSlice = mipIndex;
			    break;
		    }
		    case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
		    {
			    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			    dsvDesc.Texture2DArray.MipSlice = mipIndex;
			    dsvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
			    dsvDesc.Texture2DArray.ArraySize = 1;
			    break;
		    }
		    default:
		    {
			    return false;
		    }
	    }

	    device->CreateDepthStencilView(resource, &dsvDesc, dsvHandle);
        return true;
    }

    static inline double Lerp(double A, double B, double t)
    {
        return A * (1.0 - t) + B * t;
    }

    template <typename T>
    std::vector<double> ConvertToDoubles(const std::vector<unsigned char>& src, double multiplier)
    {
        const size_t valueCount = src.size() / sizeof(T);

        std::vector<double> ret(valueCount, 0);

        const T* srcValues = (const T*)src.data();
        for (size_t index = 0; index < valueCount; ++index)
            ret[index] = multiplier * (double)srcValues[index];

        return ret;
    }

    template <typename T>
    std::vector<double> ConvertToDoubles(const unsigned char* src, size_t valueCount, double multiplier)
    {
        std::vector<double> ret(valueCount, 0);

        const T* srcValues = (const T*)src;
        for (size_t index = 0; index < valueCount; ++index)
            ret[index] = multiplier * (double)srcValues[index];

        return ret;
    }

    static bool ConvertToDoubles(const std::vector<unsigned char>& src, DXGI_FORMAT_Info::ChannelType type, std::vector<double>& doubles)
    {
        switch (type)
        {
            case DXGI_FORMAT_Info::ChannelType::_uint8_t: doubles = ConvertToDoubles<uint8_t>(src, 1.0 / 255.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint16_t: doubles = ConvertToDoubles<uint16_t>(src, 1.0 / 65535.0); break;
            case DXGI_FORMAT_Info::ChannelType::_int16_t: doubles = ConvertToDoubles<int16_t>(src, 1.0 / 32767.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint32_t: doubles = ConvertToDoubles<uint32_t>(src, 1.0 / 4294967296.0); break;
            //case DXGI_FORMAT_Info::ChannelType::_half: doubles = ConvertToDoubles<half>(src, 1.0); break;
            case DXGI_FORMAT_Info::ChannelType::_float: doubles = ConvertToDoubles<float>(src, 1.0); break;
            default: return false;
        }
        return true;
    }

    static bool ConvertToDoubles(const unsigned char* src, size_t valueCount, DXGI_FORMAT_Info::ChannelType type, std::vector<double>& doubles)
    {
        switch (type)
        {
            case DXGI_FORMAT_Info::ChannelType::_uint8_t: doubles = ConvertToDoubles<uint8_t>(src, valueCount, 1.0 / 255.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint16_t: doubles = ConvertToDoubles<uint16_t>(src, valueCount, 1.0 / 65535.0); break;
            case DXGI_FORMAT_Info::ChannelType::_int16_t: doubles = ConvertToDoubles<int16_t>(src, valueCount, 1.0 / 32767.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint32_t: doubles = ConvertToDoubles<uint32_t>(src, valueCount, 1.0 / 4294967296.0); break;
            //case DXGI_FORMAT_Info::ChannelType::_half: doubles = ConvertToDoubles<half>(src, valueCount, 1.0); break;
            case DXGI_FORMAT_Info::ChannelType::_float: doubles = ConvertToDoubles<float>(src, valueCount, 1.0); break;
            default: return false;
        }
        return true;
    }

    template <typename T>
    std::vector<unsigned char> ConvertFromDoubles(const std::vector<double>& src, double multiplier, double theMin, double theMax)
    {
        const size_t valueCount = src.size();

        std::vector<unsigned char> ret(valueCount * sizeof(T), 0);

        T* destValues = (T*)ret.data();
        for (size_t index = 0; index < valueCount; ++index)
            destValues[index] = (T)(max(min(src[index] * multiplier, theMax), theMin));

        return ret;
    }

    static bool ConvertFromDoubles(const std::vector<double>& doubles, DXGI_FORMAT_Info::ChannelType type, std::vector<unsigned char>& dest)
    {
        switch (type)
        {
            case DXGI_FORMAT_Info::ChannelType::_uint8_t: dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, 0.0, 255.0); break;
            case DXGI_FORMAT_Info::ChannelType::_int8_t: dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, -128.0, 127.0); break;
            case DXGI_FORMAT_Info::ChannelType::_int16_t: dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, -32768.0, 32767.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint16_t: dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, 0.0, 65535.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint32_t: dest = ConvertFromDoubles<uint32_t>(doubles, 4294967296.0, 0.0, 4294967295.0); break;
            //case DXGI_FORMAT_Info::ChannelType::_half: dest = ConvertFromDoubles<half>(doubles, 1.0, -65504.0, 65504.0); break;
            case DXGI_FORMAT_Info::ChannelType::_float: dest = ConvertFromDoubles<float>(doubles, 1.0, -FLT_MAX, FLT_MAX); break;
            default: return false;
        }
        return true;
    }

    static bool ConvertFromDoubles(const std::vector<double>& doubles, DXGI_FORMAT_Info::ChannelType type, unsigned char* dest)
    {
        std::vector<unsigned char> _dest;
        switch (type)
        {
            case DXGI_FORMAT_Info::ChannelType::_uint8_t: _dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, 0.0, 255.0); break;
            case DXGI_FORMAT_Info::ChannelType::_int8_t: _dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, -128.0, 127.0); break;
            case DXGI_FORMAT_Info::ChannelType::_int16_t: _dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, -32768.0, 32767.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint16_t: _dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, 0.0, 65535.0); break;
            case DXGI_FORMAT_Info::ChannelType::_uint32_t: _dest = ConvertFromDoubles<uint32_t>(doubles, 4294967296.0, 0.0, 4294967295.0); break;
            //case DXGI_FORMAT_Info::ChannelType::_half: _dest = ConvertFromDoubles<half>(doubles, 1.0, -65504.0, 65504.0); break;
            case DXGI_FORMAT_Info::ChannelType::_float: _dest = ConvertFromDoubles<float>(doubles, 1.0, -FLT_MAX, FLT_MAX); break;
            default: return false;
        }

        memcpy(dest, _dest.data(), _dest.size());

        return true;
    }

    static void BilinearSample(const unsigned char* src, const DXGI_FORMAT_Info& formatInfo, const int dims[3], float U, float V, unsigned char* dest)
    {
        float srcX = U * float(dims[0]) - 0.5f;
        float srcY = V * float(dims[1]) - 0.5f;

        int x1 = int(srcX);
        int y1 = int(srcY);
        int x2 = (x1 + 1) % dims[0];
        int y2 = (y1 + 1) % dims[1];

        float fracX = std::fmod(srcX, 1.0f);
        float fracY = std::fmod(srcY, 1.0f);

        // read the source pixels
        std::vector<double> P00Values, P01Values, P10Values, P11Values;
        ConvertToDoubles(&src[(y1 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P00Values);
        ConvertToDoubles(&src[(y1 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P10Values);
        ConvertToDoubles(&src[(y2 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P01Values);
        ConvertToDoubles(&src[(y2 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P11Values);

        // Calculate the result
        std::vector<double> result(formatInfo.channelCount);
        for (size_t channelIndex = 0; channelIndex < formatInfo.channelCount; ++channelIndex)
        {
            double resultx0 = Lerp(P00Values[channelIndex], P10Values[channelIndex], fracX);
            double resultx1 = Lerp(P01Values[channelIndex], P11Values[channelIndex], fracX);
            result[channelIndex] = Lerp(resultx0, resultx1, fracY);
        }

        // Write the dest pixel
        ConvertFromDoubles(result, formatInfo.channelType, dest);
    }

    static void TrilinearSample(const unsigned char* src, const DXGI_FORMAT_Info& formatInfo, const int dims[3], float U, float V, float W, unsigned char* dest)
    {
        float srcX = U * float(dims[0]) - 0.5f;
        float srcY = V * float(dims[1]) - 0.5f;
        float srcZ = W * float(dims[2]) - 0.5f;

        int x1 = int(srcX);
        int y1 = int(srcY);
        int z1 = int(srcZ);
        int x2 = (x1 + 1) % dims[0];
        int y2 = (y1 + 1) % dims[1];
        int z2 = (z1 + 1) % dims[2];

        float fracX = std::fmod(srcX, 1.0f);
        float fracY = std::fmod(srcY, 1.0f);
        float fracZ = std::fmod(srcZ, 1.0f);

        // read the source pixels
        std::vector<double> P000Values, P010Values, P100Values, P110Values;
        std::vector<double> P001Values, P011Values, P101Values, P111Values;
        ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y1 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P000Values);
        ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y1 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P100Values);
        ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y2 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P010Values);
        ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y2 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P110Values);
        ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y1 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P001Values);
        ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y1 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P101Values);
        ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y2 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P011Values);
        ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y2 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P111Values);

        // Calculate the result
        std::vector<double> result(formatInfo.channelCount);
        for (size_t channelIndex = 0; channelIndex < formatInfo.channelCount; ++channelIndex)
        {
            double resultx00 = Lerp(P000Values[channelIndex], P100Values[channelIndex], fracX);
            double resultx10 = Lerp(P010Values[channelIndex], P110Values[channelIndex], fracX);

            double resultx01 = Lerp(P001Values[channelIndex], P101Values[channelIndex], fracX);
            double resultx11 = Lerp(P011Values[channelIndex], P111Values[channelIndex], fracX);

            double resultxy0 = Lerp(resultx00, resultx10, fracY);
            double resultxy1 = Lerp(resultx01, resultx11, fracY);

            result[channelIndex] = Lerp(resultxy0, resultxy1, fracZ);
        }

        // Write the dest pixel
        ConvertFromDoubles(result, formatInfo.channelType, dest);
    }

    void MakeMip(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const DXGI_FORMAT_Info& formatInfo, D3D12_RESOURCE_DIMENSION dimension, const int srcDims[3])
    {
        // calculate the size of the destination dims
        bool is3D = (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D);
        int destDims[3] = { max(srcDims[0] / 2, 1), max(srcDims[1] / 2, 1), is3D ? max(srcDims[2] / 2, 1) : srcDims[2] };

        // Allocate space for the mip
        dest.resize(destDims[0] * destDims[1] * destDims[2] * formatInfo.bytesPerPixel, 0);

        int srcUnalignedPitch = srcDims[0] * formatInfo.bytesPerPixel;
        int destUnalignedPitch = destDims[0] * formatInfo.bytesPerPixel;

        if (is3D)
        {
            // For each z slize
            for (size_t destZ = 0; destZ < destDims[2]; ++destZ)
            {
                float W = (float(destZ) + 0.5f) / float(destDims[2]);

                size_t destArrayOffset = destZ * destDims[1] * destUnalignedPitch;

                // for each pixel
                for (size_t destY = 0; destY < destDims[1]; ++destY)
                {
                    float V = (float(destY) + 0.5f) / float(destDims[1]);

                    size_t destOffset = destArrayOffset + destY * destUnalignedPitch;

                    for (size_t destX = 0; destX < destDims[0]; ++destX)
                    {
                        float U = (float(destX) + 0.5f) / float(destDims[0]);

                        unsigned char* destPx = &dest[destOffset + destX * formatInfo.bytesPerPixel];

                        // trilinear sample src[U, V, W] and write to dest
                        TrilinearSample(src.data(), formatInfo, srcDims, U, V, W, destPx);
                    }
                }
            }
        }
        else
        {
            // For each array slize
            for (size_t iZ = 0; iZ < srcDims[2]; ++iZ)
            {
                size_t srcArrayOffset = iZ * srcDims[1] * srcUnalignedPitch;
                size_t destArrayOffset = iZ * destDims[1] * destUnalignedPitch;

                // for each pixel
                for (size_t destY = 0; destY < destDims[1]; ++destY)
                {
                    float V = (float(destY) + 0.5f) / float(destDims[1]);

                    size_t destOffset = destArrayOffset + destY * destUnalignedPitch;

                    for (size_t destX = 0; destX < destDims[0]; ++destX)
                    {
                        float U = (float(destX) + 0.5f) / float(destDims[0]);

                        unsigned char* destPx = &dest[destOffset + destX * formatInfo.bytesPerPixel];

                        // bilinear sample src[U, V] and write to dest
                        BilinearSample(&src[srcArrayOffset], formatInfo, srcDims, U, V, destPx);
                    }
                }
            }
        }
    }

    void UploadTextureToGPUAndMakeMips(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::UploadBufferTracker& uploadBufferTracker, ID3D12Resource* destResource, const std::vector<unsigned char>& pixels, const unsigned int size[3], unsigned int numMips, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_STATES stateAfter, TLogFn logFn)
    {
        // Get information about destination resource
        D3D12_RESOURCE_DESC destResourceDesc = destResource->GetDesc();
        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(destResourceDesc.Format, logFn);
        int mipSize[3] = { (int)size[0], (int)size[1], (int)size[2] };

        // transition to copy dest if needed
        if (state != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = destResource;
            barrier.Transition.StateBefore = state;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            commandList->ResourceBarrier(1, &barrier);
        }

        // Upload each mip
        std::vector<unsigned char> mipPixels[2];
        for (unsigned int mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            // Make the mip if we should
            if (mipIndex > 0)
            {
                const std::vector<unsigned char>& src = (mipIndex > 1)
                    ? mipPixels[mipIndex % 2]
                    : pixels;

                std::vector<unsigned char>& dest = mipPixels[(mipIndex - 1) % 2];

                MakeMip(src, dest, formatInfo, destResourceDesc.Dimension, mipSize);

                // Reduce mip size
                mipSize[0] = max(mipSize[0] / 2, 1);
                mipSize[1] = max(mipSize[1] / 2, 1);
                if (destResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
                    mipSize[2] = max(mipSize[2] / 2, 1);
            }

            // Calculate dimension
            // 3D textures have one subresource for all Z indices.
            // Other textures have one subresource per each Z index;
            int arrayCount = 0;
            int iyCount = mipSize[1];
            int izCount = 0;
            if (destResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
            {
                arrayCount = 1;
                izCount = mipSize[2];
            }
            else
            {
                arrayCount = mipSize[2];
                izCount = 1;
            }

            // Get our source pixels
            const std::vector<unsigned char>& srcPixels = (mipIndex > 0)
                ? mipPixels[(mipIndex - 1) % 2]
                : pixels;

            int srcPixelsStride = (int)srcPixels.size() / arrayCount;

            for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
            {
                unsigned int arrayIndexStart = srcPixelsStride * arrayIndex;

                // Make an upload buffer
                int unalignedPitch = mipSize[0] * formatInfo.bytesPerPixel;
                int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);
                UploadBufferTracker::Buffer* uploadBuffer = uploadBufferTracker.GetBuffer(device, alignedPitch * iyCount * izCount, logFn, false);

                // write the pixels into the upload buffer
                {
                    unsigned char* destPixels = nullptr;
                    HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&destPixels));
                    if (hr)
                    {
                        logFn(LogLevel::Error, __FUNCTION__ "(): Failed to map buffer to upload texture.");
                        return;
                    }

                    for (int iz = 0; iz < izCount; ++iz)
                    {
                        for (int iy = 0; iy < iyCount; ++iy)
                        {
                            const unsigned char* src = &srcPixels[arrayIndexStart + (iz * iyCount + iy) * unalignedPitch];
                            unsigned char* dest = &destPixels[(iz * iyCount + iy) * alignedPitch];
                            memcpy(dest, src, unalignedPitch);
                        }
                    }

                    uploadBuffer->buffer->Unmap(0, nullptr);
                }

                // Copy the upload buffer into m_resourceInitialState
                {
                    UINT subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, numMips, arrayCount);

                    std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
                    device->GetCopyableFootprints(&destResourceDesc, subresourceIndex, 1, 0, layout, nullptr, nullptr, nullptr);

                    D3D12_TEXTURE_COPY_LOCATION src = {};
                    src.pResource = uploadBuffer->buffer;
                    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                    src.PlacedFootprint = *layout;

                    D3D12_TEXTURE_COPY_LOCATION dest = {};
                    dest.pResource = destResource;
                    dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    dest.SubresourceIndex = subresourceIndex;

                    commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
                }
            }
        }

        // transition to final state if needed
        if (stateAfter != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = destResource;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = stateAfter;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            commandList->ResourceBarrier(1, &barrier);
        }
    }

} // namespace DX12Utils
