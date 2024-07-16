#include "dxutils.h"
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

    ID3D12Resource* CreateTexture(ID3D12Device* device, const unsigned int size[3], DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, ResourceType textureType, LPCWSTR debugName, TLogFn logFn)
    {
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
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

        size_t hash12 = HashCombine(hash1, hash2);
        size_t hash34 = HashCombine(hash3, hash4);
        size_t hash56 = HashCombine(hash5, hash6);

        size_t hash1234 = HashCombine(hash12, hash34);
        size_t hash567 = HashCombine(hash56, hash7);

        return HashCombine(hash1234, hash567);
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
                srvDesc.Format = descriptor.m_format;
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
                            srvDesc.Texture2D.MipLevels = 1;
                            srvDesc.Texture2D.MostDetailedMip = 0;
                            srvDesc.Texture2D.PlaneSlice = 0;
                            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                            break;
                        }
                        case ResourceType::Texture2DArray:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                            srvDesc.Texture2DArray.MipLevels = 1;
                            srvDesc.Texture2DArray.MostDetailedMip = 0;
                            srvDesc.Texture2DArray.PlaneSlice = 0;
                            srvDesc.Texture2DArray.ResourceMinLODClamp = 0;
                            srvDesc.Texture2DArray.FirstArraySlice = 0;
                            srvDesc.Texture2DArray.ArraySize = descriptor.m_count;
                            break;
                        }
                        case ResourceType::Texture3D:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                            srvDesc.Texture3D.MipLevels = 1;
                            srvDesc.Texture3D.MostDetailedMip = 0;
                            srvDesc.Texture3D.ResourceMinLODClamp = 0;
                            break;
                        }
                        case ResourceType::TextureCube:
                        {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                            srvDesc.TextureCube.MipLevels = 1;
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
                        uavDesc.Texture2D.MipSlice = 0;
                        uavDesc.Texture2D.PlaneSlice = 0;
                        break;
                    }
                    case ResourceType::Texture2DArray:
                    case ResourceType::TextureCube:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                        uavDesc.Texture2DArray.MipSlice = 0;
                        uavDesc.Texture2DArray.PlaneSlice = 0;
                        uavDesc.Texture2DArray.FirstArraySlice = 0;
                        uavDesc.Texture2DArray.ArraySize = (descriptor.m_resourceType == ResourceType::TextureCube) ? 6 : descriptor.m_count;
                        break;
                    }
                    case ResourceType::Texture3D:
                    {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                        uavDesc.Texture3D.MipSlice = 0;
                        uavDesc.Texture3D.FirstWSlice = 0;
                        uavDesc.Texture3D.WSize = descriptor.m_count;
                        break;
                    }
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
            (unsigned int)ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, max(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes)),
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

} // namespace DX12Utils
