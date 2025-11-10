///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

static bool ProcessNodeTag(const RenderGraph& renderGraph, const RenderGraphNode_Action_DrawCall& node, std::string& tag, std::string& text)
{
    bool hasVertexBuffer = (node.vertexBuffer.resourceNodeIndex != -1);
    bool hasIndexBuffer = (node.indexBuffer.resourceNodeIndex != -1);
    bool hasInstanceBuffer = (node.instanceBuffer.resourceNodeIndex != -1);

    GigiAssert(!hasVertexBuffer || renderGraph.nodes[node.vertexBuffer.resourceNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "Error: vertex buffer is not a buffer.");
    GigiAssert(!hasIndexBuffer || renderGraph.nodes[node.indexBuffer.resourceNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "Error: index buffer is not a buffer.");
    GigiAssert(!hasInstanceBuffer || renderGraph.nodes[node.instanceBuffer.resourceNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "Error: instance buffer is not a buffer.");

    const RenderGraphNode_Resource_Buffer* vertexBufferNode = hasVertexBuffer ? &renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer : nullptr;
    const RenderGraphNode_Resource_Buffer* indexBufferNode = hasIndexBuffer ? &renderGraph.nodes[node.indexBuffer.resourceNodeIndex].resourceBuffer : nullptr;
    const RenderGraphNode_Resource_Buffer* instanceBufferNode = hasInstanceBuffer ? &renderGraph.nodes[node.instanceBuffer.resourceNodeIndex].resourceBuffer : nullptr;

    // Define the shader struct
    if (tag == "/*$(TechniqueCPP_Shaders)*/")
    {
        // /*$(Node:PassParameters)*/
        {
            std::ostringstream passParameters;

            if (hasVertexBuffer)
                passParameters << "    RDG_BUFFER_ACCESS(Vertex_Buffer, ERHIAccess::VertexOrIndexBuffer)\n";

            if (hasIndexBuffer)
                passParameters << "    RDG_BUFFER_ACCESS(Index_Buffer, ERHIAccess::VertexOrIndexBuffer)\n";

            if (hasInstanceBuffer)
                passParameters << "    RDG_BUFFER_ACCESS(Instance_Buffer, ERHIAccess::VertexOrIndexBuffer)\n";


            StringReplaceAll(text, "/*$(Node:PassParameters)*/", passParameters.str());
        }

        StringReplaceAll(text, "/*$(Node:EntryPointVS)*/", node.vertexShader.shader->entryPoint);
        StringReplaceAll(text, "/*$(Node:EntryPointPS)*/", node.pixelShader.shader->entryPoint);

        std::string fileNameVS = std::filesystem::path(node.vertexShader.shader->destFileName).replace_extension(".usf").string();
        StringReplaceAll(text, "/*$(Node:FileNameVS)*/", std::string("/Engine/Private/" + renderGraph.name + "/") + fileNameVS);
        StringReplaceAll(text, "/*$(Node:FileNameVSStringEscaped)*/", StringEscape(std::string("/Engine/Private/" + renderGraph.name + "/") + fileNameVS));

        std::string fileNamePS = std::filesystem::path(node.pixelShader.shader->destFileName).replace_extension(".usf").string();
        StringReplaceAll(text, "/*$(Node:FileNamePS)*/", std::string("/Engine/Private/" + renderGraph.name + "/") + fileNamePS);
        StringReplaceAll(text, "/*$(Node:FileNamePSStringEscaped)*/", StringEscape(std::string("/Engine/Private/" + renderGraph.name + "/") + fileNamePS));

        // Shader params - constant buffer and read/write resources
        std::string shaderParamsVS;
        if (node.vertexShader.shader)
            shaderParamsVS = MakeShaderParams(renderGraph, node, node.resourceDependencies, *node.vertexShader.shader, 0, node.vertexShader.shader->resources.size());
        StringReplaceAll(text, "/*$(Node:ShaderParamsVS)*/", shaderParamsVS);
        StringReplaceAll(text, "/*$(Node:ShaderDefinesVS)*/", MakeShaderDefines(*node.vertexShader.shader));

        std::string shaderParamsPS;
        if (node.pixelShader.shader)
            shaderParamsPS = MakeShaderParams(renderGraph, node, node.resourceDependencies, *node.pixelShader.shader, node.vertexShader.shader ? node.vertexShader.shader->resources.size() : 0, node.pixelShader.shader->resources.size());
        StringReplaceAll(text, "/*$(Node:ShaderParamsPS)*/", shaderParamsPS);
        StringReplaceAll(text, "/*$(Node:ShaderDefinesPS)*/", MakeShaderDefines(*node.pixelShader.shader));

        // TODO: samplers?
    }

    // Execute the technique
    if (tag == "/*$(AddTechnique)*/")
    {
        // /*$(Node:Condition)*/
        {
            std::ostringstream condition;

            if (node.condition.comparison != ConditionComparison::Count)
                condition << "    if (" << ConditionToString(node.condition, renderGraph, &VariableToString) << ")\n";

            StringReplaceAll(text, "/*$(Node:Condition)*/", condition.str());
        }

        // /*$(Node:PassSetup)*/
        {
            std::ostringstream passSetupPre;
            std::ostringstream passSetup;

            for (int i = 0; i < node.colorTargets.size(); ++i)
            {
                if (node.colorTargets[i].resourceNodeIndex == -1)
                    break;

                const RenderGraphNode& colorTargetNodeBase = renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
                if (colorTargetNodeBase._index != RenderGraphNode::c_index_resourceTexture)
                    break;

                const RenderGraphNode_Resource_Texture& colorTargetNode = colorTargetNodeBase.resourceTexture;
                const ColorTargetSettings& ctSettings = node.colorTargetSettings[i];

                passSetup << "        PassParameters->RenderTargets[" << i << "]" << " = FRenderTargetBinding(Texture_" << colorTargetNode.name << ", ERenderTargetLoadAction::ELoad);\n\n";

                if (ctSettings.clear)
                    passSetupPre << "        AddClearRenderTargetPass(GraphBuilder, Texture_" << colorTargetNode.name << ", FLinearColor(" << std::fixed << ctSettings.clearColor[0] << "f, " << ctSettings.clearColor[1] << "f, " << ctSettings.clearColor[2] << "f, " << ctSettings.clearColor[3] << "f ));\n\n";
            }

            if (node.depthTarget.resourceNodeIndex != -1)
            {
                const RenderGraphNode_Resource_Texture& depthTargetNode = renderGraph.nodes[node.depthTarget.resourceNodeIndex].resourceTexture;

                std::string depthOp = node.depthWrite ? "Write" : "Read";

                passSetup <<
                    "        bool depthStencilIsStencil = IsStencilFormat(Texture_" << depthTargetNode.name << "->Desc.Format);\n"
                    "\n"
                    "        PassParameters->RenderTargets.DepthStencil" << " = FDepthStencilBinding(\n"
                    "            Texture_" << depthTargetNode.name << ",\n"
                    "            ERenderTargetLoadAction::ELoad,\n"
                    "            depthStencilIsStencil ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction,\n"
                    "            depthStencilIsStencil ? FExclusiveDepthStencil::" << "Depth" << depthOp << "_StencilWrite : FExclusiveDepthStencil::" << "Depth" << depthOp << "_StencilNop\n"
                    "        );\n\n"
                    ;

                if (node.depthTargetClear || node.stencilClear)
                {
                    passSetupPre <<
                        "        AddClearDepthStencilPass(GraphBuilder, Texture_" << depthTargetNode.name << ", " <<
                        (node.depthTargetClear ? "true" : "false") << ", " << std::fixed << node.depthTargetClearValue << "f, " <<
                        (node.stencilClear ? "true" : "false") << ", 0x" << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (unsigned int)node.stencilClearValue << std::dec << ");\n\n";
                }
            }
            else
            {
                passSetup <<
                    "        bool depthStencilIsStencil = false;\n";
            }

            passSetupPre << passSetup.str();

            StringReplaceAll(text, "/*$(Node:PassSetup)*/", passSetupPre.str());
        }

        // /*$(Node:SetParameters)*/
        {
            std::ostringstream setParameters;

            if (hasVertexBuffer || hasIndexBuffer || hasInstanceBuffer)
            {
                setParameters << "        // Rasterization Buffers\n";

                if (hasVertexBuffer)
                    setParameters << "        PassParameters->Vertex_Buffer = Buffer_" << renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer.name << ";\n";

                if (hasIndexBuffer)
                    setParameters << "        PassParameters->Index_Buffer = Buffer_" << renderGraph.nodes[node.indexBuffer.resourceNodeIndex].resourceBuffer.name << ";\n";

                if (hasInstanceBuffer)
                    setParameters << "        PassParameters->Instance_Buffer = Buffer_" << renderGraph.nodes[node.instanceBuffer.resourceNodeIndex].resourceBuffer.name << ";\n";

                setParameters << "\n";
            }


            if (node.vertexShader.shader)
                setParameters << FillShaderParams(renderGraph, node.resourceDependencies, *node.vertexShader.shader, 0, "        ", "PassParameters->VSParameters.", "// Vertex Shader Parameters");

            if (node.pixelShader.shader)
                setParameters << FillShaderParams(renderGraph, node.resourceDependencies, *node.pixelShader.shader, node.vertexShader.shader ? node.vertexShader.shader->resources.size() : 0, "        ", "PassParameters->PSParameters.", "// Pixel Shader Parameters");

            StringReplaceAll(text, "/*$(Node:SetParameters)*/", setParameters.str());
        }

        // /*$(Node:ViewportMax)*/
        {
            std::ostringstream viewportMax;

            int viewportTextureNodeIndex = -1;
            for (int i = 0; i < node.colorTargets.size(); ++i)
            {
                if (node.colorTargets[i].resourceNodeIndex == -1)
                    break;

                const RenderGraphNode& colorTargetNodeBase = renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
                if (colorTargetNodeBase._index != RenderGraphNode::c_index_resourceTexture)
                    break;

                viewportTextureNodeIndex = node.colorTargets[i].resourceNodeIndex;
            }

            if (node.depthTarget.resourceNodeIndex != -1)
                viewportTextureNodeIndex = node.depthTarget.resourceNodeIndex;

            if (viewportTextureNodeIndex != -1)
            {
                viewportMax <<
                    "        const FRDGTextureDesc& viewportDesc = Texture_" << renderGraph.nodes[viewportTextureNodeIndex].resourceTexture.name << "->Desc;\n"
                    "        FIntPoint viewportMax = FIntPoint(viewportDesc.Extent.X, viewportDesc.Extent.Y);\n"
                    "\n"
                    ;
            }
            else
            {
                viewportMax <<
                    "        FIntPoint viewportMax = FIntPoint(0, 0);\n"
                    ;
            }

            StringReplaceAll(text, "/*$(Node:ViewportMax)*/", viewportMax.str());
        }

        // /*$(Node:Execute)*/
        {
            std::ostringstream execute;

            execute << "                psoDesc.ConservativeRasterization = " << ((node.conservativeRasterization) ? "EConservativeRasterization::Overestimated" : "EConservativeRasterization::Disabled") << ";\n\n";

            // Color targets
            {
                int colorTargetCount = 0;
                for (int i = 0; i < node.colorTargets.size(); ++i)
                {
                    if (node.colorTargets[i].resourceNodeIndex == -1)
                        break;

                    const RenderGraphNode& colorTargetNodeBase = renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
                    if (colorTargetNodeBase._index != RenderGraphNode::c_index_resourceTexture)
                        break;

                    const RenderGraphNode_Resource_Texture& colorTargetNode = colorTargetNodeBase.resourceTexture;

                    execute << "                // Color Target: " << colorTargetNode.name << "\n";

                    // Render Target Flags
                    execute <<
                        "                psoDesc.RenderTargetFlags[" << colorTargetCount << "] = " << MakeETextureCreateFlags(colorTargetNode.accessedAs, false) << ";\n"
                        ;

                    // Render target Format
                    TextureNodeFormat format = GetTextureNodeFormat(renderGraph, colorTargetNode);
                    if (format.variableIndex != -1)
                    {
                        execute <<
                            "                psoDesc.RenderTargetFormats[" << colorTargetCount << "] = EPixelFormat(" << VariableToString(renderGraph.variables[format.variableIndex], renderGraph) << ");\n"
                            ;
                    }
                    else if (format.importedNodeIndex != -1)
                    {
                        const RenderGraphNode& importedNodeBase = renderGraph.nodes[format.importedNodeIndex];
                        GigiAssert(importedNodeBase._index == RenderGraphNode::c_index_resourceTexture, "node \"%s\" tried to use a non texture as a color target: \"%s\"", node.name.c_str(), GetNodeName(importedNodeBase));
                        const RenderGraphNode_Resource_Texture& importedNode = importedNodeBase.resourceTexture;

                        execute <<
                            "                psoDesc.RenderTargetFormats[" << colorTargetCount << "] = Texture_" << importedNode.name << "->Desc.Format;\n"
                            "                if (Texture_" << importedNode.name << "->Desc.Format & ETextureCreateFlags::SRGB)\n"
                            "                    psoDesc.RenderTargetFlags[" << colorTargetCount << "] |= ETextureCreateFlags::SRGB;\n"
                            ;
                    }
                    else
                    {
                        bool sRGB = false;

                        execute <<
                            "                psoDesc.RenderTargetFormats[" << colorTargetCount << "] = " << TextureFormatToEPixelFormat(format.format, sRGB) << ";\n"
                            ;

                        if (sRGB)
                        {
                            execute <<
                                "                psoDesc.RenderTargetFlags[" << colorTargetCount << "] |= ETextureCreateFlags::SRGB;\n"
                                ;
                        }
                    }

                    colorTargetCount++;
                    execute << "\n";
                }

                execute << "                psoDesc.RenderTargetsEnabled = " << colorTargetCount << ";\n\n";
            }

            // Depth Target
            {
                bool hasDepthTarget = false;
                if (node.depthTarget.resourceNodeIndex != -1)
                {
                    const RenderGraphNode& depthTargetNodeBase = renderGraph.nodes[node.depthTarget.resourceNodeIndex];
                    if (depthTargetNodeBase._index == RenderGraphNode::c_index_resourceTexture)
                    {
                        const RenderGraphNode_Resource_Texture& depthTargetNode = depthTargetNodeBase.resourceTexture;

                        execute << "                // Depth Target: " << depthTargetNode.name << "\n";

                        // flags
                        execute <<
                            "                psoDesc.DepthStencilTargetFlag = " << MakeETextureCreateFlags(depthTargetNode.accessedAs, false) << ";\n"
                            ;

                        // format
                        TextureNodeFormat format = GetTextureNodeFormat(renderGraph, depthTargetNode);
                        if (format.variableIndex != -1)
                        {
                            execute <<
                                "                psoDesc.DepthStencilTargetFormat = EPixelFormat(" << VariableToString(renderGraph.variables[format.variableIndex], renderGraph) << ");\n"
                                ;
                        }
                        else if (format.importedNodeIndex != -1)
                        {
                            const RenderGraphNode& importedNodeBase = renderGraph.nodes[format.importedNodeIndex];
                            GigiAssert(importedNodeBase._index == RenderGraphNode::c_index_resourceTexture, "node \"%s\" tried to use a non texture as a color target: \"%s\"", node.name.c_str(), GetNodeName(importedNodeBase));
                            const RenderGraphNode_Resource_Texture& importedNode = importedNodeBase.resourceTexture;

                            execute <<
                                "                psoDesc.DepthStencilTargetFormat = Texture_" << importedNode.name << "->Desc.Format;\n"
                                "                if (Texture_" << importedNode.name << "->Desc.Format & ETextureCreateFlags::SRGB)\n"
                                "                    psoDesc.DepthStencilTargetFlag |= ETextureCreateFlags::SRGB;\n"
                                ;
                        }
                        else
                        {
                            bool sRGB = false;

                            execute <<
                                "                psoDesc.DepthStencilTargetFormat = " << TextureFormatToEPixelFormat(format.format, sRGB) << ";\n"
                                ;

                            if (sRGB)
                            {
                                execute <<
                                    "                psoDesc.DepthStencilTargetFlag |= ETextureCreateFlags::SRGB;\n"
                                    ;
                            }
                        }
                        hasDepthTarget = true;

                        execute << "                psoDesc.DepthTargetLoadAction = ERenderTargetLoadAction::ELoad;\n";

                        execute << "                psoDesc.DepthTargetStoreAction = ERenderTargetStoreAction::EStore;\n";
                        /*
                        if (node.depthWrite)
                            execute << "                psoDesc.DepthTargetStoreAction = ERenderTargetStoreAction::EStore;\n";
                        else
                            execute << "                psoDesc.DepthTargetStoreAction = ERenderTargetStoreAction::ENoAction;\n";
                            */

                        execute << "                psoDesc.StencilTargetLoadAction = depthStencilIsStencil ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction;\n";

                        if (node.stencilWriteMask != 0)
                            execute << "                psoDesc.StencilTargetStoreAction = depthStencilIsStencil ? ERenderTargetStoreAction::EStore : ERenderTargetStoreAction::ENoAction;\n";
                        else
                            execute << "                psoDesc.StencilTargetStoreAction = ERenderTargetStoreAction::ENoAction;\n";
                    }
                }

                if (!hasDepthTarget)
                {
                    execute <<
                        "                // Depth Target: None\n"
                        "                psoDesc.DepthStencilTargetFormat = PF_Unknown;\n";
                }

                execute << "\n";
            }

            // Depth Stencil State
            {
                execute <<
                    "                // Depth Stencil State\n"
                    "                psoDesc.DepthStencilState = TStaticDepthStencilState<\n"
                    "                    " << (node.depthWrite ? "true" : "false") << ",\n"
                    "                    " << DepthTestFunctionToECompareFunction(node.depthTest) << ",\n"
                    "                    " << (node.frontFaceStencilFunc == DepthTestFunction::Never ? "false" : "true") << ",\n"
                    "                    " << DepthTestFunctionToECompareFunction(node.frontFaceStencilFunc) << ",\n"
                    "                    " << StencilOpToEStencilOp(node.frontFaceStencilFail) << ",\n"
                    "                    " << StencilOpToEStencilOp(node.frontFaceStencilDepthFail) << ",\n"
                    "                    " << StencilOpToEStencilOp(node.frontFaceStencilPass) << ",\n"
                    "                    " << (node.backFaceStencilFunc == DepthTestFunction::Never ? "false" : "true") << ",\n"
                    "                    " << DepthTestFunctionToECompareFunction(node.backFaceStencilFunc) << ",\n"
                    "                    " << StencilOpToEStencilOp(node.backFaceStencilFail) << ",\n"
                    "                    " << StencilOpToEStencilOp(node.backFaceStencilDepthFail) << ",\n"
                    "                    " << StencilOpToEStencilOp(node.backFaceStencilPass) << ",\n"
                    "                    0x" << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (unsigned int)node.stencilReadMask << std::dec << ",\n"
                    "                    0x" << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (unsigned int)node.stencilWriteMask << std::dec << "\n"
                    "                    >::GetRHI();\n"
                    "\n"
                    ;
            }

            // Blend State
            {
                execute <<
                    "                // Blend State\n"
                    "                psoDesc.BlendState = TStaticBlendState<\n"
                    ;

                for (const ColorTargetSettings& settings : node.colorTargetSettings)
                {
                    std::string writeChannelFlags;

                    if (settings.writeChannels[0] && settings.writeChannels[1] && settings.writeChannels[2] && settings.writeChannels[3])
                    {
                        writeChannelFlags = "CW_RGBA";
                    }
                    else
                    {
                        if (settings.writeChannels[0])
                            writeChannelFlags += "CW_RED | ";
                        if (settings.writeChannels[1])
                            writeChannelFlags += "CW_GREEN | ";
                        if (settings.writeChannels[2])
                            writeChannelFlags += "CW_BLUE | ";
                        if (settings.writeChannels[3])
                            writeChannelFlags += "CW_ALPHA | ";
                        if (!writeChannelFlags.empty())
                        {
                            writeChannelFlags.pop_back();
                            writeChannelFlags.pop_back();
                            writeChannelFlags.pop_back();

                            writeChannelFlags = std::string("(EColorWriteMask)(") + writeChannelFlags + std::string(")");
                        }
                    }

                    unsigned int writeChannelMask = (settings.writeChannels[0] ? 1 : 0) | ((settings.writeChannels[1] ? 1 : 0) << 1) | ((settings.writeChannels[2] ? 1 : 0) << 2) | ((settings.writeChannels[3] ? 1 : 0) << 3);
                    execute <<
                        "                    " << writeChannelFlags
                        ;

                    if (settings.enableBlending)
                    {
                        execute <<
                            ", BO_Add"
                            ", " << DrawBlendModeToEBlendFactor(settings.srcBlend) << ", " << DrawBlendModeToEBlendFactor(settings.destBlend) <<
                            ", BO_Add"
                            ", " << DrawBlendModeToEBlendFactor(settings.srcBlendAlpha) << ", " << DrawBlendModeToEBlendFactor(settings.destBlendAlpha) <<
                            ",\n"
                            ;
                    }
                    else
                    {
                        execute << ", BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero,\n";
                    }
                }

                execute <<
                    "                    " << (node.alphaAsCoverage ? "true" : "false") << "\n"
                    "                    >::GetRHI();\n"
                    "\n"
                    ;
            }

            // Raster State
            {
                execute <<
                    "                // Raster State\n"
                    "                psoDesc.RasterizerState = TStaticRasterizerState<\n"
                    "                    FM_Solid,\n"
                    "                    " << DrawCullModeToERasterizerCullMode(node.cullMode, node.frontIsCounterClockwise) << ",\n"
                    "                    false,\n"
                    "                    false\n"
                    "                    >::GetRHI();\n"
                    "\n"
                    ;
            }

            // Geometry Declaration
            {
                if (hasVertexBuffer || hasInstanceBuffer)
                {
                    execute <<
                        "                // Vertex Declaration\n"
                        "                FVertexDeclarationElementList Elements;\n"
                        ;

                    int streamIndex = 0;
                    if (hasVertexBuffer)
                    {
                        const RenderGraphNode& vbNodeBase = renderGraph.nodes[node.vertexBuffer.resourceNodeIndex];
                        GigiAssert(vbNodeBase._index == RenderGraphNode::c_index_resourceBuffer, "Error");
                        const RenderGraphNode_Resource_Buffer& vbNode = vbNodeBase.resourceBuffer;

                        if (vbNode.visibility == ResourceVisibility::Imported)
                        {
                            execute <<
                                "                for (FVertexElement element : params.inputs.BufferVertexFormat_" << vbNode.name << ")\n"
                                "                {\n"
                                "                    element.StreamIndex = " << streamIndex << ";\n"
                                "                    element.bUseInstanceIndex = 0;\n"
                                "                    Elements.Add(element);\n"
                                "                }\n"
                                ;
                        }
                        else if (vbNode.format.structureType.structIndex != -1)
                        {
                            GigiAssert(false, "TODO: implement");
                        }
                        else
                        {
                            GigiAssert(false, "TODO: implement");
                        }

                        streamIndex++;
                    }

                    if (hasInstanceBuffer)
                    {
                        const RenderGraphNode& ibNodeBase = renderGraph.nodes[node.instanceBuffer.resourceNodeIndex];
                        GigiAssert(ibNodeBase._index == RenderGraphNode::c_index_resourceBuffer, "Error");
                        const RenderGraphNode_Resource_Buffer& ibNode = ibNodeBase.resourceBuffer;

                        if (ibNode.visibility == ResourceVisibility::Imported)
                        {
                            execute <<
                                "                for (FVertexElement element : params.inputs.BufferVertexFormat_" << ibNode.name << ")\n"
                                "                {\n"
                                "                    element.StreamIndex = " << streamIndex << ";\n"
                                "                    element.bUseInstanceIndex = 1;\n"
                                "                    Elements.Add(element);\n"
                                "                }\n"
                                ;
                        }
                        else if (ibNode.format.structureType.structIndex != -1)
                        {
                            GigiAssert(false, "TODO: implement");
                        }
                        else
                        {
                            GigiAssert(false, "TODO: implement");
                        }

                        streamIndex++;
                    }

                    execute <<
                        "                psoDesc.BoundShaderState.VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);\n"
                        "\n"
                        ;
                }
                else
                {
                    execute <<
                        "                psoDesc.BoundShaderState.VertexDeclarationRHI = GEmptyVertexDeclaration.VertexDeclarationRHI;\n"
                        "\n"
                        ;
                }
            }

            execute <<
                "                SetGraphicsPipelineState(RHICmdList, psoDesc, " << (unsigned int)node.stencilRef << ");\n"
                "\n"
                "                SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), PassParameters->VSParameters);\n"
                "                SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters->PSParameters);\n"
                "\n"
                "                RHICmdList.SetViewport(0, 0, 0, viewportMax.X, viewportMax.Y, 1);\n"
                "\n"
                ;

            // Set vertex and instance buffers
            {
                int streamIndex = 0;
                if (hasVertexBuffer)
                {
                    execute <<
                        "                RHICmdList.SetStreamSource(" << streamIndex << ", PassParameters->Vertex_Buffer->GetRHI(), 0);\n"
                        ;
                    streamIndex++;
                }

                if (hasInstanceBuffer)
                {
                    execute <<
                        "                RHICmdList.SetStreamSource(" << streamIndex << ", PassParameters->Instance_Buffer->GetRHI(), 0);\n"
                        ;
                    streamIndex++;
                }
            }

            // Calculate counts for the draw call
            {
                // Calculate vertexCountPerInstance
                if (!hasIndexBuffer)
                {
                    if (hasVertexBuffer)
                    {
                        if (node.countPerInstance == -1)
                            execute << "                int vertexCountPerInstance = PassParameters->Vertex_Buffer->Desc.NumElements;\n\n";
                        else
                            execute << "                int vertexCountPerInstance = min(" << node.countPerInstance << ", PassParameters->Vertex_Buffer->Desc.NumElements);\n\n";
                    }
                    else
                        execute << "                int vertexCountPerInstance = " << node.countPerInstance << ";\n\n";
                }

                // Calculate indexCountPerInstance
                if (hasIndexBuffer)
                {
                    if (node.countPerInstance == -1)
                        execute << "                int indexCountPerInstance = PassParameters->Index_Buffer->Desc.NumElements;\n\n";
                    else
                        execute << "                int indexCountPerInstance = min(" << node.countPerInstance << ", PassParameters->Index_Buffer->Desc.NumElements);\n\n";
                }

                // Calculate instanceCount
                if (hasInstanceBuffer)
                {
                    if (node.instanceCount == -1)
                        execute << "                int instanceCount = PassParameters->Instance_Buffer->Desc.NumElements;\n\n";
                    else
                        execute << "                int instanceCount = min(" << node.instanceCount << ", PassParameters->Instance_Buffer->Desc.NumElements);\n\n";
                }
                else
                    execute << "                int instanceCount = " << node.instanceCount << ";\n\n";
            }

            if (hasIndexBuffer)
            {
                execute <<
                    "                RHICmdList.DrawIndexedPrimitive(PassParameters->Index_Buffer->GetRHI(), 0, 0, indexCountPerInstance, 0, indexCountPerInstance / 3, instanceCount);\n"
                    ;

            }
            else
            {
                execute <<
                    "                RHICmdList.DrawPrimitive(0, vertexCountPerInstance / 3, instanceCount);\n"
                    ;
            }

            StringReplaceAll(text, "/*$(Node:Execute)*/", execute.str());
        }
    }

    return true;
}
