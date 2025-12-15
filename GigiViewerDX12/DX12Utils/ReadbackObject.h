///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

class DelayedReleaseTracker;
class TransitionTracker;

#include <vector>

// This is a helper object to read back a resource every frame.
// Currently only supports buffers.
struct ReadbackObject
{
    // Call once per frame. Returns a readback resource, or nullptr on failure
    ID3D12Resource* OnNewFrame(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, int maxFramesInFlight, DelayedReleaseTracker& delayedReleaseTracker, TransitionTracker& transitionTracker);

    void Release(DelayedReleaseTracker& delayedReleaseTracker);

private:
    void ManageCreatedResources(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, int maxFramesInFlight, DelayedReleaseTracker& delayedReleaseTracker);

    struct FrameResource
    {
        ID3D12Resource* readbackResource = nullptr;
    };

    int m_frameIndex = 0;
    ID3D12Resource* m_resource = nullptr;
    std::vector<FrameResource> m_frameResources;
};
