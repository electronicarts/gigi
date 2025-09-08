#pragma once

#include "ScreenPass.h"

namespace TwoRayGensSubgraph
{
    // Inputs
    struct FTechniqueParamsInputs
    {

        // Buffer Scene - The raytracing scene
        FRHIRayTracingScene* Buffer_Scene = nullptr;
        FRDGTextureRef Texture_BlueChannel = nullptr;
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
        FRDGTextureRef Texture_Texture = nullptr;  // The texture that is rendered to
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
