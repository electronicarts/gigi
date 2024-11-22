///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Action_RayShader& node)
{
    // gather all the shaders involved
    struct ShaderExport
    {
        const Shader* shader = nullptr;
        std::string fileName;
        std::string entryPoint;
        std::string uniqueName;
        ShaderType shaderType;
    };
    std::vector<ShaderExport> shaderExports;

    // gather all the shaders involved
    for (const Shader& shader : renderGraph.shaders)
    {
        if (shader.type != ShaderType::RTClosestHit && shader.type != ShaderType::RTMiss &&
            shader.type != ShaderType::RTAnyHit && shader.type != ShaderType::RTIntersection)
            continue;

        // If this shader is referenced as RTMissIndex, include it
        bool includeShader = false;
        for (const std::string& RTMissShader : node.shader.shader->Used_RTMissIndex)
        {
            if (!_stricmp(shader.name.c_str(), RTMissShader.c_str()))
            {
                includeShader = true;
                break;
            }
        }

        // If this shader is part of an RTHitGroupIndex, include it
        if (!includeShader)
        {
            for (const std::string& RTHitGroupName : node.shader.shader->Used_RTHitGroupIndex)
            {
                int hitGroupIndex = GetHitGroupIndex(renderGraph, RTHitGroupName.c_str());
                if (hitGroupIndex >= 0)
                {
                    const RTHitGroup& hitGroup = renderGraph.hitGroups[hitGroupIndex];
                    if (!_stricmp(shader.name.c_str(), hitGroup.closestHit.name.c_str()) ||
                        !_stricmp(shader.name.c_str(), hitGroup.anyHit.name.c_str()) ||
                        !_stricmp(shader.name.c_str(), hitGroup.intersection.name.c_str()))
                    {
                        includeShader = true;
                        break;
                    }
                }
            }
        }

        if (!includeShader)
            continue;

        ShaderExport newExport;
        newExport.shader = &shader;
        newExport.fileName = shader.destFileName;
        newExport.entryPoint = shader.entryPoint;
        newExport.shaderType = shader.type;
        shaderExports.push_back(newExport);
    }
    shaderExports.push_back({ node.shader.shader, node.shader.shader->destFileName, (node.entryPoint.empty() ? node.shader.shader->entryPoint : node.entryPoint), "", ShaderType::RTRayGen });

    // give each shader export a unique name
    for (size_t i = 0; i < shaderExports.size(); ++i)
    {
        char uniqueName[256];
        sprintf_s(uniqueName, "%s_%i", shaderExports[i].entryPoint.c_str(), (int)i);
        shaderExports[i].uniqueName = uniqueName;
    }

    // Storage
    stringReplacementMap["/*$(ContextInternal)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(ContextInternal)*/"] <<
            "\n        // " << node.comment;
    }

    stringReplacementMap["/*$(ContextInternal)*/"] <<
        "\n        static ID3D12StateObject* rayShader_" << node.name << "_rtso;"
        "\n        static ID3D12RootSignature* rayShader_" << node.name << "_rootSig;"
        "\n        static ID3D12Resource* rayShader_" << node.name << "_shaderTableRayGen;"
        "\n        static unsigned int    rayShader_" << node.name << "_shaderTableRayGenSize;"
        "\n        static ID3D12Resource* rayShader_" << node.name << "_shaderTableMiss;"
        "\n        static unsigned int    rayShader_" << node.name << "_shaderTableMissSize;"
        "\n        static ID3D12Resource* rayShader_" << node.name << "_shaderTableHitGroup;"
        "\n        static unsigned int    rayShader_" << node.name << "_shaderTableHitGroupSize;"
        ;

    stringReplacementMap["/*$(StaticVariables)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(StaticVariables)*/"] <<
            "\n    // " << node.comment;
    }

    stringReplacementMap["/*$(StaticVariables)*/"] <<
        "\n    ID3D12StateObject* ContextInternal::rayShader_" << node.name << "_rtso = nullptr;"
        "\n    ID3D12RootSignature* ContextInternal::rayShader_" << node.name << "_rootSig = nullptr;"
        "\n    ID3D12Resource* ContextInternal::rayShader_" << node.name << "_shaderTableRayGen = nullptr;"
        "\n    unsigned int    ContextInternal::rayShader_" << node.name << "_shaderTableRayGenSize = 0;"
        "\n    ID3D12Resource* ContextInternal::rayShader_" << node.name << "_shaderTableMiss = nullptr;"
        "\n    unsigned int    ContextInternal::rayShader_" << node.name << "_shaderTableMissSize = 0;"
        "\n    ID3D12Resource* ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup = nullptr;"
        "\n    unsigned int    ContextInternal::rayShader_" << node.name << "_shaderTableHitGroupSize = 0;"
        ;

    // Creation
    stringReplacementMap["/*$(CreateShared)*/"] << "\n";
    stringReplacementMap["/*$(CreateShared)*/"] << "\n        // Ray Shader: " << node.name;
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n        // " << node.comment;
    }

    // Describe Root Signature Samplers
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

    // Describe Root Signature Parameters
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

    // Create Root signature
    stringReplacementMap["/*$(CreateShared)*/"] <<
        "\n"
        "\n            if(!DX12Utils::MakeRootSig(device, ranges, " << descriptorTableRangeCount << ", samplers, " << samplerCount << ", &ContextInternal::rayShader_" << node.name << "_rootSig, (c_debugNames ? L\"" << node.name << "\" : nullptr), Context::LogFn))"
        "\n                return false;";

    // Describe State object
    {
        // shader defines, including built in ones
        {
            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n            D3D_SHADER_MACRO defines[] = {";

            for (const ShaderDefine& define : node.defines)
            {
                stringReplacementMap["/*$(CreateShared)*/"] <<
                    "\n                { \"" << define.name << "\", \"" << define.value << "\" },";
            }

            for (const ShaderDefine& define : node.shader.shader->defines)
            {
                stringReplacementMap["/*$(CreateShared)*/"] <<
                    "\n                { \"" << define.name << "\", \"" << define.value << "\" },";
            }

            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n                { \"MAX_RECURSION_DEPTH\", \"" << node.maxRecursionDepth << "\" },"
                "\n                { \"RT_HIT_GROUP_COUNT\", \"" << node.shader.shader->Used_RTHitGroupIndex.size() << "\" },"
                "\n                { nullptr, nullptr }"
                "\n            };";
        }

        // compile shaders
        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n"
            "\n            // Compile shaders"
            "\n            std::vector<unsigned char> shaderCode[" << shaderExports.size() << "];";

        const char* shaderCompiler = (renderGraph.settings.dx12.shaderCompiler == DXShaderCompiler::DXC) ? "_DXC" : "_FXC";
        for (size_t i = 0; i < shaderExports.size(); ++i)
        {
            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n            // Compile " << EnumToString(shaderExports[i].shaderType) << " : " << shaderExports[i].fileName << " " << shaderExports[i].entryPoint << "()"
                "\n            shaderCode[" << i << "] = DX12Utils::CompileShaderToByteCode" << shaderCompiler << "(Context::s_techniqueLocation.c_str(), L\"shaders/" << shaderExports[i].fileName << "\", \"\", \"lib_6_3\", defines, c_debugShaders, Context::LogFn);"
                "\n            if (shaderCode[" << i << "].empty())"
                "\n                return false;";
        }

        // We'll build the sub objects in a temporary string and keep a count, so we can define storage after the fact.
        std::ostringstream stateObjectCreation;
        int subObjectCount = 0;

        // dxil library desc
        for (size_t i = 0; i < shaderExports.size(); ++i)
        {
            const ShaderExport& shaderExport = shaderExports[i];

            stateObjectCreation <<
                "\n"
                "\n            // DXIL Library for " << EnumToString(shaderExports[i].shaderType) << " : " << shaderExports[i].fileName << " " << shaderExports[i].entryPoint << "()"
                "\n            {"
                "\n                static D3D12_EXPORT_DESC exportDesc;"
                "\n                exportDesc.Name = L\"" << shaderExport.uniqueName << "\";"
                "\n                exportDesc.ExportToRename = L\"" << shaderExport.entryPoint << "\";"
                "\n                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;"
                "\n"
                "\n                static D3D12_DXIL_LIBRARY_DESC libDesc;"
                "\n                libDesc.DXILLibrary.BytecodeLength = shaderCode[" << i << "].size();"
                "\n                libDesc.DXILLibrary.pShaderBytecode = shaderCode[" << i << "].data();"
                "\n                libDesc.NumExports = 1;"
                "\n                libDesc.pExports = &exportDesc;"
                "\n"
                "\n                subObjects[" << subObjectCount << "].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;"
                "\n                subObjects[" << subObjectCount << "].pDesc = &libDesc;"
                "\n            }";
            subObjectCount++;
        }

        // Hit Groups
        if (node.shader.shader->Used_RTHitGroupIndex.size() > 0)
        {
            stateObjectCreation <<
                "\n"
                "\n            // Make the hit group sub objects"
                "\n            D3D12_HIT_GROUP_DESC hitGroupDescs[" << node.shader.shader->Used_RTHitGroupIndex.size() << "];"
                ;

            for (size_t hgi = 0; hgi < node.shader.shader->Used_RTHitGroupIndex.size(); ++hgi)
            {
                int hitGroupIndex = GetHitGroupIndex(renderGraph, node.shader.shader->Used_RTHitGroupIndex[hgi].c_str());
                if (hitGroupIndex < 0)
                    continue;

                const RTHitGroup& hitGroup = renderGraph.hitGroups[hitGroupIndex];
                stateObjectCreation <<
                    "\n"
                    "\n            // Hit group: " << hitGroup.name <<
                    "\n            {"
                    "\n                D3D12_HIT_GROUP_DESC& hitGroupDesc = hitGroupDescs[" << hgi << "];"
                    "\n                hitGroupDesc.HitGroupExport = L\"hitgroup" << hgi << "\";"
                    ;

                // Any hit Shader Import
                {
                    bool found = false;
                    for (const ShaderExport& shaderExport : shaderExports)
                    {
                        if (shaderExport.shader == hitGroup.anyHit.shader)
                        {
                            found = true;
                            stateObjectCreation << "\n                hitGroupDesc.AnyHitShaderImport = L\"" << shaderExport.uniqueName << "\";";
                            break;
                        }
                    }
                    if (!found)
                        stateObjectCreation << "\n                hitGroupDesc.AnyHitShaderImport = nullptr;";
                }

                // Closest hit Shader Import
                {
                    bool found = false;
                    for (const ShaderExport& shaderExport : shaderExports)
                    {
                        if (shaderExport.shader == hitGroup.closestHit.shader)
                        {
                            found = true;
                            stateObjectCreation << "\n                hitGroupDesc.ClosestHitShaderImport = L\"" << shaderExport.uniqueName << "\";";
                            break;
                        }
                    }
                    if (!found)
                        stateObjectCreation << "\n                hitGroupDesc.ClosestHitShaderImport = nullptr;";
                }

                // Intersection Shader Import
                {
                    bool found = false;
                    for (const ShaderExport& shaderExport : shaderExports)
                    {
                        if (shaderExport.shader == hitGroup.intersection.shader)
                        {
                            found = true;
                            stateObjectCreation << "\n                hitGroupDesc.IntersectionShaderImport = L\"" << shaderExport.uniqueName << "\";";
                            stateObjectCreation << "\n                hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;";
                            break;
                        }
                    }
                    if (!found)
                    {
                        stateObjectCreation << "\n                hitGroupDesc.IntersectionShaderImport = nullptr;";
                        stateObjectCreation << "\n                hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;";
                    }
                }

                stateObjectCreation <<
                    "\n                subObjects[" << subObjectCount << "].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;"
                    "\n                subObjects[" << subObjectCount << "].pDesc = &hitGroupDesc;"
                    "\n            }"
                    ;

                subObjectCount++;
            }
        }

        // Payload
        stateObjectCreation <<
            "\n"
            "\n            // Payload"
            "\n            D3D12_RAYTRACING_SHADER_CONFIG payloadDesc;"
            "\n            payloadDesc.MaxPayloadSizeInBytes = " << node.rayPayloadSize << ";"
            "\n            payloadDesc.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;"
            "\n"
            "\n            subObjects[" << subObjectCount << "].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;"
            "\n            subObjects[" << subObjectCount << "].pDesc = &payloadDesc;";
        subObjectCount++;

        // Associate payload with shaders
        stateObjectCreation <<
            "\n"
            "\n            // Associate payload with shaders"
            "\n            const WCHAR* shaderExports[] = { ";

        for (size_t i = 0; i < shaderExports.size(); ++i)
        {
            if (i < shaderExports.size() - 1)
                stateObjectCreation << "L\"" << shaderExports[i].uniqueName << "\", ";
            else
                stateObjectCreation << "L\"" << shaderExports[i].uniqueName << "\" };";
        }

        stateObjectCreation <<
            "\n"
            "\n            D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};"
            "\n            shaderPayloadAssociation.NumExports = " << shaderExports.size() << ";"
            "\n            shaderPayloadAssociation.pExports = shaderExports;"
            "\n            shaderPayloadAssociation.pSubobjectToAssociate = &subObjects[" << subObjectCount - 1 << "];"
            "\n"
            "\n            subObjects[" << subObjectCount << "].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;"
            "\n            subObjects[" << subObjectCount << "].pDesc = &shaderPayloadAssociation;";
        subObjectCount++;

        // pipeline config
        stateObjectCreation <<
            "\n"
            "\n            // Pipeline Config"
            "\n            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;"
            "\n            pipelineConfig.MaxTraceRecursionDepth = " << node.maxRecursionDepth << ";"
            "\n"
            "\n            subObjects[" << subObjectCount << "].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;"
            "\n            subObjects[" << subObjectCount << "].pDesc = &pipelineConfig;";
        subObjectCount++;

        // Global Root Signature
        stateObjectCreation <<
            "\n"
            "\n            // Global Root Signature"
            "\n            subObjects[" << subObjectCount << "].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;"
            "\n            subObjects[" << subObjectCount << "].pDesc = &ContextInternal::rayShader_" << node.name << "_rootSig;";
        subObjectCount++;

        // The root object
        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n"
            "\n            // Make the state object"
            "\n            D3D12_STATE_SUBOBJECT subObjects[" << subObjectCount << "];"
            "\n"
            "\n            D3D12_STATE_OBJECT_DESC soDesc;"
            "\n            soDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;"
            "\n            soDesc.NumSubobjects = " << subObjectCount << ";"
            "\n            soDesc.pSubobjects = subObjects;" <<
            stateObjectCreation.str();

        // make shader tables begin
        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n            if (FAILED(dxrDevice->CreateStateObject(&soDesc, IID_PPV_ARGS(&ContextInternal::rayShader_" << node.name << "_rtso))))"
            "\n                return false;"
            "\n"
            "\n            if (c_debugNames)"
            "\n                ContextInternal::rayShader_" << node.name << "_rtso->SetName(L\"" << node.name << " state object\");"
            "\n"
            "\n            // Create the shader tables"
            "\n            {"
            "\n                ID3D12StateObjectProperties* soprops = nullptr;"
            "\n                if(FAILED(ContextInternal::rayShader_" << node.name << "_rtso->QueryInterface(IID_PPV_ARGS(&soprops))))"
            "\n                    return false;"
            ;

        // Get the count for the number of items in each shader table
        int shaderTableMissCount = 0;
        for (const ShaderExport& shaderExport : shaderExports)
        {
            if (shaderExport.shaderType == ShaderType::RTMiss)
                shaderTableMissCount++;
        }
        int shaderTableRaygenCount = 1;
        int shaderTableHitGroupCount = (int)node.shader.shader->Used_RTHitGroupIndex.size(); // How many hit groups used by this shader

        // Ray Gen Shader Table
        {
            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n                // make the ray gen shader table and fill it out"
                "\n                {"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableRayGenSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, (unsigned int)(" << shaderTableRaygenCount << " * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableRayGen = DX12Utils::CreateBuffer(device, ContextInternal::rayShader_" << node.name << "_shaderTableRayGenSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (c_debugNames ? L\"" << node.name << " shader table ray gen\" : nullptr), nullptr);"
                "\n"
                "\n                    unsigned char* shaderTableBytes = nullptr;"
                "\n                    D3D12_RANGE readRange = { 0, 0 };"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableRayGen->Map(0, &readRange, (void**)&shaderTableBytes);"
                "\n"
                ;

            for (size_t index = 0; index < shaderExports.size(); ++index)
            {
                if (shaderExports[index].shaderType != ShaderType::RTRayGen)
                    continue;

                stringReplacementMap["/*$(CreateShared)*/"] <<
                    "\n                    memcpy(shaderTableBytes, soprops->GetShaderIdentifier(L\"" << shaderExports[index].uniqueName << "\"), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);"
                    "\n                    shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;"
                    ;
            }

            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableRayGen->Unmap(0, nullptr);"
                "\n                }"
                ;
        }

        // Miss Shader Table
        if (node.shader.shader->Used_RTMissIndex.size() > 0)
        {
            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n                // make the miss shader table and fill it out"
                "\n                {"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableMissSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, (unsigned int)(" << shaderTableMissCount << " * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableMiss = DX12Utils::CreateBuffer(device, ContextInternal::rayShader_" << node.name << "_shaderTableMissSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (c_debugNames ? L\"" << node.name << " shader table miss\" : nullptr), nullptr);"
                "\n"
                "\n                    unsigned char* shaderTableBytes = nullptr;"
                "\n                    D3D12_RANGE readRange = { 0, 0 };"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableMiss->Map(0, &readRange, (void**)&shaderTableBytes);"
                "\n"
                ;

            for (size_t index = 0; index < shaderExports.size(); ++index)
            {
                if (shaderExports[index].shaderType != ShaderType::RTMiss)
                    continue;

                stringReplacementMap["/*$(CreateShared)*/"] <<
                    "\n                    memcpy(shaderTableBytes, soprops->GetShaderIdentifier(L\"" << shaderExports[index].uniqueName << "\"), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);"
                    "\n                    shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;"
                    ;
            }

            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableMiss->Unmap(0, nullptr);"
                "\n                }"
                ;
        }

        // Hit Group Table
        if (node.shader.shader->Used_RTHitGroupIndex.size() > 0)
        {
            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n                // make the hit group shader table and fill it out"
                "\n                {"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableHitGroupSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, (unsigned int)(" << shaderTableHitGroupCount << " * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup = DX12Utils::CreateBuffer(device, ContextInternal::rayShader_" << node.name << "_shaderTableHitGroupSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (c_debugNames ? L\"" << node.name << " shader table hit group\" : nullptr), nullptr);"
                "\n"
                "\n                    unsigned char* shaderTableBytes = nullptr;"
                "\n                    D3D12_RANGE readRange = { 0, 0 };"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup->Map(0, &readRange, (void**)&shaderTableBytes);"
                "\n"
                ;

            for (int index = 0; index < node.shader.shader->Used_RTHitGroupIndex.size(); ++index)
            {
                stringReplacementMap["/*$(CreateShared)*/"] <<
                    "\n                    memcpy(shaderTableBytes, soprops->GetShaderIdentifier(L\"hitgroup" << index << "\"), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);"
                    "\n                    shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;"
                    ;
            }

            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n                    ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup->Unmap(0, nullptr);"
                "\n                }"
                ;
        }

        // make shader tables end
        stringReplacementMap["/*$(CreateShared)*/"] <<
            "\n"
            "\n                soprops->Release();"
            "\n            }"
            ;
    }

    stringReplacementMap["/*$(CreateShared)*/"] <<
        "\n        }";

    // Destruction
    stringReplacementMap["/*$(DestroyShared)*/"] << "\n";
    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(DestroyShared)*/"] <<
            "\n        // " << node.comment;
    }

    stringReplacementMap["/*$(DestroyShared)*/"] <<
        "\n        if(ContextInternal::rayShader_" << node.name << "_rtso)"
        "\n        {"
        "\n            s_delayedRelease.Add(ContextInternal::rayShader_" << node.name << "_rtso);"
        "\n            ContextInternal::rayShader_" << node.name << "_rtso = nullptr;"
        "\n        }"
        "\n"
        "\n        if(ContextInternal::rayShader_" << node.name << "_rootSig)"
        "\n        {"
        "\n            s_delayedRelease.Add(ContextInternal::rayShader_" << node.name << "_rootSig);"
        "\n            ContextInternal::rayShader_" << node.name << "_rootSig = nullptr;"
        "\n        }"
        "\n"
        "\n        if(ContextInternal::rayShader_" << node.name << "_shaderTableRayGen)"
        "\n        {"
        "\n            s_delayedRelease.Add(ContextInternal::rayShader_" << node.name << "_shaderTableRayGen);"
        "\n            ContextInternal::rayShader_" << node.name << "_shaderTableRayGen = nullptr;"
        "\n        }"
        "\n"
        "\n        if(ContextInternal::rayShader_" << node.name << "_shaderTableMiss)"
        "\n        {"
        "\n            s_delayedRelease.Add(ContextInternal::rayShader_" << node.name << "_shaderTableMiss);"
        "\n            ContextInternal::rayShader_" << node.name << "_shaderTableMiss = nullptr;"
        "\n        }"
        "\n"
        "\n        if(ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup)"
        "\n        {"
        "\n            s_delayedRelease.Add(ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup);"
        "\n            ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup = nullptr;"
        "\n        }"
        ;

    // Execute
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n        // Ray Shader: " << node.name
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
        "\n            ScopedPerfEvent scopedPerf(\"Ray Shader: " << node.name << "\", commandList, " << node.nodeIndex << ");"
        "\n            std::chrono::high_resolution_clock::time_point startPointCPU;"
        "\n            if(context->m_profile)"
        "\n            {"
        "\n                startPointCPU = std::chrono::high_resolution_clock::now();"
        "\n                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);"
        "\n            }"
        "\n"
        "\n            commandList->SetComputeRootSignature(ContextInternal::rayShader_" << node.name << "_rootSig);"
        "\n            dxrCommandList->SetPipelineState1(ContextInternal::rayShader_" << node.name << "_rtso);"
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

            int UAVMipIndex = 0;
            if (dep.pinIndex < node.linkProperties.size())
                UAVMipIndex = node.linkProperties[dep.pinIndex].UAVMipIndex;

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
                        resourceTypeString = "DX12Utils::ResourceType::RTScene";

                        stringReplacementMap["/*$(Execute)*/"] <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << ", " <<
                            "DXGI_FORMAT_UNKNOWN,"
                            ;
                        rawAndStrideAndCount << ", false, context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_tlasSize, 1";
                    }
                    else
                    {
                        resourceTypeString = "DX12Utils::ResourceType::Buffer";

                        stringReplacementMap["/*$(Execute)*/"] <<
                            "\n                { " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << ", " <<
                            "context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(depNode)) << "buffer_" << GetNodeName(depNode) << "_format,"
                            ;
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

            stringReplacementMap["/*$(Execute)*/"] << accessType << resourceTypeString << rawAndStrideAndCount.str() << ", " << UAVMipIndex << " }";
        }

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n            };"
            "\n"
            "\n            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, " << depCount << ", Context::LogFn);"
            "\n            commandList->SetComputeRootDescriptorTable(0, descriptorTable);"
        ;
    }

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
        "\n            D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};" <<
        "\n            dispatchDesc.Width = " << "((baseDispatchSize[0] + " << node.dispatchSize.preAdd[0] << ") * " << node.dispatchSize.multiply[0] << ") / " <<
        node.dispatchSize.divide[0] << " + " << node.dispatchSize.postAdd[0] << ";"
        "\n            dispatchDesc.Height = ((baseDispatchSize[1] + " << node.dispatchSize.preAdd[1] << ") * " << node.dispatchSize.multiply[1] << ") / " <<
        node.dispatchSize.divide[1] << " + " << node.dispatchSize.postAdd[1] << ";"
        "\n            dispatchDesc.Depth = ((baseDispatchSize[2] + " << node.dispatchSize.preAdd[2] << ") * " << node.dispatchSize.multiply[2] << ") / " <<
        node.dispatchSize.divide[2] << " + " << node.dispatchSize.postAdd[2] << ";";

    //if (runtimeData.m_shaderTableMiss)

    // write out the table addresses and size
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n            dispatchDesc.RayGenerationShaderRecord.StartAddress = ContextInternal::rayShader_" << node.name << "_shaderTableRayGen->GetGPUVirtualAddress();"
        "\n            dispatchDesc.RayGenerationShaderRecord.SizeInBytes = ContextInternal::rayShader_" << node.name << "_shaderTableRayGenSize;"
        "\n            if (ContextInternal::rayShader_" << node.name << "_shaderTableMiss)"
        "\n                dispatchDesc.MissShaderTable.StartAddress = ContextInternal::rayShader_" << node.name << "_shaderTableMiss->GetGPUVirtualAddress();"
        "\n            dispatchDesc.MissShaderTable.SizeInBytes = ContextInternal::rayShader_" << node.name << "_shaderTableMissSize;"
        "\n            dispatchDesc.MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;"
        "\n            if (ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup)"
        "\n                dispatchDesc.HitGroupTable.StartAddress = ContextInternal::rayShader_" << node.name << "_shaderTableHitGroup->GetGPUVirtualAddress();"
        "\n            dispatchDesc.HitGroupTable.SizeInBytes = ContextInternal::rayShader_" << node.name << "_shaderTableHitGroupSize;"
        "\n            dispatchDesc.HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;"
        "\n"
        "\n            dxrCommandList->DispatchRays(&dispatchDesc);"
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
            "\n            m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // ray shader: " << node.name <<
            "\n        }";
    }
    else
    {
        stringReplacementMap["/*$(ReadbackProfileData)*/"] <<
            "\n        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // ray shader: " << node.name;
    }
}
