///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"
#include "DX12Utils/CompileShaders.h"

#include <sstream>
#include <comdef.h>
#include <unordered_map>

#include "d3dx12/d3dx12.h"

void RuntimeTypes::RenderGraphNode_Action_DrawCall::Release(GigiInterpreterPreviewWindowDX12& interpreter)
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

struct SemanticEnumToString
{
	void Function_Position() { m_string = "POSITION"; m_semanticIndex = (int)StructFieldSemantic::Position; }
	void Function_Color() { m_string = "COLOR"; m_semanticIndex = (int)StructFieldSemantic::Color; }
	void Function_Normal() { m_string = "NORMAL"; m_semanticIndex = (int)StructFieldSemantic::Normal; }
	void Function_Tangent() { m_string = "TANGENT"; m_semanticIndex = (int)StructFieldSemantic::Tangent; }
	void Function_UV() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; }
	void Function_MaterialID() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; }
	void Function_ShapeID() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; }
	void Function_Count() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; } // If no semantic given, use auto incrementing texture coordinates
	const char* m_string = "";
	int m_semanticIndex = -1;
};

bool GigiInterpreterPreviewWindowDX12::DrawCall_MakeRootSignature(const RenderGraphNode_Action_DrawCall& node, RuntimeTypes::RenderGraphNode_Action_DrawCall& runtimeData)
{
	// Make the root signature
	if (!runtimeData.m_rootSignature)
	{
		// shader samplers
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
		if (node.vertexShader.shader)
		{
			for (size_t samplerIndex = 0; samplerIndex < node.vertexShader.shader->samplers.size(); ++samplerIndex)
			{
				const ShaderSampler& sampler = node.vertexShader.shader->samplers[samplerIndex];

				D3D12_STATIC_SAMPLER_DESC desc;

				if (!SamplerFilterToD3D12Filter(sampler.filter, desc.Filter))
					return false;

				if (!SamplerAddressModeToD3D12AddressMode(sampler.addressMode, desc.AddressU))
					return false;

				desc.AddressV = desc.AddressW = desc.AddressU;

				desc.MipLODBias = 0;
				desc.MaxAnisotropy = 0;
				desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				desc.MinLOD = 0.0f;
				desc.MaxLOD = D3D12_FLOAT32_MAX;
				desc.ShaderRegister = (UINT)samplerIndex;
				desc.RegisterSpace = 0;
				desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				samplers.push_back(desc);
			}
		}
		if (node.pixelShader.shader)
		{
			for (size_t samplerIndex = 0; samplerIndex < node.pixelShader.shader->samplers.size(); ++samplerIndex)
			{
				const ShaderSampler& sampler = node.pixelShader.shader->samplers[samplerIndex];

				D3D12_STATIC_SAMPLER_DESC desc;

				if (!SamplerFilterToD3D12Filter(sampler.filter, desc.Filter))
					return false;

				if (!SamplerAddressModeToD3D12AddressMode(sampler.addressMode, desc.AddressU))
					return false;

				desc.AddressV = desc.AddressW = desc.AddressU;

				desc.MipLODBias = 0;
				desc.MaxAnisotropy = 0;
				desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				desc.MinLOD = 0.0f;
				desc.MaxLOD = D3D12_FLOAT32_MAX;
				desc.ShaderRegister = (UINT)samplerIndex;
				desc.RegisterSpace = 1;
				desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				samplers.push_back(desc);
			}
		}
		if (node.amplificationShader.shader)
		{
			for (size_t samplerIndex = 0; samplerIndex < node.amplificationShader.shader->samplers.size(); ++samplerIndex)
			{
				const ShaderSampler& sampler = node.amplificationShader.shader->samplers[samplerIndex];

				D3D12_STATIC_SAMPLER_DESC desc;

				if (!SamplerFilterToD3D12Filter(sampler.filter, desc.Filter))
					return false;

				if (!SamplerAddressModeToD3D12AddressMode(sampler.addressMode, desc.AddressU))
					return false;

				desc.AddressV = desc.AddressW = desc.AddressU;

				desc.MipLODBias = 0;
				desc.MaxAnisotropy = 0;
				desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				desc.MinLOD = 0.0f;
				desc.MaxLOD = D3D12_FLOAT32_MAX;
				desc.ShaderRegister = (UINT)samplerIndex;
				desc.RegisterSpace = 1;
				desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_AMPLIFICATION;

				samplers.push_back(desc);
			}
		}
		if (node.meshShader.shader)
		{
			for (size_t samplerIndex = 0; samplerIndex < node.meshShader.shader->samplers.size(); ++samplerIndex)
			{
				const ShaderSampler& sampler = node.meshShader.shader->samplers[samplerIndex];

				D3D12_STATIC_SAMPLER_DESC desc;

				if (!SamplerFilterToD3D12Filter(sampler.filter, desc.Filter))
					return false;

				if (!SamplerAddressModeToD3D12AddressMode(sampler.addressMode, desc.AddressU))
					return false;

				desc.AddressV = desc.AddressW = desc.AddressU;

				desc.MipLODBias = 0;
				desc.MaxAnisotropy = 0;
				desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				desc.MinLOD = 0.0f;
				desc.MaxLOD = D3D12_FLOAT32_MAX;
				desc.ShaderRegister = sampler.registerIndex;
				desc.RegisterSpace = 1;
				desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

				samplers.push_back(desc);
			}
		}

		// Vertex Descriptor table
		std::vector<D3D12_DESCRIPTOR_RANGE> rangesVertex;
		if (node.vertexShader.shader)
		{
			for (const ShaderResource& resource : node.vertexShader.shader->resources)
			{
				D3D12_DESCRIPTOR_RANGE desc;

				switch (resource.access)
				{
					case ShaderResourceAccessType::UAV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
					case ShaderResourceAccessType::RTScene:
					case ShaderResourceAccessType::SRV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
					case ShaderResourceAccessType::CBV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
					default: return false;
				}

				desc.NumDescriptors = 1;
				desc.BaseShaderRegister = resource.registerIndex;
				desc.RegisterSpace = 0;
				desc.OffsetInDescriptorsFromTableStart = (UINT)rangesVertex.size();

				rangesVertex.push_back(desc);
			}
		}

		// Pixel Descriptor table
		std::vector<D3D12_DESCRIPTOR_RANGE> rangesPixel;
		if (node.pixelShader.shader)
		{
			for (const ShaderResource& resource : node.pixelShader.shader->resources)
			{
				D3D12_DESCRIPTOR_RANGE desc;

				switch (resource.access)
				{
					case ShaderResourceAccessType::UAV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
					case ShaderResourceAccessType::RTScene:
					case ShaderResourceAccessType::SRV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
					case ShaderResourceAccessType::CBV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
					default: return false;
				}

				desc.NumDescriptors = 1;
				desc.BaseShaderRegister = resource.registerIndex;
				desc.RegisterSpace = 0;
				desc.OffsetInDescriptorsFromTableStart = (UINT)rangesPixel.size();

				rangesPixel.push_back(desc);
			}
		}

		// Amplification Descriptor table
		std::vector<D3D12_DESCRIPTOR_RANGE> rangesAmplification;
		if (node.amplificationShader.shader)
		{
			for (const ShaderResource& resource : node.amplificationShader.shader->resources)
			{
				D3D12_DESCRIPTOR_RANGE desc;

				switch (resource.access)
				{
					case ShaderResourceAccessType::UAV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
					case ShaderResourceAccessType::RTScene:
					case ShaderResourceAccessType::SRV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
					case ShaderResourceAccessType::CBV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
					default: return false;
				}

				desc.NumDescriptors = 1;
				desc.BaseShaderRegister = resource.registerIndex;
				desc.RegisterSpace = 0;
				desc.OffsetInDescriptorsFromTableStart = (UINT)rangesAmplification.size();

				rangesAmplification.push_back(desc);
			}
		}

		// Mesh Descriptor table
		std::vector<D3D12_DESCRIPTOR_RANGE> rangesMesh;
		if (node.meshShader.shader)
		{
			for (const ShaderResource& resource : node.meshShader.shader->resources)
			{
				D3D12_DESCRIPTOR_RANGE desc;

				switch (resource.access)
				{
					case ShaderResourceAccessType::UAV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
					case ShaderResourceAccessType::RTScene:
					case ShaderResourceAccessType::SRV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
					case ShaderResourceAccessType::CBV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
					default: return false;
				}

				desc.NumDescriptors = 1;
				desc.BaseShaderRegister = resource.registerIndex;
				desc.RegisterSpace = 0;
				desc.OffsetInDescriptorsFromTableStart = (UINT)rangesMesh.size();

				rangesMesh.push_back(desc);
			}
		}

		// Vertex Root parameter
		int numParams = 0;
		D3D12_ROOT_PARAMETER rootParams[4];
		if (rangesVertex.size() > 0)
		{
			rootParams[numParams].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[numParams].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParams[numParams].DescriptorTable.NumDescriptorRanges = (UINT)rangesVertex.size();
			rootParams[numParams].DescriptorTable.pDescriptorRanges = rangesVertex.data();
			numParams++;
		}

		// Pixel Root parameter
		if (rangesPixel.size() > 0)
		{
			rootParams[numParams].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[numParams].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParams[numParams].DescriptorTable.NumDescriptorRanges = (UINT)rangesPixel.size();
			rootParams[numParams].DescriptorTable.pDescriptorRanges = rangesPixel.data();
			numParams++;
		}

		// Amplification Root parameter
		if (rangesAmplification.size() > 0)
		{
			rootParams[numParams].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[numParams].ShaderVisibility = D3D12_SHADER_VISIBILITY_AMPLIFICATION;
			rootParams[numParams].DescriptorTable.NumDescriptorRanges = (UINT)rangesAmplification.size();
			rootParams[numParams].DescriptorTable.pDescriptorRanges = rangesAmplification.data();
			numParams++;
		}

		// Mesh Root parameter
		if (rangesMesh.size() > 0)
		{
			rootParams[numParams].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[numParams].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;
			rootParams[numParams].DescriptorTable.NumDescriptorRanges = (UINT)rangesMesh.size();
			rootParams[numParams].DescriptorTable.pDescriptorRanges = rangesMesh.data();
			numParams++;
		}

		// Root desc
		D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.NumParameters = numParams;
		rootDesc.pParameters = rootParams;
		rootDesc.NumStaticSamplers = (UINT)samplers.size();
		rootDesc.pStaticSamplers = samplers.data();
		rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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

		OnRootSignature(sig, node.amplificationShader.shader);
		OnRootSignature(sig, node.meshShader.shader);
		OnRootSignature(sig, node.vertexShader.shader);
		OnRootSignature(sig, node.pixelShader.shader);



		char* sigptr = (char*)sig->GetBufferPointer();
		auto sigsize = sig->GetBufferSize();

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
	if (!runtimeData.m_pso)
	{
		// Find the maximum used UV slot non UV fields that use tex coord fields can use numbers greater than that
		int maxUsedUVSlot = -1;
		{
			if (node.vertexBuffer.resourceNodeIndex != -1)
			{
				bool exists = false;
				const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(m_renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer.name.c_str(), exists);
				if (exists && resourceInfo.m_resource && resourceInfo.m_structIndex != -1)
				{
					const Struct& structDesc = m_renderGraph.structs[resourceInfo.m_structIndex];
					for (const StructField& field : structDesc.fields)
					{
						if (field.semantic == StructFieldSemantic::UV)
							maxUsedUVSlot = max(maxUsedUVSlot, field.semanticIndex);
					}
				}
			}
			if (node.instanceBuffer.resourceNodeIndex != -1)
			{
				bool exists = false;
				const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(m_renderGraph.nodes[node.instanceBuffer.resourceNodeIndex].resourceBuffer.name.c_str(), exists);
				if (exists && resourceInfo.m_resource && resourceInfo.m_structIndex != -1)
				{
					const Struct& structDesc = m_renderGraph.structs[resourceInfo.m_structIndex];
					for (const StructField& field : structDesc.fields)
					{
						if (field.semantic == StructFieldSemantic::UV)
							maxUsedUVSlot = max(maxUsedUVSlot, field.semanticIndex);
					}
				}
			}
		}

		// Add the vertex buffer to the vertex input layout
		struct VertexInputLayoutEx
		{
			DataFieldType type;
			std::string name;
		};
		std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;
		std::vector<VertexInputLayoutEx> vertexInputLayoutEx;
		if (node.vertexBuffer.resourceNodeIndex != -1)
		{
			if (m_renderGraph.nodes[node.vertexBuffer.resourceNodeIndex]._index != RenderGraphNode::c_index_resourceBuffer)
				return false;

			bool exists = false;
			const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(m_renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer.name.c_str(), exists);
			if (!exists || !resourceInfo.m_resource)
			{
				std::ostringstream ss;
				ss << "vertex buffer not created: \"" << m_renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer.name << "\"\n";
				runtimeData.m_renderGraphText = ss.str();
                runtimeData.m_inErrorState = true;
				return true;
			}

			if (resourceInfo.m_structIndex != -1)
			{
				int offset = 0;
				const Struct& structDesc = m_renderGraph.structs[resourceInfo.m_structIndex];
				for (const StructField& field : structDesc.fields)
				{
					// Get the field info
					DataFieldTypeInfoStructDX12 fieldInfo = DataFieldTypeInfoDX12(field.type);

					// Get the semantic string
					SemanticEnumToString obj;
					EnumDispatch(obj, field.semantic);
					const char* semanticString = obj.m_string;

					// Only include this if there is a semantic string
					if (semanticString[0] != 0)
					{
						// Use the field's semanticIndex, unless this field wants to use TEXCOORD but is not a UV.
						// Then we need to give valid free texcoord indices.
						int semanticIndex = field.semanticIndex;
						if (!_stricmp(semanticString, "TEXCOORD") && field.semantic != StructFieldSemantic::UV)
						{
							maxUsedUVSlot++;
							semanticIndex = maxUsedUVSlot;
						}

						VertexInputLayoutEx layoutEx;
						layoutEx.type = field.type;
						layoutEx.name = field.name;
						vertexInputLayoutEx.push_back(layoutEx);

						D3D12_INPUT_ELEMENT_DESC desc;
						desc.SemanticName = semanticString;
						desc.SemanticIndex = semanticIndex;
						desc.Format = fieldInfo.typeFormat;
						desc.InputSlot = 0;
						desc.AlignedByteOffset = offset;
						desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
						desc.InstanceDataStepRate = 0;
						vertexInputLayout.push_back(desc);
					}

					// keep track of the offset
					offset += fieldInfo.typeBytes;
				}
			}
			else
			{
				VertexInputLayoutEx layoutEx;
				layoutEx.type = m_renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer.format.type;
				layoutEx.name = "position";
				vertexInputLayoutEx.push_back(layoutEx);

				vertexInputLayout.push_back({ "POSITION", 0, resourceInfo.m_format, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			}
		}

		// Add the instance buffer to the vertex input layout
		if (node.instanceBuffer.resourceNodeIndex != -1)
		{
			if (m_renderGraph.nodes[node.instanceBuffer.resourceNodeIndex]._index != RenderGraphNode::c_index_resourceBuffer)
				return false;

			bool exists = false;
			const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(m_renderGraph.nodes[node.instanceBuffer.resourceNodeIndex].resourceBuffer.name.c_str(), exists);
			if (!exists || !resourceInfo.m_resource)
				return true;

			if (resourceInfo.m_structIndex != -1)
			{
				int offset = 0;
				const Struct& structDesc = m_renderGraph.structs[resourceInfo.m_structIndex];
				for (const StructField& field : structDesc.fields)
				{
					// Get the field info
					DataFieldTypeInfoStructDX12 fieldInfo = DataFieldTypeInfoDX12(field.type);

					// Get the semantic string
					SemanticEnumToString obj;
					EnumDispatch(obj, field.semantic);
					const char* semanticString = obj.m_string;

					// Only include this if there is a semantic string
					if (semanticString[0] != 0)
					{
						// Use the field's semanticIndex, unless this field wants to use TEXCOORD but is not a UV.
						// Then we need to give valid free texcoord indices.
						int semanticIndex = field.semanticIndex;
						if (!_stricmp(semanticString, "TEXCOORD") && field.semantic != StructFieldSemantic::UV)
						{
							maxUsedUVSlot++;
							semanticIndex = maxUsedUVSlot;
						}

						VertexInputLayoutEx layoutEx;
						layoutEx.type = field.type;
						layoutEx.name = field.name;
						vertexInputLayoutEx.push_back(layoutEx);

						D3D12_INPUT_ELEMENT_DESC desc;
						desc.SemanticName = semanticString;
						desc.SemanticIndex = semanticIndex;
						desc.Format = fieldInfo.typeFormat;
						desc.InputSlot = 1;
						desc.AlignedByteOffset = offset;
						desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
						desc.InstanceDataStepRate = 1;
						vertexInputLayout.push_back(desc);
					}

					// keep track of the offset
					offset += fieldInfo.typeBytes;
				}
			}
			else
			{
				SemanticEnumToString obj;
				EnumDispatch(obj, StructFieldSemantic::UV);
				maxUsedUVSlot++;

				VertexInputLayoutEx layoutEx;
				layoutEx.type = m_renderGraph.nodes[node.instanceBuffer.resourceNodeIndex].resourceBuffer.format.type;
				layoutEx.name = "instance";
				vertexInputLayoutEx.push_back(layoutEx);

				vertexInputLayout.push_back({ obj.m_string, (UINT)maxUsedUVSlot, resourceInfo.m_format, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
			}
		}

		// Make the PSO desc
		enum class SetupPSODescRet
		{
			True,
			False,
			None
		};

		auto SetupPSODesc = [&](auto& psoDesc) -> SetupPSODescRet
		{
			psoDesc.pRootSignature = runtimeData.m_rootSignature;
			psoDesc.PS.pShaderBytecode = runtimeData.m_pixelShaderBytes.data();
			psoDesc.PS.BytecodeLength = (UINT)runtimeData.m_pixelShaderBytes.size();
			psoDesc.RasterizerState.FillMode = m_drawWireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
			psoDesc.RasterizerState.CullMode = DrawCullModeToD3D12_CULL_MODE(node.cullMode);
			psoDesc.RasterizerState.FrontCounterClockwise = node.frontIsCounterClockwise;
			psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			psoDesc.RasterizerState.DepthClipEnable = TRUE;
			psoDesc.RasterizerState.MultisampleEnable = FALSE;
			psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
			psoDesc.RasterizerState.ForcedSampleCount = 0;
			psoDesc.RasterizerState.ConservativeRaster = node.conservativeRasterization ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			psoDesc.BlendState.AlphaToCoverageEnable = node.alphaAsCoverage ? TRUE : FALSE;
			psoDesc.BlendState.IndependentBlendEnable = node.independentAlpha;

			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			{
				unsigned char writeChannelFlags = 0;
				for (int channelIndex = 0; channelIndex < 4; ++channelIndex)
				{
					if (node.colorTargetSettings[i].writeChannels[channelIndex])
						writeChannelFlags |= (1 << channelIndex);
				}

				psoDesc.BlendState.RenderTarget[i] = D3D12_RENDER_TARGET_BLEND_DESC{
					node.colorTargetSettings[i].enableBlending, FALSE,
					DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].srcBlend), DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].destBlend), D3D12_BLEND_OP_ADD,
					DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].srcBlendAlpha), DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].destBlendAlpha), D3D12_BLEND_OP_ADD,
					D3D12_LOGIC_OP_NOOP,
					writeChannelFlags
				};
			}
			psoDesc.DepthStencilState.DepthWriteMask = node.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
			psoDesc.DepthStencilState.DepthFunc = DepthTestFunctionToD3D12_COMPARISON_FUNC(node.depthTest);
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = GeometryTypeToD3D12_PRIMITIVE_TOPOLOGY_TYPE(node.geometryType);

			// -1 is not found yet
			int sampleCount = -1;

			// Set the render targets
			psoDesc.NumRenderTargets = 0;
			for (int i = 0; i < node.colorTargets.size(); ++i)
			{
				if (node.colorTargets[i].resourceNodeIndex == -1)
					break;

				const RenderGraphNode& resourceNode = m_renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
				if (resourceNode._index != RenderGraphNode::c_index_resourceTexture)
					break;

				bool exists = false;
				const RuntimeTypes::RenderGraphNode_Resource_Texture& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str(), exists);
				if (!exists || !textureInfo.m_resource)
					return SetupPSODescRet::True;

				psoDesc.RTVFormats[psoDesc.NumRenderTargets] = textureInfo.m_format;
				psoDesc.NumRenderTargets++;

                if (sampleCount != -1)
                {
                    if(sampleCount != textureInfo.sampleCount)
//todo                    static bool erroredHere = false;
//                   if (!erroredHere)
//                        m_logFn(LogLevel::Error, "Depth texture \"%s\" is not a valid depth format: %s", resourceNode.resourceTexture.name.c_str(), textureInfoFormatInfo.name);
//                    erroredHere = true;
                    return SetupPSODescRet::False;
                }
                else sampleCount = textureInfo.sampleCount;
			}

			// Set the depth target
			psoDesc.DepthStencilState.DepthEnable = FALSE;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			if (node.depthTarget.resourceNodeIndex != -1)
			{
				const RenderGraphNode& resourceNode = m_renderGraph.nodes[node.depthTarget.resourceNodeIndex];
				if (resourceNode._index != RenderGraphNode::c_index_resourceTexture)
					return SetupPSODescRet::True;

				bool exists = false;
				const RuntimeTypes::RenderGraphNode_Resource_Texture& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str(), exists);
				if (exists && textureInfo.m_resource)
				{
					psoDesc.DepthStencilState.DepthEnable = TRUE;
					psoDesc.DSVFormat = DSV_Safe_DXGI_FORMAT(textureInfo.m_format);

					DXGI_FORMAT_Info textureInfoFormatInfo = Get_DXGI_FORMAT_Info(textureInfo.m_format);
					if (textureInfoFormatInfo.isStencil)
					{
						psoDesc.DepthStencilState.StencilEnable = TRUE;

						psoDesc.DepthStencilState.StencilReadMask = node.stencilReadMask;
						psoDesc.DepthStencilState.StencilWriteMask = node.stencilWriteMask;

						psoDesc.DepthStencilState.FrontFace.StencilFailOp = StencilOpToD3D12_STENCIL_OP(node.frontFaceStencilFail);
						psoDesc.DepthStencilState.BackFace.StencilFailOp = StencilOpToD3D12_STENCIL_OP(node.backFaceStencilFail);
						psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = StencilOpToD3D12_STENCIL_OP(node.frontFaceStencilDepthFail);
						psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = StencilOpToD3D12_STENCIL_OP(node.backFaceStencilDepthFail);
						psoDesc.DepthStencilState.FrontFace.StencilPassOp = StencilOpToD3D12_STENCIL_OP(node.frontFaceStencilPass);
						psoDesc.DepthStencilState.BackFace.StencilPassOp = StencilOpToD3D12_STENCIL_OP(node.backFaceStencilPass);
						psoDesc.DepthStencilState.FrontFace.StencilFunc = DepthTestFunctionToD3D12_COMPARISON_FUNC(node.frontFaceStencilFunc);
						psoDesc.DepthStencilState.BackFace.StencilFunc = DepthTestFunctionToD3D12_COMPARISON_FUNC(node.backFaceStencilFunc);
					}
					if (!textureInfoFormatInfo.isDepth)
					{
						static bool erroredHere = false;
						if (!erroredHere)
							m_logFn(LogLevel::Error, "Depth texture \"%s\" is not a valid depth format: %s", resourceNode.resourceTexture.name.c_str(), textureInfoFormatInfo.name);
						erroredHere = true;
						return SetupPSODescRet::False;
					}

                    if (sampleCount != -1)
                    {
                        if (sampleCount != textureInfo.sampleCount)
                            //todo                    static bool erroredHere = false;
                            //                   if (!erroredHere)
                            //                        m_logFn(LogLevel::Error, "Depth texture \"%s\" is not a valid depth format: %s", resourceNode.resourceTexture.name.c_str(), textureInfoFormatInfo.name);
                            //                    erroredHere = true;
                            return SetupPSODescRet::False;
                    }
                    else sampleCount = textureInfo.sampleCount;

				}
			}

            assert(sampleCount > 0);
			psoDesc.SampleDesc.Count = sampleCount;

			return SetupPSODescRet::None;
		};

		// if using a mesh shader
		if (node.meshShader.shader)
		{
			D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
			SetupPSODescRet ret = SetupPSODesc(psoDesc);
			if (ret != SetupPSODescRet::None)
				return ret == SetupPSODescRet::True;

			psoDesc.AS.pShaderBytecode = runtimeData.m_amplificationShaderBytes.data();
			psoDesc.AS.BytecodeLength = (UINT)runtimeData.m_amplificationShaderBytes.size();
			psoDesc.MS.pShaderBytecode = runtimeData.m_meshShaderBytes.data();
			psoDesc.MS.BytecodeLength = (UINT)runtimeData.m_meshShaderBytes.size();

			CD3DX12_PIPELINE_MESH_STATE_STREAM meshStreamDesc = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

			// Point to our populated stream desc
			D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
			streamDesc.SizeInBytes = sizeof(meshStreamDesc);
			streamDesc.pPipelineStateSubobjectStream = &meshStreamDesc;

			HRESULT hr = m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&runtimeData.m_pso));

			if (FAILED(hr))
			{
				_com_error err(hr);
				m_logFn(LogLevel::Error, "Could not create Mesh Shader PSO for draw call %s:\n%s", node.name.c_str(), FromWideString(err.ErrorMessage()).c_str());
				runtimeData.m_failed = true;
				return false;
			}
		}
		// else using a vertex shader
		else
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			SetupPSODescRet ret = SetupPSODesc(psoDesc);
			if (ret != SetupPSODescRet::None)
				return ret == SetupPSODescRet::True;

			if (vertexInputLayout.size() > 0)
				psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };
			else
				psoDesc.InputLayout = { nullptr, 0 };

			psoDesc.VS.pShaderBytecode = runtimeData.m_vertexShaderBytes.data();
			psoDesc.VS.BytecodeLength = (UINT)runtimeData.m_vertexShaderBytes.size();

			HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&runtimeData.m_pso));

			if (FAILED(hr))
			{
				_com_error err(hr);
				m_logFn(LogLevel::Error, "Could not create Vertex Shader PSO for draw call %s:\n%s", node.name.c_str(), FromWideString(err.ErrorMessage()).c_str());

				// Provide more possible information if we got E_INVALIDARG
				if (hr == E_INVALIDARG)
				{
					std::ostringstream message;

					message <<
						"This error could be caused by the vertex shader using incorrect semantics.  The vertex shader input should look like this:\n"
						"\n"
						"struct VSInput\n"
						"{\n"
						;

					for (size_t elementIndex = 0; elementIndex < vertexInputLayout.size(); ++elementIndex)
					{
						const D3D12_INPUT_ELEMENT_DESC& desc = vertexInputLayout[elementIndex];
						VertexInputLayoutEx& descEx = vertexInputLayoutEx[elementIndex];

						message << "\t" << DataFieldTypeToHLSLType(descEx.type) << " " << descEx.name << " : " << desc.SemanticName;

						if (desc.SemanticIndex > 0)
							message << desc.SemanticIndex;
						message << ";\n";
					}

					message <<
						"};\n"
						;

					m_logFn(LogLevel::Error, "%s", message.str().c_str());
				}

				runtimeData.m_failed = true;
				return false;
			}
		}

		// name the PSO for debuggers
		runtimeData.m_pso->SetName(ToWideString(node.name.c_str()).c_str());

		// clear out the compiled shader bytes
		runtimeData.m_vertexShaderBytes.clear();
		runtimeData.m_pixelShaderBytes.clear();
		runtimeData.m_amplificationShaderBytes.clear();
		runtimeData.m_meshShaderBytes.clear();
	}

	return true;
}

 bool GigiInterpreterPreviewWindowDX12::DrawCall_MakeDescriptorTableDesc(std::vector<DescriptorTableCache::ResourceDescriptor>& descs, const RenderGraphNode_Action_DrawCall& node, const Shader& shader, int pinOffset, std::vector<TransitionTracker::Item>& queuedTransitions, const std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES>& importantResourceStates)
{
	for (int resourceIndex = 0; resourceIndex < shader.resources.size(); ++resourceIndex)
	{
		const ShaderResource& shaderResource = shader.resources[resourceIndex];

		int depIndex = 0;
		while (depIndex < node.resourceDependencies.size() && node.resourceDependencies[depIndex].pinIndex != (resourceIndex+pinOffset))
			depIndex++;

		if (depIndex >= node.resourceDependencies.size())
		{
			m_logFn(LogLevel::Error, "Could not find resource dependency for shader resource \"%s\" in draw call node \"%s\"", shaderResource.name.c_str(), node.name.c_str());
			return false;
		}
		const ResourceDependency& dep = node.resourceDependencies[depIndex];

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
					case TextureDimensionType::Texture2DMS: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2DMS; break;
					default:
						assert(0);
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
				const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());

				if (dep.access == ShaderResourceAccessType::RTScene)
				{
					desc.m_resource = resourceInfo.m_tlas;
					desc.m_format = DXGI_FORMAT_UNKNOWN;
					desc.m_stride = resourceInfo.m_tlasSize;
					desc.m_count = 1;
					desc.m_raw = false;
				}
				else
				{
					desc.m_resource = resourceInfo.m_resource;

					const ShaderResourceBuffer& shaderResourceBuffer = shader.resources[resourceIndex].buffer;
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

                    if (dep.pinIndex < node.linkProperties.size())
                    {
                        const LinkProperties& linkProperties = node.linkProperties[dep.pinIndex];

                        unsigned int unitsDivider = 1;
                        if (linkProperties.bufferViewUnits != MemoryUnitOfMeasurement::Items)
                        {
                            unitsDivider = (shaderResourceBuffer.typeStruct.structIndex != -1)
                                ? (UINT)m_renderGraph.structs[shaderResourceBuffer.typeStruct.structIndex].sizeInBytes
                                : DataFieldTypeInfoDX12(shaderResourceBuffer.type).typeBytes;
                            unitsDivider = max(unitsDivider, 1);
                        }

                        unsigned int bufferViewBegin = linkProperties.bufferViewBegin;
                        if (linkProperties.bufferViewBeginVariable.variableIndex != -1)
                        {
                            if (!GetRuntimeVariableAllowCast(linkProperties.bufferViewBeginVariable.variableIndex, bufferViewBegin))
                                return false;
                        }

                        unsigned int bufferViewSize = linkProperties.bufferViewSize;
                        if (linkProperties.bufferViewSizeVariable.variableIndex != -1)
                        {
                            if (!GetRuntimeVariableAllowCast(linkProperties.bufferViewSizeVariable.variableIndex, bufferViewSize))
                                return false;
                        }

                        desc.m_firstElement = bufferViewBegin / unitsDivider;

                        if (desc.m_count >= desc.m_firstElement)
                            desc.m_count -= desc.m_firstElement;
                        else
                            desc.m_count = 0;

                        unsigned int bufferViewNumElements = bufferViewSize / unitsDivider;
                        if (bufferViewNumElements > 0)
                            desc.m_count = min(desc.m_count, bufferViewNumElements);
                    }

					desc.m_raw = shader.resources[resourceIndex].buffer.raw;
				}
				break;
			}
			default:
			{
				m_logFn(LogLevel::Error, "Unhandled dependency node type for draw call node \"%s\"", node.name.c_str());
				return false;
			}
		}

		// This could be a temporary thing, but we can't run the draw call if we don't have the resources we need.
		if (!desc.m_resource)
			return true;

		switch (dep.access)
		{
			case ShaderResourceAccessType::UAV:
			{
				desc.m_access = DescriptorTableCache::AccessType::UAV;
				queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, GetNodeName(resourceNode).c_str()) });
				break;
			}
			case ShaderResourceAccessType::RTScene:
			{
				desc.m_access = DescriptorTableCache::AccessType::SRV;
				queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, GetNodeName(resourceNode).c_str()) });
				break;
			}
			case ShaderResourceAccessType::SRV:
			{
				desc.m_access = DescriptorTableCache::AccessType::SRV;

				if (importantResourceStates.count(desc.m_resource) == 0)
					queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, GetNodeName(resourceNode).c_str()) });

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
				m_logFn(LogLevel::Error, "Unhandled shader resource access type \"%s\" for draw call node \"%s\"", EnumToString(dep.access), node.name.c_str());
				return false;
			}
		}

		switch (dep.type)
		{
			case ShaderResourceType::Texture: break;// Handled above
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
				m_logFn(LogLevel::Error, "Unhandled shader resource type \"%s\" for draw call node \"%s\"", EnumToString(dep.type), node.name.c_str());
				return false;
			}
		}

		descs.push_back(desc);
	}

	return true;
}

