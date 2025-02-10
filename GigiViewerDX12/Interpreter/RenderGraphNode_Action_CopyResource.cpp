///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_CopyResource& node, RuntimeTypes::RenderGraphNode_Action_CopyResource& runtimeData, NodeAction nodeAction)
{
	ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, false);

	if (nodeAction == NodeAction::Execute)
	{
		std::ostringstream ss;
		if (IsConditional(node.condition))
			ss << "\nCondition: " << (EvaluateCondition(node.condition) ? "true" : "false");
		runtimeData.m_renderGraphText = ss.str();

		bool executionConditionMet = EvaluateCondition(node.condition);
		if (!executionConditionMet)
			return true;

		if (node.source.resourceNodeIndex == -1 || node.dest.resourceNodeIndex == -1)
			return false;

		RenderGraphNode& src = m_renderGraph.nodes[node.source.resourceNodeIndex];
		RenderGraphNode& dest = m_renderGraph.nodes[node.dest.resourceNodeIndex];

		// If the source and dest are the same type of resource
		if (src._index == dest._index)
		{
			// copy buffer to buffer
			if (src._index == RenderGraphNode::c_index_resourceBuffer)
			{
				std::string srcNodeName = src.resourceBuffer.name;
				std::string destNodeName = dest.resourceBuffer.name;

				bool srcExists;
				RuntimeTypes::RenderGraphNode_Resource_Buffer& srcRT = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(srcNodeName.c_str(), srcExists);

				bool destExists;
				RuntimeTypes::RenderGraphNode_Resource_Buffer& destRT = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(destNodeName.c_str(), destExists);

				if (!srcExists || !srcRT.m_resource || !destExists || !destRT.m_resource)
				{
					std::ostringstream ss;
					if (!srcExists || !srcRT.m_resource)
						ss << "source resource not created: \"" << srcNodeName << "\"\n";
					if (!destExists || !destRT.m_resource)
						ss << "destination resource not created: \"" << destNodeName << "\"\n";
					runtimeData.m_renderGraphText = ss.str();
					return true;
				}

				// Compare sizes using count * stride. We don't use m_size because that may be padded for alignment reasons.
				int srcCopySize = srcRT.m_count * srcRT.m_stride;
				int destCopySize = destRT.m_count * destRT.m_stride;
				if (srcCopySize != destCopySize)
				{
					m_logFn(LogLevel::Error, "Could not copy buffer \"%s\" to buffer \"%s\" because they are different sizes.\n", srcNodeName.c_str(), destNodeName.c_str());
					return false;
				}

				// Allow copies if the structs are compatible (same number of fields, and same type of fields, in same order)
				bool structsCompatible = true;
				if (srcRT.m_structIndex != destRT.m_structIndex)
				{
					const Struct& srcStruct = m_renderGraph.structs[srcRT.m_structIndex];
					const Struct& destStruct = m_renderGraph.structs[destRT.m_structIndex];
					if (srcStruct.fields.size() == destStruct.fields.size())
					{
						for (size_t fieldIndex = 0; fieldIndex < srcStruct.fields.size(); ++fieldIndex)
						{
							if (srcStruct.fields[fieldIndex].type != destStruct.fields[fieldIndex].type)
							{
								structsCompatible = false;
								break;
							}
						}
					}
					else
						structsCompatible = false;
				}

				if (srcRT.m_format != destRT.m_format || !structsCompatible)
				{
					const char* srcStruct = (srcRT.m_structIndex == -1) ? "None" : m_renderGraph.structs[srcRT.m_structIndex].name.c_str();
					const char* destStruct = (destRT.m_structIndex == -1) ? "None" : m_renderGraph.structs[destRT.m_structIndex].name.c_str();
					m_logFn(LogLevel::Error, "Could not copy buffer \"%s\" (format %s struct %s) to buffer \"%s\" (format %s struct %s) because they are different formats.\n", srcNodeName.c_str(), Get_DXGI_FORMAT_Info(srcRT.m_format).name, srcStruct, destNodeName.c_str(), Get_DXGI_FORMAT_Info(destRT.m_format).name, destStruct);
					return false;
				}

				runtimeData.HandleViewableBuffer(*this, (node.name + ".Source").c_str(), srcRT.m_resource, srcRT.m_format, srcRT.m_formatCount, srcRT.m_structIndex, srcRT.m_size, srcRT.m_stride, srcRT.m_count, false, false);
				runtimeData.HandleViewableBuffer(*this, (node.name + ".Dest (Before)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_formatCount, destRT.m_structIndex, destRT.m_size, destRT.m_stride, destRT.m_count, false, false);

				m_transitions.Transition(TRANSITION_DEBUG_INFO(srcRT.m_resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				m_transitions.Transition(TRANSITION_DEBUG_INFO(destRT.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Flush(m_commandList);

				// If they are the same size, there is no padding difference, and they can be coppied in full.
				if (srcRT.m_size == destRT.m_size)
					m_commandList->CopyResource(destRT.m_resource, srcRT.m_resource);
				// Else, they would be the same size if not for the padding, so we need to specify the unpadded amount to actually copy over.
				else
					m_commandList->CopyBufferRegion(destRT.m_resource, 0, srcRT.m_resource, 0, srcCopySize);

				runtimeData.HandleViewableBuffer(*this, (node.name + ".Dest (After)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_formatCount, destRT.m_structIndex, destRT.m_size, destRT.m_stride, destRT.m_count, false, true);
			}
			// copy texture to texture
			else
			{
				std::string srcNodeName = src.resourceTexture.name;
				std::string destNodeName = dest.resourceTexture.name;

				bool srcExists;
				RuntimeTypes::RenderGraphNode_Resource_Texture& srcRT = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(srcNodeName.c_str(), srcExists);

				bool destExists;
				RuntimeTypes::RenderGraphNode_Resource_Texture& destRT = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(destNodeName.c_str(), destExists);

				if (!srcExists || !srcRT.m_resource || !destExists || !destRT.m_resource)
				{
					std::ostringstream ss;
					if (!srcExists || !srcRT.m_resource)
						ss << "source resource not created: \"" << srcNodeName << "\"\n";
					if (!destExists || !destRT.m_resource)
						ss << "destination resource not created: \"" << destNodeName << "\"\n";
					runtimeData.m_renderGraphText = ss.str();
					return true;
				}

				if (srcRT.m_size[0] != destRT.m_size[0] || srcRT.m_size[1] != destRT.m_size[1] || srcRT.m_size[2] != destRT.m_size[2])
				{
					m_logFn(LogLevel::Error, "Could not copy texture \"%s\" to texture \"%s\" because they are different sizes.\n", srcNodeName.c_str(), destNodeName.c_str());
					return false;
				}

				if(!FormatsCompatibleForCopyResource(srcRT.m_format, destRT.m_format))
				{
					m_logFn(LogLevel::Error, "Could not copy texture \"%s\" to texture \"%s\" because they are incompatible formats.\n", srcNodeName.c_str(), destNodeName.c_str());
					return false;
				}

				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(src.resourceTexture.dimension), (node.name + ".Source").c_str(), srcRT.m_resource, srcRT.m_format, srcRT.m_size, srcRT.m_numMips, false, false);
				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(dest.resourceTexture.dimension), (node.name + ".Dest (Before)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_size, destRT.m_numMips, false, false);

				m_transitions.Transition(TRANSITION_DEBUG_INFO(srcRT.m_resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				m_transitions.Transition(TRANSITION_DEBUG_INFO(destRT.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Flush(m_commandList);

				m_commandList->CopyResource(destRT.m_resource, srcRT.m_resource);

				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(dest.resourceTexture.dimension), (node.name + ".Dest (After)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_size, destRT.m_numMips, false, true);
			}
		}
		// else they are different types of resources
		else
		{
			// copy buffer to texture
			if (src._index == RenderGraphNode::c_index_resourceBuffer)
			{
				std::string srcNodeName = src.resourceBuffer.name;
				std::string destNodeName = dest.resourceTexture.name;

				bool srcExists;
				RuntimeTypes::RenderGraphNode_Resource_Buffer& srcRT = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(srcNodeName.c_str(), srcExists);

				bool destExists;
				RuntimeTypes::RenderGraphNode_Resource_Texture& destRT = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(destNodeName.c_str(), destExists);

				if (!srcExists || !srcRT.m_resource || !destExists || !destRT.m_resource)
				{
					std::ostringstream ss;
					if (!srcExists || !srcRT.m_resource)
						ss << "source resource not created: \"" << srcNodeName << "\"\n";
					if (!destExists || !destRT.m_resource)
						ss << "destination resource not created: \"" << destNodeName << "\"\n";
					runtimeData.m_renderGraphText = ss.str();
					return true;
				}

				if (srcRT.m_count != destRT.m_size[0] * destRT.m_size[1] * destRT.m_size[2])
				{
					m_logFn(LogLevel::Error, "Could not copy buffer \"%s\" to texture \"%s\" because they are different sizes.\n", srcNodeName.c_str(), destNodeName.c_str());
					return false;
				}

				if (srcRT.m_format != destRT.m_format)
				{
					m_logFn(LogLevel::Error, "Could not copy buffer \"%s\" (format %s) to texture \"%s\" (format %s) because they are different formats.\n", srcNodeName.c_str(), Get_DXGI_FORMAT_Info(srcRT.m_format).name, destNodeName.c_str(), Get_DXGI_FORMAT_Info(destRT.m_format).name);
					return false;
				}

				runtimeData.HandleViewableBuffer(*this, (node.name + ".Source").c_str(), srcRT.m_resource, srcRT.m_format, srcRT.m_formatCount, srcRT.m_structIndex, srcRT.m_size, srcRT.m_stride, srcRT.m_count, false, false);
				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(dest.resourceTexture.dimension), (node.name + ".Dest (Before)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_size, destRT.m_numMips, false, false);

				m_transitions.Transition(TRANSITION_DEBUG_INFO(srcRT.m_resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				m_transitions.Transition(TRANSITION_DEBUG_INFO(destRT.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Flush(m_commandList);

				D3D12_RESOURCE_DESC destResourceDesc = destRT.m_resource->GetDesc();
				unsigned char destLayoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)destLayoutMem;
				m_device->GetCopyableFootprints(&destResourceDesc, 0, 1, 0, destLayout, nullptr, nullptr, nullptr);

				D3D12_TEXTURE_COPY_LOCATION destLoc = {};
				destLoc.pResource = destRT.m_resource;
				destLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				destLoc.SubresourceIndex = 0;

				D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
				srcLoc.pResource = srcRT.m_resource;
				srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				srcLoc.PlacedFootprint = *destLayout;

				m_commandList->CopyTextureRegion(&destLoc, 0, 0, 0, &srcLoc, nullptr);

				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(dest.resourceTexture.dimension), (node.name + ".Dest (After)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_size, destRT.m_numMips, false, true);

			}
			// copy texture to buffer
			else
			{
				std::string srcNodeName = src.resourceTexture.name;
				std::string destNodeName = dest.resourceBuffer.name;

				bool srcExists;
				RuntimeTypes::RenderGraphNode_Resource_Texture& srcRT = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(srcNodeName.c_str(), srcExists);

				bool destExists;
				RuntimeTypes::RenderGraphNode_Resource_Buffer& destRT = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(destNodeName.c_str(), destExists);

				if (!srcExists || !srcRT.m_resource || !destExists || !destRT.m_resource)
				{
					std::ostringstream ss;
					if (!srcExists || !srcRT.m_resource)
						ss << "source resource not created: \"" << srcNodeName << "\"\n";
					if (!destExists || !destRT.m_resource)
						ss << "destination resource not created: \"" << destNodeName << "\"\n";
					runtimeData.m_renderGraphText = ss.str();
					return true;
				}

				if (srcRT.m_size[0] * srcRT.m_size[1] * srcRT.m_size[2] != destRT.m_count)
				{
					m_logFn(LogLevel::Error, "Could not copy texture \"%s\" to buffer \"%s\" because they are different sizes.\n", srcNodeName.c_str(), destNodeName.c_str());
					return false;
				}

				if (!FormatsCompatibleForCopyResource(srcRT.m_format, destRT.m_format))
				{
					m_logFn(LogLevel::Error, "Could not copy texture \"%s\" to buffer \"%s\" because they are incompatible formats.\n", srcNodeName.c_str(), destNodeName.c_str());
					return false;
				}

				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(src.resourceTexture.dimension), (node.name + ".Source").c_str(), srcRT.m_resource, srcRT.m_format, srcRT.m_size, srcRT.m_numMips, false, false);
				runtimeData.HandleViewableBuffer(*this, (node.name + ".Dest (Before)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_formatCount, destRT.m_structIndex, destRT.m_size, destRT.m_stride, destRT.m_count, false, false);

				m_transitions.Transition(TRANSITION_DEBUG_INFO(srcRT.m_resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				m_transitions.Transition(TRANSITION_DEBUG_INFO(destRT.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Flush(m_commandList);

				D3D12_RESOURCE_DESC srcResourceDesc = srcRT.m_resource->GetDesc();
				unsigned char srcLayoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* srcLayout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)srcLayoutMem;
				m_device->GetCopyableFootprints(&srcResourceDesc, 0, 1, 0, srcLayout, nullptr, nullptr, nullptr);

				D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
				srcLoc.pResource = srcRT.m_resource;
				srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				srcLoc.SubresourceIndex = 0;

				D3D12_TEXTURE_COPY_LOCATION destLoc = {};
				destLoc.pResource = destRT.m_resource;
				destLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				destLoc.PlacedFootprint = *srcLayout;

				m_commandList->CopyTextureRegion(&destLoc, 0, 0, 0, &srcLoc, nullptr);

				runtimeData.HandleViewableBuffer(*this, (node.name + ".Dest (After)").c_str(), destRT.m_resource, destRT.m_format, destRT.m_formatCount, destRT.m_structIndex, destRT.m_size, destRT.m_stride, destRT.m_count, false, true);
			}
		}
	}

	return true;
}
