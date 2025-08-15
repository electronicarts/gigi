///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"
#include "DX12Utils/Utils.h"
#include "GigiCompilerLib/ParseCSV.h"
#include <filesystem>

#include <istream>
#include <streambuf>
#include <unordered_set>

#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
// clang-format on

void RuntimeTypes::RenderGraphNode_Resource_Buffer::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
	RenderGraphNode_Base::Release(interpreter);

	// free resources
	if (m_resourceInitialState)
	{
		interpreter.m_transitions.Untrack(m_resourceInitialState);
		interpreter.m_delayedRelease.Add(m_resourceInitialState);
		m_resourceInitialState = nullptr;
	}

	if (m_resource)
	{
		interpreter.m_transitions.Untrack(m_resource);
		interpreter.m_delayedRelease.Add(m_resource);
		m_resource = nullptr;
	}

	if (m_tlas)
	{
		interpreter.m_transitions.Untrack(m_tlas);
		interpreter.m_delayedRelease.Add(m_tlas);
		m_tlas = nullptr;
	}

	if (m_blas)
	{
		interpreter.m_transitions.Untrack(m_blas);
		interpreter.m_delayedRelease.Add(m_blas);
		m_blas = nullptr;
	}

	if (m_instanceDescs)
	{
		interpreter.m_transitions.Untrack(m_instanceDescs);
		interpreter.m_delayedRelease.Add(m_instanceDescs);
		m_instanceDescs = nullptr;
	}
}

bool GigiInterpreterPreviewWindowDX12::MakeAccelerationStructures(const RenderGraphNode_Resource_Buffer& node)
{
	ImportedResourceDesc& resourceDesc = m_importedResources[node.name];
	RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData = m_RenderGraphNode_Resource_Buffer_RuntimeData.GetOrCreate(node.name);

	m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	m_transitions.Flush(m_commandList);

	return MakeAccelerationStructures(node, resourceDesc, runtimeData);
}

bool GigiInterpreterPreviewWindowDX12::MakeAccelerationStructures(const RenderGraphNode_Resource_Buffer& node, const ImportedResourceDesc& resourceDesc, RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData)
{
	if (!SupportsRaytracing())
	{
		m_logFn(LogLevel::Error, "Raytracing is not supported\n");
		return false;
	}

	// Make BLAS
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};

	if (resourceDesc.buffer.IsAABBs)
	{
		// verify that the format is ok: float or float3. No structs.
		DXGI_FORMAT vertexFormat = runtimeData.m_format;
		{
			if (runtimeData.m_structIndex != -1)
			{
				m_logFn(LogLevel::Error, "Cannot use a struct for AABBs BLAS\n");
				return false;
			}

			switch (vertexFormat)
			{
				case DXGI_FORMAT_R32_FLOAT:
				case DXGI_FORMAT_R32G32B32_FLOAT:
					break;
				default:
				{
					m_logFn(LogLevel::Error, "Invalid format for AABBs BLAS (%s)\n", Get_DXGI_FORMAT_Info(vertexFormat).name);
					return false;
				}
			}
		}
		DXGI_FORMAT_Info vertexFormatInfo = Get_DXGI_FORMAT_Info(vertexFormat);

		// Calculate how many AABBs there are
		UINT AABBCount = (runtimeData.m_count * vertexFormatInfo.channelCount) / 6;
		if (AABBCount == 0)
			return false;

		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
		geometryDesc.AABBs.AABBCount = AABBCount;
		geometryDesc.AABBs.AABBs.StartAddress = runtimeData.m_resource->GetGPUVirtualAddress();
		geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(float) * 6;
	}
	else
	{
		// We need 3 vertices for every triangle.
		// Round down, and fail if we don't have a single triangle.
		UINT vertexCount = 3 * (runtimeData.m_count / 3);
		if (vertexCount == 0)
			return false;

		DXGI_FORMAT vertexFormat = DXGI_FORMAT_FORCE_UINT;
		UINT64 vertexOffset = 0;
		UINT64 vertexSize = 0;
		if (runtimeData.m_structIndex != -1)
		{
			bool foundPosition = false;
			for (const StructField& field : m_renderGraph.structs[runtimeData.m_structIndex].fields)
			{
				if (field.semantic == StructFieldSemantic::Position)
				{
					vertexFormat = DataFieldTypeInfoDX12(field.type).typeFormat;
					foundPosition = true;
					break;
				}
				vertexOffset += field.sizeInBytes;
			}
			if (!foundPosition)
				return false;

			vertexSize = m_renderGraph.structs[runtimeData.m_structIndex].sizeInBytes;
		}
		else
		{
			vertexFormat = runtimeData.m_format;
			vertexSize = Get_DXGI_FORMAT_Info(vertexFormat).bytesPerPixel;
		}

		// verify that the vertex format is ok
		switch (vertexFormat)
		{
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
				break;
			default:
			{
				m_logFn(LogLevel::Error, "Invalid vertex format for BLAS (%s)\n", Get_DXGI_FORMAT_Info(vertexFormat).name);
				return false;
			}
		}

		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.IndexBuffer = 0;
		geometryDesc.Triangles.IndexCount = 0;
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
		geometryDesc.Triangles.Transform3x4 = 0;
		geometryDesc.Triangles.VertexFormat = vertexFormat;
		geometryDesc.Triangles.VertexCount = vertexCount;
		geometryDesc.Triangles.VertexBuffer.StartAddress = runtimeData.m_resource->GetGPUVirtualAddress() + vertexOffset;
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexSize;
	}

	// flags
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

	if (resourceDesc.buffer.BLASOpaque)
		geometryDesc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	if (resourceDesc.buffer.BLASNoDuplicateAnyhitInvocations)
		geometryDesc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

	// Get required sizes for an acceleration structure.
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = GGUserFile_TLASBuildFlagsToD3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(resourceDesc.buffer.RT_BuildFlags);
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes == 0)
		return false;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
	bottomLevelInputs.NumDescs = 1;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes == 0)
		return false;

	// allocate a scratch buffer for creating our BLAS and TLAS
	ID3D12Resource* scratch = CreateBuffer(
		m_device,
		(unsigned int)ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes)),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_HEAP_TYPE_DEFAULT,
		"RTAS Scratch"
	);

	// Create buffers for TLAS and BLAS, and track them in the resource tracker
	std::string nodeNameTLAS = node.name + " TLAS";
	runtimeData.m_tlasSize = (int)topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
	runtimeData.m_tlas = CreateBuffer(
		m_device,
		(unsigned int)ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, topLevelPrebuildInfo.ResultDataMaxSizeInBytes),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		D3D12_HEAP_TYPE_DEFAULT,
		nodeNameTLAS.c_str()
	);
	m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_tlas, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE));

	std::string nodeNameBLAS = node.name + " BLAS";
	runtimeData.m_blasSize = (int)bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
	runtimeData.m_blas = CreateBuffer(
		m_device,
		(unsigned int)ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		D3D12_HEAP_TYPE_DEFAULT,
		nodeNameBLAS.c_str()
	);
	m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_blas, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE));

	// Create an upload buffer for the instance descs and put the data in there	
	{
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc;
		memset(&instanceDesc, 0, sizeof(instanceDesc));
		instanceDesc.Transform[0][0] = 1.0f;
		instanceDesc.Transform[1][1] = 1.0f;
		instanceDesc.Transform[2][2] = 1.0f;
		instanceDesc.InstanceID = 0;
		instanceDesc.InstanceMask = 1;
		instanceDesc.AccelerationStructure = runtimeData.m_blas->GetGPUVirtualAddress();

		switch (resourceDesc.buffer.BLASCullMode)
		{
			case GGUserFile_BLASCullMode::CullNone: instanceDesc.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE; break;
			case GGUserFile_BLASCullMode::FrontIsClockwise: break;
			case GGUserFile_BLASCullMode::FrontIsCounterClockwise: instanceDesc.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE; break;
		}

		runtimeData.m_instanceDescs = CreateBuffer(
			m_device,
			sizeof(instanceDesc),
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_UPLOAD,
			(node.name + " Instance Descs").c_str()
		);

		D3D12_RAYTRACING_INSTANCE_DESC* data = nullptr;
		D3D12_RANGE  readRange = { 0, 0 };
		runtimeData.m_instanceDescs->Map(0, &readRange, reinterpret_cast<void**>(&data));
		memcpy(data, &instanceDesc, sizeof(instanceDesc));
		runtimeData.m_instanceDescs->Unmap(0, nullptr);
	}

	// BLAS Desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	{
		bottomLevelBuildDesc.Inputs = bottomLevelInputs;
		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = runtimeData.m_blas->GetGPUVirtualAddress();
	}

	// TLAS Desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	{
		topLevelInputs.InstanceDescs = runtimeData.m_instanceDescs->GetGPUVirtualAddress();
		topLevelBuildDesc.Inputs = topLevelInputs;
		topLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
		topLevelBuildDesc.DestAccelerationStructureData = runtimeData.m_tlas->GetGPUVirtualAddress();
	}

	// Build BLAS
	m_dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

	// Wait for it to be done
	// We need a UAV barrier here, but it is in state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE which won't get picked up by transition logic, so we force it.
	m_transitions.UAVBarrier(runtimeData.m_blas);
	m_transitions.Flush(m_commandList);

	// Build TLAS
	m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	// Wait for it to be done
	m_transitions.UAVBarrier(runtimeData.m_tlas);
	m_transitions.Flush(m_commandList);

	// Our resource promoted from common to non pixel shader resource, by being part of the BLAS.
	m_transitions.SetStateWithoutTransition(runtimeData.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// Free the scratch memory used to make the tlas and blas
	m_delayedRelease.Add(scratch);

	return true;
}

