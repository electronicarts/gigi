///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

ENUM_BEGIN(TextureFormat, "")
    ENUM_ITEM(Any, "Only valid for imported textures.")

    // 8 bit
    ENUM_ITEM(R8_Unorm, "R 8 bit unorm")
    ENUM_ITEM(RG8_Unorm, "RG 8 bit unorm")
    ENUM_ITEM(RGBA8_Unorm, "RGBA 8 bit unorm")
    ENUM_ITEM(RGBA8_Unorm_sRGB, "RGBA 8 bit unorm, sRGB")

    ENUM_ITEM(R8_Snorm, "R 8 bit snorm")
    ENUM_ITEM(RG8_Snorm, "RG 8 bit snorm")
    ENUM_ITEM(RGBA8_Snorm, "RGBA 8 bit snorm")

    ENUM_ITEM(R8_Uint, "R 8 bit uint")
    ENUM_ITEM(RG8_Uint, "RG 8 bit uint")
    ENUM_ITEM(RGBA8_Uint, "RGBA 8 bit uint")

    ENUM_ITEM(R8_Sint, "R 8 bit sint")
    ENUM_ITEM(RG8_Sint, "RG 8 bit sint")
    ENUM_ITEM(RGBA8_Sint, "RGBA 8 bit sint")

    // 16 bit
    ENUM_ITEM(R16_Float, "R 16 bit float")
    ENUM_ITEM(RG16_Float, "RG 16 bit float")
    ENUM_ITEM(RGBA16_Float, "RGBA 16 bit float")
    ENUM_ITEM(RGBA16_Unorm, "RGBA 16 bit unorm")
    ENUM_ITEM(RGBA16_Snorm, "RGBA 16 bit snorm")

    // 32 bit float
    ENUM_ITEM(R32_Float, "R 32 bit float")
    ENUM_ITEM(RG32_Float, "RG 32 bit float")
    ENUM_ITEM(RGBA32_Float, "RGBA 32 bit float")

    // 32 bit uint
    ENUM_ITEM(R32_Uint, "R 32 bit uint")
    ENUM_ITEM(RGBA32_Uint, "RGBA 32 bit uint")

    // Other
    ENUM_ITEM(R11G11B10_Float, "RGB 32 bit (total) float")

    // Depth formats
    ENUM_ITEM(D32_Float, "32 bit float depth")
    ENUM_ITEM(D16_Unorm, "16 bit unorm depth")

    // Depth stencil formats
    ENUM_ITEM(D32_Float_S8, "32 bit depth, 8 bit stencil, and 24 more bits unused")
    ENUM_ITEM(D24_Unorm_S8, "24 bit depth, 8 bit stencil")

    // Block compressed formats
    ENUM_ITEM(BC4_Unorm, "R 8 bit unorm. block compressed.")
    ENUM_ITEM(BC4_Snorm, "R 8 bit snorm. block compressed.")
    ENUM_ITEM(BC5_Unorm, "RG 8 bit unorm. block compressed.")
    ENUM_ITEM(BC5_Snorm, "RG 8 bit snorm. block compressed.")

    ENUM_ITEM(BC7_Unorm, "RGB, alpha optional. block compressed.")
    ENUM_ITEM(BC7_Unorm_sRGB, "RGB, alpha optional. sRGB. block compressed.")

    ENUM_ITEM(BC6_UF16, "RGB, 16 bit float unsigned. block compressed.")
    ENUM_ITEM(BC6_SF16, "RGB, 16 bit float signed. block compressed.")
ENUM_END()