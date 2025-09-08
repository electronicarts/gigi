#pragma once

#include "ScreenPass.h"

namespace Stencil
{
    struct FInitParams
    {
        FSoftObjectPath TexturePath__loadedTexture_0; // Asset reference for imported texture cabinsmall.png
    };

    // Must be called from game thread, not render thread
    void Initialize(FInitParams& params);

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
        FRDGTextureRef Texture_Color = nullptr;
        FRDGTextureRef Texture_Depth_Stencil = nullptr;
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    struct Struct_VertexBuffer
    {
        FVector3f pos = {0.f, 0.f, 0.f};
        FVector3f normal = {0.f, 0.f, 0.f};
        FVector2f uv = {0.f, 0.f};
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
