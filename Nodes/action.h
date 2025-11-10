///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline bool GetIsResourceNode(const RenderGraphNode_ActionBase& node)
    {
        return node.c_isResourceNode;
    }

    // Action specific
    inline void AddResourceDependency(RenderGraphNode_ActionBase& node, int pinIndex, int resourceNodeIndex, ShaderResourceType type, ShaderResourceAccessType accessedAs)
    {
        ResourceDependency dep;
        dep.nodeIndex = resourceNodeIndex;
        dep.pinIndex = pinIndex;
        dep.access = accessedAs;
        dep.type = type;
        node.resourceDependencies.push_back(dep);
    }

    // resource specific
    inline void SetStartingState(RenderGraphNode_ActionBase& node, ShaderResourceAccessType state)
    {
        GigiAssert(false, "Function should not get called for action nodes");
    }

    inline void SetFinalState(RenderGraphNode_ActionBase& node, ShaderResourceAccessType state)
    {
        GigiAssert(false, "Function should not get called for action nodes");
    }

    inline ShaderResourceAccessType GetStartingState(const RenderGraphNode_ActionBase& node)
    {
        GigiAssert(false, "Function should not get called for action nodes");
        return ShaderResourceAccessType::Count;
    }

    inline ShaderResourceAccessType GetFinalState(const RenderGraphNode_ActionBase& node)
    {
        GigiAssert(false, "Function should not get called for action nodes");
        return ShaderResourceAccessType::Count;
    }

    inline ResourceVisibility GetResourceVisibility(const RenderGraphNode_ActionBase& node)
    {
        GigiAssert(false, "Function should not get called for action nodes");
        return ResourceVisibility::Count;
    }

    inline void AddResourceAccessedAs(RenderGraphNode_ActionBase& node, ShaderResourceAccessType state, ShaderResourceAccessType originalState)
    {
        GigiAssert(false, "Function should not get called for action nodes");
    }
};
