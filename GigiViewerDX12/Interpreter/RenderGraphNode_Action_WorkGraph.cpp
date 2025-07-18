///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"
#include "DX12Utils/CompileShaders.h"

#define ALLOW_MESH_NODES 0

void RuntimeTypes::RenderGraphNode_Action_WorkGraph::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
    RenderGraphNode_Base::Release(interpreter);

    if (m_stateObject)
    {
        m_stateObject->Release();
        m_stateObject = nullptr;
    }
}

bool CompileWorkGraph(const RenderGraphNode_Action_WorkGraph& node, RuntimeTypes::RenderGraphNode_Action_WorkGraph& runtimeData)
{
    __debugbreak();
}

bool GigiInterpreterPreviewWindowDX12::WorkGraph_MakeDescriptorTableDesc(std::vector<DescriptorTableCache::ResourceDescriptor>& descs, const RenderGraphNode_Action_WorkGraph& node, const Shader& shader, int pinOffset, std::vector<TransitionTracker::Item>& queuedTransitions)
{
    for (int resourceIndex = 0; resourceIndex < shader.resources.size(); ++resourceIndex)
    {
        const ShaderResource& shaderResource = shader.resources[resourceIndex];

        int depIndex = 0;
        while (depIndex < node.resourceDependencies.size() && node.resourceDependencies[depIndex].pinIndex != (resourceIndex + pinOffset))
            depIndex++;

        if (depIndex >= node.resourceDependencies.size())
        {
            m_logFn(LogLevel::Error, "Could not find resource dependency for shader resource \"%s\" in work graph node \"%s\"", shaderResource.name.c_str(), node.name.c_str());
            return false;
        }
        const ResourceDependency& dep = node.resourceDependencies[depIndex];

        DescriptorTableCache::ResourceDescriptor desc;

        const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];
        switch (resourceNode._index)
        {
        case RenderGraphNode::c_index_resourceTexture:
        {
            const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
            desc.m_resource = resourceInfo.m_resource;
            desc.m_format = resourceInfo.m_format;

            if (dep.pinIndex < node.linkProperties.size())
            {
                desc.m_UAVMipIndex = min(node.linkProperties[dep.pinIndex].UAVMipIndex, resourceInfo.m_numMips - 1);
            }

            switch (resourceNode.resourceTexture.dimension)
            {
            case TextureDimensionType::Texture2D: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2D; break;
            case TextureDimensionType::Texture2DArray: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2DArray; desc.m_count = resourceInfo.m_size[2]; break;
            case TextureDimensionType::Texture3D: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture3D; desc.m_count = resourceInfo.m_size[2]; break;
            case TextureDimensionType::TextureCube: desc.m_resourceType = DescriptorTableCache::ResourceType::TextureCube; desc.m_count = 6; break;
            }
            break;
        }
        case RenderGraphNode::c_index_resourceShaderConstants:
        {
            const RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_ShaderConstants(resourceNode.resourceShaderConstants.name.c_str());
            desc.m_resource = resourceInfo.m_buffer->buffer;
            desc.m_format = DXGI_FORMAT_UNKNOWN;
            desc.m_stride = (UINT)resourceInfo.m_buffer->size;
            desc.m_count = 1;
            break;
        }
        case RenderGraphNode::c_index_resourceBuffer:
        {
            const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());

            if (dep.access == ShaderResourceAccessType::RTScene)
            {
                desc.m_resource = resourceInfo.m_tlas;
                desc.m_format = DXGI_FORMAT_UNKNOWN;
                desc.m_stride = resourceInfo.m_tlasSize;
                desc.m_count = 1;
                desc.m_raw = false;
            }
            else
            {
                desc.m_resource = resourceInfo.m_resource;

                const ShaderResourceBuffer& shaderResourceBuffer = shader.resources[resourceIndex].buffer;
                bool isStructuredBuffer = ShaderResourceBufferIsStructuredBuffer(shaderResourceBuffer);
                if (isStructuredBuffer)
                {
                    desc.m_format = DXGI_FORMAT_UNKNOWN;
                    if (shaderResourceBuffer.typeStruct.structIndex != -1)
                        desc.m_stride = (UINT)m_renderGraph.structs[shaderResourceBuffer.typeStruct.structIndex].sizeInBytes;
                    else
                        desc.m_stride = DataFieldTypeInfo(shaderResourceBuffer.type).typeBytes;
                    desc.m_count = resourceInfo.m_size / desc.m_stride;
                }
                else
                {
                    desc.m_format = DataFieldTypeInfoDX12(shaderResourceBuffer.type).typeFormat;
                    desc.m_stride = 0;
                    desc.m_count = resourceInfo.m_count;
                }

                desc.m_raw = shader.resources[resourceIndex].buffer.raw;
            }
            break;
        }
        default:
        {
            m_logFn(LogLevel::Error, "Unhandled dependency node type for work graph node \"%s\"", node.name.c_str());
            return false;
        }
        }

        // This could be a temporary thing, but we can't run the work graph if we don't have the resources we need.
        if (!desc.m_resource)
            return true;

        switch (dep.access)
        {
        case ShaderResourceAccessType::UAV:
        {
            desc.m_access = DescriptorTableCache::AccessType::UAV;
            queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, GetNodeName(resourceNode).c_str()) });
            break;
        }
        case ShaderResourceAccessType::RTScene:
        {
            desc.m_access = DescriptorTableCache::AccessType::SRV;
            queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, GetNodeName(resourceNode).c_str()) });
            break;
        }
        case ShaderResourceAccessType::SRV:
        {
            desc.m_access = DescriptorTableCache::AccessType::SRV;
            break;
        }
        case ShaderResourceAccessType::CBV:
        {
            // constant buffers are upload heap, don't need transitions to be written from CPU or read by shaders
            desc.m_access = DescriptorTableCache::AccessType::CBV;
            break;
        }
        case ShaderResourceAccessType::Indirect:
        {
            // This is handled elsewhere
            continue;
        }
        default:
        {
            m_logFn(LogLevel::Error, "Unhandled shader resource access type \"%s\" for work graph node \"%s\"", EnumToString(dep.access), node.name.c_str());
            return false;
        }
        }

        switch (dep.type)
        {
        case ShaderResourceType::Texture: break;// Handled above
        case ShaderResourceType::Buffer:
        {
            if (dep.access == ShaderResourceAccessType::RTScene)
                desc.m_resourceType = DescriptorTableCache::ResourceType::RTScene;
            else
                desc.m_resourceType = DescriptorTableCache::ResourceType::Buffer;
            break;
        }
        case ShaderResourceType::ConstantBuffer: desc.m_resourceType = DescriptorTableCache::ResourceType::Buffer; break;
        default:
        {
            m_logFn(LogLevel::Error, "Unhandled shader resource type \"%s\" for work graph node \"%s\"", EnumToString(dep.type), node.name.c_str());
            return false;
        }
        }

        descs.push_back(desc);
    }

    return true;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_WorkGraph& node, RuntimeTypes::RenderGraphNode_Action_WorkGraph& runtimeData, NodeAction nodeAction)
{
    if (nodeAction == NodeAction::Init)
    {
        CompileWorkGraph(node, runtimeData);
    }
    else if (nodeAction == NodeAction::Execute)
    {
        if (runtimeData.m_failed)
            return false;

        // If we aren't supposed to do the work graph, exit out
        if (!EvaluateCondition(node.condition))
            return true;

        // Queue up transitions, so we only do them if we actually execute the node
        std::vector<TransitionTracker::Item> queuedTransitions;

#if ALLOW_MESH_NODES
        struct RenderTargetClearData
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle;
            Vec4 color;
        };
        std::vector<RenderTargetClearData> renderTargetClearData;
        struct DepthTargetClearData
        {
            bool clear = false;
            D3D12_CPU_DESCRIPTOR_HANDLE handle;
            D3D12_CLEAR_FLAGS flags;
            float depth;
            uint8_t stencil;
        };
        DepthTargetClearData depthTargetClearData;

        int rasterWidth = -1;
        int rasterHeight = -1;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> colorTargetHandles;
        D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = nullptr;

        if (runtimeData.m_usesMeshNodes)
        {
            // Shading rate image transition
            if (node.shadingRateImage.resourceNodeIndex != -1)
            {
                const RenderGraphNode& resourceNode = m_renderGraph.nodes[node.shadingRateImage.resourceNodeIndex];
                if (resourceNode._index == RenderGraphNode::c_index_resourceTexture)
                {
                    bool exists = false;
                    const auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str(), exists);
                    if (exists && textureInfo.m_resource)
                    {
                        // publish as a viewable resource
                        runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), (node.name + std::string(".shadingRateImage")).c_str(), textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, false);

                        // transition
                        queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(textureInfo.m_resource, D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE, GetNodeName(resourceNode).c_str()) });

                        if (VRSSupportLevel() == D3D12_VARIABLE_SHADING_RATE_TIER_2)
                        {
                            ID3D12GraphicsCommandList5* VRSCommandList = nullptr;
                            if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&VRSCommandList))))
                            {
                                m_logFn(LogLevel::Error, "Work Graph node \"%s\" couldn't get a ID3D12GraphicsCommandList5*", node.name.c_str());
                                return false;
                            }

                            // Set the shading rate image
                            VRSCommandList->RSSetShadingRateImage(textureInfo.m_resource);

                            VRSCommandList->Release();
                        }
                        else
                        {
                            m_logFn(LogLevel::Error, "Work Graph call node \"%s\" could not enable sparse shading because it is not supported", node.name.c_str());
                        }
                    }
                }
            }

            // Set the graphics root signature and PSO
            // TODO: Jan
            __debugbreak();

            // publish render targets as viewable resources (before)
            {
                bool firstRenderTarget = true;
                int renderTargetSize[3] = { 0, 0, 0 };

                for (int i = 0; i < node.colorTargets.size(); ++i)
                {
                    if (node.colorTargets[i].resourceNodeIndex == -1)
                        break;

                    const RenderGraphNode& colorTargetNode = m_renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
                    if (colorTargetNode._index != RenderGraphNode::c_index_resourceTexture)
                        break;

                    char buffer[256];
                    sprintf_s(buffer, "%s.colorTarget%i (Before)", node.name.c_str(), i);

                    bool exists = false;
                    auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(colorTargetNode.resourceTexture.name.c_str(), exists);
                    if (!exists || !textureInfo.m_resource)
                        break;
                    const ColorTargetSettings& ctSettings = node.colorTargetSettings[i];

                    int textureMipSize[3] = { textureInfo.m_size[0], textureInfo.m_size[1], textureInfo.m_size[2] };
                    for (int i = 0; i < ctSettings.mipLevel; ++i)
                    {
                        textureMipSize[0] = max(textureMipSize[0] / 2, 1);
                        textureMipSize[1] = max(textureMipSize[1] / 2, 1);
                    }

                    if (firstRenderTarget)
                    {
                        firstRenderTarget = false;
                        renderTargetSize[0] = textureMipSize[0];
                        renderTargetSize[1] = textureMipSize[1];
                        renderTargetSize[2] = textureMipSize[2];
                    }
                    else if (
                        renderTargetSize[0] != textureMipSize[0] ||
                        renderTargetSize[1] != textureMipSize[1])
                    {
                        std::ostringstream ss;
                        ss << "color target " << i << " is size (" << textureMipSize[0] << ", " << textureMipSize[1] << ", " << textureMipSize[2] << ")"
                            " which mismatches a previously seen render target size of (" << renderTargetSize[0] << ", " << renderTargetSize[1] << ", " << renderTargetSize[2] << ")";
                        runtimeData.m_renderGraphText = ss.str();
                        return true;
                    }

                    rasterWidth = textureMipSize[0];
                    rasterHeight = textureMipSize[1];

                    runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(colorTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, false);

                    queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(textureInfo.m_resource, D3D12_RESOURCE_STATE_RENDER_TARGET, colorTargetNode.resourceTexture.name.c_str()) });

                    D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandle;
                    if (!textureInfo.GetRTV(m_device, colorTargetHandle, m_RTVHeapAllocationTracker, colorTargetNode.resourceTexture.dimension, ctSettings.arrayIndex, ctSettings.mipLevel, colorTargetNode.resourceTexture.name.c_str()))
                    {
                        m_logFn(LogLevel::Error, "node \"%s\": cannot make RTV for \"%s\" (%i,%i).\n", node.name.c_str(), colorTargetNode.resourceTexture.name.c_str(), ctSettings.arrayIndex, ctSettings.mipLevel);
                        return false;
                    }

                    if (ctSettings.clear)
                        renderTargetClearData.push_back({ colorTargetHandle, ctSettings.clearColor });

                    colorTargetHandles.push_back(colorTargetHandle);
                }

                if (node.depthTarget.resourceNodeIndex != -1)
                {
                    const RenderGraphNode& depthTargetNode = m_renderGraph.nodes[node.depthTarget.resourceNodeIndex];
                    if (depthTargetNode._index == RenderGraphNode::c_index_resourceTexture)
                    {
                        bool exists = false;
                        auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(depthTargetNode.resourceTexture.name.c_str(), exists);
                        if (exists && textureInfo.m_resource)
                        {
                            int textureMipSize[3] = { textureInfo.m_size[0], textureInfo.m_size[1], textureInfo.m_size[2] };
                            for (int i = 0; i < node.depthMipLevel; ++i)
                            {
                                textureMipSize[0] = max(textureMipSize[0] / 2, 1);
                                textureMipSize[1] = max(textureMipSize[1] / 2, 1);
                            }

                            if (firstRenderTarget)
                            {
                                firstRenderTarget = false;
                                renderTargetSize[0] = textureMipSize[0];
                                renderTargetSize[1] = textureMipSize[1];
                                renderTargetSize[2] = textureMipSize[2];
                            }
                            else if (
                                renderTargetSize[0] != textureMipSize[0] ||
                                renderTargetSize[1] != textureMipSize[1])
                            {
                                std::ostringstream ss;
                                ss << "depth target is size (" << textureMipSize[0] << ", " << textureMipSize[1] << ", " << textureMipSize[2] << ")"
                                    " which mismatches a previously seen render target size of (" << renderTargetSize[0] << ", " << renderTargetSize[1] << ", " << renderTargetSize[2] << ")";
                                runtimeData.m_renderGraphText = ss.str();
                                return true;
                            }

                            rasterWidth = textureMipSize[0];
                            rasterHeight = textureMipSize[1];
                            char buffer[256];
                            sprintf_s(buffer, "%s.depthTarget (Before)", node.name.c_str());
                            runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(depthTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, false);

                            queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(textureInfo.m_resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, depthTargetNode.resourceTexture.name.c_str()) });

                            if (!textureInfo.GetDSV(m_device, depthTargetHandle, m_DSVHeapAllocationTracker, depthTargetNode.resourceTexture.dimension, node.depthArrayIndex, node.depthMipLevel, depthTargetNode.resourceTexture.name.c_str()))
                            {
                                m_logFn(LogLevel::Error, "node \"%s\": cannot make DSV for \"%s\" (%i,%i).\n", node.name.c_str(), depthTargetNode.resourceTexture.name.c_str(), node.depthArrayIndex, node.depthMipLevel);
                                return false;
                            }

                            depthTargetHandlePtr = &depthTargetHandle;

                            if (node.depthTargetClear || node.stencilClear)
                            {
                                D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
                                if (node.depthTargetClear)
                                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                                if (node.stencilClear)
                                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

                                depthTargetClearData.clear = true;
                                depthTargetClearData.handle = depthTargetHandle;
                                depthTargetClearData.flags = clearFlags;
                                depthTargetClearData.depth = node.depthTargetClearValue;
                                depthTargetClearData.stencil = node.stencilClearValue;
                            }
                        }
                    }
                }
            }
        }
