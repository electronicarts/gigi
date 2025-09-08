#include "StencilTechnique.h"
#include "ScenePrivate.h"

namespace Stencil
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

static FTextureRHIRef LoadedTexture__loadedTexture_0 = nullptr;

void Initialize(FInitParams& params)
{
    LoadedTexture__loadedTexture_0 = LoadObject<UTexture2D>(nullptr, *params.TexturePath__loadedTexture_0.ToString(), nullptr, LOAD_None, nullptr)->GetResource()->TextureRHI;
}

// Draw Call "Draw_1" Vertex Shader
class FDraw_1VS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FDraw_1VS);
    SHADER_USE_PARAMETER_STRUCT(FDraw_1VS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(FMatrix44f, cb_Draw1VSCB_ViewProjMtx)
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

IMPLEMENT_GLOBAL_SHADER(FDraw_1VS, "/Engine/Private/Stencil/StencilDraw1VS.usf", "Draw1VS", SF_Vertex);

// Draw Call "Draw_1" Pixel Shader
class FDraw_1PS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FDraw_1PS);
    SHADER_USE_PARAMETER_STRUCT(FDraw_1PS, FGlobalShader);

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

IMPLEMENT_GLOBAL_SHADER(FDraw_1PS, "/Engine/Private/Stencil/StencilDraw1PS.usf", "Draw1PS", SF_Pixel);

BEGIN_SHADER_PARAMETER_STRUCT(FDraw_1Parameters, )
    SHADER_PARAMETER_STRUCT_INCLUDE(FDraw_1VS::FParameters, VSParameters)
    SHADER_PARAMETER_STRUCT_INCLUDE(FDraw_1PS::FParameters, PSParameters)
    RENDER_TARGET_BINDING_SLOTS()
    RDG_BUFFER_ACCESS(Vertex_Buffer, ERHIAccess::VertexOrIndexBuffer)
END_SHADER_PARAMETER_STRUCT()

// Draw Call "Draw_2" Vertex Shader
class FDraw_2VS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FDraw_2VS);
    SHADER_USE_PARAMETER_STRUCT(FDraw_2VS, FGlobalShader);

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

IMPLEMENT_GLOBAL_SHADER(FDraw_2VS, "/Engine/Private/Stencil/StencilDraw2VS.usf", "Draw2VS", SF_Vertex);

// Draw Call "Draw_2" Pixel Shader
class FDraw_2PS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FDraw_2PS);
    SHADER_USE_PARAMETER_STRUCT(FDraw_2PS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, _loadedTexture_0)
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

IMPLEMENT_GLOBAL_SHADER(FDraw_2PS, "/Engine/Private/Stencil/StencilDraw2PS.usf", "Draw2PS", SF_Pixel);

