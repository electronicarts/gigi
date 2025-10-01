///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_ComputeShader& node)
{
    // Storage
    stringReplacementMap["/*$(ContextInternal)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(ContextInternal)*/"] <<
            "\n        // " << node.comment;
    }

    stringReplacementMap["/*$(ContextInternal)*/"] <<
        "\n        static ID3D12PipelineState* computeShader_" << node.name << "_pso;"
        "\n        static ID3D12RootSignature* computeShader_" << node.name << "_rootSig;"
    ;

    stringReplacementMap["/*$(StaticVariables)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(StaticVariables)*/"] <<
            "\n    // " << node.comment;
    }

    stringReplacementMap["/*$(StaticVariables)*/"] <<
        "\n    ID3D12PipelineState* ContextInternal::computeShader_" << node.name << "_pso = nullptr;"
        "\n    ID3D12RootSignature* ContextInternal::computeShader_" << node.name << "_rootSig = nullptr;"
    ;

    // Creation
    stringReplacementMap["/*$(CreateShared)*/"] << "\n";
    stringReplacementMap["/*$(CreateShared)*/"] << "\n        // Compute Shader: " << node.name;
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n        // " << node.comment;
    }

    // Samplers
    int samplerCount = (int)node.shader.shader->samplers.size();
    if (samplerCount == 0)
        stringReplacementMap["/*$(CreateShared)*/"] << "\n        {\n            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;";
    else
        stringReplacementMap["/*$(CreateShared)*/"] << "\n        {\n            D3D12_STATIC_SAMPLER_DESC samplers[" << samplerCount << "];";

    for (size_t samplerIndex = 0; samplerIndex < node.shader.shader->samplers.size(); ++samplerIndex)
    {
        ShaderSampler& sampler = node.shader.shader->samplers[samplerIndex];

        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n"
            "\n            // " << sampler.name <<
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
    }

    // Descriptor table
    int descriptorTableRangeCount = (int)node.shader.shader->resources.size();
    if (descriptorTableRangeCount == 0)
        stringReplacementMap["/*$(CreateShared)*/"] << "\n\n            D3D12_DESCRIPTOR_RANGE* ranges = nullptr;";
    else
        stringReplacementMap["/*$(CreateShared)*/"] << "\n\n            D3D12_DESCRIPTOR_RANGE ranges[" << descriptorTableRangeCount << "];";

    for (size_t descriptorTableRangeIndex = 0; descriptorTableRangeIndex < node.shader.shader->resources.size(); ++descriptorTableRangeIndex)
    {
        ShaderResource& resource = node.shader.shader->resources[descriptorTableRangeIndex];

        stringReplacementMap["/*$(CreateShared)*/"] << "\n\n            // " << resource.name;

        switch (resource.access)
        {
            case ShaderResourceAccessType::UAV: stringReplacementMap["/*$(CreateShared)*/"] << "\n            ranges[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;"; break;
            case ShaderResourceAccessType::RTScene:
            case ShaderResourceAccessType::SRV: stringReplacementMap["/*$(CreateShared)*/"] << "\n            ranges[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;"; break;
            case ShaderResourceAccessType::CBV: stringReplacementMap["/*$(CreateShared)*/"] << "\n            ranges[" << descriptorTableRangeIndex << "].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;"; break;
            default:
            {
                Assert(false, "Unhandled resource access type: %i", resource.access);
            }
        }

        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n            ranges[" << descriptorTableRangeIndex << "].NumDescriptors = 1;"
            "\n            ranges[" << descriptorTableRangeIndex << "].BaseShaderRegister = " << resource.registerIndex << ";"
            "\n            ranges[" << descriptorTableRangeIndex << "].RegisterSpace = 0;"
            "\n            ranges[" << descriptorTableRangeIndex << "].OffsetInDescriptorsFromTableStart = " << descriptorTableRangeIndex << ";"
        ;
    }

    // Root signature
    stringReplacementMap["/*$(CreateShared)*/"] <<
        "\n"
        "\n            if(!DX12Utils::MakeRootSig(device, ranges, " << descriptorTableRangeCount << ", samplers, " << samplerCount << ", &ContextInternal::computeShader_" << node.name << "_rootSig, (c_debugNames ? L\"" << node.name << "\" : nullptr), Context::LogFn))"
        "\n                return false;";

    // Shader compilation info
    stringReplacementMap["/*$(CreateShared)*/"] <<
        "\n"
        "\n            ShaderCompilationInfo shaderCompilationInfo;"
        "\n            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / \"shaders\" / \"" << node.shader.shader->destFileName << "\";"
        "\n            shaderCompilationInfo.entryPoint = \"" << node.shader.shader->entryPoint << "\";"
        "\n            shaderCompilationInfo.shaderModel = \"" << renderGraph.settings.dx12.shaderModelCs << "\";"
        "\n            shaderCompilationInfo.debugName = (c_debugNames ? \"" << (node.name) << "\" : \"\");"
        "\n            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;";

    if (renderGraph.settings.dx12.DXC_HLSL_2021)
    {
		stringReplacementMap["/*$(CreateShared)*/"] <<
        "\n            shaderCompilationInfo.flags |= ShaderCompilationFlags::HLSL2021;";
    }

    for (const ShaderDefine& define : node.shader.shader->defines)
    {
		stringReplacementMap["/*$(CreateShared)*/"] <<
        "\n            shaderCompilationInfo.defines.emplace_back(\"" << define.name << "\",\"" << define.value << "\");";
    }

    // PSO
    const char* shaderCompiler = (renderGraph.settings.dx12.shaderCompiler == DXShaderCompiler::DXC) ? "_DXC" : "_FXC";
    stringReplacementMap["/*$(CreateShared)*/"] <<
        "\n"
        "\n            if(!DX12Utils::MakeComputePSO" << shaderCompiler << "(device, shaderCompilationInfo,"
        "\n               ContextInternal::computeShader_" << node.name << "_rootSig, &ContextInternal::computeShader_" << node.name << "_pso, Context::LogFn))"
        "\n                return false;"
        "\n        }"
    ;

    // Destruction
    stringReplacementMap["/*$(DestroyShared)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(DestroyShared)*/"] <<
            "\n        // " << node.comment;
    }

    stringReplacementMap["/*$(DestroyShared)*/"] <<
        "\n        if(ContextInternal::computeShader_" << node.name << "_pso)"
        "\n        {"
        "\n            s_delayedRelease.Add(ContextInternal::computeShader_" << node.name << "_pso);"
        "\n            ContextInternal::computeShader_" << node.name << "_pso = nullptr;"
        "\n        }"
        "\n"
        "\n        if(ContextInternal::computeShader_" << node.name << "_rootSig)"
        "\n        {"
        "\n            s_delayedRelease.Add(ContextInternal::computeShader_" << node.name << "_rootSig);"
        "\n            ContextInternal::computeShader_" << node.name << "_rootSig = nullptr;"
        "\n        }"
    ;

    // Execute
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n        // Compute Shader: " << node.name
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
        "\n            ScopedPerfEvent scopedPerf(\"Compute Shader: " << node.name << "\", commandList, " << node.nodeIndex << ");"
        "\n            std::chrono::high_resolution_clock::time_point startPointCPU;"
        "\n            if(context->m_profile)"
        "\n            {"
        "\n                startPointCPU = std::chrono::high_resolution_clock::now();"
        "\n                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);"
        "\n            }"
        "\n"
        "\n            commandList->SetComputeRootSignature(ContextInternal::computeShader_" << node.name << "_rootSig);"
        "\n            commandList->SetPipelineState(ContextInternal::computeShader_" << node.name << "_pso);"
    ;

    // Handle getting and setting the descriptor table
    if (node.resourceDependencies.size() > 0)
    {
        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            DX12Utils::ResourceDescriptor descriptors[] = {"
        ;

        int depCount = 0;
        for (size_t depIndex = 0; depIndex < node.resourceDependencies.size(); ++depIndex)
        {
            ResourceDependency& dep = node.resourceDependencies[depIndex];
            if (dep.access == ShaderResourceAccessType::Indirect)
                continue;

            unsigned int bufferViewBegin = 0;
            unsigned int bufferViewSize = 0;
            bool bufferViewInBytes = false;
            int UAVMipIndex = 0;
            if (dep.pinIndex < node.linkProperties.size())
            {
                const LinkProperties& linkProperties = node.linkProperties[dep.pinIndex];
                UAVMipIndex = linkProperties.UAVMipIndex;
                bufferViewBegin = linkProperties.bufferViewBegin;
                bufferViewSize = linkProperties.bufferViewSize;
                bufferViewInBytes = (linkProperties.bufferViewUnits == MemoryUnitOfMeasurement::Bytes);
            }

            RenderGraphNode depNode = renderGraph.nodes[dep.nodeIndex];

            if (depCount > 0)
                stringReplacementMap["/*$(Execute)*/"] << ",";
            depCount++;

            const char* resourceTypeString = "";
            std::ostringstream rawAndStrideAndCount;
            switch (dep.type)
            {
                case ShaderResourceType::ConstantBuffer:
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
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
                    if (node.shader.shader->resources[depIndex].access == ShaderResourceAccessType::RTScene)
                    {
                        stringReplacementMap["/*$(Execute)*/"] <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << ", " <<
                            "DXGI_FORMAT_UNKNOWN,"
                            ;
                        resourceTypeString = "DX12Utils::ResourceType::RTScene";
                        rawAndStrideAndCount << ", false, context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_tlasSize, 1";
                    }
                    else
                    {
                        stringReplacementMap["/*$(Execute)*/"] <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << ", " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_format,"
                            ;
                        resourceTypeString = "DX12Utils::ResourceType::Buffer";
                        rawAndStrideAndCount << ", " << (node.shader.shader->resources[depIndex].buffer.raw ? "true" : "false") << ", context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_stride, context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_count";
                    }
                    break;
                }
                case ShaderResourceType::Texture:
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n                { " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << ", " <<
                        "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_format,"
                        ;

                    rawAndStrideAndCount << ", false, 0, ";
                    switch (node.shader.shader->resources[depIndex].texture.dimension)
                    {
                        case TextureDimensionType::Texture2D: resourceTypeString = "DX12Utils::ResourceType::Texture2D"; rawAndStrideAndCount << "0"; break;
                        case TextureDimensionType::Texture2DArray: resourceTypeString = "DX12Utils::ResourceType::Texture2DArray"; rawAndStrideAndCount << "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_size[2]"; break;
                        case TextureDimensionType::Texture3D: resourceTypeString = "DX12Utils::ResourceType::Texture3D"; rawAndStrideAndCount << "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "texture_" << GetNodeName(depNode) << "_size[2]"; break;
                        case TextureDimensionType::TextureCube: resourceTypeString = "DX12Utils::ResourceType::TextureCube"; rawAndStrideAndCount << "6"; break;
                        default:
                        {
                            Assert(false, "Unhandled TextureDimensionType: %s (%i)", EnumToString(node.shader.shader->resources[depIndex].texture.dimension), (int)node.shader.shader->resources[depIndex].texture.dimension);
                            break;
                        }
                    }

                    break;
                }
                default:
                {
                    Assert(false, "Unhandled resource type");
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

            stringReplacementMap["/*$(Execute)*/"] << accessType << resourceTypeString << rawAndStrideAndCount.str() << ", " << UAVMipIndex << ", " << bufferViewBegin << ", " << bufferViewSize << ", " << ( bufferViewInBytes ? "true" : "false" ) << " }";
        }

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n            };"
            "\n"
            "\n            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, " << depCount << ", Context::LogFn);"
            "\n            commandList->SetComputeRootDescriptorTable(0, descriptorTable);"
        ;
    }

    // Indirect dispatch
    if (node.dispatchSize.indirectBuffer.nodeIndex != -1)
    {
        int indirectBufferResourceNodeIndex = node.dispatchSize.indirectBuffer.nodeIndex;
        if (!GetNodeIsResourceNode(renderGraph.nodes[indirectBufferResourceNodeIndex]))
            indirectBufferResourceNodeIndex = GetResourceNodeForPin(renderGraph, node.dispatchSize.indirectBuffer.nodeIndex, node.dispatchSize.indirectBuffer.nodePinIndex);

        Assert(indirectBufferResourceNodeIndex != -1, "Could not find resource node for indirect dispatch");
        Assert(renderGraph.nodes[indirectBufferResourceNodeIndex]._index == RenderGraphNode::c_index_resourceBuffer, "Error");
        RenderGraphNode_Resource_Buffer& bufferNode = renderGraph.nodes[indirectBufferResourceNodeIndex].resourceBuffer;

        if (node.dispatchSize.indirectOffsetVariable.variableIndex != -1)
        {
            const Variable& variable = renderGraph.variables[node.dispatchSize.indirectOffsetVariable.variableIndex];
            if (variable.type == DataFieldType::Int)
            {
                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            int executeIndirectOffset = " << VariableToString(variable, renderGraph) << ";"
                    ;
            }
            else
            {
                Assert(false, "Unhandled data type \"%s\" for Indirect Offset variable \"%s\" in compute shader node \"%s\"", EnumToString(variable.type), variable.name.c_str(), node.name.c_str());
            }
        }
        else
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            int executeIndirectOffset = " << node.dispatchSize.indirectOffsetValue << ";"
                ;
        }

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            commandList->ExecuteIndirect("
            "\n                ContextInternal::s_commandSignatureDispatch,"
            "\n                1,"
            "\n                context->" << GetResourceNodePathInContext(bufferNode.visibility) << "buffer_" << bufferNode.name.c_str() << ","
            "\n                executeIndirectOffset * 4 * 4, // byte offset.  *4 because sizeof(UINT) is 4.  *4 again because of 4 items per dispatch."
            "\n                nullptr,"
            "\n                0);";
    }
    else
    {
        // Get dispatch size
        if (node.dispatchSize.node.textureNode)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            unsigned int baseDispatchSize[3] = {" <<
                "\n                context->" << GetResourceNodePathInContext(node.dispatchSize.node.textureNode->visibility) <<
                "texture_" << node.dispatchSize.node.textureNode->name.c_str() << "_size[0]," <<
                "\n                context->" << GetResourceNodePathInContext(node.dispatchSize.node.textureNode->visibility) <<
                "texture_" << node.dispatchSize.node.textureNode->name.c_str() << "_size[1]," <<
                "\n                context->" << GetResourceNodePathInContext(node.dispatchSize.node.textureNode->visibility) <<
                "texture_" << node.dispatchSize.node.textureNode->name.c_str() << "_size[2]" <<
                "\n            };"
                ;
        }
        else if (node.dispatchSize.node.bufferNode)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            unsigned int baseDispatchSize[3] = { context->" << GetResourceNodePathInContext(node.dispatchSize.node.bufferNode->visibility) << "buffer_" << node.dispatchSize.node.bufferNode->name.c_str() << "_count, 1, 1 };"
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
                        "\n"
                        "\n            unsigned int baseDispatchSize[3] = { (unsigned int)" << VariableToString(var, renderGraph) << ", 1, 1 };";
                    break;
                }
                case 2:
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n"
                        "\n            unsigned int baseDispatchSize[3] = { (unsigned int)" << VariableToString(var, renderGraph) << "[0], (unsigned int)" << VariableToString(var, renderGraph) << "[1], 1 };";
                    break;
                }
                case 3:
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n"
                        "\n            unsigned int baseDispatchSize[3] = { (unsigned int)" << VariableToString(var, renderGraph) << "[0], (unsigned int)" << VariableToString(var, renderGraph) << "[1], (unsigned int)" << VariableToString(var, renderGraph) << "[2] };";
                    break;
                }
                default:
                {
                    Assert(false, "Inappropriate variable type given for dispatch size.");
                }
            }
        }
        else
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            unsigned int baseDispatchSize[3] = { 1, 1, 1 };"
                ;
        }

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            unsigned int dispatchSize[3] = {" <<
            "\n                " << "(((baseDispatchSize[0] + " << node.dispatchSize.preAdd[0] << ") * " << node.dispatchSize.multiply[0] << ") / " <<
            node.dispatchSize.divide[0] << " + " << node.dispatchSize.postAdd[0] << " + " << node.shader.shader->NumThreads[0] << " - 1) / " << node.shader.shader->NumThreads[0] << ","
            "\n                (((baseDispatchSize[1] + " << node.dispatchSize.preAdd[1] << ") * " << node.dispatchSize.multiply[1] << ") / " <<
            node.dispatchSize.divide[1] << " + " << node.dispatchSize.postAdd[1] << " + " << node.shader.shader->NumThreads[1] << " - 1) / " << node.shader.shader->NumThreads[1] << ","
            "\n                (((baseDispatchSize[2] + " << node.dispatchSize.preAdd[2] << ") * " << node.dispatchSize.multiply[2] << ") / " <<
            node.dispatchSize.divide[2] << " + " << node.dispatchSize.postAdd[2] << " + " << node.shader.shader->NumThreads[2] << " - 1) / " << node.shader.shader->NumThreads[2] <<
            "\n            };"
            "\n"
            "\n            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);";
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
            "\n            m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: " << node.name <<
            "\n        }";
    }
    else
    {
        stringReplacementMap["/*$(ReadbackProfileData)*/"] <<
            "\n        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: " << node.name;
    }
}
