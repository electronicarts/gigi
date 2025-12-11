///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_WorkGraph& node)
    {
        int ret = 0;

        ret += node.entryShader.shader ? (int)node.entryShader.shader->resources.size() : 0;

        ret += 1; // records buffer

        return ret;
    }

    inline std::string GetPinName(const RenderGraphNode_Action_WorkGraph& node, int pinIndex)
    {
        // required vertex shader resource pins
        if (node.entryShader.shader)
        {
            if (pinIndex < node.entryShader.shader->resources.size())
                return node.entryShader.shader->resources[pinIndex].name;
            pinIndex -= (int)node.entryShader.shader->resources.size();
        }

        // optional records buffer
        if (pinIndex == 0)
            return "recordsBuffer";
        pinIndex--;

        return "";
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_WorkGraph& node, int pinIndex)
    {
        int origPinIndex = pinIndex;

        InputNodeInfo ret;
        ret.nodeIndex = -1;

        if (node.entryShader.shader)
        {
            if (pinIndex < node.entryShader.shader->resources.size())
            {
                for (size_t index = 0; index < node.connections.size(); ++index)
                {
                    const NodePinConnection& connection = node.connections[index];
                    if (connection.srcNodePinIndex == origPinIndex)
                    {
                        ret.access = node.entryShader.shader->resources[index].access;
                        ret.originalAccess = node.entryShader.shader->resources[index].originalAccess;
                        ret.nodeIndex = connection.dstNodeIndex;
                        ret.pinIndex = connection.dstNodePinIndex;
                        return ret;
                    }
                }
            }
            pinIndex -= (int)node.entryShader.shader->resources.size();
        }

        // Optional records buffer
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::SRV;
            ret.nodeIndex = node.records.nodeIndex;
            ret.pinIndex = node.records.nodePinIndex;
            ret.required = false;
            return ret;
        }
        pinIndex--;

        return ret;

    }

};