template <typename T>
static void AppendBytes(std::vector<char>& bytes, const T& t)
{
	size_t offset = bytes.size();
	bytes.resize(offset + sizeof(T));
	memcpy(&bytes[offset], &t, sizeof(T));
}

template <typename T>
inline void AssignWithCast(char* dest, int index, DataFieldComponentType destType, T* src)
{
	switch (destType)
	{
		case DataFieldComponentType::_int: ((int*)dest)[index] = (int)src[index]; break;
		case DataFieldComponentType::_uint16_t: ((uint16_t*)dest)[index] = (uint16_t)src[index]; break;
		case DataFieldComponentType::_uint32_t: ((uint32_t*)dest)[index] = (uint32_t)src[index]; break;
		case DataFieldComponentType::_float: ((float*)dest)[index] = (float)src[index]; break;
	}
}

inline void SetToZero(char* dest, int index, DataFieldComponentType destType)
{
	switch (destType)
	{
		case DataFieldComponentType::_int: ((int*)dest)[index] = 0; break;
		case DataFieldComponentType::_uint16_t: ((uint16_t*)dest)[index] = 0; break;
		case DataFieldComponentType::_uint32_t: ((uint32_t*)dest)[index] = 0; break;
		case DataFieldComponentType::_float: ((float*)dest)[index] = 0.0f; break;
	}
}

static std::vector<char> LoadTypedBufferPly(const GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc, const PLYCache::PLYData& plyData)
{
	std::vector<char> ret;

	// Get the "vertex" element group from the plyData
	size_t vertexElementGroupIndex = ~0;
	for (size_t i = 0; i < plyData.elementGroups.size(); ++i)
	{
		if (plyData.elementGroups[i].name == "vertex")
		{
			vertexElementGroupIndex = i;
			break;
		}
	}
	if (vertexElementGroupIndex >= plyData.elementGroups.size())
		return ret;
	const PLYCache::ElementGroup& vertexElementGroup = plyData.elementGroups[vertexElementGroupIndex];

	// Get the number of components to copy
	unsigned int copyComponentCount = 0;
	const auto& typeInfo = DataFieldTypeInfo(desc.buffer.type);
	copyComponentCount = min((unsigned int)typeInfo.componentCount, (unsigned int)vertexElementGroup.properties.size());

	// Load each vertex
	{
		for (unsigned int vertIndex = 0; vertIndex < vertexElementGroup.count; ++vertIndex)
		{
			const unsigned char* src = &vertexElementGroup.data[vertIndex * vertexElementGroup.propertiesSizeBytes];

			unsigned int srcPropertyIndex = 0;
			for (int componentIndex = 0; componentIndex < typeInfo.componentCount; ++componentIndex)
			{
				switch (typeInfo.componentType2)
				{
					case DataFieldType::Int:
					{
						int val = 0;
						if (srcPropertyIndex < copyComponentCount)
							src = PLYCache::ReadFromBinaryAndCastTo(src, vertexElementGroup.properties[srcPropertyIndex].type, val);
						AppendBytes(ret, val);
						break;
					}
					case DataFieldType::Uint:
					{
						unsigned int val = 0;
						if (srcPropertyIndex < copyComponentCount)
							src = PLYCache::ReadFromBinaryAndCastTo(src, vertexElementGroup.properties[srcPropertyIndex].type, val);
						AppendBytes(ret, val);
						break;
					}
					case DataFieldType::Float:
					{
						float val = 0.0f;
						if (srcPropertyIndex < copyComponentCount)
							src = PLYCache::ReadFromBinaryAndCastTo(src, vertexElementGroup.properties[srcPropertyIndex].type, val);
						AppendBytes(ret, val);
						break;
					}
					default: return ret;
				}
				srcPropertyIndex++;
			}
		}
	}

	return ret;
}

