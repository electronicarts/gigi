#pragma once

#include "ScreenPass.h"

namespace SlangAutoDiff
{
    // Inputs
    struct FTechniqueParamsInputs
    {
        FRDGTextureRef Texture_Output = nullptr;
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
