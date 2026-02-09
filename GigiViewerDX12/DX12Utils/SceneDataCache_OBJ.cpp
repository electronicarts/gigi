///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "SceneDataCache.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include <filesystem>
#include <algorithm>

bool SceneDataCache::LoadOBJ(FileCache::File& fileData, SceneData& sceneData)
{
	tinyobj::MaterialFileReader materialReader(std::filesystem::path(fileData.GetFileName()).remove_filename().string());

	std::stringstream objStream;
    objStream << fileData.GetBytes();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

	if (!tinyobj::LoadObj(
		&attrib,
		&shapes,
		&materials,
		&sceneData.warn,
		&sceneData.error,
		&objStream,
		&materialReader))
	{
        return false;
	}

    // Get the materials - obj materials are a little "application defined" so let's try our best.
    sceneData.materials.resize(materials.size());
    for (size_t materialIndex = 0; materialIndex < materials.size(); ++materialIndex)
    {
        tinyobj::material_t& srcMaterial = materials[materialIndex];
        SceneData::Material& destMaterial = sceneData.materials[materialIndex];

        destMaterial.name = srcMaterial.name;

        destMaterial.emissiveFactor[0] = srcMaterial.emission[0];
        destMaterial.emissiveFactor[1] = srcMaterial.emission[1];
        destMaterial.emissiveFactor[2] = srcMaterial.emission[2];
        destMaterial.emissiveTexture.fileName = srcMaterial.emissive_texname;
        destMaterial.emissiveTexture.channels = ".rgb";

        destMaterial.baseColorFactor[0] = srcMaterial.diffuse[0];
        destMaterial.baseColorFactor[1] = srcMaterial.diffuse[1];
        destMaterial.baseColorFactor[2] = srcMaterial.diffuse[2];
        destMaterial.baseColorTexture.fileName = srcMaterial.diffuse_texname;
        destMaterial.baseColorTexture.channels = ".rgba";

        destMaterial.roughnessFactor = srcMaterial.roughness;
        destMaterial.roughnessTexture.fileName = srcMaterial.roughness_texname;
        destMaterial.roughnessTexture.channels = ".r";

        destMaterial.metallicFactor = srcMaterial.metallic;
        destMaterial.metallicTexture.fileName = srcMaterial.metallic_texname;
        destMaterial.metallicTexture.channels = ".r";

        destMaterial.normalTexture.fileName = srcMaterial.normal_texname;
        destMaterial.normalTexture.channels = ".rgb";
    }

	// Flatten the obj so that it doesn't use indices
	std::vector<SceneData::Vertex>& geometry = sceneData.flattenedVertices;
	int shapeIndex = -1;
	for (const auto& shape : shapes)
	{
		shapeIndex++;
		size_t geometryIndexStart = geometry.size();

		// flatten the indexed vertices
		for (const auto& index : shape.mesh.indices)
		{
			size_t nextGeometryIndex = geometry.size();
			geometry.resize(nextGeometryIndex + 1);
            SceneData::Vertex& newVertex = geometry[nextGeometryIndex];
			newVertex.shapeIndex = shapeIndex;

			newVertex.albedo = Vec4
			{
				attrib.colors[3 * index.vertex_index + 0],
                attrib.colors[3 * index.vertex_index + 1],
                attrib.colors[3 * index.vertex_index + 2],
				1.0f
			};

			newVertex.position = Vec3
			{
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
			};

			if (index.normal_index >= 0)
			{
				newVertex.normal = Vec3
				{
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
				};
			}

			if (index.texcoord_index >= 0)
			{
				newVertex.uvs[0] = Vec2
				{
                    attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
				};
			}

			newVertex.uvs[1] = Vec2{ 0.0f, 0.0f };
			newVertex.uvs[2] = Vec2{ 0.0f, 0.0f };
			newVertex.uvs[3] = Vec2{ 0.0f, 0.0f };
		}

		// get the material IDs
		size_t numFaces = shape.mesh.indices.size() / 3;
		for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
		{
			geometry[geometryIndexStart + faceIndex * 3 + 0].materialID = shape.mesh.material_ids[faceIndex];
			geometry[geometryIndexStart + faceIndex * 3 + 1].materialID = shape.mesh.material_ids[faceIndex];
			geometry[geometryIndexStart + faceIndex * 3 + 2].materialID = shape.mesh.material_ids[faceIndex];
		}

		// Calculate normals, tangents and bitangents
		std::vector<Vec3> normals(attrib.vertices.size(), Vec3{ 0.0f, 0.0f, 0.0f });
		std::vector<Vec3> tangents(attrib.vertices.size(), Vec3{ 0.0f, 0.0f, 0.0f });
		std::vector<Vec3> bitangents(attrib.vertices.size(), Vec3{ 0.0f, 0.0f, 0.0f });
		for (size_t indexIndex = 0; indexIndex < shape.mesh.indices.size(); indexIndex += 3)
		{
			int vi1 = shape.mesh.indices[indexIndex + 0].vertex_index;
			int ti1 = shape.mesh.indices[indexIndex + 0].texcoord_index;

			int vi2 = shape.mesh.indices[indexIndex + 1].vertex_index;
			int ti2 = shape.mesh.indices[indexIndex + 1].texcoord_index;

			int vi3 = shape.mesh.indices[indexIndex + 2].vertex_index;
			int ti3 = shape.mesh.indices[indexIndex + 2].texcoord_index;

			Vec3 pos1 = Vec3
			{
                attrib.vertices[3 * vi1 + 0],
                attrib.vertices[3 * vi1 + 1],
                attrib.vertices[3 * vi1 + 2],
			};

			Vec3 pos2 = Vec3
			{
                attrib.vertices[3 * vi2 + 0],
                attrib.vertices[3 * vi2 + 1],
                attrib.vertices[3 * vi2 + 2],
			};

			Vec3 pos3 = Vec3
			{
                attrib.vertices[3 * vi3 + 0],
                attrib.vertices[3 * vi3 + 1],
                attrib.vertices[3 * vi3 + 2],
			};

			Vec3 pos21 = pos2 - pos1;
			Vec3 pos31 = pos3 - pos1;

			normals[vi1] += Cross(pos21, pos31);
			normals[vi2] += Cross(pos21, pos31);
			normals[vi3] += Cross(pos21, pos31);

			if (ti1 == -1 || ti2 == -1 || ti3 == -1)
				continue;

			Vec2 uv1 = Vec2
			{
                attrib.texcoords[2 * ti1 + 0],
                attrib.texcoords[2 * ti1 + 1],
			};

			Vec2 uv2 = Vec2
			{
                attrib.texcoords[2 * ti2 + 0],
                attrib.texcoords[2 * ti2 + 1],
			};

			Vec2 uv3 = Vec2
			{
                attrib.texcoords[2 * ti3 + 0],
                attrib.texcoords[2 * ti3 + 1],
			};

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

		for (size_t indexIndex = 0; indexIndex < shape.mesh.indices.size(); ++indexIndex)
		{
			int vi = shape.mesh.indices[indexIndex].vertex_index;
			int ni = shape.mesh.indices[indexIndex].normal_index;

			Vec3 normal = Vec3{ 0.0f, 0.0f, 0.0f };

			if (ni >= 0)
			{
				normal = Vec3
				{
                    attrib.normals[3 * ni + 0],
                    attrib.normals[3 * ni + 1],
                    attrib.normals[3 * ni + 2]
				};
			}
			else
			{
				normal = Normalize(normals[vi]);
				geometry[geometryIndexStart + indexIndex].normal[0] = normal[0];
				geometry[geometryIndexStart + indexIndex].normal[1] = normal[1];
				geometry[geometryIndexStart + indexIndex].normal[2] = normal[2];
			}

			Vec3& tangent = tangents[vi];
			Vec3& bitangent = bitangents[vi];

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

	return geometry.size() > 0;
}
