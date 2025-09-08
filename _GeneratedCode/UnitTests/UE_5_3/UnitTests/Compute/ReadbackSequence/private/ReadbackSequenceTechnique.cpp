#include "ReadbackSequenceTechnique.h"
#include "ScenePrivate.h"

namespace ReadbackSequence
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "Node_1"
class FNode_1CS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FNode_1CS);
    SHADER_USE_PARAMETER_STRUCT(FNode_1CS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, output)
        SHADER_PARAMETER(int32, cb_ReadbackSequenceCSCB_frameIndex)
        SHADER_PARAMETER(FVector3f, cb_ReadbackSequenceCSCB__padding0)
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

IMPLEMENT_GLOBAL_SHADER(FNode_1CS, "/Engine/Private/ReadbackSequence/ReadbackSequence.usf", "csmain", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    FRDGTextureRef Texture_Output = params.inputs.Texture_Output;

    // Run Compute Shader "Node_1"
    {
        // Set shader parameters
        FNode_1CS::FParameters* PassParameters = GraphBuilder.AllocParameters<FNode_1CS::FParameters>();
        PassParameters->output = GraphBuilder.CreateUAV(Texture_Output);
        PassParameters->cb_ReadbackSequenceCSCB_frameIndex = View.ViewState->TechniqueState_ReadbackSequence.Var_frameIndex;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Output->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FNode_1CS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("ReadbackSequence.Node_1"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

}

};
