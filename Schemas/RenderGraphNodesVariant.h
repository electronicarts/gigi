///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

VARIANT_BEGIN(RenderGraphNode, "Render graph node variant")
    VARIANT_TYPE(RenderGraphNode_Resource_Buffer, resourceBuffer, {}, "")
    VARIANT_TYPE(RenderGraphNode_Resource_ShaderConstants, resourceShaderConstants, {}, "")
    VARIANT_TYPE(RenderGraphNode_Resource_Texture, resourceTexture, {}, "")
    VARIANT_TYPE(RenderGraphNode_Reroute, reroute, {}, "")

    VARIANT_TYPE(RenderGraphNode_Action_ComputeShader, actionComputeShader, {}, "")
    VARIANT_TYPE(RenderGraphNode_Action_RayShader, actionRayShader, {}, "")
    VARIANT_TYPE(RenderGraphNode_Action_CopyResource, actionCopyResource, {}, "")
    VARIANT_TYPE(RenderGraphNode_Action_DrawCall, actionDrawCall, {}, "")
    VARIANT_TYPE(RenderGraphNode_Action_SubGraph, actionSubGraph, {}, "")
    VARIANT_TYPE(RenderGraphNode_Action_Barrier, actionBarrier, {}, "")
VARIANT_END()
