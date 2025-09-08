#pragma once

#include "ScreenPass.h"

namespace buffertest
{
    // Inputs
    struct FTechniqueParamsInputs
    {

        // Buffer InputTypedBuffer - This is the buffer to be filtered.
        FRDGBufferRef Buffer_InputTypedBuffer = nullptr;
        EPixelFormat BufferFormat_InputTypedBuffer = PF_Unknown;

        // Buffer InputStructuredBuffer
        FRDGBufferRef Buffer_InputStructuredBuffer = nullptr;
        EPixelFormat BufferFormat_InputStructuredBuffer = PF_Unknown;

        // Buffer InputTypedBufferRaw - This is the buffer to be filtered.
        FRDGBufferRef Buffer_InputTypedBufferRaw = nullptr;
        EPixelFormat BufferFormat_InputTypedBufferRaw = PF_Unknown;

        // Buffer InputTypedStructBuffer - This is the buffer to be filtered.
        FRDGBufferRef Buffer_InputTypedStructBuffer = nullptr;
        EPixelFormat BufferFormat_InputTypedStructBuffer = PF_Unknown;
    };

    // Outputs
    struct FTechniqueParamsOutputs
    {
        FRDGBufferRef Buffer_OutputTypedBuffer = nullptr;  // An internal buffer used during the filtering process.
        FRDGBufferRef Buffer_OutputStructuredBuffer = nullptr;
        FRDGBufferRef Buffer_OutputTypedBufferRaw = nullptr;  // An internal buffer used during the filtering process.
        FRDGBufferRef Buffer_OutputTypedStructBuffer = nullptr;  // An internal buffer used during the filtering process.
    };

    struct FTechniqueParams
    {
        FTechniqueParamsInputs inputs;
        FTechniqueParamsOutputs outputs;
    };

    struct Struct_TestStruct
    {
        FVector4f TheFloat4 = {0.f, 0.f, 0.f, 0.f};
        FIntVector4 TheInt4 = {0, 0, 0, 0};
        bool TheBool = false;
    };

    void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params);
};
