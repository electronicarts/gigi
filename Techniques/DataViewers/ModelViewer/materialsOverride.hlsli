
/*
struct Struct_Materials
{
    float4 baseColor;
    float3 emissive;
    float metallic;
    float roughness;
    int alphaMode;
    float alphaCutoff;
    int doubleSided;
};
*/

void ApplyMaterialOverride(inout Struct_Materials material)
{
    // Optionally modify the material here.
    if (/*$(Variable:MatBaseColorOverride)*/)
        material.baseColor = /*$(Variable:MatBaseColor)*/;

    if (/*$(Variable:MatEmissiveOverride)*/)
        material.emissive = /*$(Variable:MatEmissive)*/;

    if (/*$(Variable:MatMetallicOverride)*/)
        material.metallic = /*$(Variable:MatMetallic)*/;

    if (/*$(Variable:MatRoughnessOverride)*/)
        material.roughness = /*$(Variable:MatRoughness)*/;

    if (/*$(Variable:MatAlphaModeOverride)*/)
        material.alphaMode = /*$(Variable:MatAlphaMode)*/;

    if (/*$(Variable:MatAlphaCutoffOverride)*/)
        material.alphaCutoff = /*$(Variable:MatAlphaCutoff)*/;

    if (/*$(Variable:MatDoubleSidedOverride)*/)
        material.doubleSided = /*$(Variable:MatDoubleSided)*/;
}