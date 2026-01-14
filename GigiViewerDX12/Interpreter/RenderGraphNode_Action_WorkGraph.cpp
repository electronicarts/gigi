///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"
#include "DX12Utils/CompileShaders.h"

#include <d3dx12/d3dx12_state_object.h>

void RuntimeTypes::RenderGraphNode_Action_WorkGraph::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
    RenderGraphNode_Base::Release(interpreter);

    if (m_stateObject)
    {
        interpreter.m_delayedRelease.Add(m_stateObject);
        m_stateObject = nullptr;
    }

    if (m_rootSignature)
    {
        interpreter.m_delayedRelease.Add(m_rootSignature);
        m_rootSignature = nullptr;
    }

    if (m_backingMemory)
    {
        interpreter.m_delayedRelease.Add(m_backingMemory);
        m_backingMemory = nullptr;
    }
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Action_WorkGraph& node, RuntimeTypes::RenderGraphNode_Action_WorkGraph& runtimeData, NodeAction nodeAction)
{
    ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, false);

    if (nodeAction == NodeAction::Init)
    {
        // check support:
        if (m_dx12_options21.WorkGraphsTier == D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED)
        {
            m_logFn(LogLevel::Error, "Work Graphs aren't supported on your device");
            return false;
        }

        // Select shader file name
        const auto shaderFileName = node.entryShader;

        // make the root signature
        {
            // shader samplers
            std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
            int samplerIndex = -1;
            for (const ShaderSampler& sampler : node.entryShader.shader->samplers)
            {
                samplerIndex++;
                D3D12_STATIC_SAMPLER_DESC desc;

                if (!SamplerFilterToD3D12Filter(sampler.filter, desc.Filter))
                {
                    m_logFn(LogLevel::Error, "Shader \"%s\" has an invalid sampler filter for sampler %i: \"%s\"", node.entryShader.shader->name.c_str(), samplerIndex, EnumToString(sampler.filter));
                    return false;
                }

                if (!SamplerAddressModeToD3D12AddressMode(sampler.addressMode, desc.AddressU))
                {
                    m_logFn(LogLevel::Error, "Shader \"%s\" has an invalid address mode for sampler %i: \"%s\"", node.entryShader.shader->name.c_str(), samplerIndex, EnumToString(sampler.addressMode));
                    return false;
                }

                desc.AddressV = desc.AddressW = desc.AddressU;

                desc.MipLODBias = 0;
                desc.MaxAnisotropy = 0;
                desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
                desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                desc.MinLOD = 0.0f;
                desc.MaxLOD = D3D12_FLOAT32_MAX;
                desc.ShaderRegister = sampler.registerIndex;
                desc.RegisterSpace = 0;
                desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                samplers.push_back(desc);
            }

            // Descriptor table
            std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
            for (const ShaderResource& resource : node.entryShader.shader->resources)
            {
                D3D12_DESCRIPTOR_RANGE desc;

                switch (resource.access)
                {
                case ShaderResourceAccessType::UAV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
                case ShaderResourceAccessType::RTScene:
                case ShaderResourceAccessType::SRV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
                case ShaderResourceAccessType::CBV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
                default:
                {
                    m_logFn(LogLevel::Error, "Shader \"%s\" unhandled resource access: \"%s\"", node.entryShader.shader->name.c_str(), EnumToString(resource.access));
                    return false;
                }
                }

                desc.NumDescriptors = 1;
                desc.BaseShaderRegister = resource.registerIndex;
                desc.RegisterSpace = 0;
                desc.OffsetInDescriptorsFromTableStart = (UINT)ranges.size();

                ranges.push_back(desc);
            }

            // Root parameter
            D3D12_ROOT_PARAMETER rootParam;
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParam.DescriptorTable.NumDescriptorRanges = (UINT)ranges.size();
            rootParam.DescriptorTable.pDescriptorRanges = ranges.data();

            D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
            rootDesc.NumParameters = (ranges.size() > 0) ? 1 : 0;
            rootDesc.pParameters = &rootParam;
            rootDesc.NumStaticSamplers = (UINT)samplers.size();
            rootDesc.pStaticSamplers = samplers.data();
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

            // Create it
            ID3DBlob* sig = nullptr;
            ID3DBlob* error = nullptr;
            HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    m_logFn(LogLevel::Error, "Could not serialize root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            OnRootSignature(sig, node.entryShader.shader);
            hr = m_device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&runtimeData.m_rootSignature));
            if (FAILED(hr))
            {
                const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
                if (errorMsg)
                    m_logFn(LogLevel::Error, "Could not create root signature: %s", errorMsg);
                if (sig) sig->Release();
                if (error) error->Release();
                return false;
            }

            if (sig)
                sig->Release();

            if (error)
                error->Release();

            // name the root signature for debuggers
            runtimeData.m_rootSignature->SetName(ToWideString(node.name.c_str()).c_str());
        }

        // Create work graph
        CD3DX12_STATE_OBJECT_DESC stateObjectDesc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

        // set root signature for work graph
        auto rootSignatureSubobject = stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        rootSignatureSubobject->SetRootSignature(runtimeData.m_rootSignature);

        auto workgraphSubobject = stateObjectDesc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
        workgraphSubobject->IncludeAllAvailableNodes();

        std::wstring programName = ToWideString(node.name.c_str());
        workgraphSubobject->SetProgramName(programName.c_str());

        // shader defines
        std::string fullFileName = (std::filesystem::path(GetTempDirectory()) / "shaders" / node.entryShader.shader->destFileName).string();
        std::string entrypoint = node.entryPoint.empty() ? node.entryShader.shader->entryPoint : node.entryPoint;

        ShaderCompilationInfo shaderCompilationInfo;

        shaderCompilationInfo.sourceFileName = node.entryShader.shader->fileName;

        shaderCompilationInfo.rootDirectory = m_renderGraph.baseDirectory;
        StringReplaceAll(shaderCompilationInfo.rootDirectory, "\\", "/");
        if (!shaderCompilationInfo.rootDirectory.empty() && shaderCompilationInfo.rootDirectory.back() == '/')
            shaderCompilationInfo.rootDirectory.pop_back();

        shaderCompilationInfo.fileName = fullFileName;
        shaderCompilationInfo.entryPoint = entrypoint;
        shaderCompilationInfo.shaderModel = m_renderGraph.settings.dx12.shaderModelWg;
        shaderCompilationInfo.debugName = node.name;
        shaderCompilationInfo.defines = node.entryShader.shader->defines;

        if (!node.defines.empty())
        {
            shaderCompilationInfo.defines.insert(shaderCompilationInfo.defines.end(), node.defines.begin(), node.defines.end());
        }

        if (m_compileShadersForDebug)
        {
            shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
        }

        if (m_renderGraph.settings.dx12.Allow16BitTypes && m_dx12_options4.Native16BitShaderOpsSupported)
            shaderCompilationInfo.flags |= ShaderCompilationFlags::Enable16BitTypes;

        if (m_renderGraph.settings.dx12.DXC_HLSL_2021)
        {
            shaderCompilationInfo.flags |= ShaderCompilationFlags::HLSL2021;
        }

        if (m_renderGraph.settings.common.shaderWarningAsErrors)
        {
            shaderCompilationInfo.flags |= ShaderCompilationFlags::WarningsAsErrors;
        }

        if (m_renderGraph.settings.common.createPDBsAndBinaries)
        {
            shaderCompilationInfo.flags |= ShaderCompilationFlags::CreatePDBsAndBinaries;
        }

        std::vector<std::string> allShaderFiles;
        std::vector<unsigned char> blob = CompileShaderToByteCode_dxc(shaderCompilationInfo, m_logFn, &allShaderFiles);

		// Watch the shader file source for file changes, even if it failed compilation, so we can detect when it's edited and try again
		for (const std::string& fileName : allShaderFiles)
		{
			std::string sourceFileName = (std::filesystem::path(m_renderGraph.baseDirectory) / std::filesystem::proximate(fileName, std::filesystem::path(GetTempDirectory()) / "shaders")).string();
			m_fileWatcher.Add(sourceFileName.c_str(), FileWatchOwner::Shaders);
		}

		if (blob.data() == nullptr)
		{
			return false;
		}

        // Add library to graph
        {
            auto shaderBytecode = CD3DX12_SHADER_BYTECODE(blob.data(), blob.size());

            // add blob to state object
            auto librarySubobject = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            librarySubobject->SetDXILLibrary(&shaderBytecode);
        }

        // Create work graph state object
        HRESULT hr = m_device->CreateStateObject(stateObjectDesc, IID_PPV_ARGS(&runtimeData.m_stateObject));
        if (FAILED(hr))
        {
            m_logFn(LogLevel::Error, "Could not CreateStateObject");
            return false;
        }

        // get program desc and entrypoint index
            // Get work graph properties
        ID3D12StateObjectProperties1* stateObjectProperties;
        ID3D12WorkGraphProperties*    workGraphProperties;

        hr = runtimeData.m_stateObject->QueryInterface(IID_PPV_ARGS(&stateObjectProperties));
        if (FAILED(hr))
        {
            m_logFn(LogLevel::Error, "Could not QueryInterface for stateObjectProperties");
            return false;
        }

        hr = runtimeData.m_stateObject->QueryInterface(IID_PPV_ARGS(&workGraphProperties));
        if (FAILED(hr))
        {
            m_logFn(LogLevel::Error, "Could not QueryInterface for workGraphProperties");
            return false;
        }

        // Get the index of our work graph inside the state object (state object can contain multiple work graphs)
        const auto workGraphIndex = workGraphProperties->GetWorkGraphIndex(programName.c_str());
        if (workGraphIndex == -1)
        {
            m_logFn(LogLevel::Error, "WorkGraph with this entrypoint could not be found. are you sure the entry function is tagged with [NodeIsProgramEntry]");
            return false;
        }

        // Create backing memory buffer
        // See https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#getworkgraphmemoryrequirements
        D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS memoryRequirements = {};
        workGraphProperties->GetWorkGraphMemoryRequirements(workGraphIndex, &memoryRequirements);

        // Work graphs can also request no backing memory (i.e., MaxSizeInBytes = 0)
        runtimeData.m_backingMemorySizeBytes = (uint32_t)memoryRequirements.MaxSizeInBytes;
        if (runtimeData.m_backingMemorySizeBytes > 0) {
            CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC   resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(runtimeData.m_backingMemorySizeBytes,
                                                                               D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            m_device->CreateCommittedResource(&heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COMMON,
                NULL,
                IID_PPV_ARGS(&runtimeData.m_backingMemory));
        }

        // Prepare work graph desc
        // See https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#d3d12_set_program_desc
        runtimeData.m_programDesc.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
        runtimeData.m_programDesc.WorkGraph.ProgramIdentifier = stateObjectProperties->GetProgramIdentifier(programName.c_str());
        runtimeData.m_programDesc.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
        // Set backing memory
        if (runtimeData.m_backingMemory) {
            runtimeData.m_programDesc.WorkGraph.BackingMemory.StartAddress = runtimeData.m_backingMemory->GetGPUVirtualAddress();
            runtimeData.m_programDesc.WorkGraph.BackingMemory.SizeInBytes = runtimeData.m_backingMemory->GetDesc().Width;
        }
        else
        {
            runtimeData.m_programDesc.WorkGraph.BackingMemory.StartAddress = 0;
            runtimeData.m_programDesc.WorkGraph.BackingMemory.SizeInBytes = 0;
        }

        runtimeData.m_entrypointIndex = workGraphProperties->GetEntrypointIndex(workGraphIndex, { ToWideString(entrypoint.c_str()).c_str(), 0});
        runtimeData.m_recordStrideInBytes = workGraphProperties->GetEntrypointRecordSizeInBytes(workGraphIndex, runtimeData.m_entrypointIndex);

        workGraphProperties->Release();
        stateObjectProperties->Release();
    }
    else if (nodeAction == NodeAction::Execute)
    {
        // check support:
        if (m_dx12_options21.WorkGraphsTier == D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED || m_graphicsCommandList10 == nullptr)
        {
            m_logFn(LogLevel::Error, "Work Graphs aren't supported on your device");
            return false;
        }

        // If we aren't supposed to do the work graph, exit out
        if (!EvaluateCondition(node.condition))
            return true;

        // Queue up transitions, so we only do them if we actually execute the node
        std::vector<TransitionTracker::Item> queuedTransitions;

        // Make the GPU input for dispatching the entry point node of the work graph
        D3D12_NODE_GPU_INPUT gpuInputCPU = {};
        {
            gpuInputCPU.EntrypointIndex = runtimeData.m_entrypointIndex;

            // If there is no input record to the entry point node, use node.numRecords
            if (runtimeData.m_recordStrideInBytes == 0)
            {
                gpuInputCPU.NumRecords = GetRuntimeVariableValueAllowCast_NoFail<bool>(node.numRecords.variable.variableIndex);
            }
            // Otherwise, fill in the dispatch data using the records buffer
            if (node.records.resourceNodeIndex != -1)
            {
                const RenderGraphNode& resourceNode = m_renderGraph.nodes[node.records.resourceNodeIndex];
                if (resourceNode._index == RenderGraphNode::c_index_resourceBuffer)
                {
                    bool exists = false;
                    const auto& bufferInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str(), exists);
                    if (exists && bufferInfo.m_resource)
                    {
                        unsigned int bufferViewBegin = 0;
                        unsigned int bufferViewSize = 0;
                        bool bufferViewInBytes = false;

                        int pinIdx = node.records.nodePinIndex; // TODO: jan test if works
                        if (pinIdx < node.linkProperties.size())
                        {
                            const LinkProperties& linkProperties = node.linkProperties[pinIdx];
                            bufferViewBegin = linkProperties.bufferViewBegin;
                            bufferViewSize = linkProperties.bufferViewSize;
                            bufferViewInBytes = linkProperties.bufferViewUnits == MemoryUnitOfMeasurement::Bytes;
                        }

                        const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
                        runtimeData.HandleViewableBuffer(*this, (node.name + std::string(".recordsBuffer")).c_str()
                            , resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count
                            , false, false, bufferViewBegin, bufferViewSize, bufferViewInBytes);

                        // transition
                        queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(bufferInfo.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, GetNodeName(resourceNode).c_str()) });

                        if (node.records.resourceNodeIndex != -1 && runtimeData.m_recordStrideInBytes > 0)
                        {
                            const RenderGraphNode& resourceNode = m_renderGraph.nodes[node.records.resourceNodeIndex];
                            if (resourceNode._index == RenderGraphNode::c_index_resourceBuffer)
                            {
                                bool exists = false;
                                const auto& bufferInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str(), exists);
                                if (exists)
                                {
                                    gpuInputCPU.NumRecords = bufferInfo.m_size / runtimeData.m_recordStrideInBytes;
                                    gpuInputCPU.Records.StrideInBytes = runtimeData.m_recordStrideInBytes;
                                    gpuInputCPU.Records.StartAddress = bufferInfo.m_resource->GetGPUVirtualAddress();
                                }
                            }
                        }
                    }
                }
            }
        }

        // publish shader resources as viewable resources, before the shader execution
        int depIndex = -1;
        for (const ResourceDependency& dep : node.resourceDependencies)
        {
            if (dep.access == ShaderResourceAccessType::Indirect)
                continue;

            depIndex++;
            const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];

            const Shader* shader = nullptr;
            std::string shaderResourceName;
            int shaderBasePinIndex = 0;
            {
                int pinIndex = dep.pinIndex;

                if (pinIndex >= 0)
                {
                    if (node.entryShader.shader && pinIndex < node.entryShader.shader->resources.size())
                    {
                        shader = node.entryShader.shader;
                        shaderResourceName = shader->resources[pinIndex].name;
                        pinIndex = -1;
                    }
                    else if (node.entryShader.shader)
                    {
                        pinIndex -= (int)node.entryShader.shader->resources.size();
                        shaderBasePinIndex += (int)node.entryShader.shader->resources.size();
                    }
                }
            }

            if (!shader)
                continue;

            switch (resourceNode._index)
            {
            case RenderGraphNode::c_index_resourceTexture:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceTexture.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - Before)";
                else
                    label = label + " (SRV)";

                const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
                runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_size, resourceInfo.m_numMips, false, false);
                break;
            }
            case RenderGraphNode::c_index_resourceShaderConstants:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceShaderConstants.name;
                label = label + " (CBV)";

                const RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_ShaderConstants(resourceNode.resourceShaderConstants.name.c_str());
                runtimeData.HandleViewableConstantBuffer(*this, label.c_str(), resourceInfo.m_buffer->buffer, (int)resourceInfo.m_buffer->size, shader->resources[dep.pinIndex - shaderBasePinIndex].constantBufferStructIndex, false, false);
                break;
            }
            case RenderGraphNode::c_index_resourceBuffer:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceBuffer.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - Before)";
                else
                    label = label + " (SRV)";

                unsigned int bufferViewBegin = 0;
                unsigned int bufferViewSize = 0;
                bool bufferViewInBytes = false;
                if (dep.pinIndex < node.linkProperties.size())
                {
                    const LinkProperties& linkProperties = node.linkProperties[dep.pinIndex];
                    bufferViewBegin = linkProperties.bufferViewBegin;
                    bufferViewSize = linkProperties.bufferViewSize;
                    bufferViewInBytes = linkProperties.bufferViewUnits == MemoryUnitOfMeasurement::Bytes;
                }

                const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
                runtimeData.HandleViewableBuffer(*this, label.c_str()
                    , resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count
                    , false, false, bufferViewBegin, bufferViewSize, bufferViewInBytes);
                break;
            }
            }
        }

        m_commandList->SetComputeRootSignature(runtimeData.m_rootSignature);

        // Make the entry shader descriptor table gigi description
        std::vector<DescriptorTableCache::ResourceDescriptor> descriptorsWorkGraph;
        int descriptorTablePinOffset = 0;
        if (node.entryShader.shader)
        {
            // If a resource is used as a vertex buffer and srv, it should transition to only the vertex buffer state.
            // This map is meant to resolve situations like this. but is not used for base compute work graphs.
            // mesh nodes might need this to be properly filled out though.
            std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> importantResourceStates;

            if (!MakeDescriptorTableDesc(descriptorsWorkGraph,runtimeData, node.resourceDependencies, node.linkProperties, node.name.c_str(), node.entryShader.shader, descriptorTablePinOffset, queuedTransitions, importantResourceStates))
                return false;
            descriptorTablePinOffset += (int)node.entryShader.shader->resources.size();
        }

        // Do all resource transitions desired by the descriptor tables
        m_transitions.Transition(queuedTransitions);
        m_transitions.Flush(m_commandList);

        // Get or make the entry shader descriptor table and set it
        int rootSigParamIndex = 0;
        if (descriptorsWorkGraph.size() > 0)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableWorkGraph;
            std::string error;
            if (!m_descriptorTableCache.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker, descriptorsWorkGraph.data(), (int)descriptorsWorkGraph.size(), descriptorTableWorkGraph, error, HEAP_DEBUG_TEXT()))
            {
                m_logFn(LogLevel::Error, "Work Graph Node \"%s\" could not allocate a descriptor table for VS: %s", node.name.c_str(), error.c_str());
                return false;
            }

            // todo jan: will need to be graphics when mesh nodes
            m_commandList->SetComputeRootDescriptorTable(rootSigParamIndex, descriptorTableWorkGraph);
            rootSigParamIndex++;
        }

        std::ostringstream ss;
        ss << "DispatchGraph:\n  " << node.name << "\n  NumRecords: " << gpuInputCPU.NumRecords << "\n  Backing Memory Bytes: " << runtimeData.m_backingMemorySizeBytes << "\n  Record Stride Bytes: " << runtimeData.m_recordStrideInBytes;

        // Make an upload buffer holding the gpu input dispatch information
        UploadBufferTracker::Buffer* gpuInputGPU = m_uploadBufferTracker.GetBufferT(m_device, false, gpuInputCPU);

        D3D12_DISPATCH_GRAPH_DESC dispatchDesc = {};
        dispatchDesc.Mode = D3D12_DISPATCH_MODE_NODE_GPU_INPUT;
        dispatchDesc.NodeGPUInput = gpuInputGPU->buffer->GetGPUVirtualAddress();

        // Set program and dispatch the work graphs.
        // See
        // https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#setprogram
        // https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#dispatchgraph

        m_graphicsCommandList10->SetProgram(&runtimeData.m_programDesc);
        m_graphicsCommandList10->DispatchGraph(&dispatchDesc);

        // Clear backing memory initialization flag, as the graph has run at least once now
        // See https://microsoft.github.io/DirectX-Specs/d3d/WorkGraphs.html#d3d12_set_work_graph_flags
        runtimeData.m_programDesc.WorkGraph.Flags &= ~D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;

        if (IsConditional(node.condition))
            ss << "\nCondition: " << (EvaluateCondition(node.condition) ? "true" : "false");
        runtimeData.m_renderGraphText = ss.str();

        // publish shader resources as viewable resources, after the shader execution
        depIndex = -1;
        for (const ResourceDependency& dep : node.resourceDependencies)
        {
            if (dep.access == ShaderResourceAccessType::Indirect)
                continue;

            depIndex++;
            const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];

            const Shader* shader = nullptr;
            std::string shaderResourceName;
            int shaderBasePinIndex = 0;
            {
                int pinIndex = dep.pinIndex;

                if (pinIndex >= 0)
                {
                    if (node.entryShader.shader && pinIndex < node.entryShader.shader->resources.size())
                    {
                        shader = node.entryShader.shader;
                        shaderResourceName = shader->resources[pinIndex].name;
                        pinIndex = -1;
                    }
                    else if (node.entryShader.shader)
                    {
                        pinIndex -= (int)node.entryShader.shader->resources.size();
                        shaderBasePinIndex += (int)node.entryShader.shader->resources.size();
                    }
                }
            }

            if (!shader)
                continue;

            switch (resourceNode._index)
            {
            case RenderGraphNode::c_index_resourceTexture:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceTexture.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - After)";
                else
                    continue;

                const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
                runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(resourceNode.resourceTexture.dimension), label.c_str(), resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_size, resourceInfo.m_numMips, false, true);
                break;
            }
            case RenderGraphNode::c_index_resourceBuffer:
            {
                std::string label = node.name + std::string(".") + shaderResourceName + std::string(": ") + resourceNode.resourceBuffer.name;
                if (dep.access == ShaderResourceAccessType::UAV)
                    label = label + " (UAV - After)";
                else
                    continue;

                unsigned int bufferViewBegin = 0;
                unsigned int bufferViewSize = 0;
                bool bufferViewInBytes = false;
                if (dep.pinIndex < node.linkProperties.size())
                {
                    const LinkProperties& linkProperties = node.linkProperties[dep.pinIndex];
                    bufferViewBegin = linkProperties.bufferViewBegin;
                    bufferViewSize = linkProperties.bufferViewSize;
                    bufferViewInBytes = linkProperties.bufferViewUnits == MemoryUnitOfMeasurement::Bytes;
                }

                const RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());
                runtimeData.HandleViewableBuffer(*this, label.c_str()
                    , resourceInfo.m_resource, resourceInfo.m_format, resourceInfo.m_formatCount, resourceInfo.m_structIndex, resourceInfo.m_size, resourceInfo.m_stride, resourceInfo.m_count
                    , false, true, bufferViewBegin, bufferViewSize, bufferViewInBytes);
                break;
            }
            }
        }

        return true;
    }
    return true;
}