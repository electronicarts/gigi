#include "StridesTechnique.h"
#include "ScenePrivate.h"

namespace Strides
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

// Compute Shader "Initialize"
class FInitializeCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FInitializeCS);
    SHADER_USE_PARAMETER_STRUCT(FInitializeCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWByteAddressBuffer, buff)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWByteAddressBuffer, buff2)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
        OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);

        OutEnvironment.SetDefine(TEXT("__GigiDispatchMultiply"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchDivide"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPreAdd"), TEXT("uint3(0,0,0)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPostAdd"), TEXT("uint3(0,0,0)"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FInitializeCS, "/Engine/Private/Strides/Strides_Init.usf", "main", SF_Compute);

// Compute Shader "Float4RW"
class FFloat4RWCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FFloat4RWCS);
    SHADER_USE_PARAMETER_STRUCT(FFloat4RWCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float4>, buff)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float4>, buff2)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
        OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);

        OutEnvironment.SetDefine(TEXT("__GigiDispatchMultiply"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchDivide"), TEXT("uint3(4,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPreAdd"), TEXT("uint3(0,0,0)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPostAdd"), TEXT("uint3(0,0,0)"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FFloat4RWCS, "/Engine/Private/Strides/Strides_Float4RW.usf", "main", SF_Compute);

// Compute Shader "Float16RW"
class FFloat16RWCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FFloat16RWCS);
    SHADER_USE_PARAMETER_STRUCT(FFloat16RWCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<half>, buff)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<half>, buff2)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
        OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);

        OutEnvironment.SetDefine(TEXT("__GigiDispatchMultiply"), TEXT("uint3(2,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchDivide"), TEXT("uint3(1,1,1)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPreAdd"), TEXT("uint3(0,0,0)"));
        OutEnvironment.SetDefine(TEXT("__GigiDispatchPostAdd"), TEXT("uint3(0,0,0)"));
    }
};

IMPLEMENT_GLOBAL_SHADER(FFloat16RWCS, "/Engine/Private/Strides/Strides_Float16RW.usf", "main", SF_Compute);

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    // Buffer TheBuffer
    FRDGBufferRef Buffer_TheBuffer;
    {
        // Calculate size
        uint32 NumElements = 1;
        NumElements = ((NumElements + 0) * 32) / 1 + 0;

        uint32 BytesPerElement = 4;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static | EBufferUsageFlags::UnorderedAccess | EBufferUsageFlags::ShaderResource | EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::ByteAddressBuffer;

        // Create Buffer
        Buffer_TheBuffer = GraphBuilder.CreateBuffer(desc, TEXT("Strides.TheBuffer"));
    }

    // Buffer TheBuffer2
    FRDGBufferRef Buffer_TheBuffer2;
    {
        // Calculate size
        uint32 NumElements = Buffer_TheBuffer->Desc.NumElements;
        NumElements = ((NumElements + 0) * 1) / 1 + 0;

        uint32 BytesPerElement = Buffer_TheBuffer->Desc.BytesPerElement;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static | EBufferUsageFlags::UnorderedAccess | EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::ByteAddressBuffer;

        // Create Buffer
        Buffer_TheBuffer2 = GraphBuilder.CreateBuffer(desc, TEXT("Strides.TheBuffer2"));
    }

    // Run Compute Shader "Initialize"
    {
        // Set shader parameters
        FInitializeCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FInitializeCS::FParameters>();
        PassParameters->buff = GraphBuilder.CreateUAV(Buffer_TheBuffer, PF_R32_FLOAT);
        PassParameters->buff2 = GraphBuilder.CreateUAV(Buffer_TheBuffer2, PF_R32_FLOAT);

        // Calculate dispatch size
        FIntVector dispatchSize = FIntVector(Buffer_TheBuffer->Desc.NumElements, 1, 1);
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 1 + 0 + 64 - 1) / 64;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FInitializeCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("Strides.Initialize"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "Float4RW"
    {
        // Set shader parameters
        FFloat4RWCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FFloat4RWCS::FParameters>();
        PassParameters->buff = GraphBuilder.CreateSRV(Buffer_TheBuffer, PF_R32_FLOAT);
        PassParameters->buff2 = GraphBuilder.CreateUAV(Buffer_TheBuffer2, PF_R32_FLOAT);

        // Calculate dispatch size
        FIntVector dispatchSize = FIntVector(Buffer_TheBuffer->Desc.NumElements, 1, 1);
        dispatchSize.X = (((dispatchSize.X + 0) * 1) / 4 + 0 + 64 - 1) / 64;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FFloat4RWCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("Strides.Float4RW"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

    // Run Compute Shader "Float16RW"
    {
        // Set shader parameters
        FFloat16RWCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FFloat16RWCS::FParameters>();
        PassParameters->buff = GraphBuilder.CreateSRV(Buffer_TheBuffer, PF_R32_FLOAT);
        PassParameters->buff2 = GraphBuilder.CreateUAV(Buffer_TheBuffer2, PF_R32_FLOAT);

        // Calculate dispatch size
        FIntVector dispatchSize = FIntVector(Buffer_TheBuffer->Desc.NumElements, 1, 1);
        dispatchSize.X = (((dispatchSize.X + 0) * 2) / 1 + 0 + 64 - 1) / 64;
        dispatchSize.Y = (((dispatchSize.Y + 0) * 1) / 1 + 0 + 1 - 1) / 1;
        dispatchSize.Z = (((dispatchSize.Z + 0) * 1) / 1 + 0 + 1 - 1) / 1;

        // Execute shader
        TShaderMapRef<FFloat16RWCS> ComputeShader(View.ShaderMap);
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("Strides.Float16RW"),
            ComputeShader,
            PassParameters,
            dispatchSize
        );
    }

}

};
