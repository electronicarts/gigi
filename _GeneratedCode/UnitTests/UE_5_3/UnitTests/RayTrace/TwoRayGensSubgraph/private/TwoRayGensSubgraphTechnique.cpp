#include "TwoRayGensSubgraphTechnique.h"
#include "ScenePrivate.h"

namespace TwoRayGensSubgraph
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Ray Shader "A_DoRT1" Ray Gen
class FA_DoRT1RG : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FA_DoRT1RG);
    SHADER_USE_ROOT_PARAMETER_STRUCT(FA_DoRT1RG, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, g_texture)
        SHADER_PARAMETER_RDG_BUFFER_SRV(RaytracingAccelerationStructure, g_scene)
        SHADER_PARAMETER(FVector3f, cb_A_TwoRayGens1CB_cameraPos)
        SHADER_PARAMETER(float, cb_A_TwoRayGens1CB__padding0)
        SHADER_PARAMETER(FMatrix44f, cb_A_TwoRayGens1CB_clipToWorld)
        SHADER_PARAMETER(float, cb_A_TwoRayGens1CB_depthNearPlane)
        SHADER_PARAMETER(FVector3f, cb_A_TwoRayGens1CB__padding1)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }

    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
    {
        return ERayTracingPayloadType::Default;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
    }
};

IMPLEMENT_GLOBAL_SHADER(FA_DoRT1RG, "/Engine/Private/TwoRayGensSubgraph/TwoRayGensSubgraphA_A/TwoRayGens1.usf", "RayGen1", SF_RayGen);

// Ray Shader "B_DoRT2" Ray Gen
class FB_DoRT2RG : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FB_DoRT2RG);
    SHADER_USE_ROOT_PARAMETER_STRUCT(FB_DoRT2RG, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, g_texture)
        SHADER_PARAMETER_RDG_BUFFER_SRV(RaytracingAccelerationStructure, g_scene)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, g_blueChannel)
        SHADER_PARAMETER(FVector3f, cb_B_TwoRayGens2CB_cameraPos)
        SHADER_PARAMETER(float, cb_B_TwoRayGens2CB__padding0)
        SHADER_PARAMETER(FMatrix44f, cb_B_TwoRayGens2CB_clipToWorld)
        SHADER_PARAMETER(float, cb_B_TwoRayGens2CB_depthNearPlane)
        SHADER_PARAMETER(FVector3f, cb_B_TwoRayGens2CB__padding1)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }

    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
    {
        return ERayTracingPayloadType::Default;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
    }
};

IMPLEMENT_GLOBAL_SHADER(FB_DoRT2RG, "/Engine/Private/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.usf", "RayGen2", SF_RayGen);

// Miss Shader "A_TwoRayGensMiss1"
class FA_TwoRayGensMiss1MS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FA_TwoRayGensMiss1MS);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }

    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
    {
        return ERayTracingPayloadType::Default;
    }
};

IMPLEMENT_GLOBAL_SHADER(FA_TwoRayGensMiss1MS, "/Engine/Private/TwoRayGensSubgraph/TwoRayGensSubgraphA_A/TwoRayGens1.usf", "Miss1", SF_RayMiss);

// Miss Shader "B_TwoRayGensMiss2A"
class FB_TwoRayGensMiss2AMS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FB_TwoRayGensMiss2AMS);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }

    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
    {
        return ERayTracingPayloadType::Default;
    }
};

IMPLEMENT_GLOBAL_SHADER(FB_TwoRayGensMiss2AMS, "/Engine/Private/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.usf", "Miss2A", SF_RayMiss);

// Miss Shader "B_TwoRayGensMiss2B"
class FB_TwoRayGensMiss2BMS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FB_TwoRayGensMiss2BMS);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }

    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
    {
        return ERayTracingPayloadType::Default;
    }
};

IMPLEMENT_GLOBAL_SHADER(FB_TwoRayGensMiss2BMS, "/Engine/Private/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.usf", "Miss2B", SF_RayMiss);

// Hit Group "A_HitGroup1"
class FA_HitGroup1HG : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FA_HitGroup1HG);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }

    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
    {
        return ERayTracingPayloadType::Default;
    }
};

