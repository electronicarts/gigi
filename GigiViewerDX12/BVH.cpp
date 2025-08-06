///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "BVH.h"

#define TINYBVH_IMPLEMENTATION
#include "external/tinybvh/tiny_bvh.h"

#include <vector>
#include <filesystem>

// Assumes:
// * firstPos is a float3.
// * you add "stride" bytes toget to the next position.
// * count is the number of times you do this. (the total number of positions)
bool SaveAsBVH(const char* fileName, const unsigned char* firstPos, size_t stride, size_t count)
{
	// Grab the position data into a flat array of tinybvh::bvhvec4 values
	std::vector<tinybvh::bvhvec4> vertices(count, tinybvh::bvhvec4{ 0.0f, 0.0f, 0.0f, 0.0f });
	for (size_t i = 0; i < count; ++i)
		memcpy(&vertices[i], &firstPos[i * stride], sizeof(float) * 3);

	// Build the bvh
	tinybvh::BVH_GPU bvh;
	bvh.BuildHQ(vertices.data(), (uint32_t)vertices.size() / 3);

	/*
	// Example of using tiny bvh library to shoot a ray into the bvh
	tinybvh::bvhvec3 O(0.5f, 0.5f, -1);
	tinybvh::bvhvec3 D(0.1f, 0, 2);
	tinybvh::Ray ray(O, D);
	int steps = bvh.Intersect(ray);
	*/

	// save the files to disk
	{
		std::filesystem::path basePath(fileName);
		std::string extension = ".bvh";
		if (basePath.has_extension())
		{
			extension = basePath.extension().string();
			basePath.replace_extension("");
		}

		// Vertex Data
		{
			std::filesystem::path path = basePath;
			path.replace_extension(".vertices.bvh");

			FILE* file = nullptr;
			fopen_s(&file, path.string().c_str(), "wb");
			if (!file)
				return false;

			fwrite(vertices.data(), sizeof(vertices[0]), vertices.size(), file);

			fclose(file);
		}

		// BVH Node Data
		{
			std::filesystem::path path = basePath;
			path.replace_extension(".nodes.bvh");

			FILE* file = nullptr;
			fopen_s(&file, path.string().c_str(), "wb");
			if (!file)
				return false;

			fwrite(bvh.bvhNode, sizeof(tinybvh::BVH_GPU::BVHNode), bvh.usedNodes, file);

			fclose(file);
		}

		// Triangle Index Data
		{
			std::filesystem::path path = basePath;
			path.replace_extension(".triindices.bvh");

			FILE* file = nullptr;
			fopen_s(&file, path.string().c_str(), "wb");
			if (!file)
				return false;

			fwrite(bvh.bvh.primIdx, sizeof(uint32_t), bvh.idxCount, file);

			fclose(file);
		}

		// All data in one file
		{
			std::filesystem::path path = basePath;
			path.replace_extension(".combined.bvh");

			FILE* file = nullptr;
			fopen_s(&file, path.string().c_str(), "wb");
			if (!file)
				return false;

			// write how many bytes big the vertex and node data are, so that all 3 buffers can be accessed
			uint32_t vertexBufferBytes = uint32_t(sizeof(vertices[0]) * vertices.size());
			uint32_t nodeBufferBytes = uint32_t(sizeof(tinybvh::BVH_GPU::BVHNode) * bvh.usedNodes);
			fwrite(&vertexBufferBytes, sizeof(vertexBufferBytes), 1, file);
			fwrite(&nodeBufferBytes, sizeof(nodeBufferBytes), 1, file);

			// Write the vertex data, node data, then triangle index data
			fwrite(vertices.data(), sizeof(vertices[0]), vertices.size(), file);
			fwrite(bvh.bvhNode, sizeof(tinybvh::BVH_GPU::BVHNode), bvh.usedNodes, file);
			fwrite(bvh.bvh.primIdx, sizeof(uint32_t), bvh.idxCount, file);

			fclose(file);
		}
	}
	return true;
}