static std::vector<char> LoadStructuredBufferPly(const GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc, const RenderGraph& renderGraph, const PLYCache::PLYData& plyData)
{
	std::vector<char> ret;

	// Get the "vertex" element group from the plyData
	size_t vertexElementGroupIndex = ~0;
	for (size_t i = 0; i < plyData.elementGroups.size(); ++i)
	{
		if (plyData.elementGroups[i].name == "vertex")
		{
			vertexElementGroupIndex = i;
			break;
		}
	}
	if (vertexElementGroupIndex >= plyData.elementGroups.size())
		return ret;
	const PLYCache::ElementGroup& vertexElementGroup = plyData.elementGroups[vertexElementGroupIndex];

	// Get the number of components to copy
	unsigned int copyComponentCount = 0;
	const Struct& structDesc = renderGraph.structs[desc.buffer.structIndex];
	{
		unsigned int componentCount = 0;
		for (const StructField& field : structDesc.fields)
		{
			const auto& fieldInfo = DataFieldTypeInfo(field.type);
			componentCount += fieldInfo.componentCount;
		}

		copyComponentCount = min(componentCount, (unsigned int)vertexElementGroup.properties.size());
	}

	// Load each vertex
	{
		for (unsigned int vertIndex = 0; vertIndex < vertexElementGroup.count; ++vertIndex)
		{
			const unsigned char* src = &vertexElementGroup.data[vertIndex * vertexElementGroup.propertiesSizeBytes];

			unsigned int srcPropertyIndex = 0;
			for (const StructField& field : structDesc.fields)
			{
				const auto& fieldInfo = DataFieldTypeInfo(field.type);
				for (int componentIndex = 0; componentIndex < fieldInfo.componentCount; ++componentIndex)
				{
					switch (fieldInfo.componentType2)
					{
						case DataFieldType::Int:
						{
							int val = 0;
							if (srcPropertyIndex < copyComponentCount)
								src = PLYCache::ReadFromBinaryAndCastTo(src, vertexElementGroup.properties[srcPropertyIndex].type, val);
							AppendBytes(ret, val);
							break;
						}
						case DataFieldType::Uint:
						{
							unsigned int val = 0;
							if (srcPropertyIndex < copyComponentCount)
								src = PLYCache::ReadFromBinaryAndCastTo(src, vertexElementGroup.properties[srcPropertyIndex].type, val);
							AppendBytes(ret, val);
							break;
						}
						case DataFieldType::Float:
						{
							float val = 0.0f;
							if (srcPropertyIndex < copyComponentCount)
								src = PLYCache::ReadFromBinaryAndCastTo(src, vertexElementGroup.properties[srcPropertyIndex].type, val);
							AppendBytes(ret, val);
							break;
						}
						default: return ret;
					}
					srcPropertyIndex++;
				}
			}
		}
	}

	return ret;
}

static std::vector<char> LoadStructuredBuffer(const GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc, const RenderGraph &renderGraph, const std::vector<FlattenedVertex>& flattenedVertices)
{
    // Get the transforms
    DirectX::XMMATRIX transform, transformInverseTranspose;
    {
        memcpy(&transform, desc.buffer.GeometryTransform, sizeof(transform));
        transformInverseTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, transform));
    }

    // See if we need to transform geometry
    bool needsTransformation = false;
    {
        const float* gt = desc.buffer.GeometryTransform;
        needsTransformation =
            gt[0]  != 1.0f || gt[1]  != 0.0f || gt[2]  != 0.0f || gt[3]  != 0.0f ||
            gt[4]  != 0.0f || gt[5]  != 1.0f || gt[6]  != 0.0f || gt[7]  != 0.0f ||
            gt[8]  != 0.0f || gt[9]  != 0.0f || gt[10] != 1.0f || gt[11] != 0.0f ||
            gt[12] != 0.0f || gt[13] != 0.0f || gt[14] != 0.0f || gt[15] != 1.0f;
    }

    // Transform position
    auto transformPosition = [&](float* position)
    {
        // Transform by matrix
        DirectX::XMVECTOR v = DirectX::XMVectorSet(position[0], position[1], position[2], 1.0f);
        DirectX::XMVECTOR result = DirectX::XMVector4Transform(v, transform);
        position[0] = result.m128_f32[0];
        position[1] = result.m128_f32[1];
        position[2] = result.m128_f32[2];
    };

    // Transform tangent
    auto transformTangent = [&](float* tangent)
    {
        // Transform by matrix and normalize
        DirectX::XMVECTOR v = DirectX::XMVectorSet(tangent[0], tangent[1], tangent[2], 0.0f);
        DirectX::XMVECTOR result = DirectX::XMVector4Transform(v, transform);
        result = DirectX::XMVector3Normalize(result);
        tangent[0] = result.m128_f32[0];
        tangent[1] = result.m128_f32[1];
        tangent[2] = result.m128_f32[2];
    };

    // Transform normal
    auto transformNormal = [&](float* normal)
    {
        // Transform by matrix inverse transpose and normalize
        DirectX::XMVECTOR v = DirectX::XMVectorSet(normal[0], normal[1], normal[2], 0.0f);
        DirectX::XMVECTOR result = DirectX::XMVector4Transform(v, transformInverseTranspose);
        result = DirectX::XMVector3Normalize(result);
        normal[0] = result.m128_f32[0];
        normal[1] = result.m128_f32[1];
        normal[2] = result.m128_f32[2];
    };

	// Allocate space to hold the results
	const Struct& structDesc = renderGraph.structs[desc.buffer.structIndex];
	size_t vertexCount = flattenedVertices.size();
	size_t destVertexSize = structDesc.sizeInBytes;
	std::vector<char> ret(destVertexSize * vertexCount, 0);

	// gather the data
	size_t offset = 0;
	for (const StructField& field : structDesc.fields)
	{
		DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(field.type);

		// Note: There are two loops in each case.
		// 1) The first loop copies data from flattenedVertices into the structured buffer "ret".  If ret doesn't have enough channels, it only takes as many values as it can.
		// 2) If ret had more channels than it should, it fills it with zeros to ensure initialization.
		switch (field.semantic)
		{
			case StructFieldSemantic::Position:
			{
				for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
                    Vec3 position = flattenedVertices[vertexIndex].position;
                    if (needsTransformation)
                        transformPosition(position.data());

					int index = 0;
					while (index < min(3, typeInfo.componentCount))
					{
                        AssignWithCast(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType, position.data());
                        index++;
					}

					while (index < typeInfo.componentCount)
					{
						SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
						index++;
					}
				}
				break;
			}
			case StructFieldSemantic::Color:
			{
				for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					int index = 0;
					while (index < min(4, typeInfo.componentCount))
					{
						AssignWithCast(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType, flattenedVertices[vertexIndex].albedo.data());
						index++;
					}

					while (index < typeInfo.componentCount)
					{
						SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
						index++;
					}
				}
				break;
			}
			case StructFieldSemantic::Normal:
			{
				for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
                    Vec3 normal = flattenedVertices[vertexIndex].normal;
                    if (needsTransformation)
                        transformNormal(normal.data());

					int index = 0;
					while (index < min(3, typeInfo.componentCount))
					{
						AssignWithCast(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType, normal.data());
						index++;
					}

					while (index < typeInfo.componentCount)
					{
						SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
						index++;
					}
				}
				break;
			}
			case StructFieldSemantic::Tangent:
			{
				for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
                    Vec4 tangent = flattenedVertices[vertexIndex].tangent;
                    if (needsTransformation)
                        transformTangent(tangent.data());

					int index = 0;
					while (index < min(4, typeInfo.componentCount))
					{
						AssignWithCast(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType, tangent.data());
						index++;
					}

					while (index < typeInfo.componentCount)
					{
						SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
						index++;
					}
				}
				break;
			}
			case StructFieldSemantic::UV:
			{
				if (field.semanticIndex < _countof(FlattenedVertex::uvs))
				{
					for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
					{
						int index = 0;
						while (index < min(2, typeInfo.componentCount))
						{
							AssignWithCast(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType, flattenedVertices[vertexIndex].uvs[field.semanticIndex].data());
							index++;
						}

						while (index < typeInfo.componentCount)
						{
							SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
							index++;
						}
					}
				}
				else
				{
					// invalid semantic index gets all zeros
					for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
					{
						int index = 0;
						while (index < typeInfo.componentCount)
						{
							SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
							index++;
						}
					}
				}
				break;
			}
			case StructFieldSemantic::MaterialID:
			{
				for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					int index = 0;
					while (index < min(1, typeInfo.componentCount))
					{
						AssignWithCast(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType, &flattenedVertices[vertexIndex].materialID);
						index++;
					}

					while (index < typeInfo.componentCount)
					{
						SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
						index++;
					}
				}
				break;
			}
			case StructFieldSemantic::ShapeID:
			{
				for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					int index = 0;
					while (index < min(1, typeInfo.componentCount))
					{
						AssignWithCast(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType, &flattenedVertices[vertexIndex].shapeIndex);
						index++;
					}

					while (index < typeInfo.componentCount)
					{
						SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
						index++;
					}
				}
				break;
			}
			default:
			{
				for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
					for (int index = 0; index < typeInfo.componentCount; ++index)
						SetToZero(&ret[vertexIndex * destVertexSize + offset], index, typeInfo.componentType);
			}
		}

		offset += field.sizeInBytes;
	}

	return ret;
}

