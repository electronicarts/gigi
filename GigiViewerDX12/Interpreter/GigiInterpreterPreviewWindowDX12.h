///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include "GigiCompilerLib/gigiinterpreter.h"
#include "GigiCompilerLib/Utils.h"
#include <unordered_map>
#include <vector>
#include <d3d12.h>
#include "DX12Utils/UploadBufferTracker.h"
#include "DX12Utils/TransitionTracker.h"
#include "DX12Utils/TextureCache.h"
#include "DX12Utils/DelayedReleaseTracker.h"
#include "DX12Utils/HeapAllocationTracker.h"
#include "DX12Utils/DescriptorTableCache.h"
#include "DX12Utils/CreateResources.h"
#include "DX12Utils/Profiler.h"
#include "DX12Utils/FileCache.h"
#include "DX12Utils/FileWatcher.h"
#include "DX12Utils/ObjCache.h"
#include "DX12Utils/FBXCache.h"
#include "DX12Utils/PLYCache.h"

#include "pix3.h"

#include "RuntimeNodeTypes.h"
#include <chrono>
// clang-format on

// The DX12 interpreter
class GigiInterpreterPreviewWindowDX12 : public IGigiInterpreter<RuntimeTypes>
{
public:
	GigiInterpreterPreviewWindowDX12()
		: m_fileWatcher(1.0f)
	{
	}

