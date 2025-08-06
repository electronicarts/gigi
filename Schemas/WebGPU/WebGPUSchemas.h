///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

//========================================================
// Structs
//========================================================

STRUCT_BEGIN(WebGPU_RWTextureSplit, "")
    STRUCT_FIELD(std::string, nodeName, "", "The name of the shader affected", 0)
    STRUCT_FIELD(std::string, shaderName, "", "The name of the shader affected", 0)
    STRUCT_FIELD(std::string, pinName, "", "The name of the pin affected. This pin is also write only.", 0)
    STRUCT_FIELD(std::string, pinNameR, "", "The name of the read only version of the pin.", 0)
STRUCT_END()

STRUCT_BEGIN(BackendData_WebGPU, "")
    STRUCT_DYNAMIC_ARRAY(WebGPU_RWTextureSplit, RWTextureSplits, "All RW textures that got split into an R texture and a W texture", 0)
STRUCT_END()
