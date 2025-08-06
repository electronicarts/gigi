///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_Barrier& node)
{
	// No-op. Should never get to a backend.
	Assert(false, "Barrier nodes should be removed by the front end.");
}
