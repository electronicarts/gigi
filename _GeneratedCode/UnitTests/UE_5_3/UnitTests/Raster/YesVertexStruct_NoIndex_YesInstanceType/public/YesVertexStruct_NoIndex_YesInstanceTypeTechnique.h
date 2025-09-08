#pragma once

#include "ScreenPass.h"

namespace YesVertexStruct_NoIndex_YesInstanceType
{
    // Inputs
    struct FTechniqueParamsInputs
    {

        // Buffer Vertex_Buffer
        FRDGBufferRef Buffer_Vertex_Buffer = nullptr;
        EPixelFormat BufferFormat_Vertex_Buffer = PF_Unknown;
        FVertexDeclarationElementList BufferVertexFormat_Vertex_Buffer;

        // Buffer Instance_Buffer
        FRDGBufferRef Buffer_Instance_Buffer = nullptr;
        EPixelFormat BufferFormat_Instance_Buffer = PF_Unknown;
        FVertexDeclarationElementList BufferVertexFormat_Instance_Buffer;
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
        Normal,
        UV,
        Solid,
    };

    inline const char* EnumToString(ViewMode value, bool displayString = false)
    {
        switch(value)
        {
            case ViewMode::Normal: return displayString ? "Normal" : "Normal";
            case ViewMode::UV: return displayString ? "UV" : "UV";
            case ViewMode::Solid: return displayString ? "Solid" : "Solid";
            default: return nullptr;
        }
    }

    struct Struct_VertexFormat
    {
        FVector3f Position = {0.f, 0.f, 0.f};
        FVector3f Normal = {0.f, 0.f, 0.f};
        FVector2f UV = {0.f, 0.f};
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