IMPLEMENT_GLOBAL_SHADER(FA_HitGroup1HG, "/Engine/Private/TwoRayGensSubgraph/TwoRayGensSubgraphA_A/TwoRayGens1.usf", "closesthit=ClosestHit1", SF_RayHitGroup);

// Hit Group "B_HitGroup2"
class FB_HitGroup2HG : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FB_HitGroup2HG);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }

    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
    {
        return ERayTracingPayloadType::Default;
    }
};

IMPLEMENT_GLOBAL_SHADER(FB_HitGroup2HG, "/Engine/Private/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.usf", "closesthit=ClosestHit2", SF_RayHitGroup);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    // Transient texture Texture - The texture that is rendered to
    FRDGTextureRef Texture_Texture;
    {
        // Calculate size
        FIntVector textureSize = FIntVector(1,1,1);
        textureSize.X = ((textureSize.X + 0) * 256) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 256) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::UAV;

        EPixelFormat textureFormat = PF_R8G8B8A8;
        createFlags |= ETextureCreateFlags::SRGB;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        Texture_Texture = GraphBuilder.CreateTexture(desc, TEXT("TwoRayGensSubgraph.Texture"));
    }

    FRDGBufferRef Buffer_Scene = params.inputs.Buffer_Scene;

    FRDGTextureRef Texture_BlueChannel = params.inputs.Texture_BlueChannel;

    // Run Ray Shader "A_DoRT1"
    {
        // Set shader parameters
        FA_DoRT1RG::FParameters* PassParameters = GraphBuilder.AllocParameters<FA_DoRT1RG::FParameters>();
        PassParameters->g_texture = GraphBuilder.CreateUAV(Texture_Texture);
        PassParameters->cb_A_TwoRayGens1CB_cameraPos = View.ViewState->TechniqueState_TwoRayGensSubgraph.Var_cameraPos;
        PassParameters->cb_A_TwoRayGens1CB_clipToWorld = View.ViewState->TechniqueState_TwoRayGensSubgraph.Var_clipToWorld;
        PassParameters->cb_A_TwoRayGens1CB_depthNearPlane = View.ViewState->TechniqueState_TwoRayGensSubgraph.Var_depthNearPlane;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Texture->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0);
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0);
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0);

        // Execute shader
        TShaderMapRef<FA_DoRT1RG> RayGenerationShader(View.ShaderMap);
        GraphBuilder.AddPass(
            RDG_EVENT_NAME("TwoRayGensSubgraph.A_DoRT1"),
            PassParameters,
            ERDGPassFlags::Compute,
            [PassParameters, RayGenerationShader, dispatchSize](FRHIRayTracingCommandList& RHICmdList)
        {
        });
    }

    // Run Ray Shader "B_DoRT2"
    {
        // Set shader parameters
        FB_DoRT2RG::FParameters* PassParameters = GraphBuilder.AllocParameters<FB_DoRT2RG::FParameters>();
        PassParameters->g_texture = GraphBuilder.CreateUAV(Texture_Texture);
        PassParameters->g_blueChannel = Texture_BlueChannel;
        PassParameters->cb_B_TwoRayGens2CB_cameraPos = View.ViewState->TechniqueState_TwoRayGensSubgraph.Var_cameraPos;
        PassParameters->cb_B_TwoRayGens2CB_clipToWorld = View.ViewState->TechniqueState_TwoRayGensSubgraph.Var_clipToWorld;
        PassParameters->cb_B_TwoRayGens2CB_depthNearPlane = View.ViewState->TechniqueState_TwoRayGensSubgraph.Var_depthNearPlane;

        // Calculate dispatch size
        FIntVector dispatchSize = Texture_Texture->Desc.GetSize();
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0);
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0);
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0);

        // Execute shader
        TShaderMapRef<FB_DoRT2RG> RayGenerationShader(View.ShaderMap);
        GraphBuilder.AddPass(
            RDG_EVENT_NAME("TwoRayGensSubgraph.B_DoRT2"),
            PassParameters,
            ERDGPassFlags::Compute,
            [PassParameters, RayGenerationShader, dispatchSize](FRHIRayTracingCommandList& RHICmdList)
        {
        });
    }

    params.outputs.Texture_Texture = Texture_Texture;

}

};
