///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_DrawCall& node)
{
	// Storage
	stringReplacementMap["/*$(MembersInternal)*/"] <<
		"// Draw call Shader " << node.name << "\n"
		"Hash_DrawCall_" << node.name << " = 0;\n"
		"ShaderModule_DrawCallVS_" << node.name << " = null;\n"
		"ShaderModule_DrawCallPS_" << node.name << " = null;\n"
		"BindGroupLayout_DrawCall_" << node.name << " = null;\n"
		"PipelineLayout_DrawCall_" << node.name << " = null;\n"
		"Pipeline_DrawCall_" << node.name << " = null;\n"
		"\n"
		;

	// Initialization
	std::ostringstream shaderCodeStringReplacementVS;
	std::ostringstream shaderCodeStringReplacementPS;
	{
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"    // (Re)create draw call shader " << node.name << "\n"
			"    {\n"
			"        const bindGroupEntries =\n"
			"        [\n"
			;

		// Vertex Shader
		int resourceIndex = -1;
		for (const ShaderResource& resource : node.vertexShader.shader->resources)
		{
			const Shader& shader = *node.vertexShader.shader;

			resourceIndex++;
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            {\n"
				"                // " << resource.name << "\n"
				"                binding: " << resource.registerIndex << ",\n"
				"                visibility: GPUShaderStage.VERTEX,\n"
				;

			switch (resource.type)
			{
				case ShaderResourceType::Texture:
				{
					RenderGraphNode depNodeBase = renderGraph.nodes[node.resourceDependencies[resourceIndex].nodeIndex];
					GigiAssert(depNodeBase._index == RenderGraphNode::c_index_resourceTexture, "Expected a texture node");
					RenderGraphNode_Resource_Texture& depNode = depNodeBase.resourceTexture;

					// Figure out what kind of access this texture has: R, W, or R/W
					const char* access = "read-only";
					if (resource.access == ShaderResourceAccessType::UAV)
					{
						access = "read-write";
						for (const WebGPU_RWTextureSplit& textureSplit : renderGraph.backendData.webGPU.RWTextureSplits)
						{
							if (textureSplit.shaderName != shader.name)
								continue;

							if (resource.name == textureSplit.pinName)
							{
								shaderCodeStringReplacementVS << "            shaderCodeVS = shaderCodeVS.replace(\"/*(" << textureSplit.pinName << "_format)*/\", Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format));\n";
								access = "write-only";
								break;
							}

							if (resource.name == textureSplit.pinNameR)
							{
								shaderCodeStringReplacementVS << "            shaderCodeVS = shaderCodeVS.replace(\"/*(" << textureSplit.pinNameR << "_format)*/\", Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format));\n";
								access = "read-only";
								break;
							}
						}
					}

					if (resource.access == ShaderResourceAccessType::UAV)
					{
						stringReplacementMap["/*$(ExecuteInit)*/"] <<
							"                storageTexture : { access: \"" << access << "\", format: Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format), viewDimension: \"" << TextureDimensionTypeToViewDimension(depNode.dimension) << "\" }\n"
							;
					}
					else
					{
						stringReplacementMap["/*$(ExecuteInit)*/"] <<
							"                texture : { viewDimension: \"" << TextureDimensionTypeToViewDimension(depNode.dimension) << "\", sampleType: Shared.GetTextureFormatInfo(this.texture_" << depNode.name << "_format).sampleType }\n"
							;
					}

					break;
				}
				case ShaderResourceType::Buffer:
				{
					const char* type = (resource.access == ShaderResourceAccessType::UAV) ? "storage" : "read-only-storage";
					stringReplacementMap["/*$(ExecuteInit)*/"] <<
						"                buffer: { type: \"" << type << "\" }\n"
						;
					break;
				}
				case ShaderResourceType::ConstantBuffer:
				{
					stringReplacementMap["/*$(ExecuteInit)*/"] <<
						"                buffer: { type: \"uniform\" }\n"
						;
					break;
				}
				default:
				{
					GigiAssert(false, "Unhandled resource type: %s (%i)", EnumToString(resource.type), (int)resource.type);
				}
			}

			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            },\n"
				;
		}
		for (const ShaderSampler& sampler : node.vertexShader.shader->samplers)
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            {\n"
				"                // " << sampler.name << "\n"
				"                binding: " << sampler.registerIndex << ",\n"
				"                visibility: GPUShaderStage.VERTEX,\n"
				"                sampler: { },\n"
				"            },\n"
				;
		}

		// Pixel Shader
		for (const ShaderResource& resource : node.pixelShader.shader->resources)
		{
			const Shader& shader = *node.pixelShader.shader;

			resourceIndex++;
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            {\n"
				"                // " << resource.name << "\n"
				"                binding: " << resource.registerIndex << ",\n"
				"                visibility: GPUShaderStage.FRAGMENT,\n"
				;

			switch (resource.type)
			{
				case ShaderResourceType::Texture:
				{
					RenderGraphNode depNodeBase = renderGraph.nodes[node.resourceDependencies[resourceIndex].nodeIndex];
					GigiAssert(depNodeBase._index == RenderGraphNode::c_index_resourceTexture, "Expected a texture node");
					RenderGraphNode_Resource_Texture& depNode = depNodeBase.resourceTexture;

					// Figure out what kind of access this texture has: R, W, or R/W
					const char* access = "read-only";
					if (resource.access == ShaderResourceAccessType::UAV)
					{
						access = "read-write";
						for (const WebGPU_RWTextureSplit& textureSplit : renderGraph.backendData.webGPU.RWTextureSplits)
						{
							if (textureSplit.shaderName != shader.name)
								continue;

							if (resource.name == textureSplit.pinName)
							{
								shaderCodeStringReplacementPS << "            shaderCodePS = shaderCodePS.replace(\"/*(" << textureSplit.pinName << "_format)*/\", Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format));\n";
								access = "write-only";
								break;
							}

							if (resource.name == textureSplit.pinNameR)
							{
								shaderCodeStringReplacementPS << "            shaderCodePS = shaderCodePS.replace(\"/*(" << textureSplit.pinNameR << "_format)*/\", Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format));\n";
								access = "read-only";
								break;
							}
						}
					}

					if (resource.access == ShaderResourceAccessType::UAV)
					{
						stringReplacementMap["/*$(ExecuteInit)*/"] <<
							"                storageTexture : { access: \"" << access << "\", format: Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format), viewDimension: \"" << TextureDimensionTypeToViewDimension(depNode.dimension) << "\" }\n"
							;
					}
					else
					{
						stringReplacementMap["/*$(ExecuteInit)*/"] <<
							"                texture : { viewDimension: \"" << TextureDimensionTypeToViewDimension(depNode.dimension) << "\", sampleType: Shared.GetTextureFormatInfo(this.texture_" << depNode.name << "_format).sampleType }\n"
							;
					}

					break;
				}
				case ShaderResourceType::Buffer:
				{
					const char* type = (resource.access == ShaderResourceAccessType::UAV) ? "storage" : "read-only-storage";
					stringReplacementMap["/*$(ExecuteInit)*/"] <<
						"                buffer: { type: \"" << type << "\" }\n"
						;
					break;
				}
				case ShaderResourceType::ConstantBuffer:
				{
					stringReplacementMap["/*$(ExecuteInit)*/"] <<
						"                buffer: { type: \"uniform\" }\n"
						;
					break;
				}
				default:
				{
					GigiAssert(false, "Unhandled resource type: %s (%i)", EnumToString(resource.type), (int)resource.type);
				}
			}

			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            },\n"
				;
		}
		for (const ShaderSampler& sampler : node.pixelShader.shader->samplers)
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            {\n"
				"                // " << sampler.name << "\n"
				"                binding: " << sampler.registerIndex << ",\n"
				"                visibility: GPUShaderStage.FRAGMENT,\n"
				"                sampler: { },\n"
				"            },\n"
				;
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"        ];\n"
			"\n"
			;

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"        const newHash = JSON.stringify(bindGroupEntries).hashCode();\n"
			"\n"
			"        if (this.ShaderModule_DrawCall_" << node.name << " === null || newHash !== this.Hash_DrawCall_" << node.name << ")\n"
			"        {\n"
			"            this.Hash_DrawCall_" << node.name << " = newHash;\n"
			"\n"
			"            let shaderCodeVS = class_" << renderGraph.name << ".ShaderCode_" << node.name << "_" << node.vertexShader.name << ";\n"
			"            let shaderCodePS = class_" << renderGraph.name << ".ShaderCode_" << node.name << "_" << node.pixelShader.name << ";\n"
			"\n"
			;

		if (!shaderCodeStringReplacementVS.str().empty())
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				shaderCodeStringReplacementVS.str() <<
				"\n"
				;
		}

		if (!shaderCodeStringReplacementPS.str().empty())
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				shaderCodeStringReplacementPS.str() <<
				"\n"
				;
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"            this.ShaderModule_DrawCallVS_" << node.name << " = device.createShaderModule({ code: shaderCodeVS, label: \"DrawCall Shader VS " << node.name << "\"});\n"
			"            this.ShaderModule_DrawCallPS_" << node.name << " = device.createShaderModule({ code: shaderCodePS, label: \"DrawCall Shader PS " << node.name << "\"});\n"
			"\n"
			"            this.BindGroupLayout_DrawCall_" << node.name << " = device.createBindGroupLayout({\n"
			"                label: \"DrawCall Bind Group Layout " << node.name << "\",\n"
			"                entries: bindGroupEntries\n"
			"            });\n"
			"\n"
			"            this.PipelineLayout_DrawCall_" << node.name << " = device.createPipelineLayout({\n"
			"                label: \"DrawCall Pipeline Layout " << node.name << "\",\n"
			"                bindGroupLayouts: [this.BindGroupLayout_DrawCall_" << node.name << "],\n"
			"            });\n"
			"\n"
			"            let renderPipelineDesc = {\n"
			"                label: \"DrawCall Pipeline " << node.name << "\",\n"
			"                layout: this.PipelineLayout_DrawCall_" << node.name << ",\n"
			"                primitive:\n"
			"                {\n"
			"                    cullMode: \"" << DrawCullModeToCullMode(node.cullMode) << "\",\n"
			"                    frontFace: \"" << (node.frontIsCounterClockwise ? "ccw" : "cw") << "\",\n"
			"                    topology: \"" << GeometryTypeToTopology(node.geometryType) << "\",\n"
			;


		if (node.indexBuffer.nodeIndex != -1)
		{
			int ibNodeIndex =
				GetNodeIsResourceNode(renderGraph.nodes[node.indexBuffer.nodeIndex])
				? node.indexBuffer.nodeIndex
				: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.indexBuffer.nodeIndex], node.indexBuffer.nodePinIndex);

			const RenderGraphNode_Resource_Buffer& indexBuffer = renderGraph.nodes[ibNodeIndex].resourceBuffer;

			// Only needed for "strip" types. like triangle strip. Not triangle list
#if 0
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                    stripIndexFormat: this.buffer_" << indexBuffer.name << "_indexFormat,\n"
				;
#endif
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"                },\n"
			"                multisample:\n"
			"                {\n"
			"                    alphaToCoverageEnabled: " << (node.alphaAsCoverage ? "true" : "false") << ",\n"
			"                },\n"
			;

		// depthStencil
		if (node.depthTarget.nodeIndex != -1)
		{
			int depthTargetNodeIndex =
				GetNodeIsResourceNode(renderGraph.nodes[node.depthTarget.nodeIndex])
				? node.depthTarget.nodeIndex
				: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.depthTarget.nodeIndex], node.depthTarget.nodePinIndex);

			const RenderGraphNode_Resource_Texture& depthTarget = renderGraph.nodes[depthTargetNodeIndex].resourceTexture;

			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                depthStencil:\n"
				"                {\n"
				"                    depthCompare: \"" << DepthTestFunctionToDepthCompare(node.depthTest) << "\",\n"
				"                    depthWriteEnabled: " << (node.depthWrite ? "true" : "false") << ",\n"
				"                    format: this.texture_" << depthTarget.name << "_format,\n"
				"                    stencilBack:\n"
				"                    {\n"
				"                        compare: \"" << DepthTestFunctionToDepthCompare(node.backFaceStencilFunc) << "\",\n"
				"                        depthFailOp: \"" << StencilOpToStencilOp(node.backFaceStencilDepthFail) << "\",\n"
				"                        failOp: \"" << StencilOpToStencilOp(node.backFaceStencilFail) << "\",\n"
				"                        passOp: \"" << StencilOpToStencilOp(node.backFaceStencilPass) << "\",\n"
				"                    },\n"
				"                    stencilFront:\n"
				"                    {\n"
				"                        compare: \"" << DepthTestFunctionToDepthCompare(node.frontFaceStencilFunc) << "\",\n"
				"                        depthFailOp: \"" << StencilOpToStencilOp(node.frontFaceStencilDepthFail) << "\",\n"
				"                        failOp: \"" << StencilOpToStencilOp(node.frontFaceStencilFail) << "\",\n"
				"                        passOp: \"" << StencilOpToStencilOp(node.frontFaceStencilPass) << "\",\n"
				"                    },\n"
				"                    stencilReadMask: 0x" << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (unsigned int)node.stencilReadMask << std::dec << ",\n"
				"                    stencilWriteMask: 0x" << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (unsigned int)node.stencilWriteMask << std::dec << ",\n"
				"                },\n"
				;
		}

		// Vertex
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                vertex:\n"
				"                {\n"
				"                    module: this.ShaderModule_DrawCallVS_" << node.name << ",\n"
				;

			// vertex and instance buffers
			if (node.vertexBuffer.nodeIndex != -1 || node.instanceBuffer.nodeIndex != -1)
			{
				stringReplacementMap["/*$(ExecuteInit)*/"] <<
					"                    buffers:\n"
					"                    [\n"
					;

				// vertex buffer
				if (node.vertexBuffer.nodeIndex != -1)
				{
					int vbNodeIndex =
						GetNodeIsResourceNode(renderGraph.nodes[node.vertexBuffer.nodeIndex])
						? node.vertexBuffer.nodeIndex
						: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.vertexBuffer.nodeIndex], node.vertexBuffer.nodePinIndex);

					const RenderGraphNode_Resource_Buffer& vertexBuffer = renderGraph.nodes[vbNodeIndex].resourceBuffer;

					stringReplacementMap["/*$(ExecuteInit)*/"] <<
						"                        {\n"
						"                            arrayStride: this.buffer_" << vertexBuffer.name << "_stride,\n"
						"                            stepMode: \"vertex\",\n"
						"                            attributes: this.buffer_" << vertexBuffer.name << "_vertexBufferAttributes,\n"
						"                        },\n"
						;
				}

				// instance buffer
				if (node.instanceBuffer.nodeIndex != -1)
				{
					int ibNodeIndex =
						GetNodeIsResourceNode(renderGraph.nodes[node.instanceBuffer.nodeIndex])
						? node.instanceBuffer.nodeIndex
						: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.instanceBuffer.nodeIndex], node.instanceBuffer.nodePinIndex);

					const RenderGraphNode_Resource_Buffer& instanceBuffer = renderGraph.nodes[ibNodeIndex].resourceBuffer;

					stringReplacementMap["/*$(ExecuteInit)*/"] <<
						"                        {\n"
						"                            arrayStride: this.buffer_" << instanceBuffer.name << "_stride,\n"
						"                            stepMode: \"instance\",\n"
						"                            attributes: this.buffer_" << instanceBuffer.name << "_vertexBufferAttributes,\n"
						"                        },\n"
						;
				}

				stringReplacementMap["/*$(ExecuteInit)*/"] <<
					"                    ],\n"
					;
			}

			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                },\n"
				;
		}

		// fragment
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                fragment:\n"
				"                {\n"
				"                    module: this.ShaderModule_DrawCallPS_" << node.name << ",\n"
				"                    targets:\n"
				"                    [\n"
				;

			// fragment.targets[]
			for (int i = 0; i < node.colorTargetSettings.size(); ++i)
			{
				int colorTargetIndex = node.colorTargets[i].resourceNodeIndex;
				if (colorTargetIndex == -1)
					break;

				const ColorTargetSettings& ctSettings = node.colorTargetSettings[i];
				const RenderGraphNode_Resource_Texture& ct = renderGraph.nodes[colorTargetIndex].resourceTexture;

				static const char* c_writeChannelFlagNames[] = { "GPUColorWrite.RED", "GPUColorWrite.GREEN", "GPUColorWrite.BLUE", "GPUColorWrite.ALPHA" };
				std::string writeChannelFlags;
				for (int channelIndex = 0; channelIndex < 4; ++channelIndex)
				{
					if (!node.colorTargetSettings[i].writeChannels[channelIndex])
						continue;

					if (!writeChannelFlags.empty())
						writeChannelFlags = writeChannelFlags + " | ";

					writeChannelFlags = writeChannelFlags + c_writeChannelFlagNames[channelIndex];
				}

				stringReplacementMap["/*$(ExecuteInit)*/"] <<
					"                        {\n"
					"                            blend:\n"
					"                            {\n"
					"                                color:\n"
					"                                {\n"
					"                                    srcFactor: \"" << DrawBlendModeToFactor(ctSettings.srcBlend) << "\",\n"
					"                                    dstFactor: \"" << DrawBlendModeToFactor(ctSettings.destBlend) << "\",\n"
					"                                },\n"
					"                                alpha:\n"
					"                                {\n"
					"                                    srcFactor: \"" << DrawBlendModeToFactor(ctSettings.srcBlendAlpha) << "\",\n"
					"                                    dstFactor: \"" << DrawBlendModeToFactor(ctSettings.destBlendAlpha) << "\",\n"
					"                                },\n"
					"                            },\n"
					"                            format: this.texture_" << ct.name << "_format,\n"
					"                            writeMask: " << writeChannelFlags << ",\n"
					"                        },\n"
					;
			}

			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                    ],\n"
				"                },\n"
				;
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"            };\n"
			"\n"
			"            if (useBlockingAPIs)\n"
			"            {\n"
			"                this.Pipeline_DrawCall_" << node.name << " = device.createRenderPipeline(renderPipelineDesc);\n"
			"            }\n"
			"            else\n"
			"            {\n"
			"                this.loadingPromises.add(\"" << node.name << "\");\n"
			"                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_" << node.name << " = handle; this.loadingPromises.delete(\"" << node.name << "\"); } );\n"
			"            }\n"
			"        }\n"
			"    }\n"
			"\n"
			;
	}

	// Execution
	{
		stringReplacementMap["/*$(Execute)*/"] <<
			"    // Run draw call shader " << node.name << "\n"
			;

		if (!node.comment.empty())
		{
			stringReplacementMap["/*$(Execute)*/"] <<
				"    // " << node.comment << "\n"
				;
		}

		WriteCondition(renderGraph, stringReplacementMap["/*$(Execute)*/"], "    ", node.condition);

		stringReplacementMap["/*$(Execute)*/"] <<
			"    {\n"
			;

		// Create the bind group
		{
			stringReplacementMap["/*$(Execute)*/"] <<
				"        const bindGroup = device.createBindGroup({\n"
				"            label: \"DrawCall Bind Group " << node.name << "\",\n"
				"            layout: this.BindGroupLayout_DrawCall_" << node.name << ",\n"
				"            entries:\n"
				"            [\n"
				;

			// Vertex Shader
			int resourceIndex = -1;
			for (const ShaderResource& resource : node.vertexShader.shader->resources)
			{
				const Shader& shader = *node.vertexShader.shader;

				resourceIndex++;

				ResourceDependency& dep = node.resourceDependencies[resourceIndex];
				if (dep.access == ShaderResourceAccessType::Indirect)
					continue;

				int UAVMipIndex = 0;
				if (dep.pinIndex < node.linkProperties.size())
					UAVMipIndex = node.linkProperties[dep.pinIndex].UAVMipIndex;

				RenderGraphNode depNodeBase = renderGraph.nodes[dep.nodeIndex];

				stringReplacementMap["/*$(Execute)*/"] <<
					"                {\n"
					"                    // " << resource.name << "\n"
					"                    binding: " << resource.registerIndex << ",\n"
					;

				switch (dep.type)
				{
				case ShaderResourceType::Texture:
				{
					GigiAssert(depNodeBase._index == RenderGraphNode::c_index_resourceTexture, "Expected a texture node");
					RenderGraphNode_Resource_Texture& depNode = depNodeBase.resourceTexture;
					stringReplacementMap["/*$(Execute)*/"] <<
						"                    resource: this.texture_" << depNode.name << ".createView({ dimension: \"" << TextureDimensionTypeToViewDimension(depNode.dimension) << "\""
						;

					if (dep.access != ShaderResourceAccessType::UAV)
						stringReplacementMap["/*$(Execute)*/"] << ", format: this.texture_" << depNode.name << "_format, usage: GPUTextureUsage.TEXTURE_BINDING";
					else
						stringReplacementMap["/*$(Execute)*/"] << ", mipLevelCount: 1, baseMipLevel: " << UAVMipIndex;

					stringReplacementMap["/*$(Execute)*/"] <<
						" })\n"
						;
					break;
				}
				case ShaderResourceType::Buffer:
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"                    resource: { buffer: this.buffer_" << GetNodeName(depNodeBase) << " }\n"
						;
					break;
				}
				case ShaderResourceType::ConstantBuffer:
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"                    resource: { buffer: this.constantBuffer_" << GetNodeName(depNodeBase) << " }\n"
						;
					break;
				}
				default:
				{
					GigiAssert(false, "Unhandled resource type: %s (%i)", EnumToString(dep.type), (int)dep.type);
				}
				}

				stringReplacementMap["/*$(Execute)*/"] <<
					"                },\n"
					;
			}
			for (const ShaderSampler& sampler : node.vertexShader.shader->samplers)
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"                {\n"
					"                    // " << sampler.name << "\n"
					"                    binding: " << sampler.registerIndex << ",\n"
					"                    resource: device.createSampler({\n"
					"                        label: \"Sampler " << sampler.name << "\","
					"                        addressModeU: \"" << SamplerAddressModeToAddressMode(sampler.addressMode) << "\","
					"                        addressModeV: \"" << SamplerAddressModeToAddressMode(sampler.addressMode) << "\","
					"                        addressModeW: \"" << SamplerAddressModeToAddressMode(sampler.addressMode) << "\","
					"                        magFilter: \"" << SamplerFilterToMagFilter(sampler.filter) << "\","
					"                        minFilter: \"" << SamplerFilterToMinFilter(sampler.filter) << "\","
					"                        mipmapFilter: \"" << SamplerFilterToMipMapFilter(sampler.filter) << "\","
					"                    }),\n"
					"                },\n"
					;
			}

			// Pixel shader
			for (const ShaderResource& resource : node.pixelShader.shader->resources)
			{
				const Shader& shader = *node.pixelShader.shader;

				resourceIndex++;

				ResourceDependency& dep = node.resourceDependencies[resourceIndex];
				if (dep.access == ShaderResourceAccessType::Indirect)
					continue;

				int UAVMipIndex = 0;
				if (dep.pinIndex < node.linkProperties.size())
					UAVMipIndex = node.linkProperties[dep.pinIndex].UAVMipIndex;

				RenderGraphNode depNodeBase = renderGraph.nodes[dep.nodeIndex];

				stringReplacementMap["/*$(Execute)*/"] <<
					"                {\n"
					"                    // " << resource.name << "\n"
					"                    binding: " << resource.registerIndex << ",\n"
					;

				switch (dep.type)
				{
				case ShaderResourceType::Texture:
				{
					GigiAssert(depNodeBase._index == RenderGraphNode::c_index_resourceTexture, "Expected a texture node");
					RenderGraphNode_Resource_Texture& depNode = depNodeBase.resourceTexture;
					stringReplacementMap["/*$(Execute)*/"] <<
						"                    resource: this.texture_" << depNode.name << ".createView({ dimension: \"" << TextureDimensionTypeToViewDimension(depNode.dimension) << "\""
						;

					if (dep.access != ShaderResourceAccessType::UAV)
						stringReplacementMap["/*$(Execute)*/"] << ", format: this.texture_" << depNode.name << "_format, usage: GPUTextureUsage.TEXTURE_BINDING";
					else
						stringReplacementMap["/*$(Execute)*/"] << ", mipLevelCount: 1, baseMipLevel: " << UAVMipIndex;

					stringReplacementMap["/*$(Execute)*/"] <<
						" })\n"
						;
					break;
				}
				case ShaderResourceType::Buffer:
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"                    resource: { buffer: this.buffer_" << GetNodeName(depNodeBase) << " }\n"
						;
					break;
				}
				case ShaderResourceType::ConstantBuffer:
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"                    resource: { buffer: this.constantBuffer_" << GetNodeName(depNodeBase) << " }\n"
						;
					break;
				}
				default:
				{
					GigiAssert(false, "Unhandled resource type: %s (%i)", EnumToString(dep.type), (int)dep.type);
				}
				}

				stringReplacementMap["/*$(Execute)*/"] <<
					"                },\n"
					;
			}

			for (const ShaderSampler& sampler : node.pixelShader.shader->samplers)
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"                {\n"
					"                    // " << sampler.name << "\n"
					"                    binding: " << sampler.registerIndex << ",\n"
					"                    resource: device.createSampler({\n"
					"                        label: \"Sampler " << sampler.name << "\",\n"
					"                        addressModeU: \"" << SamplerAddressModeToAddressMode(sampler.addressMode) << "\",\n"
					"                        addressModeV: \"" << SamplerAddressModeToAddressMode(sampler.addressMode) << "\",\n"
					"                        addressModeW: \"" << SamplerAddressModeToAddressMode(sampler.addressMode) << "\",\n"
					"                        magFilter: \"" << SamplerFilterToMagFilter(sampler.filter) << "\",\n"
					"                        minFilter: \"" << SamplerFilterToMinFilter(sampler.filter) << "\",\n"
					"                        mipmapFilter: \"" << SamplerFilterToMipMapFilter(sampler.filter) << "\",\n"
					"                    }),\n"
					"                },\n"
					;
			}

			stringReplacementMap["/*$(Execute)*/"] <<
				"            ]\n"
				"        });\n"
				"\n"
				;
		}

		// Render pass descriptor
		{
			stringReplacementMap["/*$(Execute)*/"] <<
				"        let renderPassDescriptor =\n"
				"        {\n"
				"            label: \"DrawCall Render Pass Descriptor " << node.name << "\",\n"
				;

			// colorAttachments
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"            colorAttachments:\n"
					"            [\n"
					;

				for (int i = 0; i < node.colorTargetSettings.size(); ++i)
				{
					int colorTargetIndex = node.colorTargets[i].resourceNodeIndex;
					if (colorTargetIndex == -1)
						break;

					const ColorTargetSettings& ctSettings = node.colorTargetSettings[i];
					const RenderGraphNode_Resource_Texture& ct = renderGraph.nodes[colorTargetIndex].resourceTexture;

					stringReplacementMap["/*$(Execute)*/"] <<
						"                {\n"
						"                    clearValue: [ " << ctSettings.clearColor[0] << ", " << ctSettings.clearColor[1] << ", " << ctSettings.clearColor[2] << ", " << ctSettings.clearColor[3] << " ],\n"
						"                    loadOp: \"" << (ctSettings.clear ? "clear" : "load") << "\",\n"
						"                    storeOp: \"store\",\n"
						"                    view: this.texture_" << ct.name << ".createView({ dimension: \"" << TextureDimensionTypeToViewDimension(ct.dimension) << "\", format: this.texture_" << ct.name << "_format, usage: GPUTextureUsage.RENDER_ATTACHMENT"
						", baseArrayLayer: " << ctSettings.arrayIndex << ", arrayLayerCount: 1, baseMipLevel: " << ctSettings.mipLevel << ", mipLevelCount: 1 }),\n"
						;

					stringReplacementMap["/*$(Execute)*/"] <<
						"                },\n"
						;
				}
				stringReplacementMap["/*$(Execute)*/"] <<
					"            ],\n"
					;
			}

			// depthStencilAttachment
			if (node.depthTarget.nodeIndex != -1)
			{
				int depthTargetNodeIndex =
					GetNodeIsResourceNode(renderGraph.nodes[node.depthTarget.nodeIndex])
					? node.depthTarget.nodeIndex
					: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.depthTarget.nodeIndex], node.depthTarget.nodePinIndex);

				const RenderGraphNode_Resource_Texture& dt = renderGraph.nodes[depthTargetNodeIndex].resourceTexture;

				stringReplacementMap["/*$(Execute)*/"] <<
					"            depthStencilAttachment:\n"
					"            {\n"
					"                depthClearValue: " << node.depthTargetClearValue << ",\n"
					"                depthReadOnly: " << (node.depthWrite ? "false" : "true") << ",\n"
					;

				if (node.depthWrite)
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"                depthLoadOp: \"" << (node.depthTargetClear ? "clear" : "load") << "\",\n"
						"                depthStoreOp: \"" << (node.depthWrite ? "store" : "discard") << "\",\n"
						;
				}

				stringReplacementMap["/*$(Execute)*/"] <<
					"                view: this.texture_" << dt.name << ".createView({ baseArrayLayer: " << node.depthArrayIndex << ", arrayLayerCount: 1, baseMipLevel: " << node.depthMipLevel << ", mipLevelCount: 1} ),\n"
					"            },\n"
					"        };\n"
					"\n"
					"        if (Shared.GetTextureFormatInfo(this.texture_" << dt.name << "_format).isDepthStencil)\n"
					"        {\n"
					"            renderPassDescriptor.depthStencilAttachment.stencilClearValue = 0x" << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (unsigned int)node.stencilClearValue << std::dec << ";\n"
					"            renderPassDescriptor.depthStencilAttachment.stencilLoadOp = \"" << (node.stencilClear ? "clear" : "load") << "\";\n"
					"            renderPassDescriptor.depthStencilAttachment.stencilStoreOp = \"store\";\n"
					"        }\n"
					"\n"
					;
			}
			else
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"        };\n"
					"\n"
					;
			}
		}

		// Draw!
		{
			// Calculate vertexCountPerInstance
			if (node.vertexBuffer.resourceNodeIndex != -1)
			{
				const RenderGraphNode_Resource_Buffer& vbNode = renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer;

				if (node.countPerInstance == -1)
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"        const vertexCountPerInstance = this.buffer_" << vbNode.name << "_count;\n"
						;
				}
				else
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"        const vertexCountPerInstance = Math.min(" << node.countPerInstance << ", this.buffer_" << vbNode.name << "_count);\n"
						;
				}
			}
			else
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"        const vertexCountPerInstance = " << node.countPerInstance << ";\n"
					;
			}

			if (node.instanceBuffer.resourceNodeIndex != -1)
			{
				int ibNodeIndex =
					GetNodeIsResourceNode(renderGraph.nodes[node.instanceBuffer.nodeIndex])
					? node.instanceBuffer.nodeIndex
					: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.instanceBuffer.nodeIndex], node.instanceBuffer.nodePinIndex);

				const RenderGraphNode_Resource_Buffer& instanceBuffer = renderGraph.nodes[ibNodeIndex].resourceBuffer;

				if (node.instanceCount == -1)
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"        const instanceCount = this.buffer_" << instanceBuffer.name << "_count;\n"
						;
				}
				else
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"        const instanceCount = Math.min(" << node.instanceCount << ", this.buffer_" << instanceBuffer.name << "_count);\n"
						;
				}
			}
			else
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"        const instanceCount = " << node.instanceCount << ";\n"
					;
			}

			// If we have an index buffer
			if (node.indexBuffer.resourceNodeIndex != -1)
			{
				int ibNodeIndex =
					GetNodeIsResourceNode(renderGraph.nodes[node.indexBuffer.nodeIndex])
					? node.indexBuffer.nodeIndex
					: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.indexBuffer.nodeIndex], node.indexBuffer.nodePinIndex);

				const RenderGraphNode_Resource_Buffer& indexBuffer = renderGraph.nodes[ibNodeIndex].resourceBuffer;

				if (node.countPerInstance == -1)
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"        const indexCountPerInstance = this.buffer_" << indexBuffer.name << "_count;\n"
						;
				}
				else
				{
					stringReplacementMap["/*$(Execute)*/"] <<
						"        const indexCountPerInstance = Math.min(" << node.countPerInstance << ", this.buffer_" << indexBuffer.name << "_count);\n"
						;
				}
			}

			stringReplacementMap["/*$(Execute)*/"] <<
				"\n"
				"        if (this.Pipeline_DrawCall_" << node.name << " !== null)\n"
				"        {\n"
				"            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);\n"
				"                passEncoder.setPipeline(this.Pipeline_DrawCall_" << node.name << ");\n"
				"                passEncoder.setBindGroup(0, bindGroup);\n"
				"                passEncoder.setStencilReference(" << (unsigned int)node.stencilRef << ");\n"
				;

			int vertexBufferIndex = 0;

			if (node.vertexBuffer.resourceNodeIndex != -1)
			{
				const RenderGraphNode_Resource_Buffer& vbNode = renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer;
				stringReplacementMap["/*$(Execute)*/"] <<
					"                passEncoder.setVertexBuffer(" << vertexBufferIndex << ", this.buffer_" << vbNode.name << ");\n"
					;
				vertexBufferIndex++;
			}

			if (node.instanceBuffer.resourceNodeIndex != -1)
			{
				const RenderGraphNode_Resource_Buffer& ibNode = renderGraph.nodes[node.instanceBuffer.resourceNodeIndex].resourceBuffer;
				stringReplacementMap["/*$(Execute)*/"] <<
					"                passEncoder.setVertexBuffer(" << vertexBufferIndex << ", this.buffer_" << ibNode.name << ");\n"
					;
				vertexBufferIndex++;
			}

			if (node.indexBuffer.resourceNodeIndex != -1)
			{
				int ibNodeIndex =
					GetNodeIsResourceNode(renderGraph.nodes[node.indexBuffer.nodeIndex])
					? node.indexBuffer.nodeIndex
					: GetResourceNodeForPin(renderGraph, renderGraph.nodes[node.indexBuffer.nodeIndex], node.indexBuffer.nodePinIndex);

				const RenderGraphNode_Resource_Buffer& indexBuffer = renderGraph.nodes[ibNodeIndex].resourceBuffer;

				stringReplacementMap["/*$(Execute)*/"] <<
					"                passEncoder.setIndexBuffer(this.buffer_" << indexBuffer.name << ", this.buffer_" << indexBuffer.name << "_indexFormat);\n"
					"                passEncoder.drawIndexed(indexCountPerInstance, instanceCount);\n"
					;
			}
			else
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"                passEncoder.draw(vertexCountPerInstance, instanceCount);\n"
					;
			}

			stringReplacementMap["/*$(Execute)*/"] <<
				"            passEncoder.end();\n"
				"        }\n"
				"    }\n"
				"\n"
				;
		}
	}
}

// TODO: the draw call hash needs to contain more than bind group entries. also color / depth target formats! (anyhting else can change at runtime?)