#endif

        // publish shader resources as viewable resources, before the shader execution
        int depIndex = -1;
        for (const ResourceDependency& dep : node.resourceDependencies)
        {
            if (dep.access == ShaderResourceAccessType::Indirect)
                continue;

            depIndex++;
            const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];

            const Shader* shader = nullptr;
            std::string shaderResourceName;
            int shaderBasePinIndex = 0;
            {
                int pinIndex = dep.pinIndex;

                if (pinIndex >= 0)
                {
                    if (node.entryShader.shader && pinIndex < node.entryShader.shader->resources.size())
                    {
                        shader = node.entryShader.shader;
                        shaderResourceName = shader->resources[pinIndex].name;
                        pinIndex = -1;
                    }
                    else if (node.entryShader.shader)
                    {
                        pinIndex -= (int)node.entryShader.shader->resources.size();
                        shaderBasePinIndex += (int)node.entryShader.shader->resources.size();
                    }
                }
            }

            if (!shader)
                continue;

            switch (resourceNode._index)
            {
            case RenderGraphNode::c_index_resourceTexture:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceTexture.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - Before)";
                else
                    label = label + " (SRV)";

                const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
                runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_size, resourceInfo.m_numMips, false, false);
                break;
            }
            case RenderGraphNode::c_index_resourceShaderConstants:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceShaderConstants.name;
                label = label + " (CBV)";

                const RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_ShaderConstants(resourceNode.resourceShaderConstants.name.c_str());
                runtimeData.HandleViewableConstantBuffer(*this, label.c_str(), resourceInfo.m_buffer->buffer, (int)resourceInfo.m_buffer->size, shader->resources[dep.pinIndex - shaderBasePinIndex].constantBufferStructIndex, false, false);
                break;
            }
            case RenderGraphNode::c_index_resourceBuffer:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceBuffer.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - Before)";
                else
                    label = label + " (SRV)";

                const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
                runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, false);
                break;
            }
            }
        }

        // Make the entry shader descriptor table gigi description
        std::vector<DescriptorTableCache::ResourceDescriptor> descriptorsWorkGraph;
        int descriptorTablePinOffset = 0;
        if (node.entryShader.shader)
        {
            if (!WorkGraph_MakeDescriptorTableDesc(descriptorsWorkGraph, node, *node.entryShader.shader, descriptorTablePinOffset, queuedTransitions))
                return false;
            descriptorTablePinOffset += (int)node.entryShader.shader->resources.size();
        }

        // Do all resource transitions desired by the descriptor tables
        m_transitions.Transition(queuedTransitions);
        m_transitions.Flush(m_commandList);

        // Get or make the entry shader descriptor table and set it
        int rootSigParamIndex = 0;
        if (descriptorsWorkGraph.size() > 0)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableWorkGraph;
            std::string error;
            if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptorsWorkGraph.data(), (int)descriptorsWorkGraph.size(), descriptorTableWorkGraph, error, HEAP_DEBUG_TEXT()))
            {
                m_logFn(LogLevel::Error, "Work Graph Node \"%s\" could not allocate a descriptor table for VS: %s", node.name.c_str(), error.c_str());
                return false;
            }

            m_commandList->SetGraphicsRootDescriptorTable(rootSigParamIndex, descriptorTableWorkGraph);
            rootSigParamIndex++;
        }