BEGIN_SHADER_PARAMETER_STRUCT(FDraw_2Parameters, )
    SHADER_PARAMETER_STRUCT_INCLUDE(FDraw_2VS::FParameters, VSParameters)
    SHADER_PARAMETER_STRUCT_INCLUDE(FDraw_2PS::FParameters, PSParameters)
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    FRDGBufferRef Buffer_Vertex_Buffer = params.inputs.Buffer_Vertex_Buffer;

    // Transient texture Color
    FRDGTextureRef Texture_Color;
    {
        // Calculate size
        FIntVector textureSize = FIntVector(1,1,1);
        textureSize.X = ((textureSize.X + 0) * 128) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 128) / 1 + 0;
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
        Texture_Color = GraphBuilder.CreateTexture(desc, TEXT("Stencil.Color"));
    }

    // Persistent texture Depth_Stencil
    if (!View.ViewState->TechniqueState_Stencil.Texture_Depth_Stencil.IsValid())
    {
        // Calculate size
        FIntVector textureSize = Texture_Color->Desc.GetSize();
        textureSize.X = ((textureSize.X + 0) * 1) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 1) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;

        EPixelFormat textureFormat = PF_DepthStencil;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        FRDGTextureRef texture = GraphBuilder.CreateTexture(desc, TEXT("Stencil.Depth_Stencil"));
        View.ViewState->TechniqueState_Stencil.Texture_Depth_Stencil = GraphBuilder.ConvertToExternalTexture(texture).GetReference()->GetRHI();
    }
    FRDGTextureRef Texture_Depth_Stencil = RegisterExternalTexture(GraphBuilder, View.ViewState->TechniqueState_Stencil.Texture_Depth_Stencil, TEXT("Stencil.Depth_Stencil"));

    // Run Draw Call "Draw_1"
    {
        FDraw_1Parameters* PassParameters = GraphBuilder.AllocParameters<FDraw_1Parameters>();

        // Rasterization Buffers
        PassParameters->Vertex_Buffer = Buffer_Vertex_Buffer;

        // Vertex Shader Parameters
        PassParameters->VSParameters.cb_Draw1VSCB_ViewProjMtx = View.ViewState->TechniqueState_Stencil.Var_ViewProjMtx;

        const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());
        TShaderMapRef<FDraw_1VS> VertexShader(ShaderMap);
        TShaderMapRef<FDraw_1PS> PixelShader(ShaderMap);

        AddClearRenderTargetPass(GraphBuilder, Texture_Color, FLinearColor(0.000000f, 0.000000f, 0.000000f, 1.000000f ));

        AddClearDepthStencilPass(GraphBuilder, Texture_Depth_Stencil, true, 0.000000f, true, 0x00);

        PassParameters->RenderTargets[0] = FRenderTargetBinding(Texture_Color, ERenderTargetLoadAction::ELoad);

        bool depthStencilIsStencil = IsStencilFormat(Texture_Depth_Stencil->Desc.Format);

        PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(
            Texture_Depth_Stencil,
            ERenderTargetLoadAction::ELoad,
            depthStencilIsStencil ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction,
            depthStencilIsStencil ? FExclusiveDepthStencil::DepthWrite_StencilWrite : FExclusiveDepthStencil::DepthWrite_StencilNop
        );

        const FRDGTextureDesc& viewportDesc = Texture_Depth_Stencil->Desc;
        FIntPoint viewportMax = FIntPoint(viewportDesc.Extent.X, viewportDesc.Extent.Y);

        GraphBuilder.AddPass(
            RDG_EVENT_NAME("Stencil.Draw_1"),
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

                // Color Target: Color
                psoDesc.RenderTargetFlags[0] = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;
                psoDesc.RenderTargetFormats[0] = PF_R8G8B8A8;
                psoDesc.RenderTargetFlags[0] |= ETextureCreateFlags::SRGB;

                psoDesc.RenderTargetsEnabled = 1;

                // Depth Target: Depth_Stencil
                psoDesc.DepthStencilTargetFlag = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;
                psoDesc.DepthStencilTargetFormat = PF_DepthStencil;
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
                    SO_Replace,
                    true,
                    CF_Always,
                    SO_Keep,
                    SO_Keep,
                    SO_Replace,
                    0xFF,
                    0xFF
                    >::GetRHI();

                // Blend State
                psoDesc.BlendState = TStaticBlendState<
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
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
                    CM_None,
                    false,
                    false
                    >::GetRHI();

                // Vertex Declaration
                FVertexDeclarationElementList Elements;
                for (FVertexElement element : params.inputs.BufferVertexFormat_Vertex_Buffer)
                {
                    element.StreamIndex = 0;
                    element.bUseInstanceIndex = 0;
                    Elements.Add(element);
                }
                psoDesc.BoundShaderState.VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);

                SetGraphicsPipelineState(RHICmdList, psoDesc, 128);

                SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), PassParameters->VSParameters);
                SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters->PSParameters);

                RHICmdList.SetViewport(0, 0, 0, viewportMax.X, viewportMax.Y, 1);

                RHICmdList.SetStreamSource(0, PassParameters->Vertex_Buffer->GetRHI(), 0);
                int vertexCountPerInstance = PassParameters->Vertex_Buffer->Desc.NumElements;

                int instanceCount = 1;

                RHICmdList.DrawPrimitive(0, vertexCountPerInstance / 3, instanceCount);
            }
        );
    }

    // Loaded texture _loadedTexture_0
    FRDGTextureRef Texture__loadedTexture_0 = RegisterExternalTexture(GraphBuilder, LoadedTexture__loadedTexture_0, TEXT("Stencil._loadedTexture_0"));

    // Run Draw Call "Draw_2"
    {
        FDraw_2Parameters* PassParameters = GraphBuilder.AllocParameters<FDraw_2Parameters>();

        // Pixel Shader Parameters
        PassParameters->PSParameters._loadedTexture_0 = Texture__loadedTexture_0;

        const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());
        TShaderMapRef<FDraw_2VS> VertexShader(ShaderMap);
        TShaderMapRef<FDraw_2PS> PixelShader(ShaderMap);

        PassParameters->RenderTargets[0] = FRenderTargetBinding(Texture_Color, ERenderTargetLoadAction::ELoad);

        bool depthStencilIsStencil = IsStencilFormat(Texture_Depth_Stencil->Desc.Format);

        PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(
            Texture_Depth_Stencil,
            ERenderTargetLoadAction::ELoad,
            depthStencilIsStencil ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction,
            depthStencilIsStencil ? FExclusiveDepthStencil::DepthRead_StencilWrite : FExclusiveDepthStencil::DepthRead_StencilNop
        );

        const FRDGTextureDesc& viewportDesc = Texture_Depth_Stencil->Desc;
        FIntPoint viewportMax = FIntPoint(viewportDesc.Extent.X, viewportDesc.Extent.Y);

        GraphBuilder.AddPass(
            RDG_EVENT_NAME("Stencil.Draw_2"),
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

                // Color Target: Color
                psoDesc.RenderTargetFlags[0] = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;
                psoDesc.RenderTargetFormats[0] = PF_R8G8B8A8;
                psoDesc.RenderTargetFlags[0] |= ETextureCreateFlags::SRGB;

                psoDesc.RenderTargetsEnabled = 1;

                // Depth Target: Depth_Stencil
                psoDesc.DepthStencilTargetFlag = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;
                psoDesc.DepthStencilTargetFormat = PF_DepthStencil;
                psoDesc.DepthTargetLoadAction = ERenderTargetLoadAction::ELoad;
                psoDesc.DepthTargetStoreAction = ERenderTargetStoreAction::EStore;
                psoDesc.StencilTargetLoadAction = depthStencilIsStencil ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction;
                psoDesc.StencilTargetStoreAction = depthStencilIsStencil ? ERenderTargetStoreAction::EStore : ERenderTargetStoreAction::ENoAction;

                // Depth Stencil State
                psoDesc.DepthStencilState = TStaticDepthStencilState<
                    false,
                    CF_Always,
                    true,
                    CF_Equal,
                    SO_Keep,
                    SO_Keep,
                    SO_Keep,
                    true,
                    CF_Equal,
                    SO_Keep,
                    SO_Keep,
                    SO_Keep,
                    0xFF,
                    0xFF
                    >::GetRHI();

                // Blend State
                psoDesc.BlendState = TStaticBlendState<
                    CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,
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
                    CM_None,
                    false,
                    false
                    >::GetRHI();

                psoDesc.BoundShaderState.VertexDeclarationRHI = GEmptyVertexDeclaration.VertexDeclarationRHI;

                SetGraphicsPipelineState(RHICmdList, psoDesc, 128);

                SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), PassParameters->VSParameters);
                SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters->PSParameters);

                RHICmdList.SetViewport(0, 0, 0, viewportMax.X, viewportMax.Y, 1);

                int vertexCountPerInstance = 4;

                int instanceCount = 1;

                RHICmdList.DrawPrimitive(0, vertexCountPerInstance / 3, instanceCount);
            }
        );
    }

    params.outputs.Texture_Color = Texture_Color;

    params.outputs.Texture_Depth_Stencil = Texture_Depth_Stencil;

}

};
