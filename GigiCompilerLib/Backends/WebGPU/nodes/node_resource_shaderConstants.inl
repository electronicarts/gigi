///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Resource_ShaderConstants& node)
{
	const char* storageToken = "/*$(MembersInternal)*/";

	const Struct& s = renderGraph.structs[node.structure.structIndex];

	// Write the storage for the buffer
	{
		stringReplacementMap[storageToken] << "// Constant buffer " << node.name << "";
		if (!node.comment.empty())
			stringReplacementMap[storageToken] << " : " << node.comment << "\n";
		else
			stringReplacementMap[storageToken] << "\n";

		stringReplacementMap[storageToken] <<
			"constantBuffer_" << node.name << " = null;\n"
			"constantBuffer_" << node.name << "_size = " << s.sizeInBytes << ";\n"
			"constantBuffer_" << node.name << "_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;\n\n"
			;
	}

	// Create the buffer if it doesn't yest exist
	stringReplacementMap["/*$(Execute)*/"] <<
		"    // Create constant buffer " << node.name << "\n"
		"    if (this.constantBuffer_" << node.name << " === null)\n"
		"    {\n"
		"        this.constantBuffer_" << node.name << " = device.createBuffer({\n"
		"            label: \"" << renderGraph.name << "." << node.name << "\",\n"
		"            size: Shared.Align(16, this.constructor.StructOffsets_" << s.name << "._size),\n"
		"            usage: this.constantBuffer_" << node.name << "_usageFlags,\n"
		"        });\n"
		"    }\n"
		"\n"
		;

	// Fill in the buffer
	stringReplacementMap["/*$(Execute)*/"] <<
		"    // Upload values to constant buffer " << node.name << "\n"
		"    {\n"
		"        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets_" << s.name << "._size));\n"
		"        const view = new DataView(bufferCPU);\n"
		;

	// Fill the CPU side buffer data
	for (const StructField& field : s.fields)
	{
		if (field.isPadding)
			continue;

		// Figure out what variable to set this field to
		std::string variableName = "";
		for (const SetCBFromVar& setFromVar : node.setFromVar)
		{
			if (setFromVar.field == field.name)
			{
				variableName = setFromVar.variable.name;
				break;
			}
		}

		// Set the field to a  variable
		{
			if (field.type == DataFieldType::Bool)
			{
				stringReplacementMap["/*$(Execute)*/"] <<
					"        view.setUint32(this.constructor.StructOffsets_" << s.name << "." << field.name << ", (this.variable_" << variableName << " === true ? 1 : 0), true);\n"
					;
			}
			else
			{
				DataFieldTypeInfoStruct fieldInfo = DataFieldTypeInfo(field.type);
				const char* writeFunctionName = nullptr;
				switch (fieldInfo.componentType2)
				{
					case DataFieldType::Int: writeFunctionName = "setInt32"; break;
					case DataFieldType::Uint: writeFunctionName = "setUint32"; break;
					case DataFieldType::Float: writeFunctionName = "setFloat32"; break;
					case DataFieldType::Bool: writeFunctionName = "setUint32"; break;
					default: Assert(false, "Unhandled data field type in constant buffer: %s (%i)", EnumToString(field.type), (int)field.type);
				}

				for (int index = 0; index < fieldInfo.componentCount; ++index)
				{
					char arrayIndexString[256];
					arrayIndexString[0] = 0;
					if (fieldInfo.componentCount > 1)
						sprintf_s(arrayIndexString, "_%i", index);

					stringReplacementMap["/*$(Execute)*/"] <<
						"        view." << writeFunctionName << "(this.constructor.StructOffsets_" << s.name << "." << field.name << arrayIndexString << ", ";

					if (!variableName.empty())
					{
						stringReplacementMap["/*$(Execute)*/"] << "this.variable_" << variableName;

						if (fieldInfo.componentCount > 1)
							stringReplacementMap["/*$(Execute)*/"] << "[" << index << "]";
					}
					else
						stringReplacementMap["/*$(Execute)*/"] << "0";

					stringReplacementMap["/*$(Execute)*/"] << ", true);\n";
				}
			}
		}
	}

	// Copy the cpu side buffer data to the gpu
	stringReplacementMap["/*$(Execute)*/"] <<
		"        device.queue.writeBuffer(this.constantBuffer_" << node.name << ", 0, bufferCPU);\n"
		"    }\n"
		"\n"
		;

	// Siggraph presentation slides

	// TODO: how do we fill the constant buffer with data? and how do we allocate a CPU side buffer that we can pack values like this into?
	// TODO: constant buffer usage flags should be such that it's ok for CPU write

	// TODO: i think we can create this in Init instead of Execute?
}
