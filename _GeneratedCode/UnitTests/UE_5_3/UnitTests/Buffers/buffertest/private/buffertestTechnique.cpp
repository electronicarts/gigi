#include "buffertestTechnique.h"
#include "ScenePrivate.h"

static bool CVarChanged_buffertest_gain = false;
TAutoConsoleVariable<float> CVar_buffertest_gain(
    TEXT("r.buffertest.gain"), float(0.5f),
    TEXT("Overall Volume Adjustment"),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_buffertest_gain = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_buffertest_alpha1 = false;
TAutoConsoleVariable<float> CVar_buffertest_alpha1(
    TEXT("r.buffertest.alpha1"), float(1.f),
    TEXT("Adjusts the contribution of sample n-1"),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_buffertest_alpha1 = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_buffertest_alpha2 = false;
TAutoConsoleVariable<float> CVar_buffertest_alpha2(
    TEXT("r.buffertest.alpha2"), float(0.f),
    TEXT("Adjusts the contribution of sample n-2"),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_buffertest_alpha2 = true;
        }),
    ECVF_RenderThreadSafe);

namespace buffertest
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "BufferTest" - BufferTest compute shader
class FBufferTestCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FBufferTestCS);
    SHADER_USE_PARAMETER_STRUCT(FBufferTestCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float>, InputTyped)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>, OutputTyped)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float>, InputTypedStruct)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float>, OutputTypedStruct)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<TestStruct>, InputStructured)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<TestStruct>, OutputStructured)
        SHADER_PARAMETER_RDG_BUFFER_SRV(ByteAddressBuffer, InputTypedRaw)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWByteAddressBuffer, OutputTypedRaw)
        SHADER_PARAMETER(float, cb_BufferTest_0CB_alpha1)
        SHADER_PARAMETER(float, cb_BufferTest_0CB_alpha2)
        SHADER_PARAMETER(float, cb_BufferTest_0CB_gain)
        SHADER_PARAMETER(float, cb_BufferTest_0CB__padding0)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

        OutEnvironment.SetDefine(TEXT("__GigiDispatchMultiply"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchDivide"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPreAdd"), TEXT("uint3(0,0,0)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPostAdd"), TEXT("uint3(0,0,0)"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FBufferTestCS, "/Engine/Private/buffertest/BufferTest.usf", "Main", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    //We can't modify cvars on render thread, so we keep the real values in view state, which can be modified by code when needed.
    if (CVarChanged_buffertest_gain)
    {
        View.ViewState->TechniqueState_buffertest.Var_gain = CVar_buffertest_gain.GetValueOnRenderThread();
        CVarChanged_buffertest_gain = false;
    }

    if (CVarChanged_buffertest_alpha1)
    {
        View.ViewState->TechniqueState_buffertest.Var_alpha1 = CVar_buffertest_alpha1.GetValueOnRenderThread();
        CVarChanged_buffertest_alpha1 = false;
    }

    if (CVarChanged_buffertest_alpha2)
    {
        View.ViewState->TechniqueState_buffertest.Var_alpha2 = CVar_buffertest_alpha2.GetValueOnRenderThread();
        CVarChanged_buffertest_alpha2 = false;
    }

    FRDGBufferRef Buffer_InputTypedBuffer = params.inputs.Buffer_InputTypedBuffer;

    // Buffer OutputTypedBuffer - An internal buffer used during the filtering process.
    FRDGBufferRef Buffer_OutputTypedBuffer;
    {
        // Calculate size
        uint32 NumElements = Buffer_InputTypedBuffer->Desc.NumElements;
        NumElements = ((NumElements + 0) * 1) / 1 + 0;

        uint32 BytesPerElement = Buffer_InputTypedBuffer->Desc.BytesPerElement;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static | EBufferUsageFlags::UnorderedAccess;

        // Create Buffer
        Buffer_OutputTypedBuffer = GraphBuilder.CreateBuffer(desc, TEXT("buffertest.OutputTypedBuffer"));
    }

    FRDGBufferRef Buffer_InputStructuredBuffer = params.inputs.Buffer_InputStructuredBuffer;

    // Buffer OutputStructuredBuffer
    FRDGBufferRef Buffer_OutputStructuredBuffer;
    {
        // Calculate size
        uint32 NumElements = Buffer_InputStructuredBuffer->Desc.NumElements;
        NumElements = ((NumElements + 0) * 1) / 1 + 0;

        uint32 BytesPerElement = Buffer_InputStructuredBuffer->Desc.BytesPerElement;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static | EBufferUsageFlags::UnorderedAccess | EBufferUsageFlags::StructuredBuffer;

        // Create Buffer
        Buffer_OutputStructuredBuffer = GraphBuilder.CreateBuffer(desc, TEXT("buffertest.OutputStructuredBuffer"));
    }

    FRDGBufferRef Buffer_InputTypedBufferRaw = params.inputs.Buffer_InputTypedBufferRaw;

    // Buffer OutputTypedBufferRaw - An internal buffer used during the filtering process.
    FRDGBufferRef Buffer_OutputTypedBufferRaw;
    {
        // Calculate size
        uint32 NumElements = Buffer_InputTypedBufferRaw->Desc.NumElements;
        NumElements = ((NumElements + 0) * 1) / 1 + 0;

        uint32 BytesPerElement = Buffer_InputTypedBufferRaw->Desc.BytesPerElement;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static | EBufferUsageFlags::UnorderedAccess | EBufferUsageFlags::ByteAddressBuffer;

        // Create Buffer
        Buffer_OutputTypedBufferRaw = GraphBuilder.CreateBuffer(desc, TEXT("buffertest.OutputTypedBufferRaw"));
    }

    FRDGBufferRef Buffer_InputTypedStructBuffer = params.inputs.Buffer_InputTypedStructBuffer;

    // Buffer OutputTypedStructBuffer - An internal buffer used during the filtering process.
    FRDGBufferRef Buffer_OutputTypedStructBuffer;
    {
        // Calculate size
        uint32 NumElements = Buffer_InputTypedBuffer->Desc.NumElements;
        NumElements = ((NumElements + 0) * 1) / 1 + 0;

        uint32 BytesPerElement = Buffer_InputTypedBuffer->Desc.BytesPerElement;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static | EBufferUsageFlags::UnorderedAccess | EBufferUsageFlags::StructuredBuffer;

        // Create Buffer
        Buffer_OutputTypedStructBuffer = GraphBuilder.CreateBuffer(desc, TEXT("buffertest.OutputTypedStructBuffer"));
    }

    // Run Compute Shader "BufferTest" - BufferTest compute shader
    {
        // Set shader parameters
        FBufferTestCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBufferTestCS::FParameters>();
        PassParameters->InputTyped = GraphBuilder.CreateSRV(Buffer_InputTypedBuffer, params.inputs.BufferFormat_InputTypedBuffer);
        PassParameters->OutputTyped = GraphBuilder.CreateUAV(Buffer_OutputTypedBuffer, params.inputs.BufferFormat_InputTypedBuffer);
        PassParameters->InputTypedStruct = GraphBuilder.CreateSRV(Buffer_InputTypedStructBuffer, params.inputs.BufferFormat_InputTypedStructBuffer);
        PassParameters->OutputTypedStruct = GraphBuilder.CreateUAV(Buffer_OutputTypedStructBuffer, params.inputs.BufferFormat_InputTypedBuffer);
        PassParameters->InputStructured = GraphBuilder.CreateSRV(Buffer_InputStructuredBuffer, PF_Unknown);
        PassParameters->OutputStructured = GraphBuilder.CreateUAV(Buffer_OutputStructuredBuffer, PF_Unknown);
        PassParameters->InputTypedRaw = GraphBuilder.CreateSRV(Buffer_InputTypedBufferRaw, params.inputs.BufferFormat_InputTypedBufferRaw);
        PassParameters->OutputTypedRaw = GraphBuilder.CreateUAV(Buffer_OutputTypedBufferRaw, params.inputs.BufferFormat_InputTypedBufferRaw);
        PassParameters->cb_BufferTest_0CB_alpha1 = View.ViewState->TechniqueState_buffertest.Var_alpha1;
        PassParameters->cb_BufferTest_0CB_alpha2 = View.ViewState->TechniqueState_buffertest.Var_alpha2;
        PassParameters->cb_BufferTest_0CB_gain = View.ViewState->TechniqueState_buffertest.Var_gain;

        // Calculate dispatch size
        FIntVector dispatchSize = FIntVector(Buffer_InputTypedBuffer->Desc.NumElements, 1, 1);
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 64 - 1) / 64;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FBufferTestCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("buffertest.BufferTest"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    params.outputs.Buffer_OutputTypedBuffer = Buffer_OutputTypedBuffer;

    params.outputs.Buffer_OutputStructuredBuffer = Buffer_OutputStructuredBuffer;

    params.outputs.Buffer_OutputTypedBufferRaw = Buffer_OutputTypedBufferRaw;

    params.outputs.Buffer_OutputTypedStructBuffer = Buffer_OutputTypedStructBuffer;

}

};
