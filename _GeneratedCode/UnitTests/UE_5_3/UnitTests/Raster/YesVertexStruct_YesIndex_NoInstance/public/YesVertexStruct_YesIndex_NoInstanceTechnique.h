#pragma once

#include "ScreenPass.h"

namespace YesVertexStruct_YesIndex_NoInstance
{
    // Inputs
    struct FTechniqueParamsInputs
    {

        // Buffer Vertex_Buffer
        FRDGBufferRef Buffer_Vertex_Buffer = nullptr;
        EPixelFormat BufferFormat_Vertex_Buffer = PF_Unknown;
        FVertexDeclarationElementList BufferVertexFormat_Vertex_Buffer;

        // Buffer Index_Buffer
        FRDGBufferRef Buffer_Index_Buffer = nullptr;
        EPixelFormat BufferFormat_Index_Buffer = PF_Unknown;
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

    enum class ViewMode: int
    {
        UV,
        Solid,
    };

    inline const char* EnumToString(ViewMode value, bool displayString = false)
    {
        switch(value)
        {
            case ViewMode::UV: return displayString ? "UV" : "UV";
            case ViewMode::Solid: return displayString ? "Solid" : "Solid";
            default: return nullptr;
        }
    }

    struct Struct_VertexFormat
    {
        FVector3f Position = {0.f, 0.f, 0.f};
        FVector2f UV = {0.f, 0.f};
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
