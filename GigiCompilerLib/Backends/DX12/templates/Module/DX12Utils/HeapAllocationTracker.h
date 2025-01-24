/*$(CopyrightHeader)*/#pragma once

#include <d3d12.h>
#include <vector>

#ifdef _DEBUG
#define DO_DEBUG() true
#define HEAP_DEBUG_TEXT() __FILE__ " " TOSTRING(__LINE__)
#else
#define DO_DEBUG() false
#define HEAP_DEBUG_TEXT() ""
#endif

namespace DX12Utils
{

class HeapAllocationTracker
{
public:
	void Init(ID3D12DescriptorHeap* heap, int descriptorCount, int descriptorSize)
	{
		m_delayedFreeLists.resize(1);
		m_heap = heap;
		m_allocated.clear();
		m_allocated.resize(descriptorCount, 0);
		m_descriptorSize = descriptorSize;

		#if DO_DEBUG()
			m_debugText.clear();
			m_debugText.resize(descriptorCount);
		#endif
	}

	// When something external claims specific indices, use this function.
	void MarkIndexAllocated(int index, const char* debugText)
	{
		m_allocated[index] = true;

		#if DO_DEBUG()
			m_debugText[index] = debugText;
		#endif
	}

	bool Allocate(int& allocatedIndex, const char* debugText)
	{
		for (int i = 0; i < (int)m_allocated.size(); ++i)
		{
			if (!m_allocated[i])
			{
				MarkIndexAllocated(i, debugText);
				allocatedIndex = i;
				return true;
			}
		}
		return false;
	}

	bool Allocate(int& allocatedIndex, int count, const char* debugText)
	{
		// scan for a <count> group of free indices
		for (int i = 0; i < (int)m_allocated.size(); ++i)
		{
			// If we ran out of indices, we are done
			if (i + count - 1 >= (int)m_allocated.size())
				return false;

			// See if this run of indices is free
			bool ok = true;
			for (int index = 0; index < count; ++index)
			{
				if (m_allocated[i + index])
				{
					ok = false;
					break;
				}
			}

			// if we had enough free, allocate them all and return success, along with where the block starts.
			if (ok)
			{
				allocatedIndex = i;
				for (int index = 0; index < count; ++index)
					MarkIndexAllocated(i + index, debugText);
				return true;
			}

			// otherwise, skip to the end of the "not ok" section
			i += count - 1;
		}
		return false;
	}

	void Free(int index)
	{
		m_delayedFreeLists[m_frameIndex].push_back(index);
	}

	void Free(int index, int count)
	{
		for (int i = 0; i < count; ++i)
			m_delayedFreeLists[m_frameIndex].push_back(index + i);
	}

	int AllocatedCount() const
	{
		int ret = 0;
		for (const bool& b : m_allocated)
			ret += b ? 1 : 0;
		return ret;
	}

	void FlushFreeList(int freeListIndex)
	{
		for (int index : m_delayedFreeLists[freeListIndex])
			m_allocated[index] = false;
		m_delayedFreeLists[freeListIndex].clear();
	}

	void FlushFreeLists()
	{
		for (int index = 0; index < m_delayedFreeLists.size(); ++index)
			FlushFreeList(index);
	}

	void OnNewFrame(int framesInFlight)
	{
		if (framesInFlight > m_delayedFreeLists.size())
			m_delayedFreeLists.resize(framesInFlight);

		m_frameIndex = (m_frameIndex + 1) % (int)m_delayedFreeLists.size();
		FlushFreeList(m_frameIndex);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int index) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE ret;
		ret.ptr = m_heap->GetCPUDescriptorHandleForHeapStart().ptr + index * m_descriptorSize;
		return ret;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int index) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE ret;
		ret.ptr = m_heap->GetGPUDescriptorHandleForHeapStart().ptr + index * m_descriptorSize;
		return ret;
	}

	void Release()
	{
		m_allocated.clear();

		m_heap = nullptr;
		m_descriptorSize = 0;

		m_frameIndex = 0;
		m_delayedFreeLists.clear();
	}

private:
	std::vector<bool> m_allocated;

	ID3D12DescriptorHeap* m_heap = nullptr;
	int m_descriptorSize = 0;

	int m_frameIndex = 0;
	std::vector<std::vector<int>> m_delayedFreeLists;

	#if DO_DEBUG()
		std::vector<std::string> m_debugText;
	#endif
};

} // namespace DX12Utils