///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include <string>
#include "Schemas/Types.h"
#include "GigiAssert.h"
// clang-format on

struct InputNodeInfo
{
    ShaderResourceAccessType access;
    int nodeIndex;
    int pinIndex;
    bool required = true;
};

namespace FrontEndNodes
{
    // common interface
    inline std::string GetName(const RenderGraphNode_Base& node)
    {
        return node.name;
    }
    inline bool GetIsResourceNode(const RenderGraphNode_Base& node)
    {
        Assert(false, "Should not be called");
        return false;
    }

    // pin interface
    inline int GetPinCount(const RenderGraphNode_Base& node)
    {
        return 0;
    }
    inline std::string GetPinName(const RenderGraphNode_Base& node, int pinIndex)
    {
        Assert(false, "Should not be called");
        return "";
    }
    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Base& node, int pinIndex)
    {
        Assert(false, "Should not be called");
        return InputNodeInfo();
    }

    // resource specific
    inline void SetStartingState(RenderGraphNode_Base& node, ShaderResourceAccessType state)
    {
        Assert(false, "Should not be called");
    }
    inline void SetFinalState(RenderGraphNode_Base& node, ShaderResourceAccessType state)
    {
        Assert(false, "Should not be called");
    }
    inline ShaderResourceAccessType GetStartingState(const RenderGraphNode_Base& node)
    {
        Assert(false, "Should not be called");
        return ShaderResourceAccessType::Count;
    }
    inline ShaderResourceAccessType GetFinalState(const RenderGraphNode_Base& node)
    {
        Assert(false, "Should not be called");
        return ShaderResourceAccessType::Count;
    }
    inline ResourceVisibility GetResourceVisibility(const RenderGraphNode_Base& node)
    {
        Assert(false, "Should not be called");
        return ResourceVisibility::Count;
    }
    inline void AddResourceAccessedAs(RenderGraphNode_Base& node, ShaderResourceAccessType state)
    {
        Assert(false, "Should not be called");
    }

    // action specific
    inline void AddResourceDependency(RenderGraphNode_Base& node, int pinIndex, int resourceNodeIndex, ShaderResourceType type, ShaderResourceAccessType accessedAs)
    {
        Assert(false, "Should not be called");
    }
};

#include "action.h"
#include "action_computeshader.h"
#include "action_copyresource.h"
#include "action_rayshader.h"
#include "action_drawcall.h"
#include "action_subgraph.h"
#include "action_barrier.h"
#include "reroute.h"

#include "resource.h"
#include "resource_buffer.h"
#include "resource_shaderconstants.h"
#include "resource_texture.h"

inline int GetHitGroupIndex(const RenderGraph& renderGraph, const char* name)
{
    int ret = 0;
    for (const RTHitGroup& hitGroup : renderGraph.hitGroups)
    {
        if (!_stricmp(hitGroup.name.c_str(), name))
            return ret;
        ret++;
    }
    return -1;
}

inline int GetShaderIndex(const RenderGraph& renderGraph, const char* name)
{
    int ret = 0;
    for (const Shader& shader : renderGraph.shaders)
    {
        if (!_stricmp(shader.name.c_str(), name))
            return ret;
        ret++;
    }
    return -1;
}

