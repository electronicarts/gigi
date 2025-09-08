#pragma once

#include "ScreenPass.h"

namespace BarrierTest
{
    // Inputs
    struct FTechniqueParamsInputs
    {
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
        FRDGTextureRef Texture_Output = nullptr;
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
