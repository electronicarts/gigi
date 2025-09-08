#pragma once

#include "ScreenPass.h"

namespace StructuredBuffer
{
    // Inputs
    struct FTechniqueParamsInputs
    {

        // Buffer buff
        FRDGBufferRef Buffer_buff = nullptr;
        EPixelFormat BufferFormat_buff = PF_Unknown;
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

    enum class Trinary: int
    {
        True,
        False,
        Maybe,
    };

    inline const char* EnumToString(Trinary value, bool displayString = false)
    {
        switch(value)
        {
            case Trinary::True: return displayString ? "True" : "True";
            case Trinary::False: return displayString ? "False" : "False";
            case Trinary::Maybe: return displayString ? "Maybe" : "Maybe";
            default: return nullptr;
        }
    }

    struct Struct_TheStructure
    {
        int32 TheInt = 0;
        float TheFloat = 0.f;
        int32 TheEnum = (int)Trinary::True;
        bool TheBool = true;
        uint32 TheUINT = 0;
        FVector2f TheFloat2 = {0.f, 0.f};
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
