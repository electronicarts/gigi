#include "simpleTechnique.h"
#include "ScenePrivate.h"

namespace simple
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "DoSimpleCS" - Runs the shader
class FDoSimpleCSCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FDoSimpleCSCS);
    SHADER_USE_PARAMETER_STRUCT(FDoSimpleCSCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Input)
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

IMPLEMENT_GLOBAL_SHADER(FDoSimpleCSCS, "/Engine/Private/simple/simple.usf", "Main", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    FRDGTextureRef Texture_Input = params.inputs.Texture_Input;

    // Run Compute Shader "DoSimpleCS" - Runs the shader
    if (View.ViewState->TechniqueState_simple.Var_DummyConstVar == 1)
    {
        // Set shader parameters
        FDoSimpleCSCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FDoSimpleCSCS::FParameters>();
        PassParameters->Input = GraphBuilder.CreateUAV(Texture_Input);

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Input->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FDoSimpleCSCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("simple.DoSimpleCS"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

}

};
