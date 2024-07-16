///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Resource_Texture& node) { return 1; }

    inline std::string GetPinName(const RenderGraphNode_Resource_Texture& node, int pinIndex)
    {
        if (pinIndex == 0)
            return "resource";
        else
            return "";
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Resource_Texture& node, int pinIndex)
    {
        // textures don't have any inputs
        InputNodeInfo ret;
        ret.nodeIndex = -1;
        return ret;
    }

    inline void SetStartingState(RenderGraphNode_Resource_Texture& node, ShaderResourceAccessType state)
    {
        node.startingState = state;
    }

    inline void SetFinalState(RenderGraphNode_Resource_Texture& node, ShaderResourceAccessType state)
    {
        node.finalState = state;
    }

    inline ShaderResourceAccessType GetStartingState(const RenderGraphNode_Resource_Texture& node)
    {
        return node.startingState;
    }

    inline ShaderResourceAccessType GetFinalState(const RenderGraphNode_Resource_Texture& node)
    {
        return node.finalState;
    }

    inline ResourceVisibility GetResourceVisibility(const RenderGraphNode_Resource_Texture& node)
    {
        return node.visibility;
    }

    inline void AddResourceAccessedAs(RenderGraphNode_Resource_Texture& node, ShaderResourceAccessType state)
    {
        node.accessedAs |= (1 << (unsigned int)state);
    }
};
