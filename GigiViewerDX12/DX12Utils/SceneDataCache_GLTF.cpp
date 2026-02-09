///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2026 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// GLTF Specs https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html

#include "SceneDataCache.h"

// Don't have tinygltf do anything with textures. We will handle it ourselves.
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#define TINYGLTF_IMPLEMENTATION
#include "../external/tinygltf/tiny_gltf.h"

#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>

#include <filesystem>

static SamplerAddressMode TINYGLTF_TEXTURE_WRAP_To_SamplerAddressMode(unsigned int wrap)
{
    switch (wrap)
    {
        case TINYGLTF_TEXTURE_WRAP_REPEAT: return SamplerAddressMode::Wrap;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: return SamplerAddressMode::Clamp;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return SamplerAddressMode::Mirror;
    }
    return SamplerAddressMode::Wrap;
}

static SamplerFilter TINYGLTF_TEXTURE_FILTER_To_SamplerFilter(int minFilter, int magFilter)
{
    bool minLinear = false;
    bool mipLinear = false;
    bool magLinear = false;

    // min and mip
    switch (minFilter)
    {
        case TINYGLTF_TEXTURE_FILTER_NEAREST: minLinear = mipLinear = false; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR: minLinear = mipLinear = true; break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: minLinear = false; mipLinear = false; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: minLinear = true; mipLinear = false; break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: minLinear = false; mipLinear = true; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: minLinear = true; mipLinear = true; break;
    }

    // mag can either be TINYGLTF_TEXTURE_FILTER_LINEAR or TINYGLTF_TEXTURE_FILTER_NEAREST
    magLinear = (magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR);
    
    return SamplerFilter(
        int(SamplerFilter::MinMagMipPoint) +
        ((mipLinear) ? 1 : 0) +
        ((magLinear) ? 2 : 0) +
        ((minLinear) ? 4 : 0));
}

namespace
{
    struct BufferData
    {
        const unsigned char* cursor = nullptr;
        size_t cursorCount = 0;

        const tinygltf::Accessor* accessor = nullptr;
    };

