///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"
#include "DX12Utils/CompileShaders.h"

#include <sstream>

void RuntimeTypes::RenderGraphNode_Action_ComputeShader::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
	RenderGraphNode_Base::Release(interpreter);

	if (m_rootSignature)
	{
		interpreter.m_delayedRelease.Add(m_rootSignature);
		m_rootSignature = nullptr;
	}

	if (m_pso)
	{
		interpreter.m_delayedRelease.Add(m_pso);
		m_pso = nullptr;
	}
}

void GigiInterpreterPreviewWindowDX12::OnRootSignature(ID3DBlob *sig, const Shader* shader)
{
	if (sig && shader)
	if(m_renderGraph.settings.common.createPDBsAndBinaries)
	{
		const std::string& destFileName = shader->destFileName;
		const std::filesystem::path directoryPath = "./ShaderDebugInfo";
		std::string fullFileName = (std::filesystem::path(GetTempDirectory()) / "shaders" / destFileName).stem().string();

		if (!std::filesystem::exists(directoryPath))
			std::filesystem::create_directories(directoryPath);

		{
			std::filesystem::path rootSigFilePath = directoryPath / std::filesystem::path(fullFileName + ".rs.fxo");

			FILE* fp = NULL;
			if (_wfopen_s(&fp, rootSigFilePath.c_str(), L"wb") == 0)
			{
				fwrite(sig->GetBufferPointer(), sig->GetBufferSize(), 1, fp);
				fclose(fp);
			}
		}
		{
			std::filesystem::path shaderBinaryFileName = std::filesystem::path(fullFileName + ".cso");
			std::filesystem::path rootSigFileName = std::filesystem::path(fullFileName + ".rs.fxo");
			std::filesystem::path disassemblyFileName = std::filesystem::path(fullFileName + ".dis.txt");
			std::filesystem::path statsFileName = std::filesystem::path(fullFileName + ".stats.txt");

			std::filesystem::path batFilePath = directoryPath / std::filesystem::path(fullFileName + ".bat");

			FILE* fp = NULL;
			if (_wfopen_s(&fp, batFilePath.c_str(), L"w") == 0)
			{
				fprintf(fp, "rem Make sure rga.exe is installed in the referenced location.\n");
				fprintf(fp, "rem use \"rgb.exe -l\" to find the right ASIC to target.\n");
				fprintf(fp, "rem Run this from command line.\n\n");

				// e.g "E:\\Apps\\RadeonDeveloperToolSuite\\rga.exe"
				fprintf(fp, "%s ", m_renderGraph.settings.common.rgaPath.c_str());
				fprintf(fp, "-s dx12 ");

				const char* shortShaderType = nullptr;

				if (shader->type == ShaderType::Compute)
				{
					shortShaderType = "cs";
					fprintf(fp, "--%s-model \"%s\" ", shortShaderType, m_renderGraph.settings.dx12.shaderModelCs.c_str());
				}
				else if (shader->type == ShaderType::Vertex)
				{
					shortShaderType = "vs";
					fprintf(fp, "--%s-model \"%s\" ", shortShaderType, m_renderGraph.settings.dx12.shaderModelVs.c_str());
				}
				else if (shader->type == ShaderType::Pixel)
				{
					shortShaderType = "ps";
					fprintf(fp, "--%s-model \"%s\" ", shortShaderType, m_renderGraph.settings.dx12.shaderModelPs.c_str());
				}
				else if (shader->type == ShaderType::Amplification)
				{
					shortShaderType = "as";
					fprintf(fp, "--%s-model \"%s\" ", shortShaderType, m_renderGraph.settings.dx12.shaderModelAs.c_str());
				}
				else if (shader->type == ShaderType::Mesh)
				{
					shortShaderType = "ms";
					fprintf(fp, "--%s-model \"%s\" ", shortShaderType, m_renderGraph.settings.dx12.shaderModelMs.c_str());
				}
				// internal error
				assert(shortShaderType);

				fprintf(fp, "--%s-blob %s ", shortShaderType, shaderBinaryFileName.string().c_str());
				fprintf(fp, "--%s-entry %s ", shortShaderType, shader->entryPoint.c_str());

				// target ASIC hardware
				fprintf(fp, "-c %s ", m_renderGraph.settings.common.rgaASIC.c_str());
				// output disassembly, specific to AMD ISA
				fprintf(fp, "--isa %s ", disassemblyFileName.string().c_str());
				// output stats e.g. numUsedVgprs, numUsedSgprs, ldsUsageSizeInBytes, ...
				fprintf(fp, "-a %s ", statsFileName.string().c_str());
				// don't require installed AMD hardware, might require installed driver?
				fprintf(fp, "--offline ");
				// root signature
				fprintf(fp, "--rs-bin %s\n", rootSigFileName.string().c_str());
				fprintf(fp, "\n");
				fclose(fp);
			}
		}

	}
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_ComputeShader& node, RuntimeTypes::RenderGraphNode_Action_ComputeShader& runtimeData, NodeAction nodeAction)
{
	ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, false);

	if (nodeAction == NodeAction::Init)
	{
		// make the root signature
		{
			// shader samplers
			std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
			int samplerIndex = -1;
			for (const ShaderSampler& sampler : node.shader.shader->samplers)
			{
				samplerIndex++;
				D3D12_STATIC_SAMPLER_DESC desc;

				if (!SamplerFilterToD3D12Filter(sampler.filter, desc.Filter))
				{
					m_logFn(LogLevel::Error, "Shader \"%s\" has an invalid sampler filter for sampler %i: \"%s\"", node.shader.shader->name.c_str(), samplerIndex, EnumToString(sampler.filter));
					return false;
				}

				if (!SamplerAddressModeToD3D12AddressMode(sampler.addressMode, desc.AddressU))
				{
					m_logFn(LogLevel::Error, "Shader \"%s\" has an invalid address mode for sampler %i: \"%s\"", node.shader.shader->name.c_str(), samplerIndex, EnumToString(sampler.addressMode));
					return false;
				}

				desc.AddressV = desc.AddressW = desc.AddressU;

				desc.MipLODBias = 0;
				desc.MaxAnisotropy = 0;
				desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				desc.MinLOD = 0.0f;
				desc.MaxLOD = D3D12_FLOAT32_MAX;
				desc.ShaderRegister = sampler.registerIndex;
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
						m_logFn(LogLevel::Error, "Shader \"%s\" unhandled resource access: \"%s\"", node.shader.shader->name.c_str(), EnumToString(resource.access));
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
			rootDesc.NumParameters = (ranges.size() > 0) ? 1 : 0;
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
				if (sig) sig->Release();
				if (error) error->Release();
				return false;
			}

			OnRootSignature(sig, node.shader.shader);

			hr = m_device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&runtimeData.m_rootSignature));
			if (FAILED(hr))
			{
				const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
				if (errorMsg)
					m_logFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
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

		// Make the PSO
		{
			// shader defines
			std::string fullFileName = (std::filesystem::path(GetTempDirectory()) / "shaders" / node.shader.shader->destFileName).string();

			// Shader compilation depends on which shader compiler they opted to use for this technique
			ShaderCompilationInfo shaderCompilationInfo;
			shaderCompilationInfo.fileName = fullFileName;
			shaderCompilationInfo.entryPoint = node.entryPoint.empty() ? node.shader.shader->entryPoint : node.entryPoint;
			shaderCompilationInfo.shaderModel = m_renderGraph.settings.dx12.shaderModelCs;
			shaderCompilationInfo.debugName = node.name;
			shaderCompilationInfo.defines = node.shader.shader->defines;

			if (!node.defines.empty())
			{
				shaderCompilationInfo.defines.insert(shaderCompilationInfo.defines.end(), node.defines.begin(), node.defines.end());
			}

			if (m_compileShadersForDebug)
			{
				shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
			}

			if (m_renderGraph.settings.dx12.Allow16BitTypes && m_dx12_options4.Native16BitShaderOpsSupported)
				shaderCompilationInfo.flags |= ShaderCompilationFlags::Enable16BitTypes;

			if (m_renderGraph.settings.dx12.DXC_HLSL_2021)
			{
				shaderCompilationInfo.flags |= ShaderCompilationFlags::HLSL2021;
			}

			if (m_renderGraph.settings.common.shaderWarningAsErrors)
			{
				shaderCompilationInfo.flags |= ShaderCompilationFlags::WarningsAsErrors;
			}

			if (m_renderGraph.settings.common.createPDBsAndBinaries)
			{
				shaderCompilationInfo.flags |= ShaderCompilationFlags::CreatePDBsAndBinaries;
			}

			std::vector<std::string> allShaderFiles;
			switch (m_renderGraph.settings.dx12.shaderCompiler)
			{
				case DXShaderCompiler::FXC:
				{
					MakeComputePSO_fxc(
						m_device,
						shaderCompilationInfo,
						runtimeData.m_rootSignature,
						&runtimeData.m_pso,
						m_logFn,
						&allShaderFiles
					);
					break;
				}
				case DXShaderCompiler::DXC:
				{
					MakeComputePSO_dxc(
						m_device,
						shaderCompilationInfo,
						runtimeData.m_rootSignature,
						& runtimeData.m_pso,
						m_logFn,
						&allShaderFiles
					);
					break;
				}
			}

			// Watch the shader file source for file changes, even if it failed compilation, so we can detect when it's edited and try again
			for (const std::string& fileName : allShaderFiles)
			{
				std::string sourceFileName = (std::filesystem::path(m_renderGraph.baseDirectory) / std::filesystem::proximate(fileName, std::filesystem::path(GetTempDirectory()) / "shaders")).string();
				m_fileWatcher.Add(sourceFileName.c_str(), FileWatchOwner::Shaders);
			}

			if (!runtimeData.m_pso)
			{
				m_logFn(LogLevel::Error, "Shader \"%s\" Could not create PSO", node.shader.shader->name.c_str());
				return false;
			}

			// name the PSO for debuggers
			runtimeData.m_pso->SetName(ToWideString(node.shader.shader->entryPoint.c_str()).c_str());
		}
		return true;
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

							const ShaderResourceBuffer& shaderResourceBuffer = node.shader.shader->resources[depIndex].buffer;
							bool isStructuredBuffer = ShaderResourceBufferIsStructuredBuffer(shaderResourceBuffer);
							if (isStructuredBuffer)
							{
								desc.m_format = DXGI_FORMAT_UNKNOWN;
								if (shaderResourceBuffer.typeStruct.structIndex != -1)
									desc.m_stride = (UINT)m_renderGraph.structs[shaderResourceBuffer.typeStruct.structIndex].sizeInBytes;
								else
									desc.m_stride = DataFieldTypeInfo(shaderResourceBuffer.type).typeBytes;
								desc.m_count = resourceInfo.m_size / desc.m_stride;
							}
							else
							{
								desc.m_format = DataFieldTypeInfoDX12(shaderResourceBuffer.type).typeFormat;
								desc.m_stride = 0;
								desc.m_count = resourceInfo.m_count;
							}

							desc.m_raw = node.shader.shader->resources[depIndex].buffer.raw;
						}
						break;
					}
					default:
					{
						m_logFn(LogLevel::Error, "Unhandled dependency node type for compute shader node \"%s\"", node.name.c_str());
						return false;
					}
				}

				// This could be a temporary thing, but we can't run the compute shader if we don't have the resources we need.
				if (!desc.m_resource)
				{
					std::ostringstream ss;
					ss << "Cannot run due to resource not existing:\n" << GetNodeTypeString(resourceNode) << " \"" << GetNodeName(resourceNode) << "\" (\"" << GetNodeOriginalName(resourceNode) << "\")";
					runtimeData.m_renderGraphText = ss.str();
                    runtimeData.m_inErrorState = true;
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
						m_logFn(LogLevel::Error, "Unhandled shader resource access type \"%s\" for compute shader node \"%s\"", EnumToString(dep.access), node.name.c_str());
						return false;
					}
				}

				switch (dep.type)
				{
					case ShaderResourceType::Texture: break; // handled above
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
						m_logFn(LogLevel::Error, "Unhandled shader resource type \"%s\" for compute shader node \"%s\"", EnumToString(dep.type), node.name.c_str());
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
				m_logFn(LogLevel::Error, "compute shader node \"%s\" wants to use variable \"%s\" for dispatch size, but has the wrong number of components", node.name.c_str(), var.name.c_str());
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
					m_logFn(LogLevel::Error, "compute shader node \"%s\" wants to use variable \"%s\" for dispatch size, but it is an unsupported type", node.name.c_str(), var.name.c_str());
					return false;
				}
			}
		}

		// Do fixed function calculations on dispatch size
		for (int i = 0; i < 3; ++i)
			dispatchSize[i] = ((dispatchSize[i] + node.dispatchSize.preAdd[i]) * node.dispatchSize.multiply[i]) / node.dispatchSize.divide[i] + node.dispatchSize.postAdd[i];

		if (dispatchSize[0] == 0 || dispatchSize[1] == 0 || dispatchSize[2] == 0)
		{
			m_logFn(LogLevel::Error, "compute shader node \"%s\" wanted to do a dispatch of size 0.  dispatchSize = (%u, %u, %u)", node.name.c_str(), dispatchSize[0], dispatchSize[1], dispatchSize[2]);
			return false;
		}

		if (node.shader.shader->NumThreads[0] == 0 || node.shader.shader->NumThreads[1] == 0 || node.shader.shader->NumThreads[2] == 0)
		{
			m_logFn(LogLevel::Error, "compute shader node \"%s\" wanted to run with 0 threads.  NumThreads = (%u, %u, %u)", node.name.c_str(), node.shader.shader->NumThreads[0], node.shader.shader->NumThreads[1], node.shader.shader->NumThreads[2]);
			return false;
		}

		// do numThreads calculations. Divide by numThreads but round up.
		unsigned int origDispatchSize[3] = { 1, 1, 1 };
		for (int i = 0; i < 3; ++i)
		{
			origDispatchSize[i] = dispatchSize[i];
			dispatchSize[i] = (dispatchSize[i] + node.shader.shader->NumThreads[i] - 1) / node.shader.shader->NumThreads[i];
		}

		bool executionConditionMet = EvaluateCondition(node.condition);

		// Make descriptor table, doing all required resource transitions first
		if (executionConditionMet)
		{
			// Do transitions needed by descriptor table
			m_transitions.Transition(queuedTransitions);
			m_transitions.Flush(m_commandList);

			// Get or make the descriptor table
			D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable;
			std::string error;
			if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptors.data(), (int)descriptors.size(), descriptorTable, error, HEAP_DEBUG_TEXT()))
			{
				m_logFn(LogLevel::Error, "Compute Shader Node \"%s\" could not allocate a descriptor table: %s", node.name.c_str(), error.c_str());
				return false;
			}

			// set the root signature, PSO and descriptor table
			m_commandList->SetComputeRootSignature(runtimeData.m_rootSignature);
			m_commandList->SetPipelineState(runtimeData.m_pso);
			m_commandList->SetComputeRootDescriptorTable(0, descriptorTable);
		}

		// Dispatch
		if (node.dispatchSize.indirectBuffer.resourceNodeIndex == -1)
		{
			std::ostringstream ss;
			ss << "Dispatch: (" << origDispatchSize[0] << ", " << origDispatchSize[1] << ", " << origDispatchSize[2] << ")";
			ss << "\nThreadGroups: (" << dispatchSize[0] << ", " << dispatchSize[1] << ", " << dispatchSize[2] << ")";
			ss << "\nNumThreads: (" << node.shader.shader->NumThreads[0] << ", " << node.shader.shader->NumThreads[1] << ", " << node.shader.shader->NumThreads[2] << ")";
			if (IsConditional(node.condition))
				ss << "\nCondition: " << (EvaluateCondition(node.condition) ? "true" : "false");
			runtimeData.m_renderGraphText = ss.str();

			// Do Dispatch
			if (executionConditionMet)
				m_commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
		}
		// Indirect dispatch
		else
		{
			std::ostringstream ss;
			ss << "Dispatch: Indirect";
			if (IsConditional(node.condition))
				ss << "\nCondition: " << (EvaluateCondition(node.condition) ? "true" : "false");
			runtimeData.m_renderGraphText = ss.str();

			// Get the indirect buffer and publish it as a viewable resource
			const std::string& indirectBufferName = m_renderGraph.nodes[node.dispatchSize.indirectBuffer.resourceNodeIndex].resourceBuffer.name;
			bool exists = false;
			const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(indirectBufferName.c_str(), exists);
			if (!exists)
				return true;

			std::string label = node.name + std::string(".indirectBuffer") + std::string(": ") + indirectBufferName;
			runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, false);

			// This could be a temporary thing, but we can't indirect dispatch if the buffer doesn't exist
			if (!resourceInfo.m_resource)
				return true;

			// Do resource transitions, and dispatch
			if (executionConditionMet)
			{
				// Note: maybe this could move earlier, so there isn't an extra transition call here. like in the descriptor table logic even though it doesn't go in the descriptor table?
				m_transitions.Transition(TRANSITION_DEBUG_INFO(resourceInfo.m_resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
				m_transitions.Flush(m_commandList);

				// Get the indirect buffer offset
				UINT64 argumentBufferOffset = node.dispatchSize.indirectOffsetValue;
				if (node.dispatchSize.indirectOffsetVariable.variableIndex != -1)
				{
					GigiInterpreterPreviewWindowDX12::RuntimeVariable rtVar = GetRuntimeVariable(node.dispatchSize.indirectOffsetVariable.variableIndex);
					switch (rtVar.variable->type)
					{
						case DataFieldType::Int: argumentBufferOffset = static_cast<UINT64>(*(int*)rtVar.storage.value); break;
						default:
						{
							m_logFn(LogLevel::Error, "Unhandled data type \"%s\" for Indirect Offset variable \"%s\" in compute shader node \"%s\"", EnumToString(rtVar.variable->type), rtVar.variable->name.c_str(), node.name.c_str());
							return false;
						}
					}
				}

				m_commandList->ExecuteIndirect(
					m_commandSignatureDispatch,
					1,
					resourceInfo.m_resource,
					argumentBufferOffset * 4 * 4, // byte offset.  *4 because sizeof(UINT) is 4.  *4 again because of 4 items per dispatch.
					nullptr,
					0);
			}
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
