///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_ComputeShader& node)
    {
        int shaderResourceCount = 0;
        if (node.shader.shader)
            shaderResourceCount = (int)node.shader.shader->resources.size();

        if(node.enableIndirect)
            ++shaderResourceCount;  // Indirect dispatch buffer

        return shaderResourceCount; // Shader resources
    }

    inline std::string GetPinName(const RenderGraphNode_Action_ComputeShader& node, int pinIndex)
    {
        int shaderResourceCount = 0;
        if (node.shader.shader)
            shaderResourceCount = (int)node.shader.shader->resources.size();

        if (pinIndex < shaderResourceCount)
            return node.shader.shader->resources[pinIndex].name;
        else if (node.enableIndirect && pinIndex == shaderResourceCount)
            return "indirectBuffer";
        else
            return "";
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_ComputeShader& node, int pinIndex)
    {
        InputNodeInfo ret;
        ret.nodeIndex = -1;

        int shaderResourceCount = 0;
        if (node.shader.shader)
            shaderResourceCount = (int)node.shader.shader->resources.size();

        if (pinIndex < shaderResourceCount)
        {
            for (size_t index = 0; index < node.connections.size(); ++index)
            {
                const NodePinConnection& connection = node.connections[index];
                if (connection.srcNodePinIndex == pinIndex)
                {
                    ret.access = node.shader.shader->resources[index].access;
                    ret.originalAccess = node.shader.shader->resources[index].originalAccess;
                    ret.nodeIndex = connection.dstNodeIndex;
                    ret.pinIndex = connection.dstNodePinIndex;
                    ret.shaderResource = &node.shader.shader->resources[index];
                    return ret;
                }
            }
        }
        else if (node.enableIndirect && pinIndex == shaderResourceCount)
        {
            ret.access = ShaderResourceAccessType::Indirect;
            ret.nodeIndex = node.dispatchSize.indirectBuffer.nodeIndex;
            ret.pinIndex = node.dispatchSize.indirectBuffer.nodePinIndex;
            ret.required = false;
        }

        return ret;
    }
};