#if ALLOW_MESH_NODES
        if (runtimeData.m_usesMeshNodes)
        {
            // do color and depth target clears
            for (const RenderTargetClearData& data : renderTargetClearData)
                m_commandList->ClearRenderTargetView(data.handle, data.color.data(), 0, nullptr);
            if (depthTargetClearData.clear)
                m_commandList->ClearDepthStencilView(depthTargetClearData.handle, depthTargetClearData.flags, depthTargetClearData.depth, depthTargetClearData.stencil, 0, nullptr);

            // clear viewport and scissor rect
            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(rasterWidth), float(rasterHeight), 0.0f, 1.0f };
            D3D12_RECT scissorRect = { 0, 0, (LONG)rasterWidth, (LONG)rasterHeight };
            m_commandList->RSSetViewports(1, &viewport);
            m_commandList->RSSetScissorRects(1, &scissorRect);

            m_commandList->OMSetRenderTargets((UINT)colorTargetHandles.size(), colorTargetHandles.data(), false, depthTargetHandlePtr);
            m_commandList->OMSetStencilRef(node.stencilRef);

            // variable rate shading - set sparse sampling
            if (VRSSupportLevel() > D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
            {
                ID3D12GraphicsCommandList5* VRSCommandList = nullptr;
                if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&VRSCommandList))))
                {
                    m_logFn(LogLevel::Error, "Work graph node \"%s\" couldn't get a ID3D12GraphicsCommandList5*", node.name.c_str());
                    return false;
                }

                D3D12_SHADING_RATE shadingRate;
                if (!ShadingRateToD3D12_SHADING_RATE(node.shadingRate, shadingRate))
                {
                    m_logFn(LogLevel::Error, "Unhandled shading rate \"%s\" in node \"%s\"", EnumToString(node.shadingRate), node.name.c_str());
                    return false;
                }

                D3D12_SHADING_RATE_COMBINER combiners[2];
                if (!ShadingRateCombinerToD3D12_SHADING_RATE_COMBINER(node.shadingRateCombiner1, combiners[0]))
                {
                    m_logFn(LogLevel::Error, "Unhandled shading rate combiner \"%s\" in node \"%s\"", EnumToString(node.shadingRateCombiner1), node.name.c_str());
                    return false;
                }

                if (!ShadingRateCombinerToD3D12_SHADING_RATE_COMBINER(node.shadingRateCombiner2, combiners[1]))
                {
                    m_logFn(LogLevel::Error, "Unhandled shading rate combiner \"%s\" in node \"%s\"", EnumToString(node.shadingRateCombiner2), node.name.c_str());
                    return false;
                }

                VRSCommandList->RSSetShadingRate(shadingRate, combiners);

                VRSCommandList->Release();
            }
        }
