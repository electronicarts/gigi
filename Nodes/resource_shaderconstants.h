///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Resource_ShaderConstants& node) { return 1; }

    inline std::string GetPinName(const RenderGraphNode_Resource_ShaderConstants& node, int pinIndex)
    {
        if (pinIndex == 0)
            return "resource";
        else
            return "";
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Resource_ShaderConstants& node, int pinIndex)
    {
        // shader constants don't have any inputs
        InputNodeInfo ret;
        ret.nodeIndex = -1;
        return ret;
    }

    inline void SetStartingState(RenderGraphNode_Resource_ShaderConstants& node, ShaderResourceAccessType state)
    {
        node.startingState = state;
    }

    inline void SetFinalState(RenderGraphNode_Resource_ShaderConstants& node, ShaderResourceAccessType state)
    {
        node.finalState = state;
    }

    inline ShaderResourceAccessType GetStartingState(const RenderGraphNode_Resource_ShaderConstants& node)
    {
        return node.startingState;
    }

    inline ShaderResourceAccessType GetFinalState(const RenderGraphNode_Resource_ShaderConstants& node)
    {
        return node.finalState;
    }

    inline ResourceVisibility GetResourceVisibility(const RenderGraphNode_Resource_ShaderConstants& node)
    {
        return ResourceVisibility::Internal;
    }

    inline void AddResourceAccessedAs(RenderGraphNode_Resource_ShaderConstants& node, ShaderResourceAccessType state, ShaderResourceAccessType originalState)
    {
        node.accessedAs |= (1 << (unsigned int)state);

        if (originalState == ShaderResourceAccessType::Count)
            originalState = state;

        node.originallyAccessedAs |= (1 << (unsigned int)originalState);
    }
};
