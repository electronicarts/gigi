#pragma once

#include <d3d12.h>
#include <vector>
#include <cmath>
#include <unordered_map>
#include "shadercompiler.h"
#include "logfn.h"
#include "SRGB.h"

#define ALIGN(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

namespace DX12Utils
{
    enum class AccessType
    {
        SRV,
        UAV,
        CBV,
    };

    enum class ResourceType
    {
        Buffer,
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCube,
        RTScene
    };

    struct Heap
    {
        ID3D12DescriptorHeap* m_heap = nullptr;
        size_t indexCount = 0;
        size_t nextIndexFree = 0;
        size_t indexSize = 0;

        std::unordered_map<size_t, size_t> descriptorTableCache;
    };

    struct UploadBufferTracker
    {
        struct Buffer
        {
            ID3D12Resource* buffer = nullptr;
            size_t size = 0;
            size_t age = 0;
        };

        void OnNewFrame(int framesInFlight)
        {
            // advance the age of each in use buffer. Put them in the free list when it's safe to do so
            inUse.erase(
                std::remove_if(inUse.begin(), inUse.end(),
                    [framesInFlight, this] (Buffer* buffer)
                    {
                        buffer->age++;
                        if(buffer->age >= framesInFlight)
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

        Buffer* GetBuffer(ID3D12Device* device, size_t size, TLogFn logFn, bool align256 = true);

        std::vector<Buffer*> inUse;
        std::vector<Buffer*> free;
    };

    bool CreateHeap(Heap& heap, ID3D12Device* device, int numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, TLogFn logFn);
    void DestroyHeap(Heap& heap);

    ID3D12Resource* CreateTexture(ID3D12Device* device, const unsigned int size[3], DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, ResourceType textureType, LPCWSTR debugName, TLogFn logFn);
    ID3D12Resource* CreateBuffer(ID3D12Device* device, unsigned int size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_HEAP_TYPE heapType, LPCWSTR debugName, TLogFn logFn);

    bool CopyConstantsCPUToGPU(UploadBufferTracker& tracker, ID3D12Device* device, ID3D12GraphicsCommandList* commandList,ID3D12Resource* resource, void* data, size_t dataSize, TLogFn logFn);

    template <typename T>
    bool CopyConstantsCPUToGPU(UploadBufferTracker& tracker, ID3D12Device* device, ID3D12GraphicsCommandList* commandList,ID3D12Resource* resource, const T&data, TLogFn logFn)
    {
        return CopyConstantsCPUToGPU(tracker, device, commandList, resource, (void*)&data, sizeof(data), logFn);
    }

    bool MakeRootSig(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_RANGE* ranges,
        int rangeCount,
        D3D12_STATIC_SAMPLER_DESC* samplers,
        int samplerCount,
        ID3D12RootSignature** rootSig,
        LPCWSTR debugName,
        TLogFn logFn);

    struct ResourceDescriptor
    {
        ID3D12Resource* m_res;
        DXGI_FORMAT m_format;
        AccessType m_access;
        ResourceType m_resourceType;
        bool m_raw;

        // used by buffers, constant buffers, texture2darrays and texture3ds
        UINT m_stride;

        // used by buffers
        UINT m_count;
    };

    D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorTable(ID3D12Device* device, Heap& srvHeap, const ResourceDescriptor* descriptors, size_t count, TLogFn logFn);

    inline bool isStencilDXGI_FORMAT(DXGI_FORMAT format)
    {
        switch (format)
        {
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            {
                return true;
            }
            default: return false;
        }
    }

    inline DXGI_FORMAT SRV_Safe_DXGI_FORMAT(DXGI_FORMAT format)
    {
	    switch (format)
	    {
		    case DXGI_FORMAT_D32_FLOAT:return DXGI_FORMAT_R32_FLOAT;
		    case DXGI_FORMAT_D16_UNORM:return DXGI_FORMAT_R16_UNORM;
		    default: return format;
	    }
    }

    inline DXGI_FORMAT DSV_Safe_DXGI_FORMAT(DXGI_FORMAT format)
    {
	    switch (format)
	    {
		    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:return DXGI_FORMAT_D24_UNORM_S8_UINT;
		    default: return format;
	    }
    }

    struct DXGI_FORMAT_Info
    {
	    enum class ChannelType
	    {
		    _uint8_t,
		    _uint16_t,
		    _uint32_t,

		    _int8_t,
		    _int16_t,
		    _int32_t,

		    _float,
	    };

	    DXGI_FORMAT_Info() {}

	    DXGI_FORMAT_Info(DXGI_FORMAT _format, const char* _name, int _bytesPerChannel, int _channelCount, ChannelType _channelType, bool _sRGB, bool _isStencil, bool _isDepth, int _planeIndex, int _planeCount)
	    {
		    format = _format;
		    name = _name;
		    bytesPerChannel = _bytesPerChannel;
		    channelCount = _channelCount;
		    bytesPerPixel = channelCount * bytesPerChannel;
		    channelType = _channelType;
		    sRGB = _sRGB;
		    isStencil = _isStencil;
		    isDepth = _isDepth;
		    planeIndex = _planeIndex;
		    planeCount = _planeCount;
	    }

	    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	    const char* name = nullptr;
	    int bytesPerChannel = 0;
	    int channelCount = 0;
	    int bytesPerPixel = 0;
	    ChannelType channelType = ChannelType::_uint8_t;
	    bool sRGB = false;
	    bool isStencil = false;
	    bool isDepth = false;
	    int planeIndex = 0;
	    int planeCount = 1;
    };

    #define DXGI_FORMAT_INFO(type, channelCount, sRGB) DXGI_FORMAT_Info(DXGI_FORMAT_UNKNOWN, "", sizeof(type), channelCount, DXGI_FORMAT_Info::ChannelType::_##type, sRGB, false, false, 0, 1)
    #define DXGI_FORMAT_INFO_CASE(name, type, channelCount, sRGB, isStencil, isDepth, planeIndex, planeCount) case name: return DXGI_FORMAT_Info(name, #name, sizeof(type), channelCount, DXGI_FORMAT_Info::ChannelType::_##type, sRGB, isStencil, isDepth, planeIndex, planeCount);

    inline DXGI_FORMAT_Info Get_DXGI_FORMAT_Info(DXGI_FORMAT format, TLogFn logFn)
    {
	    switch (format)
	    {
		    // Unsigned integers
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_UNORM, uint8_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_UNORM, uint8_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_UNORM, uint8_t, 4, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, uint8_t, 4, true, false, false, 0, 1);

            DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_UINT, uint8_t, 1, false, false, false, 0, 1);
            DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_UINT, uint8_t, 2, false, false, false, 0, 1);
            DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_UINT, uint8_t, 4, false, false, false, 0, 1);

		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_UINT, uint16_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_UNORM, uint16_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16_UINT, uint16_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_UINT, uint16_t, 4, false, false, false, 0, 1);

		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_UINT, uint32_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32_UINT, uint32_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32_UINT, uint32_t, 3, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32A32_UINT, uint32_t, 4, false, false, false, 0, 1);