#endif

        std::ostringstream ss;
        ss << "DispatchGraph:\n  " << node.name << '\n';

        __debugbreak();

        D3D12_DISPATCH_GRAPH_DESC dispatchDesc = {};
        dispatchDesc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT;
        dispatchDesc.NodeCPUInput = {};
        dispatchDesc.NodeCPUInput.EntrypointIndex = runtimeData.m_entrypointIndex;
        // Launch graph with one record TODO: jan, make flexible
        dispatchDesc.NodeCPUInput.NumRecords = 1;
        // Record does not contain any data
        dispatchDesc.NodeCPUInput.RecordStrideInBytes = 0;
        dispatchDesc.NodeCPUInput.pRecords = nullptr;

        // Set program and dispatch the work graphs.
        // See
        // https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#setprogram
        // https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#dispatchgraph

        //commandList->SetProgram(&programDesc_);
        //commandList->DispatchGraph(&dispatchDesc);

        // Clear backing memory initialization flag, as the graph has run at least once now
        // See https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#d3d12_set_work_graph_flags
        runtimeData.m_programDesc.WorkGraph.Flags &= ~D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;

#if ALLOW_MESH_NODES
        if (runtimeData.m_usesMeshNodes)
        {
            // variable rate shading - set it back to dense sampling
            if (VRSSupportLevel() > D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
            {
                ID3D12GraphicsCommandList5* VRSCommandList = nullptr;
                if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&VRSCommandList))))
                {
                    m_logFn(LogLevel::Error, "Work graph node \"%s\" couldn't get a ID3D12GraphicsCommandList5*", node.name.c_str());
                    return false;
                }

                VRSCommandList->RSSetShadingRate(D3D12_SHADING_RATE_1X1, nullptr);

                if (VRSSupportLevel() == D3D12_VARIABLE_SHADING_RATE_TIER_2)
                    VRSCommandList->RSSetShadingRateImage(nullptr);

                VRSCommandList->Release();
            }

            // publish render targets as viewable resources (after)
            {
                for (int i = 0; i < node.colorTargets.size(); ++i)
                {
                    if (node.colorTargets[i].resourceNodeIndex == -1)
                        break;

                    const RenderGraphNode& colorTargetNode = m_renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
                    if (colorTargetNode._index != RenderGraphNode::c_index_resourceTexture)
                        break;

                    char buffer[256];
                    sprintf_s(buffer, "%s.colorTarget%i (After)", node.name.c_str(), i);

                    bool exists = false;
                    const auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(colorTargetNode.resourceTexture.name.c_str(), exists);
                    if (!exists || !textureInfo.m_resource)
                        break;

                    runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(colorTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, true);
                }

                if (node.depthTarget.resourceNodeIndex != -1)
                {
                    const RenderGraphNode& depthTargetNode = m_renderGraph.nodes[node.depthTarget.resourceNodeIndex];
                    if (depthTargetNode._index == RenderGraphNode::c_index_resourceTexture)
                    {
                        bool exists = false;
                        const auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(depthTargetNode.resourceTexture.name.c_str(), exists);
                        if (exists && textureInfo.m_resource)
                        {
                            char buffer[256];
                            sprintf_s(buffer, "%s.depthTarget (After)", node.name.c_str());
                            runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(depthTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, true);
                        }
                    }
                }
            }
        }
