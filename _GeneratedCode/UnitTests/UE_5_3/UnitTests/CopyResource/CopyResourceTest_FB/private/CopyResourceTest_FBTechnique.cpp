#include "CopyResourceTest_FBTechnique.h"
#include "ScenePrivate.h"

namespace CopyResourceTest_FB
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

    FRDGTextureRef Texture_Source_Texture = params.inputs.Texture_Source_Texture;

    // Transient texture Texture_From_Texture
    FRDGTextureRef Texture_Texture_From_Texture;
    {
        // Calculate size
        FIntVector textureSize = Texture_Source_Texture->Desc.GetSize();
        textureSize.X = ((textureSize.X + 0) * 1) / 1 + 0;
        textureSize.Y = ((textureSize.Y + 0) * 1) / 1 + 0;
        textureSize.Z = ((textureSize.Z + 0) * 1) / 1 + 0;

        // Make Desc
        ETextureCreateFlags createFlags = ETextureCreateFlags::UAV;

        EPixelFormat textureFormat = Texture_Source_Texture->Desc.Format;
        if (((uint64)Texture_Source_Texture->Desc.Flags & (uint64)ETextureCreateFlags::SRGB) != 0)
            createFlags |= ETextureCreateFlags::SRGB;

        FRDGTextureDesc desc = FRDGTextureDesc::Create2D(
            FIntPoint(textureSize.X, textureSize.Y),
            textureFormat,
            FClearValueBinding::None,
            createFlags
        );

        // Create Texture
        Texture_Texture_From_Texture = GraphBuilder.CreateTexture(desc, TEXT("CopyResourceTest_FB.Texture_From_Texture"));
    }

    FRDGBufferRef Buffer_Source_Buffer = params.inputs.Buffer_Source_Buffer;

    // Buffer Buffer_From_Buffer
    FRDGBufferRef Buffer_Buffer_From_Buffer;
    {
        // Calculate size
        uint32 NumElements = Buffer_Source_Buffer->Desc.NumElements;
        NumElements = ((NumElements + 0) * 1) / 1 + 0;

        uint32 BytesPerElement = Buffer_Source_Buffer->Desc.BytesPerElement;

        FRDGBufferDesc desc;
        desc.BytesPerElement = BytesPerElement;
        desc.NumElements = NumElements;
        desc.Usage = EBufferUsageFlags::Static;

        // Create Buffer
        Buffer_Buffer_From_Buffer = GraphBuilder.CreateBuffer(desc, TEXT("CopyResourceTest_FB.Buffer_From_Buffer"));
    }

    // Run Copy Resource "Copy_Texture_To_Texture"
    {
        // Copy from Source_Texture to Texture_From_Texture
        FRHICopyTextureInfo CopyTextureInfo;
        CopyTextureInfo.Size = FIntVector(Texture_Texture_From_Texture->Desc.GetSize().X, Texture_Texture_From_Texture->Desc.GetSize().Y, 0);
        AddCopyTexturePass(GraphBuilder, Texture_Source_Texture, Texture_Texture_From_Texture, CopyTextureInfo);
    }

    // Run Copy Resource "Copy_Buffer_To_Buffer"
    {
        // Copy from Source_Buffer to Buffer_From_Buffer
        AddCopyBufferPass(GraphBuilder, Buffer_Source_Buffer, Buffer_Buffer_From_Buffer);
    }

    params.outputs.Texture_Texture_From_Texture = Texture_Texture_From_Texture;

    params.outputs.Buffer_Buffer_From_Buffer = Buffer_Buffer_From_Buffer;

}

};
