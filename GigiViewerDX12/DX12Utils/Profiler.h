///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "pix3.h"

class Profiler
{
public:
	void Init(ID3D12Device* device, ID3D12CommandQueue* commandQueue)
	{
		m_device = device;
		m_commandQueue = commandQueue;
	}

	void OnNewFrame(ID3D12GraphicsCommandList* commandList, bool profile)
	{
		m_commandList = commandList;
		m_profile = profile;

		m_timeStampIndex = 0;

		// if profiling is on, but we don't have the means to profile, create them
		if (m_profile && !m_TimestampQueryHeap)
		{
			D3D12_QUERY_HEAP_DESC QueryHeapDesc;
			QueryHeapDesc.Count = c_numTimeStampsProfiler;
			QueryHeapDesc.NodeMask = 1;
			QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
			m_device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&m_TimestampQueryHeap));
			m_TimestampQueryHeap->SetName(L"Profiler Time Stamp Query Heap");
			m_TimestampReadbackBuffer = CreateBuffer(m_device, sizeof(uint64_t) * QueryHeapDesc.Count, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, "Profiler Time Stamp Query Heap");
			m_commandList->EndQuery(m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, m_timeStampIndex++);
		}

		// clear any existing profiling data
		m_profilingData.clear();
	}

	void Release()
	{
		if (m_TimestampQueryHeap)
		{
			m_TimestampQueryHeap->Release();
			m_TimestampQueryHeap = nullptr;
		}

		if (m_TimestampReadbackBuffer)
		{
			m_TimestampReadbackBuffer->Release();
			m_TimestampReadbackBuffer = nullptr;
		}
	}

	void Resolve()
	{
		if (!m_profile)
			return;

		// Resolve the profiling data
		m_commandList->EndQuery(m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, m_timeStampIndex++);
		m_commandList->ResolveQueryData(m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, m_timeStampIndex, m_TimestampReadbackBuffer, 0);

		uint64_t GPUFrequency;
		m_commandQueue->GetTimestampFrequency(&GPUFrequency);
		double GPUTickDelta = 1.0 / static_cast<double>(GPUFrequency);

		uint64_t* timeStampBuffer = nullptr;
		m_TimestampReadbackBuffer->Map(0, nullptr, (void**)&timeStampBuffer);

		for (ProfilingEntry& entry : m_profilingData)
			entry.GPUDurationSeconds = float(GPUTickDelta * double(timeStampBuffer[entry.GPUTimerIndexStop] - timeStampBuffer[entry.GPUTimerIndexStart]));

		D3D12_RANGE writeRange;
		writeRange.Begin = 1;
		writeRange.End = 0;

		m_TimestampReadbackBuffer->Unmap(0, &writeRange);
	}

	struct ProfilingEntry
	{
		std::string label;
		bool isResourceNode = false;
		float CPUDurationSeconds = 0.0f;
		float GPUDurationSeconds = 0.0f;

		int GPUTimerIndexStart = 0;
		int GPUTimerIndexStop = 0;
	};

	const std::vector<ProfilingEntry>& GetProfilingData() const
	{
		return m_profilingData;
	}

private:
	static const int c_numTimeStampsProfiler = 1024;

	ID3D12Device* m_device = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;

private:
	friend class ScopeProfiler;

	std::vector<ProfilingEntry> m_profilingData;
	ID3D12QueryHeap* m_TimestampQueryHeap = nullptr;
	ID3D12Resource* m_TimestampReadbackBuffer = nullptr;
	int m_timeStampIndex = 0;
	bool m_profile = false;

	int m_scopesInFlight = 0;

	int m_eventIndex = 0;
};

class ScopeProfiler
{
public:
	ScopeProfiler(Profiler& profiler, const char* viewerLabel, const char* pixLabel, bool enable, bool isResourceNode)
		: m_profiler(profiler)
		, m_enable(enable)
		, m_isResourceNode(isResourceNode)
	{
		if (!m_enable)
			return;

		// Pix
		PIXBeginEvent(m_profiler.m_commandList, PIX_COLOR_INDEX((unsigned char)m_profiler.m_eventIndex), "%s", pixLabel ? pixLabel : viewerLabel);
		m_profiler.m_eventIndex++;

		if (!m_profiler.m_profile)
			return;

		m_label = viewerLabel;

		// GPU
		m_GPUTimerIndexStart = m_profiler.m_timeStampIndex++;
		m_GPUTimerIndexStop = m_profiler.m_timeStampIndex++;
		m_profiler.m_commandList->EndQuery(m_profiler.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, m_GPUTimerIndexStart);

		// CPU
		m_CPUStart = std::chrono::high_resolution_clock::now();

		m_profiler.m_scopesInFlight++;
	}

	~ScopeProfiler()
	{
		if (!m_enable)
			return;

		// Pix
		PIXEndEvent(m_profiler.m_commandList);

		if (!m_profiler.m_profile)
			return;

		// CPU
		std::chrono::high_resolution_clock::time_point cpuStop = std::chrono::high_resolution_clock::now();
		m_CPUDurationSeconds = (float)std::chrono::duration_cast<std::chrono::duration<double>>(cpuStop - m_CPUStart).count();

		// GPU
		m_profiler.m_commandList->EndQuery(m_profiler.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, m_GPUTimerIndexStop);

		// Add the profiling data entry
		Profiler::ProfilingEntry entry;
		entry.label = m_label;
		entry.isResourceNode = m_isResourceNode;
		entry.CPUDurationSeconds = m_CPUDurationSeconds;
		entry.GPUTimerIndexStart = m_GPUTimerIndexStart;
		entry.GPUTimerIndexStop = m_GPUTimerIndexStop;
		m_profiler.m_profilingData.push_back(entry);

		m_profiler.m_scopesInFlight--;
	}

private:
	Profiler& m_profiler;

	std::string m_label;
	bool m_enable = true;
	bool m_isResourceNode = false;

	// CPU
	std::chrono::high_resolution_clock::time_point m_CPUStart;
	float m_CPUDurationSeconds = 0.0f;

	// GPU
	int m_GPUTimerIndexStart = 0;
	int m_GPUTimerIndexStop = 0;
};
