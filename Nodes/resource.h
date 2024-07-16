///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline bool GetIsResourceNode(const RenderGraphNode_ResourceBase& node)
    {
        return node.c_isResourceNode;
    }

    inline void AddResourceDependency(RenderGraphNode_ResourceBase& node, int pinIndex, int resourceNodeIndex, ShaderResourceType type, ShaderResourceAccessType accessedAs)
    {
        Assert(false, "Function should not get called for resource nodes");
    }
};
