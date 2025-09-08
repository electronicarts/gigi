#pragma once

#include "ScreenPass.h"

namespace boxblur
{
    // Inputs
    struct FTechniqueParamsInputs
    {
        FRDGTextureRef Texture_InputTexture = nullptr;  // This is the texture to be blurred
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
