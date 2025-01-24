/*$(CopyrightHeader)*/#pragma once

#include <d3d12.h>
#include <vector>
#include <cmath>
#include <unordered_map>
#include "CompileShaders.h"
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

    struct SubResourceHeapAllocationInfo
    {
        ID3D12Resource* resource = nullptr;
        int arrayIndex = 0;
        int mipIndex = 0;

        static inline size_t hash_combine(size_t A, size_t B)
        {
            return A ^ (0x9e3779b9 + (A << 6) + (A >> 2));
        }

        size_t operator()(const SubResourceHeapAllocationInfo& key) const
        {
            size_t hash0 = std::hash<void*>()(key.resource);
            size_t hash1 = std::hash<int>()(key.arrayIndex);
            size_t hash2 = std::hash<int>()(key.mipIndex);

            size_t hash01 = hash_combine(hash0, hash1);
            return hash_combine(hash01, hash2);
        }

        bool operator == (const SubResourceHeapAllocationInfo& other) const
        {
            return
                resource == other.resource &&
                arrayIndex == other.arrayIndex &&
                mipIndex == other.mipIndex;
        }
    };

    bool CreateHeap(Heap& heap, ID3D12Device* device, int numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, TLogFn logFn);
    void DestroyHeap(Heap& heap);

    ID3D12Resource* CreateTexture(ID3D12Device* device, const unsigned int size[3], unsigned int numMips, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, ResourceType textureType, LPCWSTR debugName, TLogFn logFn);
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
        ID3D12Resource* m_res = nullptr;
        DXGI_FORMAT m_format = DXGI_FORMAT_FORCE_UINT;
        AccessType m_access = AccessType::SRV;
        ResourceType m_resourceType = ResourceType::Texture2D;
        bool m_raw = false;

        // used by buffers, constant buffers, texture2darrays and texture3ds
        UINT m_stride = 0;

        // used by buffers
        UINT m_count = 0;

        // Used by textures
        UINT m_UAVMipIndex = 0;
    };

    inline constexpr UINT D3D12CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize) noexcept
    {
        return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
    }

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

	// Following the documentation at https://learn.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads
	inline bool FormatSupportedForUAV(ID3D12Device* device, DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_UNKNOWN:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
				return true;
		}

		D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData;
		ZeroMemory(&FeatureData, sizeof(FeatureData));
		HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData));
		if (FAILED(hr) || !FeatureData.TypedUAVLoadAdditionalFormats)
			return false;

		switch (format)
		{
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_R8_TYPELESS:
				return true;
		}

		D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = { format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
		hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
		return (SUCCEEDED(hr) && (FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0);
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
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_UNORM, uint16_t, 4, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_SNORM, int16_t, 4, false, false, false, 0, 1);

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

		    // Block compressed formats
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC4_UNORM, uint8_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC4_SNORM, int8_t, 1, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC5_UNORM, uint8_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC5_SNORM, int8_t, 2, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC7_UNORM, uint8_t, 4, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC7_UNORM_SRGB, uint8_t, 4, true, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC6H_UF16, uint16_t, 3, false, false, false, 0, 1);
		    DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC6H_SF16, uint16_t, 3, false, false, false, 0, 1);

		    default:
		    {
                logFn(LogLevel::Error, "Unhandled DXGI_FORMAT (%i)\n", format);
			    return DXGI_FORMAT_INFO(uint8_t, 0, false);
		    }
	    }
    }

    // The caller is responsible for freeing scratch and instanceDescs when they are no longer in used by an in flight command list.
    bool CreateTLAS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* vertexBuffer, int vertexBufferCount, bool isAABBs, D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, DXGI_FORMAT vertexPositionFormat, unsigned int vertexPositionOffset, unsigned int vertexPositionStride, ID3D12Resource*& blas, unsigned int& blasSize, ID3D12Resource*& tlas, unsigned int& tlasSize, ID3D12Resource*& scratch, ID3D12Resource*& instanceDescs, TLogFn logFn);

    bool CreateRTV(ID3D12Device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, DXGI_FORMAT format, D3D12_RTV_DIMENSION dimension, int arrayIndex, int mipIndex);
    bool CreateDSV(ID3D12Device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, DXGI_FORMAT format, D3D12_DSV_DIMENSION dimension, int arrayIndex, int mipIndex);

    void MakeMip(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const DXGI_FORMAT_Info& formatInfo, D3D12_RESOURCE_DIMENSION dimension, const int srcDims[3]);

    void UploadTextureToGPUAndMakeMips(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::UploadBufferTracker& uploadBufferTracker, ID3D12Resource* destResource, const std::vector<unsigned char>& pixels, const unsigned int size[3], unsigned int numMips, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_STATES stateAfter, TLogFn logFn);

} // namespace DX12Utils
