///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

/*
This file has redundant functionality that also exists in the Gigi project.

The reason for that is because Gigi does some fixup / changes to the render graph at load, both for convinience
and also to generate the right output (like adding structs for automatically created constant buffers).

The editor doesn't want any of that, and just wants to edit the data as authored, so unfortunately that
means some code redundancy.
*/

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
        case ShaderResourceAccessType::IndexBuffer: return true;
        case ShaderResourceAccessType::RenderTarget: return false;
        case ShaderResourceAccessType::DepthTarget: return false;
        case ShaderResourceAccessType::Barrier: return false;
        case ShaderResourceAccessType::ShadingRate: return true;
    }

    //Assert(false, "Unhandled ShaderResourceType: %i", access);
    return true;
}

template <typename LAMBDA>
void DispatchLambdaActionInner(RenderGraphNode_ActionBase& node, const LAMBDA& lambda)
{
    lambda(node);
}

template <typename LAMBDA>
void DispatchLambdaActionInner(const RenderGraphNode_ActionBase& node, const LAMBDA& lambda)
{
    lambda(node);
}

template <typename LAMBDA>
void DispatchLambdaActionInner(RenderGraphNode_ResourceBase& node, const LAMBDA& lambda)
{
}

template <typename LAMBDA>
void DispatchLambdaActionInner(const RenderGraphNode_ResourceBase& node, const LAMBDA& lambda)
{
}