	bool Init(ID3D12Device2* device, ID3D12CommandQueue* commandQueue, int maxFramesInFlight, ID3D12DescriptorHeap* ImGuiSRVHeap, int ImGuiSRVHeapDescriptorCount, int ImGuiSRVHeapDescriptorSize)
	{
		m_device = device;
		m_commandQueue = commandQueue;
		m_maxFramesInFlight = maxFramesInFlight;

		m_descriptorTableCache_imgui.Init(maxFramesInFlight);
		m_descriptorTableCache.Init(maxFramesInFlight);

		m_SRVHeapAllocationTracker_imgui.Init(m_maxFramesInFlight, ImGuiSRVHeap, ImGuiSRVHeapDescriptorCount, ImGuiSRVHeapDescriptorSize);
		m_SRVHeapAllocationTracker_imgui.MarkIndexAllocated(0, HEAP_DEBUG_TEXT()); // used by the imgui font texture

		// Create SRV descriptor heap
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = c_numSRVDescriptors;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_SRVHeap)) != S_OK)
				return false;

			m_SRVHeapAllocationTracker.Init(m_maxFramesInFlight, m_SRVHeap, c_numSRVDescriptors, (int)m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		}

		// Create RTV descriptor heap
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = c_numRTVDescriptors;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVHeap)) != S_OK)
				return false;

			m_RTVHeapAllocationTracker.Init(m_maxFramesInFlight, m_RTVHeap, c_numRTVDescriptors, (int)m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		}

		// Create DSV descriptor heap
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			desc.NumDescriptors = c_numDSVDescriptors;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DSVHeap)) != S_OK)
				return false;

			m_DSVHeapAllocationTracker.Init(m_maxFramesInFlight, m_DSVHeap, c_numDSVDescriptors, (int)m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
		}

		m_profiler.Init(device, commandQueue);

		// create indirect dispatch command
		{
			D3D12_INDIRECT_ARGUMENT_DESC dispatchArg = {};
			dispatchArg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

			D3D12_COMMAND_SIGNATURE_DESC dispatchDesc = {};
			dispatchDesc.ByteStride = sizeof(uint32_t) * 4;
			dispatchDesc.NumArgumentDescs = 1;
			dispatchDesc.pArgumentDescs = &dispatchArg;
			dispatchDesc.NodeMask = 0x0;

			device->CreateCommandSignature(
				&dispatchDesc,
				nullptr,
				IID_PPV_ARGS(&m_commandSignatureDispatch));
		}

		// DX12 capabilities
		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &m_dx12_options5, sizeof(m_dx12_options5))))
			return false;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &m_dx12_options6, sizeof(m_dx12_options6))))
			return false;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &m_dx12_options7, sizeof(m_dx12_options7))))
			return false;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS8, &m_dx12_options8, sizeof(m_dx12_options8))))
			return false;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, &m_dx12_options9, sizeof(m_dx12_options9))))
			return false;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS10, &m_dx12_options10, sizeof(m_dx12_options10))))
			return false;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS11, &m_dx12_options11, sizeof(m_dx12_options11))))
			return false;

		// Create the DXR device
		if (m_dx12_options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
			m_device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice));

		return true;
	}

	void WriteGPUResource(const char* viewableResourceName, int subresourceIndex, const char* data, size_t size)
	{
		std::vector<char> storedData(size);
		memcpy(storedData.data(), data, size);
		m_GPUResourceWrites[viewableResourceName].data = storedData;
		m_GPUResourceWrites[viewableResourceName].subresourceIndex = subresourceIndex;
	}

	void ClearCachedFiles()
	{
		m_files.ClearCache();
		m_textures.ClearCache();
		m_objs.ClearCache();
		m_fbxs.ClearCache();
	}

	// This assumes that there are no more frames in flight and that it's safe to immediately release everything
	void Release()
	{
		// Forget our device
		m_device = nullptr;

		// Clean up the upload buffer tracker
		m_uploadBufferTracker.Release();

		// Clean up the descriptor table cache
		m_descriptorTableCache.Release(m_SRVHeapAllocationTracker);
		m_descriptorTableCache_imgui.Release(m_SRVHeapAllocationTracker_imgui);

		// Call release on all runtime data
		// clang-format off
		#include "external/df_serialize/_common.h"
		#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) m_##_TYPE##_RuntimeData.ForEach([this](auto& key, auto& data) { data.Release(*this); }); \
		m_##_TYPE##_RuntimeData.Clear();
		#include "external/df_serialize/_fillunsetdefines.h"
		#include "Schemas/RenderGraphNodesVariant.h"
		// clang-format on

		// Ensure the transitions tracker is empty
		if (!m_transitions.Empty())
			m_logFn(LogLevel::Error, "The transition tracker is not empty. Resource leak?");
		m_transitions.Clear();

		// release anything remaining in the delayed release tracker
		m_delayedRelease.Release();

		// Ensure the heaps are clear
		m_SRVHeapAllocationTracker_imgui.FlushFreeLists();
		if (m_SRVHeapAllocationTracker_imgui.AllocatedCount() != 1) // one is for the imgui font texture
			m_logFn(LogLevel::Error, "The ImGui SRV heap is not empty. Resource leak?");
		m_SRVHeapAllocationTracker_imgui.Release();

		m_SRVHeapAllocationTracker.FlushFreeLists();
		if (m_SRVHeapAllocationTracker.AllocatedCount() != 0)
			m_logFn(LogLevel::Error, "The SRV heap is not empty. Resource leak?");
		m_SRVHeapAllocationTracker.Release();

		m_RTVHeapAllocationTracker.FlushFreeLists();
		if (m_RTVHeapAllocationTracker.AllocatedCount() != 0)
			m_logFn(LogLevel::Error, "The RTV heap is not empty. Resource leak?");
		m_RTVHeapAllocationTracker.Release();

		m_DSVHeapAllocationTracker.FlushFreeLists();
		if (m_DSVHeapAllocationTracker.AllocatedCount() != 0)
			m_logFn(LogLevel::Error, "The DSV heap is not empty. Resource leak?");
		m_DSVHeapAllocationTracker.Release();

		if (m_SRVHeap)
		{
			m_SRVHeap->Release();
			m_SRVHeap = nullptr;
		}

		if (m_RTVHeap)
		{
			m_RTVHeap->Release();
			m_RTVHeap = nullptr;
		}

		if (m_DSVHeap)
		{
			m_DSVHeap->Release();
			m_DSVHeap = nullptr;
		}

		m_profiler.Release();

		if (m_commandSignatureDispatch)
		{
			m_commandSignatureDispatch->Release();
			m_commandSignatureDispatch = nullptr;
		}

		if (m_dxrDevice)
		{
			m_dxrDevice->Release();
			m_dxrDevice = nullptr;
		}
	}

	void ShowUI(bool minimalUI, bool paused);

	void OnPreCompile() override final
	{
		m_GPUResourceWrites.clear();
		m_fileWatcher.Clear();
		m_sourceFilesModified = false;

		// See if DXR is supported
		// DXR is supported if m_dxrDevice is not null
		if (m_dxrDevice)
		{
			m_dxrDevice->Release();
			m_dxrDevice = nullptr;
		}
		if (m_allowRaytracing)
		{
			D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};
			if (m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData)) >= 0)
				if (featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
					m_device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice));
		}
	}

	void OnCompileOK() override final
	{
		// Mark all imported textures as stale. Any that are still wanted will be marked as not stale during NodeAction::Init
		for (auto& it : m_importedResources)
		{
			it.second.stale = true;
			it.second.state = ImportedResourceState::dirty;
		}

		// Call release on all runtime data
		// clang-format off
		#include "external/df_serialize/_common.h"
		#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) m_##_TYPE##_RuntimeData.ForEach([this](auto& key, auto& data) { data.Release(*this); data.OnCompileOK(*this); });
		#include "external/df_serialize/_fillunsetdefines.h"
		#include "Schemas/RenderGraphNodesVariant.h"
		// clang-format on
	}

	bool SourceFilesModified() const
	{
		return m_sourceFilesModified;
	}

	template <typename T, size_t N>
	bool UploadDataToBuffer(ID3D12GraphicsCommandList* commandList, const char* nodeName, T(&keyStates)[N])
	{
		bool exists = false;
		RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(nodeName, exists);
		if (!exists || !runtimeData.m_resource || runtimeData.m_structIndex != -1)
			return false;

		if (runtimeData.m_count != N)
			return false;

		DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(runtimeData.m_format);
		if (formatInfo.channelCount != 1)
			return false;

		UploadBufferTracker::Buffer* uploadBuffer = m_uploadBufferTracker.GetBuffer(m_device, runtimeData.m_size, false);

		void* bufferStart = nullptr;
		HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&bufferStart));
		if (hr)
			return false;

		auto copyWithCast = [=](auto* A, auto* B, size_t count)
		{
			for (size_t i = 0; i < count; ++i)
				A[i] = B[i];
		};

		switch (formatInfo.channelType)
		{
			case DXGI_FORMAT_Info::ChannelType::_uint8_t:copyWithCast((uint8_t*)bufferStart, keyStates, N); break;
			case DXGI_FORMAT_Info::ChannelType::_uint16_t:copyWithCast((uint16_t*)bufferStart, keyStates, N); break;
			case DXGI_FORMAT_Info::ChannelType::_uint32_t:copyWithCast((uint32_t*)bufferStart, keyStates, N); break;
			case DXGI_FORMAT_Info::ChannelType::_int8_t:copyWithCast((int8_t*)bufferStart, keyStates, N); break;
			case DXGI_FORMAT_Info::ChannelType::_int16_t:copyWithCast((int16_t*)bufferStart, keyStates, N); break;
			case DXGI_FORMAT_Info::ChannelType::_int32_t:copyWithCast((int32_t*)bufferStart, keyStates, N); break;
			case DXGI_FORMAT_Info::ChannelType::_float:copyWithCast((float*)bufferStart, keyStates, N); break;
		}

		uploadBuffer->buffer->Unmap(0, nullptr);

		m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
		m_transitions.Flush(commandList);

		commandList->CopyResource(runtimeData.m_resource, uploadBuffer->buffer);

		return true;
	}

	void WatchGGFile(const char* fileName)
	{
		m_fileWatcher.Add(fileName, FileWatchOwner::GGFile);
	}

	void Tick()
	{
		m_sourceFilesModified |= m_fileWatcher.Tick(
			[this] (const std::string& fileName, const FileWatchOwner& owner)
			{
				// Remove from higher order caches
				switch (owner)
				{
					case FileWatchOwner::Shaders: return; // Shaders don't use the file cache, but we still watch them
					case FileWatchOwner::FileCache: break; // Nothing extra for FileCache
					case FileWatchOwner::TextureCache:
					{
						if (!m_textures.Remove(fileName.c_str()))
							m_logFn(LogLevel::Error, "Tried to remove modifiled file from the texture cache, but it wasn't there! \"%s\"", fileName.c_str());
						break;
					}
					case FileWatchOwner::ObjCache:
					{
						if (!m_objs.Remove(fileName.c_str()))
							m_logFn(LogLevel::Error, "Tried to remove modifiled file from the object cache, but it wasn't there! \"%s\"", fileName.c_str());
						break;
					}
					case FileWatchOwner::FBXCache:
					{
						if (!m_fbxs.Remove(fileName.c_str()))
							m_logFn(LogLevel::Error, "Tried to remove modifiled file from the FBX cache, but it wasn't there! \"%s\"", fileName.c_str());
						break;
					}
					case FileWatchOwner::PLYCache:
					{
						if (!m_plys.Remove(fileName.c_str()))
							m_logFn(LogLevel::Error, "Tried to remove modifiled file from the PLY cache, but it wasn't there! \"%s\"", fileName.c_str());
						break;
					}
				}

				// Remove from the file cache
				if (owner != FileWatchOwner::GGFile)
				{
					if (!m_files.Remove(fileName.c_str()))
						m_logFn(LogLevel::Error, "Tried to remove modifiled file from the file cache, but it wasn't there! \"%s\"", fileName.c_str());
				}
			}
		);
	}

	bool Execute(ID3D12GraphicsCommandList* commandList)
	{
		// Set the command list
		m_commandList = commandList;

		// Get the DXR command list
		if (SupportsRaytracing())
		{
			if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList))))
			{
				m_logFn(LogLevel::Error, "Could not get DXR command list.");
				return false;
			}
		}

		// give the profiler the first OnNewFrame since we profile the scope of everything else getting their OnNewFrame
		m_profiler.OnNewFrame(m_commandList, m_enableProfiling);

		// Profile the root level of execution
		bool ret = true;
		{
			ScopeProfiler _p(m_profiler, "Total", m_renderGraph.name.c_str(), true, false);

			// Remove imported textures that are marked as stale.
			// Only happens in OnCompileOK() but no better place to do this work it at the moment.
			std::vector<std::string> keysToRemove;
			for (auto& it : m_importedResources)
			{
				if (it.second.stale)
					keysToRemove.push_back(it.first);
			}
			for (const std::string& key : keysToRemove)
				m_importedResources.erase(key);

			// Let systems know that a new frame is happening
			m_uploadBufferTracker.OnNewFrame(m_maxFramesInFlight);
			m_delayedRelease.OnNewFrame(m_maxFramesInFlight);
			m_SRVHeapAllocationTracker_imgui.OnNewFrame();
			m_SRVHeapAllocationTracker.OnNewFrame();
			m_RTVHeapAllocationTracker.OnNewFrame();
			m_DSVHeapAllocationTracker.OnNewFrame();
			m_descriptorTableCache.OnNewFrame(m_SRVHeapAllocationTracker);
			m_descriptorTableCache_imgui.OnNewFrame(m_SRVHeapAllocationTracker_imgui);

			// Call OnNewFrame on all runtime data
			// clang-format off
			#include "external/df_serialize/_common.h"
			#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
				m_##_TYPE##_RuntimeData.ForEach( \
					[](auto& key, RuntimeTypes::_TYPE& data) \
					{ \
						data.OnNewFrame(); \
					} \
				);
			#include "external/df_serialize/_fillunsetdefines.h"
			#include "Schemas/RenderGraphNodesVariant.h"
			// clang-format on

			// Set our SRV heap
			ID3D12DescriptorHeap* ppHeaps[] = { m_SRVHeap };
			m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

			// Execute
			ret = IGigiInterpreter<RuntimeTypes>::Execute();

			// Make sure all transitions are flushed. Some are important for viewing things in imgui in the viewer.
			m_transitions.Flush(m_commandList);
		}

		// Resolve profiling data
		m_profiler.Resolve();

		// release the DXR command list if we have one
		if (m_dxrCommandList)
		{
			m_dxrCommandList->Release();
			m_dxrCommandList = nullptr;
		}

		// Clear out the GPU resource writes
		m_GPUResourceWrites.clear();

		return ret;
	}

	inline DescriptorTableCache& GetDescriptorTableCache_ImGui()
	{
		return m_descriptorTableCache_imgui;
	}

	inline HeapAllocationTracker& GetSRVHeapAllocationTracker_ImGui()
	{
		return m_SRVHeapAllocationTracker_imgui;
	}

	const std::vector<Profiler::ProfilingEntry>& GetProfilingData() const
	{
		return m_profiler.GetProfilingData();
	}

	struct FiredAssertInfo
	{
		uint32_t formatStringId;
		std::string fmt;
		std::string displayName;
		std::string msg;
	};
	const std::vector<FiredAssertInfo>& getCollectedShaderAsserts() const { return collectedAsserts; }
	std::vector<RuntimeTypes::ViewableResource*> MarkShaderAssertsForReadback();
	void CollectShaderAsserts(const std::vector<RuntimeTypes::ViewableResource*>& assertsBuffers);
	void LogCollectedShaderAsserts() const;

