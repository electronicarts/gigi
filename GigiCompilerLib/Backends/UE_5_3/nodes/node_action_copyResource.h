///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

static bool ProcessNodeTag(const RenderGraph& renderGraph, const RenderGraphNode_Action_CopyResource& node, std::string& tag, std::string& text)
{
    // Execute the technique
    if (tag == "/*$(AddTechnique)*/")
    {
        // /*$(Node:Condition)*/
        {
            std::ostringstream condition;

            if (node.condition.comparison != ConditionComparison::Count)
                condition << "    if (" << ConditionToString(node.condition, renderGraph, &VariableToString) << ")\n";

            StringReplaceAll(text, "/*$(Node:Condition)*/", condition.str());
        }

        // /*$(Node:DoCopy)*/
        {
            std::ostringstream docopy;

			int sourceIndex =
				GetNodeIsResourceNode(renderGraph.nodes[node.source.nodeIndex])
				? node.source.nodeIndex
				: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.source.nodeIndex], node.source.nodePinIndex);

			int destIndex =
				GetNodeIsResourceNode(renderGraph.nodes[node.dest.nodeIndex])
				? node.dest.nodeIndex
				: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.dest.nodeIndex], node.dest.nodePinIndex);

			const RenderGraphNode& sourceNode = renderGraph.nodes[sourceIndex];
			const RenderGraphNode& destNode = renderGraph.nodes[destIndex];

			std::string sourceNodeName = GetNodeName(sourceNode);
			std::string destNodeName = GetNodeName(destNode);

			if (sourceNode._index == destNode._index)
			{
				switch (sourceNode._index)
				{
					case RenderGraphNode::c_index_resourceBuffer:
					{
						docopy <<
							"        // Copy from " << sourceNodeName << " to " << destNodeName << "\n"
							"        AddCopyBufferPass(GraphBuilder, Buffer_" << sourceNodeName << ", Buffer_" << destNodeName << ");\n"
							;

                        if (node.bufferToBuffer.srcBegin != 0 || node.bufferToBuffer.destBegin != 0 || node.bufferToBuffer.size != 0)
                        {
                            docopy << "\n        TODO: this needs to be a partial buffer copy. srcBegin = " << node.bufferToBuffer.srcBegin << ", destBegin = " << node.bufferToBuffer.destBegin << ", size = ";
                            if (node.bufferToBuffer.size == 0)
                                docopy << "0 (all)";
                            else
                                docopy << node.bufferToBuffer.size;
                            docopy << " (in bytes)\n";
                        }

						break;
					}
					case RenderGraphNode::c_index_resourceTexture:
					{
						docopy <<
							"        // Copy from " << sourceNodeName << " to " << destNodeName << "\n"
							"        FRHICopyTextureInfo CopyTextureInfo;\n"
							"        CopyTextureInfo.Size = FIntVector(Texture_" << destNodeName << "->Desc.GetSize().X, Texture_" << destNodeName << "->Desc.GetSize().Y, 0);\n"
							"        AddCopyTexturePass(GraphBuilder, Texture_" << sourceNodeName << ", Texture_" << destNodeName << ", CopyTextureInfo);\n"
							;
						break;
					}
					default:
					{
                        GigiAssert(false, "Unsupported node type.");
					}
				}
			}
			else
			{
				// Not currently supported.
                GigiAssert(false, "Not supported in UE when this code was written.");

				// A custom compute shader to do copying could be emitted!
			}

            StringReplaceAll(text, "/*$(Node:DoCopy)*/", docopy.str());
        }
    }

    return true;
}
