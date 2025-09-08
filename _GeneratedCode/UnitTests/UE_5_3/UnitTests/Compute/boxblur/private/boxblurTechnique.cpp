#include "boxblurTechnique.h"
#include "ScenePrivate.h"

static bool CVarChanged_boxblur_enabled = false;
TAutoConsoleVariable<bool> CVar_boxblur_enabled(
    TEXT("r.boxblur.enabled"), bool(true),
    TEXT("Enables or disables the blurring effect."),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_boxblur_enabled = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_boxblur_radius = false;
TAutoConsoleVariable<int32> CVar_boxblur_radius(
    TEXT("r.boxblur.radius"), int32(2),
    TEXT("The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2"),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_boxblur_radius = true;
        }),
    ECVF_RenderThreadSafe);

static bool CVarChanged_boxblur_sRGB = false;
TAutoConsoleVariable<bool> CVar_boxblur_sRGB(
    TEXT("r.boxblur.sRGB"), bool(true),
    TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)
        {
            CVarChanged_boxblur_sRGB = true;
        }),
    ECVF_RenderThreadSafe);

namespace boxblur
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "BlurH" - Horizontal blur pass
class FBlurHCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FBlurHCS);
    SHADER_USE_PARAMETER_STRUCT(FBlurHCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, Input)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
        SHADER_PARAMETER(int32, cb_BoxBlur_0CB_radius)
        SHADER_PARAMETER(uint32, cb_BoxBlur_0CB_sRGB)
        SHADER_PARAMETER(FVector2f, cb_BoxBlur_0CB__padding0)
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

IMPLEMENT_GLOBAL_SHADER(FBlurHCS, "/Engine/Private/boxblur/boxblur_0.usf", "BlurH", SF_Compute);

// Compute Shader "BlurV" - Vertical blur pass
class FBlurVCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FBlurVCS);
    SHADER_USE_PARAMETER_STRUCT(FBlurVCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, Input)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
        SHADER_PARAMETER(int32, cb_BoxBlur_1CB_radius)
        SHADER_PARAMETER(uint32, cb_BoxBlur_1CB_sRGB)
        SHADER_PARAMETER(FVector2f, cb_BoxBlur_1CB__padding0)
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

IMPLEMENT_GLOBAL_SHADER(FBlurVCS, "/Engine/Private/boxblur/boxblur_1.usf", "BlurV", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    //We can't modify cvars on render thread, so we keep the real values in view state, which can be modified by code when needed.
    if (CVarChanged_boxblur_enabled)
    {
        View.ViewState->TechniqueState_boxblur.Var_enabled = CVar_boxblur_enabled.GetValueOnRenderThread();
        CVarChanged_boxblur_enabled = false;
    }

    if (CVarChanged_boxblur_radius)
    {
        View.ViewState->TechniqueState_boxblur.Var_radius = CVar_boxblur_radius.GetValueOnRenderThread();
        CVarChanged_boxblur_radius = false;
    }

    if (CVarChanged_boxblur_sRGB)
    {
        View.ViewState->TechniqueState_boxblur.Var_sRGB = CVar_boxblur_sRGB.GetValueOnRenderThread();
        CVarChanged_boxblur_sRGB = false;
    }

    FRDGTextureRef Texture_InputTexture = params.inputs.Texture_InputTexture;

    // Transient texture PingPongTexture - An internal texture used during the blurring process
    FRDGTextureRef Texture_PingPongTexture;
    {
        // Calculate size
        FIntVector textureSize = Texture_InputTexture->Desc.GetSize();
        textureSize.X = ((textureSize.X + 0) * 1) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 1) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::UAV | ETextureCreateFlags::ShaderResource;

        EPixelFormat textureFormat = Texture_InputTexture->Desc.Format;
        if (((uint64)Texture_InputTexture->Desc.Flags & (uint64)ETextureCreateFlags::SRGB) != 0)
            createFlags |= ETextureCreateFlags::SRGB;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        Texture_PingPongTexture = GraphBuilder.CreateTexture(desc, TEXT("boxblur.PingPongTexture"));
    }

    // Run Compute Shader "BlurH" - Horizontal blur pass
    if (View.ViewState->TechniqueState_boxblur.Var_enabled)
    {
        // Set shader parameters
        FBlurHCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBlurHCS::FParameters>();
        PassParameters->Input = Texture_InputTexture;
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_PingPongTexture);
        PassParameters->cb_BoxBlur_0CB_radius = View.ViewState->TechniqueState_boxblur.Var_radius;
        PassParameters->cb_BoxBlur_0CB_sRGB = View.ViewState->TechniqueState_boxblur.Var_sRGB;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_InputTexture->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FBlurHCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("boxblur.BlurH"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "BlurV" - Vertical blur pass
    if (View.ViewState->TechniqueState_boxblur.Var_enabled)
    {
        // Set shader parameters
        FBlurVCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBlurVCS::FParameters>();
        PassParameters->Input = Texture_PingPongTexture;
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_InputTexture);
        PassParameters->cb_BoxBlur_1CB_radius = View.ViewState->TechniqueState_boxblur.Var_radius;
        PassParameters->cb_BoxBlur_1CB_sRGB = View.ViewState->TechniqueState_boxblur.Var_sRGB;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_InputTexture->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FBlurVCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("boxblur.BlurV"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

}

};
