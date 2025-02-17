///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"

#include "DX12Utils/CompileShaders.h"
#include <comdef.h>

#include <sstream>

struct ShaderExport
{
	const Shader* shader = nullptr;
	std::string fileName;
	std::string entryPoint;
	std::wstring entryPointW;
	std::wstring uniqueName;
	ShaderType shaderType;
};

void RuntimeTypes::RenderGraphNode_Action_RayShader::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
	RenderGraphNode_Base::Release(interpreter);

	if (m_stateObject)
	{
		interpreter.m_delayedRelease.Add(m_stateObject);
		m_stateObject = nullptr;
	}

	if (m_rootSignature)
	{
		interpreter.m_delayedRelease.Add(m_rootSignature);
		m_rootSignature = nullptr;
	}

	if (m_shaderTableRayGen)
	{
		interpreter.m_delayedRelease.Add(m_shaderTableRayGen);
		m_shaderTableRayGen = nullptr;
	}

	if (m_shaderTableMiss)
	{
		interpreter.m_delayedRelease.Add(m_shaderTableMiss);
		m_shaderTableMiss = nullptr;
	}

	if (m_shaderTableHitGroup)
	{
		interpreter.m_delayedRelease.Add(m_shaderTableHitGroup);
		m_shaderTableHitGroup = nullptr;
	}
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_RayShader& node, RuntimeTypes::RenderGraphNode_Action_RayShader& runtimeData, NodeAction nodeAction)
{
	ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, false);

	if (!SupportsRaytracing())
	{
		m_logFn(LogLevel::Error, "Raytracing is not supported\n");
		return false;
	}

	if (nodeAction == NodeAction::Init)
	{
		// make the root signature
		{
			// shader samplers
			std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
			for (const ShaderSampler& sampler : node.shader.shader->samplers)
			{
				D3D12_STATIC_SAMPLER_DESC desc;

				if (!SamplerFilterToD3D12Filter(sampler.filter, desc.Filter))
				{
					m_logFn(LogLevel::Error, "Could not convert SamplerFilterToD3D12Filter\n");
					return false;
				}

				if (!SamplerAddressModeToD3D12AddressMode(sampler.addressMode, desc.AddressU))
				{
					m_logFn(LogLevel::Error, "Could not convert SamplerAddressModeToD3D12AddressMode\n");
					return false;
				}

				desc.AddressV = desc.AddressW = desc.AddressU;

				desc.MipLODBias = 0;
				desc.MaxAnisotropy = 0;
				desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				desc.MinLOD = 0.0f;
				desc.MaxLOD = D3D12_FLOAT32_MAX;
				desc.ShaderRegister = (UINT)samplers.size();
				desc.RegisterSpace = 0;
				desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

				samplers.push_back(desc);
			}

			// Descriptor table
			std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
			for (const ShaderResource& resource : node.shader.shader->resources)
			{
				D3D12_DESCRIPTOR_RANGE desc;

				switch (resource.access)
				{
					case ShaderResourceAccessType::UAV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
					case ShaderResourceAccessType::RTScene:
					case ShaderResourceAccessType::SRV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
					case ShaderResourceAccessType::CBV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
					default:
					{
						m_logFn(LogLevel::Error, "Unhandled resource access type (%s)\n", EnumToString(resource.access));
						return false;
					}
				}

				desc.NumDescriptors = 1;
				desc.BaseShaderRegister = resource.registerIndex;
				desc.RegisterSpace = 0;
				desc.OffsetInDescriptorsFromTableStart = (UINT)ranges.size();

				ranges.push_back(desc);
			}

			// Root parameter
			D3D12_ROOT_PARAMETER rootParam;
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam.DescriptorTable.NumDescriptorRanges = (UINT)ranges.size();
			rootParam.DescriptorTable.pDescriptorRanges = ranges.data();

			D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
			rootDesc.NumParameters = 1;
			rootDesc.pParameters = &rootParam;
			rootDesc.NumStaticSamplers = (UINT)samplers.size();
			rootDesc.pStaticSamplers = samplers.data();
			rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

			// Create it
			ID3DBlob* sig = nullptr;
			ID3DBlob* error = nullptr;
			HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
			if (FAILED(hr))
			{
				const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
				if (errorMsg)
					m_logFn(LogLevel::Error, "Could not serialize root signature: %s", errorMsg);
				else
					m_logFn(LogLevel::Error, "Could not serialize root signature");
				if (sig) sig->Release();
				if (error) error->Release();
				return false;
			}

			hr = m_device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&runtimeData.m_rootSignature));
			if (FAILED(hr))
			{
				const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
				if (errorMsg)
					m_logFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
				else
					m_logFn(LogLevel::Error, "Could not create root signature");
				if (sig) sig->Release();
				if (error) error->Release();
				return false;
			}

			if (sig)
				sig->Release();

			if (error)
				error->Release();

			// name the root signature for debuggers
			runtimeData.m_rootSignature->SetName(ToWideString(node.name.c_str()).c_str());
		}

		// Compile the shaders
		std::vector<ShaderExport> shaderExports;
		std::vector<std::vector<unsigned char>> allShaderCode;
		int countRaygen = 1;
		int countMiss = 0;
		int countHitGroups = (int)node.shader.shader->Used_RTHitGroupIndex.size(); // How many hit groups used by this shader
		{
			// gather all the shaders involved
			for (const Shader& shader : m_renderGraph.shaders)
			{
				if (shader.type != ShaderType::RTClosestHit && shader.type != ShaderType::RTMiss &&
					shader.type != ShaderType::RTAnyHit && shader.type != ShaderType::RTIntersection)
					continue;

				// If this shader is referenced as RTMissIndex, include it
				bool includeShader = false;
				for (const std::string& RTMissShader : node.shader.shader->Used_RTMissIndex)
				{
					if (!_stricmp(shader.name.c_str(), RTMissShader.c_str()))
					{
						includeShader = true;
						break;
					}
				}

				// If this shader is part of an RTHitGroupIndex, include it
				if (!includeShader)
				{
					for (const std::string& RTHitGroupName : node.shader.shader->Used_RTHitGroupIndex)
					{
						int hitGroupIndex = GetHitGroupIndex(m_renderGraph, RTHitGroupName.c_str());
						if (hitGroupIndex >= 0)
						{
							const RTHitGroup& hitGroup = m_renderGraph.hitGroups[hitGroupIndex];
							if (!_stricmp(shader.name.c_str(), hitGroup.closestHit.name.c_str()) ||
								!_stricmp(shader.name.c_str(), hitGroup.anyHit.name.c_str()) ||
								!_stricmp(shader.name.c_str(), hitGroup.intersection.name.c_str()))
							{
								includeShader = true;
								break;
							}
						}
					}
				}

				if (!includeShader)
					continue;

				ShaderExport newExport;
				newExport.shader = &shader;
				newExport.fileName = shader.destFileName;
				newExport.entryPoint = shader.entryPoint;
				newExport.entryPointW = ToWideString(shader.entryPoint.c_str());
				newExport.shaderType = shader.type;
				shaderExports.push_back(newExport);
			}
			shaderExports.push_back({ node.shader.shader, node.shader.shader->destFileName, (node.entryPoint.empty() ? node.shader.shader->entryPoint : node.entryPoint), ToWideString((node.entryPoint.empty() ? node.shader.shader->entryPoint : node.entryPoint).c_str()), L"", ShaderType::RTRayGen});

			// give each shader export a unique name
			for (size_t i = 0; i < shaderExports.size(); ++i)
			{
				wchar_t uniqueName[256];
				swprintf_s(uniqueName, L"%s_%i", shaderExports[i].entryPointW.c_str(), (int)i);
				shaderExports[i].uniqueName = uniqueName;
			}

			// Count how many we have of each shader type: ray gen, miss, hit group
			for (const ShaderExport& shaderExport : shaderExports)
			{
				switch (shaderExport.shaderType)
				{
					case ShaderType::RTMiss: countMiss++; break;
					case ShaderType::RTRayGen:
					case ShaderType::RTClosestHit:
					case ShaderType::RTAnyHit:
					case ShaderType::RTIntersection:
					{
						break;
					}
					default:
					{
						Assert(false, "Unhandled shader type");
					}
				}
			}

			// make the shader defines
			std::vector<D3D_SHADER_MACRO> defines;
			for (const ShaderDefine& define : node.shader.shader->defines)
				defines.push_back({ define.name.c_str(), define.value.c_str() });
			for (const ShaderDefine& define : node.defines)
				defines.push_back({ define.name.c_str(), define.value.c_str() });

			char maxRecursionDepthStr[256];
			sprintf_s(maxRecursionDepthStr, "%i", node.maxRecursionDepth);
			defines.push_back({ "MAX_RECURSION_DEPTH", maxRecursionDepthStr });
			char countHitGroupsStr[256];
			sprintf_s(countHitGroupsStr, "%i", countHitGroups);
			defines.push_back({ "RT_HIT_GROUP_COUNT", countHitGroupsStr });
			defines.push_back({ nullptr, nullptr });

			// Ray tracing shader compilation must use dxc
			for (const ShaderExport& shaderExport : shaderExports)
			{
				std::string fullFileName = (std::filesystem::path(m_tempDirectory) / "shaders" / shaderExport.fileName).string();

				std::vector<std::string> allShaderFiles;
				std::vector<unsigned char> code = CompileShaderToByteCode_dxc(
					fullFileName.c_str(),
					"",
					m_renderGraph.settings.dx12.shaderModelRayShaders.c_str(),
					defines.size() > 0 ? defines.data() : nullptr,
					m_compileShadersForDebug,
					m_renderGraph.settings.dx12.DXC_HLSL_2021,
					m_logFn,
					&allShaderFiles
				);

				// Watch the shader file source for file changes, even if it failed compilation, so we can detect when it's edited and try again
				for (const std::string& fileName : allShaderFiles)
				{
					std::string sourceFileName = (std::filesystem::path(m_renderGraph.baseDirectory) / std::filesystem::proximate(fileName, std::filesystem::path(m_tempDirectory) / "shaders")).string();
					m_fileWatcher.Add(sourceFileName.c_str(), FileWatchOwner::Shaders);
				}

				if (code.size() == 0)
				{
					m_logFn(LogLevel::Error, "Compiled shader %s is zero bytes (error)\n", shaderExport.fileName.c_str());
					return false;
				}
				allShaderCode.push_back(code);
			}
		}

		// Make the state object
		{
			std::vector<D3D12_STATE_SUBOBJECT> subObjects(
				shaderExports.size() + // One for each shader
				countHitGroups + // one for each hit group
				4 // Payload,  associate payload with shaders, pipeline config, global root signature
			);
			int subObjectIndex = 0;

			D3D12_STATE_OBJECT_DESC soDesc;
			soDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
			soDesc.NumSubobjects = (UINT)subObjects.size();
			soDesc.pSubobjects = subObjects.data();

			// DXIL libraries for the shaders
			std::vector<D3D12_EXPORT_DESC> exportDescs(shaderExports.size());
			std::vector<D3D12_DXIL_LIBRARY_DESC> libDescs(shaderExports.size());
			for (size_t index = 0; index < shaderExports.size(); ++index)
			{
				D3D12_EXPORT_DESC& exportDesc = exportDescs[index];
				exportDesc.Name = shaderExports[index].uniqueName.c_str();
				exportDesc.ExportToRename = shaderExports[index].entryPointW.c_str();
				exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

				D3D12_DXIL_LIBRARY_DESC& libDesc = libDescs[index];
				libDesc.DXILLibrary.BytecodeLength = allShaderCode[index].size();
				libDesc.DXILLibrary.pShaderBytecode = allShaderCode[index].data();
				libDesc.NumExports = 1;
				libDesc.pExports = &exportDesc;

				subObjects[subObjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				subObjects[subObjectIndex].pDesc = &libDesc;
				subObjectIndex++;
			}

			// Hit groups
			std::vector<std::wstring> hitGroupNameStrings(countHitGroups);
			std::vector<D3D12_HIT_GROUP_DESC> hitGroupDescs(countHitGroups);

			for (size_t index = 0; index < countHitGroups; ++index)
			{
				const std::string& RTHitGroupName = node.shader.shader->Used_RTHitGroupIndex[index];

				int HGIndex = GetHitGroupIndex(m_renderGraph, RTHitGroupName.c_str());
				if (HGIndex < 0)
					continue;

				wchar_t buffer[256];
				swprintf_s(buffer, L"hitgroup_%i", (int)index);
				hitGroupNameStrings[index] = buffer;

				D3D12_HIT_GROUP_DESC& hitGroupDesc = hitGroupDescs[index];
				hitGroupDesc.HitGroupExport = hitGroupNameStrings[index].c_str();
				hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

				// Any Hit Shader Import
				{
					const std::wstring* importName = nullptr;
					for (const ShaderExport& shaderExport : shaderExports)
					{
						if (shaderExport.shader == m_renderGraph.hitGroups[HGIndex].anyHit.shader)
						{
							importName = &shaderExport.uniqueName;
							break;
						}
					}
					hitGroupDesc.AnyHitShaderImport = importName ? importName->c_str() : nullptr;
				}

				// Closest Hit Shader Import
				{
					const std::wstring* importName = nullptr;
					for (const ShaderExport& shaderExport : shaderExports)
					{
						if (shaderExport.shader == m_renderGraph.hitGroups[HGIndex].closestHit.shader)
						{
							importName = &shaderExport.uniqueName;
							break;
						}
					}
					hitGroupDesc.ClosestHitShaderImport = importName ? importName->c_str() : nullptr;
				}

				// Intersection Shader Import
				{
					const std::wstring* importName = nullptr;
					for (const ShaderExport& shaderExport : shaderExports)
					{
						if (shaderExport.shader == m_renderGraph.hitGroups[HGIndex].intersection.shader)
						{
							importName = &shaderExport.uniqueName;
							break;
						}
					}
					hitGroupDesc.IntersectionShaderImport = importName ? importName->c_str() : nullptr;
					if (importName)
						hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
				}

				subObjects[subObjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
				subObjects[subObjectIndex].pDesc = &hitGroupDesc;
				subObjectIndex++;
			}

			// Payload
			D3D12_RAYTRACING_SHADER_CONFIG payloadDesc;
			payloadDesc.MaxPayloadSizeInBytes = node.rayPayloadSize;
			payloadDesc.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;
			subObjects[subObjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
			subObjects[subObjectIndex].pDesc = &payloadDesc;
			subObjectIndex++;

			// Associate payload with shaders
			std::vector<const wchar_t*> shaderExportNames;
			for (size_t index = 0; index < shaderExports.size(); ++index)
				shaderExportNames.push_back(shaderExports[index].uniqueName.c_str());

			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
			shaderPayloadAssociation.NumExports = (UINT)shaderExportNames.size();
			shaderPayloadAssociation.pExports = shaderExportNames.data();
			shaderPayloadAssociation.pSubobjectToAssociate = &subObjects[subObjectIndex - 1];

			subObjects[subObjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			subObjects[subObjectIndex].pDesc = &shaderPayloadAssociation;
			subObjectIndex++;

			// Pipeline Config
			D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
			pipelineConfig.MaxTraceRecursionDepth = node.maxRecursionDepth;
			subObjects[subObjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
			subObjects[subObjectIndex].pDesc = &pipelineConfig;
			subObjectIndex++;

			// Global Root Signature
			subObjects[subObjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
			subObjects[subObjectIndex].pDesc = &runtimeData.m_rootSignature;
			subObjectIndex++;

			soDesc.NumSubobjects = subObjectIndex;
			HRESULT hr = m_dxrDevice->CreateStateObject(&soDesc, IID_PPV_ARGS(&runtimeData.m_stateObject));
			if (FAILED(hr))
			{
				_com_error err(hr);
				m_logFn(LogLevel::Error, "Could not create state object: %s\n", FromWideString(err.ErrorMessage()).c_str());
				return false;
			}

			runtimeData.m_stateObject->SetName(ToWideString(node.name.c_str()).c_str());
		}

		// Create the shader tables
		{
			ID3D12StateObjectProperties* soprops = nullptr;
			if (FAILED(runtimeData.m_stateObject->QueryInterface(IID_PPV_ARGS(&soprops))))
			{
				m_logFn(LogLevel::Error, "Could not get ID3D12StateObjectProperties\n");
				return false;
			}

			// Make the ray gen shader table
			{
				runtimeData.m_shaderTableRayGenSize = (unsigned int)(countRaygen * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
				runtimeData.m_shaderTableRayGenSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, runtimeData.m_shaderTableRayGenSize);

				runtimeData.m_shaderTableRayGen = CreateBuffer(m_device, runtimeData.m_shaderTableRayGenSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (node.name + " Ray Gen Shader Table").c_str());

				unsigned char* shaderTableBytes = nullptr;
				D3D12_RANGE readRange = { 0, 0 };
				runtimeData.m_shaderTableRayGen->Map(0, &readRange, (void**)&shaderTableBytes);

				for (size_t index = 0; index < shaderExports.size(); ++index)
				{
					if (shaderExports[index].shaderType != ShaderType::RTRayGen)
						continue;

					void* location = soprops->GetShaderIdentifier(shaderExports[index].uniqueName.c_str());
					memcpy(shaderTableBytes, location, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

					shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
				}

				runtimeData.m_shaderTableRayGen->Unmap(0, nullptr);
			}

			// Make the miss shader table
			if (countMiss > 0)
			{
				runtimeData.m_shaderTableMissSize = (unsigned int)(countMiss * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
				runtimeData.m_shaderTableMissSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, runtimeData.m_shaderTableMissSize);

				runtimeData.m_shaderTableMiss = CreateBuffer(m_device, runtimeData.m_shaderTableMissSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (node.name + " Miss Shader Table").c_str());

				unsigned char* shaderTableBytes = nullptr;
				D3D12_RANGE readRange = { 0, 0 };
				runtimeData.m_shaderTableMiss->Map(0, &readRange, (void**)&shaderTableBytes);

				for (size_t index = 0; index < shaderExports.size(); ++index)
				{
					if (shaderExports[index].shaderType != ShaderType::RTMiss)
						continue;

					void* location = soprops->GetShaderIdentifier(shaderExports[index].uniqueName.c_str());
					memcpy(shaderTableBytes, location, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

					shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
				}

				runtimeData.m_shaderTableMiss->Unmap(0, nullptr);
			}

			// Make the hit group table
			if (countHitGroups > 0)
			{
				runtimeData.m_shaderTableHitGroupSize = (unsigned int)(countHitGroups * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
				runtimeData.m_shaderTableHitGroupSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, runtimeData.m_shaderTableHitGroupSize);

				runtimeData.m_shaderTableHitGroup = CreateBuffer(m_device, runtimeData.m_shaderTableHitGroupSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (node.name + " Hit Group Shader Table").c_str());

				unsigned char* shaderTableBytes = nullptr;
				D3D12_RANGE readRange = { 0, 0 };
				runtimeData.m_shaderTableHitGroup->Map(0, &readRange, (void**)&shaderTableBytes);

				for (int index = 0; index < countHitGroups; ++index)
				{
					wchar_t buffer[256];
					swprintf_s(buffer, L"hitgroup_%i", index);

					void* location = soprops->GetShaderIdentifier(buffer);
					memcpy(shaderTableBytes, location, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

					shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
				}

				runtimeData.m_shaderTableHitGroup->Unmap(0, nullptr);
			}

			soprops->Release();
		}
	}

	if (nodeAction == NodeAction::Execute)
	{
		// publish SRVs and UAVs as viewable textures, before the shader execution
		int depIndex = -1;
		for (const ResourceDependency& dep : node.resourceDependencies)
		{
			if (dep.access == ShaderResourceAccessType::Indirect)
				continue;

			depIndex++;
			const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];
			switch (resourceNode._index)
			{
				case RenderGraphNode::c_index_resourceTexture:
				{
					std::string label = node.name + std::string(".") + node.shader.shader->resources[depIndex].name + std::string(": ") + resourceNode.resourceTexture.name;
					if (dep.access == ShaderResourceAccessType::UAV)
						label = label + " (UAV - Before)";
					else
						label = label + " (SRV)";

					const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
					runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_size, resourceInfo.m_numMips, false, false);
					break;
				}
				case RenderGraphNode::c_index_resourceShaderConstants:
				{
					std::string label = node.name + std::string(".") + node.shader.shader->resources[depIndex].name + std::string(": ") + resourceNode.resourceShaderConstants.name;
					label = label + " (CBV)";

					const RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_ShaderConstants(resourceNode.resourceShaderConstants.name.c_str());
					runtimeData.HandleViewableConstantBuffer(*this, label.c_str(), resourceInfo.m_buffer->buffer, (int)resourceInfo.m_buffer->size, node.shader.shader->resources[depIndex].constantBufferStructIndex, false, false);
					break;
				}
				case RenderGraphNode::c_index_resourceBuffer:
				{
					std::string label = node.name + std::string(".") + node.shader.shader->resources[depIndex].name + std::string(": ") + resourceNode.resourceBuffer.name;
					if (dep.access == ShaderResourceAccessType::UAV)
						label = label + " (UAV - Before)";
					else
						label = label + " (SRV)";

					const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
					runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, false);
					break;
				}
			}
		}
		//runtimeData.HandleViewableBuffer(*this, (node.name + std::string(".shaderTable")).c_str(), runtimeData.m_shaderTable, DXGI_FORMAT_R32G32B32A32_UINT, 1, -1, runtimeData.m_shaderTableSize, 0, runtimeData.m_shaderTableSize / 16);

		// Queue up transitions, so we only do them if we actually execute the node
		std::vector<TransitionTracker::Item> queuedTransitions;

		// Fill out the descriptor table information
		std::vector<DescriptorTableCache::ResourceDescriptor> descriptors;
		{
			int depIndex = -1;
			for (const ResourceDependency& dep : node.resourceDependencies)
			{
				if (dep.access == ShaderResourceAccessType::Indirect)
					continue;

				depIndex++;
				DescriptorTableCache::ResourceDescriptor desc;

				const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];
				switch (resourceNode._index)
				{
					case RenderGraphNode::c_index_resourceTexture:
					{
						const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo =  GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
						desc.m_resource = resourceInfo.m_resource;
						desc.m_format = resourceInfo.m_format;

						if (dep.pinIndex < node.linkProperties.size())
						{
							desc.m_UAVMipIndex = min(node.linkProperties[dep.pinIndex].UAVMipIndex, resourceInfo.m_numMips - 1);
						}

						switch (resourceNode.resourceTexture.dimension)
						{
							case TextureDimensionType::Texture2D: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2D; break;
							case TextureDimensionType::Texture2DArray: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2DArray; desc.m_count = resourceInfo.m_size[2]; break;
							case TextureDimensionType::Texture3D: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture3D; desc.m_count = resourceInfo.m_size[2]; break;
							case TextureDimensionType::TextureCube: desc.m_resourceType = DescriptorTableCache::ResourceType::TextureCube; desc.m_count = 6; break;
						}
						break;
					}
					case RenderGraphNode::c_index_resourceShaderConstants:
					{
						const RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_ShaderConstants(resourceNode.resourceShaderConstants.name.c_str());
						desc.m_resource = resourceInfo.m_buffer->buffer;
						desc.m_format = DXGI_FORMAT_UNKNOWN;
						desc.m_stride = (UINT)resourceInfo.m_buffer->size;
						desc.m_count = 1;
						break;
					}
					case RenderGraphNode::c_index_resourceBuffer:
					{
						RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());

						if (dep.access == ShaderResourceAccessType::RTScene)
						{
							if (!AccessIsReadOnly(resourceNode.resourceBuffer.accessedAs))
							{
								m_transitions.Untrack(resourceInfo.m_tlas);
								m_delayedRelease.Add(resourceInfo.m_tlas);
								resourceInfo.m_tlas = nullptr;
								resourceInfo.m_tlasSize = 0;

								m_transitions.Untrack(resourceInfo.m_blas);
								m_delayedRelease.Add(resourceInfo.m_blas);
								resourceInfo.m_blas = nullptr;
								resourceInfo.m_blasSize = 0;

								m_transitions.Untrack(resourceInfo.m_instanceDescs);
								m_delayedRelease.Add(resourceInfo.m_instanceDescs);
								resourceInfo.m_instanceDescs = nullptr;
							}

							if (!resourceInfo.m_tlas)
							{
								if (!MakeAccelerationStructures(resourceNode.resourceBuffer))
								{
									m_logFn(LogLevel::Error, "Failed to make acceleration structure for buffer \"%s\"", resourceNode.resourceBuffer.name.c_str());
									return false;
								}
							}

							desc.m_resource = resourceInfo.m_tlas;
							desc.m_format = DXGI_FORMAT_UNKNOWN;
							desc.m_stride = resourceInfo.m_tlasSize;
							desc.m_count = 1;
							desc.m_raw = false;
						}
						else
						{
							desc.m_resource = resourceInfo.m_resource;
							desc.m_format = resourceInfo.m_format;
							desc.m_stride = (desc.m_format == DXGI_FORMAT_UNKNOWN) ? resourceInfo.m_size / resourceInfo.m_count : 0;
							desc.m_count = resourceInfo.m_count;
							desc.m_raw = node.shader.shader->resources[depIndex].buffer.raw;
						}
						break;
					}
					default:
					{
						m_logFn(LogLevel::Error, "Unhandled dependency node type for ray gen shader node \"%s\"", node.name.c_str());
						return false;
					}
				}

				// This could be a temporary thing, but we can't run the compute shader if we don't have the resources we need.
				if (!desc.m_resource)
				{
					std::ostringstream ss;
					ss << "Cannot execute: resource \"" << GetNodeName(m_renderGraph.nodes[dep.nodeIndex]) << "\" doesn't exist yet";
					runtimeData.m_renderGraphText = ss.str();
					return true;
				}

				switch (dep.access)
				{
					case ShaderResourceAccessType::UAV:
					{
						desc.m_access = DescriptorTableCache::AccessType::UAV;
						queuedTransitions.push_back({ TRANSITION_DEBUG_INFO(desc.m_resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) });
						break;
					}
					case ShaderResourceAccessType::RTScene:
					{
						desc.m_access = DescriptorTableCache::AccessType::SRV;
						queuedTransitions.push_back({ TRANSITION_DEBUG_INFO(desc.m_resource, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) });
						break;
					}
					case ShaderResourceAccessType::SRV:
					{
						desc.m_access = DescriptorTableCache::AccessType::SRV;
						queuedTransitions.push_back({ TRANSITION_DEBUG_INFO(desc.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) });
						break;
					}
					case ShaderResourceAccessType::CBV:
					{
						// constant buffers are upload heap, don't need transitions to be written from CPU or read by shaders
						desc.m_access = DescriptorTableCache::AccessType::CBV;
						break;
					}
					case ShaderResourceAccessType::Indirect:
					{
						// This is handled elsewhere
						continue;
					}
					default:
					{
						m_logFn(LogLevel::Error, "Unhandled shader resource access type \"%s\" for ray gen shader node \"%s\"", EnumToString(dep.access), node.name.c_str());
						return false;
					}
				}

				switch (dep.type)
				{
					case ShaderResourceType::Texture: break;// handled above
					case ShaderResourceType::Buffer:
					{
						if (dep.access == ShaderResourceAccessType::RTScene)
							desc.m_resourceType = DescriptorTableCache::ResourceType::RTScene;
						else
							desc.m_resourceType = DescriptorTableCache::ResourceType::Buffer;
						break;
					}
					case ShaderResourceType::ConstantBuffer: desc.m_resourceType = DescriptorTableCache::ResourceType::Buffer; break;
					default:
					{
						m_logFn(LogLevel::Error, "Unhandled shader resource type \"%s\" for ray gen shader node \"%s\"", EnumToString(dep.type), node.name.c_str());
						return false;
					}
				}

				descriptors.push_back(desc);
			}
		}

		// calculate dispatch size
		unsigned int dispatchSize[3] = { 1, 1, 1 };
		if (node.dispatchSize.node.textureNode)
		{
			IVec3 size = GetDesiredSize(*this, *node.dispatchSize.node.textureNode);
			dispatchSize[0] = size[0];
			dispatchSize[1] = size[1];
			dispatchSize[2] = size[2];
		}
		else if (node.dispatchSize.node.bufferNode)
		{
			dispatchSize[0] = GetDesiredCount(*this, *node.dispatchSize.node.bufferNode);
			dispatchSize[1] = 1;
			dispatchSize[2] = 1;
		}
		else if (node.dispatchSize.variable.variableIndex != -1)
		{
            Variable& var = m_renderGraph.variables[node.dispatchSize.variable.variableIndex];
			const RuntimeVariable& rtVar = GetRuntimeVariable(node.dispatchSize.variable.variableIndex);

			DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(var.type);
			if (typeInfo.componentCount < 1 || typeInfo.componentCount > 3)
			{
				m_logFn(LogLevel::Error, "Ray gen shader node \"%s\" wants to use variable \"%s\" for dispatch size, but has the wrong number of components", node.name.c_str(), var.name.c_str());
				return false;
			}

			switch (typeInfo.componentType)
			{
				case DataFieldComponentType::_int:
				{
					for (int i = 0; i < typeInfo.componentCount; ++i)
						dispatchSize[i] = ((int*)rtVar.storage.value)[i];
					break;
				}
				case DataFieldComponentType::_uint16_t:
				{
					for (int i = 0; i < typeInfo.componentCount; ++i)
						dispatchSize[i] = ((uint16_t*)rtVar.storage.value)[i];
					break;
				}
				case DataFieldComponentType::_uint32_t:
				{
					for (int i = 0; i < typeInfo.componentCount; ++i)
						dispatchSize[i] = ((uint32_t*)rtVar.storage.value)[i];
					break;
				}
				case DataFieldComponentType::_float:
				{
					for (int i = 0; i < typeInfo.componentCount; ++i)
						dispatchSize[i] = (unsigned int)((float*)rtVar.storage.value)[i];
					break;
				}
				default:
				{
					m_logFn(LogLevel::Error, "Ray gen shader node \"%s\" wants to use variable \"%s\" for dispatch size, but it is an unsupported type", node.name.c_str(), var.name.c_str());
					return false;
				}
			}
		}

		// Do fixed function calculations on dispatch size
		for (int i = 0; i < 3; ++i)
			dispatchSize[i] = ((dispatchSize[i] + node.dispatchSize.preAdd[i]) * node.dispatchSize.multiply[i]) / node.dispatchSize.divide[i] + node.dispatchSize.postAdd[i];

		if (dispatchSize[0] == 0 || dispatchSize[1] == 0 || dispatchSize[2] == 0)
		{
			m_logFn(LogLevel::Error, "Ray gen shader node \"%s\" wanted to do a dispatch of size 0.  dispatchSize = (%u, %u, %u)", node.name.c_str(), dispatchSize[0], dispatchSize[1], dispatchSize[2]);
			return false;
		}

		std::ostringstream ss;
		ss << "Dispatch: (" << dispatchSize[0] << ", " << dispatchSize[1] << ", " << dispatchSize[2] << ")";
		if (IsConditional(node.condition))
			ss << "\nCondition: " << (EvaluateCondition(node.condition) ? "true" : "false");
		runtimeData.m_renderGraphText = ss.str();

		// Do resource transitions and dispatch
		if (EvaluateCondition(node.condition))
		{
			m_transitions.Transition(queuedTransitions);
			m_transitions.Flush(m_dxrCommandList);

			// Get or make the descriptor table
			D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable;
			std::string error;
			if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptors.data(), (int)descriptors.size(), descriptorTable, error, HEAP_DEBUG_TEXT()))
			{
				m_logFn(LogLevel::Error, "Ray gen shader node \"%s\" could not allocate a descriptor table: %s", node.name.c_str(), error.c_str());
				return false;
			}

			m_dxrCommandList->SetComputeRootSignature(runtimeData.m_rootSignature);
			m_dxrCommandList->SetPipelineState1(runtimeData.m_stateObject);
			m_dxrCommandList->SetComputeRootDescriptorTable(0, descriptorTable);

			D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
			dispatchDesc.Width = dispatchSize[0];
			dispatchDesc.Height = dispatchSize[1];
			dispatchDesc.Depth = dispatchSize[2];

			dispatchDesc.RayGenerationShaderRecord.StartAddress = runtimeData.m_shaderTableRayGen->GetGPUVirtualAddress();
			dispatchDesc.RayGenerationShaderRecord.SizeInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

			if (runtimeData.m_shaderTableMiss)
				dispatchDesc.MissShaderTable.StartAddress = runtimeData.m_shaderTableMiss->GetGPUVirtualAddress();
			dispatchDesc.MissShaderTable.SizeInBytes = runtimeData.m_shaderTableMissSize;
			dispatchDesc.MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

			if (runtimeData.m_shaderTableHitGroup)
				dispatchDesc.HitGroupTable.StartAddress = runtimeData.m_shaderTableHitGroup->GetGPUVirtualAddress();
			dispatchDesc.HitGroupTable.SizeInBytes = runtimeData.m_shaderTableHitGroupSize;
			dispatchDesc.HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

			m_dxrCommandList->DispatchRays(&dispatchDesc);
		}

		// publish SRVs and UAVs as viewable textures, after the shader execution
		depIndex = -1;
		for (const ResourceDependency& dep : node.resourceDependencies)
		{
			if (dep.access == ShaderResourceAccessType::Indirect)
				continue;

			depIndex++;
			const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];
			switch (resourceNode._index)
			{
				case RenderGraphNode::c_index_resourceTexture:
				{
					std::string label = node.name + std::string(".") + node.shader.shader->resources[depIndex].name + std::string(": ") + resourceNode.resourceTexture.name;
					if (dep.access == ShaderResourceAccessType::UAV)
						label = label + " (UAV - After)";
					else
						continue;

					const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
					runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_size, resourceInfo.m_numMips, false, true);
					break;
				}
				case RenderGraphNode::c_index_resourceBuffer:
				{
					std::string label = node.name + std::string(".") + node.shader.shader->resources[depIndex].name + std::string(": ") + resourceNode.resourceBuffer.name;
					if (dep.access == ShaderResourceAccessType::UAV)
						label = label + " (UAV - After)";
					else
						continue;

					const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
					runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, true);
					break;
				}
			}
		}
	}

	return true;
}