    template <typename T>
    static bool ReadBufferDataScalar(BufferData& bd, size_t index, T& data)
    {
        if (bd.cursorCount == 0 || index >= bd.cursorCount)
            return false;

        switch (bd.accessor->componentType)
        {
            case TINYGLTF_COMPONENT_TYPE_BYTE: data = static_cast<T>(((int8_t*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: data = static_cast<T>(((uint8_t*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_SHORT: data = static_cast<T>(((int16_t*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: data = static_cast<T>(((uint16_t*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_INT: data = static_cast<T>(((int32_t*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: data = static_cast<T>(((uint32_t*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_FLOAT: data = static_cast<T>(((float*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_DOUBLE: data = static_cast<T>(((double*)bd.cursor)[index]); break;
            default: return false;
        }
        return true;
    }

    // Float specialization assumes unorm for the integer types
    template <>
    static bool ReadBufferDataScalar<float>(BufferData& bd, size_t index, float& data)
    {
        if (bd.cursorCount == 0 || index >= bd.cursorCount)
            return false;

        switch (bd.accessor->componentType)
        {
            case TINYGLTF_COMPONENT_TYPE_BYTE: data = static_cast<float>(((int8_t*)bd.cursor)[index]) / 127.0f; break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: data = static_cast<float>(((uint8_t*)bd.cursor)[index]) / 255.0f; break;
            case TINYGLTF_COMPONENT_TYPE_SHORT: data = static_cast<float>(((int16_t*)bd.cursor)[index]) / 32767.0f; break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: data = static_cast<float>(((uint16_t*)bd.cursor)[index]) / 65535.0f; break;
            case TINYGLTF_COMPONENT_TYPE_INT: data = static_cast<float>(((int32_t*)bd.cursor)[index]) / 2147483647.0f; break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: data = static_cast<float>(((uint32_t*)bd.cursor)[index]) / 4294967295.0f; break;
            case TINYGLTF_COMPONENT_TYPE_FLOAT: data = static_cast<float>(((float*)bd.cursor)[index]); break;
            case TINYGLTF_COMPONENT_TYPE_DOUBLE: data = static_cast<float> (((double*)bd.cursor)[index]); break;
            default: return false;
        }
        return true;
    }

    template <typename T>
    static bool ReadBufferDataVector(BufferData& bd, size_t count, size_t index, T* data)
    {
        for (size_t i = 0; i < count; ++i)
        {
            if (!ReadBufferDataScalar(bd, index * count + i, data[i]))
                return false;
        }
        return true;
    };
}

bool SceneDataCache::LoadGLTF(FileCache::File& fileData, SceneData& sceneData)
{
    // Load the data
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::filesystem::path filePath = std::filesystem::weakly_canonical(fileData.GetFileName()).replace_filename("");

    if (std::filesystem::weakly_canonical(fileData.GetFileName()).extension() == ".glb")
    {
        // this code path requires an image loader, despite having TINYGLTF_NO_EXTERNAL_IMAGE set,
        // because images may be embedded in the .glb file.
        loader.SetImageLoader(
            [](tinygltf::Image* image, const int image_idx, std::string* err,
                std::string* warn, int req_width, int req_height,
                const unsigned char* bytes, int size, void* user_data)
            {
                return true;
            },
            this
        );

        if (!loader.LoadBinaryFromMemory(&model, &sceneData.error, &sceneData.warn, (const unsigned char*)fileData.GetBytes(), (unsigned int)fileData.GetSize(), filePath.string().c_str()))
            return false;
    }
    else
    {
        if (!loader.LoadASCIIFromString(&model, &sceneData.error, &sceneData.warn, fileData.GetBytes(), (unsigned int)fileData.GetSize(), filePath.string().c_str()))
            return false;
    }

    // Gather the materials
    {
        auto GetTextureDetails = [&model](auto& textureDetail, SceneData::Material::Texture& destTexture)
            {
                if (textureDetail.index < 0)
                    return;

                const tinygltf::Texture& srcTexture = model.textures[textureDetail.index];
                if (srcTexture.source < 0)
                    return;

                destTexture.fileName = model.images[srcTexture.source].uri;
                destTexture.texCoordIndex = textureDetail.texCoord;

                if (srcTexture.sampler >= 0)
                {
                    const tinygltf::Sampler& srcSampler = model.samplers[srcTexture.sampler];
                    destTexture.filter = TINYGLTF_TEXTURE_FILTER_To_SamplerFilter(srcSampler.minFilter, srcSampler.magFilter);
                    destTexture.wrapS = TINYGLTF_TEXTURE_WRAP_To_SamplerAddressMode(srcSampler.wrapS);
                    destTexture.wrapT = TINYGLTF_TEXTURE_WRAP_To_SamplerAddressMode(srcSampler.wrapT);
                }
            }
        ;

        sceneData.materials.resize(model.materials.size());
        for (size_t materialIndex = 0; materialIndex < model.materials.size(); ++materialIndex)
        {
            const tinygltf::Material& srcMaterial = model.materials[materialIndex];
            SceneData::Material& destMaterial = sceneData.materials[materialIndex];

            destMaterial.name = srcMaterial.name;

            destMaterial.emissiveFactor[0] = (float)srcMaterial.emissiveFactor[0];
            destMaterial.emissiveFactor[1] = (float)srcMaterial.emissiveFactor[1];
            destMaterial.emissiveFactor[2] = (float)srcMaterial.emissiveFactor[2];

            GetTextureDetails(srcMaterial.emissiveTexture, destMaterial.emissiveTexture);
            destMaterial.emissiveTexture.channels = ".rgb";

            if (!_stricmp(srcMaterial.alphaMode.c_str(), "MASK"))
                destMaterial.alphaMode = SceneData::Material::AlphaMode::Mask;
            else if (!_stricmp(srcMaterial.alphaMode.c_str(), "Blend"))
                destMaterial.alphaMode = SceneData::Material::AlphaMode::Blend;
            else
                destMaterial.alphaMode = SceneData::Material::AlphaMode::Opaque;
            destMaterial.alphaCutoff = (float)srcMaterial.alphaCutoff;

            destMaterial.doubleSided = srcMaterial.doubleSided;

            destMaterial.baseColorFactor[0] = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[0];
            destMaterial.baseColorFactor[1] = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[1];
            destMaterial.baseColorFactor[2] = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[2];
            destMaterial.baseColorFactor[3] = (float)srcMaterial.pbrMetallicRoughness.baseColorFactor[3];

            GetTextureDetails(srcMaterial.pbrMetallicRoughness.baseColorTexture, destMaterial.baseColorTexture);
            destMaterial.baseColorTexture.channels = ".rgba";

            destMaterial.metallicFactor = (float)srcMaterial.pbrMetallicRoughness.metallicFactor;
            destMaterial.roughnessFactor = (float)srcMaterial.pbrMetallicRoughness.roughnessFactor;

            GetTextureDetails(srcMaterial.pbrMetallicRoughness.metallicRoughnessTexture, destMaterial.metallicTexture);
            destMaterial.metallicTexture.channels = ".b";

            GetTextureDetails(srcMaterial.pbrMetallicRoughness.metallicRoughnessTexture, destMaterial.roughnessTexture);
            destMaterial.roughnessTexture.channels = ".g";

            GetTextureDetails(srcMaterial.normalTexture, destMaterial.normalTexture);
            destMaterial.normalTexture.channels = ".rgb";
            destMaterial.normalScale = (float)srcMaterial.normalTexture.scale;

            GetTextureDetails(srcMaterial.occlusionTexture, destMaterial.occlusionTexture);
            destMaterial.occlusionTexture.channels = ".r";
            destMaterial.occlusionStrength = (float)srcMaterial.occlusionTexture.strength;
        }
    }

    // Flatten the geometry into a single mesh

    auto GetBufferFromAccessorIndex = [&model](int accessorIndex) -> BufferData
        {
            BufferData bufferData;

            if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
                return bufferData;
            bufferData.accessor = &model.accessors[accessorIndex];

            if (bufferData.accessor->bufferView < 0 || bufferData.accessor->bufferView >= static_cast<int>(model.bufferViews.size()))
                return bufferData;
            const tinygltf::BufferView& bufferView = model.bufferViews[bufferData.accessor->bufferView];

            if (bufferView.buffer < 0 || bufferView.buffer >= static_cast<int>(model.buffers.size()))
                return bufferData;
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            bufferData.cursor = buffer.data.data() + bufferView.byteOffset + bufferData.accessor->byteOffset;
            bufferData.cursorCount = bufferData.accessor->count * tinygltf::GetNumComponentsInType(bufferData.accessor->type);

            return bufferData;
        }
    ;

    bool allHaveTangents = true;
    bool allHaveUV0 = true;
    bool loadFailed = false;
    auto VisitNode = [&model, &sceneData, GetBufferFromAccessorIndex, &loadFailed, &allHaveTangents, &allHaveUV0](const tinygltf::Node& node, const DirectX::XMMATRIX& parentXForm, auto&& VisitNodeFn) -> void
        {
            DirectX::XMMATRIX relXForm;

            if (node.matrix.size() == 16)
            {
                relXForm = DirectX::XMMatrixSet(
                    (float)node.matrix[0], (float)node.matrix[1], (float)node.matrix[2], (float)node.matrix[3],
                    (float)node.matrix[4], (float)node.matrix[5], (float)node.matrix[6], (float)node.matrix[7],
                    (float)node.matrix[8], (float)node.matrix[9], (float)node.matrix[10], (float)node.matrix[11],
                    (float)node.matrix[12], (float)node.matrix[13], (float)node.matrix[14], (float)node.matrix[15]
                );
            }
            else
            {
                DirectX::XMMATRIX scale = (node.scale.size() == 3)
                    ? DirectX::XMMatrixScaling((float)node.scale[0], (float)node.scale[1], (float)node.scale[2])
                    : DirectX::XMMatrixIdentity();

                DirectX::XMMATRIX translation = (node.translation.size() == 3)
                    ? DirectX::XMMatrixTranslation((float)node.translation[0], (float)node.translation[1], (float)node.translation[2])
                    : DirectX::XMMatrixIdentity();

                DirectX::XMMATRIX rotation;
                if (node.rotation.size() == 4)
                {
                    DirectX::XMVECTOR quaternion = DirectX::XMVectorSet((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]);
                    rotation = DirectX::XMMatrixRotationQuaternion(quaternion);
                }
                else
                {
                    rotation = DirectX::XMMatrixIdentity();
                }

                relXForm = scale * rotation * translation;
            }

            DirectX::XMMATRIX xForm = parentXForm * relXForm;

            if (node.light >= 0)
            {
                const tinygltf::Light& light = model.lights[node.light];

                bool isDirectional = (!_stricmp(light.type.c_str(), "directional"));
                bool isSpot = (!_stricmp(light.type.c_str(), "spot"));

                DirectX::XMVECTOR v = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, isDirectional ? 0.0f : 1.0f);
                DirectX::XMVECTOR result = DirectX::XMVector4Transform(v, xForm);

                SceneData::Light newLight;
                newLight.posDir[0] = result.m128_f32[0];
                newLight.posDir[1] = result.m128_f32[1];
                newLight.posDir[2] = result.m128_f32[2];
                newLight.posDir[3] = isDirectional ? 0.0f : 1.0f;

                newLight.colorIntensity[0] = (float)light.color[0];
                newLight.colorIntensity[1] = (float)light.color[1];
                newLight.colorIntensity[2] = (float)light.color[2];
                newLight.colorIntensity[3] = (float)light.intensity;

                newLight.range = (float)light.range;
                newLight.spotInnerOuterRad[0] = isSpot ? (float)light.spot.innerConeAngle : 0.0f;
                newLight.spotInnerOuterRad[1] = isSpot ? (float)light.spot.outerConeAngle : 0.0f;

                sceneData.lights.push_back(newLight);
            }

            if (node.mesh >= 0)
            {
                const tinygltf::Mesh& mesh = model.meshes[node.mesh];

                for (size_t primitivesIndex = 0; primitivesIndex < mesh.primitives.size(); ++primitivesIndex)
                {
                    const tinygltf::Primitive& primitive = mesh.primitives[primitivesIndex];

                    BufferData bdPos, bdNormal, bdTangent, bdAlbedo, bdUV0, bdUV1, bdUV2, bdUV3;

                    for (const auto& it : primitive.attributes)
                    {
                        if (!_stricmp(it.first.c_str(), "NORMAL"))
                        {
                            bdNormal = GetBufferFromAccessorIndex(it.second);
                        }
                        else if (!_stricmp(it.first.c_str(), "TANGENT"))
                        {
                            bdTangent = GetBufferFromAccessorIndex(it.second);
                        }
                        else if (!_stricmp(it.first.c_str(), "POSITION"))
                        {
                            bdPos = GetBufferFromAccessorIndex(it.second);
                        }
                        else if (!_stricmp(it.first.c_str(), "TEXCOORD_0"))
                        {
                            bdUV0 = GetBufferFromAccessorIndex(it.second);
                        }
                        else if (!_stricmp(it.first.c_str(), "TEXCOORD_1"))
                        {
                            bdUV1 = GetBufferFromAccessorIndex(it.second);
                        }
                        else if (!_stricmp(it.first.c_str(), "TEXCOORD_2"))
                        {
                            bdUV2 = GetBufferFromAccessorIndex(it.second);
                        }
                        else if (!_stricmp(it.first.c_str(), "TEXCOORD_3"))
                        {
                            bdUV3 = GetBufferFromAccessorIndex(it.second);
                        }
                        else if (!_stricmp(it.first.c_str(), "COLOR_0"))
                        {
                            bdAlbedo = GetBufferFromAccessorIndex(it.second);
                        }
                        else
                        {
                            char buffer[256];
                            sprintf_s(buffer, "Unknown Vertex Attribute Ignored: \"%s\"\n", it.first.c_str());
                            sceneData.warn += buffer;
                        }
                    }

                    if (!bdTangent.cursor)
                        allHaveTangents = false;

                    if (!bdUV0.cursor)
                        allHaveUV0 = false;

                    BufferData bdIndices = GetBufferFromAccessorIndex(primitive.indices);

                    // We need at least positions and indices
                    if (bdIndices.cursor && bdPos.cursor)
                    {
                        // reserve space for our new vertices
                        size_t vertexStart = sceneData.flattenedVertices.size();
                        size_t vertexCount = bdIndices.cursorCount;
                        sceneData.flattenedVertices.resize(vertexStart + vertexCount);

                        for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
                        {
                            size_t index = 0;
                            if (!ReadBufferDataScalar(bdIndices, vertexIndex, index))
                            {
                                loadFailed = true;
                                break;
                            }

                            SceneData::Vertex& newVertex = sceneData.flattenedVertices[vertexStart + vertexIndex];

                            newVertex.materialID = primitive.material;
                            newVertex.shapeIndex = (int)primitivesIndex;

                            loadFailed &= ReadBufferDataVector(bdPos, 3, index, &newVertex.position[0]);

                            if (bdNormal.cursor)
                                loadFailed &= ReadBufferDataVector(bdNormal, 3, index, &newVertex.normal[0]);

                            if (bdTangent.cursor)
                                loadFailed &= ReadBufferDataVector(bdTangent, 4, index, &newVertex.tangent[0]);

                            if (bdAlbedo.cursor)
                                loadFailed &= ReadBufferDataVector(bdAlbedo, 4, index, &newVertex.albedo[0]);

                            if (bdUV0.cursor)
                                loadFailed &= ReadBufferDataVector(bdUV0, 2, index, &newVertex.uvs[0][0]);

                            if (bdUV1.cursor)
                                loadFailed &= ReadBufferDataVector(bdUV1, 2, index, &newVertex.uvs[1][0]);

                            if (bdUV2.cursor)
                                loadFailed &= ReadBufferDataVector(bdUV2, 2, index, &newVertex.uvs[2][0]);

                            if (bdUV3.cursor)
                                loadFailed &= ReadBufferDataVector(bdUV3, 2, index, &newVertex.uvs[3][0]);

                            // transform the vertex position by the xForm matrix
                            {
                                DirectX::XMVECTOR v = DirectX::XMVectorSet(newVertex.position[0], newVertex.position[1], newVertex.position[2], 1.0f);
                                DirectX::XMVECTOR result = DirectX::XMVector4Transform(v, xForm);
                                newVertex.position[0] = result.m128_f32[0];
                                newVertex.position[1] = result.m128_f32[1];
                                newVertex.position[2] = result.m128_f32[2];
                            }

                            // transform normal and tangent with the inverse transpose of the xForm matrix
                            {
                                DirectX::XMMATRIX invTransXForm = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, xForm));

                                if (bdNormal.cursor)
                                {
                                    DirectX::XMVECTOR n = DirectX::XMVectorSet(newVertex.normal[0], newVertex.normal[1], newVertex.normal[2], 0.0f);
                                    DirectX::XMVECTOR nResult = DirectX::XMVector4Transform(n, invTransXForm);
                                    nResult = DirectX::XMVector3Normalize(nResult);
                                    newVertex.normal[0] = nResult.m128_f32[0];
                                    newVertex.normal[1] = nResult.m128_f32[1];
                                    newVertex.normal[2] = nResult.m128_f32[2];
                                }
                                if (bdTangent.cursor)
                                {
                                    DirectX::XMVECTOR t = DirectX::XMVectorSet(newVertex.tangent[0], newVertex.tangent[1], newVertex.tangent[2], 0.0f);
                                    DirectX::XMVECTOR tResult = DirectX::XMVector4Transform(t, invTransXForm);
                                    tResult = DirectX::XMVector3Normalize(tResult);
                                    newVertex.tangent[0] = tResult.m128_f32[0];
                                    newVertex.tangent[1] = tResult.m128_f32[1];
                                    newVertex.tangent[2] = tResult.m128_f32[2];
                                    // w component (handedness) remains unchanged
                                }
                            }

                            if (loadFailed)
                                break;
                        }
                    }
                }
            }

            for (size_t childIndex = 0; childIndex < node.children.size(); ++childIndex)
            {
                const tinygltf::Node& childNode = model.nodes[node.children[childIndex]];
                VisitNodeFn(childNode, xForm, VisitNodeFn);
            }
        }
    ;

    for (size_t sceneIndex = 0; sceneIndex < model.scenes.size(); ++sceneIndex)
    {
        DirectX::XMMATRIX xForm = DirectX::XMMatrixIdentity();

        const tinygltf::Scene& scene = model.scenes[sceneIndex];
        for (size_t nodeIndex = 0; nodeIndex < scene.nodes.size(); ++nodeIndex)
        {
            const tinygltf::Node& node = model.nodes[scene.nodes[nodeIndex]];
            VisitNode(node, xForm, VisitNode);
        }
    }

    // Calculate tangents using uv0 if no tangents present
    if (!allHaveTangents && allHaveUV0)
    {
        std::vector<SceneData::Vertex>& geometry = sceneData.flattenedVertices;

        std::vector<Vec3> tangents(geometry.size(), Vec3{ 0.0f, 0.0f, 0.0f });
        std::vector<Vec3> bitangents(geometry.size(), Vec3{ 0.0f, 0.0f, 0.0f });
        for (size_t vertexIndex = 0; vertexIndex < geometry.size(); vertexIndex += 3)
        {
            SceneData::Vertex& v1 = geometry[vertexIndex + 0];
            SceneData::Vertex& v2 = geometry[vertexIndex + 1];
            SceneData::Vertex& v3 = geometry[vertexIndex + 2];

            Vec3 pos1 = v1.position;
            Vec3 pos2 = v2.position;
            Vec3 pos3 = v3.position;

            Vec2 uv1 = v1.uvs[0];
            Vec2 uv2 = v2.uvs[0];
            Vec2 uv3 = v3.uvs[0];

            Vec3 pos21 = pos2 - pos1;
            Vec3 pos31 = pos3 - pos1;

            Vec2 uv21 = uv2 - uv1;
            Vec2 uv31 = uv3 - uv1;

            float r = 1.0f / (uv21[0] * uv31[1] - uv21[1] * uv31[0]);

            if (std::isfinite(r))
            {
                Vec3 u = Vec3{ (uv31[1] * pos21[0] - uv21[1] * pos31[0]) * r, (uv31[1] * pos21[1] - uv21[1] * pos31[1]) * r, (uv31[1] * pos21[2] - uv21[1] * pos31[2]) * r };
                Vec3 v = Vec3{ (uv21[0] * pos31[0] - uv31[0] * pos21[0]) * r, (uv21[0] * pos31[1] - uv31[0] * pos21[1]) * r, (uv21[0] * pos31[2] - uv31[0] * pos21[2]) * r };

                tangents[vertexIndex + 0] += u;
                tangents[vertexIndex + 1] += u;
                tangents[vertexIndex + 2] += u;

                bitangents[vertexIndex + 0] += v;
                bitangents[vertexIndex + 1] += v;
                bitangents[vertexIndex + 2] += v;
            }
        }

        for (size_t vertexIndex = 0; vertexIndex < geometry.size(); ++vertexIndex)
        {
            SceneData::Vertex& v = geometry[vertexIndex];

            const Vec3& normal = v.normal;
            const Vec3& tangent = tangents[vertexIndex];
            const Vec3& bitangent = bitangents[vertexIndex];

            Vec3 tangentOut = tangent - normal * Dot(normal, tangent);
            if (Dot(tangentOut, tangentOut) < 0.00001f)
                tangentOut = tangent;

            if (Dot(tangentOut, tangentOut) > 0.0f)
            {
                tangentOut = Normalize(tangentOut);
                v.tangent[0] = tangentOut[0];
                v.tangent[1] = tangentOut[1];
                v.tangent[2] = tangentOut[2];
                v.tangent[3] = (Dot(Cross(normal, tangent), bitangent) < 0.0f) ? 0.0f : 1.0f;
            }
            else
            {
                v.tangent[0] = 1.0f;
                v.tangent[1] = 0.0f;
                v.tangent[2] = 0.0f;
                v.tangent[3] = 1.0f;
            }
        }
    }

    return !loadFailed;
}
