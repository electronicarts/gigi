///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

STRUCT_BEGIN(ExternalNode_AMD_FidelityFXSDK_Upscaling, "")
    STRUCT_STATIC_ARRAY(unsigned int, maxRenderSize, 2, {1024 COMMA 768}, "The maximum size that rendering will be performed at.", 0)
    STRUCT_STATIC_ARRAY(unsigned int, maxUpscaleSize, 2, { 1024 COMMA 768 }, "The size of the presentation resolution targeted by the upscaling process.", 0)
    STRUCT_FIELD(bool, ENABLE_HIGH_DYNAMIC_RANGE, false, "A bit indicating if the input color data provided is using a high-dynamic range.", 0)
    STRUCT_FIELD(bool, ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS, false, "A bit indicating if the motion vectors are rendered at display resolution.", 0)
    STRUCT_FIELD(bool, ENABLE_MOTION_VECTORS_JITTER_CANCELLATION, false, "A bit indicating that the motion vectors have the jittering pattern applied to them.", 0)
    STRUCT_FIELD(bool, ENABLE_DEPTH_INVERTED, false, "A bit indicating that the input depth buffer data provided is inverted [1..0].", 0)
    STRUCT_FIELD(bool, ENABLE_DEPTH_INFINITE, false, "A bit indicating that the input depth buffer data provided is using an infinite far plane.", 0)
    STRUCT_FIELD(bool, ENABLE_AUTO_EXPOSURE, false, "A bit indicating if automatic exposure should be applied to input color data.", 0)
    STRUCT_FIELD(bool, ENABLE_DYNAMIC_RESOLUTION, false, "A bit indicating that the application uses dynamic resolution scaling.", 0)
    STRUCT_FIELD(bool, ENABLE_DEBUG_CHECKING, false, "A bit indicating that the runtime should check some API values and report issues.", 0)
    STRUCT_FIELD(bool, ENABLE_NON_LINEAR_COLORSPACE, false, "A bit indicating that the color resource contains perceptual (gamma corrected) colors", 0)
    STRUCT_FIELD(bool, ENABLE_DEBUG_VISUALIZATION, false, "A bit indicating if debug visualization is allowed. (memory consumption could increase)", 0)
STRUCT_END()

VARIANT_BEGIN(ExternalNodeData, "Render graph node variant")
    VARIANT_TYPE(ExternalNode_AMD_FidelityFXSDK_Upscaling, AMD_FidelityFXSDK_Upscaling, {}, "")
VARIANT_END()
