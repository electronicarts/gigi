///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "SceneDataCache.h"

#include "../external/OpenFBX/ofbx.h"

#include <filesystem>
#include <algorithm>

static Vec2 ToVec2(const ofbx::Vec2& v)
{
	Vec2 ret;
	ret[0] = (float)v.x;
	ret[1] = (float)v.y;
	return ret;
}

static Vec3 ToVec3(const ofbx::Vec3& v)
{
	Vec3 ret;
	ret[0] = (float)v.x;
	ret[1] = (float)v.y;
	ret[2] = (float)v.z;
	return ret;
}

bool SceneDataCache::LoadFBX(FileCache::File& fileData, SceneData& sceneData)
{
	// Ignoring certain nodes will only stop them from being processed not tokenised (i.e. they will still be in the tree)
	ofbx::LoadFlags loadFlags =
		//		ofbx::LoadFlags::IGNORE_MODELS |
		//		ofbx::LoadFlags::IGNORE_GEOMETRY |
		ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
		ofbx::LoadFlags::IGNORE_CAMERAS |
		ofbx::LoadFlags::IGNORE_LIGHTS |
		ofbx::LoadFlags::IGNORE_TEXTURES |
		ofbx::LoadFlags::IGNORE_SKIN |
		ofbx::LoadFlags::IGNORE_BONES |
		ofbx::LoadFlags::IGNORE_PIVOTS |
		//		ofbx::LoadFlags::IGNORE_MATERIALS |
		ofbx::LoadFlags::IGNORE_POSES |
		ofbx::LoadFlags::IGNORE_VIDEOS |
		ofbx::LoadFlags::IGNORE_LIMBS |
		//		ofbx::LoadFlags::IGNORE_MESHES |
		ofbx::LoadFlags::IGNORE_ANIMATIONS;

	ofbx::IScene* fbxScene = ofbx::load((ofbx::u8*)fileData.GetBytes(), (int)fileData.GetSize(), (ofbx::u16)loadFlags);
    if (!fbxScene)
        return false;

	struct ImportMeshData
	{
		const ofbx::Mesh* fbxMesh = nullptr;
		int materialID;
		int submeshIndex;
	};

	int meshCount = fbxScene->getMeshCount();

	// Find mesh partitions by material index
    // Also get the material IDs
	std::vector<ImportMeshData> importMeshes;
	{
        int materialIndexOffset = 0;
		for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex)
		{
			const ofbx::Mesh* mesh = fbxScene->getMesh(meshIndex);
			const int materialCount = mesh->getMaterialCount();

            sceneData.materials.resize(materialIndexOffset + materialCount);

			for (int matIndex = 0; matIndex < materialCount; ++matIndex)
			{
				auto& importMesh = importMeshes.emplace_back();
				importMesh.fbxMesh = mesh;
				importMesh.materialID = materialIndexOffset + matIndex;
				importMesh.submeshIndex = matIndex;

                SceneData::Material& destMaterial = sceneData.materials[materialIndexOffset + matIndex];
                const ofbx::Material* srcMaterial_ = mesh->getMaterial(matIndex);
                if (!srcMaterial_)
                    continue;
                const ofbx::Material& srcMaterial = *srcMaterial_;

                destMaterial.name = srcMaterial.name;

                const ofbx::Color& emissiveColor = srcMaterial.getEmissiveColor();
                float emissiveFactor = (float)srcMaterial.getEmissiveFactor();
                destMaterial.emissiveFactor[0] = emissiveColor.r * emissiveFactor;
                destMaterial.emissiveFactor[1] = emissiveColor.g * emissiveFactor;
                destMaterial.emissiveFactor[2] = emissiveColor.b * emissiveFactor;
                const ofbx::Texture* emissiveTexture = srcMaterial.getTexture(ofbx::Texture::EMISSIVE);
                if (emissiveTexture)
                {
                    char fileName[1024];
                    emissiveTexture->getRelativeFileName().toString(fileName);
                    destMaterial.emissiveTexture.fileName = fileName;
                    destMaterial.emissiveTexture.channels = ".rgb";
                }

                const ofbx::Color& diffuseColor = srcMaterial.getDiffuseColor();
                float diffuseColorFactor = (float)srcMaterial.getDiffuseFactor();
                destMaterial.baseColorFactor[0] = diffuseColor.r * diffuseColorFactor;
                destMaterial.baseColorFactor[1] = diffuseColor.g * diffuseColorFactor;
                destMaterial.baseColorFactor[2] = diffuseColor.b * diffuseColorFactor;
                const ofbx::Texture* diffuseTexture = srcMaterial.getTexture(ofbx::Texture::DIFFUSE);
                if (diffuseTexture)
                {
                    char fileName[1024];
                    diffuseTexture->getRelativeFileName().toString(fileName);
                    destMaterial.baseColorTexture.fileName = fileName;
                    destMaterial.baseColorTexture.channels = ".rgba";
                }

                const ofbx::Texture* normalTexture = srcMaterial.getTexture(ofbx::Texture::NORMAL);
                if (normalTexture)
                {
                    char fileName[1024];
                    normalTexture->getRelativeFileName().toString(fileName);
                    destMaterial.normalTexture.fileName = fileName;
                    destMaterial.normalTexture.channels = ".rgb";
                }
			}
            materialIndexOffset += materialCount;
		}
	}

	// Flatten the fbx so that it doesn't use indices
	std::vector<SceneData::Vertex>& geometry = sceneData.flattenedVertices;
	{
		std::vector<int> tmpTriIndices;

		for (size_t meshIndex = 0; meshIndex < importMeshes.size(); ++meshIndex)
		{
			int geometryIndexStart = (int)geometry.size();

			const ImportMeshData& importMesh = importMeshes.at(meshIndex);
			const ofbx::Mesh& mesh = *importMesh.fbxMesh;

			const ofbx::GeometryData& geom = mesh.getGeometryData();

			ofbx::GeometryPartition partition = geom.getPartition(importMesh.submeshIndex);
			if (partition.polygon_count == 0)
			{
				continue;
			}

			ofbx::Vec3Attributes positions = geom.getPositions();
			ofbx::Vec3Attributes normals = geom.getNormals();
			ofbx::Vec3Attributes tangents = geom.getTangents();
			ofbx::Vec4Attributes colors = geom.getColors();

			tmpTriIndices.resize(partition.max_polygon_triangles * 3);

			for (int polyIndex = 0; polyIndex < partition.polygon_count; ++polyIndex)
			{
				const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polyIndex];

				uint32_t indexCount = ofbx::triangulate(geom, polygon, tmpTriIndices.data());

				for (uint32_t i = 0; i < indexCount; ++i)
				{
					size_t nextGeometryIndex = geometry.size();
					geometry.resize(nextGeometryIndex + 1);
					SceneData::Vertex& newVertex = geometry[nextGeometryIndex];
					newVertex.shapeIndex = static_cast<int>(meshIndex);

					int vertexIndex = tmpTriIndices[i];

					newVertex.position[0] = (float)positions.get(vertexIndex).x;
					newVertex.position[1] = (float)positions.get(vertexIndex).y;
					newVertex.position[2] = (float)positions.get(vertexIndex).z;

					if (normals.values)
					{
						newVertex.normal[0] = (float)normals.get(vertexIndex).x;
						newVertex.normal[1] = (float)normals.get(vertexIndex).y;
						newVertex.normal[2] = (float)normals.get(vertexIndex).z;
					}

					if (colors.values)
					{
						newVertex.albedo[0] = (float)colors.get(vertexIndex).x;
						newVertex.albedo[1] = (float)colors.get(vertexIndex).y;
						newVertex.albedo[2] = (float)colors.get(vertexIndex).z;
						newVertex.albedo[3] = (float)colors.get(vertexIndex).w;
					}

					for (int uvIndex = 0; uvIndex < ofbx::Geometry::s_uvs_max; ++uvIndex)
					{
						ofbx::Vec2Attributes uvs = geom.getUVs(uvIndex);

						if (uvs.values)
						{
							newVertex.uvs[uvIndex][0] = (float)uvs.get(vertexIndex).x;
							newVertex.uvs[uvIndex][1] = (float)uvs.get(vertexIndex).y;
						}
						else
						{
							newVertex.uvs[uvIndex][0] = 0.0f;
							newVertex.uvs[uvIndex][1] = 0.0f;
						}
					}

					newVertex.materialID = importMesh.materialID;
				}
			}
		}
	}

	// Calculate tangents and bitangents, using uv0
	{
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

    return geometry.size() > 0;;
}
