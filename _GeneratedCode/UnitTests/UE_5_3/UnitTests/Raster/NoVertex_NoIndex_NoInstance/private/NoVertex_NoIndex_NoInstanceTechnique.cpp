#include "NoVertex_NoIndex_NoInstanceTechnique.h"
#include "ScenePrivate.h"

namespace NoVertex_NoIndex_NoInstance
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Draw Call "Rasterize" Vertex Shader
class FRasterizeVS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FRasterizeVS);
    SHADER_USE_PARAMETER_STRUCT(FRasterizeVS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(FMatrix44f, cb_VertexShaderCB_ViewProjMtx)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
    }
};

IMPLEMENT_GLOBAL_SHADER(FRasterizeVS, "/Engine/Private/NoVertex_NoIndex_NoInstance/NoVertex_NoIndex_NoInstance_VS.usf", "VSMain", SF_Vertex);

// Draw Call "Rasterize" Pixel Shader
class FRasterizePS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FRasterizePS);
    SHADER_USE_PARAMETER_STRUCT(FRasterizePS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
    }
};

IMPLEMENT_GLOBAL_SHADER(FRasterizePS, "/Engine/Private/NoVertex_NoIndex_NoInstance/NoVertex_NoIndex_NoInstance_PS.usf", "PSMain", SF_Pixel);

