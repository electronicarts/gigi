///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2026 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

inline StaticNodeInfo GetStaticNodeInfo(RenderGraphNode_Action_External& node)
{
    StaticNodeInfo ret;
    StaticNodePinInfo pinInfo;

    auto NodePinReferenceIsOptional =
        [](auto reference)
        {
            return std::is_same_v<decltype(reference), NodePinReferenceOptional>;
        }
    ;

    switch (node.externalNodeData._index)
    {
        case ExternalNodeData::c_index_AMD_FidelityFXSDK_Upscaling:
        {
            ExternalNode_AMD_FidelityFXSDK_Upscaling& nodeData = node.externalNodeData.AMD_FidelityFXSDK_Upscaling;

            #define HANDLE_PIN(Name, Access, ToolTip) \
                pinInfo.srcPin = #Name; \
                pinInfo.dstNode = &nodeData.Name.node; \
                pinInfo.dstPin = &nodeData.Name.pin; \
                pinInfo.toolTip = ToolTip; \
                pinInfo.readOnly = ShaderResourceTypeIsReadOnly(Access); \
                pinInfo.access = Access; \
                pinInfo.required = !NodePinReferenceIsOptional(nodeData.Name); \
                ret.pins.push_back(pinInfo);

            HANDLE_PIN(color, ShaderResourceAccessType::SRV, "Color buffer for the current frame (at render resolution).");
            HANDLE_PIN(colorOpaqueOnly, ShaderResourceAccessType::SRV, "containing the opaque only color buffer for the current frame (at render resolution).\nOnly needed if generating a reactive mask");
            HANDLE_PIN(depth, ShaderResourceAccessType::SRV, "32bit depth values for the current frame (at render resolution).");
            HANDLE_PIN(motionVectors, ShaderResourceAccessType::SRV, "2-dimensional motion vectors (at render resolution if FFX_FSR_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS is not set).");
            HANDLE_PIN(exposure, ShaderResourceAccessType::SRV, "Optional resource containing a 1x1 exposure value.");
            HANDLE_PIN(reactive, ShaderResourceAccessType::UAV, "Optional resource containing alpha value of reactive objects in the scene.");
            //HANDLE_PIN(transparencyAndComposition, ShaderResourceAccessType::SRV, "Optional resource containing alpha value of special objects in the scene.");
            HANDLE_PIN(output, ShaderResourceAccessType::UAV, "Output color buffer for the current frame (at presentation resolution).");

            #undef HANDLE_PIN

            break;
        }
        default:
        {
            Assert(false, "Unhandled external node type in " __FUNCTION__);
            break;
        }
    }

    for (int i = 0; i < EnumCount<Backend>(); ++i)
        ret.backendSupported[i] = ((Backend)i) == Backend::Interpreter;

    return ret;
}

inline StaticNodeInfo GetStaticNodeInfo(const RenderGraphNode_Action_External& node)
{
    RenderGraphNode_Action_External* nonConstPtr = const_cast<RenderGraphNode_Action_External*>(&node);
    return GetStaticNodeInfo(*nonConstPtr);
}