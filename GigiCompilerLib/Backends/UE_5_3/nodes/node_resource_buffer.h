///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

static void MakeBufferDesc(const RenderGraph& renderGraph, const RenderGraphNode_Resource_Buffer& node, std::ostringstream& makeDesc, const char* indent)
{
    makeDesc << indent << "// Calculate size\n";

    // make size
    {
        if (node.count.node.bufferNode)
        {
            makeDesc << indent << "uint32 NumElements = Buffer_" << node.count.node.bufferNode->name << "->Desc.NumElements;\n";
        }
        else if (node.count.variable.variableIndex != -1)
        {
            const Variable& variable = renderGraph.variables[node.count.variable.variableIndex];
            DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(variable.type);
            switch (typeInfo.componentCount)
            {
                case 1: makeDesc << indent << "uint32 NumElements = " << VariableToString(variable, renderGraph) << ";\n"; break;
                default:
                {
                    GigiAssert(false, "Inappropriate variable type given for buffer size.");
                }
            }
        }
        else
        {
            makeDesc << indent << "uint32 NumElements = 1;\n";
        }

        makeDesc <<
            indent << "NumElements = ((NumElements + " << node.count.preAdd << ") * " << node.count.multiply << ") / " <<
            node.count.divide << " + " << node.count.postAdd << ";\n"
            "\n"
            ;
    }

    // Bytes per element
    if (node.format.node.nodeIndex != -1)
    {
        makeDesc << indent << "uint32 BytesPerElement = Buffer_" << node.count.node.bufferNode->name << "->Desc.BytesPerElement;\n";
    }
    else if (node.format.structureType.structIndex != -1)
    {
        makeDesc << indent << "uint32 BytesPerElement = " << renderGraph.structs[node.format.structureType.structIndex].sizeInBytes << ";\n";
    }
    else
    {
        makeDesc << indent << "uint32 BytesPerElement = " << DataFieldTypeToSize(node.format.type) << ";\n";
    }

    makeDesc <<
        "\n"
        << indent << "FRDGBufferDesc desc;\n"
        << indent << "desc.BytesPerElement = BytesPerElement;\n"
        << indent << "desc.NumElements = NumElements;\n"
        ;

    // Usage flags
    {
        std::ostringstream flags;

        flags << "EBufferUsageFlags::Static";

        #define HANDLE_FLAG(GIGI_FLAG, UNREAL_FLAG) \
            if (node.accessedAs & (1 << (unsigned int)GIGI_FLAG)) \
                flags << " | " #UNREAL_FLAG;

        HANDLE_FLAG(ShaderResourceAccessType::UAV, EBufferUsageFlags::UnorderedAccess);
        HANDLE_FLAG(ShaderResourceAccessType::SRV, EBufferUsageFlags::ShaderResource);
        HANDLE_FLAG(ShaderResourceAccessType::CopySource, EBufferUsageFlags::SourceCopy);
        HANDLE_FLAG(ShaderResourceAccessType::Indirect, EBufferUsageFlags::DrawIndirect);
        HANDLE_FLAG(ShaderResourceAccessType::VertexBuffer, EBufferUsageFlags::VertexBuffer);
        HANDLE_FLAG(ShaderResourceAccessType::IndexBuffer, EBufferUsageFlags::IndexBuffer);

        // StructuredBuffer and ByteAddressBuffer
        {
            bool bufferUsedAsRaw = false;
            bool bufferUsedAsStructuredBuffer = false;
            for (const RenderGraphNode& nodeBase : renderGraph.nodes)
            {
                if (GetNodeIsResourceNode(nodeBase))
                    continue;

                int pinCount = GetNodePinCount(nodeBase);
                for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
                {
                    int resourceNodeIndex = GetResourceNodeForPin(renderGraph, nodeBase, pinIndex);
                    if (resourceNodeIndex != node.nodeIndex)
                        continue;

                    const ShaderResource* shaderResource = GetNodePinInputNodeInfo(nodeBase, pinIndex).shaderResource;
                    if (!shaderResource)
                        continue;

                    if (shaderResource->buffer.raw)
                        bufferUsedAsRaw = true;

                    if (ShaderResourceBufferIsStructuredBuffer(shaderResource->buffer))
                        bufferUsedAsStructuredBuffer = true;
                }
            }

            BufferNodeType bufferNodeType = GetBufferNodeType(renderGraph, node);
            if (bufferNodeType.structIndex != -1 || bufferUsedAsStructuredBuffer)
                flags << " | EBufferUsageFlags::StructuredBuffer";

            if (bufferUsedAsRaw)
                flags << " | EBufferUsageFlags::ByteAddressBuffer";
        }

        #undef HANDLE_FLAG

        makeDesc << indent << "desc.Usage = " << flags.str() << ";\n";
    }
}

static bool ProcessNodeTag(const RenderGraph& renderGraph, const RenderGraphNode_Resource_Buffer& node, std::string& tag, std::string& text)
{
    // Buffers that are not imported, and are not transient, have persistent storage in FTechniqueState
    {
        if (tag == "/*$(FTechniqueState-NotImported-Persistent)*/")
        {
            if (node.visibility != ResourceVisibility::Imported && !node.transient)
            {
                tag = "/*$(FTechniqueState)*/";
                return true;
            }
            return false;
        }
        //<ResourceBuffer:AddTechnique-NotImported-Persistent>
        if (tag == "/*$(AddTechnique-NotImported-Persistent)*/")
        {
            if (node.visibility != ResourceVisibility::Imported && !node.transient)
            {
                std::ostringstream makeDesc;
                MakeBufferDesc(renderGraph, node, makeDesc, "        ");
                StringReplaceAll(text, "/*$(Node:MakeDesc)*/", makeDesc.str());
                tag = "/*$(AddTechnique)*/";
                return true;
            }
            return false;
        }
    }

    // Buffers that are not imported, and transient, live as locals in AddTechnique
    // They need to be created though.
    if (tag == "/*$(AddTechnique-NotImported-Transient)*/")
    {
        if (node.visibility != ResourceVisibility::Imported && node.transient)
        {
            std::ostringstream makeDesc;
            MakeBufferDesc(renderGraph, node, makeDesc, "        ");
            StringReplaceAll(text, "/*$(Node:MakeDesc)*/", makeDesc.str());
            tag = "/*$(AddTechnique)*/";
            return true;
        }

        return false;
    }

    // Imported and exported buffers get storage in FTechniqueParams to be input, or output
    {
        if (tag == "/*$(FTechniqueParamsInputs-Imported-NonRT)*/")
        {
            if (node.visibility == ResourceVisibility::Imported && ((node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RTScene)) == 0))
            {
                tag = "/*$(FTechniqueParamsInputs)*/";

                // /*$(Node:VertexFormat)*/
                {
                    std::ostringstream vertexFormat;
                    if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::VertexBuffer))
                        vertexFormat << "        FVertexDeclarationElementList BufferVertexFormat_" << node.name << ";\n";
                    StringReplaceAll(text, "/*$(Node:VertexFormat)*/", vertexFormat.str());
                }

                return true;
            }

            return false;
        }

        if (tag == "/*$(FTechniqueParamsInputs-Imported-RT)*/")
        {
            if (node.visibility == ResourceVisibility::Imported && ((node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RTScene)) != 0))
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
