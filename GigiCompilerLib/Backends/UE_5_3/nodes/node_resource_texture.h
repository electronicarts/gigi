///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

static void MakeTextureDesc(const RenderGraph& renderGraph, const RenderGraphNode_Resource_Texture& node, std::ostringstream& makeDesc, const char* indent)
{
    makeDesc << indent << "// Calculate size\n";

    // make size
    {
        if (node.size.node.textureNode)
        {
            makeDesc << indent << "FIntVector textureSize = Texture_" << node.size.node.textureNode->name << "->Desc.GetSize();\n";
        }
        else if (node.size.variable.variableIndex != -1)
        {
            const Variable& variable = renderGraph.variables[node.size.variable.variableIndex];
            DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(variable.type);
            switch (typeInfo.componentCount)
            {
                case 1: makeDesc << indent << "FIntVector textureSize = FIntVector(" << VariableToString(variable, renderGraph) << ", 1, 1);\n"; break;
                case 2: makeDesc << indent << "FIntVector textureSize = FIntVector(" << VariableToString(variable, renderGraph) << "[0], " << VariableToString(variable, renderGraph) << "[1], 1);\n"; break;
                case 3: makeDesc << indent << "FIntVector textureSize = FIntVector(" << VariableToString(variable, renderGraph) << "[0], " << VariableToString(variable, renderGraph) << "[1], " << VariableToString(variable, renderGraph) << "[2]);\n"; break;
                default:
                {
                    GigiAssert(false, "Inappropriate variable type given for dispatch size.");
                }
            }
        }
        else
        {
            makeDesc << indent << "FIntVector textureSize = FIntVector(1,1,1);\n";
        }

        makeDesc
            << indent << "textureSize.X = ((textureSize.X + " << node.size.preAdd[0] << ") * " << node.size.multiply[0] << ") / " <<
            node.size.divide[0] << " + " << node.size.postAdd[0] << ";\n"
            << indent << "textureSize.Y = ((textureSize.Y + " << node.size.preAdd[1] << ") * " << node.size.multiply[1] << ") / " <<
            node.size.divide[1] << " + " << node.size.postAdd[1] << ";\n"
            << indent << "textureSize.Z = ((textureSize.Z + " << node.size.preAdd[2] << ") * " << node.size.multiply[2] << ") / " <<
            node.size.divide[2] << " + " << node.size.postAdd[2] << ";\n"
            "\n"
            ;
    }

    makeDesc << indent << "// Make Desc\n";

    // TexCreate flags
    makeDesc << indent << "ETextureCreateFlags createFlags = " << MakeETextureCreateFlags(node.accessedAs, false) << ";\n";

    // Get desired format
    if (node.format.variable.variableIndex != -1)
    {
        const Variable& var = renderGraph.variables[node.format.variable.variableIndex];
        makeDesc <<
            "\n"
            << indent << "EPixelFormat textureFormat = (EPixelFormat)" << VariableToString(var, renderGraph) << ";\n";
    }
    else if (node.format.node.textureNode)
    {
        makeDesc <<
            "\n"
            << indent << "EPixelFormat textureFormat = Texture_" << node.format.node.textureNode->name << "->Desc.Format;\n"
            << indent << "if (((uint64)Texture_" << node.format.node.textureNode->name << "->Desc.Flags & (uint64)ETextureCreateFlags::SRGB) != 0)\n"
            << indent << "    createFlags |= ETextureCreateFlags::SRGB;\n"
            ;
    }
    else
    {
        bool isSRGB = false;
        makeDesc <<
            "\n"
            << indent << "EPixelFormat textureFormat = " << TextureFormatToEPixelFormat(node.format.format, isSRGB) << ";\n"
            ;

        if (isSRGB)
            makeDesc << indent << "createFlags |= ETextureCreateFlags::SRGB;\n";
    }

    switch (node.dimension)
    {
        case TextureDimensionType::Texture2D:
        {
            makeDesc <<
                "\n"
                << indent << "FRDGTextureDesc desc = FRDGTextureDesc::Create2D(\n"
                << indent << "    FIntPoint(textureSize.X, textureSize.Y),\n"
                << indent << "    textureFormat,\n"
                << indent << "    FClearValueBinding::None,\n"
                << indent << "    createFlags\n"
                << indent << ");\n"
                ;
            break;
        }
        case TextureDimensionType::Texture2DArray:
        {
            makeDesc <<
                "\n"
                << indent << "FRDGTextureDesc desc = FRDGTextureDesc::Create2DArray(\n"
                << indent << "    FIntPoint(textureSize.X, textureSize.Y),\n"
                << indent << "    textureFormat,\n"
                << indent << "    FClearValueBinding::None,\n"
                << indent << "    createFlags,\n"
                << indent << "    textureSize.Z\n"
                << indent << ");\n"
                ;
            break;
        }
        case TextureDimensionType::Texture3D:
        {
            makeDesc <<
                "\n"
                << indent << "FRDGTextureDesc desc = FRDGTextureDesc::Create2D(\n"
                << indent << "    textureSize,\n"
                << indent << "    textureFormat,\n"
                << indent << "    FClearValueBinding::None,\n"
                << indent << "    createFlags\n"
                << indent << ");\n"
                ;
            break;
        }
        case TextureDimensionType::TextureCube:
        {
            makeDesc <<
                "\n"
                << indent << "FRDGTextureDesc desc = FRDGTextureDesc::CreateCube(\n"
                << indent << "    textureSize.X,\n"
                << indent << "    textureFormat,\n"
                << indent << "    FClearValueBinding::None,\n"
                << indent << "    createFlags\n"
                << indent << ");\n"
                ;
            break;
        }
        default:
        {
            GigiAssert(false, "Unhandled texture dimensionality \"%s\" in node \"%s\"", EnumToString(node.dimension), node.name);
        }
    }
}

