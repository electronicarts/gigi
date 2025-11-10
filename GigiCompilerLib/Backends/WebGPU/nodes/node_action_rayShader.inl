///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// We emulate ray gen shaders in compute shaders, so this code largely matches compute shader code.

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_RayShader& node)
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
        "// RayGen Shader " << node.name << "\n"
        "Hash_RayGen_" << node.name << " = 0;\n"
        "ShaderModule_RayGen_" << node.name << " = null;\n"
        "BindGroupLayout_RayGen_" << node.name << " = null;\n"
        "PipelineLayout_RayGen_" << node.name << " = null;\n"
        "Pipeline_RayGen_" << node.name << " = null;\n"
        "\n"
        ;

	// Initialization
	std::ostringstream shaderCodeStringReplacement;
	{
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"    // (Re)create raygen shader " << node.name << "\n"
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
				"            shaderCode = \"fn SCENE_IS_AABBS() -> bool { return false; }\\n\" + shaderCode;\n"
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
			"        if (this.ShaderModule_RayGen_" << node.name << " === null || newHash !== this.Hash_RayGen_" << node.name << ")\n"
			"        {\n"
			"            this.Hash_RayGen_" << node.name << " = newHash;\n"
			"\n"
			<< shaderCodeStringReplacement.str() << (shaderCodeStringReplacement.str().empty() ? "" : "\n") <<
			"            this.ShaderModule_RayGen_" << node.name << " = device.createShaderModule({ code: shaderCode, label: \"RayGen Shader " << node.name << "\"});\n"
			"            this.BindGroupLayout_RayGen_" << node.name << " = device.createBindGroupLayout({\n"
			"                label: \"RayGen Bind Group Layout " << node.name << "\",\n"
			"                entries: bindGroupEntries\n"
			"            });\n"
			"\n"
			"            this.PipelineLayout_RayGen_" << node.name << " = device.createPipelineLayout({\n"
			"                label: \"RayGen Pipeline Layout " << node.name << "\",\n"
			"                bindGroupLayouts: [this.BindGroupLayout_RayGen_" << node.name << "],\n"
			"            });\n"
			"\n"
			"            if (useBlockingAPIs)\n"
			"            {\n"
			"                this.Pipeline_RayGen_" << node.name << " = device.createComputePipeline({\n"
			"                    label: \"RayGen Pipeline " << node.name << "\",\n"
			"                    layout: this.PipelineLayout_RayGen_" << node.name << ",\n"
			"                    compute: {\n"
			"                        module: this.ShaderModule_RayGen_" << node.name << ",\n"
			"                        entryPoint: \"" << node.shader.shader->entryPoint << "\",\n"
			"                    }\n"
			"                });\n"
			"            }\n"
			"            else\n"
			"            {\n"
			"                this.loadingPromises.add(\"" << node.name << "\");\n"
			"\n"
			"                device.createComputePipelineAsync({\n"
			"                    label: \"RayGen Pipeline " << node.name << "\",\n"
			"                    layout: this.PipelineLayout_RayGen_" << node.name << ",\n"
			"                    compute: {\n"
			"                        module: this.ShaderModule_RayGen_" << node.name << ",\n"
			"                        entryPoint: \"" << node.shader.shader->entryPoint << "\",\n"
			"                    }\n"
			"                }).then( handle => { this.Pipeline_RayGen_" << node.name << " = handle; this.loadingPromises.delete(\"" << node.name << "\"); } );\n"
			"            }\n"
			"        }\n"
			"    }\n"
			"\n"
			;
	}

	// Execution
	{
		stringReplacementMap["/*$(Execute)*/"] <<
			"    // Run raygen shader " << node.name << "\n"
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
				"            label: \"RayGen Bind Group " << node.name << "\",\n"
				"            layout: this.BindGroupLayout_RayGen_" << node.name << ",\n"
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
			stringReplacementMap["/*$(Execute)*/"] <<
				"            ]\n"
				"        });\n"
				"\n"
				;
		}

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
			"        const dispatchSizePreDiv = [\n"
			"            Math.floor((parseInt(baseDispatchSize[0]) + " << node.dispatchSize.preAdd[0] << ") * " << node.dispatchSize.multiply[0] << ") / " <<
			node.dispatchSize.divide[0] << " + " << node.dispatchSize.postAdd[0] << ",\n"
			"            Math.floor((parseInt(baseDispatchSize[1]) + " << node.dispatchSize.preAdd[1] << ") * " << node.dispatchSize.multiply[1] << ") / " <<
			node.dispatchSize.divide[1] << " + " << node.dispatchSize.postAdd[1] << ",\n"
			"            Math.floor((parseInt(baseDispatchSize[2]) + " << node.dispatchSize.preAdd[2] << ") * " << node.dispatchSize.multiply[2] << ") / " <<
			node.dispatchSize.divide[2] << " + " << node.dispatchSize.postAdd[2] << "\n"
			"        ];\n"
			"\n"
			;

		stringReplacementMap["/*$(Execute)*/"] <<
			"        const dispatchSize = [\n"
			"            Math.floor((parseInt(dispatchSizePreDiv[0]) + " << node.shader.shader->NumThreads[0] << " - 1) / " << node.shader.shader->NumThreads[0] << "),\n"
			"            Math.floor((parseInt(dispatchSizePreDiv[1]) + " << node.shader.shader->NumThreads[1] << " - 1) / " << node.shader.shader->NumThreads[1] << "),\n"
			"            Math.floor((parseInt(dispatchSizePreDiv[2]) + " << node.shader.shader->NumThreads[2] << " - 1) / " << node.shader.shader->NumThreads[2] << ")\n"
			"        ];\n"
			"\n"
			;

		// Update the dispatch size in the constant buffer
		{
			// Find the constant buffer
			int constantBufferResourceIndex = -1;
			for (int resourceIndex = 0; resourceIndex < (int)node.shader.shader->resources.size(); ++resourceIndex)
			{
				if (node.shader.shader->resources[resourceIndex].access == ShaderResourceAccessType::CBV)
					constantBufferResourceIndex = resourceIndex;
			}

			GigiAssert(constantBufferResourceIndex != -1, "Could not find constant buffer for ray gen shader \"%s\" for node \"%s\"", node.shader.shader->name.c_str(), node.name.c_str());
			const ShaderResource& shaderResourceConstantBuffer = node.shader.shader->resources[constantBufferResourceIndex];
			const Struct& constantBufferStruct = renderGraph.structs[shaderResourceConstantBuffer.constantBufferStructIndex];

			std::string specialVariablesBuffer3 = "0";
			if (RTSceneBufferNodeIndex >= 0)
			{
				GigiAssert(renderGraph.nodes[RTSceneBufferNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "RTScene is expected to be a buffer");
				const RenderGraphNode_Resource_Buffer& RTSceneBufferNode = renderGraph.nodes[RTSceneBufferNodeIndex].resourceBuffer;
				specialVariablesBuffer3 = std::string("(this.buffer_") + RTSceneBufferNode.name + std::string("_isAABBs ? 1 : 0)");
			}

			stringReplacementMap["/*$(Execute)*/"] <<
				"        // Update the dispatch size in the constant buffer\n"
				"        {\n"
				"            let specialVariablesBuffer = new Uint32Array(3);\n"
				"            specialVariablesBuffer[0] = dispatchSizePreDiv[0];\n"
				"            specialVariablesBuffer[1] = dispatchSizePreDiv[1];\n"
				"            specialVariablesBuffer[2] = dispatchSizePreDiv[2];\n"
				"            device.queue.writeBuffer("
				"this.constantBuffer_" << shaderResourceConstantBuffer.name << ", "
				"this.constructor.StructOffsets_" << constantBufferStruct.name << "._dispatchSize_" << node.shader.shader->name << "_0, "
				"specialVariablesBuffer);\n"
				"        }\n"
				"\n"
				;
		}

		// Do compute pass
		stringReplacementMap["/*$(Execute)*/"] <<
			"        if (this.Pipeline_RayGen_" << node.name << " !== null)\n"
			"        {\n"
			"            const computePass = encoder.beginComputePass();\n"
			"                computePass.setPipeline(this.Pipeline_RayGen_" << node.name << ");\n"
			"                computePass.setBindGroup(0, bindGroup);\n"
			;

		stringReplacementMap["/*$(Execute)*/"] <<
			"                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);\n"
			;

		stringReplacementMap["/*$(Execute)*/"] <<
			"            computePass.end();\n"
			"        }\n"
			"    }\n"
			"\n"
			;
	}
}
