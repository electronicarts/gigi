#pragma once

#include "ScreenPass.h"

namespace simple
{
    // Inputs
    struct FTechniqueParamsInputs
    {
        FRDGTextureRef Texture_Input = nullptr;  // This is the input texture to be modified
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