static std::vector<char> LoadTypedBuffer(const GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc, const std::vector<FlattenedVertex>& flattenedVertices)
{
    // Get the transforms
    DirectX::XMMATRIX transform, transformInverseTranspose;
    {
        memcpy(&transform, desc.buffer.GeometryTransform, sizeof(transform));
        transformInverseTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, transform));
    }

    // See if we need to transform geometry
    bool needsTransformation = false;
    {
        const float* gt = desc.buffer.GeometryTransform;
        needsTransformation =
            gt[0]  != 1.0f || gt[1]  != 0.0f || gt[2]  != 0.0f || gt[3]  != 0.0f ||
            gt[4]  != 0.0f || gt[5]  != 1.0f || gt[6]  != 0.0f || gt[7]  != 0.0f ||
            gt[8]  != 0.0f || gt[9]  != 0.0f || gt[10] != 1.0f || gt[11] != 0.0f ||
            gt[12] != 0.0f || gt[13] != 0.0f || gt[14] != 0.0f || gt[15] != 1.0f;
    }

    // Transform position
    auto transformPosition = [&](float* position)
    {
        // Transform by matrix
        DirectX::XMVECTOR v = DirectX::XMVectorSet(position[0], position[1], position[2], 1.0f);
        DirectX::XMVECTOR result = DirectX::XMVector4Transform(v, transform);
        position[0] = result.m128_f32[0];
        position[1] = result.m128_f32[1];
        position[2] = result.m128_f32[2];
    };

	// Allocate space to hold the results
	DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(desc.buffer.type);
	size_t vertexCount = flattenedVertices.size();
	size_t destVertexSize = typeInfo.typeBytes;
	std::vector<char> ret(destVertexSize * vertexCount, 0);

	// gather the data
	for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
        Vec3 position = flattenedVertices[vertexIndex].position;
        if (needsTransformation)
            transformPosition(position.data());

		int index = 0;
		while (index < min(3, typeInfo.componentCount))
		{
			AssignWithCast(&ret[vertexIndex * destVertexSize], index, typeInfo.componentType, position.data());
			index++;
		}

		while (index < typeInfo.componentCount)
		{
			SetToZero(&ret[vertexIndex * destVertexSize], index, typeInfo.componentType);
			index++;
		}
	}

	return ret;
}

static std::vector<char> LoadCSVStructuredBuffer(const GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc, const RenderGraph& renderGraph, const char* csvData)
{
	std::vector<char> ret;

	int offset = 0;
	int fieldIndex = 0;
	int componentIndex = 0;

	const Struct& s = renderGraph.structs[desc.buffer.structIndex];

	bool success = ParseCSV::ForEachValue(csvData, desc.buffer.CSVHeaderRow,
		[&](int tokenIndex, const char* token)
		{
			// get the field and field type info
			const StructField& field = s.fields[fieldIndex];
			DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(field.type);

			// Advance componentIndex and fieldIndex as appropriate, for next iteration
			componentIndex++;
			if (componentIndex >= typeInfo.componentCount)
			{
				componentIndex = 0;
				fieldIndex = (fieldIndex + 1) % s.fields.size();
			}

			// read a value
			switch (typeInfo.componentType)
			{
				case DataFieldComponentType::_int:
				{
					int v;
					if (field.enumIndex != -1)
					{
						bool enumFound = false;
						v = -1;
						for (const EnumItem& item : renderGraph.enums[field.enumIndex].items)
						{
							v++;
							if (!_stricmp(item.displayLabel.c_str(), token))
							{
								enumFound = true;
								break;
							}
						}

						if (!enumFound)
							return false;
					}
					else
					{
						if (sscanf_s(token, "%i", &v) != 1)
							return false;
					}
					AppendBytes(ret, v);
					return true;
				}
				case DataFieldComponentType::_uint16_t:
				{
					unsigned int v;
					if (sscanf_s(token, "%u", &v) != 1)
						return false;
					AppendBytes(ret, (uint16_t)v);
					return true;
				}
				case DataFieldComponentType::_uint32_t:
				{
					unsigned int v;
					if (field.type == DataFieldType::Bool)
					{
						if (!_stricmp(token, "true") || !_stricmp(token, "1"))
							v = 1;
						else if (!_stricmp(token, "false") || !_stricmp(token, "0"))
							v = 0;
						else
							return false;
					}
					else
					{
						if (sscanf_s(token, "%u", &v) != 1)
							return false;
					}
					AppendBytes(ret, v);
					return true;
				}
				case DataFieldComponentType::_float:
				{
					float v;
					if (sscanf_s(token, "%f", &v) != 1)
						return false;
					AppendBytes(ret, v);
					return true;
				}
			}
			return false;
		}
	);

	if (!success)
		ret.clear();

	return ret;
}