		    // Signed integers
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_SNORM, int8_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_SNORM, int8_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_SNORM, int8_t, 4, false, false, false, 0, 1);

            DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_SINT, int8_t, 1, false, false, false, 0, 1);
            DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_SINT, int8_t, 2, false, false, false, 0, 1);
            DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_SINT, int8_t, 4, false, false, false, 0, 1);

		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_SINT, int16_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16_SINT, int16_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_SINT, int16_t, 4, false, false, false, 0, 1);

		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_SINT, int32_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32_SINT, int32_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32_SINT, int32_t, 3, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32A32_SINT, int32_t, 4, false, false, false, 0, 1);

		    // 16 bit float
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_FLOAT, uint16_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16_FLOAT, uint16_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_FLOAT, uint16_t, 4, false, false, false, 0, 1);

		    // 32 bit float
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_FLOAT, float, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32_FLOAT, float, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32_FLOAT, float, 3, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32A32_FLOAT, float, 4, false, false, false, 0, 1);

		    // Other
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R11G11B10_FLOAT, uint32_t, 1, false, false, false, 0, 1);  // Treat as 1 x u32

		    // Depth
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D32_FLOAT, float, 1, false, false, true, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D16_UNORM, uint16_t, 1, false, false, true, 0, 1);

		    // Depth Stencil
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, uint32_t, 2, false, true, true, 0, 2);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, float, 1, false, true, true, 0, 2);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, uint8_t, 1, false, true, true, 1, 2);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D24_UNORM_S8_UINT, uint32_t, 1, false, true, true, 0, 2);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, uint32_t, 1, false, true, true, 0, 2);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_X24_TYPELESS_G8_UINT, uint8_t, 1, false, true, true, 1, 2);

		    default:
		    {
                logFn(LogLevel::Error, "Unhandled DXGI_FORMAT (%i)\n", format);
			    return DXGI_FORMAT_INFO(uint8_t, 0, false);
		    }
	    }
    }

    // The caller is responsible for freeing scratch and instanceDescs when they are no longer in used by an in flight command list.
    bool CreateTLAS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* vertexBuffer, int vertexBufferCount, bool isAABBs, D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, DXGI_FORMAT vertexPositionFormat, unsigned int vertexPositionOffset, unsigned int vertexPositionStride, ID3D12Resource*& blas, unsigned int& blasSize, ID3D12Resource*& tlas, unsigned int& tlasSize, ID3D12Resource*& scratch, ID3D12Resource*& instanceDescs, TLogFn logFn);

} // namespace DX12Utils