template <typename LAMBDA>
void DispatchLambdaAction(RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                DispatchLambdaActionInner(node.##_NAME, lambda); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

template <typename LAMBDA>
void DispatchLambdaAction(const RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                DispatchLambdaAction(node.##_NAME, lambda); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

template <typename LAMBDA>
void DispatchLambda(RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                lambda(node.##_NAME); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

template <typename LAMBDA>
void DispatchLambda(const RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                lambda(node.##_NAME); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

inline std::string GetNodeTypeName(const RenderGraphNode& node)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)  case RenderGraphNode::c_index_##_NAME: return #_NAME;
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
    Assert(false, "Unhandled node type");
    return "";
}

inline std::string GetNodeShortTypeName(const RenderGraphNode& node)
{
    std::string ret;
    DispatchLambda(node,
        [&ret](auto& node)
        {
            ret = node.c_shortTypeName;
        }
    );
    return ret;
}

inline std::string GetNodeShorterTypeName(const RenderGraphNode& node)
{
    std::string ret;
    DispatchLambda(node,
        [&ret](auto& node)
        {
            ret = node.c_shorterTypeName;
        }
    );
    return ret;
}

inline std::string GetNodeName(const RenderGraphNode& node)
{
    std::string ret;
    DispatchLambda(node,
        [&ret](auto& node)
        {
            ret = node.name;
        }
    );
    return ret;
}

inline void SetNodeName(RenderGraphNode& node, const std::string& name)
{
    DispatchLambda(node,
        [&](auto& node)
        {
            node.name = name;
        }
    );
}

inline bool GetResourceVisibility(const RenderGraphNode& node, ResourceVisibility& visibility)
{
    switch (node._index)
    {
        case RenderGraphNode::c_index_resourceBuffer:
        {
            visibility = node.resourceBuffer.visibility;
            return true;
        }
        case RenderGraphNode::c_index_resourceTexture:
        {
            visibility = node.resourceTexture.visibility;
            return true;
        }
    }

    return false;
}

inline bool GetNodeIsResourceNode(const RenderGraphNode& node)
{
    bool ret;
    DispatchLambda(node,
        [&ret](auto& node)
        {
            ret = node.c_isResourceNode;
        }
    );
    return ret;
}

inline ImColor GetNodeColor(const RenderGraphNode& node)
{
    if (node._index == RenderGraphNode::c_index_reroute)
    {
        return ImColor(255, 255, 255, 200);
    }

	// colors like Frostbite FrameGraph
    // slide 17 https://www.slideshare.net/DICEStudio/framegraph-extensible-rendering-architecture-in-frostbite
    bool isResourceNode = GetNodeIsResourceNode(node);
    return isResourceNode ? ImColor(128, 128, 255, 128) : ImColor(255, 128, 64, 128);
}

inline std::array<float, 2> GetNodeEditorPos(const RenderGraphNode& node)
{
    std::array<float, 2> ret;
    DispatchLambda(node,
        [&ret](auto& node)
        {
            ret = node.editorPos;
        }
    );
    return ret;
}

inline void SetNodeEditorPos(RenderGraphNode& node, const std::array<float, 2>& pos)
{
    DispatchLambda(node,
        [&](auto& node)
        {
            node.editorPos = pos;
        }
    );
}

inline int GetNodeIndexByName(const RenderGraph& renderGraph, const char* name)
{
    for (int i = 0; i < (int)renderGraph.nodes.size(); ++i)
    {
        if (!_stricmp(name, GetNodeName(renderGraph.nodes[i]).c_str()))
            return i;
    }
    return -1;
}

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

struct NodePinInfo
{
    std::string name;
    bool isInput = true;
    std::string accessLabel;

    int* inputEditorId = nullptr;
    int* outputEditorId = nullptr;

    std::string* inputNode = nullptr;
    std::string* inputNodePin = nullptr;

    LinkProperties* linkProperties = nullptr;
};

inline int GetNodePinIndexByName(const std::vector<NodePinInfo>& nodePinInfo, const char* name)
{
    for (int i = 0; i < nodePinInfo.size(); ++i)
    {
        if (nodePinInfo[i].name == name)
            return i;
    }
    return -1;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Resource_Buffer& node)
{
    std::vector<NodePinInfo> ret;

    NodePinInfo pin;
    pin.name = "resource";
    pin.isInput = false;
    ret.push_back(pin);

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Resource_Texture& node)
{
    std::vector<NodePinInfo> ret;

    NodePinInfo pin;
    pin.name = "resource";
    pin.isInput = false;
    ret.push_back(pin);

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Resource_ShaderConstants& node)
{
    std::vector<NodePinInfo> ret;

    NodePinInfo pin;
    pin.name = "resource";
    pin.isInput = false;
    ret.push_back(pin);

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Action_ComputeShader& node)
{
    std::vector<NodePinInfo> ret;

    // Get the shader the shader reference
    int shaderIndex = GetShaderIndexByName(renderGraph, ShaderType::Compute, node.shader.name.c_str());

    // It's ok if the shader wasn't found. it means the user isn't done editing
    if (shaderIndex != -1)
    {
        // make sure there is a connection on the node for each resource
        const Shader& shader = renderGraph.shaders[shaderIndex];
        for (const ShaderResource& resource : shader.resources)
        {
            bool found = false;
            for (NodePinConnection& connection : node.connections)
            {
                if (connection.srcPin == resource.name)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                node.connections.resize(node.connections.size() + 1);
                node.connections.rbegin()->srcPin = resource.name;
            }
        }

        // make a pin for each resource
        for (const ShaderResource& resource : shader.resources)
        {
            NodePinInfo pin;
            pin.name = resource.name;

            for (NodePinConnection& connection : node.connections)
            {
                if (connection.srcPin == resource.name)
                {
                    pin.inputNode = &connection.dstNode;
                    pin.inputNodePin = &connection.dstPin;
                    pin.accessLabel = ShaderResourceTypeIsReadOnly(resource.access) ? " (R)" : " (RW)";
                    break;
                }
            }

            ret.push_back(pin);
        }
    }

    // make a pin for indirect dispatch
    NodePinInfo pin;
    pin.name = "indirectBuffer";
    pin.inputNode = &node.dispatchSize.indirectBuffer.node;
    pin.inputNodePin = &node.dispatchSize.indirectBuffer.pin;
    pin.accessLabel = " (R)";
    ret.push_back(pin);

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Action_RayShader& node)
{
    std::vector<NodePinInfo> ret;

    // Get the shader the shader reference
    int shaderIndex = GetShaderIndexByName(renderGraph, ShaderType::RTRayGen, node.shader.name.c_str());

    // It's ok if the shader wasn't found. it means the user isn't done editing
    if (shaderIndex != -1)
    {
        // make sure there is a connection on the node for each resource
        const Shader& shader = renderGraph.shaders[shaderIndex];
        for (const ShaderResource& resource : shader.resources)
        {
            bool found = false;
            for (NodePinConnection& connection : node.connections)
            {
                if (connection.srcPin == resource.name)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                node.connections.resize(node.connections.size() + 1);
                node.connections.rbegin()->srcPin = resource.name;
            }
        }

        // make a pin for each resource
        for (const ShaderResource& resource : shader.resources)
        {
            NodePinInfo pin;
            pin.name = resource.name;

            for (NodePinConnection& connection : node.connections)
            {
                if (connection.srcPin == resource.name)
                {
                    pin.inputNode = &connection.dstNode;
                    pin.inputNodePin = &connection.dstPin;
                    pin.accessLabel = ShaderResourceTypeIsReadOnly(resource.access) ? " (R)" : " (RW)";
                    break;
                }
            }

            ret.push_back(pin);
        }
    }

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Action_CopyResource& node)
{
    std::vector<NodePinInfo> ret;

    NodePinInfo pin;
    pin.name = "source";
    pin.inputNode = &node.source.node;
    pin.inputNodePin = &node.source.pin;
    pin.accessLabel = " (R)";
    ret.push_back(pin);

    pin.name = "dest";
    pin.inputNode = &node.dest.node;
    pin.inputNodePin = &node.dest.pin;
    pin.accessLabel = " (RW)";
    ret.push_back(pin);

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Action_DrawCall& node)
{
    std::vector<NodePinInfo> ret;

    // Vertex Shader Pins
    {
        // Get the shader the shader reference
        int shaderIndex = GetShaderIndexByName(renderGraph, ShaderType::Vertex, node.vertexShader.name.c_str());

        // It's ok if the shader wasn't found. it means the user isn't done editing
        if (shaderIndex != -1)
        {
            // make sure there is a connection on the node for each resource
            const Shader& shader = renderGraph.shaders[shaderIndex];
            for (const ShaderResource& resource : shader.resources)
            {
                bool found = false;
                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    node.connections.resize(node.connections.size() + 1);
                    node.connections.rbegin()->srcPin = resource.name;
                }
            }

            // make a pin for each resource
            for (const ShaderResource& resource : shader.resources)
            {
                NodePinInfo pin;
                pin.name = resource.name;

                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        pin.inputNode = &connection.dstNode;
                        pin.inputNodePin = &connection.dstPin;
                        pin.accessLabel = ShaderResourceTypeIsReadOnly(resource.access) ? " (R)" : " (RW)";
                        break;
                    }
                }

                ret.push_back(pin);
            }
        }
    }

    // Pixel Shader Pins
    {
        // Get the shader the shader reference
        int shaderIndex = GetShaderIndexByName(renderGraph, ShaderType::Pixel, node.pixelShader.name.c_str());

        // It's ok if the shader wasn't found. it means the user isn't done editing
        if (shaderIndex != -1)
        {
            // make sure there is a connection on the node for each resource
            const Shader& shader = renderGraph.shaders[shaderIndex];
            for (const ShaderResource& resource : shader.resources)
            {
                bool found = false;
                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    node.connections.resize(node.connections.size() + 1);
                    node.connections.rbegin()->srcPin = resource.name;
                }
            }

            // make a pin for each resource
            for (const ShaderResource& resource : shader.resources)
            {
                NodePinInfo pin;
                pin.name = resource.name;

                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        pin.inputNode = &connection.dstNode;
                        pin.inputNodePin = &connection.dstPin;
                        pin.accessLabel = ShaderResourceTypeIsReadOnly(resource.access) ? " (R)" : " (RW)";
                        break;
                    }
                }

                ret.push_back(pin);
            }
        }
    }

    // Amplification Shader Pins
    {
        // Get the shader the shader reference
        int shaderIndex = GetShaderIndexByName(renderGraph, ShaderType::Amplification, node.amplificationShader.name.c_str());

        // It's ok if the shader wasn't found. it means the user isn't done editing
        if (shaderIndex != -1)
        {
            // make sure there is a connection on the node for each resource
            const Shader& shader = renderGraph.shaders[shaderIndex];
            for (const ShaderResource& resource : shader.resources)
            {
                bool found = false;
                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    node.connections.resize(node.connections.size() + 1);
                    node.connections.rbegin()->srcPin = resource.name;
                }
            }

            // make a pin for each resource
            for (const ShaderResource& resource : shader.resources)
            {
                NodePinInfo pin;
                pin.name = resource.name;

                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        pin.inputNode = &connection.dstNode;
                        pin.inputNodePin = &connection.dstPin;
                        pin.accessLabel = ShaderResourceTypeIsReadOnly(resource.access) ? " (R)" : " (RW)";
                        break;
                    }
                }

                ret.push_back(pin);
            }
        }
    }

    // Mesh Shader Pins
    {
        // Get the shader the shader reference
        int shaderIndex = GetShaderIndexByName(renderGraph, ShaderType::Mesh, node.meshShader.name.c_str());

        // It's ok if the shader wasn't found. it means the user isn't done editing
        if (shaderIndex != -1)
        {
            // make sure there is a connection on the node for each resource
            const Shader& shader = renderGraph.shaders[shaderIndex];
            for (const ShaderResource& resource : shader.resources)
            {
                bool found = false;
                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    node.connections.resize(node.connections.size() + 1);
                    node.connections.rbegin()->srcPin = resource.name;
                }
            }

            // make a pin for each resource
            for (const ShaderResource& resource : shader.resources)
            {
                NodePinInfo pin;
                pin.name = resource.name;

                for (NodePinConnection& connection : node.connections)
                {
                    if (connection.srcPin == resource.name)
                    {
                        pin.inputNode = &connection.dstNode;
                        pin.inputNodePin = &connection.dstPin;
                        pin.accessLabel = ShaderResourceTypeIsReadOnly(resource.access) ? " (R)" : " (RW)";
                        break;
                    }
                }

                ret.push_back(pin);
            }
        }
    }

    // Shading Rate Image
    NodePinInfo pin;
    pin.name = "shadingRateImage";
    pin.inputNode = &node.shadingRateImage.node;
    pin.inputNodePin = &node.shadingRateImage.pin;
    pin.accessLabel = " (R)";
    ret.push_back(pin);

    // Vertex Buffer
    pin.name = "vertexBuffer";
    pin.inputNode = &node.vertexBuffer.node;
    pin.inputNodePin = &node.vertexBuffer.pin;
    pin.accessLabel = " (R)";
    ret.push_back(pin);

    // Index Buffer
    pin.name = "indexBuffer";
    pin.inputNode = &node.indexBuffer.node;
    pin.inputNodePin = &node.indexBuffer.pin;
    pin.accessLabel = " (R)";
    ret.push_back(pin);

    // Instance Buffer
    pin.name = "instanceBuffer";
    pin.inputNode = &node.instanceBuffer.node;
    pin.inputNodePin = &node.instanceBuffer.pin;
    pin.accessLabel = " (R)";
    ret.push_back(pin);

    // Depth Target
    pin.name = "depthTarget";
    pin.inputNode = &node.depthTarget.node;
    pin.inputNodePin = &node.depthTarget.pin;
    pin.accessLabel = " (RW)";
    ret.push_back(pin);

    // Color targets
    for (int i = 0; i < node.colorTargets.size(); ++i)
    {
        char buffer[256];
        sprintf_s(buffer, "colorTarget%i", i);
        pin.name = buffer;
        pin.inputNode = &node.colorTargets[i].node;
        pin.inputNodePin = &node.colorTargets[i].pin;
        pin.accessLabel = " (RW)";
        ret.push_back(pin);
        if (pin.inputNode->empty())
            break;
    }

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode& node_)
{
    // Get the pins
    std::vector<NodePinInfo> ret;
    DispatchLambda(node_, [&renderGraph, &ret](auto& node) { ret = GetNodePins(renderGraph, node); });

    // Make sure there is one link property per pin
    DispatchLambdaAction(node_,
        [&ret](RenderGraphNode_ActionBase& node)
        {
            node.linkProperties.resize(ret.size());

            for (size_t i = 0; i < ret.size(); ++i)
                ret[i].linkProperties = &node.linkProperties[i];
        }
    );

    // make sure there is an entry for each pin, before we get pointers, so a realloc doesn't invalidate the pointers
    DispatchLambda(node_,
        [&ret](RenderGraphNode_Base& node)
        {
            for (NodePinInfo& pinInfo : ret)
            {
                if (pinInfo.isInput)
                    node.inputPinIds.insert(std::make_pair( pinInfo.name, -1 ));
                node.outputPinIds.insert(std::make_pair( pinInfo.name, -1 ));
            }
        }
    );

    // Get the pointer of each pin's input and output editor id
    DispatchLambda(node_,
        [&ret](RenderGraphNode_Base& node)
        {
            for (NodePinInfo& pinInfo : ret)
            {
                if (pinInfo.isInput)
                    pinInfo.inputEditorId = &node.inputPinIds[pinInfo.name];
                pinInfo.outputEditorId = &node.outputPinIds[pinInfo.name];
            }
        }
    );

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Action_SubGraph& node)
{
    std::vector<NodePinInfo> ret;

    // Imported resources
    for (const std::string& name : node.subGraphData.importedResources)
    {
        int connectionIndex = -1;
        for (int i = 0; i < (int)node.connections.size(); ++i)
        {
            if (node.connections[i].srcPin != name)
                continue;
            connectionIndex = i;
            break;
        }
        Assert(connectionIndex >= 0, "GetNodePins in SubGraph could not find pin for imported resource!");

        NodePinInfo pin;
        pin.name = name;
        pin.inputNode = &node.connections[connectionIndex].dstNode;
        pin.inputNodePin = &node.connections[connectionIndex].dstPin;
        ret.push_back(pin);
    }

    // Exported resources
    for (const std::string& name : node.subGraphData.exportedResources)
    {
        int connectionIndex = -1;
        for (int i = 0; i < (int)node.connections.size(); ++i)
        {
            if (node.connections[i].srcPin != name)
                continue;
            connectionIndex = i;
            break;
        }
        Assert(connectionIndex >= 0, "GetNodePins in SubGraph could not find pin for exported resource!");

        NodePinInfo pin;
        pin.name = name;
        pin.isInput = false;
        pin.inputNode = &node.connections[connectionIndex].dstNode;
        pin.inputNodePin = &node.connections[connectionIndex].dstPin;
        ret.push_back(pin);
    }

    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Action_Barrier& node)
{
    // make sure there is always an empty pin at the end
    if (node.connections.size() == 0 || !node.connections.rbegin()->dstNode.empty())
    {
        node.connections.resize(node.connections.size() + 1);

        char buffer[256];
        sprintf_s(buffer, "Pin %i", (int)node.connections.size() - 1);
        node.connections.rbegin()->srcPin = buffer;
    }

    // make a pin for each connection
    std::vector<NodePinInfo> ret(node.connections.size());
    for (size_t i = 0; i < node.connections.size(); ++i)
    {
        ret[i].name = node.connections[i].srcPin;
        ret[i].inputNode = &node.connections[i].dstNode;
        ret[i].inputNodePin = &node.connections[i].dstPin;
        ret[i].accessLabel = " (Barrier)";
    }
    return ret;
}

