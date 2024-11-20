///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "FBXCache.h"

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

FBXCache::FBXData& FBXCache::Get(FileCache& fileCache, const char* fileName_)
{
	// normalize the string by making it canonical and making it lower case
	std::string s = std::filesystem::weakly_canonical(fileName_).string();
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	const char* fileName = s.c_str();

	if (m_cache.count(fileName) != 0)
		return m_cache[fileName];

	FileCache::File fileData = fileCache.Get(fileName);

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

	FBXData fbxData;
	ofbx::IScene* fbxScene = ofbx::load((ofbx::u8*)fileData.GetBytes(), (int)fileData.GetSize(), (ofbx::u16)loadFlags);
	if (!fbxScene)
	{
		fbxData.valid = false;
		m_cache[fileName] = fbxData;
		return m_cache[fileName];
	}

	struct ImportMeshData
	{
		const ofbx::Mesh* fbxMesh = nullptr;
		int materialID;
		int submeshIndex;
	};

	int meshCount = fbxScene->getMeshCount();

	// Find mesh partitions by material index
	std::vector<ImportMeshData> importMeshes;
	{
		for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex)
		{
			const ofbx::Mesh* mesh = fbxScene->getMesh(meshIndex);
			const int materialCount = mesh->getMaterialCount();

			for (int matIndex = 0; matIndex < materialCount; ++matIndex)
			{
				auto& importMesh = importMeshes.emplace_back();
				importMesh.fbxMesh = mesh;
				importMesh.materialID = matIndex;
				importMesh.submeshIndex = matIndex;
			}
		}
	}

	// Flatten the fbx so that it doesn't use indices
	std::vector<FlattenedVertex>& geometry = fbxData.flattenedVertices;
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
					FlattenedVertex& newVertex = geometry[nextGeometryIndex];
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

				ofbx::Vec2Attributes uvs = geom.getUVs(0);
				if (!uvs.values)
					continue;

				// Calculate tangents and bitangents, using uv0
				std::vector<Vec3> tangents(indexCount, Vec3{ 0.0f, 0.0f, 0.0f });
				std::vector<Vec3> bitangents(indexCount, Vec3{ 0.0f, 0.0f, 0.0f });
				for (int i = 0; i < static_cast<int>(indexCount); i += 3)
				{
					int vertexIndex1 = tmpTriIndices[i + 0];
					int vertexIndex2 = tmpTriIndices[i + 1];
					int vertexIndex3 = tmpTriIndices[i + 2];

					int vi1 = i + 0;
					int vi2 = i + 1;
					int vi3 = i + 2;

					Vec3 pos1 = ToVec3(positions.get(vertexIndex1));
					Vec3 pos2 = ToVec3(positions.get(vertexIndex2));
					Vec3 pos3 = ToVec3(positions.get(vertexIndex3));

					Vec2 uv1 = ToVec2(uvs.get(vertexIndex1));
					Vec2 uv2 = ToVec2(uvs.get(vertexIndex2));
					Vec2 uv3 = ToVec2(uvs.get(vertexIndex3));

					Vec3 pos21 = pos2 - pos1;
					Vec3 pos31 = pos3 - pos1;

					Vec2 uv21 = uv2 - uv1;
					Vec2 uv31 = uv3 - uv1;

					float r = 1.0f / (uv21[0] * uv31[1] - uv21[1] * uv31[0]);

					if (std::isfinite(r))
					{
						Vec3 u = Vec3{ (uv31[1] * pos21[0] - uv21[1] * pos31[0]) * r, (uv31[1] * pos21[1] - uv21[1] * pos31[1]) * r, (uv31[1] * pos21[2] - uv21[1] * pos31[2]) * r };
						Vec3 v = Vec3{ (uv21[0] * pos31[0] - uv31[0] * pos21[0]) * r, (uv21[0] * pos31[1] - uv31[0] * pos21[1]) * r, (uv21[0] * pos31[2] - uv31[0] * pos21[2]) * r };

						tangents[vi1] += u;
						tangents[vi2] += u;
						tangents[vi3] += u;

						bitangents[vi1] += v;
						bitangents[vi2] += v;
						bitangents[vi3] += v;
					}
				}

				for (size_t indexIndex = 0; indexIndex < indexCount; ++indexIndex)
				{
					int vi = tmpTriIndices[indexIndex];

					Vec3 normal = ToVec3(normals.get(vi));

					Vec3& tangent = tangents[indexIndex];
					Vec3& bitangent = bitangents[indexIndex];

					Vec3 tangentOut = tangent - normal * Dot(normal, tangent);
					if (Dot(tangentOut, tangentOut) < 0.00001f)
						tangentOut = tangent;

					if (Dot(tangentOut, tangentOut) > 0.0f)
					{
						tangentOut = Normalize(tangentOut);
						geometry[geometryIndexStart + indexIndex].tangent[0] = tangentOut[0];
						geometry[geometryIndexStart + indexIndex].tangent[1] = tangentOut[1];
						geometry[geometryIndexStart + indexIndex].tangent[2] = tangentOut[2];
						geometry[geometryIndexStart + indexIndex].tangent[3] = (Dot(Cross(normal, tangent), bitangent) < 0.0f) ? 0.0f : 1.0f;
					}
					else
					{
						geometry[geometryIndexStart + indexIndex].tangent[0] = 1.0f;
						geometry[geometryIndexStart + indexIndex].tangent[1] = 0.0f;
						geometry[geometryIndexStart + indexIndex].tangent[2] = 0.0f;
						geometry[geometryIndexStart + indexIndex].tangent[3] = 1.0f;
					}
				}
			}
		}
	}

	fbxData.valid = geometry.size() > 0;
	m_cache[fileName] = fbxData;
	return m_cache[fileName];
}
