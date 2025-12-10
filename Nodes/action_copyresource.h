///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_CopyResource& node) { return 2; }

    inline std::string GetPinName(const RenderGraphNode_Action_CopyResource& node, int pinIndex)
    {
        switch (pinIndex)
        {
            case 0: return "source";
            case 1: return "dest";
            default:
            {
                GigiAssert(false, "pinIndex out of range for node %s", node.name.c_str());
                return "";
            }
        }
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_CopyResource& node, int pinIndex)
    {
        InputNodeInfo ret;

        switch (pinIndex)
        {
            case 0:
            {
                ret.nodeIndex = node.source.nodeIndex;
                ret.pinIndex = node.source.nodePinIndex;
                ret.access = ShaderResourceAccessType::CopySource;
                break;
            }
            case 1:
            {
                ret.nodeIndex = node.dest.nodeIndex;
                ret.pinIndex = node.dest.nodePinIndex;
                ret.access = ShaderResourceAccessType::CopyDest;
                break;
            }
            default:
            {
                GigiAssert(false, "pinIndex out of range for node %s", node.name.c_str());
            }
        }
        return ret;
    }
};
