///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ReadbackObject.h"

#include <d3d12.h>
#include <string>

#include "DX12Utils/CreateResources.h"
#include "DX12Utils/DelayedReleaseTracker.h"
#include "DX12Utils/TransitionTracker.h"
#include "DX12Utils/Profiler.h"

#include "GigiAssert.h"

ID3D12Resource* ReadbackObject::OnNewFrame(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, int maxFramesInFlight, DelayedReleaseTracker& delayedReleaseTracker, TransitionTracker& transitionTracker)
{
    //BasicPixScopeProfiler _p(commandList, "ReadbackObject::OnNewFrame");

    ManageCreatedResources(device, commandList, resource, maxFramesInFlight, delayedReleaseTracker);
    if (!resource)
        return nullptr;

    ID3D12Resource* readbackResource = m_frameResources[m_frameIndex].readbackResource;

    // Copy into the readback resource
    transitionTracker.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
    transitionTracker.Flush(commandList);
    commandList->CopyResource(readbackResource, resource);

    // Move to next frame
    m_frameIndex = (m_frameIndex + 1) % maxFramesInFlight;

    // return the readback resource
    return readbackResource;
}

void ReadbackObject::ManageCreatedResources(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, int maxFramesInFlight, DelayedReleaseTracker& delayedReleaseTracker)
{
    // If nothing changed since last time, nothing to do
    if (m_resource == resource && m_frameResources.size() == maxFramesInFlight)
        return;

    // Destroy anything we have created
    Release(delayedReleaseTracker);

    // Recreate new things
    m_resource = resource;
    m_frameResources.resize(maxFramesInFlight);

    // If no resource, nothing else to do
    if (!resource)
        return;

    // Figure out how big the readback buffers should be
    size_t totalBytes = 0;
    {
        D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
        switch (resourceDesc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_BUFFER:
            {
                totalBytes = resourceDesc.Width;
                break;
            }
            // Assuming a texture
            default:
            {
                // TODO: support textures when needed
                // We use the below to calculate the total number of bytes for the readback buffer
                // We have to use CopyTextureRegion to actually do the copy though which takes extra logic. See RuntimeTypes::RenderGraphNode_Base::HandleViewableResource().
                /*
                unsigned int numSubResources = (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
                    ? resourceDesc.MipLevels
                    : resourceDesc.MipLevels * resourceDesc.DepthOrArraySize;

                device->GetCopyableFootprints(&resourceDesc, 0, numSubResources, 0, nullptr, nullptr, nullptr, &totalBytes);
                break;
                */
                return;
            }
        }
    }

    // Create the buffers
    for (FrameResource& frameResource : m_frameResources)
        frameResource.readbackResource = CreateBuffer(device, (unsigned int)totalBytes, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, "ReadbackObject readbackResource");
}

void ReadbackObject::Release(DelayedReleaseTracker& delayedReleaseTracker)
{
    m_frameIndex = 0;
    m_resource = nullptr;

    for (FrameResource& frameResource : m_frameResources)
        delayedReleaseTracker.Add(frameResource.readbackResource);

    m_frameResources.clear();
}
