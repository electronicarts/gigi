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
		Texture2DMS,
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
        UINT m_firstElement = 0;
		bool m_raw = false;

        // Number of items in a buffer, or number of indices / z slices in a texture
        UINT m_count = 0;
	};

	bool GetDescriptorTable(ID3D12Device* device, HeapAllocationTracker& heapAllocationTracker, const ResourceDescriptor* descriptors, int count, D3D12_GPU_DESCRIPTOR_HANDLE& handle, std::string& error, const char* debugText)
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
		{
			error = "Ran out of handles";
			return false;
		}

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
							bool isStructuredBuffer = (descriptor.m_stride > 0);
							bool isByteAddressBuffer = (descriptor.m_raw);

							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
							srvDesc.Buffer.FirstElement = descriptor.m_firstElement;

							// A buffer can't both be structured and raw in DX12. Raw takes precedence in Gigi.
							if (isByteAddressBuffer)
							{
								srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
								srvDesc.Buffer.StructureByteStride = 0;
								srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;

                                // We need to translate the NumElements into "how many uint32s do we have?" since we use R32_TYPELESS
                                UINT itemBytes = isStructuredBuffer
                                    ? descriptor.m_stride
                                    : (UINT)Get_DXGI_FORMAT_Info(descriptor.m_format).bytesPerPixel;
                                srvDesc.Buffer.NumElements = descriptor.m_count * itemBytes / 4;
                                srvDesc.Buffer.FirstElement = descriptor.m_firstElement * itemBytes / 4;
							}
							else
							{
								srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
								srvDesc.Buffer.StructureByteStride = descriptor.m_stride;
								srvDesc.Format = isStructuredBuffer ? DXGI_FORMAT_UNKNOWN : descriptor.m_format;
                                srvDesc.Buffer.NumElements = descriptor.m_count;
							}

                            // Verify the view is in range of the resource
                            if (descriptor.m_resource)
                            {
                                D3D12_RESOURCE_DESC resourceDesc = descriptor.m_resource->GetDesc();

                                unsigned int itemSize = 0;

                                if (isByteAddressBuffer)
                                    itemSize = 4;
                                else
                                    itemSize = (srvDesc.Buffer.StructureByteStride > 0) ? srvDesc.Buffer.StructureByteStride : Get_DXGI_FORMAT_Info(srvDesc.Format).bytesPerPixel;

                                size_t itemCount = (size_t)(resourceDesc.Width / itemSize);

                                if (srvDesc.Buffer.FirstElement >= itemCount)
                                {
                                    char buffer[2048];
                                    sprintf_s(buffer, "[entry %i] Buffer view (SRV) first element is out of range at %zu items (%zu bytes) when buffer has %zu items (%zu bytes).", i, srvDesc.Buffer.FirstElement, srvDesc.Buffer.FirstElement * itemSize, itemCount, itemCount * itemSize);
                                    error = buffer;
                                    return false;
                                }

                                if (srvDesc.Buffer.FirstElement + srvDesc.Buffer.NumElements > itemCount)
                                {
                                    char buffer[2048];
                                    sprintf_s(buffer, "[entry %i] Buffer view (SRV) firstElement+NumElements is out of range at %zu items (%zu bytes) when buffer has %zu items (%zu bytes).", i, srvDesc.Buffer.FirstElement + srvDesc.Buffer.NumElements, (srvDesc.Buffer.FirstElement + srvDesc.Buffer.NumElements) * itemSize, itemCount, itemCount * itemSize);
                                    error = buffer;
                                    return false;
                                }
                            }

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
						case ResourceType::Texture2DMS:
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
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
						bool isStructuredBuffer = (descriptor.m_stride > 0);
						bool isByteAddressBuffer = (descriptor.m_raw);

						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
						uavDesc.Buffer.FirstElement = descriptor.m_firstElement;
						uavDesc.Buffer.CounterOffsetInBytes = 0;

						// A buffer can't both be structured and raw in DX12. Raw takes precedence in Gigi.
						if (isByteAddressBuffer)
						{
							uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
							uavDesc.Buffer.StructureByteStride = 0;
							uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;

                            // We need to translate the NumElements into "how many uint32s do we have?" since we use R32_TYPELESS
                            UINT itemBytes = isStructuredBuffer
                                ? descriptor.m_stride
                                : (UINT)Get_DXGI_FORMAT_Info(descriptor.m_format).bytesPerPixel;
                            uavDesc.Buffer.NumElements = descriptor.m_count * itemBytes / 4;
                            uavDesc.Buffer.FirstElement = descriptor.m_firstElement * itemBytes / 4;
						}
						else
						{
							uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
							uavDesc.Buffer.StructureByteStride = descriptor.m_stride;
							uavDesc.Format = isStructuredBuffer ? DXGI_FORMAT_UNKNOWN : descriptor.m_format;
                            uavDesc.Buffer.NumElements = descriptor.m_count;
						}

                        // Verify the view is in range of the resource
                        if (descriptor.m_resource)
                        {
                            D3D12_RESOURCE_DESC resourceDesc = descriptor.m_resource->GetDesc();

                            unsigned int itemSize = 0;

                            if (isByteAddressBuffer)
                                itemSize = 4;
                            else
                                itemSize = (uavDesc.Buffer.StructureByteStride > 0) ? uavDesc.Buffer.StructureByteStride : Get_DXGI_FORMAT_Info(uavDesc.Format).bytesPerPixel;

                            size_t itemCount = (size_t)(resourceDesc.Width / itemSize);

                            if (uavDesc.Buffer.FirstElement >= itemCount)
                            {
                                char buffer[2048];
                                sprintf_s(buffer, "[entry %i] Buffer view (SRV) first element is out of range at %zu items (%zu bytes) when buffer has %zu items (%zu bytes).", i, uavDesc.Buffer.FirstElement, uavDesc.Buffer.FirstElement * itemSize, itemCount, itemCount * itemSize);
                                error = buffer;
                                return false;
                            }

                            if (uavDesc.Buffer.FirstElement + uavDesc.Buffer.NumElements > itemCount)
                            {
                                char buffer[2048];
                                sprintf_s(buffer, "[entry %i] Buffer view (SRV) firstElement+NumElements is out of range at %zu items (%zu bytes) when buffer has %zu items (%zu bytes).", i, uavDesc.Buffer.FirstElement + uavDesc.Buffer.NumElements, (uavDesc.Buffer.FirstElement + uavDesc.Buffer.NumElements) * itemSize, itemCount, itemCount * itemSize);
                                error = buffer;
                                return false;
                            }
                        }

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
					case ResourceType::Texture2DMS:
					{
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMS;
						break;
					}
				}

				if (!FormatSupportedForUAV(device, uavDesc.Format))
				{
					char buffer[2048];
					sprintf_s(buffer, "Format \"%s\" (%i) can't be used for UAVs", Get_DXGI_FORMAT_Info(uavDesc.Format).name, uavDesc.Format);
					error = buffer;
					return false;
				}

                if(descriptor.m_resource)
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
				char buffer[2048];
				sprintf_s(buffer, "Unknown resource access type: %i", (int)descriptor.m_access);
				error = buffer;
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
        size_t hash9 = std::hash<size_t>()(static_cast<size_t>(v.m_firstElement));

		size_t hash12 = HashCombine(hash1, hash2);
		size_t hash34 = HashCombine(hash3, hash4);
		size_t hash56 = HashCombine(hash5, hash6);
		size_t hash78 = HashCombine(hash7, hash8);

		size_t hash1234 = HashCombine(hash12, hash34);
		size_t hash5678 = HashCombine(hash56, hash78);

        return HashCombine(HashCombine(hash1234, hash5678), hash9);
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