static std::vector<char> LoadCSVTypedBuffer(const GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc, const char* csvData)
{
	std::vector<char> ret;

	int offset = 0;
	DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(desc.buffer.type);

	bool success = ParseCSV::ForEachValue(csvData, desc.buffer.CSVHeaderRow,
		[&](int tokenIndex, const char* token)
		{
			// skip empty tokens, caused by trailing commas
			if (token[0] == 0)
				return true;

			switch (typeInfo.componentType)
			{
				case DataFieldComponentType::_int:
				{
					int v;
					if (sscanf_s(token, "%i", &v) != 1)
						return false;
					AppendBytes(ret, v);
					return true;
				}
				case DataFieldComponentType::_uint16_t:
				{
					unsigned int v;
					if (sscanf_s(token, "%u", &v) != 1)
						return false;
					AppendBytes(ret, (uint16_t)v);
					return true;
				}
				case DataFieldComponentType::_uint32_t:
				{
					unsigned int v;
					if (sscanf_s(token, "%u", &v) != 1)
						return false;
					AppendBytes(ret, v);
					return true;
				}
				case DataFieldComponentType::_float:
				{
					float v;
					if (sscanf_s(token, "%f", &v) != 1)
						return false;
					AppendBytes(ret, v);
					return true;
				}
			}
			return false;
		}
	);

	if (!success)
		ret.clear();

	return ret;
}

void GigiInterpreterPreviewWindowDX12::CooperativeVectorAdjustBufferSize(const CooperativeVectorData& cvData, int& size)
{
    if (!cvData.convert)
        return;

    if (!m_previewDevice)
    {
        m_logFn(LogLevel::Error, "CooperativeVectorConvert() failed cooperative vectors are not enabled.  Please see UserDocumentation/CooperativeVectors.pdf for info on enabling them.");
        return;
    }

    // Gather the conversion data
    D3D12_LINEAR_ALGEBRA_DATATYPE destType;
    CooperativeVectorDataTypeToD3D12_LINEAR_ALGEBRA_DATATYPE(cvData.destType, destType);

    D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT destLayout;
    CooperativeVectorBufferLayoutToD3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT(cvData.destLayout, destLayout);
    bool destLayoutOptimized = CooperativeVectorBufferLayoutIsOptimized(cvData.destLayout);

    unsigned int width = cvData.width;
    unsigned int height = cvData.height;

    size_t destTypeSize = 0;
    D3D12_LINEAR_ALGEBRA_DATATYPE_Size(destType, destTypeSize);

    // Figure out the size of our destination matrix
    D3D12_LINEAR_ALGEBRA_MATRIX_CONVERSION_DEST_INFO conversionDestInfo =
    {
        destLayoutOptimized ? 0 : (unsigned int)(width * height * destTypeSize),
        destLayout,
        destLayoutOptimized ? 0 : (unsigned int)(width * destTypeSize),
        width,
        height,
        destType
    };
    m_previewDevice->GetLinearAlgebraMatrixConversionDestinationInfo(&conversionDestInfo);

    size = max(size, (int)conversionDestInfo.DestSize);
}

void GigiInterpreterPreviewWindowDX12::CooperativeVectorConvert(ID3D12Resource* resource, D3D12_RESOURCE_STATES resourceState, const CooperativeVectorData& cvData)
{
	if (!cvData.convert)
		return;

	if (!m_previewDevice)
	{
		m_logFn(LogLevel::Error, "CooperativeVectorConvert() failed cooperative vectors are not enabled.  Please see UserDocumentation/CooperativeVectors.pdf for info on enabling them.");
		return;
	}

	// Make a temporary buffer to convert to
	// Conversion dest buffer must be in D3D12_RESOURCE_STATE_UNORDERED_ACCESS state
	D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
	ID3D12Resource* tempResource = CreateBuffer(m_device, (unsigned int)resourceDesc.Width, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, "CooperativeVectorConvert() tempResource");
	m_transitions.Track(TRANSITION_DEBUG_INFO(tempResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	// Delay destroy the tempResource since we only need it here, within this function
	m_delayedRelease.Add(tempResource);

	// Gather the conversion data
	D3D12_LINEAR_ALGEBRA_DATATYPE srcType;
	CooperativeVectorDataTypeToD3D12_LINEAR_ALGEBRA_DATATYPE(cvData.srcType, srcType);

	D3D12_LINEAR_ALGEBRA_DATATYPE destType;
	CooperativeVectorDataTypeToD3D12_LINEAR_ALGEBRA_DATATYPE(cvData.destType, destType);

	D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT srcLayout;
	CooperativeVectorBufferLayoutToD3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT(cvData.srcLayout, srcLayout);

	D3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT destLayout;
	CooperativeVectorBufferLayoutToD3D12_LINEAR_ALGEBRA_MATRIX_LAYOUT(cvData.destLayout, destLayout);
	bool destLayoutOptimized = CooperativeVectorBufferLayoutIsOptimized(cvData.destLayout);

	unsigned int width = cvData.width;
	unsigned int height = cvData.height;

	size_t srcTypeSize = 0;
	D3D12_LINEAR_ALGEBRA_DATATYPE_Size(srcType, srcTypeSize);

	size_t destTypeSize = 0;
	D3D12_LINEAR_ALGEBRA_DATATYPE_Size(destType, destTypeSize);

	// Set up conversion info struct
	D3D12_LINEAR_ALGEBRA_MATRIX_CONVERSION_INFO infoDesc =
	{
		// DestInfo
		{
			destLayoutOptimized ? 0 : (unsigned int)(width * height * destTypeSize),
			destLayout,
			destLayoutOptimized ? 0 : (unsigned int)(width * destTypeSize),
			width,
			height,
			destType
		},

		// SrcInfo
		{
			(unsigned int)(width * height * srcTypeSize),
			srcType,
			srcLayout,
			(unsigned int)(width * srcTypeSize)
		},

		// DataDesc
		{
			tempResource->GetGPUVirtualAddress(),
			resource->GetGPUVirtualAddress()
		}
	};
	m_previewDevice->GetLinearAlgebraMatrixConversionDestinationInfo(&infoDesc.DestInfo);

	// make sure the resource is in the correct state
	// Conversion source buffer must be in D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE state
	if (!m_transitions.IsTracked(resource))
		m_transitions.Track(TRANSITION_DEBUG_INFO(resource, resourceState));
	m_transitions.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	m_transitions.Flush(m_commandList);

	// Convert
	m_previewCommandList->ConvertLinearAlgebraMatrix(&infoDesc, 1);

	// Copy from tempResource to resource
	m_transitions.Transition(TRANSITION_DEBUG_INFO(tempResource, D3D12_RESOURCE_STATE_COPY_SOURCE));
	m_transitions.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_COPY_DEST));
	m_transitions.Flush(m_commandList);
	m_commandList->CopyResource(resource, tempResource);
}

