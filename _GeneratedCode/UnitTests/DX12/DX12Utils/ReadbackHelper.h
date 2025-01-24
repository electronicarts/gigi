#pragma once

#include <d3d12.h>

#include "DelayedReleaseTracker.h"
#include <unordered_map>
#include "dxutils.h"

namespace DX12Utils
{

class ReadbackHelper
{
public:

	int RequestReadback(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES resourceState, int arrayIndex, int mipIndex, TLogFn logFn)
	{
		if (resource == nullptr)
		{
			logFn(LogLevel::Error, __FUNCTION__ "(): resource is null");
			return -1;
		}

		// make a new request
		ReadbackRequest newRequest;
		newRequest.age = 0;
		newRequest.resourceDesc = resource->GetDesc();
		newRequest.arrayIndex = arrayIndex;
		newRequest.mipIndex = mipIndex;

		// calculate size, taking into account mipIndex
		int readbackWidth = newRequest.resourceDesc.Width;
		int readbackHeight = newRequest.resourceDesc.Height;
		int readbackDepth = (newRequest.resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? newRequest.resourceDesc.DepthOrArraySize : 1;
		{
			for (int i = 0; i < mipIndex; ++i)
			{
				readbackWidth = max(readbackWidth / 2, 1);
				readbackHeight = max(readbackHeight / 2, 1);
				readbackDepth = max(readbackDepth / 2, 1);
			}
		}

		// Allocate a readback buffer
		int bytesPerPixel = 1;
		int planeCount = 1;
		int planeIndex = 0;
		if (newRequest.resourceDesc.Format != DXGI_FORMAT_UNKNOWN)
		{
			DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(newRequest.resourceDesc.Format, logFn);
			bytesPerPixel = formatInfo.bytesPerPixel;
			planeCount = formatInfo.planeCount;
			planeIndex = formatInfo.planeIndex;
		}
		int unalignedPitch = readbackWidth * bytesPerPixel;
		int alignedPitch = (newRequest.resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? unalignedPitch : ALIGN((D3D12_TEXTURE_DATA_PITCH_ALIGNMENT * planeCount), unalignedPitch);
		newRequest.readbackResourceSize = alignedPitch * readbackHeight * readbackDepth;
		newRequest.readbackResource = CreateBuffer(device, newRequest.readbackResourceSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, L"ReadbackHelper", logFn);
		newRequest.unalignedPitch = unalignedPitch;
		newRequest.alignedPitch = alignedPitch;

		// make sure the resource is in the copy source state, so we can do a copy
		if (resourceState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = resource;
			barrier.Transition.StateBefore = resourceState;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			commandList->ResourceBarrier(1, &barrier);
		}

		// Copy the resource
		switch (newRequest.resourceDesc.Dimension)
		{
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			{
				unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;

				DXGI_FORMAT_Info resourceFormatInfo = Get_DXGI_FORMAT_Info(newRequest.resourceDesc.Format, logFn);
				UINT firstSubResource = D3D12CalcSubresource(mipIndex, arrayIndex, resourceFormatInfo.planeIndex, newRequest.resourceDesc.MipLevels, newRequest.resourceDesc.DepthOrArraySize);
				device->GetCopyableFootprints(&newRequest.resourceDesc, firstSubResource, 1, 0, layout, nullptr, nullptr, nullptr);

				D3D12_TEXTURE_COPY_LOCATION src = {};
				src.pResource = resource;
				src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				src.SubresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, resourceFormatInfo.planeIndex, newRequest.resourceDesc.MipLevels, newRequest.resourceDesc.DepthOrArraySize);

				D3D12_TEXTURE_COPY_LOCATION dest = {};
				dest.pResource = newRequest.readbackResource;
				dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				dest.PlacedFootprint = *layout;

				commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

				break;
			}
			case D3D12_RESOURCE_DIMENSION_BUFFER:
			{
				commandList->CopyResource(newRequest.readbackResource, resource);
				break;
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			{
				D3D12_BOX srcBox;
				srcBox.left = 0;
				srcBox.right = readbackWidth;
				srcBox.top = 0;
				srcBox.bottom = readbackHeight;
				srcBox.front = 0;
				srcBox.back = readbackDepth;

				D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
				DXGI_FORMAT_Info resourceFormatInfo = Get_DXGI_FORMAT_Info(newRequest.resourceDesc.Format, logFn);
				UINT firstSubResource = D3D12CalcSubresource(mipIndex, 0, resourceFormatInfo.planeIndex, newRequest.resourceDesc.MipLevels, newRequest.resourceDesc.DepthOrArraySize);

				unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;
				device->GetCopyableFootprints(&resourceDesc, firstSubResource, 1, 0, layout, nullptr, nullptr, nullptr);

				D3D12_TEXTURE_COPY_LOCATION src = {};
				src.pResource = resource;
				src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				src.SubresourceIndex = firstSubResource;

				D3D12_TEXTURE_COPY_LOCATION dest = {};
				dest.pResource = newRequest.readbackResource;
				dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				dest.PlacedFootprint = *layout;

				commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, &srcBox);
				break;
			}
			default:
			{
				logFn(LogLevel::Error, __FUNCTION__ "(): Unknown resource dimension encountered");
			}
		}

		// put the resource back into the state it was
		if (resourceState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = resource;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier.Transition.StateAfter = resourceState;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			commandList->ResourceBarrier(1, &barrier);
		}

		// store the request and return the request ID
		int ret = m_nextId;
		m_nextId++;
		m_requests[ret] = newRequest;
		return ret;
	}

	void OnNewFrame(int maxFramesInFlight)
	{
		m_delayedReleaseTracker.OnNewFrame(maxFramesInFlight);

		// process all pending requests
		std::vector<int> finishedRequests;
		for (auto& it : m_requests)
		{
			it.second.age++;
			if (it.second.age > maxFramesInFlight)
			{
				// Prepare the memory to hold the read back data
				ReadbackComplete data;
				data.arrayIndex = it.second.arrayIndex;
				data.mipIndex = it.second.mipIndex;
				data.bytes.resize(it.second.readbackResourceSize);
				data.resourceDesc = it.second.resourceDesc;

				// Map the memory, copy the contents, and release the readback resource
				D3D12_RANGE writeRange;
				writeRange.Begin = 1;
				writeRange.End = 0;
				void* mappedMemory = nullptr;
				it.second.readbackResource->Map(0, nullptr, &mappedMemory);
				memcpy(data.bytes.data(), mappedMemory, it.second.readbackResourceSize);
				it.second.readbackResource->Unmap(0, &writeRange);
				m_delayedReleaseTracker.Add(it.second.readbackResource);

				// de-align the rows if we need to
				if (it.second.alignedPitch != it.second.unalignedPitch)
				{
					int numRows = data.bytes.size() / it.second.alignedPitch;
					for (int rowIndex = 0; rowIndex < numRows; ++rowIndex)
						memcpy(&data.bytes[rowIndex * it.second.unalignedPitch], &data.bytes[rowIndex * it.second.alignedPitch], it.second.unalignedPitch);
					data.bytes.resize(numRows * it.second.unalignedPitch);
				}

				// store the completed request
				m_completeRequests[it.first] = data;
				finishedRequests.push_back(it.first);
			}
		}

		// remove all requests which are complete
		for (int id : finishedRequests)
			m_requests.erase(id);
	}

	void Release()
	{
		for (auto& it : m_requests)
			m_delayedReleaseTracker.Add(it.second.readbackResource);
		m_requests.clear();
		m_delayedReleaseTracker.Release();
	}

	bool ReadbackReady(int id) const
	{
		return m_completeRequests.count(id) != 0;
	}

	std::vector<unsigned char> GetReadbackData(int id, D3D12_RESOURCE_DESC& resourceDesc, int& arrayIndex, int& mipIndex)
	{
		std::vector<unsigned char> ret;

		if (!ReadbackReady(id))
			return ret;

		resourceDesc = m_completeRequests[id].resourceDesc;
		arrayIndex = m_completeRequests[id].arrayIndex;
		mipIndex = m_completeRequests[id].mipIndex;
		ret = m_completeRequests[id].bytes;

		m_completeRequests.erase(id);

		return ret;
	}

	DelayedReleaseTracker& GetDelayedReleaseTracker()
	{
		return m_delayedReleaseTracker;
	}

private:
	struct ReadbackRequest
	{
		int age = 0;
		D3D12_RESOURCE_DESC resourceDesc;
		int mipIndex = 0;
		int arrayIndex = 0;
		ID3D12Resource* readbackResource = nullptr;
		unsigned int readbackResourceSize = 0;
		int unalignedPitch = 0;
		int alignedPitch = 0;
	};

	struct ReadbackComplete
	{
		D3D12_RESOURCE_DESC resourceDesc;
		int mipIndex = 0;
		int arrayIndex = 0;
		std::vector<unsigned char> bytes;
	};

	int m_nextId = 0;

	std::unordered_map<int, ReadbackRequest> m_requests;
	std::unordered_map<int, ReadbackComplete> m_completeRequests;

	DelayedReleaseTracker m_delayedReleaseTracker;
};

} // namespace DX12Utils
