#include "VariableAliasesTechnique.h"
#include "ScenePrivate.h"

static bool CVarChanged_VariableAliases_Node1Channel = false;
TAutoConsoleVariable<int32> CVar_VariableAliases_Node1Channel(
    TEXT("r.VariableAliases.Node1Channel"), int32(0),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_VariableAliases_Node1Channel = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_VariableAliases_Node1Value = false;
TAutoConsoleVariable<float> CVar_VariableAliases_Node1Value(
    TEXT("r.VariableAliases.Node1Value"), float(0.25f),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_VariableAliases_Node1Value = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_VariableAliases_Node2Channel = false;
TAutoConsoleVariable<int32> CVar_VariableAliases_Node2Channel(
    TEXT("r.VariableAliases.Node2Channel"), int32(1),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_VariableAliases_Node2Channel = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_VariableAliases_Node2Value = false;
TAutoConsoleVariable<float> CVar_VariableAliases_Node2Value(
    TEXT("r.VariableAliases.Node2Value"), float(0.5f),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_VariableAliases_Node2Value = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_VariableAliases_Node3Channel = false;
TAutoConsoleVariable<int32> CVar_VariableAliases_Node3Channel(
    TEXT("r.VariableAliases.Node3Channel"), int32(2),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_VariableAliases_Node3Channel = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_VariableAliases_Node3Value = false;
TAutoConsoleVariable<float> CVar_VariableAliases_Node3Value(
    TEXT("r.VariableAliases.Node3Value"), float(0.75f),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_VariableAliases_Node3Value = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_VariableAliases_UnusedFloat = false;
TAutoConsoleVariable<float> CVar_VariableAliases_UnusedFloat(
    TEXT("r.VariableAliases.UnusedFloat"), float(0.f),
    TEXT("This is for the unused alias in the shader"),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_VariableAliases_UnusedFloat = true;
        }),
    ECVF_RenderThreadSafe);

namespace VariableAliases
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "Set_Red"
class FSet_RedCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSet_RedCS);
    SHADER_USE_PARAMETER_STRUCT(FSet_RedCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
        SHADER_PARAMETER(int32, cb_SetChannel_0CB__alias_Channel)
        SHADER_PARAMETER(float, cb_SetChannel_0CB__alias_Value)
        SHADER_PARAMETER(FVector2f, cb_SetChannel_0CB__padding0)
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

IMPLEMENT_GLOBAL_SHADER(FSet_RedCS, "/Engine/Private/VariableAliases/VariableAliases_SetChannel.usf", "main", SF_Compute);

// Compute Shader "Set_Green"
class FSet_GreenCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSet_GreenCS);
    SHADER_USE_PARAMETER_STRUCT(FSet_GreenCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
        SHADER_PARAMETER(int32, cb_SetChannel_0CB__alias_Channel)
        SHADER_PARAMETER(float, cb_SetChannel_0CB__alias_Value)
        SHADER_PARAMETER(FVector2f, cb_SetChannel_0CB__padding0)
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

IMPLEMENT_GLOBAL_SHADER(FSet_GreenCS, "/Engine/Private/VariableAliases/VariableAliases_SetChannel.usf", "main", SF_Compute);

// Compute Shader "Set_Blue"
class FSet_BlueCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSet_BlueCS);
    SHADER_USE_PARAMETER_STRUCT(FSet_BlueCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
        SHADER_PARAMETER(int32, cb_SetChannel_1CB__alias_Channel)
        SHADER_PARAMETER(float, cb_SetChannel_1CB__alias_Value)
        SHADER_PARAMETER(FVector2f, cb_SetChannel_1CB__padding0)
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
        OutEnvironment.SetDefine(TEXT("__GIGI_AlIAS_VARIABLE_CONST_Value"), TEXT("0.75"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FSet_BlueCS, "/Engine/Private/VariableAliases/VariableAliases_SetChannel.usf", "main", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    //We can't modify cvars on render thread, so we keep the real values in view state, which can be modified by code when needed.
    if (CVarChanged_VariableAliases_Node1Channel)
    {
        View.ViewState->TechniqueState_VariableAliases.Var_Node1Channel = CVar_VariableAliases_Node1Channel.GetValueOnRenderThread();
        CVarChanged_VariableAliases_Node1Channel = false;
    }

    if (CVarChanged_VariableAliases_Node1Value)
    {
        View.ViewState->TechniqueState_VariableAliases.Var_Node1Value = CVar_VariableAliases_Node1Value.GetValueOnRenderThread();
        CVarChanged_VariableAliases_Node1Value = false;
    }

    if (CVarChanged_VariableAliases_Node2Channel)
    {
        View.ViewState->TechniqueState_VariableAliases.Var_Node2Channel = CVar_VariableAliases_Node2Channel.GetValueOnRenderThread();
        CVarChanged_VariableAliases_Node2Channel = false;
    }

    if (CVarChanged_VariableAliases_Node2Value)
    {
        View.ViewState->TechniqueState_VariableAliases.Var_Node2Value = CVar_VariableAliases_Node2Value.GetValueOnRenderThread();
        CVarChanged_VariableAliases_Node2Value = false;
    }

    if (CVarChanged_VariableAliases_Node3Channel)
    {
        View.ViewState->TechniqueState_VariableAliases.Var_Node3Channel = CVar_VariableAliases_Node3Channel.GetValueOnRenderThread();
        CVarChanged_VariableAliases_Node3Channel = false;
    }

    if (CVarChanged_VariableAliases_Node3Value)
    {
        View.ViewState->TechniqueState_VariableAliases.Var_Node3Value = CVar_VariableAliases_Node3Value.GetValueOnRenderThread();
        CVarChanged_VariableAliases_Node3Value = false;
    }

    if (CVarChanged_VariableAliases_UnusedFloat)
    {
        View.ViewState->TechniqueState_VariableAliases.Var_UnusedFloat = CVar_VariableAliases_UnusedFloat.GetValueOnRenderThread();
        CVarChanged_VariableAliases_UnusedFloat = false;
    }

    // Transient texture Color
    FRDGTextureRef Texture_Color;
    {
        // Calculate size
        FIntVector textureSize = FIntVector(View.ViewState->TechniqueState_VariableAliases.Var_RenderSize[0], View.ViewState->TechniqueState_VariableAliases.Var_RenderSize[1], 1);
        textureSize.X = ((textureSize.X + 0) * 1) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 1) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::UAV;

        EPixelFormat textureFormat = PF_R8G8B8A8;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        Texture_Color = GraphBuilder.CreateTexture(desc, TEXT("VariableAliases.Color"));
    }

    // Run Compute Shader "Set_Red"
    {
        // Set shader parameters
        FSet_RedCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FSet_RedCS::FParameters>();
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Color);
        PassParameters->cb_SetChannel_0CB__alias_Channel = View.ViewState->TechniqueState_VariableAliases.Var_Node1Channel;
        PassParameters->cb_SetChannel_0CB__alias_Value = View.ViewState->TechniqueState_VariableAliases.Var_Node1Value;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Color->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FSet_RedCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("VariableAliases.Set_Red"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "Set_Green"
    {
        // Set shader parameters
        FSet_GreenCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FSet_GreenCS::FParameters>();
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Color);
        PassParameters->cb_SetChannel_0CB__alias_Channel = View.ViewState->TechniqueState_VariableAliases.Var_Node2Channel;
        PassParameters->cb_SetChannel_0CB__alias_Value = View.ViewState->TechniqueState_VariableAliases.Var_Node2Value;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Color->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FSet_GreenCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("VariableAliases.Set_Green"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "Set_Blue"
    {
        // Set shader parameters
        FSet_BlueCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FSet_BlueCS::FParameters>();
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Color);
        PassParameters->cb_SetChannel_1CB__alias_Channel = View.ViewState->TechniqueState_VariableAliases.Var_Node3Channel;
        PassParameters->cb_SetChannel_1CB__alias_Value = View.ViewState->TechniqueState_VariableAliases.Var_Node3Value;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Color->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FSet_BlueCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("VariableAliases.Set_Blue"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    params.outputs.Texture_Color = Texture_Color;

}

};
