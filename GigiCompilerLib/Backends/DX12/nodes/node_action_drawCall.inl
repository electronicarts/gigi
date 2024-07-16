///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

struct SemanticEnumToString
{
    void Function_Position() { m_string = "POSITION"; m_semanticIndex = (int)StructFieldSemantic::Position; }
    void Function_Color() { m_string = "COLOR"; m_semanticIndex = (int)StructFieldSemantic::Color; }
    void Function_Normal() { m_string = "NORMAL"; m_semanticIndex = (int)StructFieldSemantic::Normal; }
    void Function_Tangent() { m_string = "TANGENT"; m_semanticIndex = (int)StructFieldSemantic::Tangent; }
    void Function_UV() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; }
    void Function_MaterialID() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; }
    void Function_ShapeID() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; }
    void Function_Count() { m_string = "TEXCOORD"; m_semanticIndex = (int)StructFieldSemantic::UV; } // If no semantic given, use auto incrementing texture coordinates
    const char* m_string = "";
    int m_semanticIndex = -1;
};

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_DrawCall& node)
{
    // Storage
    stringReplacementMap["/*$(ContextInternal)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(ContextInternal)*/"] <<
            "\n        // " << node.comment;
    }

    stringReplacementMap["/*$(ContextInternal)*/"] <<
        "\n        ID3D12PipelineState* drawCall_" << node.name << "_pso = nullptr;"
        "\n        ID3D12RootSignature* drawCall_" << node.name << "_rootSig = nullptr;"
        ;

    // Creation
    stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
        "\n"
        "\n        // Draw Call: " << node.name;
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n        // " << node.comment;
    }
    stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
        "\n        if (dirty) // TODO: This is too heavy handed. We should only (re)create when needed."
        "\n        {"
        "\n            if (m_internal.drawCall_" << node.name << "_pso)"
        "\n            {"
        "\n                s_delayedRelease.Add(m_internal.drawCall_" << node.name << "_pso);"
        "\n                m_internal.drawCall_" << node.name << "_pso = nullptr;"
        "\n            }"
        "\n            if (m_internal.drawCall_" << node.name << "_rootSig)"
        "\n            {"
        "\n                s_delayedRelease.Add(m_internal.drawCall_" << node.name << "_rootSig);"
        "\n                m_internal.drawCall_" << node.name << "_rootSig = nullptr;"
        "\n            }"
        "\n        }"
        "\n        if(!m_internal.drawCall_" << node.name << "_pso || !m_internal.drawCall_" << node.name << "_rootSig)"
        "\n        {";

    // Samplers
    int samplerCount = int(node.vertexShader.shader->samplers.size() + node.pixelShader.shader->samplers.size());
    if (samplerCount > 0)
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n            D3D12_STATIC_SAMPLER_DESC samplers[" << samplerCount << "];";

        for (size_t samplerIndex = 0; samplerIndex < node.vertexShader.shader->samplers.size(); ++samplerIndex)
        {
            ShaderSampler& sampler = node.vertexShader.shader->samplers[samplerIndex];

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                "\n            // " << sampler.name << " (VS)"
                "\n            samplers[" << samplerIndex << "].Filter = " << SamplerFilterToD3D12_FILTER(sampler.filter) << ";"
                "\n            samplers[" << samplerIndex << "].AddressU = " << SamplerAddressModeToD3D12_TEXTURE_ADDRESS_MODE(sampler.addressMode) << ";"
                "\n            samplers[" << samplerIndex << "].AddressV = " << SamplerAddressModeToD3D12_TEXTURE_ADDRESS_MODE(sampler.addressMode) << ";"
                "\n            samplers[" << samplerIndex << "].AddressW = " << SamplerAddressModeToD3D12_TEXTURE_ADDRESS_MODE(sampler.addressMode) << ";"
                "\n            samplers[" << samplerIndex << "].MipLODBias  = 0;"
                "\n            samplers[" << samplerIndex << "].MaxAnisotropy  = 0;"
                "\n            samplers[" << samplerIndex << "].ComparisonFunc  = D3D12_COMPARISON_FUNC_NEVER;"
                "\n            samplers[" << samplerIndex << "].BorderColor  = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;"
                "\n            samplers[" << samplerIndex << "].MinLOD = 0.0f;"
                "\n            samplers[" << samplerIndex << "].MaxLOD = D3D12_FLOAT32_MAX;"
                "\n            samplers[" << samplerIndex << "].ShaderRegister = " << samplerIndex << ";"
                "\n            samplers[" << samplerIndex << "].RegisterSpace = 0;"
                "\n            samplers[" << samplerIndex << "].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;";
            ;
        }
        for (size_t samplerIndex = 0; samplerIndex < node.pixelShader.shader->samplers.size(); ++samplerIndex)
        {
            ShaderSampler& sampler = node.pixelShader.shader->samplers[samplerIndex];

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                "\n            // " << sampler.name << " (PS)"
                "\n            samplers[" << samplerIndex << "].Filter = " << SamplerFilterToD3D12_FILTER(sampler.filter) << ";"
                "\n            samplers[" << samplerIndex << "].AddressU = " << SamplerAddressModeToD3D12_TEXTURE_ADDRESS_MODE(sampler.addressMode) << ";"
                "\n            samplers[" << samplerIndex << "].AddressV = " << SamplerAddressModeToD3D12_TEXTURE_ADDRESS_MODE(sampler.addressMode) << ";"
                "\n            samplers[" << samplerIndex << "].AddressW = " << SamplerAddressModeToD3D12_TEXTURE_ADDRESS_MODE(sampler.addressMode) << ";"
                "\n            samplers[" << samplerIndex << "].MipLODBias  = 0;"
                "\n            samplers[" << samplerIndex << "].MaxAnisotropy  = 0;"
                "\n            samplers[" << samplerIndex << "].ComparisonFunc  = D3D12_COMPARISON_FUNC_NEVER;"
                "\n            samplers[" << samplerIndex << "].BorderColor  = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;"
                "\n            samplers[" << samplerIndex << "].MinLOD = 0.0f;"
                "\n            samplers[" << samplerIndex << "].MaxLOD = D3D12_FLOAT32_MAX;"
                "\n            samplers[" << samplerIndex << "].ShaderRegister = " << samplerIndex << ";"
                "\n            samplers[" << samplerIndex << "].RegisterSpace = 1;"
                "\n            samplers[" << samplerIndex << "].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;";
            ;
        }
    }

    // Vertex Descriptor table
    int descriptorTableRangeCountVertex = (int)node.vertexShader.shader->resources.size();
    if (descriptorTableRangeCountVertex > 0)
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n\n            D3D12_DESCRIPTOR_RANGE rangesVertex[" << descriptorTableRangeCountVertex << "];";

        for (size_t descriptorTableRangeIndex = 0; descriptorTableRangeIndex < node.vertexShader.shader->resources.size(); ++descriptorTableRangeIndex)
        {
            ShaderResource& resource = node.vertexShader.shader->resources[descriptorTableRangeIndex];

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n\n            // " << resource.name;

            switch (resource.access)
            {
                case ShaderResourceAccessType::UAV: stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n            rangesVertex[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;"; break;
                case ShaderResourceAccessType::RTScene:
                case ShaderResourceAccessType::SRV: stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n            rangesVertex[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;"; break;
                case ShaderResourceAccessType::CBV: stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n            rangesVertex[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;"; break;
                default:
                {
                    Assert(false, "Unhandled resource access type: %i", resource.access);
                }
            }

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n            rangesVertex[" << descriptorTableRangeIndex << "].NumDescriptors = 1;"
                "\n            rangesVertex[" << descriptorTableRangeIndex << "].BaseShaderRegister = " << resource.registerIndex << ";"
                "\n            rangesVertex[" << descriptorTableRangeIndex << "].RegisterSpace = 0;"
                "\n            rangesVertex[" << descriptorTableRangeIndex << "].OffsetInDescriptorsFromTableStart = " << descriptorTableRangeIndex << ";"
            ;
        }
    }

    // Pixel Descriptor table
    int descriptorTableRangeCountPixel = (int)node.pixelShader.shader->resources.size();
    if (descriptorTableRangeCountPixel > 0)
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n\n            D3D12_DESCRIPTOR_RANGE rangesPixel[" << descriptorTableRangeCountPixel << "];";

        for (size_t descriptorTableRangeIndex = 0; descriptorTableRangeIndex < node.pixelShader.shader->resources.size(); ++descriptorTableRangeIndex)
        {
            ShaderResource& resource = node.pixelShader.shader->resources[descriptorTableRangeIndex];

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n\n            // " << resource.name;

            switch (resource.access)
            {
                case ShaderResourceAccessType::UAV: stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n            rangesPixel[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;"; break;
                case ShaderResourceAccessType::RTScene:
                case ShaderResourceAccessType::SRV: stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n            rangesPixel[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;"; break;
                case ShaderResourceAccessType::CBV: stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] << "\n            rangesPixel[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;"; break;
                default:
                {
                    Assert(false, "Unhandled resource access type: %i", resource.access);
                }
            }

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n            rangesPixel[" << descriptorTableRangeIndex << "].NumDescriptors = 1;"
                "\n            rangesPixel[" << descriptorTableRangeIndex << "].BaseShaderRegister = " << resource.registerIndex << ";"
                "\n            rangesPixel[" << descriptorTableRangeIndex << "].RegisterSpace = 0;"
                "\n            rangesPixel[" << descriptorTableRangeIndex << "].OffsetInDescriptorsFromTableStart = " << descriptorTableRangeIndex << ";"
                ;
        }
    }

    int rootParamCount = 0 + (descriptorTableRangeCountVertex > 0 ? 1 : 0) + (descriptorTableRangeCountPixel > 0 ? 1 : 0);
    if (rootParamCount > 0)
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            D3D12_ROOT_PARAMETER rootParams[" << rootParamCount << "];"
            ;
        int rootParamIndex = 0;
        if (descriptorTableRangeCountVertex > 0)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                "\n            rootParams[" << rootParamIndex << "].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;"
                "\n            rootParams[" << rootParamIndex << "].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;"
                "\n            rootParams[" << rootParamIndex << "].DescriptorTable.NumDescriptorRanges = " << descriptorTableRangeCountVertex << ";"
                "\n            rootParams[" << rootParamIndex << "].DescriptorTable.pDescriptorRanges = rangesVertex;"
                ;
            rootParamIndex++;
        }
        if (descriptorTableRangeCountPixel > 0)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                "\n            rootParams[" << rootParamIndex << "].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;"
                "\n            rootParams[" << rootParamIndex << "].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;"
                "\n            rootParams[" << rootParamIndex << "].DescriptorTable.NumDescriptorRanges = " << descriptorTableRangeCountPixel << ";"
                "\n            rootParams[" << rootParamIndex << "].DescriptorTable.pDescriptorRanges = rangesPixel;"
                ;
            rootParamIndex++;
        }
    }
    else
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            D3D12_ROOT_PARAMETER *rootParams = nullptr;"
            ;
    }

    stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
        "\n"
        "\n            // Root desc"
        "\n            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};"
        "\n            rootDesc.NumParameters = " << rootParamCount << ";"
        "\n            rootDesc.pParameters = rootParams;"
        "\n            rootDesc.NumStaticSamplers = " << samplerCount << ";"
        "\n            rootDesc.pStaticSamplers = " << (samplerCount > 0 ? "samplers" : "nullptr") << ";"
        "\n            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;"
        "\n"
        "\n            // Create it"
        "\n            ID3DBlob* sig = nullptr;"
        "\n            ID3DBlob* error = nullptr;"
        "\n            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);"
        "\n            if (FAILED(hr))"
        "\n            {"
        "\n                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);"
        "\n                if (errorMsg)"
        "\n                    Context::LogFn(LogLevel::Error, \"Could not serialize root signature : %s\", errorMsg);"
        "\n                if (sig) sig->Release();"
        "\n                if (error) error->Release();"
        "\n                return false;"
        "\n            }"
        "\n"
        "\n            char* sigptr = (char*)sig->GetBufferPointer();"
        "\n            auto sigsize = sig->GetBufferSize();"
        "\n"
        "\n            hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&m_internal.drawCall_" << node.name << "_rootSig));"
        "\n            if (FAILED(hr))"
        "\n            {"
        "\n                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);"
        "\n                if (errorMsg)"
        "\n                    Context::LogFn(LogLevel::Error, \"Could not create root signature: %s\", errorMsg);"
        "\n                if (sig) sig->Release();"
        "\n                if (error) error->Release();"
        "\n                return false;"
        "\n            }"
        "\n"
        "\n            if (sig)"
        "\n                sig->Release();"
        "\n"
        "\n            if (error)"
        "\n                error->Release();"
        "\n"
        "\n            // name the root signature for debuggers"
        "\n            if (c_debugNames)"
        "\n                m_internal.drawCall_" << node.name << "_rootSig->SetName(L\"" << node.name << "\");"
        ;

    // Vertex Shader
    if (node.defines.empty() && node.vertexShader.shader->defines.empty())
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            D3D_SHADER_MACRO* definesVS = nullptr;";
    }
    else
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            D3D_SHADER_MACRO definesVS[] = {";

        for (const ShaderDefine& define : node.vertexShader.shader->defines)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n                { \"" << define.name << "\", \"" << define.value << "\" },";
        }

        for (const ShaderDefine& define : node.defines)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n                { \"" << define.name << "\", \"" << define.value << "\" },";
        }

        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n                { nullptr, nullptr }"
            "\n            };";
    }

    const char* shaderCompiler = (renderGraph.settings.dx12.shaderCompiler == DXShaderCompiler::DXC) ? "_DXC" : "_FXC";
    stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
        "\n"
        "\n            std::vector<unsigned char> byteCodeVS = DX12Utils::CompileShaderToByteCode" << shaderCompiler << "(Context::s_techniqueLocation.c_str(), L\"shaders/" << node.vertexShader.shader->destFileName << "\", \"" << node.vertexShader.shader->entryPoint << "\", \"" << renderGraph.settings.dx12.shaderModelVs << "\", definesVS, c_debugShaders, Context::LogFn);"
        "\n            if (byteCodeVS.size() == 0)"
        "\n                return false;"
        ;

    // Pixel Shader
    if (node.defines.empty() && node.pixelShader.shader->defines.empty())
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            D3D_SHADER_MACRO* definesPS = nullptr;";
    }
    else
    {
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            D3D_SHADER_MACRO definesPS[] = {";

        for (const ShaderDefine& define : node.pixelShader.shader->defines)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n                { \"" << define.name << "\", \"" << define.value << "\" },";
        }

        for (const ShaderDefine& define : node.defines)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n                { \"" << define.name << "\", \"" << define.value << "\" },";
        }

        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n                { nullptr, nullptr }"
            "\n            };";
    }

    stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
        "\n"
        "\n            std::vector<unsigned char> byteCodePS = DX12Utils::CompileShaderToByteCode" << shaderCompiler << "(Context::s_techniqueLocation.c_str(), L\"shaders/" << node.pixelShader.shader->destFileName << "\", \"" << node.pixelShader.shader->entryPoint << "\", \"" << renderGraph.settings.dx12.shaderModelPs << "\", definesPS, c_debugShaders, Context::LogFn);"
        "\n            if (byteCodePS.size() == 0)"
        "\n                return false;"
        ;

    // PSO
    {
        // get the maximum used UV slot, non UV fields that use tex coord fields can use numbers greater than that
        int maxUsedUVSlot = -1;
        {
            if (node.vertexBuffer.resourceNodeIndex != -1)
            {
                Assert(renderGraph.nodes[node.vertexBuffer.resourceNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "Error");
                RenderGraphNode_Resource_Buffer& bufferNode = renderGraph.nodes[node.vertexBuffer.resourceNodeIndex].resourceBuffer;
                if (bufferNode.format.structureType.structIndex != -1)
                {
                    const Struct& structDesc = renderGraph.structs[bufferNode.format.structureType.structIndex];
                    for (const StructField& field : structDesc.fields)
                    {
                        if (field.semantic == StructFieldSemantic::UV)
                            maxUsedUVSlot = std::max(maxUsedUVSlot, field.semanticIndex);
                    }
                }
            }

            if (node.instanceBuffer.resourceNodeIndex != -1)
            {
                Assert(renderGraph.nodes[node.instanceBuffer.resourceNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "Error");
                RenderGraphNode_Resource_Buffer& bufferNode = renderGraph.nodes[node.instanceBuffer.resourceNodeIndex].resourceBuffer;
                if (bufferNode.format.structureType.structIndex != -1)
                {
                    const Struct& structDesc = renderGraph.structs[bufferNode.format.structureType.structIndex];
                    for (const StructField& field : structDesc.fields)
                    {
                        if (field.semantic == StructFieldSemantic::UV)
                            maxUsedUVSlot = std::max(maxUsedUVSlot, field.semanticIndex);
                    }
                }
            }
        }

        // The vertex input layout will be dynamically sized if either a vertex or instance buffer are imported
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInputLayout;"
            ;

        // Add the vertex buffer to the vertex input layout
        if (node.vertexBuffer.resourceNodeIndex != -1)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                "\n            // Vertex buffer vertex input layout"
                ;

            RenderGraphNode& nodeBase = renderGraph.nodes[node.vertexBuffer.resourceNodeIndex];
            Assert(nodeBase._index == RenderGraphNode::c_index_resourceBuffer, "Error");
            RenderGraphNode_Resource_Buffer& bufferNode = nodeBase.resourceBuffer;
            if (bufferNode.visibility == ResourceVisibility::Imported)
            {
                stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                    "\n            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_" << bufferNode.name << "_vertexInputLayout.begin(), m_input.buffer_" << bufferNode.name << "_vertexInputLayout.end());"
                    ;
            }
            else if (bufferNode.format.structureType.structIndex != -1)
            {
                int fieldCount = 0;
                int offset = 0;
                const Struct& structDesc = renderGraph.structs[bufferNode.format.structureType.structIndex];
                for (const StructField& field : structDesc.fields)
                {
                    // Get the field info
                    DataFieldTypeInfoStruct fieldInfo = DataFieldTypeInfo(field.type);

                    // Get the semantic string
                    SemanticEnumToString obj;
                    EnumDispatch(obj, field.semantic);
                    const char* semanticString = obj.m_string;

                    // Only include this if there is a semantic string
                    if (semanticString[0] != 0)
                    {
                        // Use the field's semanticIndex, unless this field wants to use TEXCOORD but is not a UV.
                        // Then we need to give valid free texcoord indices.
                        int semanticIndex = field.semanticIndex;
                        if (!_stricmp(semanticString, "TEXCOORD") && field.semantic != StructFieldSemantic::UV)
                        {
                            maxUsedUVSlot++;
                            semanticIndex = maxUsedUVSlot;
                        }

                        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                            "\n                vertexInputLayout.push_back({ \"" << semanticString << "\", " << semanticIndex << ", " << DataFieldTypeToDXGIFormat(field.type) << ", 0, " << offset << ", D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });"
                            ;
                        fieldCount++;
                    }

                    // keep track of the offset
                    offset += fieldInfo.typeBytes;
                }
            }
            else
            {
                stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                    "\n                vertexInputLayout.push_back({ \"POSITION\", 0, " << GetResourceNodePathInContext(GetNodeResourceVisibility(nodeBase)) << "buffer_" << GetNodeName(nodeBase) << "_format, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });"
                    ;
            }
        }

        // Add the instance buffer to the vertex input layout
        if (node.instanceBuffer.resourceNodeIndex != -1)
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                "\n            // Instance buffer vertex input layout"
                ;

            RenderGraphNode& nodeBase = renderGraph.nodes[node.instanceBuffer.resourceNodeIndex];
            Assert(nodeBase._index == RenderGraphNode::c_index_resourceBuffer, "Error");
            RenderGraphNode_Resource_Buffer& bufferNode = nodeBase.resourceBuffer;
            if (bufferNode.visibility == ResourceVisibility::Imported)
            {
                stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                    "\n            vertexInputLayout.insert(vertexInputLayout.end(), m_input.buffer_" << bufferNode.name << "_vertexInputLayout.begin(), m_input.buffer_" << bufferNode.name << "_vertexInputLayout.end());"
                    ;
            }
            else if (bufferNode.format.structureType.structIndex != -1)
            {
                int fieldCount = 0;
                int offset = 0;
                const Struct& structDesc = renderGraph.structs[bufferNode.format.structureType.structIndex];
                for (const StructField& field : structDesc.fields)
                {
                    // Get the field info
                    DataFieldTypeInfoStruct fieldInfo = DataFieldTypeInfo(field.type);

                    // Get the semantic string
                    SemanticEnumToString obj;
                    EnumDispatch(obj, field.semantic);
                    const char* semanticString = obj.m_string;

                    // Only include this if there is a semantic string
                    if (semanticString[0] != 0)
                    {
                        // Use the field's semanticIndex, unless this field wants to use TEXCOORD but is not a UV.
                        // Then we need to give valid free texcoord indices.
                        int semanticIndex = field.semanticIndex;
                        if (!_stricmp(semanticString, "TEXCOORD") && field.semantic != StructFieldSemantic::UV)
                        {
                            maxUsedUVSlot++;
                            semanticIndex = maxUsedUVSlot;
                        }

                        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                            "\n                vertexInputLayout.push_back({ \"" << semanticString << "\", " << semanticIndex << ", " << DataFieldTypeToDXGIFormat(field.type) << ", 1, " << offset << ", D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 0 });"
                            ;
                        fieldCount++;
                    }

                    // keep track of the offset
                    offset += fieldInfo.typeBytes;
                }
            }
            else
            {
                stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                    "\n                vertexInputLayout.push_back({ \"POSITION\", 0, " << GetResourceNodePathInContext(GetNodeResourceVisibility(nodeBase)) << "buffer_" << GetNodeName(nodeBase) << "_format, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 0 });"
                    ;
            }
        }

        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            // Make the PSO desc"
            "\n            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};"
            "\n            psoDesc.InputLayout = { vertexInputLayout.data(), (UINT)vertexInputLayout.size() };"
            "\n            psoDesc.pRootSignature = m_internal.drawCall_" << node.name << "_rootSig;"
            "\n            psoDesc.VS.pShaderBytecode = byteCodeVS.data();"
            "\n            psoDesc.VS.BytecodeLength = byteCodeVS.size();"
            "\n            psoDesc.PS.pShaderBytecode = byteCodePS.data();"
            "\n            psoDesc.PS.BytecodeLength = byteCodePS.size();"
            "\n            psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;"
            "\n            psoDesc.RasterizerState.CullMode = " << DrawCullModeToD3D12_CULL_MODE(node.cullMode) << ";"
            "\n            psoDesc.RasterizerState.FrontCounterClockwise = " << (node.frontIsCounterClockwise ? "TRUE" : "FALSE") << ";"
            "\n            psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;"
            "\n            psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;"
            "\n            psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;"
            "\n            psoDesc.RasterizerState.DepthClipEnable = TRUE;"
            "\n            psoDesc.RasterizerState.MultisampleEnable = FALSE;"
            "\n            psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;"
            "\n            psoDesc.RasterizerState.ForcedSampleCount = 0;"
            "\n            psoDesc.RasterizerState.ConservativeRaster = " << (node.conservativeRasterization ? "D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON" : "D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF") << ";"
            "\n            psoDesc.BlendState.AlphaToCoverageEnable = FALSE;"
            "\n            psoDesc.BlendState.IndependentBlendEnable = " << (node.independentAlpha ? "TRUE" : "FALSE") << ";"
            ;

        // render target blend state settings
        for (int i = 0; i < node.colorTargetSettings.size(); ++i)
        {
            unsigned int writeChannelFlags = 0;
            for (int channelIndex = 0; channelIndex < 4; ++channelIndex)
            {
                if (node.colorTargetSettings[i].writeChannels[channelIndex])
                    writeChannelFlags |= (1 << channelIndex);
            }

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                "\n            psoDesc.BlendState.RenderTarget[" << i << "] = D3D12_RENDER_TARGET_BLEND_DESC"
                "\n            {"
                "\n                " << (node.colorTargetSettings[i].enableBlending ? "TRUE" : "FALSE") << ", FALSE,"
                "\n                " << DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].srcBlend) << ", " << DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].destBlend) << ", D3D12_BLEND_OP_ADD,"
                "\n                " << DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].srcBlendAlpha) << ", " << DrawBlendModeToD3D12_BLEND(node.colorTargetSettings[i].destBlendAlpha) << ", D3D12_BLEND_OP_ADD,"
                "\n                D3D12_LOGIC_OP_NOOP, " << writeChannelFlags <<
                "\n            };"
                ;
        }

        // depth target settings
        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n"
            "\n            psoDesc.DepthStencilState.DepthWriteMask = " << (node.depthWrite ? "D3D12_DEPTH_WRITE_MASK_ALL" : "D3D12_DEPTH_WRITE_MASK_ZERO") << ";"
            "\n            psoDesc.DepthStencilState.DepthFunc = " << DepthTestFunctionToD3D12_COMPARISON_FUNC(node.depthTest) << ";"
            "\n            psoDesc.SampleMask = UINT_MAX;"
            "\n            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;"
            ;

        // Setup the render targets
        {
            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n"
                ;

            int numRenderTargets = 0;
            for (int i = 0; i < node.colorTargets.size(); ++i)
            {
                if (node.colorTargets[i].resourceNodeIndex == -1)
                    break;

                const RenderGraphNode& resourceNode = renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
                if (resourceNode._index != RenderGraphNode::c_index_resourceTexture)
                    break;

                stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                    "\n            psoDesc.RTVFormats[" << i << "] = " << GetResourceNodePathInContext(GetNodeResourceVisibility(resourceNode)) << "texture_" << GetNodeName(resourceNode) << "_format;"
                    ;
                numRenderTargets++;
            }

            stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                "\n            psoDesc.NumRenderTargets = " << numRenderTargets << ";"
                ;
        }

        // setup the depth target
        {
            bool hasDepthTarget = false;
            if (node.depthTarget.resourceNodeIndex != -1)
            {
                const RenderGraphNode& resourceNode = renderGraph.nodes[node.depthTarget.resourceNodeIndex];
                if (resourceNode._index == RenderGraphNode::c_index_resourceTexture)
                {
                    hasDepthTarget = true;
                    stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                        "\n"
                        "\n            psoDesc.DepthStencilState.DepthEnable = TRUE;"
                        "\n            psoDesc.DSVFormat = DX12Utils::DSV_Safe_DXGI_FORMAT(" << GetResourceNodePathInContext(GetNodeResourceVisibility(resourceNode)) << "texture_" << GetNodeName(resourceNode) << "_format);"
                        "\n            if (DX12Utils::isStencilDXGI_FORMAT(psoDesc.DSVFormat))"
                        "\n            {"
                        "\n                psoDesc.DepthStencilState.StencilEnable = TRUE;"
                        "\n                psoDesc.DepthStencilState.StencilReadMask = " << (unsigned int)node.stencilReadMask << ";"
                        "\n                psoDesc.DepthStencilState.StencilWriteMask = " << (unsigned int)node.stencilWriteMask << ";"
                        "\n                psoDesc.DepthStencilState.FrontFace.StencilFailOp = " << StencilOpToD3D12_STENCIL_OP(node.frontFaceStencilFail) << ";"
                        "\n                psoDesc.DepthStencilState.BackFace.StencilFailOp = " << StencilOpToD3D12_STENCIL_OP(node.backFaceStencilFail) << ";"
                        "\n                psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = " << StencilOpToD3D12_STENCIL_OP(node.frontFaceStencilDepthFail) << ";"
                        "\n                psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = " << StencilOpToD3D12_STENCIL_OP(node.backFaceStencilDepthFail) << ";"
                        "\n                psoDesc.DepthStencilState.FrontFace.StencilPassOp = " << StencilOpToD3D12_STENCIL_OP(node.frontFaceStencilPass) << ";"
                        "\n                psoDesc.DepthStencilState.BackFace.StencilPassOp = " << StencilOpToD3D12_STENCIL_OP(node.backFaceStencilPass) << ";"
                        "\n                psoDesc.DepthStencilState.FrontFace.StencilFunc = " << DepthTestFunctionToD3D12_COMPARISON_FUNC(node.frontFaceStencilFunc) << ";"
                        "\n                psoDesc.DepthStencilState.BackFace.StencilFunc = " << DepthTestFunctionToD3D12_COMPARISON_FUNC(node.backFaceStencilFunc) << ";"
                        "\n            }"
                        ;
                }
            }

            if (!hasDepthTarget)
            {
                stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
                    "\n"
                    "\n            psoDesc.DepthStencilState.DepthEnable = FALSE;"
                    "\n            psoDesc.DepthStencilState.StencilEnable = FALSE;"
                    ;
            }
        }

        stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
            "\n            psoDesc.SampleDesc.Count = 1;"
            "\n            hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_internal.drawCall_" << node.name << "_pso));"
            "\n            if (FAILED(hr))"
            "\n            {"
            "\n                Context::LogFn(LogLevel::Error, \"Could not create PSO for " << node.name << "\");"
            "\n                return false;"
            "\n            }"
            "\n"
            "\n            // name the PSO for debuggers"
            "\n            if (c_debugNames)"
            "\n                m_internal.drawCall_" << node.name << "_pso->SetName(L\"" << node.name << "\");"
            ;
    }

    stringReplacementMap["/*$(CreateDrawCallPSOs)*/"] <<
        "\n        }"
        ;

    // Destruction
    stringReplacementMap["/*$(ContextDestructor)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(ContextDestructor)*/"] <<
            "\n        // " << node.comment;
    }

    stringReplacementMap["/*$(ContextDestructor)*/"] <<
        "\n        if(m_internal.drawCall_" << node.name << "_pso)"
        "\n        {"
        "\n            s_delayedRelease.Add(m_internal.drawCall_" << node.name << "_pso);"
        "\n            m_internal.drawCall_" << node.name << "_pso = nullptr;"
        "\n        }"
        "\n"
        "\n        if(m_internal.drawCall_" << node.name << "_rootSig)"
        "\n        {"
        "\n            s_delayedRelease.Add(m_internal.drawCall_" << node.name << "_rootSig);"
        "\n            m_internal.drawCall_" << node.name << "_rootSig = nullptr;"
        "\n        }"
        ;

    // Execute
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n        // Draw Call: " << node.name
        ;

    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n        // " << node.comment
            ;
    }

    // condition for execution
    if (node.condition.comparison != ConditionComparison::Count)
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n        if(" << ConditionToString(node.condition, renderGraph, &VariableToString) << ")";
    }

    stringReplacementMap["/*$(Execute)*/"] <<
        "\n        {"
        "\n            ScopedPerfEvent scopedPerf(\"Draw Call: " << node.name << "\", commandList, " << node.nodeIndex << ");"
        "\n            std::chrono::high_resolution_clock::time_point startPointCPU;"
        "\n            if(context->m_profile)"
        "\n            {"
        "\n                startPointCPU = std::chrono::high_resolution_clock::now();"
        "\n                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);"
        "\n            }"
        "\n"
        "\n            commandList->SetGraphicsRootSignature(context->m_internal.drawCall_" << node.name << "_rootSig);"
        "\n            commandList->SetPipelineState(context->m_internal.drawCall_" << node.name << "_pso);"
        ;

    // Vertex shader descriptor table
    int rootSigParamIndex = 0;
    {
        const Shader& shader = *node.vertexShader.shader;
        int pinOffset = 0;

        std::ostringstream descriptorsText;
        int descriptorCount = 0;
        for (int resourceIndex = 0; resourceIndex < shader.resources.size(); ++resourceIndex)
        {
            const ShaderResource& shaderResource = shader.resources[resourceIndex];

            int depIndex = 0;
            while (depIndex < node.resourceDependencies.size() && node.resourceDependencies[depIndex].pinIndex != (resourceIndex + pinOffset))
                depIndex++;

            if (depIndex >= node.resourceDependencies.size())
            {
                Assert(false, "Could not find resource dependency for shader resource \"%s\" in draw call node \"%s\"", shaderResource.name.c_str(), node.name.c_str());
                return;
            }
            const ResourceDependency& dep = node.resourceDependencies[depIndex];

            RenderGraphNode depNode = renderGraph.nodes[dep.nodeIndex];

            const char* resourceTypeString = "";
            std::ostringstream rawAndStrideAndCount;
            switch (dep.type)
            {
                case ShaderResourceType::ConstantBuffer:
                {
                    descriptorsText <<
                        "\n                { " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "constantBuffer_" << GetNodeName(depNode) << ", " <<
                        "DXGI_FORMAT_UNKNOWN,"
                        ;
                    resourceTypeString = "DX12Utils::ResourceType::Buffer";

                    Assert(renderGraph.nodes[dep.nodeIndex]._index == RenderGraphNode::c_index_resourceShaderConstants, "Unexpected problem occured!");
                    RenderGraphNode_Resource_ShaderConstants& node = renderGraph.nodes[dep.nodeIndex].resourceShaderConstants;
                    size_t sizeInBytesAligned = ALIGN(256, renderGraph.structs[node.structure.structIndex].sizeInBytes);
                    rawAndStrideAndCount << ", false, " << sizeInBytesAligned << ", 1";
                    break;
                }
                case ShaderResourceType::Buffer:
                {
                    if (dep.access == ShaderResourceAccessType::RTScene)
                    {
                        descriptorsText <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "tlas, " <<
                            "DXGI_FORMAT_UNKNOWN,"
                            ;
                        resourceTypeString = "DX12Utils::ResourceType::Buffer";
                        rawAndStrideAndCount << ", false, context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_tlasSize, 1";
                    }
                    else
                    {
                        descriptorsText <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << ", " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_format,"
                            ;
                        resourceTypeString = "DX12Utils::ResourceType::Buffer";
                        rawAndStrideAndCount << ", " << (shader.resources[depIndex].buffer.raw ? "true" : "false") << ", context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_stride, context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_count";
                    }
                    break;
                }
                case ShaderResourceType::Texture:
                {
                    descriptorsText <<
                        "\n                { " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << ", " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_format,"
                        ;

                    rawAndStrideAndCount << ", false, 0, ";
                    switch (shader.resources[depIndex].texture.dimension)
                    {
                        case TextureDimensionType::Texture2D: resourceTypeString = "DX12Utils::ResourceType::Texture2D"; rawAndStrideAndCount << "0"; break;
                        case TextureDimensionType::Texture2DArray: resourceTypeString = "DX12Utils::ResourceType::Texture2DArray"; rawAndStrideAndCount << "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_size[2]"; break;
                        case TextureDimensionType::Texture3D: resourceTypeString = "DX12Utils::ResourceType::Texture3D"; rawAndStrideAndCount << "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_size[2]"; break;
                        case TextureDimensionType::TextureCube: resourceTypeString = "DX12Utils::ResourceType::TextureCube"; rawAndStrideAndCount << "6"; break;
                        default:
                        {
                            Assert(false, "Unhandled TextureDimensionType: %s (%i)", EnumToString(shader.resources[depIndex].texture.dimension), (int)shader.resources[depIndex].texture.dimension);
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    Assert(false, "Unhandled resource type for draw call node \"%s\"", node.name.c_str());
                    return;
                }
            }

            const char* accessType = nullptr;
            switch (dep.access)
            {
                case ShaderResourceAccessType::UAV: accessType = " DX12Utils::AccessType::UAV, "; break;
                case ShaderResourceAccessType::RTScene:
                case ShaderResourceAccessType::SRV: accessType = " DX12Utils::AccessType::SRV, "; break;
                case ShaderResourceAccessType::CBV: accessType = " DX12Utils::AccessType::CBV, "; break;
                default:
                {
                    Assert(false, "Unhandled resource type: %i", dep.access);
                }
            }

            descriptorsText << accessType << resourceTypeString << rawAndStrideAndCount.str() << " },";
            descriptorCount++;
        }

        if (descriptorCount > 0)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            DX12Utils::ResourceDescriptor descriptorsVS[] ="
                "\n            {"
                << descriptorsText.str() <<
                "\n            };"
                "\n            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableVS = GetDescriptorTable(device, s_srvHeap, descriptorsVS, " << descriptorCount << ", Context::LogFn);"
                "\n            commandList->SetGraphicsRootDescriptorTable(" << rootSigParamIndex << ", descriptorTableVS);"
                ;
            rootSigParamIndex++;
        }
    }

    // Pixel shader descriptor table
    {
        const Shader& shader = *node.pixelShader.shader;
        int pinOffset = (int)node.vertexShader.shader->resources.size();

        std::ostringstream descriptorsText;
        int descriptorCount = 0;
        for (int resourceIndex = 0; resourceIndex < shader.resources.size(); ++resourceIndex)
        {
            const ShaderResource& shaderResource = shader.resources[resourceIndex];

            int depIndex = 0;
            while (depIndex < node.resourceDependencies.size() && node.resourceDependencies[depIndex].pinIndex != (resourceIndex + pinOffset))
                depIndex++;

            if (depIndex >= node.resourceDependencies.size())
            {
                Assert(false, "Could not find resource dependency for shader resource \"%s\" in draw call node \"%s\"", shaderResource.name.c_str(), node.name.c_str());
                return;
            }
            const ResourceDependency& dep = node.resourceDependencies[depIndex];

            RenderGraphNode depNode = renderGraph.nodes[dep.nodeIndex];

            const char* resourceTypeString = "";
            std::ostringstream rawAndStrideAndCount;
            switch (dep.type)
            {
                case ShaderResourceType::ConstantBuffer:
                {
                    descriptorsText <<
                        "\n                { " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "constantBuffer_" << GetNodeName(depNode) << ", " <<
                        "DXGI_FORMAT_UNKNOWN,"
                        ;
                    resourceTypeString = "DX12Utils::ResourceType::Buffer";

                    Assert(renderGraph.nodes[dep.nodeIndex]._index == RenderGraphNode::c_index_resourceShaderConstants, "Unexpected problem occured!");
                    RenderGraphNode_Resource_ShaderConstants& node = renderGraph.nodes[dep.nodeIndex].resourceShaderConstants;
                    size_t sizeInBytesAligned = ALIGN(256, renderGraph.structs[node.structure.structIndex].sizeInBytes);
                    rawAndStrideAndCount << ", false, " << sizeInBytesAligned << ", 1";
                    break;
                }
                case ShaderResourceType::Buffer:
                {
                    if (dep.access == ShaderResourceAccessType::RTScene)
                    {
                        descriptorsText <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "tlas, " <<
                            "DXGI_FORMAT_UNKNOWN,"
                            ;
                        resourceTypeString = "DX12Utils::ResourceType::Buffer";
                        rawAndStrideAndCount << ", false, context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_tlasSize, 1";
                    }
                    else
                    {
                        descriptorsText <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << ", " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_format,"
                            ;
                        resourceTypeString = "DX12Utils::ResourceType::Buffer";
                        rawAndStrideAndCount << ", " << (shader.resources[depIndex].buffer.raw ? "true" : "false") << ", context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_stride, context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_count";
                    }
                    break;
                }
                case ShaderResourceType::Texture:
                {
                    descriptorsText <<
                        "\n                { " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << ", " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_format,"
                        ;

                    rawAndStrideAndCount << ", false, 0, ";
                    switch (shader.resources[depIndex].texture.dimension)
                    {
                        case TextureDimensionType::Texture2D: resourceTypeString = "DX12Utils::ResourceType::Texture2D"; rawAndStrideAndCount << "0"; break;
                        case TextureDimensionType::Texture2DArray: resourceTypeString = "DX12Utils::ResourceType::Texture2DArray"; rawAndStrideAndCount << "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_size[2]"; break;
                        case TextureDimensionType::Texture3D: resourceTypeString = "DX12Utils::ResourceType::Texture3D"; rawAndStrideAndCount << "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_size[2]"; break;
                        case TextureDimensionType::TextureCube: resourceTypeString = "DX12Utils::ResourceType::TextureCube"; rawAndStrideAndCount << "6"; break;
                        default:
                        {
                            Assert(false, "Unhandled TextureDimensionType: %s (%i)", EnumToString(shader.resources[depIndex].texture.dimension), (int)shader.resources[depIndex].texture.dimension);
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    Assert(false, "Unhandled resource type for draw call node \"%s\"", node.name.c_str());
                    return;
                }
            }

            const char* accessType = nullptr;
            switch (dep.access)
            {
                case ShaderResourceAccessType::UAV: accessType = " DX12Utils::AccessType::UAV, "; break;
                case ShaderResourceAccessType::RTScene:
                case ShaderResourceAccessType::SRV: accessType = " DX12Utils::AccessType::SRV, "; break;
                case ShaderResourceAccessType::CBV: accessType = " DX12Utils::AccessType::CBV, "; break;
                default:
                {
                    Assert(false, "Unhandled resource type: %i", dep.access);
                }
            }

            descriptorsText << accessType << resourceTypeString << rawAndStrideAndCount.str() << " },";
            descriptorCount++;
        }

        if (descriptorCount > 0)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            DX12Utils::ResourceDescriptor descriptorsPS[] ="
                "\n            {"
                << descriptorsText.str() <<
                "\n            };"
                "\n            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTablePS = GetDescriptorTable(device, s_srvHeap, descriptorsPS, " << descriptorCount << ", Context::LogFn);"
                "\n            commandList->SetGraphicsRootDescriptorTable(" << rootSigParamIndex << ", descriptorTablePS);"
                ;
            rootSigParamIndex++;
        }
    }

    // Handle the vertex buffer
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            // Vertex Buffer"
            ;
        bool hasVertexBuffer = false;
        if (node.vertexBuffer.resourceNodeIndex != -1)
        {
            const RenderGraphNode& vbNodeBase = renderGraph.nodes[node.vertexBuffer.resourceNodeIndex];
            if (vbNodeBase._index == RenderGraphNode::c_index_resourceBuffer)
            {
                hasVertexBuffer = true;
                const RenderGraphNode_Resource_Buffer& vbNode = vbNodeBase.resourceBuffer;

                if (node.countPerInstance == -1)
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n            int vertexCountPerInstance = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(vbNodeBase)) << "buffer_" << vbNode.name << "_count;"
                        ;
                }
                else
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n            int vertexCountPerInstance = min(" << node.countPerInstance << ", context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(vbNodeBase)) << "buffer_" << vbNode.name << "_count);"
                        ;
                }

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            D3D12_VERTEX_BUFFER_VIEW vbView;"
                    "\n            vbView.BufferLocation = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(vbNodeBase)) << "buffer_" << vbNode.name << "->GetGPUVirtualAddress();"
                    "\n            vbView.StrideInBytes = (context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(vbNodeBase)) << "buffer_" << vbNode.name << "_format == DXGI_FORMAT_UNKNOWN)"
                    "\n                ? context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(vbNodeBase)) << "buffer_" << vbNode.name << "_stride"
                    "\n                : DX12Utils::Get_DXGI_FORMAT_Info(context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(vbNodeBase)) << "buffer_" << vbNode.name << "_format, Context::LogFn).bytesPerPixel;"
                    "\n            vbView.SizeInBytes = vbView.StrideInBytes * context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(vbNodeBase)) << "buffer_" << vbNode.name << "_count;"
                    "\n"
                    "\n            commandList->IASetVertexBuffers(0, 1, &vbView);"
                    ;
            }
        }
        if (!hasVertexBuffer)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n            int vertexCountPerInstance = " << node.countPerInstance << ";"
                ;
        }
    }

    // Handle the index buffer
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            // Index Buffer"
            ;
        bool hasIndexBuffer = false;
        if (node.indexBuffer.resourceNodeIndex != -1)
        {
            const RenderGraphNode& ibNodeBase = renderGraph.nodes[node.indexBuffer.resourceNodeIndex];
            if (ibNodeBase._index == RenderGraphNode::c_index_resourceBuffer)
            {
                hasIndexBuffer = true;
                const RenderGraphNode_Resource_Buffer& ibNode = ibNodeBase.resourceBuffer;

                if (node.countPerInstance == -1)
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n            int indexCountPerInstance = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_count;"
                        ;
                }
                else
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n            int indexCountPerInstance = min(" << node.countPerInstance << ", context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_count);"
                        ;
                }

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            D3D12_INDEX_BUFFER_VIEW ibView;"
                    "\n            ibView.BufferLocation = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "->GetGPUVirtualAddress();"
                    "\n            ibView.SizeInBytes = DX12Utils::Get_DXGI_FORMAT_Info(context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_format, Context::LogFn).bytesPerPixel * context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_count;"
                    "\n            ibView.Format = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_format;"
                    "\n"
                    "\n            commandList->IASetIndexBuffer(&ibView);"
                    ;
            }
        }
        if (!hasIndexBuffer)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n            int indexCountPerInstance = " << node.countPerInstance << ";"
                ;
        }
    }

    // Handle the instance buffer
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            // Instance Buffer"
            ;
        bool hasInstanceBuffer = false;

        if (node.instanceBuffer.resourceNodeIndex != -1)
        {
            const RenderGraphNode& ibNodeBase = renderGraph.nodes[node.instanceBuffer.resourceNodeIndex];
            if (ibNodeBase._index == RenderGraphNode::c_index_resourceBuffer)
            {
                hasInstanceBuffer = true;
                const RenderGraphNode_Resource_Buffer& ibNode = ibNodeBase.resourceBuffer;

                if (node.instanceCount == -1)
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n            int instanceCount = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_count;"
                        ;
                }
                else
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n            int instanceCount = min(" << node.instanceCount << ", context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_count);"
                        ;
                }

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            D3D12_VERTEX_BUFFER_VIEW ivbView;"
                    "\n            ivbView.BufferLocation = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "->GetGPUVirtualAddress();"
                    "\n            ivbView.StrideInBytes = (context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_format == DXGI_FORMAT_UNKNOWN)"
                    "\n                ? context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_stride"
                    "\n                : DX12Utils::Get_DXGI_FORMAT_Info(context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_format, Context::LogFn).bytesPerPixel;"
                    "\n            ivbView.SizeInBytes = ivbView.StrideInBytes * context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(ibNodeBase)) << "buffer_" << ibNode.name << "_count;"
                    "\n"
                    "\n            commandList->IASetVertexBuffers(1, 1, &ivbView);"
                    ;
            }
        }

        if (!hasInstanceBuffer)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n            int instanceCount = " << node.instanceCount << ";"
                ;
        }
    }

    // clear any color targets that need clearing
    for (int i = 0; i < node.colorTargets.size(); ++i)
    {
        if (node.colorTargets[i].resourceNodeIndex == -1)
            break;

        const ColorTargetSettings& ctSettings = node.colorTargetSettings[i];
        if (!ctSettings.clear)
            continue;

        const RenderGraphNode& colorTargetNodeBase = renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
        if (colorTargetNodeBase._index != RenderGraphNode::c_index_resourceTexture)
            break;

        const RenderGraphNode_Resource_Texture colorTargetNode = colorTargetNodeBase.resourceTexture;

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            // Clear " << colorTargetNode.name <<
            "\n            {"
            "\n                 float clearValues[4] = { " << std::fixed << ctSettings.clearColor[0] << "f, " << ctSettings.clearColor[1] << "f, " << ctSettings.clearColor[2] << "f, " << ctSettings.clearColor[3] << "f };"
            "\n                 commandList->ClearRenderTargetView(s_heapAllocationTrackerRTV.GetCPUHandle(context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(colorTargetNodeBase)) << "texture_" << colorTargetNode.name << "_rtv), clearValues, 0, nullptr);"
            "\n            }"
            ;
    }

    // Clear depth target if we should
    if (node.depthTargetClear || node.stencilClear)
    {
        if (node.depthTarget.resourceNodeIndex != -1)
        {
            const RenderGraphNode& depthTargetNodeBase = renderGraph.nodes[node.depthTarget.resourceNodeIndex];
            if (depthTargetNodeBase._index == RenderGraphNode::c_index_resourceTexture)
            {
                const RenderGraphNode_Resource_Texture depthTargetNode = depthTargetNodeBase.resourceTexture;

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            // Clear " << depthTargetNode.name <<
                    "\n            {"
                    "\n                D3D12_CLEAR_FLAGS clearFlags = ";
                    ;

                if (node.depthTargetClear && node.stencilClear)
                    stringReplacementMap["/*$(Execute)*/"] << "D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;";
                else if (node.depthTargetClear)
                    stringReplacementMap["/*$(Execute)*/"] << "D3D12_CLEAR_FLAG_DEPTH;";
                else if (node.stencilClear)
                    stringReplacementMap["/*$(Execute)*/"] << "D3D12_CLEAR_FLAG_STENCIL;";

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n                commandList->ClearDepthStencilView(s_heapAllocationTrackerDSV.GetCPUHandle(context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depthTargetNodeBase)) << "texture_" << depthTargetNode.name << "_dsv), clearFlags, " << std::fixed << node.depthTargetClearValue << "f, " << (unsigned int)node.stencilClearValue << ", 0, nullptr);"
                    "\n            }"
                    ;
            }
        }
    }

    // Gather the color target handles, and raster size
    {
        bool first = true;
        for (int i = 0; i < node.colorTargets.size(); ++i)
        {
            if (node.colorTargets[i].resourceNodeIndex == -1)
                break;

            const RenderGraphNode& colorTargetNodeBase = renderGraph.nodes[node.colorTargets[i].resourceNodeIndex];
            if (colorTargetNodeBase._index != RenderGraphNode::c_index_resourceTexture)
                break;

            const RenderGraphNode_Resource_Texture colorTargetNode = colorTargetNodeBase.resourceTexture;

            if (first)
            {
                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            int renderWidth = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(colorTargetNodeBase)) << "texture_" << colorTargetNode.name << "_size[0];"
                    "\n            int renderHeight = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(colorTargetNodeBase)) << "texture_" << colorTargetNode.name << "_size[1];"
                    "\n"
                    "\n            D3D12_CPU_DESCRIPTOR_HANDLE colorTargetHandles[] ="
                    "\n            {"
                    ;
                first = false;
            }

            stringReplacementMap["/*$(Execute)*/"] <<
                "\n                s_heapAllocationTrackerRTV.GetCPUHandle(context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(colorTargetNodeBase)) << "texture_" << colorTargetNode.name << "_rtv),"
                ;
        }
        if (first)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            int renderWidth = 0;"
                "\n            int renderHeight = 0;"
                "\n"
                "\n            D3D12_CPU_DESCRIPTOR_HANDLE* colorTargetHandles = nullptr;"
                "\n"
                "\n            int colorTargetHandleCount = 0;"
                ;
        }
        else
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n            };"
                "\n"
                "\n            int colorTargetHandleCount = _countof(colorTargetHandles);"
                ;
        }
    }

    // Get the depth target if it exists
    {
        bool hasDepth = false;
        if (node.depthTarget.resourceNodeIndex != -1)
        {
            const RenderGraphNode& depthTargetNodeBase = renderGraph.nodes[node.depthTarget.resourceNodeIndex];
            if (depthTargetNodeBase._index == RenderGraphNode::c_index_resourceTexture)
            {
                const RenderGraphNode_Resource_Texture depthTargetNode = depthTargetNodeBase.resourceTexture;
                hasDepth = true;

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            D3D12_CPU_DESCRIPTOR_HANDLE depthTargetHandle = s_heapAllocationTrackerDSV.GetCPUHandle(context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depthTargetNodeBase)) << "texture_" << depthTargetNode.name << "_dsv);"
                    "\n            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = &depthTargetHandle;"
                    ;
            }
        }
        if (!hasDepth)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            D3D12_CPU_DESCRIPTOR_HANDLE* depthTargetHandlePtr = nullptr;"
                ;
        }
    }

    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n            // clear viewport and scissor rect"
        "\n            D3D12_VIEWPORT viewport = { 0.0f, 0.0f, float(renderWidth), float(renderHeight), 0.0f, 1.0f };"
        "\n            D3D12_RECT scissorRect = { 0, 0, (LONG)renderWidth, (LONG)renderHeight };"
        "\n            commandList->RSSetViewports(1, &viewport);"
        "\n            commandList->RSSetScissorRects(1, &scissorRect);"
        "\n"
        "\n            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);"
        "\n            commandList->OMSetStencilRef(" << (unsigned int)node.stencilRef << ");"
        "\n"
        "\n            commandList->OMSetRenderTargets(colorTargetHandleCount, colorTargetHandles, false, depthTargetHandlePtr);"
        ;

    // If we have an index buffer, do DrawIndexedInstanced
    if (node.indexBuffer.resourceNodeIndex != -1)
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, 0, 0, 0);"
            ;
    }
    // else do DrawInstanced
    else
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            commandList->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);"
            ;
    }


    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n            if(context->m_profile)"
        "\n            {"
        "\n                context->m_profileData[(s_timerIndex-1)/2].m_label = \"" << node.name << "\";"
        "\n                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();"
        "\n                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);"
        "\n            }"
        "\n        }";

    if (node.condition.comparison != ConditionComparison::Count)
    {
        stringReplacementMap["/*$(ReadbackProfileData)*/"] <<
            "\n        if(" << ConditionToString(node.condition, renderGraph, &VariableToStringInsideContext) << ")"
            "\n        {"
            "\n            m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: " << node.name <<
            "\n        }";
    }
    else
    {
        stringReplacementMap["/*$(ReadbackProfileData)*/"] <<
            "\n        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // Draw Call: " << node.name;
    }
}