template <typename LAMBDA>
void ExecuteOnNode(RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: lambda(node.##_NAME); break;
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

template <typename LAMBDA>
void ExecuteOnNode(const RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: lambda(node.##_NAME); break;
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

// Runs the lambda on the node if it's a resource node. Else a no-op
template <typename LAMBDA>
void ExecuteOnResourceNode(const RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        case RenderGraphNode::c_index_resourceBuffer:
        {
            lambda(node.resourceBuffer);
            break;
        }
        case RenderGraphNode::c_index_resourceTexture:
        {
            lambda(node.resourceTexture);
            break;
        }
        case RenderGraphNode::c_index_resourceShaderConstants:
        {
            lambda(node.resourceShaderConstants);
            break;
        }
        default:
        {
            bool isResourceNode;
            ExecuteOnNode(node,
                [&isResourceNode](auto& node)
                {
                    isResourceNode = FrontEndNodes::GetIsResourceNode(node);
                }
            );

            Assert(!isResourceNode, "Unhandled resource node type in " __FUNCTION__);
        }
    }
}

// Runs the lambda on the node if it's an action node. Else a no-op
template <typename LAMBDA>
void ExecuteOnActionNode(const RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        case RenderGraphNode::c_index_actionComputeShader:
        {
            lambda(node.actionComputeShader);
            break;
        }
        case RenderGraphNode::c_index_actionRayShader:
        {
            lambda(node.actionRayShader);
            break;
        }
        case RenderGraphNode::c_index_actionCopyResource:
        {
            lambda(node.actionCopyResource);
            break;
        }
        case RenderGraphNode::c_index_actionDrawCall:
        {
            lambda(node.actionDrawCall);
            break;
        }
        case RenderGraphNode::c_index_actionSubGraph:
        {
            lambda(node.actionSubGraph);
            break;
        }
        case RenderGraphNode::c_index_actionBarrier:
        {
            lambda(node.actionBarrier);
            break;
        }
        default:
        {
            bool isResourceNode;
            ExecuteOnNode(node,
                [&isResourceNode](auto& node)
                {
                    isResourceNode = FrontEndNodes::GetIsResourceNode(node);
                }
            );

            Assert(isResourceNode, "Unhandled action node type in " __FUNCTION__);
        }
    }
}

inline int GetNodeIndex(const RenderGraphNode& node)
{
    int ret = 0;
    ExecuteOnNode(node, [&ret](auto& node) { ret = node.nodeIndex; });
    return ret;
}

inline int GetNodePinCount(const RenderGraphNode& node)
{
    int ret = 0;
    ExecuteOnNode(node,
        [&ret] (auto& node)
        {
            ret = FrontEndNodes::GetPinCount(node);
        }
    );
    return ret;
}

inline std::string GetNodePinName(const RenderGraphNode& node, int pinIndex)
{
    std::string ret;
    ExecuteOnNode(node,
        [&ret, pinIndex](auto& node)
        {
            ret = FrontEndNodes::GetPinName(node, pinIndex);
        }
    );
    return ret;
}

inline InputNodeInfo GetNodePinInputNodeInfo(const RenderGraphNode& node, int pinIndex)
{
    InputNodeInfo ret;
    ExecuteOnNode(node,
        [&ret, pinIndex](auto& node)
        {
            ret = FrontEndNodes::GetPinInputNodeInfo(node, pinIndex);
        }
    );
    return ret;
}

inline ShaderResourceAccessType GetResourceNodeStartingState(const RenderGraphNode& node)
{
    ShaderResourceAccessType ret;
    ExecuteOnNode(node,
        [&ret](auto& node)
        {
            ret = FrontEndNodes::GetStartingState(node);
        }
    );
    return ret;
}

inline void SetResourceNodeStartingState(RenderGraphNode& node, ShaderResourceAccessType state)
{
    ExecuteOnNode(node,
        [state](auto& node)
        {
            FrontEndNodes::SetStartingState(node, state);
        }
    );
}

inline ShaderResourceAccessType GetResourceNodeFinalState(const RenderGraphNode& node)
{
    ShaderResourceAccessType ret;
    ExecuteOnNode(node,
        [&ret](auto& node)
        {
            ret = FrontEndNodes::GetFinalState(node);
        }
    );
    return ret;
}

inline void SetResourceNodeFinalState(RenderGraphNode& node, ShaderResourceAccessType state)
{
    ExecuteOnNode(node,
        [state](auto& node)
        {
            FrontEndNodes::SetFinalState(node, state);
        }
    );
}

inline ResourceVisibility GetNodeResourceVisibility(const RenderGraphNode& node)
{
    ResourceVisibility ret;
    ExecuteOnNode(node,
        [&ret](auto& node)
        {
            ret = FrontEndNodes::GetResourceVisibility(node);
        }
    );
    return ret;
}

inline bool GetNodeIsResourceNode(const RenderGraphNode& node)
{
    bool ret;
    ExecuteOnNode(node,
        [&ret](auto& node)
        {
            ret = FrontEndNodes::GetIsResourceNode(node);
        }
    );
    return ret;
}

inline unsigned int GetNodeBackendFlags(const RenderGraphNode& node)
{
    unsigned int ret;
    ExecuteOnNode(node,
        [&ret](auto& node)
        {
            ret = node.backends.backendFlags;
        }
    );
    return ret;
}

inline std::string GetNodeName(const RenderGraphNode& node)
{
    std::string ret;
    ExecuteOnNode(node,
        [&ret](auto& node)
        {
            ret = FrontEndNodes::GetName(node);
        }
    );
    return ret;
}

inline std::string GetNodeName(const RenderGraph& renderGraph, int nodeIndex)
{
    return GetNodeName(renderGraph.nodes[nodeIndex]);
}

inline std::string GetNodeOriginalName(const RenderGraphNode& node)
{
    std::string ret;
    ExecuteOnNode(node,
        [&ret](auto& node)
        {
            ret = node.originalName;
        }
    );
    return ret;
}

inline void AddResourceNodeAccessedAs(RenderGraphNode& node, ShaderResourceAccessType access)
{
    ExecuteOnNode(node,
        [access](auto& node)
        {
            FrontEndNodes::AddResourceAccessedAs(node, access);
        }
    );
}

inline void AddResourceDependency(RenderGraphNode& node, int pinIndex, int resourceNodeIndex, ShaderResourceType type, ShaderResourceAccessType access)
{
    ExecuteOnNode(node,
        [pinIndex, resourceNodeIndex, type, access](auto& node)
        {
            FrontEndNodes::AddResourceDependency(node, pinIndex, resourceNodeIndex, type, access);
        }
    );
}

inline bool ShaderResourceTypeIsReadOnly(ShaderResourceAccessType access)
{
    switch (access)
    {
        case ShaderResourceAccessType::UAV: return false;
        case ShaderResourceAccessType::RTScene: return true;
        case ShaderResourceAccessType::SRV: return true;
        case ShaderResourceAccessType::CopySource: return true;
        case ShaderResourceAccessType::CopyDest: return false;
        case ShaderResourceAccessType::CBV: return true;
        case ShaderResourceAccessType::Indirect: return true;
        case ShaderResourceAccessType::VertexBuffer: return true;
        case ShaderResourceAccessType::RenderTarget: return false;
        case ShaderResourceAccessType::DepthTarget: return false;
        case ShaderResourceAccessType::Barrier: return false;
        case ShaderResourceAccessType::ShadingRate: return true;
    }

    Assert(false, "Unhandled ShaderResourceType: %i", access);
    return false;
}

inline bool AccessIsReadOnly(unsigned int accessedAs)
{
    for (unsigned int i = 0; i < (unsigned int)ShaderResourceAccessType::Count; ++i)
    {
        if ((accessedAs & (1 << i)) == 0)
            continue;

        if (!ShaderResourceTypeIsReadOnly((ShaderResourceAccessType)i))
            return false;
    }

    return true;
}

// A An interface that doesn't rely on node indices etc being calculated yet.
// A bit redundant. Need to rethink how nodes are interacted with by the editor, compiler, viewer, etc.
// EditorNodes.h GetNodePins() does a lot of the same work, it'd be nice to combine the core functionality into here since this is shared more widely than just the editor.
namespace FrontEndNodesNoCaching
{
    struct PinInfo
    {
        std::string srcPin;  // The name of this pin
        std::string* dstNode = nullptr; // The name of the node plugged into this pin
        std::string* dstPin = nullptr;  // The name of the pin on the node plugged into this pin

        bool outputOnly = false;
        bool readOnly = true;
    };

    // Returns -1 if it couldn't find it
    inline int GetNodeIndexByName(const RenderGraph& renderGraph, const char* nodeName)
    {
        for (size_t i = 0; i < renderGraph.nodes.size(); ++i)
        {
            if (!_stricmp(GetNodeName(renderGraph.nodes[i]).c_str(), nodeName))
                return (int)i;
        }
        return -1;
    }

    // Returns -1 if it couldn't find it
    inline int GetShaderIndexByName(const RenderGraph& renderGraph, ShaderType shaderType, const char* name)
    {
        // Get the shader the shader reference
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (shaderType != ShaderType::Count && renderGraph.shaders[index].type != shaderType)
                continue;

            if (!_stricmp(renderGraph.shaders[index].name.c_str(), name))
                return index;
        }
        return -1;
    }

    // returns nullptr if it couldn't find it
    inline const ShaderResource* GetShaderResourceByName(const RenderGraph& renderGraph, ShaderType shaderType, const char* shaderName, const char* resourceName)
    {
        int shaderIndex = GetShaderIndexByName(renderGraph, shaderType, shaderName);
        if (shaderIndex == -1)
            return nullptr;

        for (const ShaderResource& shaderResource : renderGraph.shaders[shaderIndex].resources)
        {
            if (shaderResource.name == resourceName)
                return &shaderResource;
        }

        return nullptr;
    }

    // Returns -1 if it couldn't find it
    inline int GetNodeIndex(const RenderGraph& renderGraph, const RenderGraphNode& node)
    {
        return GetNodeIndexByName(renderGraph, GetNodeName(node).c_str());
    }

    // gets information about all pins for a specific node
    inline std::vector<PinInfo> GetPinInfo(const RenderGraph& renderGraph, RenderGraphNode& nodeBase)
    {
        std::vector<PinInfo> ret;
        switch (nodeBase._index)
        {
            case RenderGraphNode::c_index_resourceBuffer:
            case RenderGraphNode::c_index_resourceShaderConstants:
            case RenderGraphNode::c_index_resourceTexture:
            {
                PinInfo info;
                info.srcPin = "resource";
                info.outputOnly = true;
                info.readOnly = true;
                ret.push_back(info);
                break;
            }
            case RenderGraphNode::c_index_actionComputeShader:
            {
                RenderGraphNode_Action_ComputeShader& node = nodeBase.actionComputeShader;
                for (NodePinConnection& connection : node.connections)
                {
                    PinInfo info;
                    info.srcPin = connection.srcPin;
                    info.dstNode = &connection.dstNode;
                    info.dstPin = &connection.dstPin;

                    const ShaderResource* shaderResource = GetShaderResourceByName(renderGraph, ShaderType::Compute, node.shader.name.c_str(), connection.srcPin.c_str());
                    Assert(shaderResource != nullptr, "Could not find shader resource \"%s\" in shader \"%s\" in " __FUNCTION__, connection.srcPin.c_str(), node.shader.name.c_str());
                    if (shaderResource)
                        info.readOnly = ShaderResourceTypeIsReadOnly(shaderResource->access);

                    ret.push_back(info);
                }
                break;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                RenderGraphNode_Action_RayShader& node = nodeBase.actionRayShader;
                for (NodePinConnection& connection : node.connections)
                {
                    PinInfo info;
                    info.srcPin = connection.srcPin;
                    info.dstNode = &connection.dstNode;
                    info.dstPin = &connection.dstPin;

                    const ShaderResource* shaderResource = GetShaderResourceByName(renderGraph, ShaderType::RTRayGen, node.shader.name.c_str(), connection.srcPin.c_str());
                    Assert(shaderResource != nullptr, "Could not find shader resource \"%s\" in shader \"%s\" in " __FUNCTION__, connection.srcPin.c_str(), node.shader.name.c_str());
                    if (shaderResource)
                        info.readOnly = ShaderResourceTypeIsReadOnly(shaderResource->access);

                    ret.push_back(info);
                }
                break;
            }
            case RenderGraphNode::c_index_actionCopyResource:
            {
                RenderGraphNode_Action_CopyResource& node = nodeBase.actionCopyResource;

                PinInfo info;
                info.srcPin = "source";
                info.dstNode = &node.source.node;
                info.dstPin = &node.source.pin;
                info.readOnly = true;
                ret.push_back(info);

                info.srcPin = "dest";
                info.dstNode = &node.dest.node;
                info.dstPin = &node.dest.pin;
                info.readOnly = false;
                ret.push_back(info);

                break;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                RenderGraphNode_Action_DrawCall& node = nodeBase.actionDrawCall;
                for (NodePinConnection& connection : node.connections)
                {
                    PinInfo info;
                    info.srcPin = connection.srcPin;
                    info.dstNode = &connection.dstNode;
                    info.dstPin = &connection.dstPin;

                    const ShaderResource* shaderResourceAS = GetShaderResourceByName(renderGraph, ShaderType::Amplification, node.amplificationShader.name.c_str(), connection.srcPin.c_str());
                    const ShaderResource* shaderResourceMS = GetShaderResourceByName(renderGraph, ShaderType::Mesh, node.meshShader.name.c_str(), connection.srcPin.c_str());
                    const ShaderResource* shaderResourceVS = GetShaderResourceByName(renderGraph, ShaderType::Vertex, node.vertexShader.name.c_str(), connection.srcPin.c_str());
                    const ShaderResource* shaderResourcePS = GetShaderResourceByName(renderGraph, ShaderType::Pixel, node.pixelShader.name.c_str(), connection.srcPin.c_str());

                    Assert(shaderResourceAS != nullptr || shaderResourceMS != nullptr || shaderResourceVS != nullptr || shaderResourcePS != nullptr, "Could not find shader resource \"%s\" for draw call node \"%s\" " __FUNCTION__, connection.srcPin.c_str(), node.name.c_str());

                    if (shaderResourceAS)
                        info.readOnly = ShaderResourceTypeIsReadOnly(shaderResourceAS->access);
                    else if (shaderResourceMS)
                        info.readOnly = ShaderResourceTypeIsReadOnly(shaderResourceMS->access);
                    else if (shaderResourceVS)
                        info.readOnly = ShaderResourceTypeIsReadOnly(shaderResourceVS->access);
                    else if (shaderResourcePS)
                        info.readOnly = ShaderResourceTypeIsReadOnly(shaderResourcePS->access);

                    ret.push_back(info);
                }

                PinInfo info;
                info.srcPin = "shadingRateImage";
                info.dstNode = &node.shadingRateImage.node;
                info.dstPin = &node.shadingRateImage.pin;
                info.readOnly = true;
                ret.push_back(info);

                info.srcPin = "vertexBuffer";
                info.dstNode = &node.vertexBuffer.node;
                info.dstPin = &node.vertexBuffer.pin;
                info.readOnly = true;
                ret.push_back(info);

                info.srcPin = "indexBuffer";
                info.dstNode = &node.indexBuffer.node;
                info.dstPin = &node.indexBuffer.pin;
                info.readOnly = true;
                ret.push_back(info);

                info.srcPin = "instanceBuffer";
                info.dstNode = &node.instanceBuffer.node;
                info.dstPin = &node.instanceBuffer.pin;
                info.readOnly = true;
                ret.push_back(info);

                info.srcPin = "depthTarget";
                info.dstNode = &node.depthTarget.node;
                info.dstPin = &node.depthTarget.pin;
                info.readOnly = !node.depthWrite;
                ret.push_back(info);

                for (int i = 0; i < node.colorTargets.size(); ++i)
                {
                    char pinName[256];
                    sprintf_s(pinName, "colorTarget%i", i);
                    info.srcPin = pinName;
                    info.dstNode = &node.colorTargets[i].node;
                    info.dstPin = &node.colorTargets[i].pin;
                    info.readOnly = false;
                    ret.push_back(info);
                }
                break;
            }
            case RenderGraphNode::c_index_actionSubGraph:
            {
                RenderGraphNode_Action_SubGraph& node = nodeBase.actionSubGraph;
                for (NodePinConnection& connection : node.connections)
                {
                    PinInfo info;
                    info.srcPin = connection.srcPin;
                    info.dstNode = &connection.dstNode;
                    info.dstPin = &connection.dstPin;
                    // Assume it could be written. This won't come up if doing work after inlining subgraphs, which happens early in compilation.
                    info.readOnly = false;
                    ret.push_back(info);
                }
                break;
            }
            case RenderGraphNode::c_index_actionBarrier:
            {
                RenderGraphNode_Action_Barrier& node = nodeBase.actionBarrier;
                for (NodePinConnection& connection : node.connections)
                {
                    PinInfo info;
                    info.srcPin = connection.srcPin;
                    info.dstNode = &connection.dstNode;
                    info.dstPin = &connection.dstPin;
                    info.readOnly = true;
                    ret.push_back(info);
                }
                break;
            }
			case RenderGraphNode::c_index_reroute:
			{
				RenderGraphNode_Reroute& node = nodeBase.reroute;
				for (NodePinConnection& connection : node.connections)
				{
					PinInfo info;
					info.srcPin = connection.srcPin;
					info.dstNode = &connection.dstNode;
					info.dstPin = &connection.dstPin;
					ret.push_back(info);
				}
				break;
			}
            default:
            {
                Assert(false, "Unhandled node type in " __FUNCTION__);
                break;
            }
        }

        return ret;
    }

    inline bool GetConnectingNode(RenderGraph& renderGraph, RenderGraphNode& sourceNode, int sourcePinIndex, int& connectedNodeIndex, int& connectedPinIndex)
    {
        std::string sourceNodeName = GetNodeName(sourceNode);
        std::string sourcePinName = GetPinInfo(renderGraph, sourceNode)[sourcePinIndex].srcPin;

        for (int nodeIndex = 0; nodeIndex < (int)renderGraph.nodes.size(); ++nodeIndex)
        {
            std::vector<PinInfo> pinInfos = GetPinInfo(renderGraph, renderGraph.nodes[nodeIndex]);

            for (int pinIndex = 0; pinIndex < (int)pinInfos.size(); ++pinIndex)
            {
                const PinInfo& pinInfo = pinInfos[pinIndex];
                if (pinInfo.dstNode != nullptr && pinInfo.dstPin != nullptr && *pinInfo.dstNode == sourceNodeName && *pinInfo.dstPin == sourcePinName)
                {
                    connectedNodeIndex = nodeIndex;
                    connectedPinIndex = pinIndex;
                    return true;
                }
            }
        }

        return false;
    }

    // Returns true if this node pin, or any node reachable from this node pin has write access to the resource carried on the connection line
    inline bool DoesSubtreeWriteResource(RenderGraph& renderGraph, RenderGraphNode& node, int pinIndex)
    {
        struct LinkToInvestigate
        {
            RenderGraphNode* node = nullptr;
            int pinIndex = 1;
        };
        std::vector<LinkToInvestigate> linksToInvestigate;

        // Check the node itself
        std::vector<PinInfo> pinInfos = GetPinInfo(renderGraph, node);
        if (!pinInfos[pinIndex].readOnly)
            return true;

        linksToInvestigate.push_back(LinkToInvestigate{&node, pinIndex});

        while (!linksToInvestigate.empty())
        {
            LinkToInvestigate link = *linksToInvestigate.rbegin();
            linksToInvestigate.pop_back();

            int connectedNodeIndex = -1;
            int connectedPinIndex = -1;

            if (GetConnectingNode(renderGraph, *link.node, link.pinIndex, connectedNodeIndex, connectedPinIndex))
            {
                RenderGraphNode& connectedNode = renderGraph.nodes[connectedNodeIndex];
                std::vector<PinInfo> pinInfos = GetPinInfo(renderGraph, connectedNode);
                PinInfo& pinInfo = pinInfos[connectedPinIndex];

                // If this pin is not read only, we have a write, return true!
                if (!pinInfo.readOnly)
                    return true;

                // Otherwise, add this link to investigate further
                linksToInvestigate.push_back(LinkToInvestigate{ &connectedNode, connectedPinIndex });
            }
        }

        // if we got here, nothing had write access.
        return false;
    }

    // Returns -1 if it couldn't find it
    inline int GetPinIndexByName(const std::vector<PinInfo>& pinInfos, const char* pinName)
    {
        for (size_t i = 0; i < pinInfos.size(); ++i)
        {
            if (!_stricmp(pinInfos[i].srcPin.c_str(), pinName))
                return (int)i;
        }
        return -1;
    }

    // Returns -1 if it couldn't find it
    inline int GetPinIndexByName(const RenderGraph& renderGraph, RenderGraphNode& node, const char* pinName)
    {
        std::vector<PinInfo> pinInfos = GetPinInfo(renderGraph, node);
        return GetPinIndexByName(pinInfos, pinName);
    }

    // Returns the last node found by starting at this node and pin and walking left through the graph along connections.
    // Returns -1 if there are problems.
    inline int GetRootNodeIndex(RenderGraph& renderGraph, const std::string& node, const std::string& pin)
    {
        int ret = -1;

        const std::string* searchNode = &node;
        const std::string* searchPin = &pin;

        while (searchNode && searchPin && !searchNode->empty() && !searchPin->empty())
        {
            ret = GetNodeIndexByName(renderGraph, searchNode->c_str());
            if (ret < 0)
                return ret;

            bool foundPin = false;
            auto pinInfo = GetPinInfo(renderGraph, renderGraph.nodes[ret]);
            for (auto& pin : pinInfo)
            {
                if (pin.srcPin == *searchPin)
                {
                    searchNode = pin.dstNode;
                    searchPin = pin.dstPin;
                    foundPin = true;
                    break;
                }
            }

            if (!foundPin)
                return -1;
        }

        return ret;
    }
};
