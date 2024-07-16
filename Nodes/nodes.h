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

#include "resource.h"
#include "resource_buffer.h"
#include "resource_shaderconstants.h"
#include "resource_texture.h"

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

// A An interface that doesn't rely on node indices etc being calculated yet.
// A bit redundant. Need to rethink how nodes are interacted with by the editor, compiler, viewer, etc.
namespace FrontEndNodesNoCaching
{
    struct PinInfo
    {
        std::string srcPin;  // The name of this pin
        std::string* dstNode = nullptr; // The name of the node plugged into this pin
        std::string* dstPin = nullptr;  // The name of the pin on the node plugged into this pin
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

    // gets information about all pins for a specific node
    inline std::vector<PinInfo> GetPinInfo(RenderGraphNode& nodeBase)
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
                ret.push_back(info);

                info.srcPin = "dest";
                info.dstNode = &node.dest.node;
                info.dstPin = &node.dest.pin;
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
                    ret.push_back(info);
                }

                PinInfo info;
                info.srcPin = "shadingRateImage";
                info.dstNode = &node.shadingRateImage.node;
                info.dstPin = &node.shadingRateImage.pin;
                ret.push_back(info);

                info.srcPin = "vertexBuffer";
                info.dstNode = &node.vertexBuffer.node;
                info.dstPin = &node.vertexBuffer.pin;
                ret.push_back(info);

                info.srcPin = "indexBuffer";
                info.dstNode = &node.indexBuffer.node;
                info.dstPin = &node.indexBuffer.pin;
                ret.push_back(info);

                info.srcPin = "instanceBuffer";
                info.dstNode = &node.instanceBuffer.node;
                info.dstPin = &node.instanceBuffer.pin;
                ret.push_back(info);

                info.srcPin = "depthTarget";
                info.dstNode = &node.depthTarget.node;
                info.dstPin = &node.depthTarget.pin;
                ret.push_back(info);

                for (int i = 0; i < node.colorTargets.size(); ++i)
                {
                    char pinName[256];
                    sprintf_s(pinName, "colorTarget%i", i);
                    info.srcPin = pinName;
                    info.dstNode = &node.colorTargets[i].node;
                    info.dstPin = &node.colorTargets[i].pin;
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
            auto pinInfo = GetPinInfo(renderGraph.nodes[ret]);
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
