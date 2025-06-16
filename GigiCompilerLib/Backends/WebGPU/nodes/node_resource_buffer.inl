///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Resource_Buffer& node)
{
	const char* storageToken = nullptr;
	switch (node.visibility)
	{
		case ResourceVisibility::Imported: storageToken = "/*$(MembersImported)*/"; break;
		case ResourceVisibility::Internal: storageToken = "/*$(MembersInternal)*/"; break;
		case ResourceVisibility::Exported: storageToken = "/*$(MembersExported)*/"; break;
	}

	// Write the storage for the buffer
	{
		stringReplacementMap[storageToken] << "// Buffer " << node.name << "";
		if (!node.comment.empty())
			stringReplacementMap[storageToken] << " : " << node.comment << "\n";
		else
			stringReplacementMap[storageToken] << "\n";

		stringReplacementMap[storageToken] <<
			"buffer_" << node.name << " = null;\n" <<
			"buffer_" << node.name << "_count = 0;\n" <<
			"buffer_" << node.name << "_stride = 0;\n" <<
			"buffer_" << node.name << "_usageFlags = "
			;

		// Usage flags, as described at https://developer.mozilla.org/en-US/docs/Web/API/GPUBuffer/usage
		const char* flagPrefix = "";
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE";
			flagPrefix = " | ";
		}
		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::CopySource))
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUBufferUsage.COPY_SRC";
			flagPrefix = " | ";
		}
		//if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::CopyDest))
		//{
		//	stringReplacementMap[storageToken] << flagPrefix << "GPUBufferUsage.COPY_DST";
		//	flagPrefix = " | ";
		//}
		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::IndexBuffer))
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUBufferUsage.INDEX";
			flagPrefix = " | ";
		}
		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::Indirect))
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUBufferUsage.INDIRECT";
			flagPrefix = " | ";
		}
		//if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV))
		//{
		//	stringReplacementMap[storageToken] << flagPrefix << "GPUBufferUsage.STORAGE";
		//	flagPrefix = " | ";
		//}
		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::VertexBuffer))
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUBufferUsage.VERTEX";
			flagPrefix = " | ";
		}

		//if (flagPrefix[0] == 0)
		//{
		//	stringReplacementMap[storageToken] << "GPUBufferUsage.COPY_DST";
		//}

		stringReplacementMap[storageToken] <<
			";\n"
			;

		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::VertexBuffer))
		{
			if (node.format.structureType.structIndex == -1)
			{
				stringReplacementMap[storageToken] <<
					"buffer_" << node.name << "_vertexBufferAttributes = null;\n"
					;
			}
			else
			{
				const Struct& s = renderGraph.structs[node.format.structureType.structIndex];
				stringReplacementMap[storageToken] <<
					"buffer_" << node.name << "_vertexBufferAttributes = class_" << renderGraph.name << ".StructVertexBufferAttributes_" << s.name << ";\n"
					;
			}
		}

		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::IndexBuffer))
		{
			stringReplacementMap[storageToken] <<
				"buffer_" << node.name << "_indexFormat = \"uint32\"; // Can be \"uint16\" or \"uint32\"\n"
				;
		}

		if (node.originallyAccessedAs & (1 << (unsigned int)ShaderResourceAccessType::RTScene))
		{
			stringReplacementMap[storageToken] <<
				"buffer_" << node.name << "_isAABBs = false; // For raytracing: if true, treated as AABBs for an intersection shader, else treated as triangles\n"
				"buffer_" << node.name << "_isBVH = false; // For raytracing: if true, treated as a BVH of triangles\n"
				;
		}

		stringReplacementMap[storageToken] <<
			"\n"
			;
	}

	// Imported Resources must be provided
	if (node.visibility == ResourceVisibility::Imported)
	{
		// Enforce in Execute() and Init()
		stringReplacementMap["/*$(ValidateImports)*/"] <<
			"    // Validate buffer " << node.name << "\n"
			"    if (this.buffer_" << node.name << " === null)\n"
			"    {\n"
			"        Shared.LogError(\"Imported resource buffer_" << node.name << " was not provided\");\n"
			"        return false;\n"
			"    }\n"
			"\n"
		;

		// Put TODO in host html.
		stringReplacementMap["/*$(TODO_NeedUserInput)*/"] <<
			"\n                    " << renderGraph.name << ".buffer_" << node.name << " = null;"
			"\n                    " << renderGraph.name << ".buffer_" << node.name << "_count = 0;"
			"\n                    " << renderGraph.name << ".buffer_" << node.name << "_stride = 0;"
		;
		if (!node.comment.empty())
			stringReplacementMap["/*$(TODO_NeedUserInput)*/"] << "  // " << node.comment << "";
	}

	// Internal and exported resources need to be created
	if (node.visibility == ResourceVisibility::Internal || node.visibility == ResourceVisibility::Exported)
	{
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"    // Handle (re)creation of buffer " << node.name << "\n"
			"    {\n"
			;

		// calculate desiredCount
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"        const baseCount = "
				;

			if (node.count.node.bufferNode)
			{
				stringReplacementMap["/*$(ExecuteInit)*/"] << "this.buffer_" << node.count.node.bufferNode->name << "_count;\n";
			}
			else if (node.count.variable.variableIndex != -1)
			{
				Variable& var = renderGraph.variables[node.count.variable.variableIndex];
				stringReplacementMap["/*$(ExecuteInit)*/"] << "this.variable_" << var.name << ";\n";
				Assert(DataFieldTypeComponentCount(var.type) == 1, "Inappropriate variable type given for buffer size.");
			}
			else
			{
				stringReplacementMap["/*$(ExecuteInit)*/"] << "1;\n";
			}

			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"        const desiredCount = " << "Math.floor(((parseInt(baseCount) + " << node.count.preAdd << " ) * " << node.count.multiply << ") / " << node.count.divide << ") + " << node.count.postAdd << ";\n"
				;
		}

		// calculate desiredStride
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"        const desiredStride = "
				;

			if (node.format.node.bufferNode)
				stringReplacementMap["/*$(ExecuteInit)*/"] << "this.buffer_" << node.format.node.bufferNode->name << "_stride;\n";
			else if (node.format.structureType.structIndex != -1)
				stringReplacementMap["/*$(ExecuteInit)*/"] << renderGraph.structs[node.format.structureType.structIndex].sizeInBytes << ";\n";
			else
				stringReplacementMap["/*$(ExecuteInit)*/"] << DataFieldTypeToSize(node.format.type) << ";\n";
		}

		// (Re)Create the buffer as needed
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"        if (this.buffer_" << node.name << " !== null && (this.buffer_" << node.name << "_count != desiredCount || this.buffer_" << node.name << "_stride != desiredStride))\n"
			"        {\n"
			"            this.buffer_" << node.name << ".destroy();\n"
			"            this.buffer_" << node.name << " = null;\n"
			"        }\n"
			"\n"
			"        if (this.buffer_" << node.name << " === null)\n"
			"        {\n"
			"            this.buffer_" << node.name << "_count = desiredCount;\n"
			"            this.buffer_" << node.name << "_stride = desiredStride;\n"
			"            this.buffer_" << node.name << " = device.createBuffer({\n"
			"                label: \"buffer " << renderGraph.name << "." << node.name << "\",\n"
			"                size: Shared.Align(16, this.buffer_" << node.name << "_count * this.buffer_" << node.name << "_stride),\n"
			"                usage: this.buffer_" << node.name << "_usageFlags,\n"
			"            });\n"
			"        }\n"
			"    }\n"
			"\n"
			;
	}
}
