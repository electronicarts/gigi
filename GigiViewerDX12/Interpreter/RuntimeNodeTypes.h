///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "api/include/dx12/ffx_api_dx12.hpp"
#include "upscalers/include/ffx_upscale.hpp"
#include "framegeneration/include/ffx_framegeneration.hpp"

class GigiInterpreterPreviewWindowDX12;

// Runtime storage for each render graph node type
struct RuntimeTypes
{
	struct ViewableResource
	{
		enum class Type
		{
			Texture2D,
			Texture2DArray,
			Texture3D,
			TextureCube,
			ConstantBuffer,
			Texture2DMS,
			Buffer,
		};

		ResourceType GetResourceType() const
		{
			switch (m_type)
			{
				case Type::Texture2D: return ResourceType::Texture2D;
				case Type::Texture2DArray: return ResourceType::Texture2DArray;
				case Type::Texture3D: return ResourceType::Texture3D;
				case Type::TextureCube: return ResourceType::TextureCube;
				case Type::Texture2DMS: return ResourceType::Texture2DMS;
			}
			return ResourceType::Texture2D;
		}

		Type m_type = Type::Texture2D;
		std::string m_displayName;
		ID3D12Resource* m_resource = nullptr;                  // Textures: The 2D slice of the resource being viewed (specific index or z, and specific mip). Format m_resourceFormat. Buffer: The full resource.
		DXGI_FORMAT m_format = DXGI_FORMAT_FORCE_UINT;         // The viewable format (like DXGI_FORMAT_R8_UINT instead of DXGI_FORMAT_X32_TYPELESS_G8X24_UINT). THe format of m_resource
		ID3D12Resource* m_resourceReadback = nullptr;          // The full resource, all indices and mips. Format m_formatReadback, the format of the source resource.
		D3D12_RESOURCE_DESC m_origResourceDesc;                // The desc of the resource we are reading back. Useful to decode the "untyped" readback buffer memory
		int m_formatCount = 1; // A float4x4 would need 4 float4s, so this would be 4 in that case and the format would be for float4.
		int m_size[3] = { 0, 0, 0 };
		int m_numMips = 1;
		int m_stride = 0;
		int m_count = 0;
		bool m_hideFromUI = false;
		bool m_isResultOfWrite = false;
		int m_structIndex = -1; // can be used by buffers

        // For views of buffers. In item count / indices of data type.
        unsigned int m_bufferViewBegin = 0;
        unsigned int m_bufferViewCount = 0;

		// If true, this will be updated. else, it will not.
		// The creator of this struct will set this to false every execution, and it is up to the consumer to set this to true as long as it is wanted.
		bool m_wantsToBeViewed = false;
		bool m_wantsToBeReadBack = false;
		int m_arrayIndex = 0;
		int m_mipIndex = 0;
	};

