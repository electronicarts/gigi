Struct_Materials Material_MaterialBuffer(StructuredBuffer<Struct_Materials> MaterialsBuffer, int materialID, float2 uv0, float2 uv1, float2 uv2, float2 uv3, out float3 normal, out float occlusion,
    float2 uv0ddx = float2(0.0f, 0.0f), float2 uv0ddy = float2(0.0f, 0.0f), float2 uv1ddx = float2(0.0f, 0.0f), float2 uv1ddy = float2(0.0f, 0.0f),
    float2 uv2ddx = float2(0.0f, 0.0f), float2 uv2ddy = float2(0.0f, 0.0f), float2 uv3ddx = float2(0.0f, 0.0f), float2 uv3ddy = float2(0.0f, 0.0f))
{
    Struct_Materials ret = (Struct_Materials)0;
    ret.baseColor = float4(1.f, 1.f, 1.f, 0.f);
    ret.emissive = float3(0.f, 0.f, 0.f);
    ret.metallic = float(0.f);
    ret.roughness = float(1.f);
    ret.alphaMode = int(0);
    ret.alphaCutoff = float(0.f);
    ret.doubleSided = int(1);

    normal = float3(0.0f, 0.0f, 1.0f);
    occlusion = 1.0f;

    float normalScale = 1.0f;
    float occlusionStrength = 1.0f;

    uint materialCount, materialStride;
    MaterialsBuffer.GetDimensions(materialCount, materialStride);

    if (materialID < 0 || materialID >= materialCount)
        return ret;

    // Read material data from the buffer
    ret = MaterialsBuffer[materialID];

    normal = normalize((normal * 2.0f - 1.0f) * float3(normalScale, normalScale, 1.0f));
    ret.baseColor.rgb = lerp(ret.baseColor.rgb, ret.baseColor.rgb * occlusion, occlusionStrength);

    return ret;
};
