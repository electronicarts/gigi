///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_CopyResource& node)
{
    // Get the source and destination nodes
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

    // Get the source and destination node prefix
    const char* srcPrefix = "";
    bool srcIsBuffer = true;
    switch (sourceNode._index)
    {
        case RenderGraphNode::c_index_resourceTexture: srcPrefix = "texture"; srcIsBuffer = false; break;
        case RenderGraphNode::c_index_resourceBuffer: srcPrefix = "buffer";  break;
        default:
        {
            GigiAssert(false, "unhandled resource type in CopyResource node");
            break;
        }
    }

    const char* destPrefix = "";
    bool destIsBuffer = true;
    switch (destNode._index)
    {
        case RenderGraphNode::c_index_resourceTexture: destPrefix = "texture"; destIsBuffer = false; break;
        case RenderGraphNode::c_index_resourceBuffer: destPrefix = "buffer";  break;
        default:
        {
            GigiAssert(false, "unhandled resource type in CopyResource node");
            break;
        }
    }

    stringReplacementMap["/*$(Execute)*/"] <<
        "    // Copy " << srcPrefix << " " << GetNodeName(sourceNode) << " to " << destPrefix << " " << GetNodeName(destNode) << "\n"
        ;

    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "    // " << node.comment << "\n"
            ;
    }

    WriteCondition(renderGraph, stringReplacementMap["/*$(Execute)*/"], "    ", node.condition);

    stringReplacementMap["/*$(Execute)*/"] <<
        "    {\n"
        ;

    if (srcIsBuffer == destIsBuffer)
    {
        if (srcIsBuffer)
        {
            // Copy buffer to buffer
            stringReplacementMap["/*$(Execute)*/"] <<
                "        encoder.copyBufferToBuffer(\n"
                "            this.buffer_" << GetNodeName(sourceNode) << ",\n"
                "            0,\n"
                "            this.buffer_" << GetNodeName(destNode) << ",\n"
                "            0,\n"
                "            Math.min(this.buffer_" << GetNodeName(sourceNode) << ".size, this.buffer_" << GetNodeName(destNode) << ".size)\n"
                "        );\n"
                ;
        }
        else
        {
            // Copy texture to texture
            stringReplacementMap["/*$(Execute)*/"] <<
                "        const numMips = Math.min(this.texture_" << GetNodeName(sourceNode) << ".mipLevelCount, this.texture_" << GetNodeName(destNode) << ".mipLevelCount);\n"
                "\n"
                "        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)\n"
                "        {\n"
                "            const mipWidth = Math.max(this.texture_" << GetNodeName(sourceNode) << ".width >> mipIndex, 1);\n"
                "            const mipHeight = Math.max(this.texture_" << GetNodeName(sourceNode) << ".height >> mipIndex, 1);\n"
                "            let mipDepth = this.texture_" << GetNodeName(sourceNode) << ".depthOrArrayLayers;\n"
                "\n"
                "            if (this.texture_" << GetNodeName(sourceNode) << ".dimension == \"3d\")\n"
                "                mipDepth = Math.max(mipDepth >> mipIndex, 1);\n"
                "\n"
                "            encoder.copyTextureToTexture(\n"
                "                { texture: this.texture_" << GetNodeName(sourceNode) << ", mipLevel: mipIndex },\n"
                "                { texture: this.texture_" << GetNodeName(destNode) << ", mipLevel: mipIndex },\n"
                "                {\n"
                "                    width: mipWidth,\n"
                "                    height: mipHeight,\n"
                "                    depthOrArrayLayers: mipDepth,\n"
                "                }\n"
                "            );\n"
                "        }\n"
                ;
       }
    }
    else
    {
        if (srcIsBuffer)
        {
            // Copy buffer to texture
            stringReplacementMap["/*$(Execute)*/"] <<
                "        const bytesPerRow = Shared.GetTextureFormatInfo(this.texture_" << GetNodeName(destNode) << ".format).bytesPerPixel * this.texture_" << GetNodeName(destNode) << ".width;\n"
                "\n"
                "        encoder.copyBufferToTexture(\n"
                "            { buffer: this.buffer_" << GetNodeName(sourceNode) << ", bytesPerRow: bytesPerRow },\n"
                "            { texture: this.texture_" << GetNodeName(destNode) << " },\n"
                "            {\n"
                "                width: this.texture_" << GetNodeName(destNode) << ".width,\n"
                "                height: this.texture_" << GetNodeName(destNode) << ".height,\n"
                "                depthOrArrayLayers: this.texture_" << GetNodeName(destNode) << ".depthOrArrayLayers,\n"
                "            }\n"
                "        );\n"
                ;
        }
        else
        {
            // Copy texture to buffer
            stringReplacementMap["/*$(Execute)*/"] <<
                "        const bytesPerRow = Shared.GetTextureFormatInfo(this.texture_" << GetNodeName(sourceNode) << ".format).bytesPerPixel * this.texture_" << GetNodeName(sourceNode) << ".width;\n"
                "\n"
                "        encoder.copyTextureToBuffer(\n"
                "            { texture: this.texture_" << GetNodeName(sourceNode) << " },\n"
                "            { buffer: this.buffer_" << GetNodeName(destNode) << ", bytesPerRow: bytesPerRow },\n"
                "            {\n"
                "                width: this.texture_" << GetNodeName(sourceNode) << ".width,\n"
                "                height: this.texture_" << GetNodeName(sourceNode) << ".height,\n"
                "                depthOrArrayLayers: this.texture_" << GetNodeName(sourceNode) << ".depthOrArrayLayers,\n"
                "            }\n"
                "        );\n"
                ;
        }
    }

    stringReplacementMap["/*$(Execute)*/"] <<
        "    }\n"
        "\n"
        ;
}
