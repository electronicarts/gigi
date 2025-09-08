#pragma once

#include "ScreenPass.h"

namespace simpleRaster2
{
    // Inputs
    struct FTechniqueParamsInputs
    {

        // Buffer VertexBuffer
        FRDGBufferRef Buffer_VertexBuffer = nullptr;
        EPixelFormat BufferFormat_VertexBuffer = PF_Unknown;
        FVertexDeclarationElementList BufferVertexFormat_VertexBuffer;
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

    struct Struct_VertexFormat
    {
        FVector3f Position = {0.f, 0.f, 0.f};
        FVector3f Normal = {0.f, 0.f, 0.f};
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
