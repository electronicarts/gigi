#pragma once

#include "ScreenPass.h"

namespace /*$(Name)*/
{
/*$(FInitParams)*/    // Inputs
    struct FTechniqueParamsInputs
    {
/*$(FTechniqueParamsInputs)*/    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
/*$(FTechniqueParamsOutputs)*/    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };
/*$(TechniqueHNamespace)*/
    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
