#pragma once

#include "ScreenPass.h"

namespace Strides
{
    // Inputs
    struct FTechniqueParamsInputs
    {
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
