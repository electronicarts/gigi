#include "SubGraphTestTechnique.h"
#include "ScenePrivate.h"

namespace SubGraphTest
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "Swap_Colors"
class FSwap_ColorsCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSwap_ColorsCS);
    SHADER_USE_PARAMETER_STRUCT(FSwap_ColorsCS, FGlobalShader);

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

IMPLEMENT_GLOBAL_SHADER(FSwap_ColorsCS, "/Engine/Private/SubGraphTest/SubGraphTest_SwapColors.usf", "csmain", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    FRDGTextureRef Texture_Test = params.inputs.Texture_Test;

    // Transient texture Inner_Exported_Tex
    FRDGTextureRef Texture_Inner_Exported_Tex;
    {
        // Calculate size
        FIntVector textureSize = Texture_Test->Desc.GetSize();
        textureSize.X = ((textureSize.X + 0) * 1) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 1) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::UAV;

        EPixelFormat textureFormat = Texture_Test->Desc.Format;
        if (((uint64)Texture_Test->Desc.Flags & (uint64)ETextureCreateFlags::SRGB) != 0)
            createFlags |= ETextureCreateFlags::SRGB;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        Texture_Inner_Exported_Tex = GraphBuilder.CreateTexture(desc, TEXT("SubGraphTest.Inner_Exported_Tex"));
    }

    // Run Copy Resource "Inner_Copy_Import_To_Export"
    {
        // Copy from Test to Inner_Exported_Tex
        FRHICopyTextureInfo CopyTextureInfo;
        CopyTextureInfo.Size = FIntVector(Texture_Inner_Exported_Tex->Desc.GetSize().X, Texture_Inner_Exported_Tex->Desc.GetSize().Y, 0);
        AddCopyTexturePass(GraphBuilder, Texture_Test, Texture_Inner_Exported_Tex, CopyTextureInfo);
    }

    // Run Compute Shader "Swap_Colors"
    {
        // Set shader parameters
        FSwap_ColorsCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FSwap_ColorsCS::FParameters>();
        PassParameters->Output = GraphBuilder.CreateUAV(Texture_Inner_Exported_Tex);

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Test->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 8 - 1) / 8;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FSwap_ColorsCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("SubGraphTest.Swap_Colors"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    params.outputs.Texture_Inner_Exported_Tex = Texture_Inner_Exported_Tex;

}

};
