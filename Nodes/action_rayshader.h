///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_RayShader& node)
    {
        return (int)node.shader.shader->resources.size();
    }

    inline std::string GetPinName(const RenderGraphNode_Action_RayShader& node, int pinIndex)
    {
        if (pinIndex < node.shader.shader->resources.size())
            return node.shader.shader->resources[pinIndex].name;
        else
            return "";
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_RayShader& node, int pinIndex)
    {
        InputNodeInfo ret;
        ret.nodeIndex = -1;

        if (pinIndex < node.shader.shader->resources.size())
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
                    return ret;
                }
            }
        }

        return ret;
    }
};