	struct RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter);
		void OnCompileOK(GigiInterpreterPreviewWindowDX12& interpreter);
		void OnNewFrame()
		{
			m_viewableResourceIndex = 0;
			m_renderGraphText = ""; // clear the text every frame
		}
		void HandleViewableResource(GigiInterpreterPreviewWindowDX12& interpreter, ViewableResource::Type type, const char* displayName, ID3D12Resource* resource, DXGI_FORMAT resourceFormat, int resourceFormatCount, int structIndex, const int resourceSize[3], int numMips, int stride, int count, bool hideFromUI, bool isResultOfWrite, unsigned int bufferViewBegin, unsigned int bufferViewCount);
		void HandleViewableTexture(GigiInterpreterPreviewWindowDX12& interpreter, ViewableResource::Type type, const char* displayName, ID3D12Resource* resource, DXGI_FORMAT resourceFormat, const int resourceSize[3], int numMips, bool hideFromUI, bool isResultOfWrite)
		{
			HandleViewableResource(interpreter, type, displayName, resource, resourceFormat, 1, -1, resourceSize, numMips, 0, 0, hideFromUI, isResultOfWrite, 0, 0);
		}
		void HandleViewableConstantBuffer(GigiInterpreterPreviewWindowDX12& interpreter, const char* displayName, ID3D12Resource* resource, int resourceSize, int structIndex, bool hideFromUI, bool isResultOfWrite)
		{
			int size[3] = { resourceSize, 1, 1 };
			HandleViewableResource(interpreter, ViewableResource::Type::ConstantBuffer, displayName, resource, DXGI_FORMAT_UNKNOWN, 1, structIndex, size, 1, 0, 0, hideFromUI, isResultOfWrite, 0, 0);
		}
        void HandleViewableBuffer(GigiInterpreterPreviewWindowDX12& interpreter, const char* displayName, ID3D12Resource* resource, DXGI_FORMAT resourceFormat, int resourceFormatCount, int structIndex, int resourceSize, int stride, int count, bool hideFromUI, bool isResultOfWrite, unsigned int bufferViewBegin, unsigned int bufferViewSize, bool bufferViewInBytes)
        {
            // Calculate the view as item count
            unsigned int bufferViewCount = 0;
            {
                unsigned int unitsDivider = 1;
                if (bufferViewInBytes)
                {
                    unitsDivider = (stride > 0)
                        ? stride
                        : Get_DXGI_FORMAT_Info(resourceFormat).bytesPerPixel;
                    unitsDivider = (unitsDivider > 0) ? unitsDivider : 1;
                }
                bufferViewBegin = bufferViewBegin / unitsDivider;
                bufferViewCount = count;

                if (bufferViewCount >= bufferViewBegin)
                    bufferViewCount -= bufferViewBegin;
                else
                    bufferViewCount = 0;

                unsigned int bufferViewNumElements = bufferViewSize / unitsDivider;
                if (bufferViewNumElements > 0 && bufferViewNumElements < bufferViewCount)
                    bufferViewCount = bufferViewNumElements;
            }

			int size[3] = { resourceSize, 1, 1 };
			HandleViewableResource(interpreter, ViewableResource::Type::Buffer, displayName, resource, resourceFormat, resourceFormatCount, structIndex, size, 1, stride, count, hideFromUI, isResultOfWrite, bufferViewBegin, bufferViewCount);
		}

		std::vector<ViewableResource> m_viewableResources;
		int m_viewableResourceIndex = 0;
		std::string m_renderGraphText;
        bool m_inErrorState = false;
        bool m_conditionIsTrue = true;
	};

	struct RenderGraphNode_Resource_Buffer : public RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter);

		ID3D12Resource* m_resourceInitialState = nullptr;
		ID3D12Resource* m_resource = nullptr;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		ID3D12Resource* m_tlas = nullptr;
		ID3D12Resource* m_blas = nullptr;
		int m_tlasSize = 0;
		int m_blasSize = 0;

		ID3D12Resource* m_instanceDescs = nullptr;

		bool m_resourceWantsReset = false;
		DXGI_FORMAT m_format = DXGI_FORMAT_FORCE_UINT;
		int m_formatCount = 1; // For buffers storing a float4x4, it would need 4 float4s, so this would be 4.
		int m_structIndex = -1;
		int m_stride = 0;
		int m_size = 0;
		int m_count = 0;

		struct MaterialInfo
		{
			MaterialInfo(std::string name_, bool used_)
				: name(name_)
				, used(used_)
			{
			}

			std::string name;
			bool used;
		};

		// From mesh data
		std::vector<std::string> shapes;
		std::vector<MaterialInfo> materials;
	};

	struct RenderGraphNode_Resource_ShaderConstants : public RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter)
		{
			RenderGraphNode_Base::Release(interpreter);
		}

		std::vector<char> m_cpuData;
		UploadBufferTracker::Buffer* m_buffer = nullptr; // owned by m_uploadBufferTracker
	};

	struct RenderGraphNode_Resource_Texture : public RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter);

		bool IsCreated() const
		{
			return m_resourceInitialState != nullptr;
		}

		ID3D12Resource* m_resourceInitialState = nullptr;
		ID3D12Resource* m_resource = nullptr;

		bool m_resourceWantsReset = false;
		DXGI_FORMAT m_format = DXGI_FORMAT_FORCE_UINT;
		int m_size[3] = {0, 0, 0};
		int m_numMips = 1;
        unsigned int sampleCount = 1;

		struct SubResourceKey
		{
			int arrayIndex = 0;
			int mipLevel = 0;

			static inline size_t hash_combine(size_t A, size_t B)
			{
				return A ^ (0x9e3779b9 + (A << 6) + (A >> 2));
			}

			size_t operator()(const SubResourceKey& key) const
			{
				return hash_combine(std::hash<int>()(key.arrayIndex), std::hash<int>()(key.mipLevel));
			}

			bool operator == (const SubResourceKey& other) const
			{
				return
					arrayIndex == other.arrayIndex &&
					mipLevel == other.mipLevel;
			}
		};

		bool GetDSV(ID3D12Device2* device, D3D12_CPU_DESCRIPTOR_HANDLE& handle, HeapAllocationTracker& DSVHeapAllocationTracker, TextureDimensionType dimension, int arrayIndex, int mipLevel, int sampleCount, const char* resourceName);
		bool GetRTV(ID3D12Device2* device, D3D12_CPU_DESCRIPTOR_HANDLE& handle, HeapAllocationTracker& RTVHeapAllocationTracker, TextureDimensionType dimension, int arrayIndex, int mipLevel, int sampleCount, const char* resourceName);

		std::unordered_map<SubResourceKey, int, SubResourceKey> m_dsvIndices;
		std::unordered_map<SubResourceKey, int, SubResourceKey> m_rtvIndices;

		bool m_failed = false;
	};

	struct RenderGraphNode_Action_ComputeShader : public RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter);

		ID3D12RootSignature* m_rootSignature = nullptr;
		ID3D12PipelineState* m_pso = nullptr;
	};

	struct RenderGraphNode_Action_RayShader : public RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter);

		ID3D12StateObject* m_stateObject = nullptr;
		ID3D12RootSignature* m_rootSignature = nullptr;

		ID3D12Resource* m_shaderTableRayGen = nullptr;
		unsigned int m_shaderTableRayGenSize = 0;

		ID3D12Resource* m_shaderTableMiss = nullptr;
		unsigned int m_shaderTableMissSize = 0;

		ID3D12Resource* m_shaderTableHitGroup = nullptr;
		unsigned int m_shaderTableHitGroupSize = 0;
	};

	struct RenderGraphNode_Action_CopyResource : public RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter)
		{
			RenderGraphNode_Base::Release(interpreter);
		}
	};

	struct RenderGraphNode_Action_DrawCall : public RenderGraphNode_Base
	{
		void Release(GigiInterpreterPreviewWindowDX12& interpreter);

		ID3D12RootSignature* m_rootSignature = nullptr;
		ID3D12PipelineState* m_pso = nullptr;

		std::vector<unsigned char> m_vertexShaderBytes;
		std::vector<unsigned char> m_pixelShaderBytes;
		std::vector<unsigned char> m_amplificationShaderBytes;
		std::vector<unsigned char> m_meshShaderBytes;

		bool m_failed = false;
	};

	struct RenderGraphNode_Action_SubGraph : public RenderGraphNode_Base
	{
	};

	struct RenderGraphNode_Action_Barrier : public RenderGraphNode_Base
	{
	};

	struct RenderGraphNode_Reroute : public RenderGraphNode_Base
	{};

    struct RenderGraphNode_Action_External : public RenderGraphNode_Base
    {
        struct AMD_FidelityFXSDK_Upscaling
        {
            bool m_initialized = false;
            ffx::CreateBackendDX12Desc m_backendDesc = {};
            ffx::CreateContextDescUpscale m_createFsrDesc = {};
            ffxQueryGetProviderVersion m_getVersion = {};

            FfxApiEffectMemoryUsage m_gpuMemoryUsageUpscaler = { 0 };
            ffxQueryDescUpscaleGetGPUMemoryUsageV2 m_upscalerGetGPUMemoryUsageV2 = {};

            ffx::Context m_UpscalingContext = nullptr;
        };
        AMD_FidelityFXSDK_Upscaling m_AMD_FidelityFXSDK_Upscaling;
    };
};

inline RuntimeTypes::ViewableResource::Type TextureDimensionTypeToViewableResourceType(TextureDimensionType dimensionType)
{
	switch (dimensionType)
	{
		case TextureDimensionType::Texture2D: return RuntimeTypes::ViewableResource::Type::Texture2D;
		case TextureDimensionType::Texture2DArray: return RuntimeTypes::ViewableResource::Type::Texture2DArray;
		case TextureDimensionType::Texture3D: return RuntimeTypes::ViewableResource::Type::Texture3D;
		case TextureDimensionType::TextureCube: return RuntimeTypes::ViewableResource::Type::TextureCube;
		case TextureDimensionType::Texture2DMS: return RuntimeTypes::ViewableResource::Type::Texture2DMS;
	}
	return RuntimeTypes::ViewableResource::Type::Texture2D;
}
