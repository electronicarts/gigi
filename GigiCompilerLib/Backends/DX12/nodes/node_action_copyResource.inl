///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_CopyResource& node)
{
    // Copy resource doesn't need any storage or initialization

    // Execute
    int sourceIndex =
        GetNodeIsResourceNode(renderGraph.nodes[node.source.nodeIndex])
        ? node.source.nodeIndex
        : GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.source.nodeIndex], node.source.nodePinIndex);

    int destIndex =
        GetNodeIsResourceNode(renderGraph.nodes[node.dest.nodeIndex])
        ? node.dest.nodeIndex
        : GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.dest.nodeIndex], node.dest.nodePinIndex);

    RenderGraphNode& sourceNode = renderGraph.nodes[sourceIndex];
    RenderGraphNode& destNode = renderGraph.nodes[destIndex];

    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n        // Copy Resource: " << node.name;

    // condition for execution
    if (node.condition.comparison != ConditionComparison::Count)
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n        if(" << ConditionToString(node.condition, renderGraph, &VariableToString) << ")";
    }

    // Perf Event Begin
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n        {"
        "\n            ScopedPerfEvent scopedPerf(\"Copy Resource: " << node.name << "\", commandList, " << node.nodeIndex << ");"
        "\n            std::chrono::high_resolution_clock::time_point startPointCPU;"
        "\n            if(context->m_profile)"
        "\n            {"
        "\n                startPointCPU = std::chrono::high_resolution_clock::now();"
        "\n                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);"
        "\n            }"
        "\n";

    // If the source and dest are the same type of resource
    if (sourceNode._index == destNode._index)
    {
        const char* prefix = "";
        switch (sourceNode._index)
        {
            case RenderGraphNode::c_index_resourceTexture: prefix = "texture_"; break;
            case RenderGraphNode::c_index_resourceBuffer: prefix = "buffer_";  break;
            default:
            {
                Assert(false, "unhandled resource type in CopyResource node");
                break;
            }
        }

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n            // Even if two buffers have the same stride and count, one could be padded for alignment differently based on use"
            "\n            unsigned int srcSize = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(sourceNode)) << prefix << GetNodeName(sourceNode) << "->GetDesc().Width;"
            "\n            unsigned int destSize = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(destNode)) << prefix << GetNodeName(destNode) << "->GetDesc().Width;"
            "\n            if (srcSize == destSize)"
            "\n                commandList->CopyResource("
            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(destNode)) << prefix << GetNodeName(destNode) << ", "
            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(sourceNode)) << prefix << GetNodeName(sourceNode) <<
            ");"
            "\n            else"
            "\n                commandList->CopyBufferRegion("
            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(destNode)) << prefix << GetNodeName(destNode) << ", 0, "
            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(sourceNode)) << prefix << GetNodeName(sourceNode) << ", 0, min(srcSize, destSize));"
            ;
    }
    else
    {
        const char* srcPrefix = "";
        switch (sourceNode._index)
        {
            case RenderGraphNode::c_index_resourceTexture: srcPrefix = "texture_"; break;
            case RenderGraphNode::c_index_resourceBuffer: srcPrefix = "buffer_";  break;
            default:
            {
                Assert(false, "unhandled resource type in CopyResource node");
                break;
            }
        }

        const char* destPrefix = "";
        switch (destNode._index)
        {
            case RenderGraphNode::c_index_resourceTexture: destPrefix = "texture_"; break;
            case RenderGraphNode::c_index_resourceBuffer: destPrefix = "buffer_";  break;
            default:
            {
                Assert(false, "unhandled resource type in CopyResource node");
                break;
            }
        }

        // Copy buffer to texture
        if (sourceNode._index == RenderGraphNode::c_index_resourceBuffer)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n            D3D12_RESOURCE_DESC destResourceDesc = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(destNode)) << destPrefix << GetNodeName(destNode) << "->GetDesc();"
                "\n            unsigned char destLayoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];"
                "\n            D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)destLayoutMem;"
                "\n            device->GetCopyableFootprints(&destResourceDesc, 0, 1, 0, destLayout, nullptr, nullptr, nullptr);"
                "\n"
                "\n            D3D12_TEXTURE_COPY_LOCATION destLoc = {};"
                "\n            destLoc.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(destNode)) << destPrefix << GetNodeName(destNode) << ";"
                "\n            destLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;"
                "\n            destLoc.SubresourceIndex = 0;"
                "\n"
                "\n            D3D12_TEXTURE_COPY_LOCATION srcLoc = {};"
                "\n            srcLoc.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(sourceNode)) << srcPrefix << GetNodeName(sourceNode) << ";"
                "\n            srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;"
                "\n            srcLoc.PlacedFootprint = *destLayout;"
                "\n"
                "\n            commandList->CopyTextureRegion(&destLoc, 0, 0, 0, &srcLoc, nullptr);"
                ;
        }
        // Copy texture to buffer
        else
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n            D3D12_RESOURCE_DESC srcResourceDesc = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(sourceNode)) << srcPrefix << GetNodeName(sourceNode) << "->GetDesc();"
                "\n            unsigned char srcLayoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];"
                "\n            D3D12_PLACED_SUBRESOURCE_FOOTPRINT* srcLayout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)srcLayoutMem;"
                "\n            device->GetCopyableFootprints(&srcResourceDesc, 0, 1, 0, srcLayout, nullptr, nullptr, nullptr);"
                "\n"
                "\n            D3D12_TEXTURE_COPY_LOCATION srcLoc = {};"
                "\n            srcLoc.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(sourceNode)) << srcPrefix << GetNodeName(sourceNode) << ";"
                "\n            srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;"
                "\n            srcLoc.SubresourceIndex = 0;"
                "\n"
                "\n            D3D12_TEXTURE_COPY_LOCATION destLoc = {};"
                "\n            destLoc.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(destNode)) << destPrefix << GetNodeName(destNode) << ";"
                "\n            destLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;"
                "\n            destLoc.PlacedFootprint = *srcLayout;"
                "\n"
                "\n            commandList->CopyTextureRegion(&destLoc, 0, 0, 0, &srcLoc, nullptr);"
                ;
        }
    }

    // Perf Event End
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n            if(context->m_profile)"
        "\n            {"
        "\n                context->m_profileData[(s_timerIndex-1)/2].m_label = \"" << node.name << "\";"
        "\n                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();"
        "\n                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);"
        "\n            }"
        "\n        }";


    if (node.condition.comparison != ConditionComparison::Count)
    {
        stringReplacementMap["/*$(ReadbackProfileData)*/"] <<
            "\n        if(" << ConditionToString(node.condition, renderGraph, &VariableToStringInsideContext) << ")"
            "\n        {"
            "\n            m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // copy resource: " << node.name <<
            "\n        }";
    }
    else
    {
        stringReplacementMap["/*$(ReadbackProfileData)*/"] <<
            "\n        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // copy resource: " << node.name;
    }
}
