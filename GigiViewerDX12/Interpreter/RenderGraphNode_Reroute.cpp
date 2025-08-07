///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Reroute& node, RuntimeTypes::RenderGraphNode_Reroute& runtimeData, NodeAction nodeAction)
{
	// No-op. Should never get to a backend.
	Assert(false, "Reroute nodes should be removed by the front end.");
	return false;
}
