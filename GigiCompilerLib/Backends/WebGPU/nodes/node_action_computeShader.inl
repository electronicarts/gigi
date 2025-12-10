///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_ComputeShader& node)
{
	// Get the node index of the scene buffer, if there is one.
	int RTSceneBufferNodeIndex = -1;
	for (size_t depIndex = 0; depIndex < node.resourceDependencies.size(); ++depIndex)
	{
		ResourceDependency& dep = node.resourceDependencies[depIndex];
		if (dep.access == ShaderResourceAccessType::Indirect)
			continue;

		const ShaderResource& resource = node.shader.shader->resources[depIndex];

		if (resource.originalAccess == ShaderResourceAccessType::RTScene)
			RTSceneBufferNodeIndex = dep.nodeIndex;
	}

	// Storage
	stringReplacementMap["/*$(MembersInternal)*/"] <<
		"// Compute Shader " << node.name << "\n"
		"Hash_Compute_" << node.name << " = 0;\n"
		"ShaderModule_Compute_" << node.name << " = null;\n"
		"BindGroupLayout_Compute_" << node.name << " = null;\n"
		"PipelineLayout_Compute_" << node.name << " = null;\n"
		"Pipeline_Compute_" << node.name << " = null;\n"
		"\n"
		;

	// Initialization
	std::ostringstream shaderCodeStringReplacement;
	{
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"    // (Re)create compute shader " << node.name << "\n"
			"    {\n"
			"        const bindGroupEntries =\n"
			"        [\n"
			;

		int resourceIndex = -1;
		for (const ShaderResource& resource : node.shader.shader->resources)
		{
			resourceIndex++;
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            {\n"
				"                // " << resource.name << "\n"
				"                binding: " << resource.registerIndex << ",\n"
				"                visibility: GPUShaderStage.COMPUTE,\n"
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
							if (textureSplit.shaderName != node.shader.name)
								continue;

							if (resource.name == textureSplit.pinName)
							{
								shaderCodeStringReplacement << "            shaderCode = shaderCode.replace(\"/*(" << textureSplit.pinName << "_format)*/\", Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format));\n";
								access = "write-only";
								break;
							}

							if (resource.name == textureSplit.pinNameR)
							{
								shaderCodeStringReplacement << "            shaderCode = shaderCode.replace(\"/*(" << textureSplit.pinNameR << "_format)*/\", Shared.GetNonSRGBFormat(this.texture_" << depNode.name << "_format));\n";
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

		for (const ShaderSampler& sampler : node.shader.shader->samplers)
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            {\n"
				"                // " << sampler.name << "\n"
				"                binding: " << sampler.registerIndex << ",\n"
				"                visibility: GPUShaderStage.COMPUTE,\n"
				"                sampler: { },\n"
				"            },\n"
				;
		}

		// Get the shader code
		std::ostringstream modifyHashString;
		if (RTSceneBufferNodeIndex != -1)
		{
			GigiAssert(renderGraph.nodes[RTSceneBufferNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "RTScene is expected to be a buffer");
			const RenderGraphNode_Resource_Buffer& RTSceneBufferNode = renderGraph.nodes[RTSceneBufferNodeIndex].resourceBuffer;

			std::string oldShaderCodeStringReplacement = shaderCodeStringReplacement.str();
			shaderCodeStringReplacement = std::ostringstream();
			shaderCodeStringReplacement <<
				"            let shaderCode = this.buffer_" << RTSceneBufferNode.name << "_isBVH\n"
				"                ? class_" << renderGraph.name << ".ShaderCode_" << node.name << "_" << node.shader.shader->name << "_BVH\n"
				"                : class_" << renderGraph.name << ".ShaderCode_" << node.name << "_" << node.shader.shader->name << "_NonBVH\n"
				"            ;\n"
				"            shaderCode = \"fn SCENE_IS_AABBS() -> bool { return \" + this.buffer_" + RTSceneBufferNode.name + "_isAABBs.toString() + \"; }\\n\" + shaderCode;\n"
				<< oldShaderCodeStringReplacement
				;

			modifyHashString << "        hashString = hashString + this.buffer_" + RTSceneBufferNode.name + "_isBVH.toString();\n";
		}
		else
		{
			std::string oldShaderCodeStringReplacement = shaderCodeStringReplacement.str();
			shaderCodeStringReplacement = std::ostringstream();
			shaderCodeStringReplacement <<
				"            let shaderCode = class_" << renderGraph.name << ".ShaderCode_" << node.name << "_" << node.shader.shader->name << ";\n"
				//"            shaderCode = \"fn SCENE_IS_AABBS() -> bool { return false; }\\n\" + shaderCode;\n"
				<< oldShaderCodeStringReplacement
				;
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"        ];\n"
			"\n"
			"        let hashString = JSON.stringify(bindGroupEntries);\n"
			<< modifyHashString.str() <<
			"        const newHash = hashString.hashCode();\n"
			"\n"
			"        if (this.ShaderModule_Compute_" << node.name << " === null || newHash !== this.Hash_Compute_" << node.name << ")\n"
			"        {\n"
			"            this.Hash_Compute_" << node.name << " = newHash;\n"
			"\n"
			<< shaderCodeStringReplacement.str() << (shaderCodeStringReplacement.str().empty() ? "" : "\n") <<
			"            this.ShaderModule_Compute_" << node.name << " = device.createShaderModule({ code: shaderCode, label: \"Compute Shader " << node.name << "\"});\n"
			"            this.BindGroupLayout_Compute_" << node.name << " = device.createBindGroupLayout({\n"
			"                label: \"Compute Bind Group Layout " << node.name << "\",\n"
			"                entries: bindGroupEntries\n"
			"            });\n"
			"\n"
			"            this.PipelineLayout_Compute_" << node.name << " = device.createPipelineLayout({\n"
			"                label: \"Compute Pipeline Layout " << node.name << "\",\n"
			"                bindGroupLayouts: [this.BindGroupLayout_Compute_" << node.name << "],\n"
			"            });\n"
			"\n"
			"            if (useBlockingAPIs)\n"
			"            {\n"
			"                this.Pipeline_Compute_" << node.name << " = device.createComputePipeline({\n"
			"                    label: \"Compute Pipeline " << node.name << "\",\n"
			"                    layout: this.PipelineLayout_Compute_" << node.name << ",\n"
			"                    compute: {\n"
			"                        module: this.ShaderModule_Compute_" << node.name << ",\n"
			"                        entryPoint: \"" << node.shader.shader->entryPoint << "\",\n"
			"                    }\n"
			"                });\n"
			"            }\n"
			"            else\n"
			"            {\n"
			"                this.loadingPromises.add(\"" << node.name << "\");\n"
			"\n"
			"                device.createComputePipelineAsync({\n"
			"                    label: \"Compute Pipeline " << node.name << "\",\n"
			"                    layout: this.PipelineLayout_Compute_" << node.name << ",\n"
			"                    compute: {\n"
			"                        module: this.ShaderModule_Compute_" << node.name << ",\n"
			"                        entryPoint: \"" << node.shader.shader->entryPoint << "\",\n"
			"                    }\n"
			"                }).then( handle => { this.Pipeline_Compute_" << node.name << " = handle; this.loadingPromises.delete(\"" << node.name << "\"); } );\n"
			"            }\n"
			"        }\n"
			"    }\n"
			"\n"
			;
	}

	// Execution
	{
		stringReplacementMap["/*$(Execute)*/"] <<
			"    // Run compute shader " << node.name << "\n"
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
				"            label: \"Compute Bind Group " << node.name << "\",\n"
				"            layout: this.BindGroupLayout_Compute_" << node.name << ",\n"
				"            entries: [\n"
				;

			for (size_t depIndex = 0; depIndex < node.resourceDependencies.size(); ++depIndex)
			{
				ResourceDependency& dep = node.resourceDependencies[depIndex];
				if (dep.access == ShaderResourceAccessType::Indirect)
					continue;

				int UAVMipIndex = 0;
				if (dep.pinIndex < node.linkProperties.size())
					UAVMipIndex = node.linkProperties[dep.pinIndex].UAVMipIndex;

				RenderGraphNode depNodeBase = renderGraph.nodes[dep.nodeIndex];

				const ShaderResource& resource = node.shader.shader->resources[depIndex];
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

			for (const ShaderSampler& sampler : node.shader.shader->samplers)
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

        // Calculate dispatch size
        int indirectBufferResourceNodeIndex = node.enableIndirect ? node.dispatchSize.indirectBuffer.nodeIndex : -1;
		if (indirectBufferResourceNodeIndex == -1)
		{
			stringReplacementMap["/*$(Execute)*/"] << "        // Calculate dispatch size\n";

			if (node.dispatchSize.node.textureNode)
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"        const baseDispatchSize = this.texture_" << node.dispatchSize.node.textureNode->name << "_size;\n"
					;
			}
			else if (node.dispatchSize.node.bufferNode)
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"        const baseDispatchSize = [this.buffer_" << node.dispatchSize.node.bufferNode->name << "_count, 1, 1];\n"
					;
			}
			else if (node.dispatchSize.variable.variableIndex != -1)
			{
				Variable& var = renderGraph.variables[node.dispatchSize.variable.variableIndex];
				switch (DataFieldTypeComponentCount(var.type))
				{
					case 1:
					{
						stringReplacementMap["/*$(Execute)*/"] <<
							"\n        const baseDispatchSize = [ this.variable_" << var.name << ", 1, 1 ];\n";
						break;
					}
					case 2:
					{
						stringReplacementMap["/*$(Execute)*/"] <<
							"\n        const baseDispatchSize = [ this.variable_" << var.name << "[0], this.variable_" << var.name << "[1], 1 ];\n";
						break;
					}
					case 3:
					{
						stringReplacementMap["/*$(Execute)*/"] <<
							"\n        const baseDispatchSize = [ this.variable_" << var.name << "[0], this.variable_" << var.name << "[1], this.variable_" << var.name << "[2] ];\n";
						break;
					}
					default:
					{
						GigiAssert(false, "Inappropriate variable type given for dispatch size.");
					}
				}
			}
			else
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"        const baseDispatchSize = [1, 1, 1];\n"
					;
			}

			stringReplacementMap["/*$(Execute)*/"] <<
				"        const dispatchSize = [\n"
				"            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + " << node.dispatchSize.preAdd[0] << ") * " << node.dispatchSize.multiply[0] << ") / " <<
				node.dispatchSize.divide[0] << ") + " << node.dispatchSize.postAdd[0] << " + " << node.shader.shader->NumThreads[0] << " - 1) / " << node.shader.shader->NumThreads[0] << "),\n"
				"            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + " << node.dispatchSize.preAdd[1] << ") * " << node.dispatchSize.multiply[1] << ") / " <<
				node.dispatchSize.divide[1] << ") + " << node.dispatchSize.postAdd[1] << " + " << node.shader.shader->NumThreads[1] << " - 1) / " << node.shader.shader->NumThreads[1] << "),\n"
				"            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + " << node.dispatchSize.preAdd[2] << ") * " << node.dispatchSize.multiply[2] << ") / " <<
				node.dispatchSize.divide[2] << ") + " << node.dispatchSize.postAdd[2] << " + " << node.shader.shader->NumThreads[2] << " - 1) / " << node.shader.shader->NumThreads[2] << ")\n"
				"        ];\n"
				"\n"
				;
		}

		// Do compute pass
		stringReplacementMap["/*$(Execute)*/"] <<
			"        if (this.Pipeline_Compute_" << node.name << " !== null)\n"
			"        {\n"
			"            const computePass = encoder.beginComputePass();\n"
			"                computePass.setPipeline(this.Pipeline_Compute_" << node.name << ");\n"
			"                computePass.setBindGroup(0, bindGroup);\n"
			;

		if (indirectBufferResourceNodeIndex != -1)
		{
			if (!GetNodeIsResourceNode(renderGraph.nodes[indirectBufferResourceNodeIndex]))
				indirectBufferResourceNodeIndex = GetResourceNodeForPin(renderGraph, indirectBufferResourceNodeIndex, node.dispatchSize.indirectBuffer.nodePinIndex);

			RenderGraphNode& bufferNodeBase = renderGraph.nodes[indirectBufferResourceNodeIndex];
			GigiAssert(bufferNodeBase._index == RenderGraphNode::c_index_resourceBuffer, "Expected a buffer node");
			RenderGraphNode_Resource_Buffer& bufferNode = bufferNodeBase.resourceBuffer;

			stringReplacementMap["/*$(Execute)*/"] <<
				"                computePass.dispatchWorkgroupsIndirect(this.buffer_" << bufferNode.name << ", ";

			if (node.dispatchSize.indirectOffsetVariable.variableIndex != -1)
				stringReplacementMap["/*$(Execute)*/"] <<  "this.variable_" << renderGraph.variables[node.dispatchSize.indirectOffsetVariable.variableIndex].name;
			else
				stringReplacementMap["/*$(Execute)*/"] << node.dispatchSize.indirectOffsetValue;

			stringReplacementMap["/*$(Execute)*/"] << " * 4 * 4); // byte offset.  *4 because sizeof(UINT) is 4.  *4 again because of 4 items per dispatch.\n";
		}
        else
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);\n"
                ;
        }

		stringReplacementMap["/*$(Execute)*/"] <<
			"            computePass.end();\n"
			"        }\n"
			"    }\n"
			"\n"
			;
	}
}