inline std::vector<NodePinInfo> GetNodePins(const RenderGraph& renderGraph, RenderGraphNode_Reroute& node)
{
    node.connections.resize(1);
    node.connections[0].srcPin = "Pin";

    std::vector<NodePinInfo> result(1);

	result[0].name = node.connections[0].srcPin;
	result[0].inputNode = &node.connections[0].dstNode;
	result[0].inputNodePin = &node.connections[0].dstPin;
	result[0].accessLabel = "";

    return result;
}

inline int GetResourceNodeIndexForPin(RenderGraph& renderGraph, const char* nodeName, const char* pinName)
{
    while (true)
    {
        int nodeIndex = GetNodeIndexByName(renderGraph, nodeName);
        if (nodeIndex == -1)
            return -1;

        std::vector<NodePinInfo> pinInfo = GetNodePins(renderGraph, renderGraph.nodes[nodeIndex]);
        int pinIndex = GetNodePinIndexByName(pinInfo, pinName);
        if (pinIndex == -1)
            return -1;

        if (pinInfo[pinIndex].inputNode == nullptr || pinInfo[pinIndex].inputNode->empty())
            return nodeIndex;

        nodeName = pinInfo[pinIndex].inputNode->c_str();
        pinName = pinInfo[pinIndex].inputNodePin->c_str();
    }

    return -1;
}