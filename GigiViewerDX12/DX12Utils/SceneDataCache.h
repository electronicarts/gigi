///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2026 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include <vector>
#include <unordered_map>
#include <string>
#include <array>

#include "FileCache.h"

#include "VectorMath.h"

#include "Schemas/Types.h"

// clang-format on

struct SceneData
{
    struct Vertex
    {
        Vec3 position = Vec3{ 0.0f, 0.0f, 0.0f };
        Vec3 normal = Vec3{ 0.0f, 0.0f, 0.0f };
        Vec4 tangent = Vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
        Vec4 albedo = Vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
        Vec2 uvs[4]; // FBX supports up to 4 uvs
        int materialID = -1;
        int shapeIndex = -1; // mesh index in FBX. shape index in OBJ. etc.
    };

    struct Light
    {
        Vec4 posDir = Vec4{ 0.0f, 0.0f, 0.0f, 0.0f }; // w = 0 for directional, 1 for point
        Vec4 colorIntensity = Vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
        float range = 0.0f; // For point lights. 0 is infinite.
        Vec2 spotInnerOuterRad = Vec2{ 0.0f, 0.0f }; // In radians. Inner and outer cone angle. if outer is 0, then not a spot light.
    };

    struct Material
    {
        struct Texture
        {
            std::string fileName;
            int texCoordIndex = 0;

            SamplerFilter filter = SamplerFilter::MinMagMipLinear;
            SamplerAddressMode wrapS = SamplerAddressMode::Wrap;
            SamplerAddressMode wrapT = SamplerAddressMode::Wrap;

            std::string channels; // where the data lives in the texture, such as ".gb" for the green and blue channels
        };

        enum AlphaMode
        {
            Opaque,  // Ignore alpha
            Mask,    // Discard if alpha < alphaCutoff
            Blend    // Combined with background using "over" operator
        };

        std::string name;

        Vec3 emissiveFactor = Vec3{ 0.0f, 0.0f, 0.0f };
        Texture emissiveTexture;

        AlphaMode alphaMode = AlphaMode::Opaque;
        float alphaCutoff = 1.0f;

        bool doubleSided = true;

        // Albedo if non metal. F0 for metals.
        Vec4 baseColorFactor = Vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
        Texture baseColorTexture;
        
        float metallicFactor = 0.0f;
        Texture metallicTexture;

        float roughnessFactor = 1.0f;
        Texture roughnessTexture;

        Texture normalTexture;
        Texture occlusionTexture;

        // From GLTF docs
        // scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
        float normalScale = 1.0f;

        // From GLTF docs
        // occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
        float occlusionStrength = 1.0f;
    };

    std::string filename;
    std::string warn, error;
    std::vector<Vertex> flattenedVertices;
    std::vector<Light> lights;
    std::vector<Material> materials;
    bool valid = false;
};

class SceneDataCache
{
public:

    SceneData& Get(FileCache& fileCache, const char* fileName);

	bool Remove(const char* fileName)
	{
		if (m_cache.count(fileName) == 0)
			return false;

		m_cache.erase(fileName);
		return true;
	}

	void ClearCache()
	{
		std::unordered_map<std::string, SceneData> empty;
		std::swap(m_cache, empty);
	}

	std::unordered_map<std::string, SceneData> getCache()
	{
		return m_cache;
	}

private:
	std::unordered_map<std::string, SceneData> m_cache;

    bool LoadFBX(FileCache::File& fileData, SceneData& sceneData);
    bool LoadOBJ(FileCache::File& fileData, SceneData& sceneData);
    bool LoadGLTF(FileCache::File& fileData, SceneData& sceneData);
};
