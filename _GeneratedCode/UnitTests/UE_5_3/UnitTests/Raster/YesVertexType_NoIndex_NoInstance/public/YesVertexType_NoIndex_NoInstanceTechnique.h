#pragma once

#include "ScreenPass.h"

namespace YesVertexType_NoIndex_NoInstance
{
    // Inputs
    struct FTechniqueParamsInputs
    {

        // Buffer Vertex_Buffer
        FRDGBufferRef Buffer_Vertex_Buffer = nullptr;
        EPixelFormat BufferFormat_Vertex_Buffer = PF_Unknown;
        FVertexDeclarationElementList BufferVertexFormat_Vertex_Buffer;
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
        FRDGTextureRef Texture_Color_Buffer = nullptr;
        FRDGTextureRef Texture_Depth_Buffer = nullptr;
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