#endif

        if (IsConditional(node.condition))
            ss << "\nCondition: " << (EvaluateCondition(node.condition) ? "true" : "false");
        runtimeData.m_renderGraphText = ss.str();

        // publish shader resources as viewable resources, after the shader execution
        depIndex = -1;
        for (const ResourceDependency& dep : node.resourceDependencies)
        {
            if (dep.access == ShaderResourceAccessType::Indirect)
                continue;

            depIndex++;
            const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];

            const Shader* shader = nullptr;
            std::string shaderResourceName;
            int shaderBasePinIndex = 0;
            {
                int pinIndex = dep.pinIndex;

                if (pinIndex >= 0)
                {
                    if (node.entryShader.shader && pinIndex < node.entryShader.shader->resources.size())
                    {
                        shader = node.entryShader.shader;
                        shaderResourceName = shader->resources[pinIndex].name;
                        pinIndex = -1;
                    }
                    else if (node.entryShader.shader)
                    {
                        pinIndex -= (int)node.entryShader.shader->resources.size();
                        shaderBasePinIndex += (int)node.entryShader.shader->resources.size();
                    }
                }
            }

            if (!shader)
                continue;

            switch (resourceNode._index)
            {
            case RenderGraphNode::c_index_resourceTexture:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceTexture.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - After)";
                else
                    continue;

                const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
                runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_size, resourceInfo.m_numMips, false, true);
                break;
            }
            case RenderGraphNode::c_index_resourceBuffer:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceBuffer.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - After)";
                else
                    continue;

                const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
                runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, true);
                break;
            }
            }
        }

        return true;
    }
    return true;
}