///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

static bool ProcessNodeTag(const RenderGraph& renderGraph, const RenderGraphNode_Action_ComputeShader& node, std::string& tag, std::string& text)
{
    // Execute the technique
    if (tag == "/*$(AddTechnique)*/")
    {
        // /*$(Node:Dispatch)*/
        {
            std::ostringstream dispatch;

            if (node.dispatchSize.indirectBuffer.nodeIndex != -1)
            {
                int indirectBufferResourceNodeIndex = node.dispatchSize.indirectBuffer.nodeIndex;
                if (!GetNodeIsResourceNode(renderGraph.nodes[indirectBufferResourceNodeIndex]))
                    indirectBufferResourceNodeIndex = GetResourceNodeForPin(renderGraph, node.dispatchSize.indirectBuffer.nodeIndex, node.dispatchSize.indirectBuffer.nodePinIndex);

                dispatch <<
                    "        PassParameters->IndirectDispatchArgsBuffer = Buffer_" << renderGraph.nodes[indirectBufferResourceNodeIndex].resourceBuffer.name << ";\n"
                    "\n"
                    "        // Execute shader\n"
                    "        TShaderMapRef<F" << node.name << "CS> ComputeShader(View.ShaderMap);\n"
                    "        FComputeShaderUtils::AddPass(\n"
                    "            GraphBuilder,\n"
                    "            RDG_EVENT_NAME(\"" << renderGraph.name << "." << node.name << "\"),\n"
                    "            ComputeShader,\n"
                    "            PassParameters,\n"
                    "            Buffer_" << renderGraph.nodes[indirectBufferResourceNodeIndex].resourceBuffer.name << ",\n"
                    "            0\n"
                    "        );\n"
                    ;
            }
            else
            {
                dispatch <<
                    "        // Calculate dispatch size\n";
                if (node.dispatchSize.node.textureNode)
                {
                    dispatch << "        FIntVector dispatchSize = Texture_" << node.dispatchSize.node.textureNode->name << "->Desc.GetSize();\n";
                }
                else if (node.dispatchSize.node.bufferNode)
                {
                    dispatch << "        FIntVector dispatchSize = FIntVector(Buffer_" << node.dispatchSize.node.bufferNode->name << "->Desc.NumElements, 1, 1);\n";
                }
                else if (node.dispatchSize.variable.variableIndex != -1)
                {
                    const Variable& variable = renderGraph.variables[node.dispatchSize.variable.variableIndex];
                    DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(variable.type);
                    switch (typeInfo.componentCount)
                    {
                        case 1: dispatch << "        FIntVector dispatchSize = FIntVector(" << VariableToString(variable, renderGraph) << ", 1, 1);\n"; break;
                        case 2: dispatch << "        FIntVector dispatchSize = FIntVector(" << VariableToString(variable, renderGraph) << "[0], " << VariableToString(variable, renderGraph) << "[1], 1);\n"; break;
                        case 3: dispatch << "        FIntVector dispatchSize = FIntVector(" << VariableToString(variable, renderGraph) << "[0], " << VariableToString(variable, renderGraph) << "[1], " << VariableToString(variable, renderGraph) << "[2]);\n"; break;
                        default:
                        {
                            Assert(false, "Inappropriate variable type given for dispatch size.");
                        }
                    }
                }
                else
                {
                    dispatch << "        FIntVector dispatchSize = FIntVector(1,1,1);\n";
                }

                dispatch <<
                    "        dispatchSize.X = (((dispatchSize.X + " << node.dispatchSize.preAdd[0] << ") * " << node.dispatchSize.multiply[0] << ") / " <<
                    node.dispatchSize.divide[0] << " + " << node.dispatchSize.postAdd[0] << " + " << node.shader.shader->NumThreads[0] << " - 1) / " << node.shader.shader->NumThreads[0] << ";\n"
                    "        dispatchSize.Y = (((dispatchSize.Y + " << node.dispatchSize.preAdd[1] << ") * " << node.dispatchSize.multiply[1] << ") / " <<
                    node.dispatchSize.divide[1] << " + " << node.dispatchSize.postAdd[1] << " + " << node.shader.shader->NumThreads[1] << " - 1) / " << node.shader.shader->NumThreads[1] << ";\n"
                    "        dispatchSize.Z = (((dispatchSize.Z + " << node.dispatchSize.preAdd[2] << ") * " << node.dispatchSize.multiply[2] << ") / " <<
                    node.dispatchSize.divide[2] << " + " << node.dispatchSize.postAdd[2] << " + " << node.shader.shader->NumThreads[2] << " - 1) / " << node.shader.shader->NumThreads[2] << ";\n"
                    "\n"
                    "        // Execute shader\n"
                    "        TShaderMapRef<F" << node.name << "CS> ComputeShader(View.ShaderMap);\n"
                    "        FComputeShaderUtils::AddPass(\n"
                    "            GraphBuilder,\n"
                    "            RDG_EVENT_NAME(\"" << renderGraph.name << "." << node.name << "\"),\n"
                    "            ComputeShader,\n"
                    "            PassParameters,\n"
                    "            dispatchSize\n"
                    "        );\n"
                    ;
            }

            StringReplaceAll(text, "/*$(Node:Dispatch)*/", dispatch.str());
        }

        // /*$(Node:Condition)*/
        {
            std::ostringstream condition;

            if (node.condition.comparison != ConditionComparison::Count)
                condition << "    if (" << ConditionToString(node.condition, renderGraph, &VariableToString) << ")\n";

            StringReplaceAll(text, "/*$(Node:Condition)*/", condition.str());
        }

        // /*$(Node:SetParameters)*/
        StringReplaceAll(text, "/*$(Node:SetParameters)*/", FillShaderParams(renderGraph, node.resourceDependencies, *node.shader.shader, 0, "        ", "PassParameters->", ""));
    }

    // Define the shader struct
    if (tag == "/*$(TechniqueCPP_Shaders)*/")
    {
        std::string fileName = std::filesystem::path(node.shader.shader->destFileName).replace_extension(".usf").string();

        StringReplaceAll(text, "/*$(Node:EntryPoint)*/",node.shader.shader->entryPoint);
        StringReplaceAll(text, "/*$(Node:FileName)*/", std::string("/Engine/Private/" + renderGraph.name + "/") + fileName);

        StringReplaceAll(text, "/*$(Node:ShaderParams)*/", MakeShaderParams(renderGraph, node, node.resourceDependencies, *node.shader.shader, 0, node.resourceDependencies.size()));

        StringReplaceAll(text, "/*$(Node:ShaderDefines)*/", MakeShaderDefines(*node.shader.shader));

        // TODO: samplers?
    }

    return true;
}
