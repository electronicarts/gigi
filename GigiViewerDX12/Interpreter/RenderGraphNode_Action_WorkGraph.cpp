///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"
#include "DX12Utils/CompileShaders.h"

void RuntimeTypes::RenderGraphNode_Action_WorkGraph::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
    RenderGraphNode_Base::Release(interpreter);

    if (m_stateObject)
    {
        m_stateObject->Release();
        m_stateObject = nullptr;
    }

}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_WorkGraph& node, RuntimeTypes::RenderGraphNode_Action_WorkGraph& runtimeData, NodeAction nodeAction)
{
    if (nodeAction == NodeAction::Init)
    {

    }
    else if (nodeAction == NodeAction::Execute)
    {

    }
}