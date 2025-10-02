#pragma once

#include "ScreenPass.h"

namespace VariableAliases
{
    // Inputs
    struct FTechniqueParamsInputs
    {
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
        FRDGTextureRef Texture_Color = nullptr;
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
