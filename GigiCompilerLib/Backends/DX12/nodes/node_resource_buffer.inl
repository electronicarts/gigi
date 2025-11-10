///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

static void MakeStringReplacementForNode(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, const RenderGraphNode_Resource_Buffer& node)
{
    if (!ResourceNodeIsUsed(node))
        return;

    // imported resources are requred
    if (node.visibility == ResourceVisibility::Imported)
    {
        // If it's used as an RTScene, the TLAS needs to not be null
        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RTScene))
        {
            stringReplacementMap["/*$(ExecuteBegin)*/"] <<
                "\n"
                "\n        if (!context->" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_blas)"
                "\n        {"
                "\n            Context::LogFn(LogLevel::Error, \"" << renderGraph.name << ": Imported buffer \\\"" << node.name << "\\\" needs a BLAS/TLAS. Use CreateManagedTLAS to create them.\\n\");"
                "\n            return;"
                "\n        }"
                ;
        }

        stringReplacementMap["/*$(ExecuteBegin)*/"] <<
            "\n"
            "\n        if (!context->" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << ")"
            "\n        {"
            "\n            Context::LogFn(LogLevel::Error, \"" << renderGraph.name << ": Imported buffer \\\"" << node.name << "\\\" is null.\\n\");"
            "\n            return;"
            "\n        }"
            ;

        // If it's used as a vertex buffer, _vertexInputLayout need to be non null and _vertexInputLayoutCount needs to be non zero
        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::VertexBuffer))
        {
            stringReplacementMap["/*$(ExecuteBegin)*/"] <<
                "\n"
                "\n        if (context->" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_vertexInputLayout.size() == 0)"
                "\n        {"
                "\n            Context::LogFn(LogLevel::Error, \"" << renderGraph.name << ": Imported buffer \\\"" << node.name << "\\\" is used as a vertex buffer but no vertexInputLayout was given.\\n\");"
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
        case ResourceVisibility::Imported: location = "/*$(ContextInput)*/"; indent="            "; break;
        case ResourceVisibility::Internal: location = "/*$(ContextInternal)*/"; indent = "        "; break;
        case ResourceVisibility::Exported: location = "/*$(ContextOutput)*/"; indent = "            "; break;
    }

    {
        stringReplacementMap[location] << "\n";

        if (!node.comment.empty())
        {
            stringReplacementMap[location] <<
                "\n" << indent << "// " << node.comment;
        }

        stringReplacementMap[location] <<
            "\n" << indent << "ID3D12Resource* buffer_" << node.name << " = nullptr;"
            "\n" << indent << "DXGI_FORMAT buffer_" << node.name << "_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer"
            "\n" << indent << "unsigned int buffer_" << node.name << "_stride = 0; // For structured buffers, the size of the structure"
            "\n" << indent << "unsigned int buffer_" << node.name << "_count = 0; // How many items there are";

        switch (node.visibility)
        {
            case ResourceVisibility::Imported:
            {
                stringReplacementMap[location] << "\n" << indent << "D3D12_RESOURCE_STATES buffer_" << node.name << "_state = D3D12_RESOURCE_STATE_COMMON;";

                if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::VertexBuffer))
                    stringReplacementMap[location] << "\n" << indent << "std::vector<D3D12_INPUT_ELEMENT_DESC> buffer_" << node.name << "_vertexInputLayout;";
                break;
            }
            case ResourceVisibility::Exported:
            case ResourceVisibility::Internal:
            {
                stringReplacementMap[location] << "\n" << indent << "const D3D12_RESOURCE_STATES c_buffer_" << node.name << "_endingState = " << ShaderResourceTypeToDX12ResourceState(node.finalState) << ";";
                break;
            }
        }

        stringReplacementMap[location] <<
            "\n"
            "\n" << indent << "static const D3D12_RESOURCE_FLAGS c_buffer_" << node.name << "_flags = " << resourceFlags << "; // Flags the buffer needs to have been created with"
            ;

        if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RTScene))
        {
            stringReplacementMap[location] <<
                "\n"
                "\n" << indent << "// TLAS and BLAS information. Required for raytracing. Fill out using CreateManagedTLAS()."
                "\n" << indent << "// The resource itself is the tlas."
                "\n" << indent << "unsigned int buffer_" << node.name << "_tlasSize = 0;"
                "\n" << indent << "ID3D12Resource* buffer_" << node.name << "_blas = nullptr;"
                "\n" << indent << "unsigned int buffer_" << node.name << "_blasSize = 0;"
                ;
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

        // Get desired count
        if (node.count.node.bufferNode)
        {
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n            unsigned int baseCount = " << GetResourceNodePathInContext(node.count.node.bufferNode->visibility) << "buffer_" << node.count.node.bufferNode->name.c_str() << "_count;"
                ;
        }
        else if (node.count.variable.variableIndex != -1)
        {
            Variable& var = renderGraph.variables[node.count.variable.variableIndex];

            switch (DataFieldTypeComponentCount(var.type))
            {
                case 1:
                {
                    stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                        "\n"
                        "\n            unsigned int baseCount = (unsigned int)" << VariableToStringInsideContext(var, renderGraph) << ";";
                    break;
                }
                default:
                {
                    GigiAssert(false, "Inappropriate variable type given for buffer size.");
                }
            }

        }
        else
        {
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n            unsigned int baseCount = 1;"
            ;
        }

        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
            "\n            unsigned int desiredCount = " << "((baseCount + " << node.count.preAdd << " ) * " << node.count.multiply << ") / " << node.count.divide << " + " << node.count.postAdd << ";"
        ;

        // Get desired format and stride
        if (node.format.node.bufferNode)
        {
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n            DXGI_FORMAT desiredFormat = " << GetResourceNodePathInContext(node.format.node.bufferNode->visibility) << "buffer_" << node.format.node.bufferNode->name.c_str() << "_format;"
                "\n            unsigned int desiredStride = " << GetResourceNodePathInContext(node.format.node.bufferNode->visibility) << "buffer_" << node.format.node.bufferNode->name.c_str() << "_stride;"
                ;
        }
        else if (node.format.structureType.structIndex != -1)
        {
            stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                "\n            DXGI_FORMAT desiredFormat = DXGI_FORMAT_UNKNOWN;"
                "\n            unsigned int desiredStride = " << renderGraph.structs[node.format.structureType.structIndex].sizeInBytes << ";"
                ;
        }
        else
        {
            // convert our pod type to a regular field type.
            if (DataFieldTypeIsPOD(node.format.type))
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n            DXGI_FORMAT desiredFormat = " << DataFieldTypeToDXGIFormat(node.format.type) << ";"
                    "\n            unsigned int desiredStride = 0;";
            }
            else
            {
                stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
                    "\n            DXGI_FORMAT desiredFormat = DXGI_FORMAT_UNKNOWN;"
                    "\n            unsigned int desiredStride = " << DataFieldTypeToSize(node.format.type) << ";";
            }
        }

        // Recreate the buffer if it doesn't exist
        stringReplacementMap["/*$(EnsureResourcesCreated)*/"] <<
            "\n"
            "\n            if(!" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << " ||"
            "\n               " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_count != desiredCount ||"
            "\n               " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_format != desiredFormat ||"
            "\n               " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_stride != desiredStride)"
            "\n            {"
            "\n                dirty = true;"
            "\n                if(" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << ")"
            "\n                    s_delayedRelease.Add(" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << ");"
            "\n"
            "\n                unsigned int desiredSize = desiredCount * ((desiredStride > 0) ? desiredStride : DX12Utils::Get_DXGI_FORMAT_Info(desiredFormat, Context::LogFn).bytesPerPixel);"
            "\n"
            "\n                " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << " = DX12Utils::CreateBuffer(device, desiredSize, " << GetResourceNodePathInContext(node.visibility) << "c_buffer_" << node.name << "_flags"
            ", D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L\"" << (node.name) << "\" : nullptr), Context::LogFn);"
            "\n                " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_count = desiredCount;"
            "\n                " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_format = desiredFormat;"
            "\n                " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << "_stride = desiredStride;"
            "\n            }"
            "\n        }"

            // Buffer was created in state ShaderResourceTypeToDX12ResourceState(node.finalState), but buffers are created in common state.
        ;

        // destruction
        stringReplacementMap["/*$(ContextDestructor)*/"] << "\n";
        if (!node.comment.empty())
        {
            stringReplacementMap["/*$(ContextDestructor)*/"] <<
                "\n        // " << node.comment;
        }

        stringReplacementMap["/*$(ContextDestructor)*/"] <<
            "\n        if(" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << ")"
            "\n        {"
            "\n            s_delayedRelease.Add(" << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << ");"
            "\n            " << GetResourceNodePathInContext(node.visibility) << "buffer_" << node.name << " = nullptr;"
            "\n        }"
        ;
    }
}