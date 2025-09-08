#pragma once

#include "ScreenPass.h"

namespace CopyResourceTest_FB
{
    // Inputs
    struct FTechniqueParamsInputs
    {
        FRDGTextureRef Texture_Source_Texture = nullptr;

        // Buffer Source_Buffer
        FRDGBufferRef Buffer_Source_Buffer = nullptr;
        EPixelFormat BufferFormat_Source_Buffer = PF_Unknown;
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
        FRDGTextureRef Texture_Texture_From_Texture = nullptr;
        FRDGBufferRef Buffer_Buffer_From_Buffer = nullptr;
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
