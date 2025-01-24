///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "HeapAllocationTracker.h"
#include <unordered_map>
#include <algorithm>
#include "utils.h"
#include <assert.h>

class DescriptorTableCache
{
public:
	void Init(int numFramesInFlight)
	{
		assert(numFramesInFlight >= 2
			&& numFramesInFlight <= 3
			&& "Unexpected number of frames in flight. See the comment above the declaration of m_numFramesInFlight");

		m_numFramesInFlight = numFramesInFlight;
	}

public:
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

	struct ResourceDescriptor
	{
		ID3D12Resource* m_resource = nullptr;
		DXGI_FORMAT m_format = DXGI_FORMAT_FORCE_UINT;
		AccessType m_access = AccessType::SRV;
		ResourceType m_resourceType = ResourceType::Texture2D;

		// Used by textures
		UINT m_UAVMipIndex = 0;

		// used by buffers, constant buffers, texture2darrays and texture3ds
		UINT m_stride = 0;

		// used by buffers
		UINT m_count = 0;
		bool m_raw = false;
	};

	bool GetDescriptorTable(ID3D12Device* device, HeapAllocationTracker& heapAllocationTracker, const ResourceDescriptor* descriptors, int count, D3D12_GPU_DESCRIPTOR_HANDLE& handle, const char* debugText)
	{
		// Get the hash of the descriptor table desired
		size_t hash = 0x1ee7beef;
		for (size_t i = 0; i < count; ++i)
			hash = HashCombine(hash, Hash(descriptors[i]));

		// if this descriptor table already exists, use it
		auto it = m_descriptorTableCache.find(hash);
		if (it != m_descriptorTableCache.end())
		{
			handle = heapAllocationTracker.GetGPUHandle(it->second.m_srvHeapIndex);
			it->second.framesSinceUsed = 0;
			return true;
		}

		// allocate space for this table
		int startIndex = -1;
		if (!heapAllocationTracker.Allocate(startIndex, count, debugText))
			return false;

		// make an entry in our cache for this table
		m_descriptorTableCache[hash] = { startIndex, count, 0 };

		// fill the descriptor table with views
		for (int i = 0; i < count; ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE handle = heapAllocationTracker.GetCPUHandle(startIndex + i);
			const ResourceDescriptor& descriptor = descriptors[i];
			if (descriptor.m_access == AccessType::SRV)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = SRV_Safe_DXGI_FORMAT(descriptor.m_format);

				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				if (descriptor.m_resourceType == ResourceType::RTScene)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
					srvDesc.RaytracingAccelerationStructure.Location = descriptor.m_resource->GetGPUVirtualAddress();
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

							DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(srvDesc.Format);
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

							DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(srvDesc.Format);
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

					device->CreateShaderResourceView(descriptor.m_resource, &srvDesc, handle);
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

						DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(uavDesc.Format);
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

						DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(uavDesc.Format);
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
					return false;

				device->CreateUnorderedAccessView(descriptor.m_resource, nullptr, &uavDesc, handle);
			}
			else if(descriptor.m_access == AccessType::CBV)
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = descriptor.m_stride;
				cbvDesc.BufferLocation = descriptor.m_resource->GetGPUVirtualAddress();

				device->CreateConstantBufferView(&cbvDesc, handle);
			}
			else
			{
				return false;
			}
		}

		// all is well!
		handle = heapAllocationTracker.GetGPUHandle(startIndex);
		return true;
	}

	void OnNewFrame(HeapAllocationTracker& heap)
	{
		// free entries that haven't been used in a while
		std::vector<size_t> keysToRemove;
		for (auto& it : m_descriptorTableCache)
		{
			it.second.framesSinceUsed++;
			if (it.second.framesSinceUsed >= m_numFramesInFlight)
			{
				heap.Free(it.second.m_srvHeapIndex, it.second.m_count);
				keysToRemove.push_back(it.first);
			}
		}

		for (size_t key : keysToRemove)
			m_descriptorTableCache.erase(key);
	}

	void Release(HeapAllocationTracker& heap)
	{
		// Free everything
		for (auto& it : m_descriptorTableCache)
			heap.Free(it.second.m_srvHeapIndex, it.second.m_count);

		m_descriptorTableCache.clear();
	}

	size_t getCacheSize()
	{
		return m_descriptorTableCache.size();
	}

private:
	static inline size_t HashCombine(size_t a, size_t b)
	{
		a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
		return a;
	}

	static inline size_t Hash(const ResourceDescriptor& v)
	{
		size_t hash1 = std::hash<size_t>()(static_cast<size_t>(reinterpret_cast<std::uintptr_t>(v.m_resource)));
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

	struct Entry
	{
		int m_srvHeapIndex = -1;
		int m_count = 0;
		int framesSinceUsed = 0;
	};

private:
	// After this many frames of not being used, a descriptor table will be released. Set via Init().
	// Note: As we don't currently explicitly free descriptor tables when resource(s) are released, we must use the
	// maximum number of frames in flight as our release age to keep this system in sync with everything else.
	int m_numFramesInFlight = -1;

	// Maps hash to entry
	std::unordered_map<size_t, Entry> m_descriptorTableCache;
};