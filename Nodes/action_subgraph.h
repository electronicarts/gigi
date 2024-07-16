///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_SubGraph& node)
    {
        Assert(false, "SubGraph nodes should be removed before these functions are called");
        return 0;
    }

    inline std::string GetPinName(const RenderGraphNode_Action_SubGraph& node, int pinIndex)
    {
        Assert(false, "SubGraph nodes should be removed before these functions are called");
        return "";
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_SubGraph& node, int pinIndex)
    {
        Assert(false, "SubGraph nodes should be removed before these functions are called");
        InputNodeInfo ret;
        ret.nodeIndex = -1;
        return ret;
    }
};
