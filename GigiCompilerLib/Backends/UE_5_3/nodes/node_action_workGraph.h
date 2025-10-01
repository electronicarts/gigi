///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

static bool ProcessNodeTag(const RenderGraph& renderGraph, const RenderGraphNode_Action_WorkGraph& node, std::string& tag, std::string& text)
{
    Assert(false, "WorkGraph Nodes are not supported by Unreal Engine CodeGen. (node = \"%s\")", node.name.c_str());
    return false;
}
