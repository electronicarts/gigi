#include "StructuredBufferTechnique.h"
#include "ScenePrivate.h"

namespace StructuredBuffer
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "csmain"
class FcsmainCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FcsmainCS);
    SHADER_USE_PARAMETER_STRUCT(FcsmainCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<TheStructure>, buff)
        SHADER_PARAMETER(float, cb_csmainCB_frameDeltaTime)
        SHADER_PARAMETER(int32, cb_csmainCB_frameIndex)
        SHADER_PARAMETER(FVector2f, cb_csmainCB__padding0)
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

IMPLEMENT_GLOBAL_SHADER(FcsmainCS, "/Engine/Private/StructuredBuffer/StructuredBuffer.usf", "csmain", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    FRDGBufferRef Buffer_buff = params.inputs.Buffer_buff;

    // Run Compute Shader "csmain"
    {
        // Set shader parameters
        FcsmainCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FcsmainCS::FParameters>();
        PassParameters->buff = GraphBuilder.CreateUAV(Buffer_buff, PF_Unknown);
        PassParameters->cb_csmainCB_frameDeltaTime = View.ViewState->TechniqueState_StructuredBuffer.Var_frameDeltaTime;
        PassParameters->cb_csmainCB_frameIndex = View.ViewState->TechniqueState_StructuredBuffer.Var_frameIndex;

        // Calculate dispatch size
        FIntVector dispatchSize = FIntVector(Buffer_buff->Desc.NumElements, 1, 1);
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FcsmainCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("StructuredBuffer.csmain"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

}

};
