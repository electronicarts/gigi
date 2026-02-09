///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static std::string SanitizeFilenameForJS(const std::string& fileName)
{
    std::string ret = fileName;
    //StringReplaceAll(ret, "\\\\", "\\"); // This is a bit ugly, but trying to prevent double escaping
    StringReplaceAll(ret, "\\", "/");
    return ret;
}

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Resource_Texture& node)
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
		stringReplacementMap[storageToken] << "// Texture " << node.name << "";
		if (!node.comment.empty())
			stringReplacementMap[storageToken] << " : " << node.comment << "\n";
		else
			stringReplacementMap[storageToken] << "\n";

		stringReplacementMap[storageToken] <<
			"texture_" << node.name << " = null;\n" <<
			"texture_" << node.name << "_size = [0, 0, 0];\n" <<
			"texture_" << node.name << "_format = \"\";\n" <<
			"texture_" << node.name << "_usageFlags = "
			;

		// Usage flags, as described at https://developer.mozilla.org/en-US/docs/Web/API/GPUTexture/usage
		const char* flagPrefix = "";
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC";
			flagPrefix = " | ";
		}
		//if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::CopySource))
		//{
		//	stringReplacementMap[storageToken] << flagPrefix << "GPUTextureUsage.COPY_SRC";
		//	flagPrefix = " | ";
		//}
		//if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::CopyDest))
		//{
		//	stringReplacementMap[storageToken] << flagPrefix << "GPUTextureUsage.COPY_DST";
		//	flagPrefix = " | ";
		//}
		if ((node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget)) || (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget)))
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUTextureUsage.RENDER_ATTACHMENT";
			flagPrefix = " | ";
		}
		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV))
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUTextureUsage.STORAGE_BINDING";
			flagPrefix = " | ";
		}
		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::SRV))
		{
			stringReplacementMap[storageToken] << flagPrefix << "GPUTextureUsage.TEXTURE_BINDING";
			flagPrefix = " | ";
		}

		stringReplacementMap[storageToken] <<
			";\n\n"
			;
	}

	// Imported Resources must be provided
	if (node.visibility == ResourceVisibility::Imported)
	{
		// Enforce in Execute() and Init()
		stringReplacementMap["/*$(ValidateImports)*/"] <<
			"    // Validate texture " << node.name << "\n"
			"    if (this.texture_" << node.name << " === null)\n"
			"    {\n"
			"        Shared.LogError(\"Imported resource texture_" << node.name << " was not provided\");\n"
			"        return false;\n"
			"    }\n"
			"\n"
			;

		// Put TODO in host html.
		stringReplacementMap["/*$(TODO_NeedUserInput)*/"] <<
			"\n                    " << renderGraph.name << ".texture_" << node.name << " = null;"
			"\n                    " << renderGraph.name << ".texture_" << node.name << "_size = [0, 0, 0];"
			"\n                    " << renderGraph.name << ".texture_" << node.name << "_format = \"\";"
			;
		if (!node.comment.empty())
			stringReplacementMap["/*$(TODO_NeedUserInput)*/"] << "  // " << node.comment << "";
	}
	// Load the texture if we should
	else if (!node.loadFileName.empty())
	{
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"    // Load texture " << node.name << " from \"" << SanitizeFilenameForJS(node.loadFileName) << "\"\n"
			"    if (this.texture_" << node.name << " === null)\n"
			"    {\n"
			"        const loadedTex = await Shared.CreateTextureWithPNG(device, \"./assets/" << SanitizeFilenameForJS(node.loadFileName) << "\", this.texture_" << node.name << "_usageFlags, " << ((node.dimension == TextureDimensionType::Texture3D) ? "\"3d\"" : "\"2d\"") << ");\n"
			"        this.texture_" << node.name << " = loadedTex.texture;\n"
			"        this.texture_" << node.name << "_size = loadedTex.size;\n"
			"        this.texture_" << node.name << "_format = \"" << (node.loadFileNameAsSRGB ? "rgba8unorm-srgb" : "rgba8unorm") << "\";\n"
			"        this.texture_" << node.name << "_usageFlags = loadedTex.usageFlags;\n"
			"    }\n"
			;
	}
	// Internal and exported resources need to be created
	else
	{
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"    // Handle (re)creation of texture " << node.name << "\n"
			"    {\n"
			;

		// calculate desiredSize
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"        const baseSize = "
				;

			if (node.size.node.textureNode)
			{
				stringReplacementMap["/*$(ExecuteInit)*/"] << "this.texture_" << node.size.node.textureNode->name << "_size.slice();\n";
			}
			else if (node.size.variable.variableIndex != -1)
			{
				Variable& var = renderGraph.variables[node.size.variable.variableIndex];
				int varComponentCount = (int)DataFieldTypeComponentCount(var.type);
				switch (varComponentCount)
				{
					case 1:
					{
						stringReplacementMap["/*$(ExecuteInit)*/"] << "[ this.variable_" << var.name << ", 1, 1];\n";
						break;
					}
					case 2:
					{
						stringReplacementMap["/*$(ExecuteInit)*/"] << "[ this.variable_" << var.name << "[0], this.variable_" << var.name << "[1], 1];\n";
						break;
					}
					case 3:
					{
						stringReplacementMap["/*$(ExecuteInit)*/"] << "[ this.variable_" << var.name << "[0], this.variable_" << var.name << "[1], this.variable_" << var.name << "[2]];\n";
						break;
					}
					default:
					{
                        GigiAssert(false, "Inappropriate variable type given for texture size.");
					}
				}
			}
			else
			{
				stringReplacementMap["/*$(ExecuteInit)*/"] << "[ 1, 1, 1 ];\n";
			}

			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"        const desiredSize = [\n" <<
				"            Math.floor(((parseInt(baseSize[0]) + " << node.size.preAdd[0] << ") * " << node.size.multiply[0] << ") / " << node.size.divide[0] << ") + " << node.size.postAdd[0] << ",\n"
				"            Math.floor(((parseInt(baseSize[1]) + " << node.size.preAdd[1] << ") * " << node.size.multiply[1] << ") / " << node.size.divide[1] << ") + " << node.size.postAdd[1] << ",\n"
				"            Math.floor(((parseInt(baseSize[2]) + " << node.size.preAdd[2] << ") * " << node.size.multiply[2] << ") / " << node.size.divide[2] << ") + " << node.size.postAdd[2] << "\n"
				"        ];\n"
				;
		}

		// Get desiredFormat
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"        const desiredFormat = "
				;

			if (node.format.node.textureNode)
				stringReplacementMap["/*$(ExecuteInit)*/"] << "this.texture_" << node.format.node.textureNode->name << "_format;\n";
			else
				stringReplacementMap["/*$(ExecuteInit)*/"] << "\"" << TextureFormatToGPUTextureFormat(node.format.format) << "\";\n";
		}

		// (Re)Create the texture as needed
		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"        if (this.texture_" << node.name << " !== null && (this.texture_" << node.name << "_format != desiredFormat || this.texture_" << node.name << "_size[0] != desiredSize[0] || this.texture_" << node.name << "_size[1] != desiredSize[1] || this.texture_" << node.name << "_size[2] != desiredSize[2]))\n"
			"        {\n"
			"            this.texture_" << node.name << ".destroy();\n"
			"            this.texture_" << node.name << " = null;\n"
			"        }\n"
			"\n"
			"        if (this.texture_" << node.name << " === null)\n"
			"        {\n"
			"            this.texture_" << node.name << "_size = desiredSize.slice();\n"
			"            this.texture_" << node.name << "_format = desiredFormat;\n"
			"\n"
			"            let viewFormats = [];\n"
			"            if (Shared.IsSRGBFormat(this.texture_" << node.name << "_format))\n"
			"                viewFormats.push(this.texture_" << node.name << "_format);\n"
			"\n"
			;

		if (node.numMips == 0)
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"            let numMips = 1;\n"
				"            {\n"
				"                let maxSize = Math.max(this.texture_" << node.name << "_size[0], this.texture_" << node.name << "_size[1]);\n"
				"                while (maxSize > 1)\n"
				"                {\n"
				"                    maxSize = Math.floor(maxSize / 2);\n"
				"                    numMips = numMips + 1;\n"
				"                }\n"
				"            }\n"
				;
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"            this.texture_" << node.name << " = device.createTexture({\n"
			"                label: \"texture " << renderGraph.name << "." << node.name << "\",\n"
			"                size: this.texture_" << node.name << "_size,\n"
			;

		if (node.numMips == 0)
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                mipLevelCount: numMips,\n"
				;
		}
		else if (node.numMips != 1)
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                mipLevelCount: " << node.numMips << ",\n"
				;
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"                format: Shared.GetNonSRGBFormat(this.texture_" << node.name << "_format),\n"
			"                usage: this.texture_" << node.name << "_usageFlags,\n"
			"                viewFormats: viewFormats,\n"
			;

		if (node.dimension == TextureDimensionType::Texture3D)
		{
			stringReplacementMap["/*$(ExecuteInit)*/"] <<
				"                dimension: \"3d\",\n"
				;
		}

		stringReplacementMap["/*$(ExecuteInit)*/"] <<
			"            });\n"
			"        }\n"
			"    }\n"
			"\n"
			;
	}
}