BEGIN_SHADER_PARAMETER_STRUCT(FRasterizeParameters, )
    SHADER_PARAMETER_STRUCT_INCLUDE(FRasterizeVS::FParameters, VSParameters)
    SHADER_PARAMETER_STRUCT_INCLUDE(FRasterizePS::FParameters, PSParameters)
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    // Transient texture Color_Buffer
    FRDGTextureRef Texture_Color_Buffer;
    {
        // Calculate size
        FIntVector textureSize = FIntVector(1,1,1);
        textureSize.X = ((textureSize.X + 0) * 512) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 512) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;

        EPixelFormat textureFormat = PF_R8G8B8A8;
        createFlags |= ETextureCreateFlags::SRGB;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        Texture_Color_Buffer = GraphBuilder.CreateTexture(desc, TEXT("NoVertex_NoIndex_NoInstance.Color_Buffer"));
    }

    // Transient texture Depth_Buffer
    FRDGTextureRef Texture_Depth_Buffer;
    {
        // Calculate size
        FIntVector textureSize = Texture_Color_Buffer->Desc.GetSize();
        textureSize.X = ((textureSize.X + 0) * 1) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 1) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;

        EPixelFormat textureFormat = PF_R32_FLOAT;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        Texture_Depth_Buffer = GraphBuilder.CreateTexture(desc, TEXT("NoVertex_NoIndex_NoInstance.Depth_Buffer"));
    }

    // Run Draw Call "Rasterize"
    {
        FRasterizeParameters* PassParameters = GraphBuilder.AllocParameters<FRasterizeParameters>();

        // Vertex Shader Parameters
        PassParameters->VSParameters.cb_VertexShaderCB_ViewProjMtx = View.ViewState->TechniqueState_NoVertex_NoIndex_NoInstance.Var_ViewProjMtx;

        const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());
        TShaderMapRef<FRasterizeVS> VertexShader(ShaderMap);
        TShaderMapRef<FRasterizePS> PixelShader(ShaderMap);

        AddClearRenderTargetPass(GraphBuilder, Texture_Color_Buffer, FLinearColor(0.200000f, 0.200000f, 0.200000f, 1.000000f ));

        AddClearDepthStencilPass(GraphBuilder, Texture_Depth_Buffer, true, 0.000000f, false, 0x00);

        PassParameters->RenderTargets[0] = FRenderTargetBinding(Texture_Color_Buffer, ERenderTargetLoadAction::ELoad);

        bool depthStencilIsStencil = IsStencilFormat(Texture_Depth_Buffer->Desc.Format);

        PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(
            Texture_Depth_Buffer,
            ERenderTargetLoadAction::ELoad,
            depthStencilIsStencil ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction,
            depthStencilIsStencil ? FExclusiveDepthStencil::DepthWrite_StencilWrite : FExclusiveDepthStencil::DepthWrite_StencilNop
        );

        const FRDGTextureDesc& viewportDesc = Texture_Depth_Buffer->Desc;
        FIntPoint viewportMax = FIntPoint(viewportDesc.Extent.X, viewportDesc.Extent.Y);

        GraphBuilder.AddPass(
            RDG_EVENT_NAME("NoVertex_NoIndex_NoInstance.Rasterize"),
            PassParameters,
            ERDGPassFlags::Raster,
            [VertexShader, PixelShader, PassParameters, viewportMax, depthStencilIsStencil, params](FRHICommandListImmediate& RHICmdList)
            {
                FGraphicsPipelineStateInitializer psoDesc;
                psoDesc.PrimitiveType = PT_TriangleList;
                psoDesc.NumSamples = 1;

                psoDesc.SubpassHint = ESubpassHint::None;
                psoDesc.SubpassIndex = 0;
                psoDesc.MultiViewCount = 1;
                psoDesc.bHasFragmentDensityAttachment = false;

                psoDesc.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
                psoDesc.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

                psoDesc.ConservativeRasterization = EConservativeRasterization::Disabled;

                // Color Target: Color_Buffer
                psoDesc.RenderTargetFlags[0] = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;
                psoDesc.RenderTargetFormats[0] = PF_R8G8B8A8;
                psoDesc.RenderTargetFlags[0] |= ETextureCreateFlags::SRGB;

                psoDesc.RenderTargetsEnabled = 1;

                // Depth Target: Depth_Buffer
                psoDesc.DepthStencilTargetFlag = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;
                psoDesc.DepthStencilTargetFormat = PF_R32_FLOAT;
                psoDesc.DepthTargetLoadAction = ERenderTargetLoadAction::ELoad;
                psoDesc.DepthTargetStoreAction = ERenderTargetStoreAction::EStore;
                psoDesc.StencilTargetLoadAction = depthStencilIsStencil ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction;
                psoDesc.StencilTargetStoreAction = depthStencilIsStencil ? ERenderTargetStoreAction::EStore : ERenderTargetStoreAction::ENoAction;

                // Depth Stencil State
                psoDesc.DepthStencilState = TStaticDepthStencilState<
                    true,
                    CF_Greater,
                    true,
                    CF_Always,
                    SO_Keep,
                    SO_Keep,
                    SO_Keep,
                    true,
                    CF_Always,
                    SO_Keep,
                    SO_Keep,
                    SO_Keep,
                    0xFF,
                    0xFF
                    >::GetRHI();

                // Blend State
                psoDesc.BlendState = TStaticBlendState<
                    (EColorWriteMask)(CW_RED | CW_GREEN | CW_BLUE), BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_Zero,
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
                    false
                    >::GetRHI();

                // Raster State
                psoDesc.RasterizerState = TStaticRasterizerState<
                    FM_Solid,
                    CM_CCW,
                    false,
                    false
                    >::GetRHI();

                psoDesc.BoundShaderState.VertexDeclarationRHI = GEmptyVertexDeclaration.VertexDeclarationRHI;

                SetGraphicsPipelineState(RHICmdList, psoDesc, 0);

                SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), PassParameters->VSParameters);
                SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters->PSParameters);

                RHICmdList.SetViewport(0, 0, 0, viewportMax.X, viewportMax.Y, 1);

                int vertexCountPerInstance = 6;

                int instanceCount = 5;

                RHICmdList.DrawPrimitive(0, vertexCountPerInstance / 3, instanceCount);
            }
        );
    }

    params.outputs.Texture_Color_Buffer = Texture_Color_Buffer;

    params.outputs.Texture_Depth_Buffer = Texture_Depth_Buffer;

}

};
