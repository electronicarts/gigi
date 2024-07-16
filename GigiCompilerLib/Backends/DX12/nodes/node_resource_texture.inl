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

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget))
        {
            stringReplacementMap["/*$(ExecuteBegin)*/"] <<
                "\n"
                "\n        if (context->" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_rtv == -1)"
                "\n        {"
                "\n            Context::LogFn(LogLevel::Error, \"" << renderGraph.name << ": Imported texture \\\"" << node.name << "\\\" needs an RTV. Use CreateManagedRTV to create one.\\n\");"
                "\n            return;"
                "\n        }"
                ;
        }

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
        {
            stringReplacementMap["/*$(ExecuteBegin)*/"] <<
                "\n"
                "\n        if (context->" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_dsv == -1)"
                "\n        {"
                "\n            Context::LogFn(LogLevel::Error, \"" << renderGraph.name << ": Imported texture \\\"" << node.name << "\\\" needs a DSV. Use CreateManagedDSV to create one.\\n\");"
                "\n            return;"
                "\n        }"
                ;
        }
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
            "\n" << indent << "DXGI_FORMAT texture_" << node.name << "_format = DXGI_FORMAT_UNKNOWN;"
            "\n" << indent << "static const D3D12_RESOURCE_FLAGS texture_" << node.name << "_flags = " << resourceFlags << ";"
            ;

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget))
            stringReplacementMap[location] << "\n" << indent << "int texture_" << node.name << "_rtv = -1;";

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
            stringReplacementMap[location] << "\n" << indent << "int texture_" << node.name << "_dsv = -1;";

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

            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n"
                "\n                // Create the texture"
                "\n                dirty = true;"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[0] = size[0];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[1] = size[1];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[2] = size[2];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << " = DX12Utils::CreateTexture(device, size, " << TextureFormatToDXGIFormat(node.format.format) << ", " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_flags"
                ", D3D12_RESOURCE_STATE_COPY_DEST, DX12Utils::ResourceType::" << EnumToString(node.dimension) << ", (c_debugNames ? L\"" << (node.name) << "\" : nullptr), Context::LogFn);"
                "\n"
                ;

            // 3d textures do a single copy because it's a single sub resource.
            // 2d array textures do a copy for each slice
            // Cube maps are just 2d array textures with 6 slices.

            // 3D texture loading
            if (node.dimension == TextureDimensionType::Texture3D)
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n                // Create an upload buffer"
                    "\n                int unalignedPitch = loadedTextureSlices[0].width * DX12Utils::Get_DXGI_FORMAT_Info(" << TextureFormatToDXGIFormat(node.format.format) << ", Context::LogFn).bytesPerPixel;"
                    "\n                int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);"
                    "\n                DX12Utils::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * loadedTextureSlices[0].height * loadedTextureSlices.size(), LogFn);"
                    "\n"
                    "\n                unsigned char* dest = nullptr;"
                    "\n                D3D12_RANGE  readRange = { 0, 0 };"
                    "\n                HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);"
                    "\n                if(hr)"
                    "\n                    LogFn(LogLevel::Error, \"Could not map upload buffer\");"
                    "\n"
                    "\n                // Copy the pixels to the upload buffer"
                    "\n                for (int sliceIndex = 0; sliceIndex < (int)loadedTextureSlices.size(); ++sliceIndex)"
                    "\n                {"
                    "\n                    DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, " << (node.loadFileNameAsSRGB ? "true" : "false") << ", desiredType, formatInfo.sRGB, formatInfo.channelCount);"
                    "\n"
                    "\n                    for (int y = 0; y < loadedTexture.height; ++y)"
                    "\n                    {"
                    "\n                        const unsigned char* src = &loadedTextureSlice.pixels[y * unalignedPitch];"
                    "\n                        memcpy(&dest[sliceIndex * alignedPitch * loadedTexture.height + y * alignedPitch], src, unalignedPitch);"
                    "\n                    }"
                    "\n                }"
                    "\n"
                    "\n                uploadBuffer->buffer->Unmap(0, nullptr);"
                    "\n"
                    "\n                // copy the upload buffer into the texture"
                    "\n                {"
                    "\n                    D3D12_RESOURCE_DESC resourceDesc = " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "->GetDesc();"
                    "\n                    std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));"
                    "\n                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();"
                    "\n                    device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);"
                    "\n"
                    "\n                    D3D12_TEXTURE_COPY_LOCATION src = {};"
                    "\n                    src.pResource = uploadBuffer->buffer;"
                    "\n                    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;"
                    "\n                    src.PlacedFootprint = *layout;"
                    "\n"
                    "\n                    D3D12_TEXTURE_COPY_LOCATION dest = {};"
                    "\n                    dest.pResource = " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ";"
                    "\n                    dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;"
                    "\n                    dest.SubresourceIndex = 0;"
                    "\n"
                    "\n                    commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);"
                    "\n                }"
                    ;
            }
            // 2D texture loading
            else
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n                for (int sliceIndex = 0; sliceIndex < (int)loadedTextureSlices.size(); ++sliceIndex)"
                    "\n                {"
                    "\n                    DX12Utils::TextureCache::Texture& loadedTexture = loadedTextureSlices[sliceIndex];"
                    "\n"
                    "\n                    // Create an upload buffer"
                    "\n                    int unalignedPitch = loadedTexture.width * DX12Utils::Get_DXGI_FORMAT_Info(" << TextureFormatToDXGIFormat(node.format.format) << ", Context::LogFn).bytesPerPixel;"
                    "\n                    int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);"
                    "\n                    DX12Utils::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * loadedTexture.height, LogFn);"
                    "\n"
                    "\n                    // Copy the pixels to the upload buffer"
                    "\n                    {"
                    "\n                        unsigned char* dest = nullptr;"
                    "\n                        D3D12_RANGE  readRange = { 0, 0 };"
                    "\n                        HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);"
                    "\n                        if(hr)"
                    "\n                            LogFn(LogLevel::Error, \"Could not map upload buffer\");"
                    "\n"
                    "\n                        for (int y = 0; y < loadedTexture.height; ++y)"
                    "\n                        {"
                    "\n                            const unsigned char* src = &loadedTexture.pixels[y * unalignedPitch];"
                    "\n                            memcpy(&dest[y * alignedPitch], src, unalignedPitch);"
                    "\n                        }"
                    "\n                        uploadBuffer->buffer->Unmap(0, nullptr);"
                    "\n                    }"
                    "\n"
                    "\n                    // copy the upload buffer into the texture"
                    "\n                    {"
                    "\n                        D3D12_RESOURCE_DESC resourceDesc = " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "->GetDesc();"
                    "\n                        std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));"
                    "\n                        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();"
                    "\n                        device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);"
                    "\n"
                    "\n                        D3D12_TEXTURE_COPY_LOCATION src = {};"
                    "\n                        src.pResource = uploadBuffer->buffer;"
                    "\n                        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;"
                    "\n                        src.PlacedFootprint = *layout;"
                    "\n"
                    "\n                        D3D12_TEXTURE_COPY_LOCATION dest = {};"
                    "\n                        dest.pResource = " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ";"
                    "\n                        dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;"
                    "\n                        dest.SubresourceIndex = sliceIndex;"
                    "\n"
                    "\n                        commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);"
                    "\n                    }"
                    "\n                }"
                    ;
            }

            if (node.finalState != ShaderResourceAccessType::CopyDest)
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n                // Transition the texture to the proper state"
                    "\n                D3D12_RESOURCE_BARRIER barrier;"
                    "\n                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;"
                    "\n                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;"
                    "\n                barrier.Transition.pResource = " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ";"
                    "\n                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;"
                    "\n                barrier.Transition.StateAfter = " << ShaderResourceTypeToDX12ResourceState(node.finalState) << ";"
                    "\n                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;"
                    "\n                commandList->ResourceBarrier(1, &barrier);"
                    ;
            }

            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
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
                "\n               " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_format != desiredFormat)"
                "\n            {"
                "\n                dirty = true;"
                "\n                if(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ")"
                "\n                    s_delayedRelease.Add(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ");"
                "\n"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << " = DX12Utils::CreateTexture(device, desiredSize, desiredFormat, " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_flags"
                ", " << ShaderResourceTypeToDX12ResourceState(node.finalState) << ", " << textureType << ", (c_debugNames ? L\"" << (node.name) << "\" : nullptr), Context::LogFn);"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[0] = desiredSize[0];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[1] = desiredSize[1];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_size[2] = desiredSize[2];"
                "\n                " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_format = desiredFormat;"
                ;

            if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget))
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n                // Allocate a RTV handle"
                    "\n                if(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_rtv == -1 && !s_heapAllocationTrackerRTV.Allocate(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_rtv, \"" << node.name << "\"))"
                    "\n                    Context::LogFn(LogLevel::Error, \"Ran out of RTV descriptors, please increase c_numRTVDescriptors\");"
                    "\n"
                    "\n                // Create RTV"
                    "\n                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;"
                    "\n                rtvDesc.Format = " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_format;"
                    ;

                switch (node.dimension)
                {
                    case TextureDimensionType::Texture2D:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;"
                            "\n                rtvDesc.Texture2D.MipSlice = 0;"
                            "\n                rtvDesc.Texture2D.PlaneSlice = 0;"
                            ;
                        break;
                    }
                    case TextureDimensionType::Texture2DArray:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;"
                            "\n                rtvDesc.Texture2DArray.MipSlice = 0;"
                            "\n                rtvDesc.Texture2DArray.PlaneSlice = 0;"
                            "\n                rtvDesc.Texture2DArray.ArraySize = 1;"
                            "\n                rtvDesc.Texture2DArray.FirstArraySlice = 0;"
                            ;
                        break;
                    }
                    case TextureDimensionType::Texture3D:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;"
                            "\n                rtvDesc.Texture3D.MipSlice = 0;"
                            "\n                rtvDesc.Texture3D.WSize = 1;"
                            "\n                rtvDesc.Texture3D.FirstWSlice = i;"
                            ;
                        break;
                    }
                    default:
                    {
                        Assert(false, "Cannot use a \"%s\" as a color target (node \"%s\")", EnumToString(node.dimension), node.name.c_str());
                    }
                }

                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n                device->CreateRenderTargetView(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ", &rtvDesc, s_heapAllocationTrackerRTV.GetCPUHandle(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_rtv));"
                    ;

            }

            if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n"
                    "\n                // Allocate a DSV handle"
                    "\n                if(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_dsv == -1 && !s_heapAllocationTrackerDSV.Allocate(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_dsv, \"" << node.name << "\"))"
                    "\n                    Context::LogFn(LogLevel::Error, \"Ran out of DSV descriptors, please increase c_numDSVDescriptors\");"
                    "\n"
                    "\n                // Create DSV"
                    "\n                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;"
                    "\n                dsvDesc.Format = DX12Utils::DSV_Safe_DXGI_FORMAT(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_format);"
                    "\n                dsvDesc.Flags = D3D12_DSV_FLAG_NONE;"
                    ;

                switch (node.dimension)
                {
                    case TextureDimensionType::Texture2D:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;"
                            "\n                dsvDesc.Texture2D.MipSlice = 0;"
                            ;
                        break;
                    }
                    case TextureDimensionType::Texture2DArray:
                    {
                        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                            "\n                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;"
                            "\n                dsvDesc.Texture2DArray.MipSlice = 0;"
                            "\n                dsvDesc.Texture2DArray.FirstArraySlice = 0;"
                            "\n                dsvDesc.Texture2DArray.ArraySize = 1;"
                            ;

                        break;
                    }
                    default:
                    {
                        Assert(false, "Cannot use a \"%s\" as a depth target (node \"%s\")", EnumToString(node.dimension), node.name.c_str());
                    }
                }

                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n                device->CreateDepthStencilView(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << ", &dsvDesc, s_heapAllocationTrackerDSV.GetCPUHandle(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_dsv));"
                    ;
            }

            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
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

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget))
        {
            stringReplacementMap["/*$(ContextDestructor)*/"] <<
                "\n"
                "\n        if(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_rtv != -1)"
                "\n        {"
                "\n            s_heapAllocationTrackerRTV.Free(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_rtv);"
                "\n            " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_rtv = -1;"
                "\n        }"
                ;
        }

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
        {
            stringReplacementMap["/*$(ContextDestructor)*/"] <<
                "\n"
                "\n        if(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_dsv != -1)"
                "\n        {"
                "\n            s_heapAllocationTrackerRTV.Free(" << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_dsv);"
                "\n            " << GetResourceNodePathInContext(node.visibility) << "texture_" << node.name << "_dsv = -1;"
                "\n        }"
                ;
        }
    }
}