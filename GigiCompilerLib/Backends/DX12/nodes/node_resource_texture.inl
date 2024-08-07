///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, const RenderGraphNode_Resource_Texture& node)
{
    if (!ResourceNodeIsUsed(node))
        return;

    // Shading rate textures can't be used as render target or depth target
    for (const RenderGraphNode& nodeBase : renderGraph.nodes)
    {
        if (nodeBase._index != RenderGraphNode::c_index_resourceTexture)
            continue;

        const RenderGraphNode_Resource_Texture& node = nodeBase.resourceTexture;

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::ShadingRate))
        {
            if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget))
            {
                Assert(false, "Texture \"%s\" can't be used as both a shading rate texture and a render target texture.", node.name.c_str());
            }

            if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
            {
                Assert(false, "Texture \"%s\" can't be used as both a shading rate texture and a depth target texture.", node.name.c_str());
            }
        }
    }

    // imported resources are required
    if (node.visibility == ResourceVisibility::Imported)
    {
        stringReplacementMap["/*$(ExecuteBegin)*/"] <<
            "\n"
            "\n        if (!context->" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ")"
            "\n        {"
            "\n            Context::LogFn(LogLevel::Error, \"" << renderGraph.name << ": Imported texture \\\"" << node.name << "\\\" is null.\\n\");"
            "\n            return;"
            "\n        }"
            ;
    }

    // calculate D3D12_RESOURCE_FLAGs
    std::string resourceFlags;
    {
        std::vector<std::string> individualFlags;
        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV))
            individualFlags.push_back("D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS");
        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget))
            individualFlags.push_back("D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET");
        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
            individualFlags.push_back("D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL");
        if (individualFlags.empty())
            individualFlags.push_back("D3D12_RESOURCE_FLAG_NONE");
        std::ostringstream flagsStringStream;
        flagsStringStream << " " << individualFlags[0];
        for (size_t flagIndex = 1; flagIndex < individualFlags.size(); ++flagIndex)
            flagsStringStream << " | " << individualFlags[flagIndex];
        resourceFlags = flagsStringStream.str();
    }

    const char* location = "";
    const char* indent = "";
    switch (node.visibility)
    {
        case ResourceVisibility::Imported: location = "/*$(ContextInput)*/"; indent = "            ";  break;
        case ResourceVisibility::Internal: location = "/*$(ContextInternal)*/"; indent = "        ";  break;
        case ResourceVisibility::Exported: location = "/*$(ContextOutput)*/"; indent = "            ";  break;
    }

    {
        stringReplacementMap[location] << "\n";

        if (!node.comment.empty())
        {
            stringReplacementMap[location] <<
                "\n" << indent << "// " << node.comment;
        }

        stringReplacementMap[location] <<
            "\n" << indent << "ID3D12Resource* texture_" << node.name << " = nullptr;"
            "\n" << indent << "unsigned int texture_" << node.name << "_size[3] = { 0, 0, 0 };"
            "\n" << indent << "unsigned int texture_" << node.name << "_numMips = 0;"
            "\n" << indent << "DXGI_FORMAT texture_" << node.name << "_format = DXGI_FORMAT_UNKNOWN;"
            "\n" << indent << "static const D3D12_RESOURCE_FLAGS texture_" << node.name << "_flags = " << resourceFlags << ";"
            ;

        switch (node.visibility)
        {
            case ResourceVisibility::Imported: stringReplacementMap[location] << "\n" << indent << "D3D12_RESOURCE_STATES texture_" << node.name << "_state = D3D12_RESOURCE_STATE_COMMON;"; break;
            case ResourceVisibility::Exported:
            case ResourceVisibility::Internal:
            {
                stringReplacementMap[location] << "\n" << indent << "const D3D12_RESOURCE_STATES c_texture_" << node.name << "_endingState = " << ShaderResourceTypeToDX12ResourceState(node.finalState) << ";"; break;
            }
        }
    }

    if (node.visibility != ResourceVisibility::Imported)
    {
        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
            "\n"
            "\n        // " << node.name
            ;

        if (!node.comment.empty())
        {
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n        // " << node.comment
                ;
        }

        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
            "\n        {"
        ;

        // if we are loading a texture, do that now, since it will also tell us our texture size
        if (!node.loadFileName.empty())
        {
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n            if (!" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ")"
                "\n            {"
                "\n                // Load the texture"
                "\n                std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;"
                ;

            // It's ok that these look at node.format.format directly instead of checking if it's a node or variable first, because a format must
            // be given when declaring a loaded texture.
            if (node.dimension == TextureDimensionType::Texture2D)
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n                DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(" << TextureFormatToDXGIFormat(node.format.format) << ", Context::LogFn);"
                    "\n                DX12Utils::TextureCache::Type desiredType = DX12Utils::TextureCache::Type::U8;"
                    "\n                if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)"
                    "\n                    desiredType = DX12Utils::TextureCache::Type::U8;"
                    "\n                else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)"
                    "\n                    desiredType = DX12Utils::TextureCache::Type::F32;"
                    "\n                else"
                    "\n                    Context::LogFn(LogLevel::Error, \"Unhandled channel type for image: " << node.loadFileName << "\");"
                    "\n"
                    "\n                char loadedTextureFileName[1024];"
                    "\n                sprintf_s(loadedTextureFileName, \"%lsassets/" << node.loadFileName << "\", s_techniqueLocation.c_str());"
                    "\n"
                    "\n                loadedTextureSlices.push_back(DX12Utils::TextureCache::GetAs(loadedTextureFileName, " << (node.loadFileNameAsSRGB ? "true" : "false") << ", desiredType, formatInfo.sRGB, formatInfo.channelCount));"
                    "\n                DX12Utils::TextureCache::Texture& loadedTexture = loadedTextureSlices[0];"
                    "\n                if(!loadedTexture.Valid())"
                    "\n                    Context::LogFn(LogLevel::Error, \"Could not load image: " << node.loadFileName << "\");"
                    "\n"
                    "\n                unsigned int size[3] = { (unsigned int)loadedTexture.width, (unsigned int)loadedTexture.height, 1 };"
                    ;
            }
            else
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n                DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(" << TextureFormatToDXGIFormat(node.format.format) << ", Context::LogFn);"
                    "\n                DX12Utils::TextureCache::Type desiredType = DX12Utils::TextureCache::Type::U8;"
                    "\n                if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)"
                    "\n                    desiredType = DX12Utils::TextureCache::Type::U8;"
                    "\n                else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)"
                    "\n                    desiredType = DX12Utils::TextureCache::Type::F32;"
                    "\n                else"
                    "\n                    Context::LogFn(LogLevel::Error, \"Unhandled channel type\");"
                    "\n"
                    "\n                int textureIndex = -1;"
                    "\n                while(1)"
                    "\n                {"
                    "\n                    textureIndex++;"
                    "\n                    char indexedFileName[1024];"
                    ;

                bool useCubeMapNames = (node.loadFileName.find("%s") != std::string::npos);

                if (useCubeMapNames)
                {
                    stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                        "\n"
                        "\n                    static const char* c_cubeMapNames[] ="
                        "\n                    {"
                        "\n                        \"Right\","
                        "\n                        \"Left\","
                        "\n                        \"Up\","
                        "\n                        \"Down\","
                        "\n                        \"Front\","
                        "\n                        \"Back\""
                        "\n                    };"
                        "\n"
                        "\n                    sprintf_s(indexedFileName, \"%lsassets/" << node.loadFileName << "\", s_techniqueLocation.c_str(), c_cubeMapNames[textureIndex]);"
                        ;
                }
                else
                {
                    stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                        "\n                    sprintf_s(indexedFileName, \"%lsassets/" << node.loadFileName << "\", s_techniqueLocation.c_str(), textureIndex);"
                        ;
                }

                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n                    DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, " << (node.loadFileNameAsSRGB ? "true" : "false") << ", desiredType, formatInfo.sRGB, formatInfo.channelCount);"
                    "\n"
                    "\n                    if(!loadedTextureSlice.Valid())"
                    "\n                    {"
                    "\n                        if (textureIndex == 0)"
                    "\n                            Context::LogFn(LogLevel::Error, \"Could not load image: %s\", indexedFileName);"
                    "\n                        break;"
                    "\n                    }"
                    "\n"
                    "\n                    if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))"
                    "\n                        Context::LogFn(LogLevel::Error, \"%s does not match dimensions of the first texture loaded!\", indexedFileName);"
                    "\n"
                    "\n                    loadedTextureSlices.push_back(loadedTextureSlice);"
                    "\n                }"
                    "\n"
                    "\n                unsigned int size[3] = { (unsigned int)loadedTextureSlices[0].width, (unsigned int)loadedTextureSlices[0].height, (unsigned int)loadedTextureSlices.size() };"
                    ;
            }

            // Get Desired Mip Count
            if (node.numMips == 0)
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n                unsigned int desiredNumMips = 1;"
                    "\n                {"
                    "\n                    int maxSize = max(size[0], size[1]);"
                    "\n                    while (maxSize > 1)"
                    "\n                    {"
                    "\n                        maxSize /= 2;"
                    "\n                        desiredNumMips++;"
                    "\n                    }"
                    "\n                }"
                    ;
            }
            else
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n                static const unsigned int desiredNumMips = " << node.numMips << ";"
                    ;
            }

            // Create the texture
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n"
                "\n                // Create the texture"
                "\n                dirty = true;"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[0] = size[0];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[1] = size[1];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[2] = size[2];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_numMips = desiredNumMips;"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_format = " << TextureFormatToDXGIFormat(node.format.format) << ";"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << " = DX12Utils::CreateTexture(device, size, desiredNumMips, " << TextureFormatToDXGIFormat(node.format.format) << ", " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_flags"
                ", D3D12_RESOURCE_STATE_COPY_DEST, DX12Utils::ResourceType::" << EnumToString(node.dimension) << ", (c_debugNames ? L\"" << (node.name) << "\" : nullptr), Context::LogFn);"
                "\n"
                ;

            // Upload the texture
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n"
                "\n                std::vector<unsigned char> pixels;"
                "\n                for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)"
                "\n                    pixels.insert(pixels.end(), texture.pixels.begin(), texture.pixels.end());"
                "\n"
                "\n                DX12Utils::UploadTextureToGPUAndMakeMips(device, commandList, s_ubTracker, " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ", pixels, size, desiredNumMips, D3D12_RESOURCE_STATE_COPY_DEST, " << ShaderResourceTypeToDX12ResourceState(node.finalState) << ", LogFn);"
                "\n            }"
                "\n        }"
                ;
        }
        else
        {
            // Get desired size
            if (node.size.node.textureNode)
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n            unsigned int baseSize[3] = {" <<
                    "\n                " << GetResourceNodePathInContext(node.size.node.textureNode->visibility) <<
                    "texture_" << node.size.node.textureNode->name.c_str() << "_size[0]," <<
                    "\n                " << GetResourceNodePathInContext(node.size.node.textureNode->visibility) <<
                    "texture_" << node.size.node.textureNode->name.c_str() << "_size[1]," <<
                    "\n                " << GetResourceNodePathInContext(node.size.node.textureNode->visibility) <<
                    "texture_" << node.size.node.textureNode->name.c_str() << "_size[2]" <<
                    "\n            };"
                ;
            }
            else if (node.size.variable.variableIndex != -1)
            {
                Variable& var = renderGraph.variables[node.size.variable.variableIndex];
                switch (DataFieldTypeComponentCount(var.type))
                {
                    case 1:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n"
                            "\n            unsigned int baseSize[3] = { (unsigned int)" << VariableToStringInsideContext(var, renderGraph) << ", 1, 1 };";
                        break;
                    }
                    case 2:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n"
                            "\n            unsigned int baseSize[3] = { (unsigned int)" << VariableToStringInsideContext(var, renderGraph) << "[0], (unsigned int)" << VariableToStringInsideContext(var, renderGraph) << "[1], 1 };";
                        break;
                    }
                    case 3:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n"
                            "\n            unsigned int baseSize[3] = { (unsigned int)" << VariableToStringInsideContext(var, renderGraph) << "[0], (unsigned int)" << VariableToStringInsideContext(var, renderGraph) << "[1], (unsigned int)" << VariableToStringInsideContext(var, renderGraph) << "[2] };";
                        break;
                    }
                    default:
                    {
                        Assert(false, "Inappropriate variable type given for texture size.");
                    }
                }
            }
            else
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n            unsigned int baseSize[3] = { 1, 1, 1 };"
                ;
            }

            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n"
                "\n            unsigned int desiredSize[3] = {" << 
                "\n                " << "((baseSize[0] + " << node.size.preAdd[0] << ") * " << node.size.multiply[0] << ") / " << 
                node.size.divide[0] << " + " << node.size.postAdd[0] << ","
                "\n                ((baseSize[1] + " << node.size.preAdd[1] << ") * " << node.size.multiply[1] << ") / " <<
                node.size.divide[1] << " + " << node.size.postAdd[1] << ","
                "\n                ((baseSize[2] + " << node.size.preAdd[2] << ") * " << node.size.multiply[2] << ") / " <<
                node.size.divide[2] << " + " << node.size.postAdd[2] <<
                "\n            };"
            ;

            // Get Desired Mip Count
            if (node.numMips == 0)
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n            unsigned int desiredNumMips = 1;"
                    "\n            {"
                    "\n                int maxSize = max(desiredSize[0], desiredSize[1]);"
                    "\n                while (maxSize > 1)"
                    "\n                {"
                    "\n                    maxSize /= 2;"
                    "\n                    desiredNumMips++;"
                    "\n                }"
                    "\n            }"
                    ;
            }
            else
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n            static const unsigned int desiredNumMips = " << node.numMips << ";"
                    ;
            }

            // Get desired format
            if (node.format.variable.variableIndex != -1)
            {
                Variable& var = renderGraph.variables[node.format.variable.variableIndex];
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n            DXGI_FORMAT desiredFormat = (DXGI_FORMAT)" << VariableToStringInsideContext(var, renderGraph) << ";";
            }
            else if (node.format.node.textureNode)
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n            DXGI_FORMAT desiredFormat = " <<
                    GetResourceNodePathInContext(node.format.node.textureNode->visibility) << "texture_" <<
                    node.format.node.textureNode->name.c_str() << "_format;"
                ;
            }
            else
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n            DXGI_FORMAT desiredFormat = " <<
                    TextureFormatToDXGIFormat(node.format.format) << ";"
                ;
            }

            // Recreate the texture if it doesn't exist
            const char* textureType = "";
            switch (node.dimension)
            {
                case TextureDimensionType::Texture2D: textureType = "DX12Utils::ResourceType::Texture2D"; break;
                case TextureDimensionType::Texture2DArray: textureType = "DX12Utils::ResourceType::Texture2DArray"; break;
                case TextureDimensionType::Texture3D: textureType = "DX12Utils::ResourceType::Texture3D"; break;
                case TextureDimensionType::TextureCube: textureType = "DX12Utils::ResourceType::TextureCube"; break;
                default:
                {
                    Assert(false, "Unhandled TextureDimensionType: %s (%i)", EnumToString(node.dimension), (int)node.dimension);
                }
            }

            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n"
                "\n            if(!" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << " ||"
                "\n               " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[0] != desiredSize[0] ||"
                "\n               " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[1] != desiredSize[1] ||"
                "\n               " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[2] != desiredSize[2] ||"
                "\n               " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_numMips != desiredNumMips ||"
                "\n               " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_format != desiredFormat)"
                "\n            {"
                "\n                dirty = true;"
                "\n                if(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ")"
                "\n                    s_delayedRelease.Add(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ");"
                "\n"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << " = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_flags"
                ", " << ShaderResourceTypeToDX12ResourceState(node.finalState) << ", " << textureType << ", (c_debugNames ? L\"" << (node.name) << "\" : nullptr), Context::LogFn);"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[0] = desiredSize[0];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[1] = desiredSize[1];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[2] = desiredSize[2];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_numMips = desiredNumMips;"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_format = desiredFormat;"
                "\n            }"
                "\n        }"
                ;
        }

        // destruction
        stringReplacementMap["/*$(ContextDestructor)*/"] << "\n";
        if (!node.comment.empty())
        {
            stringReplacementMap["/*$(ContextDestructor)*/"] <<
                "\n        // " << node.comment;
        }

        stringReplacementMap["/*$(ContextDestructor)*/"] <<
            "\n        if(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ")"
            "\n        {"
            "\n            s_delayedRelease.Add(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ");"
            "\n            " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << " = nullptr;"
            "\n        }"
            ;
    }
}