public:
	bool m_showVariablesUI = true;
	bool m_compileShadersForDebug = false;
	bool m_enableProfiling = false;
	bool m_drawWireframe = false;
	bool m_allowRaytracing = true;

	// Imported Resources: Maps node name to an imported texture description
	enum class ImportedResourceState
	{
		dirty,
		failed,
		clean
	};

	struct ImportedTextureBinaryDesc
	{
		TextureFormat format = TextureFormat::Any;
		int size[3] = { 0, 0, 1 };
	};

	struct ImportedTextureDesc
	{
		std::string fileName;
		bool fileIsSRGB = true; // this is needed because if we want the output format to be f32, it needs to be converted from sRGB to linear (or not)
		bool makeMips = false;
		int size[3] = { 0, 0, 1 };
		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		TextureFormat format = TextureFormat::RGBA8_Unorm_sRGB;
		RuntimeTypes::ViewableResource::Type textureType = RuntimeTypes::ViewableResource::Type::Texture2D;

		// binary file loading info
		ImportedTextureBinaryDesc binaryDesc;
	};

	struct ImportedBufferDesc
	{
		std::string fileName;
		bool CSVHeaderRow = true; // If reading a CSV, and this is true, it will skip everything up to the first newline, to ignore a header row.
		int structIndex = -1;
		DataFieldType type = DataFieldType::Count;
		int count = 1;
		GGUserFile_TLASBuildFlags RT_BuildFlags = GGUserFile_TLASBuildFlags::PreferFastTrace;
		bool BLASOpaque = true;
		bool BLASNoDuplicateAnyhitInvocations = false;
		bool IsAABBs = false; // only for ray tracing AABBs which have an intersection shader
	};

	struct ImportedResourceDesc
	{
		ImportedResourceDesc()
		{
		}

		ImportedResourceDesc(bool _isATexture)
			: isATexture(_isATexture)
		{
		}

		// shared info
		int nodeIndex = -1;
		int resourceIndex = -1;
		ImportedResourceState state = ImportedResourceState::dirty;
		bool resetEveryFrame = true;
		bool stale = false;

		// specific type info
		bool isATexture = true;
		ImportedTextureDesc texture;
		ImportedBufferDesc buffer;
	};

	std::unordered_map<std::string, ImportedResourceDesc> m_importedResources;

