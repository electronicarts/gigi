#include "BarrierTestTechnique.h"
#include "ScenePrivate.h"

namespace BarrierTest
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "Draw_Left"
class FDraw_LeftCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FDraw_LeftCS);
    SHADER_USE_PARAMETER_STRUCT(FDraw_LeftCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
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
        OutEnvironment.SetDefine(TEXT("__GigiDispatchDivide"), TEXT("uint3(2,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPreAdd"), TEXT("uint3(0,0,0)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPostAdd"), TEXT("uint3(0,0,0)"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FDraw_LeftCS, "/Engine/Private/BarrierTest/BarrierTestLeft.usf", "csmain", SF_Compute);

// Compute Shader "Draw_Right"
class FDraw_RightCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FDraw_RightCS);
    SHADER_USE_PARAMETER_STRUCT(FDraw_RightCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
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
        OutEnvironment.SetDefine(TEXT("__GigiDispatchDivide"), TEXT("uint3(2,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPreAdd"), TEXT("uint3(0,0,0)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPostAdd"), TEXT("uint3(0,0,0)"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FDraw_RightCS, "/Engine/Private/BarrierTest/BarrierTestRight.usf", "csmain", SF_Compute);

// Compute Shader "After"
class FAfterCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FAfterCS);
    SHADER_USE_PARAMETER_STRUCT(FAfterCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
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

IMPLEMENT_GLOBAL_SHADER(FAfterCS, "/Engine/Private/BarrierTest/BarrierTestAfter.usf", "csmain", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    // Transient texture Output
    FRDGTextureRef Texture_Output;
    {
        // Calculate size
        FIntVector textureSize = FIntVector(1,1,1);
        textureSize.X = ((textureSize.X + 0) * 512) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 512) / 1 + 0;
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
        Texture_Output = GraphBuilder.CreateTexture(desc, TEXT("BarrierTest.Output"));
    }

    // Run Compute Shader "Draw_Left"
    {
        // Set shader parameters
        FDraw_LeftCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FDraw_LeftCS::FParameters>();
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Output);

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Output->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 2 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FDraw_LeftCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("BarrierTest.Draw_Left"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "Draw_Right"
    {
        // Set shader parameters
        FDraw_RightCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FDraw_RightCS::FParameters>();
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Output);

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Output->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 2 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FDraw_RightCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("BarrierTest.Draw_Right"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "After"
    {
        // Set shader parameters
        FAfterCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FAfterCS::FParameters>();
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Output);

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Output->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FAfterCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("BarrierTest.After"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    params.outputs.Texture_Output = Texture_Output;

}

};