static bool CompileShader(std::vector<unsigned char>& shaderBytes, const Shader& shader, const char* shaderModel, const std::string& directory, std::vector<std::string>& allShaderFiles, const RenderGraph& renderGraph, bool compileShadersForDebug, bool native16BitShaderOpsSupported, LogFn& logFn)
{
	std::string fullFileName = (std::filesystem::path(directory) / "shaders" / shader.destFileName).string();

	ShaderCompilationInfo shaderCompilationInfo;
	shaderCompilationInfo.fileName = fullFileName;
	shaderCompilationInfo.entryPoint = shader.entryPoint;
	shaderCompilationInfo.shaderModel = shaderModel;
	shaderCompilationInfo.defines = shader.defines;

	if (compileShadersForDebug)
	{
		shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
	}

	if (renderGraph.settings.dx12.Allow16BitTypes && native16BitShaderOpsSupported)
		shaderCompilationInfo.flags |= ShaderCompilationFlags::Enable16BitTypes;

	if (renderGraph.settings.dx12.DXC_HLSL_2021)
	{
		shaderCompilationInfo.flags |= ShaderCompilationFlags::HLSL2021;
	}

	if (renderGraph.settings.common.shaderWarningAsErrors)
	{
		shaderCompilationInfo.flags |= ShaderCompilationFlags::WarningsAsErrors;
	}

	if (renderGraph.settings.common.createPDBsAndBinaries)
	{
		shaderCompilationInfo.flags |= ShaderCompilationFlags::CreatePDBsAndBinaries;
	}

	switch (renderGraph.settings.dx12.shaderCompiler)
	{
		case DXShaderCompiler::FXC:
		{
			shaderBytes = CompileShaderToByteCode_fxc(
				shaderCompilationInfo,
				logFn,
				&allShaderFiles
			);
			break;
		}
		case DXShaderCompiler::DXC:
		{
			shaderBytes = CompileShaderToByteCode_dxc(
				shaderCompilationInfo,
				logFn,
				&allShaderFiles
			);
			break;
		}
	}

	return shaderBytes.size() > 0;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_DrawCall& node, RuntimeTypes::RenderGraphNode_Action_DrawCall& runtimeData, NodeAction nodeAction)
{
	ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, false);

	if (nodeAction == NodeAction::Init)
	{
		std::vector<std::string> allShaderFiles;

		// Compile the shaders
		bool compiledOK = true;

		if (node.vertexShader.shader)
			compiledOK &= CompileShader(runtimeData.m_vertexShaderBytes, *node.vertexShader.shader, m_renderGraph.settings.dx12.shaderModelVs.c_str(), GetTempDirectory(), allShaderFiles, m_renderGraph, m_compileShadersForDebug, m_dx12_options4.Native16BitShaderOpsSupported, m_logFn);

		if (node.pixelShader.shader)
			compiledOK &= CompileShader(runtimeData.m_pixelShaderBytes, *node.pixelShader.shader, m_renderGraph.settings.dx12.shaderModelPs.c_str(), GetTempDirectory(), allShaderFiles, m_renderGraph, m_compileShadersForDebug, m_dx12_options4.Native16BitShaderOpsSupported, m_logFn);

		if (node.amplificationShader.shader)
			compiledOK &= CompileShader(runtimeData.m_amplificationShaderBytes, *node.amplificationShader.shader, m_renderGraph.settings.dx12.shaderModelAs.c_str(), GetTempDirectory(), allShaderFiles, m_renderGraph, m_compileShadersForDebug, m_dx12_options4.Native16BitShaderOpsSupported, m_logFn);

		if (node.meshShader.shader)
			compiledOK &= CompileShader(runtimeData.m_meshShaderBytes, *node.meshShader.shader, m_renderGraph.settings.dx12.shaderModelMs.c_str(), GetTempDirectory(), allShaderFiles, m_renderGraph, m_compileShadersForDebug, m_dx12_options4.Native16BitShaderOpsSupported, m_logFn);

		// Watch the shader file source for file changes, even if it failed compilation, so we can detect when it's edited and try again
		for (const std::string& fileName : allShaderFiles)
		{
			std::string sourceFileName = (std::filesystem::path(m_renderGraph.baseDirectory) / std::filesystem::proximate(fileName, std::filesystem::path(GetTempDirectory()) / "shaders")).string();
			m_fileWatcher.Add(sourceFileName.c_str(), FileWatchOwner::Shaders);
		}

		if (!compiledOK)
			return false;
	}

	if (nodeAction == NodeAction::Execute)
	{
		if (runtimeData.m_failed)
			return false;

		// make sure root signature and pso are made
		if (!runtimeData.m_rootSignature || !runtimeData.m_pso)
		{
			if (!DrawCall_MakeRootSignature(node, runtimeData))
				return false;

			if (!runtimeData.m_rootSignature || !runtimeData.m_pso)
				return true;
		}

		// If we aren't supposed to do the draw call, exit out
        if (!EvaluateCondition(node.condition))
        {
            runtimeData.m_conditionIsTrue = false;
            return true;
        }
        runtimeData.m_conditionIsTrue = true;

		// Queue up transitions, so we only do them if we actually execute the node
		std::vector<TransitionTracker::Item> queuedTransitions;

		// If a resource is used as a vertex buffer and srv, it should transition to only the vertex buffer state.
		// This map is meant to resolve situations like this.
		std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> importantResourceStates;

		// Shading rate image transition
		if (node.shadingRateImage.resourceNodeIndex != -1)
		{
			const RenderGraphNode& resourceNode = m_renderGraph.nodes[node.shadingRateImage.resourceNodeIndex];
			if (resourceNode._index == RenderGraphNode::c_index_resourceTexture)
			{
				bool exists = false;
				const auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str(), exists);
				if (exists && textureInfo.m_resource)
				{
					// publish as a viewable resource
					runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), (node.name + std::string(".shadingRateImage")).c_str(), textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, false);

					// transition
					queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(textureInfo.m_resource, D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE, GetNodeName(resourceNode).c_str()) });

					if (VRSSupportLevel() == D3D12_VARIABLE_SHADING_RATE_TIER_2)
					{
						ID3D12GraphicsCommandList5* VRSCommandList = nullptr;
						if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&VRSCommandList))))
						{
							m_logFn(LogLevel::Error, "Draw call node \"%s\" couldn't get a ID3D12GraphicsCommandList5*", node.name.c_str());
							return false;
						}

						// Set the shading rate image
						VRSCommandList->RSSetShadingRateImage(textureInfo.m_resource);

						VRSCommandList->Release();
					}
					else
					{
						m_logFn(LogLevel::Error, "Draw call node \"%s\" could not enable sparse shading because it is not supported", node.name.c_str());
					}
				}
			}
		}

		// Handle the vertex buffer
		int vertexCountPerInstance = node.countPerInstance;
		if (node.vertexBuffer.resourceNodeIndex != -1)
		{
			const RenderGraphNode& vbNode = m_renderGraph.nodes[node.vertexBuffer.resourceNodeIndex];
			if (vbNode._index == RenderGraphNode::c_index_resourceBuffer)
			{
				bool exists = false;
				const auto& bufferInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(vbNode.resourceBuffer.name.c_str(), exists);
				if (exists && bufferInfo.m_resource)
				{
					// Set the vertex count
					if (vertexCountPerInstance == -1)
						vertexCountPerInstance = bufferInfo.m_count;
					else
						vertexCountPerInstance = min(vertexCountPerInstance, bufferInfo.m_count);

					// publish vertex buffer as a viewable resource
					runtimeData.HandleViewableBuffer(*this, (node.name + std::string(".vertexBuffer")).c_str(), bufferInfo.m_resource, bufferInfo.m_format, bufferInfo.m_formatCount, bufferInfo.m_structIndex, bufferInfo.m_size, bufferInfo.m_stride, bufferInfo.m_count, false, false, 0, 0, false);

					// transition to vertex buffer
					queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(bufferInfo.m_resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, vbNode.resourceBuffer.name.c_str()) });

					// set the vertex buffer
					m_commandList->IASetVertexBuffers(0, 1, &bufferInfo.m_vertexBufferView);

					// Remember this state so we don't set it to SRV later
					importantResourceStates[bufferInfo.m_resource] = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
				}
			}
		}

		// Handle the index buffer
		int indexCountPerInstance = node.countPerInstance;
		if (node.indexBuffer.resourceNodeIndex != -1)
		{
			const RenderGraphNode& ibNode = m_renderGraph.nodes[node.indexBuffer.resourceNodeIndex];
			if (ibNode._index == RenderGraphNode::c_index_resourceBuffer)
			{
				bool exists = false;
				const auto& bufferInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(ibNode.resourceBuffer.name.c_str(), exists);
				if (exists && bufferInfo.m_resource)
				{
					// Set the index count
					if (indexCountPerInstance == -1)
						indexCountPerInstance = bufferInfo.m_count;
					else
						indexCountPerInstance = min(indexCountPerInstance, bufferInfo.m_count);

					// publish index buffer as a viewable resource
					runtimeData.HandleViewableBuffer(*this, (node.name + std::string(".indexBuffer")).c_str(), bufferInfo.m_resource, bufferInfo.m_format, bufferInfo.m_formatCount, bufferInfo.m_structIndex, bufferInfo.m_size, bufferInfo.m_stride, bufferInfo.m_count, false, false, 0, 0, false);

					// transition to index buffer
					queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(bufferInfo.m_resource, D3D12_RESOURCE_STATE_INDEX_BUFFER, ibNode.resourceBuffer.name.c_str()) });

					// set the index buffer
					m_commandList->IASetIndexBuffer(&bufferInfo.m_indexBufferView);

					// Remember this state so we don't set it to SRV later
					importantResourceStates[bufferInfo.m_resource] = D3D12_RESOURCE_STATE_INDEX_BUFFER;
				}
			}
		}

		// Handle the instance buffer
		int instanceCount = node.instanceCount;
		if (node.instanceBuffer.resourceNodeIndex != -1)
		{
			const RenderGraphNode& ibNode = m_renderGraph.nodes[node.instanceBuffer.resourceNodeIndex];
			if (ibNode._index == RenderGraphNode::c_index_resourceBuffer)
			{
				bool exists = false;
				const auto& bufferInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(ibNode.resourceBuffer.name.c_str(), exists);
				if (exists && bufferInfo.m_resource)
				{
					// Set the instance count
					if (instanceCount == -1)
						instanceCount = bufferInfo.m_count;
					else
						instanceCount = min(instanceCount, bufferInfo.m_count);

					// publish instance buffer as a viewable resource
					runtimeData.HandleViewableBuffer(*this, (node.name + std::string(".instanceBuffer")).c_str(), bufferInfo.m_resource, bufferInfo.m_format, bufferInfo.m_formatCount, bufferInfo.m_structIndex, bufferInfo.m_size, bufferInfo.m_stride, bufferInfo.m_count, false, false, 0, 0, false);

					// transition to vertex buffer
					queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(bufferInfo.m_resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, ibNode.resourceBuffer.name.c_str()) });

					// set the index buffer
					m_commandList->IASetVertexBuffers(1, 1, &bufferInfo.m_vertexBufferView);

					// Remember this state so we don't set it to SRV later
					importantResourceStates[bufferInfo.m_resource] = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
				}
			}
		}

		// If we are running a vertex shader, validate information for that.
		if (node.vertexShader.shader)
		{
			// if we have an index buffer, we must also have a vertex buffer, and we must have >= 0 indices per instance.
			if (node.indexBuffer.resourceNodeIndex != -1)
			{
				if (node.vertexBuffer.resourceNodeIndex == -1 || indexCountPerInstance < 0)
					return true;
			}
			// else no index buffer. We just need to have >= 0 vertices per instance
			else
			{
				if (vertexCountPerInstance < 0)
					return true;
			}

			// instance count also needs to be >= 0
			if (instanceCount < 0)
				return true;
		}

		// Set the root signature and PSO
		m_commandList->SetGraphicsRootSignature(runtimeData.m_rootSignature);
		m_commandList->SetPipelineState(runtimeData.m_pso);

		struct RenderTargetClearData
		{
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			Vec4 color;
		};
		std::vector<RenderTargetClearData> renderTargetClearData;
		struct DepthTargetClearData
		{
			bool clear = false;
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			D3D12_CLEAR_FLAGS flags;
			float depth;
			uint8_t stencil;
		};
		DepthTargetClearData depthTargetClearData;

		// publish render targets as viewable resources (before)
		int rasterWidth = -1;
		int rasterHeight = -1;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> colorTargetHandles;
		D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = nullptr;
		{
			bool firstRenderTarget = true;
			int renderTargetSize[3] = { 0, 0, 0 };

			for (int i = 0; i < node.colorTargets.size(); ++i)
			{
				if (node.colorTargets[i].resourceNodeIndex == -1)
					break;

				const RenderGraphNode& colorTargetNode = m_renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
				if (colorTargetNode._index != RenderGraphNode::c_index_resourceTexture)
					break;

				char buffer[256];
				sprintf_s(buffer, "%s.colorTarget%i (Before)", node.name.c_str(), i);

				bool exists = false;
				auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(colorTargetNode.resourceTexture.name.c_str(), exists);
				if (!exists || !textureInfo.m_resource)
					break;
				const ColorTargetSettings& ctSettings = node.colorTargetSettings[i];

				int textureMipSize[3] = { textureInfo.m_size[0], textureInfo.m_size[1], textureInfo.m_size[2] };
				for (int i = 0; i < ctSettings.mipLevel; ++i)
				{
					textureMipSize[0] = max(textureMipSize[0] / 2, 1);
					textureMipSize[1] = max(textureMipSize[1] / 2, 1);
				}

				if (firstRenderTarget)
				{
					firstRenderTarget = false;
					renderTargetSize[0] = textureMipSize[0];
					renderTargetSize[1] = textureMipSize[1];
					renderTargetSize[2] = textureMipSize[2];
				}
				else if (
					renderTargetSize[0] != textureMipSize[0] ||
					renderTargetSize[1] != textureMipSize[1])
				{
					std::ostringstream ss;
					ss << "color target " << i << " is size (" << textureMipSize[0] << ", " << textureMipSize[1] << ", " << textureMipSize[2] << ")"
						" which mismatches a previously seen render target size of (" << renderTargetSize[0] << ", " << renderTargetSize[1] << ", " << renderTargetSize[2] << ")";
					runtimeData.m_renderGraphText = ss.str();
                    runtimeData.m_inErrorState = true;
					return true;
				}

				rasterWidth = textureMipSize[0];
				rasterHeight = textureMipSize[1];

				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(colorTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, false);

				queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(textureInfo.m_resource, D3D12_RESOURCE_STATE_RENDER_TARGET, colorTargetNode.resourceTexture.name.c_str()) });

				D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandle;
				if (!textureInfo.GetRTV(m_device, colorTargetHandle, m_RTVHeapAllocationTracker, colorTargetNode.resourceTexture.dimension, ctSettings.arrayIndex, ctSettings.mipLevel, textureInfo.sampleCount, colorTargetNode.resourceTexture.name.c_str()))
				{
					m_logFn(LogLevel::Error, "node \"%s\": cannot make RTV for \"%s\" (%i,%i).\n", node.name.c_str(), colorTargetNode.resourceTexture.name.c_str(), ctSettings.arrayIndex, ctSettings.mipLevel);
					return false;
				}

				if (ctSettings.clear)
					renderTargetClearData.push_back({ colorTargetHandle, ctSettings.clearColor });

				colorTargetHandles.push_back(colorTargetHandle);
			}

			if (node.depthTarget.resourceNodeIndex != -1)
			{
				const RenderGraphNode& depthTargetNode = m_renderGraph.nodes[node.depthTarget.resourceNodeIndex];
				if (depthTargetNode._index == RenderGraphNode::c_index_resourceTexture)
				{
					bool exists = false;
					auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(depthTargetNode.resourceTexture.name.c_str(), exists);
					if (exists && textureInfo.m_resource)
					{
						int textureMipSize[3] = { textureInfo.m_size[0], textureInfo.m_size[1], textureInfo.m_size[2] };
						for (int i = 0; i < node.depthMipLevel; ++i)
						{
							textureMipSize[0] = max(textureMipSize[0] / 2, 1);
							textureMipSize[1] = max(textureMipSize[1] / 2, 1);
						}

						if (firstRenderTarget)
						{
							firstRenderTarget = false;
							renderTargetSize[0] = textureMipSize[0];
							renderTargetSize[1] = textureMipSize[1];
							renderTargetSize[2] = textureMipSize[2];
						}
						else if (
							renderTargetSize[0] != textureMipSize[0] ||
							renderTargetSize[1] != textureMipSize[1])
						{
							std::ostringstream ss;
							ss << "depth target is size (" << textureMipSize[0] << ", " << textureMipSize[1] << ", " << textureMipSize[2] << ")"
								" which mismatches a previously seen render target size of (" << renderTargetSize[0] << ", " << renderTargetSize[1] << ", " << renderTargetSize[2] << ")";
							runtimeData.m_renderGraphText = ss.str();
                            runtimeData.m_inErrorState = true;
							return true;
						}

						rasterWidth = textureMipSize[0];
						rasterHeight = textureMipSize[1];
						char buffer[256];
						sprintf_s(buffer, "%s.depthTarget (Before)", node.name.c_str());
						runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(depthTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, false);

						queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(textureInfo.m_resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, depthTargetNode.resourceTexture.name.c_str()) });

						if (!textureInfo.GetDSV(m_device, depthTargetHandle, m_DSVHeapAllocationTracker, depthTargetNode.resourceTexture.dimension, node.depthArrayIndex, node.depthMipLevel, textureInfo.sampleCount, depthTargetNode.resourceTexture.name.c_str()))
						{
							m_logFn(LogLevel::Error, "node \"%s\": cannot make DSV for \"%s\" (%i,%i).\n", node.name.c_str(), depthTargetNode.resourceTexture.name.c_str(), node.depthArrayIndex, node.depthMipLevel);
							return false;
						}

						depthTargetHandlePtr = &depthTargetHandle;

						if (node.depthTargetClear || node.stencilClear)
						{
							D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
							if (node.depthTargetClear)
								clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
							if (node.stencilClear)
								clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

							depthTargetClearData.clear = true;
							depthTargetClearData.handle = depthTargetHandle;
							depthTargetClearData.flags = clearFlags;
							depthTargetClearData.depth = node.depthTargetClearValue;
							depthTargetClearData.stencil = node.stencilClearValue;
						}
					}
				}
			}
		}

		// publish shader resources as viewable resources, before the shader execution
		int depIndex = -1;
		for (const ResourceDependency& dep : node.resourceDependencies)
		{
			if (dep.access == ShaderResourceAccessType::Indirect)
				continue;

			depIndex++;
			const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];

			const Shader* shader = nullptr;
			std::string shaderResourceName;
			int shaderBasePinIndex = 0;
			{
				int pinIndex = dep.pinIndex;

				if (pinIndex >= 0)
				{
					if (node.vertexShader.shader && pinIndex < node.vertexShader.shader->resources.size())
					{
						shader = node.vertexShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.vertexShader.shader)
					{
						pinIndex -= (int)node.vertexShader.shader->resources.size();
						shaderBasePinIndex += (int)node.vertexShader.shader->resources.size();
					}
				}

				if (pinIndex >= 0)
				{
					if (node.pixelShader.shader && pinIndex < node.pixelShader.shader->resources.size())
					{
						shader = node.pixelShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.pixelShader.shader)
					{
						pinIndex -= (int)node.pixelShader.shader->resources.size();
						shaderBasePinIndex += (int)node.pixelShader.shader->resources.size();
					}
				}

				if (pinIndex >= 0)
				{
					if (node.amplificationShader.shader && pinIndex < node.amplificationShader.shader->resources.size())
					{
						shader = node.amplificationShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.amplificationShader.shader)
					{
						pinIndex -= (int)node.amplificationShader.shader->resources.size();
						shaderBasePinIndex += (int)node.amplificationShader.shader->resources.size();
					}
				}

				if (pinIndex >= 0)
				{
					if (node.meshShader.shader && pinIndex < node.meshShader.shader->resources.size())
					{
						shader = node.meshShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.meshShader.shader)
					{
						pinIndex -= (int)node.meshShader.shader->resources.size();
						shaderBasePinIndex += (int)node.meshShader.shader->resources.size();
					}
				}
			}

			if (!shader)
				continue;

			switch (resourceNode._index)
			{
				case RenderGraphNode::c_index_resourceTexture:
				{
					std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceTexture.name;
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
					std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceShaderConstants.name;
					label = label + " (CBV)";

					const RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_ShaderConstants(resourceNode.resourceShaderConstants.name.c_str());
					runtimeData.HandleViewableConstantBuffer(*this, label.c_str(), resourceInfo.m_buffer->buffer, (int)resourceInfo.m_buffer->size, shader->resources[dep.pinIndex - shaderBasePinIndex].constantBufferStructIndex, false, false);
					break;
				}
				case RenderGraphNode::c_index_resourceBuffer:
				{
					std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceBuffer.name;
					if (dep.access == ShaderResourceAccessType::UAV)
						label = label + " (UAV - Before)";
					else
						label = label + " (SRV)";

                    unsigned int bufferViewBegin = 0;
                    unsigned int bufferViewSize = 0;
                    bool bufferViewInBytes = false;
                    if (dep.pinIndex < node.linkProperties.size())
                    {
                        const LinkProperties& linkProperties = node.linkProperties[dep.pinIndex];
                        bufferViewBegin = linkProperties.bufferViewBegin;
                        bufferViewSize = linkProperties.bufferViewSize;
                        if (linkProperties.bufferViewBeginVariable.variableIndex != -1 && !GetRuntimeVariableAllowCast(linkProperties.bufferViewBeginVariable.variableIndex, bufferViewBegin))
                            return false;
                        if (linkProperties.bufferViewSizeVariable.variableIndex != -1 && !GetRuntimeVariableAllowCast(linkProperties.bufferViewSizeVariable.variableIndex, bufferViewSize))
                            return false;

                        bufferViewInBytes = linkProperties.bufferViewUnits == MemoryUnitOfMeasurement::Bytes;
                    }

					const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
					runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, false, bufferViewBegin, bufferViewSize, bufferViewInBytes);
					break;
				}
			}
		}

		// Make the vertex shader descriptor table gigi description
		std::vector<DescriptorTableCache::ResourceDescriptor> descriptorsVertexShader;
		int descriptorTablePinOffset = 0;
		if (node.vertexShader.shader)
		{
			if (!DrawCall_MakeDescriptorTableDesc(descriptorsVertexShader, node, *node.vertexShader.shader, descriptorTablePinOffset, queuedTransitions, importantResourceStates))
				return false;
			descriptorTablePinOffset += (int)node.vertexShader.shader->resources.size();
		}

		// Make the pixel shader descriptor table gigi description
		std::vector<DescriptorTableCache::ResourceDescriptor> descriptorsPixelShader;
		if (node.pixelShader.shader)
		{
			if (!DrawCall_MakeDescriptorTableDesc(descriptorsPixelShader, node, *node.pixelShader.shader, descriptorTablePinOffset, queuedTransitions, importantResourceStates))
				return false;
			descriptorTablePinOffset += (int)node.pixelShader.shader->resources.size();
		}

		// Make the amplification shader descriptor table gigi description
		std::vector<DescriptorTableCache::ResourceDescriptor> descriptorsAmplificationShader;
		if (node.amplificationShader.shader)
		{
			if (!DrawCall_MakeDescriptorTableDesc(descriptorsAmplificationShader, node, *node.amplificationShader.shader, descriptorTablePinOffset, queuedTransitions, importantResourceStates))
				return false;
			descriptorTablePinOffset += (int)node.amplificationShader.shader->resources.size();
		}

		// Make the mesh shader descriptor table gigi description
		std::vector<DescriptorTableCache::ResourceDescriptor> descriptorsMeshShader;
		if (node.meshShader.shader)
		{
			if (!DrawCall_MakeDescriptorTableDesc(descriptorsMeshShader, node, *node.meshShader.shader, descriptorTablePinOffset, queuedTransitions, importantResourceStates))
				return false;
			descriptorTablePinOffset += (int)node.meshShader.shader->resources.size();
		}

		// Do all resource transitions desired by the descriptor tables
		m_transitions.Transition(queuedTransitions);
		m_transitions.Flush(m_commandList);

		// Get or make the vertex shader descriptor table and set it
		int rootSigParamIndex = 0;
		if (descriptorsVertexShader.size() > 0)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVertexShader;
			std::string error;
			if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptorsVertexShader.data(), (int)descriptorsVertexShader.size(), descriptorTableVertexShader, error, HEAP_DEBUG_TEXT()))
			{
				m_logFn(LogLevel::Error, "Draw call Node \"%s\" could not allocate a descriptor table for VS: %s", node.name.c_str(), error.c_str());
				return false;
			}

			m_commandList->SetGraphicsRootDescriptorTable(rootSigParamIndex, descriptorTableVertexShader);
			rootSigParamIndex++;
		}

		// Get or make the pixel shader descriptor table and set it
		if (descriptorsPixelShader.size() > 0)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE descriptorTablePixelShader;
			std::string error;
			if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptorsPixelShader.data(), (int)descriptorsPixelShader.size(), descriptorTablePixelShader, error, HEAP_DEBUG_TEXT()))
			{
				m_logFn(LogLevel::Error, "Draw call Node \"%s\" could not allocate a descriptor table for PS: %s", node.name.c_str(), error.c_str());
				return false;
			}

			m_commandList->SetGraphicsRootDescriptorTable(rootSigParamIndex, descriptorTablePixelShader);
			rootSigParamIndex++;
		}

		// Get or make the amplification shader descriptor table and set it
		if (descriptorsAmplificationShader.size() > 0)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableAmplificationShader;
			std::string error;
			if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptorsAmplificationShader.data(), (int)descriptorsAmplificationShader.size(), descriptorTableAmplificationShader, error, HEAP_DEBUG_TEXT()))
			{
				m_logFn(LogLevel::Error, "Draw call Node \"%s\" could not allocate a descriptor table for AS: %s", node.name.c_str(), error.c_str());
				return false;
			}

			m_commandList->SetGraphicsRootDescriptorTable(rootSigParamIndex, descriptorTableAmplificationShader);
			rootSigParamIndex++;
		}

		// Get or make the mesh shader descriptor table and set it
		if (descriptorsMeshShader.size() > 0)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableMeshShader;
			std::string error;
			if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptorsMeshShader.data(), (int)descriptorsMeshShader.size(), descriptorTableMeshShader, error, HEAP_DEBUG_TEXT()))
			{
				m_logFn(LogLevel::Error, "Draw call Node \"%s\" could not allocate a descriptor table for MS: %s", node.name.c_str(), error.c_str());
				return false;
			}

			m_commandList->SetGraphicsRootDescriptorTable(rootSigParamIndex, descriptorTableMeshShader);
			rootSigParamIndex++;
		}

		// do color and depth target clears
		for (const RenderTargetClearData& data : renderTargetClearData)
			m_commandList->ClearRenderTargetView(data.handle, data.color.data(), 0, nullptr);
		if (depthTargetClearData.clear)
			m_commandList->ClearDepthStencilView(depthTargetClearData.handle, depthTargetClearData.flags, depthTargetClearData.depth, depthTargetClearData.stencil, 0, nullptr);

		// clear viewport and scissor rect
		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(rasterWidth), float(rasterHeight), 0.0f, 1.0f };
		D3D12_RECT scissorRect = { 0, 0, (LONG)rasterWidth, (LONG)rasterHeight };
		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &scissorRect);

		m_commandList->IASetPrimitiveTopology(GeometryTypeToD3D12_PRIMITIVE_TOPOLOGY(node.geometryType));
		m_commandList->OMSetRenderTargets((UINT)colorTargetHandles.size(), colorTargetHandles.data(), false, depthTargetHandlePtr);
		m_commandList->OMSetStencilRef(node.stencilRef);

		// variable rate shading - set sparse sampling
		if (VRSSupportLevel() > D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
		{
			ID3D12GraphicsCommandList5* VRSCommandList = nullptr;
			if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&VRSCommandList))))
			{
				m_logFn(LogLevel::Error, "Draw call node \"%s\" couldn't get a ID3D12GraphicsCommandList5*", node.name.c_str());
				return false;
			}

			D3D12_SHADING_RATE shadingRate;
			if (!ShadingRateToD3D12_SHADING_RATE(node.shadingRate, shadingRate))
			{
				m_logFn(LogLevel::Error, "Unhandled shading rate \"%s\" in node \"%s\"", EnumToString(node.shadingRate), node.name.c_str());
				return false;
			}

			D3D12_SHADING_RATE_COMBINER combiners[2];
			if (!ShadingRateCombinerToD3D12_SHADING_RATE_COMBINER(node.shadingRateCombiner1, combiners[0]))
			{
				m_logFn(LogLevel::Error, "Unhandled shading rate combiner \"%s\" in node \"%s\"", EnumToString(node.shadingRateCombiner1), node.name.c_str());
				return false;
			}

			if (!ShadingRateCombinerToD3D12_SHADING_RATE_COMBINER(node.shadingRateCombiner2, combiners[1]))
			{
				m_logFn(LogLevel::Error, "Unhandled shading rate combiner \"%s\" in node \"%s\"", EnumToString(node.shadingRateCombiner2), node.name.c_str());
				return false;
			}

			VRSCommandList->RSSetShadingRate(shadingRate, combiners);

			VRSCommandList->Release();
		}

		std::ostringstream ss;

		// If using a mesh shader, do a DispatchMesh call
		if (node.meshShader.shader)
		{
			// calculate dispatch size
			unsigned int dispatchSize[3] = { 1, 1, 1 };
			if (node.meshShaderDispatchSize.node.textureNode)
			{
				IVec3 size = GetDesiredSize(*this, *node.meshShaderDispatchSize.node.textureNode);
				dispatchSize[0] = size[0];
				dispatchSize[1] = size[1];
				dispatchSize[2] = size[2];
			}
			else if (node.meshShaderDispatchSize.node.bufferNode)
			{
				dispatchSize[0] = GetDesiredCount(*this, *node.meshShaderDispatchSize.node.bufferNode);
				dispatchSize[1] = 1;
				dispatchSize[2] = 1;
			}
			else if (node.meshShaderDispatchSize.variable.variableIndex != -1)
			{
				Variable& var = m_renderGraph.variables[node.meshShaderDispatchSize.variable.variableIndex];
				const RuntimeVariable& rtVar = GetRuntimeVariable(node.meshShaderDispatchSize.variable.variableIndex);

				DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(var.type);
				if (typeInfo.componentCount < 1 || typeInfo.componentCount > 3)
				{
					m_logFn(LogLevel::Error, "Draw call node \"%s\" wants to use variable \"%s\" for dispatch size, but has the wrong number of components", node.name.c_str(), var.name.c_str());
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
						m_logFn(LogLevel::Error, "Draw call node \"%s\" wants to use variable \"%s\" for dispatch size, but it is an unsupported type", node.name.c_str(), var.name.c_str());
						return false;
					}
				}
			}

			// Do fixed function calculations on dispatch size
			for (int i = 0; i < 3; ++i)
				dispatchSize[i] = ((dispatchSize[i] + node.meshShaderDispatchSize.preAdd[i]) * node.meshShaderDispatchSize.multiply[i]) / node.meshShaderDispatchSize.divide[i] + node.meshShaderDispatchSize.postAdd[i];

			if (dispatchSize[0] == 0 || dispatchSize[1] == 0 || dispatchSize[2] == 0)
			{
				m_logFn(LogLevel::Error, "Draw call node \"%s\" wanted to do a dispatch of size 0.  dispatchSize = (%u, %u, %u)", node.name.c_str(), dispatchSize[0], dispatchSize[1], dispatchSize[2]);
				return false;
			}

			// Verify numthreads for amplication shader if it exists
			if (node.amplificationShader.shader && (node.amplificationShader.shader->NumThreads[0] == 0 || node.amplificationShader.shader->NumThreads[1] == 0 || node.amplificationShader.shader->NumThreads[2] == 0))
			{
				m_logFn(LogLevel::Error, "Draw call node \"%s\" wanted to run amplification shader with 0 threads.  NumThreads = (%u, %u, %u)", node.name.c_str(), node.amplificationShader.shader->NumThreads[0], node.amplificationShader.shader->NumThreads[1], node.amplificationShader.shader->NumThreads[2]);
				return false;
			}

			// Verify numthreads for mesh shader if it exists
			if (node.meshShader.shader && (node.meshShader.shader->NumThreads[0] == 0 || node.meshShader.shader->NumThreads[1] == 0 || node.meshShader.shader->NumThreads[2] == 0))
			{
				m_logFn(LogLevel::Error, "Draw call node \"%s\" wanted to run mesh shader with 0 threads.  NumThreads = (%u, %u, %u)", node.name.c_str(), node.meshShader.shader->NumThreads[0], node.meshShader.shader->NumThreads[1], node.meshShader.shader->NumThreads[2]);
				return false;
			}

			// do numThreads calculations. Divide by numThreads but round up.
			const std::array<int, 3>& numThreads = node.amplificationShader.shader ? node.amplificationShader.shader->NumThreads : node.meshShader.shader->NumThreads;
			for (int i = 0; i < 3; ++i)
				dispatchSize[i] = (dispatchSize[i] + numThreads[i] - 1) / numThreads[i];

			ID3D12GraphicsCommandList6* meshCommandList = nullptr;
			if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&meshCommandList))))
			{
				m_logFn(LogLevel::Error, "Draw call node \"%s\" couldn't get a ID3D12GraphicsCommandList6*", node.name.c_str());
				return false;
			}

			meshCommandList->DispatchMesh(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

			meshCommandList->Release();

			ss << "Dispatch: (" << dispatchSize[0] << ", " << dispatchSize[1] << ", " << dispatchSize[2] << ")";
			ss << "\n" << (node.amplificationShader.shader ? "Amplification" : "Mesh") << " NumThreads: (" << numThreads[0] << ", " << numThreads[1] << ", " << numThreads[2] << ")";
		}
		// else if we have an indirect buffer, do indirect
		else if (node.enableIndirect && node.indirectBuffer.resourceNodeIndex != -1)
		{
			ss << "Draw: Indirect";
			const std::string& indirectBufferName = m_renderGraph.nodes[node.indirectBuffer.resourceNodeIndex].resourceBuffer.name;
			bool exists = false;
			const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(indirectBufferName.c_str(), exists);
			if (!exists)
				return true;

			std::string label = node.name + std::string(".indirectBuffer") + std::string(": ") + indirectBufferName;
			runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, false, 0, 0, false);

			// This could be a temporary thing, but we can't indirect dispatch if the buffer doesn't exist
			if (!resourceInfo.m_resource)
				return true;

			// Note: maybe this could move earlier, so there isn't an extra transition call here. like in the descriptor table logic even though it doesn't go in the descriptor table?
			m_transitions.Transition(TRANSITION_DEBUG_INFO(resourceInfo.m_resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
			m_transitions.Flush(m_commandList);

			if (node.indexBuffer.resourceNodeIndex != -1)
			{
				ss << "DrawIndexedInstanced";
				m_commandList->ExecuteIndirect(
					m_commandSignatureDrawIndexed,
					1,
					resourceInfo.m_resource,
					0,
					nullptr,
					0);
			}
			else
			{
				ss << "DrawInstanced";
				m_commandList->ExecuteIndirect(
					m_commandSignatureDraw,
					1,
					resourceInfo.m_resource,
					0,
					nullptr,
					0);
			}
		}
		// else do direct
		else
		{
			if (node.indexBuffer.resourceNodeIndex != -1)
			{
				ss << "DrawIndexedInstanced:\n  indexCountPerInstance = " << indexCountPerInstance << "\n  instanceCount = " << instanceCount;
				m_commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, 0, 0, 0);
			}
			// else do DrawInstanced
			else
			{
				ss << "DrawInstanced:\n  vertexCountPerInstance = " << vertexCountPerInstance << "\n  instanceCount = " << instanceCount;
				m_commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
			}
		}

		// variable rate shading - set it back to dense sampling
		if (VRSSupportLevel() > D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
		{
			ID3D12GraphicsCommandList5* VRSCommandList = nullptr;
			if (FAILED(m_commandList->QueryInterface(IID_PPV_ARGS(&VRSCommandList))))
			{
				m_logFn(LogLevel::Error, "Draw call node \"%s\" couldn't get a ID3D12GraphicsCommandList5*", node.name.c_str());
				return false;
			}

			VRSCommandList->RSSetShadingRate(D3D12_SHADING_RATE_1X1, nullptr);

			if (VRSSupportLevel() == D3D12_VARIABLE_SHADING_RATE_TIER_2)
				VRSCommandList->RSSetShadingRateImage(nullptr);

			VRSCommandList->Release();
		}

		if (IsConditional(node.condition))
			ss << "\nCondition: " << (EvaluateCondition(node.condition) ? "true" : "false");
		runtimeData.m_renderGraphText = ss.str();

		// publish render targets as viewable resources (after)
		{
			for (int i = 0; i < node.colorTargets.size(); ++i)
			{
				if (node.colorTargets[i].resourceNodeIndex == -1)
					break;

				const RenderGraphNode& colorTargetNode = m_renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
				if (colorTargetNode._index != RenderGraphNode::c_index_resourceTexture)
					break;

				char buffer[256];
				sprintf_s(buffer, "%s.colorTarget%i (After)", node.name.c_str(), i);

				bool exists = false;
				const auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(colorTargetNode.resourceTexture.name.c_str(), exists);
				if (!exists || !textureInfo.m_resource)
					break;

				runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(colorTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, true);
			}

			if (node.depthTarget.resourceNodeIndex != -1)
			{
				const RenderGraphNode& depthTargetNode = m_renderGraph.nodes[node.depthTarget.resourceNodeIndex];
				if (depthTargetNode._index == RenderGraphNode::c_index_resourceTexture)
				{
					bool exists = false;
					const auto& textureInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(depthTargetNode.resourceTexture.name.c_str(), exists);
					if (exists && textureInfo.m_resource)
					{
						char buffer[256];
						sprintf_s(buffer, "%s.depthTarget (After)", node.name.c_str());
						runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(depthTargetNode.resourceTexture.dimension), buffer, textureInfo.m_resource, textureInfo.m_format, textureInfo.m_size, textureInfo.m_numMips, false, true);
					}
				}
			}
		}

		// publish shader resources as viewable resources, after the shader execution
		depIndex = -1;
		for (const ResourceDependency& dep : node.resourceDependencies)
		{
			if (dep.access == ShaderResourceAccessType::Indirect)
				continue;

			depIndex++;
			const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];

			const Shader* shader = nullptr;
			std::string shaderResourceName;
			int shaderBasePinIndex = 0;
			{
				int pinIndex = dep.pinIndex;

				if (pinIndex >= 0)
				{
					if (node.vertexShader.shader && pinIndex < node.vertexShader.shader->resources.size())
					{
						shader = node.vertexShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.vertexShader.shader)
					{
						pinIndex -= (int)node.vertexShader.shader->resources.size();
						shaderBasePinIndex += (int)node.vertexShader.shader->resources.size();
					}
				}

				if (pinIndex >= 0)
				{
					if (node.pixelShader.shader && pinIndex < node.pixelShader.shader->resources.size())
					{
						shader = node.pixelShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.pixelShader.shader)
					{
						pinIndex -= (int)node.pixelShader.shader->resources.size();
						shaderBasePinIndex += (int)node.pixelShader.shader->resources.size();
					}
				}

				if (pinIndex >= 0)
				{
					if (node.amplificationShader.shader && pinIndex < node.amplificationShader.shader->resources.size())
					{
						shader = node.amplificationShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.amplificationShader.shader)
					{
						pinIndex -= (int)node.amplificationShader.shader->resources.size();
						shaderBasePinIndex += (int)node.amplificationShader.shader->resources.size();
					}
				}

				if (pinIndex >= 0)
				{
					if (node.meshShader.shader && pinIndex < node.meshShader.shader->resources.size())
					{
						shader = node.meshShader.shader;
						shaderResourceName = shader->resources[pinIndex].name;
						pinIndex = -1;
					}
					else if (node.meshShader.shader)
					{
						pinIndex -= (int)node.meshShader.shader->resources.size();
						shaderBasePinIndex += (int)node.meshShader.shader->resources.size();
					}
				}
			}

			if (!shader)
				continue;

			switch (resourceNode._index)
			{
				case RenderGraphNode::c_index_resourceTexture:
				{
					std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceTexture.name;
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
					std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceBuffer.name;
					if (dep.access == ShaderResourceAccessType::UAV)
						label = label + " (UAV - After)";
					else
						continue;

                    unsigned int bufferViewBegin = 0;
                    unsigned int bufferViewSize = 0;
                    bool bufferViewInBytes = false;
                    if (dep.pinIndex < node.linkProperties.size())
                    {
                        const LinkProperties& linkProperties = node.linkProperties[dep.pinIndex];
                        bufferViewBegin = linkProperties.bufferViewBegin;
                        bufferViewSize = linkProperties.bufferViewSize;
                        if (linkProperties.bufferViewBeginVariable.variableIndex != -1 && !GetRuntimeVariableAllowCast(linkProperties.bufferViewBeginVariable.variableIndex, bufferViewBegin))
                            return false;
                        if (linkProperties.bufferViewSizeVariable.variableIndex != -1 && !GetRuntimeVariableAllowCast(linkProperties.bufferViewSizeVariable.variableIndex, bufferViewSize))
                            return false;
                        bufferViewInBytes = linkProperties.bufferViewUnits == MemoryUnitOfMeasurement::Bytes;
                    }

					const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
					runtimeData.HandleViewableBuffer(*this, label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count, false, true, bufferViewBegin, bufferViewSize, bufferViewInBytes);
					break;
				}
			}
		}

		return true;
	}

	return true;
}