static bool ProcessNodeTag(const RenderGraph& renderGraph, const RenderGraphNode_Resource_Texture& node, std::string& tag, std::string& text)
{
    // Textures that are not imported, and are not transient, have persistent storage in FTechniqueState
    {
        if (tag == "/*$(FTechniqueState-NotImported-Persistent)*/")
        {
            if (node.visibility != ResourceVisibility::Imported && !node.transient && node.loadFileName.empty())
            {
                tag = "/*$(FTechniqueState)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(AddTechnique-Loaded)*/")
        {
            if (node.visibility != ResourceVisibility::Imported && !node.transient && !node.loadFileName.empty())
            {
                tag = "/*$(AddTechnique)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(AddTechnique-NotImported-Persistent)*/")
        {
            if (node.visibility != ResourceVisibility::Imported && !node.transient && node.loadFileName.empty())
            {
                std::ostringstream createTexture;

                MakeTextureDesc(renderGraph, node, createTexture, "        ");

                createTexture <<
                    "\n"
                    "        // Create Texture\n"
                    "        FRDGTextureRef texture = GraphBuilder.CreateTexture(desc, TEXT(\"" << renderGraph.name << "." << node.name << "\"));\n"
                    "        View.ViewState->TechniqueState_" << renderGraph.name << ".Texture_" << node.name << " = GraphBuilder.ConvertToExternalTexture(texture).GetReference()->GetRHI();\n"
                    ;

                StringReplaceAll(text, "/*$(Node:CreateTexture)*/", createTexture.str());

                tag = "/*$(AddTechnique)*/";
                return true;
            }
            return false;
        }
    }

    // Textures that are not imported, and transient, live as locals in AddTechnique
    // They need to be created though.
    if (tag == "/*$(AddTechnique-NotImported-Transient)*/")
    {
        if (node.visibility != ResourceVisibility::Imported && node.transient)
        {
            std::ostringstream makeDesc;
            MakeTextureDesc(renderGraph, node, makeDesc, "        ");
            StringReplaceAll(text, "/*$(Node:MakeDesc)*/", makeDesc.str());

            tag = "/*$(AddTechnique)*/";
            return true;
        }

        return false;
    }

    // Imported and exported textures get storage in FTechniqueParams to be input, or output
    {
        // TechniqueCPPNamespace-Loaded
        if (tag == "/*$(TechniqueCPPNamespace-Loaded)*/")
        {
            if (node.visibility == ResourceVisibility::Internal && !node.loadFileName.empty())
            {
                std::ostringstream outText;
                outText <<
                    "static FTextureRHIRef LoadedTexture_" << node.name << " = nullptr;\n"
                    ;
                text = outText.str();

                tag = "/*$(TechniqueCPPNamespace)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(InitializeFnCPP-Loaded)*/")
        {
            if (node.visibility == ResourceVisibility::Internal && !node.loadFileName.empty())
            {
                std::ostringstream outText;
                outText <<
                    "    LoadedTexture_" << node.name << " = LoadObject<UTexture2D>(nullptr, *params.TexturePath_" << node.name << ".ToString(), nullptr, LOAD_None, nullptr)->GetResource()->TextureRHI;\n"
                    ;
                text = outText.str();

                tag = "/*$(InitializeFnCPP)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(FInitParams-Loaded)*/")
        {
            if (node.visibility == ResourceVisibility::Internal && !node.loadFileName.empty())
            {
                std::ostringstream outText;
                outText <<
                    "        FSoftObjectPath TexturePath_" << node.name << "; // Asset reference for imported texture " << node.loadFileName << "\n"
                    ;
                text = outText.str();

                tag = "/*$(FInitParams)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(FTechniqueParamsInputs-Imported)*/")
        {
            if (node.visibility == ResourceVisibility::Imported)
            {
                tag = "/*$(FTechniqueParamsInputs)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(AddTechnique-Imported)*/")
        {
            if (node.visibility == ResourceVisibility::Imported)
            {
                tag = "/*$(AddTechnique)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(FTechniqueParamsOutputs-Exported)*/")
        {
            if (node.visibility == ResourceVisibility::Exported)
            {
                tag = "/*$(FTechniqueParamsOutputs)*/";
                return true;
            }
            return false;
        }

        if (tag == "/*$(AddTechniqueSetExports-Exported)*/")
        {
            if (node.visibility == ResourceVisibility::Exported)
            {
                tag = "/*$(AddTechniqueSetExports)*/";
                return true;
            }
            return false;
        }
    }

    return true;
}
