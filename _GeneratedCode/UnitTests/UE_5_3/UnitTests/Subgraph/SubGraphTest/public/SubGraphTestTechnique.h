#pragma once

#include "ScreenPass.h"

namespace SubGraphTest
{
    // Inputs
    struct FTechniqueParamsInputs
    {
        FRDGTextureRef Texture_Test = nullptr;
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
        FRDGTextureRef Texture_Inner_Exported_Tex = nullptr;
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
