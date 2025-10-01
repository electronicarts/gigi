#pragma once

#include "ScreenPass.h"

namespace NoVertex_NoIndex_NoInstance
{
    // Inputs
    struct FTechniqueParamsInputs
    {
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
