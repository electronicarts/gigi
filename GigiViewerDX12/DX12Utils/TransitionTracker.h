///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include <unordered_map>
#include <vector>

#ifdef _DEBUG
#define DO_DEBUG() true
#define TRANSITION_DEBUG_INFO(RESOURCE, STATE) RESOURCE, STATE, #RESOURCE " " #STATE " " __FILE__ " " TOSTRING(__LINE__)
#define TRANSITION_DEBUG_INFO_NAMED(RESOURCE, STATE, NAME) RESOURCE, STATE, (std::string(#RESOURCE " (") + std::string(NAME) + std::string(") " #STATE " " __FILE__ " " TOSTRING(__LINE__))).c_str()
#else
#define DO_DEBUG() false
#define TRANSITION_DEBUG_INFO(RESOURCE, STATE) RESOURCE, STATE, ""
#define TRANSITION_DEBUG_INFO_NAMED(RESOURCE, STATE, NAME) RESOURCE, STATE, ""
#endif

class TransitionTracker
{
public:
	struct Item
	{
		ID3D12Resource* resource = nullptr;
		D3D12_RESOURCE_STATES newState = D3D12_RESOURCE_STATE_COMMON;
		std::string debugText = nullptr;
		bool isUAVBarrier = false;
	};

	void Track(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState, const char* debugText)
	{
		if (!resource)
			return;
		TrackedResource trackedResource;
		trackedResource.currentState = initialState;
		trackedResource.desiredState = initialState;

		#if DO_DEBUG()
		trackedResource.debugText = debugText;
		#endif

		m_trackedResources[resource] = trackedResource;
	}
	
	void Untrack(ID3D12Resource* resource)
	{
		if (!resource)
			return;
		m_trackedResources.erase(resource);
	}

	// Needed for DXR. The acceleration structures are in D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, and need a uav barrier
	void UAVBarrier(ID3D12Resource* resource)
	{
		#if DO_DEBUG()
		m_debugTransitionText.push_back("ForceUAVBarrier");
		#endif

		if (m_trackedResources.count(resource) == 0)
			return;

		TrackedResource& trackedResource = m_trackedResources[resource];
		trackedResource.wantsUAVBarrier = true;
	}

	// Needed for dealing with state promotions and decay
	void SetStateWithoutTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES newState)
	{
		#if DO_DEBUG()
		m_debugTransitionText.push_back("SetStateWithoutTransition");
		#endif

		if (m_trackedResources.count(resource) == 0)
			return;

		TrackedResource& trackedResource = m_trackedResources[resource];

		trackedResource.currentState = newState;
		trackedResource.desiredState = newState;
		trackedResource.wantsUAVBarrier = false;
	}


	void Transition(const std::vector<Item>& transitions)
	{
		for (const Item& item : transitions)
		{
			if (item.isUAVBarrier)
				UAVBarrier(item.resource);
			else
				Transition(item.resource, item.newState, item.debugText.c_str());
		}
	}

	void Transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES newState, const char* debugText)
	{
		#if DO_DEBUG()
		m_debugTransitionText.push_back(debugText);
		#endif

		if (m_trackedResources.count(resource) == 0)
			return;

		TrackedResource& trackedResource = m_trackedResources[resource];

		if (trackedResource.currentState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			trackedResource.wantsUAVBarrier = true;

		trackedResource.desiredState = newState;
	}

	void Flush(ID3D12GraphicsCommandList* commandList)
	{
		m_barriers.clear();

		for (auto& it : m_trackedResources)
		{
			ID3D12Resource* resource = it.first;
			TrackedResource& trackedResource = it.second;

			if (trackedResource.currentState != trackedResource.desiredState)
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = resource;
				barrier.Transition.StateBefore = trackedResource.currentState;
				barrier.Transition.StateAfter = trackedResource.desiredState;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				m_barriers.push_back(barrier);
			}
			else if (trackedResource.wantsUAVBarrier)
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.UAV.pResource = resource;
				m_barriers.push_back(barrier);
			}

			trackedResource.currentState = trackedResource.desiredState;
			trackedResource.wantsUAVBarrier = false;
		}

		if (m_barriers.size() > 0)
			commandList->ResourceBarrier((UINT)m_barriers.size(), m_barriers.data());

		#if DO_DEBUG()
		m_debugTransitionText.clear();
		#endif
	}

	bool Empty() const
	{
		return m_trackedResources.empty();
	}

	void Clear()
	{
		m_trackedResources.clear();
	}

private:

	struct TrackedResource
	{
		D3D12_RESOURCE_STATES currentState;
		D3D12_RESOURCE_STATES desiredState;
		bool wantsUAVBarrier = false;

		#if DO_DEBUG()
			std::string debugText;
		#endif
	};

	std::unordered_map<ID3D12Resource*, TrackedResource> m_trackedResources;
	std::vector<D3D12_RESOURCE_BARRIER> m_barriers; // a member to minimize allocations

	#if DO_DEBUG()
		std::vector<std::string> m_debugTransitionText;
	#endif
};