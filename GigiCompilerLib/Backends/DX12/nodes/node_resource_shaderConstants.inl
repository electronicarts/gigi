///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, RenderGraphNode_Resource_ShaderConstants& node)
{
    if (!ResourceNodeIsUsed(node))
        return;

    stringReplacementMap["/*$(ContextInternal)*/"] << "\n";

    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(ContextInternal)*/"] <<
            "\n        // " << node.comment;
    }

    // Storage
    stringReplacementMap["/*$(ContextInternal)*/"] <<
        "\n        Struct_" << renderGraph.structs[node.structure.structIndex].name << " constantBuffer_" << node.name << "_cpu;"
        "\n        ID3D12Resource* constantBuffer_" << node.name << " = nullptr;"
    ;

    // Creation
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

    size_t sizeAligned = ALIGN(256, renderGraph.structs[node.structure.structIndex].sizeInBytes);
    stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
        "\n        if (m_internal.constantBuffer_" << node.name << " == nullptr)"
        "\n        {"
        "\n            dirty = true;"
        "\n            m_internal.constantBuffer_" << node.name << " = DX12Utils::CreateBuffer(device, " << sizeAligned << ", D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L\"" << (node.name) << "\" : nullptr), Context::LogFn);"
        "\n        }"
    ;

    // Destruction

    stringReplacementMap["/*$(ContextDestructor)*/"] <<
        "\n"
        "\n        // " << node.name
    ;

    if (!node.comment.empty())
    {
        stringReplacementMap["/*$(ContextDestructor)*/"] <<
            "\n        // " << node.comment
        ;
    }

    stringReplacementMap["/*$(ContextDestructor)*/"] <<
        "\n        if (m_internal.constantBuffer_" << node.name << ")"
        "\n        {"
        "\n            s_delayedRelease.Add(m_internal.constantBuffer_" << node.name << ");"
        "\n            m_internal.constantBuffer_" << node.name << " = nullptr;"
        "\n        }"
    ;

    // Execute
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n"
        "\n        // Shader Constants: " << node.name <<
        "\n        {";

    for (const SetCBFromVar& setFromVar : node.setFromVar)
    {
        const char* cast = (renderGraph.variables[setFromVar.variable.variableIndex].enumIndex == -1) ? "" : "(int)";

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n            context->m_internal.constantBuffer_" << node.name << "_cpu." << setFromVar.field << " = " << cast <<
            VariableToString(renderGraph.variables[setFromVar.variable.variableIndex], renderGraph) << ";";
    }

    // copy the constants to the GPU
    stringReplacementMap["/*$(Execute)*/"] <<
        "\n            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer_" << node.name << ", context->m_internal.constantBuffer_" << node.name << "_cpu, Context::LogFn);"
        "\n        }";
}
