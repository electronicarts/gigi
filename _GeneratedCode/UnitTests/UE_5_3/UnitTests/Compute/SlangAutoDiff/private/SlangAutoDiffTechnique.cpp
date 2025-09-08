#include "SlangAutoDiffTechnique.h"
#include "ScenePrivate.h"

static bool CVarChanged_SlangAutoDiff_NumGaussians = false;
TAutoConsoleVariable<int32> CVar_SlangAutoDiff_NumGaussians(
    TEXT("r.SlangAutoDiff.NumGaussians"), int32(10),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_SlangAutoDiff_NumGaussians = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_SlangAutoDiff_Reset = false;
TAutoConsoleVariable<bool> CVar_SlangAutoDiff_Reset(
    TEXT("r.SlangAutoDiff.Reset"), bool(false),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_SlangAutoDiff_Reset = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_SlangAutoDiff_LearningRate = false;
TAutoConsoleVariable<float> CVar_SlangAutoDiff_LearningRate(
    TEXT("r.SlangAutoDiff.LearningRate"), float(0.100000001f),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_SlangAutoDiff_LearningRate = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_SlangAutoDiff_MaximumStepSize = false;
TAutoConsoleVariable<float> CVar_SlangAutoDiff_MaximumStepSize(
    TEXT("r.SlangAutoDiff.MaximumStepSize"), float(0.00999999978f),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_SlangAutoDiff_MaximumStepSize = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_SlangAutoDiff_UseBackwardAD = false;
TAutoConsoleVariable<bool> CVar_SlangAutoDiff_UseBackwardAD(
    TEXT("r.SlangAutoDiff.UseBackwardAD"), bool(true),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_SlangAutoDiff_UseBackwardAD = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_SlangAutoDiff_QuantizeDisplay = false;
TAutoConsoleVariable<bool> CVar_SlangAutoDiff_QuantizeDisplay(
    TEXT("r.SlangAutoDiff.QuantizeDisplay"), bool(false),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_SlangAutoDiff_QuantizeDisplay = true;
        }),
    ECVF_RenderThreadSafe);

namespace SlangAutoDiff
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "Initialize"
class FInitializeCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FInitializeCS);
    SHADER_USE_PARAMETER_STRUCT(FInitializeCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>, Data)
        SHADER_PARAMETER(int32, cb_InitCB_FrameIndex)
        SHADER_PARAMETER(FVector3f, cb_InitCB__padding0)
        SHADER_PARAMETER(FVector4f, cb_InitCB_MouseState)
        SHADER_PARAMETER(FVector4f, cb_InitCB_MouseStateLastFrame)
        SHADER_PARAMETER(int32, cb_InitCB_NumGaussians)
        SHADER_PARAMETER(FVector3f, cb_InitCB_iResolution)
        SHADER_PARAMETER(uint32, cb_InitCB_initialized)
        SHADER_PARAMETER(FVector3f, cb_InitCB__padding1)
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

IMPLEMENT_GLOBAL_SHADER(FInitializeCS, "/Engine/Private/SlangAutoDiff/SlangAutoDiff_Initialize.usf", "csmain", SF_Compute);

// Compute Shader "GradientDescend"
class FGradientDescendCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FGradientDescendCS);
    SHADER_USE_PARAMETER_STRUCT(FGradientDescendCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>, Data)
        SHADER_PARAMETER(float, cb_Descend_0CB_LearningRate)
        SHADER_PARAMETER(float, cb_Descend_0CB_MaximumStepSize)
        SHADER_PARAMETER(int32, cb_Descend_0CB_NumGaussians)
        SHADER_PARAMETER(uint32, cb_Descend_0CB_UseBackwardAD)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

        OutEnvironment.SetDefine(TEXT("SCREEN_MIN"), TEXT("0.001f"));
        OutEnvironment.SetDefine(TEXT("SCREEN_MAX"), TEXT("0.99f"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchMultiply"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchDivide"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPreAdd"), TEXT("uint3(0,0,0)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPostAdd"), TEXT("uint3(0,0,0)"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FGradientDescendCS, "/Engine/Private/SlangAutoDiff/SlangAutoDiff_Descend_0.usf", "csmain", SF_Compute);

// Compute Shader "Render"
class FRenderCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FRenderCS);
    SHADER_USE_PARAMETER_STRUCT(FRenderCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float>, Data)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
        SHADER_PARAMETER(int32, cb_RenderCB_NumGaussians)
        SHADER_PARAMETER(uint32, cb_RenderCB_QuantizeDisplay)
        SHADER_PARAMETER(FVector2f, cb_RenderCB__padding0)
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

IMPLEMENT_GLOBAL_SHADER(FRenderCS, "/Engine/Private/SlangAutoDiff/SlangAutoDiff_Render.usf", "csmain", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    //We can't modify cvars on render thread, so we keep the real values in view state, which can be modified by code when needed.
    if (CVarChanged_SlangAutoDiff_NumGaussians)
    {
        View.ViewState->TechniqueState_SlangAutoDiff.Var_NumGaussians = CVar_SlangAutoDiff_NumGaussians.GetValueOnRenderThread();
        CVarChanged_SlangAutoDiff_NumGaussians = false;
    }

    if (CVarChanged_SlangAutoDiff_Reset)
    {
        View.ViewState->TechniqueState_SlangAutoDiff.Var_Reset = CVar_SlangAutoDiff_Reset.GetValueOnRenderThread();
        CVarChanged_SlangAutoDiff_Reset = false;
    }

    if (CVarChanged_SlangAutoDiff_LearningRate)
    {
        View.ViewState->TechniqueState_SlangAutoDiff.Var_LearningRate = CVar_SlangAutoDiff_LearningRate.GetValueOnRenderThread();
        CVarChanged_SlangAutoDiff_LearningRate = false;
    }

    if (CVarChanged_SlangAutoDiff_MaximumStepSize)
    {
        View.ViewState->TechniqueState_SlangAutoDiff.Var_MaximumStepSize = CVar_SlangAutoDiff_MaximumStepSize.GetValueOnRenderThread();
        CVarChanged_SlangAutoDiff_MaximumStepSize = false;
    }

    if (CVarChanged_SlangAutoDiff_UseBackwardAD)
    {
        View.ViewState->TechniqueState_SlangAutoDiff.Var_UseBackwardAD = CVar_SlangAutoDiff_UseBackwardAD.GetValueOnRenderThread();
        CVarChanged_SlangAutoDiff_UseBackwardAD = false;
    }

    if (CVarChanged_SlangAutoDiff_QuantizeDisplay)
    {
        View.ViewState->TechniqueState_SlangAutoDiff.Var_QuantizeDisplay = CVar_SlangAutoDiff_QuantizeDisplay.GetValueOnRenderThread();
        CVarChanged_SlangAutoDiff_QuantizeDisplay = false;
    }

    // Buffer Data - first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
    if (!View.ViewState->TechniqueState_SlangAutoDiff.Buffer_Data)
    {
        // Calculate size
        uint32 NumElements = View.ViewState->TechniqueState_SlangAutoDiff.Var_NumGaussians;
        NumElements = ((NumElements + 0) * 5) / 1 + 4;

        uint32 BytesPerElement = 4;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static | EBufferUsageFlags::UnorderedAccess | EBufferUsageFlags::ShaderResource;

        // Create Buffer
        FRDGBufferRef buffer = GraphBuilder.CreateBuffer(desc, TEXT("SlangAutoDiff.Data"));
        View.ViewState->TechniqueState_SlangAutoDiff.Buffer_Data = GraphBuilder.ConvertToExternalBuffer(buffer);
    }
    FRDGBufferRef Buffer_Data = GraphBuilder.RegisterExternalBuffer(View.ViewState->TechniqueState_SlangAutoDiff.Buffer_Data, TEXT("SlangAutoDiff.Data"));

    FRDGTextureRef Texture_Output = params.inputs.Texture_Output;

    // Run Compute Shader "Initialize"
    {
        // Set shader parameters
        FInitializeCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FInitializeCS::FParameters>();
        PassParameters->Data = GraphBuilder.CreateUAV(Buffer_Data, PF_R32_FLOAT);
        PassParameters->cb_InitCB_FrameIndex = View.ViewState->TechniqueState_SlangAutoDiff.Var_FrameIndex;
        PassParameters->cb_InitCB_MouseState = View.ViewState->TechniqueState_SlangAutoDiff.Var_MouseState;
        PassParameters->cb_InitCB_MouseStateLastFrame = View.ViewState->TechniqueState_SlangAutoDiff.Var_MouseStateLastFrame;
        PassParameters->cb_InitCB_NumGaussians = View.ViewState->TechniqueState_SlangAutoDiff.Var_NumGaussians;
        PassParameters->cb_InitCB_iResolution = View.ViewState->TechniqueState_SlangAutoDiff.Var_iResolution;
        PassParameters->cb_InitCB_initialized = View.ViewState->TechniqueState_SlangAutoDiff.Var_initialized;

        // Calculate dispatch size
        FIntVector dispatchSize = FIntVector(1,1,1);
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FInitializeCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("SlangAutoDiff.Initialize"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "GradientDescend"
    {
        // Set shader parameters
        FGradientDescendCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FGradientDescendCS::FParameters>();
        PassParameters->Data = GraphBuilder.CreateUAV(Buffer_Data, PF_R32_FLOAT);
        PassParameters->cb_Descend_0CB_LearningRate = View.ViewState->TechniqueState_SlangAutoDiff.Var_LearningRate;
        PassParameters->cb_Descend_0CB_MaximumStepSize = View.ViewState->TechniqueState_SlangAutoDiff.Var_MaximumStepSize;
        PassParameters->cb_Descend_0CB_NumGaussians = View.ViewState->TechniqueState_SlangAutoDiff.Var_NumGaussians;
        PassParameters->cb_Descend_0CB_UseBackwardAD = View.ViewState->TechniqueState_SlangAutoDiff.Var_UseBackwardAD;

        // Calculate dispatch size
        FIntVector dispatchSize = FIntVector(1,1,1);
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FGradientDescendCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("SlangAutoDiff.GradientDescend"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "Render"
    {
        // Set shader parameters
        FRenderCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FRenderCS::FParameters>();
        PassParameters->Data = GraphBuilder.CreateSRV(Buffer_Data, PF_R32_FLOAT);
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Output);
        PassParameters->cb_RenderCB_NumGaussians = View.ViewState->TechniqueState_SlangAutoDiff.Var_NumGaussians;
        PassParameters->cb_RenderCB_QuantizeDisplay = View.ViewState->TechniqueState_SlangAutoDiff.Var_QuantizeDisplay;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Output->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FRenderCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("SlangAutoDiff.Render"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }


        // Set variables
        View.ViewState->TechniqueState_SlangAutoDiff.Var_initialized = !View.ViewState->TechniqueState_SlangAutoDiff.Var_Reset;
        if(!View.ViewState->TechniqueState_SlangAutoDiff.Var_initialized)
            View.ViewState->TechniqueState_SlangAutoDiff.Var_Reset = false;}

};