private:
	using IGigiInterpreter<RuntimeTypes>::Execute;

private:
	friend struct RuntimeTypes;
	static const int c_numSRVDescriptors = 4096;
	static const int c_numRTVDescriptors = 128;
	static const int c_numDSVDescriptors = 128;

public:
	bool SupportsRaytracing() const
	{
		return m_dxrDevice != nullptr;
	}

	D3D12_VARIABLE_SHADING_RATE_TIER VRSSupportLevel() const
	{
		return m_dx12_options6.VariableShadingRateTier;
	}

	const D3D12_FEATURE_DATA_D3D12_OPTIONS5& GetOptions5() const
	{
		return m_dx12_options5;
	}

	const D3D12_FEATURE_DATA_D3D12_OPTIONS6& GetOptions6() const
	{
		return m_dx12_options6;
	}

	const D3D12_FEATURE_DATA_D3D12_OPTIONS7& GetOptions7() const
	{
		return m_dx12_options7;
	}

	const D3D12_FEATURE_DATA_D3D12_OPTIONS8& GetOptions8() const
	{
		return m_dx12_options8;
	}

	const D3D12_FEATURE_DATA_D3D12_OPTIONS9& GetOptions9() const
	{
		return m_dx12_options9;
	}

	const D3D12_FEATURE_DATA_D3D12_OPTIONS10& GetOptions10() const
	{
		return m_dx12_options10;
	}

	const D3D12_FEATURE_DATA_D3D12_OPTIONS11& GetOptions11() const
	{
		return m_dx12_options11;
	}

	UploadBufferTracker getUploadBufferTracker()
	{
		return m_uploadBufferTracker;
	}

	DelayedReleaseTracker getDelayedReleaseTracker()
	{
		return m_delayedRelease;
	}

	HeapAllocationTracker getSRVHeapAllocationTracker()
	{
		return m_SRVHeapAllocationTracker;
	}

	HeapAllocationTracker getRTVHeapAllocationTracker()
	{
		return m_RTVHeapAllocationTracker;
	}

	HeapAllocationTracker getDSVHeapAllocationTracker()
	{
		return m_DSVHeapAllocationTracker;
	}

	TextureCache getTextureCache() {
		return m_textures;
	}

	FileCache getFileCache() {
		return m_files;
	}

	ObjCache getObjCache() {
		return m_objs;
	}

	FBXCache getFBXCache() {
		return m_fbxs;
	}

	PLYCache getPLYCache() {
		return m_plys;
	}

	DescriptorTableCache getDescriptorTableCache() {
		return m_descriptorTableCache;
	}

	enum class FileWatchOwner
	{
		FileCache = 0,
		Shaders,
		TextureCache,
		ObjCache,
		FBXCache,
		PLYCache,
		GGFile,

		Count
	};

	FileWatcher<FileWatchOwner> getFileWatcher()
	{
		return m_fileWatcher;
	}