bool GigiInterpreterPreviewWindowDX12::OnNodeActionImported(const RenderGraphNode_Resource_Buffer& node, RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData, NodeAction nodeAction)
{
	// If this resource is imported, add it to the list of imported resources.
	// Let resource creation happen during execute, since the desired format or size can change due to runtime conditions.
	if (nodeAction == NodeAction::Init)
	{
		if (m_importedResources.count(node.name) == 0)
			m_importedResources[node.name] = ImportedResourceDesc(false);
		m_importedResources[node.name].stale = false;
		m_importedResources[node.name].nodeIndex = node.nodeIndex;
		m_importedResources[node.name].resourceIndex = 0; // corresponds to the initial state resource being the first viewable resource
		return true;
	}

	if (nodeAction == NodeAction::Execute)
	{
		ImportedResourceDesc& desc = m_importedResources[node.name];

		bool hasFileName = !desc.buffer.fileName.empty();
		bool hasEnoughInfoToCreate = false;
		if (desc.buffer.structIndex != -1 || desc.buffer.type != DataFieldType::Count)
		{
			if (hasFileName || desc.buffer.count > 0)
			{
				hasEnoughInfoToCreate = true;
			}
		}

		if (desc.state == ImportedResourceState::dirty && !hasEnoughInfoToCreate)
		{
			std::ostringstream ss;
			ss << "Not enough info to create buffer.";
			runtimeData.m_renderGraphText = ss.str();
            runtimeData.m_inErrorState = true;
		}

		// (Re)Create a buffer, as necessary
		// if the texture isn't dirty, nothing to do
		// if the format doesn't have a type or struct specified, wait until it does
		if (desc.state == ImportedResourceState::dirty && hasEnoughInfoToCreate)
		{
			// Release any resources which may have previously existed
			runtimeData.Release(*this);

			// Load data from a file if we should
			std::vector<char> rawBytes;
			if (hasFileName)
			{
				std::filesystem::path p(desc.buffer.fileName);
				FileWatchOwner fileWatchOwner = FileWatchOwner::FileCache;
				if (p.extension() == ".obj")
					fileWatchOwner = FileWatchOwner::ObjCache;
				else if (p.extension() == ".fbx")
					fileWatchOwner = FileWatchOwner::FBXCache;
				else if (p.extension() == ".ply")
					fileWatchOwner = FileWatchOwner::PLYCache;

				m_fileWatcher.Add(desc.buffer.fileName.c_str(), fileWatchOwner);

				if (p.extension() == ".ply")
				{
					// Load the ply data
					PLYCache::PLYData plyData = m_plys.GetFlattened(m_files, desc.buffer.fileName.c_str());

					if (!plyData.warn.empty())
						m_logFn(LogLevel::Warn, "Loading Obj for buffer \"%s\": %s", node.name.c_str(), plyData.warn.c_str());
					if (!plyData.error.empty())
						m_logFn(LogLevel::Warn, "Loading Obj for buffer \"%s\": %s", node.name.c_str(), plyData.error.c_str());

					// load the data if it's valid
					if (plyData.valid)
					{
						// Load a typed buffer
						if (desc.buffer.type != DataFieldType::Count)
							rawBytes = LoadTypedBufferPly(desc, plyData);
						// Load a structured buffer
						else
							rawBytes = LoadStructuredBufferPly(desc, m_renderGraph, plyData);
					}
				}
				else if (p.extension() == ".obj")
				{
					// Load the obj data
					std::vector<char> ret;
					const ObjCache::OBJData& objData = m_objs.Get(m_files, desc.buffer.fileName.c_str());
					if (!objData.warn.empty())
						m_logFn(LogLevel::Warn, "Loading Obj for buffer \"%s\": %s", node.name.c_str(), objData.warn.c_str());
					if (!objData.error.empty())
						m_logFn(LogLevel::Warn, "Loading Obj for buffer \"%s\": %s", node.name.c_str(), objData.error.c_str());

					if (objData.valid)
					{
						// store which shapes are in the mesh
						runtimeData.shapes.clear();
						for (auto shape : objData.shapes)
							runtimeData.shapes.push_back(shape.name);

						// find out which materials are used.
						std::unordered_set<int> materialsUsed;
						for (const auto& vertex : objData.flattenedVertices)
							materialsUsed.insert(vertex.materialID);

						// store which materials are referenced by the mesh, and which are actually used.
						runtimeData.materials.clear();
						for (int materialIndex = 0; materialIndex < (int)objData.materials.size(); ++materialIndex)
						{
							const tinyobj::material_t& material = objData.materials[materialIndex];

							bool used = (materialsUsed.count(materialIndex) > 0);

							runtimeData.materials.push_back(RuntimeTypes::RenderGraphNode_Resource_Buffer::MaterialInfo(material.name, used));
						}

						// Load a typed buffer
						if (desc.buffer.type != DataFieldType::Count)
							rawBytes = LoadTypedBuffer(desc, objData.flattenedVertices);
						// Load a structured buffer
						else
							rawBytes = LoadStructuredBuffer(desc, m_renderGraph, objData.flattenedVertices);
					}
				}
				else if (p.extension() == ".fbx")
				{
					// Load the fbx data
					std::vector<char> ret;
					const FBXCache::FBXData& fbxData = m_fbxs.Get(m_files, desc.buffer.fileName.c_str());
					if (!fbxData.warn.empty())
						m_logFn(LogLevel::Warn, "Loading fbx for buffer \"%s\": %s", node.name.c_str(), fbxData.warn.c_str());
					if (!fbxData.error.empty())
						m_logFn(LogLevel::Warn, "Loading fbx for buffer \"%s\": %s", node.name.c_str(), fbxData.error.c_str());

					if (fbxData.valid)
					{
						// Load a typed buffer
						if (desc.buffer.type != DataFieldType::Count)
							rawBytes = LoadTypedBuffer(desc, fbxData.flattenedVertices);
						// Load a structured buffer
						else
							rawBytes = LoadStructuredBuffer(desc, m_renderGraph, fbxData.flattenedVertices);
					}
				}
				else if (p.extension() == ".csv")
				{
					const FileCache::File& file = m_files.Get(desc.buffer.fileName.c_str());

					// Load a typed buffer
					if (desc.buffer.type != DataFieldType::Count)
						rawBytes = LoadCSVTypedBuffer(desc, file.GetBytes());
					// Load a structured buffer
					else
						rawBytes = LoadCSVStructuredBuffer(desc, m_renderGraph, file.GetBytes());
				}
				// Anything else, read as binary
				else
				{
					const FileCache::File& file = m_files.Get(desc.buffer.fileName.c_str());
					rawBytes.resize(file.GetSize());
					memcpy(rawBytes.data(), file.GetBytes(), file.GetSize());
				}

				if (rawBytes.size() == 0)
				{
					m_logFn(LogLevel::Error, "Could not load file \"%s\", or it was empty", desc.buffer.fileName.c_str());
					desc.state = ImportedResourceState::failed;
					return false;
				}
			}

			if (desc.buffer.type != DataFieldType::Count)
			{
				DataFieldTypeInfoStructDX12 typeInfo = DataFieldTypeInfoDX12(desc.buffer.type);

				int bufferCount = (rawBytes.size() > 0) ? ((int)rawBytes.size() / typeInfo.typeBytes) : desc.buffer.count;

				runtimeData.m_format = typeInfo.typeFormat;
				runtimeData.m_formatCount = typeInfo.typeFormatCount;
				runtimeData.m_structIndex = -1;
				runtimeData.m_stride = 0;
				runtimeData.m_count = bufferCount;
				runtimeData.m_size = typeInfo.componentBytes * typeInfo.componentCount * bufferCount;
			}
			else
			{
				int structSizeBytes = (int)m_renderGraph.structs[desc.buffer.structIndex].sizeInBytes;

				int bufferCount = (rawBytes.size() > 0) ? ((int)rawBytes.size() / structSizeBytes) : desc.buffer.count;

				runtimeData.m_format = DXGI_FORMAT_UNKNOWN;
				runtimeData.m_formatCount = 1;
				runtimeData.m_structIndex = desc.buffer.structIndex;
				runtimeData.m_stride = structSizeBytes;
				runtimeData.m_count = bufferCount;
				runtimeData.m_size = runtimeData.m_stride * bufferCount;
			}

			// Vertex buffers need to be aligned to 256
			if (node.accessedAs & (1 << ((unsigned int)(ShaderResourceAccessType::RTScene) | (unsigned int)(ShaderResourceAccessType::VertexBuffer))))
				runtimeData.m_size = ALIGN(256, runtimeData.m_size);

			if (runtimeData.m_count == 0)
			{
				m_logFn(LogLevel::Error, "Could not create buffer for node \"%s\", it had a count of 0.", node.name.c_str());
				desc.state = ImportedResourceState::failed;
				return false;
			}

            CooperativeVectorAdjustBufferSize(desc.buffer.cvData, runtimeData.m_size);

			// Make a resource for the "live" texture which may be modified during running, and also for the initial state.
			D3D12_RESOURCE_FLAGS resourceFlags = ShaderResourceAccessToD3D12_RESOURCE_FLAGs(node.accessedAs);
			std::string nodeNameInitialState = node.name + " Initial State";
			runtimeData.m_resourceInitialState = CreateBuffer(m_device, runtimeData.m_size, resourceFlags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, nodeNameInitialState.c_str());
			runtimeData.m_resource = CreateBuffer(m_device, runtimeData.m_size, resourceFlags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, node.name.c_str());

			// Make a vertex buffer view if we should
			if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::VertexBuffer))
			{
				runtimeData.m_vertexBufferView.BufferLocation = runtimeData.m_resource->GetGPUVirtualAddress();
				if (desc.buffer.structIndex != -1)
					runtimeData.m_vertexBufferView.StrideInBytes = (int)m_renderGraph.structs[desc.buffer.structIndex].sizeInBytes;
				else
					runtimeData.m_vertexBufferView.StrideInBytes = DataFieldTypeInfo(desc.buffer.type).typeBytes;
				runtimeData.m_vertexBufferView.SizeInBytes = runtimeData.m_size;
			}

			// Make an index buffer if we should
			if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::IndexBuffer))
			{
				if (desc.buffer.structIndex != -1)
				{
					m_logFn(LogLevel::Error, "buffer \"%s\" is used as an instance buffer so cannot use a struct format.", node.name.c_str());
					desc.state = ImportedResourceState::failed;
					return false;
				}

				runtimeData.m_indexBufferView.BufferLocation = runtimeData.m_resource->GetGPUVirtualAddress();
				runtimeData.m_indexBufferView.Format = DataFieldTypeInfoDX12(desc.buffer.type).typeFormat;
				runtimeData.m_indexBufferView.SizeInBytes = runtimeData.m_size;
			}

			// track the new resources for state transitions
			m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COMMON));
			m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COMMON));

			// Upload raw bytes into our initial state if we have raw bytes
			if (!rawBytes.empty())
			{
				// Make an upload buffer
				UploadBufferTracker::Buffer* uploadBuffer = m_uploadBufferTracker.GetBuffer(m_device, runtimeData.m_size, false);

				// Copy the data into the upload buffer
				{
					unsigned char* dest = nullptr;
					HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&dest));
					if (hr)
						return false;
					memcpy(dest, rawBytes.data(), runtimeData.m_size);
					uploadBuffer->buffer->Unmap(0, nullptr);
				}

				// Copy the upload buffer into m_resourceInitialState
				m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Flush(m_commandList);
				m_commandList->CopyResource(runtimeData.m_resourceInitialState, uploadBuffer->buffer);

				// Do conversion for cooperative vectors if we should
				CooperativeVectorConvert(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COPY_DEST, desc.buffer.cvData);
			}

			// Note that the resource wants to be reset to the initial state.
			runtimeData.m_resourceWantsReset = true;

			// All is well
			desc.state = ImportedResourceState::clean;
		}
	}

	return true;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeActionNotImported(const RenderGraphNode_Resource_Buffer& node, RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData, NodeAction nodeAction)
{
	if (nodeAction == NodeAction::Execute)
	{
		// failing to get the format and size could be a transient problem
		BufferFormatInfo formatInfo = GetBufferFormatInfo(*this, node);
		int count = GetDesiredCount(*this, node);

		bool hasFormat = (formatInfo.format != DXGI_FORMAT_FORCE_UINT && formatInfo.format != DXGI_FORMAT_UNKNOWN);
		bool hasStruct = (formatInfo.structIndex != -1);
		bool hasSize = (count > 0);

		if (hasSize && (hasFormat || hasStruct))
		{
			// (re) create the resource if we should
			//if (!runtimeData.m_resource ||runtimeData.m_format != desiredFormat || runtimeData.m_size[0] != desiredSize[0] || runtimeData.m_size[1] != desiredSize[1] || runtimeData.m_size[2] != desiredSize[2])
			if (!runtimeData.m_resource ||
				runtimeData.m_format != formatInfo.format || runtimeData.m_formatCount != formatInfo.formatCount ||
				runtimeData.m_structIndex != formatInfo.structIndex ||
				runtimeData.m_count != count)
			{
				// Release any resources which may have previously existed
				runtimeData.Release(*this);

				// Set the data
				if (formatInfo.structIndex != -1)
				{
					runtimeData.m_format = DXGI_FORMAT_UNKNOWN;
					runtimeData.m_formatCount = 1;
					runtimeData.m_structIndex = formatInfo.structIndex;
					runtimeData.m_stride = (int)m_renderGraph.structs[formatInfo.structIndex].sizeInBytes;
					runtimeData.m_count = count;
					runtimeData.m_size = runtimeData.m_stride * count;
				}
				else
				{
					DXGI_FORMAT_Info info = Get_DXGI_FORMAT_Info(formatInfo.format);
					runtimeData.m_format = formatInfo.format;
					runtimeData.m_formatCount = formatInfo.formatCount;
					runtimeData.m_structIndex = -1;
					runtimeData.m_stride = 0;
					runtimeData.m_count = count;
					runtimeData.m_size = info.bytesPerPixel * formatInfo.formatCount * runtimeData.m_count;
				}

				// Vertex buffers need to be aligned to 256
				if (node.accessedAs & (1 << ((unsigned int)(ShaderResourceAccessType::RTScene) | (unsigned int)(ShaderResourceAccessType::VertexBuffer))))
					runtimeData.m_size = ALIGN(256, runtimeData.m_size);

				// Make a resource for the "live" texture which may be modified during running, and also for the initial state.
				D3D12_RESOURCE_FLAGS resourceFlags = ShaderResourceAccessToD3D12_RESOURCE_FLAGs(node.accessedAs);
				std::string nodeNameInitialState = node.name + " Initial State";
				runtimeData.m_resourceInitialState = CreateBuffer(m_device, runtimeData.m_size, resourceFlags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, nodeNameInitialState.c_str());
				runtimeData.m_resource = CreateBuffer(m_device, runtimeData.m_size, resourceFlags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, node.name.c_str());

				// Make a vertex buffer view if we should
				if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::VertexBuffer))
				{
					runtimeData.m_vertexBufferView.BufferLocation = runtimeData.m_resource->GetGPUVirtualAddress();
					if (formatInfo.structIndex != -1)
					{
						runtimeData.m_vertexBufferView.StrideInBytes = (int)m_renderGraph.structs[formatInfo.structIndex].sizeInBytes;
					}
					else
					{
						DXGI_FORMAT_Info info = Get_DXGI_FORMAT_Info(formatInfo.format);
						runtimeData.m_vertexBufferView.StrideInBytes = info.bytesPerPixel * formatInfo.formatCount;
					}
					runtimeData.m_vertexBufferView.SizeInBytes = runtimeData.m_size;
				}

				// track the new resources for state transitions
				m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COMMON));
				m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COMMON));

				// Note that the resource wants to be reset to the initial state.
				runtimeData.m_resourceWantsReset = true;
			}
		}
		else
		{
			std::ostringstream ss;
			ss << "Not enough info to create non imported buffer\n";
			if (!hasSize)
				ss << "Cannot determine size.";
			else
				ss << "Cannot determine format.";
			runtimeData.m_renderGraphText = ss.str();
            runtimeData.m_inErrorState = true;
		}
	}

	return true;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Resource_Buffer& node, RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData, NodeAction nodeAction)
{
	ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, true);

	// Make sure this buffer is used only as an RTScene, a VertexBuffer, or an IndexBuffer, or none of them, but not multiple.
	if (nodeAction == NodeAction::Init)
	{
		int accessTypeCount = 0;
		if (node.accessedAs & (1 << ((unsigned int)(ShaderResourceAccessType::RTScene))))
			accessTypeCount++;
		if (node.accessedAs & (1 << ((unsigned int)(ShaderResourceAccessType::VertexBuffer))))
			accessTypeCount++;
		if (node.accessedAs & (1 << ((unsigned int)(ShaderResourceAccessType::IndexBuffer))))
			accessTypeCount++;

		if (accessTypeCount > 1)
		{
			m_logFn(LogLevel::Error, "A buffer may only be used as an RTScene, a VertexBuffer, or an IndexBuffer. Buffer \"%s\" was used for multiple.", node.name.c_str());
			return false;
		}
	}

	if (node.visibility == ResourceVisibility::Imported)
	{
		if (!OnNodeActionImported(node, runtimeData, nodeAction))
			return false;
	}
	else
	{
		if (!OnNodeActionNotImported(node, runtimeData, nodeAction))
			return false;
	}

	if (nodeAction == NodeAction::Execute)
	{
		// See if we should reset the resource to the initial state
		bool resourceWantsReset = runtimeData.m_resourceWantsReset;
		runtimeData.m_resourceWantsReset = false;
		if (node.visibility == ResourceVisibility::Imported)
		{
			ImportedResourceDesc& desc = m_importedResources[node.name];
			resourceWantsReset |= desc.resetEveryFrame;
		}
		else
		{
			// A transient resource doesn't have to be transient, but it's safer to force them to be here, for situations on other platforms where they actually will be.
			resourceWantsReset |= node.transient;
		}

		// Do it if so
		if (resourceWantsReset)
		{
			if (runtimeData.m_resource && runtimeData.m_resourceInitialState)
			{
				// transition the resources as needed
				m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COPY_SOURCE));
				m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Flush(m_commandList);

				// copy
				m_commandList->CopyResource(runtimeData.m_resource, runtimeData.m_resourceInitialState);
			}
		}

		// publish the initial and current state as viewable buffers
		runtimeData.HandleViewableBuffer(*this, (node.name + ".resource" + " - Initial State").c_str(), runtimeData.m_resourceInitialState, runtimeData.m_format, runtimeData.m_formatCount, runtimeData.m_structIndex, runtimeData.m_size, runtimeData.m_stride, runtimeData.m_count, true, false, 0, 0, false);
		runtimeData.HandleViewableBuffer(*this, (node.name + ".resource").c_str(), runtimeData.m_resource, runtimeData.m_format, runtimeData.m_formatCount, runtimeData.m_structIndex, runtimeData.m_size, runtimeData.m_stride, runtimeData.m_count, false, resourceWantsReset, 0, 0, false);

		//if (runtimeData.m_instanceDescs)
			//runtimeData.HandleViewableBuffer(*this, (node.name + ".instanceDesc").c_str(), runtimeData.m_instanceDescs, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, -1, sizeof(D3D12_RAYTRACING_INSTANCE_DESC), sizeof(D3D12_RAYTRACING_INSTANCE_DESC), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) / 16);
	}

	return true;
}