private:
	// there is an "OnNodeAction()" function defined for each node type, for initialization and execution.
	// clang-format off
	#include "external/df_serialize/_common.h"
	#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
		bool OnNodeAction(const _TYPE& node, RuntimeTypes::_TYPE& runtimeData, NodeAction nodeAction) final;
	#include "external/df_serialize/_fillunsetdefines.h"
	#include "Schemas/RenderGraphNodesVariant.h"
	// clang-format on

	// helpers
	bool LoadTexture(std::vector<TextureCache::Texture>& loadedTextures, const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, std::string fileName, bool fileIsSRGB, const ImportedTextureBinaryDesc& binaryDesc, DXGI_FORMAT desiredFormat);
	bool CreateAndUploadTextures(const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, std::vector<TextureCache::Texture>& loadedTextures);

	bool OnNodeActionImported(const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, NodeAction nodeAction);
	bool OnNodeActionNotImported(const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, NodeAction nodeAction);
	bool OnNodeActionImported(const RenderGraphNode_Resource_Buffer& node, RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData, NodeAction nodeAction);
	bool OnNodeActionNotImported(const RenderGraphNode_Resource_Buffer& node, RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData, NodeAction nodeAction);
	bool MakeAccelerationStructures(const RenderGraphNode_Resource_Buffer& node, const ImportedResourceDesc& resourceDesc, RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData);
	bool MakeAccelerationStructures(const RenderGraphNode_Resource_Buffer& node);
	bool DrawCall_MakeRootSignature(const RenderGraphNode_Action_DrawCall& node, RuntimeTypes::RenderGraphNode_Action_DrawCall& runtimeData);
	bool DrawCall_MakeDescriptorTableDesc(std::vector<DescriptorTableCache::ResourceDescriptor>& descs, const RenderGraphNode_Action_DrawCall& node, const Shader& shader, int pinOffset, std::vector<TransitionTracker::Item>& queuedTransitions, const std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES>& importantResourceStates);

	std::vector<FiredAssertInfo> collectedAsserts;

	ID3D12Device2* m_device = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	UploadBufferTracker m_uploadBufferTracker;
	TransitionTracker m_transitions;
	TextureCache m_textures;
	FileCache m_files;
	ObjCache m_objs;
	FBXCache m_fbxs;
	PLYCache m_plys;
	DelayedReleaseTracker m_delayedRelease;
	Profiler m_profiler;
	int m_maxFramesInFlight = 0;
	FileWatcher<FileWatchOwner> m_fileWatcher;
	bool m_sourceFilesModified = false;

	// Pending GPU resource writes. Happens during next execution and is cleared out after
	struct GPUResourceWrite
	{
		int subresourceIndex = 0;
		std::vector<char> data;
	};
	std::unordered_map<std::string, GPUResourceWrite> m_GPUResourceWrites;

	// DXR
	ID3D12Device5* m_dxrDevice = nullptr;
	ID3D12GraphicsCommandList4* m_dxrCommandList = nullptr;

	// ImGui SRV heap
	DescriptorTableCache m_descriptorTableCache_imgui;
	HeapAllocationTracker m_SRVHeapAllocationTracker_imgui;

	// SRV heap
	DescriptorTableCache m_descriptorTableCache;
	HeapAllocationTracker m_SRVHeapAllocationTracker;
	ID3D12DescriptorHeap* m_SRVHeap = nullptr;

	// RTV heap
	HeapAllocationTracker m_RTVHeapAllocationTracker;
	ID3D12DescriptorHeap* m_RTVHeap = nullptr;

	// DSV heap
	HeapAllocationTracker m_DSVHeapAllocationTracker;
	ID3D12DescriptorHeap* m_DSVHeap = nullptr;

	// Indirect dispatch
	ID3D12CommandSignature* m_commandSignatureDispatch = nullptr;

	// DX12 Capabilities
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 m_dx12_options5 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS6 m_dx12_options6 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 m_dx12_options7 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS8 m_dx12_options8 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS9 m_dx12_options9 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS10 m_dx12_options10 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS11 m_dx12_options11 = {};
};

inline const char* EnumToString(GigiInterpreterPreviewWindowDX12::FileWatchOwner e)
{
	switch (e)
	{
		case GigiInterpreterPreviewWindowDX12::FileWatchOwner::FileCache: return "FileCache";
		case GigiInterpreterPreviewWindowDX12::FileWatchOwner::Shaders: return "Shaders";
		case GigiInterpreterPreviewWindowDX12::FileWatchOwner::TextureCache: return "TextureCache";
		case GigiInterpreterPreviewWindowDX12::FileWatchOwner::ObjCache: return "ObjCache";
		case GigiInterpreterPreviewWindowDX12::FileWatchOwner::FBXCache: return "FBXCache";
		case GigiInterpreterPreviewWindowDX12::FileWatchOwner::PLYCache: return "PLYCache";
		case GigiInterpreterPreviewWindowDX12::FileWatchOwner::GGFile: return "GGFile";
		default: return "<unknown>";
	}
}
