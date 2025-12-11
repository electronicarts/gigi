///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include "Schemas/Types.h"
#include "GigiCompilerLib/Backends/Shared.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include "GigiCompilerLib/Utils.h"
#include "GigiCompilerLib/ParseCSV.h"
#include "Nodes/nodes.h"

bool AdjustStructForAlignment_WebGPU(Struct& s, const std::string& path, bool isUniformBuffer);
bool AdjustUniformStructForAlignment_DX12(Struct& s, const std::string& path);

inline void ZeroDfltIfEmpty(std::string& dflt, DataFieldType type, const std::string& path)
{
    if (dflt.empty())
    {
        switch (type)
        {
            case DataFieldType::Int: dflt = "0"; break;
            case DataFieldType::Int2: dflt = "0,0"; break;
            case DataFieldType::Int3: dflt = "0,0,0"; break;
            case DataFieldType::Int4: dflt = "0,0,0,0"; break;
            case DataFieldType::Uint: dflt = "0"; break;
            case DataFieldType::Uint2: dflt = "0,0"; break;
            case DataFieldType::Uint3: dflt = "0,0,0"; break;
            case DataFieldType::Uint4: dflt = "0,0,0,0"; break;
            case DataFieldType::Float: dflt = "0.0f"; break;
            case DataFieldType::Float2: dflt = "0.0f,0.0f"; break;
            case DataFieldType::Float3: dflt = "0.0f,0.0f,0.0f"; break;
            case DataFieldType::Float4: dflt = "0.0f,0.0f,0.0f,0.0f"; break;
            case DataFieldType::Bool: dflt = "false"; break;
            case DataFieldType::Float4x4: dflt = "0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f"; break;
            case DataFieldType::Uint_16: dflt = "0"; break;
            case DataFieldType::Int_64: dflt = "0"; break;
            case DataFieldType::Uint_64: dflt = "0"; break;
            case DataFieldType::Float_16: dflt = "0"; break;
            default:
            {
                GigiAssert(false, "Unhandled data field type %s (%i).\nIn %s\n", EnumToString(type), type, path.c_str());
                break;
            }
        }
    }
}

inline int GetEnumIndex(const RenderGraph& renderGraph, const char* scope, const char* name)
{
    for (size_t i = 0; i < renderGraph.enums.size(); ++i)
    {
        const Enum& e = renderGraph.enums[i];
        if (e.scope == scope && e.originalName == name)
            return (int)i;
    }

    return -1;
}

struct DfltFixupVisitor
{
    DfltFixupVisitor(Backend backend)
        : m_backend(backend)
    {

    }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool FixupTypeDflt(const DataFieldType& type, std::string& dflt)
    {
        DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(type);

        // Lazily implementing the cases that need to be fixed. add more as needed.
        if (typeInfo.componentType2 == DataFieldType::Float)
        {
            std::vector<float> values;
            ParseCSV::ForEachValue(dflt.c_str(), false,
                [&](int tokenIndex, const char* token)
                {
                    float f;
                    sscanf_s(token, "%f", &f);
                    values.push_back(f);
                    return true;
                }
            );

            std::ostringstream stream;

            while (values.size() < typeInfo.componentCount)
                values.push_back(0.0f);

            for (int componentIndex = 0; componentIndex < typeInfo.componentCount; ++componentIndex)
            {
                if (componentIndex > 0)
                    stream << ", ";

                std::ostringstream singleValueStream;
                singleValueStream << std::setprecision(std::numeric_limits<float>::max_digits10) << values[componentIndex];
                std::string singleValueString = singleValueStream.str();
                stream << singleValueString;
                if (singleValueString.find('.') == std::string::npos)
                    stream << ".";

                if (m_backend != Backend::WebGPU)
                    stream << "f";
            }
            dflt = stream.str();
        }
        else if (typeInfo.componentType2 == DataFieldType::Bool)
        {
            if (m_backend == Backend::WebGPU)
            {
                std::vector<bool> values;
                ParseCSV::ForEachValue(dflt.c_str(), false,
                    [&](int tokenIndex, const char* token)
                    {
                            bool value = false;
                            if (!_stricmp(token, "false"))
                                value = false;
                            else if (!_stricmp(token, "true"))
                                value = true;
                            else
                            {
                                int i;
                                sscanf_s(token, "%i", &i);
                                value = (i != 0);
                            }
                            values.push_back(value);
                            return true;
                    }
                );

                std::ostringstream stream;

                while (values.size() < typeInfo.componentCount)
                    values.push_back(false);

                for (int componentIndex = 0; componentIndex < typeInfo.componentCount; ++componentIndex)
                {
                    if (componentIndex > 0)
                        stream << ", ";

                    stream << (values[componentIndex] ? "true" : "false");
                }
                dflt = stream.str();
            }
        }

        return true;
    }

    bool Visit(StructField& field, const std::string& path)
    {
        FixupTypeDflt(field.type, field.dflt);
        return true;
    }

    bool Visit(Variable& variable, const std::string& path)
    {
        FixupTypeDflt(variable.type, variable.dflt);
        return true;
    }

    Backend m_backend;
};

struct DataFixupVisitor
{
    DataFixupVisitor(RenderGraph& renderGraph_, Backend& backend_)
        : renderGraph(renderGraph_)
        , backend(backend_)
    { }

    ~DataFixupVisitor()
    {
        renderGraph.structs.insert(renderGraph.structs.end(), newStructs.begin(), newStructs.end());
    }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(Condition& condition, const std::string& path)
    {
        if (condition.comparison == ConditionComparison::Count)
            return true;

        if (condition.variable1.empty() || condition.variable1Index == -1)
            condition.comparison = ConditionComparison::Count;

        return true;
    }

    bool Visit(Struct& s, const std::string& path)
    {
        if (backend != Backend::WebGPU || s.isForShaderConstants)
            return true;

        Struct sCopy = s;

        if (AdjustStructForAlignment_WebGPU(s, path, false))
        {
            sCopy.name += "_Unpadded";
            newStructs.push_back(sCopy);
        }

        return true;
    }

    bool Visit(RenderGraphNode_Action_External& node, const std::string& path)
    {
        StaticNodeInfo staticNodeInfo = GetStaticNodeInfo(node);
        if (staticNodeInfo.backendSupported[(int)backend])
            return true;

        GigiAssert(false, "Node \"%s\" is not supported on backend \"%s\"\n", node.name.c_str(), EnumToString(backend));
        return false;
    }

    std::vector<Struct> newStructs;

    RenderGraph& renderGraph;
    Backend backend;
};

struct HandleValueOrVariablesVisitor
{
    HandleValueOrVariablesVisitor(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(ValueOrVariable_Bool& v, const std::string& path)
    {
        if (!v.variable.name.empty())
            return true;

        Variable newVar;
        newVar.name = FrontEndNodesNoCaching::GetUniqueVariableName(renderGraph, "ValOrVar");
        newVar.comment = "Created for ValueOrVariable";
        newVar.type = DataFieldType::Bool;
        newVar.Const = true;
        newVar.Static = true;
        newVar.dflt = v.value ? "true" : "false";
        renderGraph.variables.push_back(newVar);

        v.variable.name = newVar.name;

        return true;
    }

    bool Visit(ValueOrVariable_Float& v, const std::string& path)
    {
        if (!v.variable.name.empty())
            return true;

        std::ostringstream valueString;
        valueString << v.value;

        Variable newVar;
        newVar.name = FrontEndNodesNoCaching::GetUniqueVariableName(renderGraph, "ValOrVar");
        newVar.comment = "Created for ValueOrVariable";
        newVar.type = DataFieldType::Float;
        newVar.Const = true;
        newVar.Static = true;
        newVar.dflt = valueString.str();
        renderGraph.variables.push_back(newVar);

        v.variable.name = newVar.name;

        return true;
    }

    bool Visit(ValueOrVariable_Float2& v, const std::string& path)
    {
        if (!v.variable.name.empty())
            return true;

        std::ostringstream valueString;
        valueString << v.value[0] << ", " << v.value[1];

        Variable newVar;
        newVar.name = FrontEndNodesNoCaching::GetUniqueVariableName(renderGraph, "ValOrVar");
        newVar.comment = "Created for ValueOrVariable";
        newVar.type = DataFieldType::Float2;
        newVar.Const = true;
        newVar.Static = true;
        newVar.dflt = valueString.str();
        renderGraph.variables.push_back(newVar);

        v.variable.name = newVar.name;

        return true;
    }

    bool Visit(ValueOrVariable_Float3& v, const std::string& path)
    {
        if (!v.variable.name.empty())
            return true;

        std::ostringstream valueString;
        valueString << v.value[0] << ", " << v.value[1] << ", " << v.value[2];

        Variable newVar;
        newVar.name = FrontEndNodesNoCaching::GetUniqueVariableName(renderGraph, "ValOrVar");
        newVar.comment = "Created for ValueOrVariable";
        newVar.type = DataFieldType::Float3;
        newVar.Const = true;
        newVar.Static = true;
        newVar.dflt = valueString.str();
        renderGraph.variables.push_back(newVar);

        v.variable.name = newVar.name;

        return true;
    }

    bool Visit(ValueOrVariable_Int4& v, const std::string& path)
    {
        if (!v.variable.name.empty())
            return true;

        std::ostringstream valueString;
        valueString << v.value[0] << ", " << v.value[1] << ", " << v.value[2] << ", " << v.value[3];

        Variable newVar;
        newVar.name = FrontEndNodesNoCaching::GetUniqueVariableName(renderGraph, "ValOrVar");
        newVar.comment = "Created for ValueOrVariable";
        newVar.type = DataFieldType::Int4;
        newVar.Const = true;
        newVar.Static = true;
        newVar.dflt = valueString.str();
        renderGraph.variables.push_back(newVar);

        v.variable.name = newVar.name;

        return true;
    }

    bool Visit(ValueOrVariable_Enum_ExternalNode_AMD_FidelityFXSDK_Upscaling_GenerateReactiveMask_ReactiveMaskMode& v, const std::string& path)
    {
        if (!v.variable.name.empty())
            return true;

        std::ostringstream valueString;
        valueString << (int)(v.value);

        Variable newVar;
        newVar.name = FrontEndNodesNoCaching::GetUniqueVariableName(renderGraph, "ValOrVar");
        newVar.comment = "Created for ValueOrVariable";
        newVar.type = DataFieldType::Int;
        newVar.Const = true;
        newVar.Static = true;
        newVar.dflt = valueString.str();
        renderGraph.variables.push_back(newVar);

        v.variable.name = newVar.name;

        return true;
    }

    bool Visit(ValueOrVariable_Enum_ExternalNode_AMD_FidelityFXSDK_Upscaling_Version& v, const std::string& path)
    {
        if (!v.variable.name.empty())
            return true;

        std::ostringstream valueString;
        valueString << (int)(v.value);

        Variable newVar;
        newVar.name = FrontEndNodesNoCaching::GetUniqueVariableName(renderGraph, "ValOrVar");
        newVar.comment = "Created for ValueOrVariable";
        newVar.type = DataFieldType::Int;
        newVar.Const = true;
        newVar.Static = true;
        newVar.dflt = valueString.str();
        renderGraph.variables.push_back(newVar);

        v.variable.name = newVar.name;

        return true;
    }

    RenderGraph& renderGraph;
};

struct AddNodeInfoToShadersVisitor
{
    AddNodeInfoToShadersVisitor(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(RenderGraphNode_Action_ComputeShader& node, const std::string& path)
    {
        ShaderDefine newDefine;
        std::ostringstream stream;

        // Make node the dispatch parameters available
        newDefine.name = "__GigiDispatchMultiply";
        stream = std::ostringstream();
        stream << "uint3(" << node.dispatchSize.multiply[0] << "," << node.dispatchSize.multiply[1] << "," << node.dispatchSize.multiply[2] << ")";
        newDefine.value = stream.str();
        node.defines.push_back(newDefine);

        newDefine.name = "__GigiDispatchDivide";
        stream = std::ostringstream();
        stream << "uint3(" << node.dispatchSize.divide[0] << "," << node.dispatchSize.divide[1] << "," << node.dispatchSize.divide[2] << ")";
        newDefine.value = stream.str();
        node.defines.push_back(newDefine);

        newDefine.name = "__GigiDispatchPreAdd";
        stream = std::ostringstream();
        stream << "uint3(" << node.dispatchSize.preAdd[0] << "," << node.dispatchSize.preAdd[1] << "," << node.dispatchSize.preAdd[2] << ")";
        newDefine.value = stream.str();
        node.defines.push_back(newDefine);

        newDefine.name = "__GigiDispatchPostAdd";
        stream = std::ostringstream();
        stream << "uint3(" << node.dispatchSize.postAdd[0] << "," << node.dispatchSize.postAdd[1] << "," << node.dispatchSize.postAdd[2] << ")";
        newDefine.value = stream.str();
        node.defines.push_back(newDefine);

        // All shader variable aliases that are const variables need to be added as defines so they are compile time constants
        for (const ShaderVariableAlias& alias : node.shaderVariableAliases.aliases)
        {
            int variableIndex =  GetVariableIndex(renderGraph, alias.variable.name.c_str());
            if (variableIndex == -1)
                continue;

            const Variable& var = renderGraph.variables[variableIndex];
            if (!var.Const)
                continue;

            stream = std::ostringstream();
            stream << "__GIGI_AlIAS_VARIABLE_CONST_" << alias.name;
            newDefine.name = stream.str();
            stream = std::ostringstream();
            stream << var.dflt;
            newDefine.value = stream.str();
            node.defines.push_back(newDefine);
        }

        return true;
    }

    bool Visit(RenderGraphNode_Action_RayShader& node, const std::string& path)
    {
        ShaderDefine newDefine;
        std::ostringstream stream;

        // All shader variable aliases that are const variables need to be added as defines so they are compile time constants
        for (const ShaderVariableAlias& alias : node.shaderVariableAliases.aliases)
        {
            int variableIndex = GetVariableIndex(renderGraph, alias.variable.name.c_str());
            if (variableIndex == -1)
                continue;

            const Variable& var = renderGraph.variables[variableIndex];
            if (!var.Const)
                continue;

            stream = std::ostringstream();
            stream << "__GIGI_AlIAS_VARIABLE_CONST_" << alias.name;
            newDefine.name = stream.str();
            stream = std::ostringstream();
            stream << var.dflt;
            newDefine.value = stream.str();
            node.defines.push_back(newDefine);
        }

        return true;
    }

    bool Visit(RenderGraphNode_Action_DrawCall& node, const std::string& path)
    {
        auto HandleShaderVariableAliases = [this, &node](const ShaderVariableAliases& aliases)
            {
                ShaderDefine newDefine;
                std::ostringstream stream;

                for (const ShaderVariableAlias& alias : aliases.aliases)
                {
                    int variableIndex = GetVariableIndex(renderGraph, alias.variable.name.c_str());
                    if (variableIndex == -1)
                        continue;

                    const Variable& var = renderGraph.variables[variableIndex];
                    if (!var.Const)
                        continue;

                    stream = std::ostringstream();
                    stream << "__GIGI_AlIAS_VARIABLE_CONST_" << alias.name;
                    newDefine.name = stream.str();
                    stream = std::ostringstream();
                    stream << var.dflt;
                    newDefine.value = stream.str();
                    node.defines.push_back(newDefine);
                }
            }
        ;

        HandleShaderVariableAliases(node.amplificationShaderVariableAliases);
        HandleShaderVariableAliases(node.meshShaderVariableAliases);
        HandleShaderVariableAliases(node.vertexShaderVariableAliases);
        HandleShaderVariableAliases(node.pixelShaderVariableAliases);

        return true;
    }

    RenderGraph& renderGraph;
};

struct ErrorCheckVisitor
{
    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(RenderGraphNode_Action_DrawCall& node, const std::string& path)
    {
        // If there's a vertex shader, there can't be a mesh or amplification shader
        if (node.vertexShader.shaderIndex != -1)
        {
            if (node.meshShader.shaderIndex != -1)
            {
                GigiAssert(false, "Node %s has both a vertex shader and a mesh shader, only one is allowed.\nIn %s\n", node.name.c_str(), path.c_str());
                return false;
            }

            if (node.amplificationShader.shaderIndex != -1)
            {
                GigiAssert(false, "Node %s has both a vertex shader and an amplification shader. Amplification shader is not allowed when a vertex shader is specified.\nIn %s\n", node.name.c_str(), path.c_str());
                return false;
            }
        }
        // else, there needs to be a mesh shader
        else if (node.meshShader.shaderIndex == -1)
        {
            GigiAssert(false, "Node %s has neither a vertex shader or mesh shader.  One must be specified.\nIn %s\n", node.name.c_str(), path.c_str());
            return false;
        }

        // If this is a mesh shader (not a vertex shader) draw call, there can't be a vertex, index, or instance buffer plugged in
        if (node.meshShader.shaderIndex != -1)
        {
            if (node.vertexBuffer.nodeIndex != -1)
            {
                GigiAssert(false, "Node %s has uses a mesh shader, but has a vertex buffer plugged in, which is not allowed. You can give this buffer to the shader as an SRV.\nIn %s\n", node.name.c_str(), path.c_str());
                return false;
            }
            if (node.indexBuffer.nodeIndex != -1)
            {
                GigiAssert(false, "Node %s has uses a mesh shader, but has an index buffer plugged in, which is not allowed. You can give this buffer to the shader as an SRV\nIn %s\n", node.name.c_str(), path.c_str());
                return false;
            }
            if (node.instanceBuffer.nodeIndex != -1)
            {
                GigiAssert(false, "Node %s has uses a mesh shader, but has an instanceBuffer buffer plugged in, which is not allowed. You can give this buffer to the shader as an SRV\nIn %s\n", node.name.c_str(), path.c_str());
                return false;
            }
        }

        return true;
    }
};

struct DepluralizeFileCopiesVisitor
{
    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(RenderGraph& renderGraph, const std::string& path)
    {
        std::vector<FileCopy> newFileCopies;

        renderGraph.fileCopies.erase(
            std::remove_if(renderGraph.fileCopies.begin(), renderGraph.fileCopies.end(),
                [&](FileCopy& copy)
                {
                    if (!copy.plural)
                        return false;

                    // make sure there is a %i in the source file name
                    bool useCubeMapNames = false;
                    bool filenameIsPlural = false;
                    if (copy.fileName.find("%i") == std::string::npos)
                    {
                        if (copy.fileName.find("%s") != std::string::npos)
                        {
                            useCubeMapNames = true;
                            filenameIsPlural = true;
                        }
                    }
                    else
                    {
                        filenameIsPlural = true;
                    }

                    int fileCopyCount = -1;

                    static const char* c_cubeMapNames[] =
                    {
                        "Right",
                        "Left",
                        "Up",
                        "Down",
                        "Front",
                        "Back"
                    };

                    while (true)
                    {
                        fileCopyCount++;

                        char indexedFileName[1024];
                        if (useCubeMapNames)
                            sprintf_s(indexedFileName, copy.fileName.c_str(), c_cubeMapNames[fileCopyCount]);
                        else
                            sprintf_s(indexedFileName, copy.fileName.c_str(), fileCopyCount);

                        char indexedDestFileName[1024];
                        sprintf_s(indexedDestFileName, copy.destFileName.c_str(), fileCopyCount);

                        char fullFileName[4096];
                        sprintf_s(fullFileName, "%s%s", renderGraph.baseDirectory.c_str(), indexedFileName);

                        if (!FileExists(fullFileName))
                        {
                            if (fileCopyCount == 0)
                            {
                                GigiAssert(false, "No files found for file copy pattern %s!\nIn %s\n", copy.fileName.c_str(), path.c_str());
                                return false;
                            }
                            break;
                        }

                        FileCopy newFileCopy = copy;
                        newFileCopy.plural = false;
                        newFileCopy.fileName = indexedFileName;
                        newFileCopy.destFileName = indexedDestFileName;
                        newFileCopies.push_back(newFileCopy);

                        if (useCubeMapNames && fileCopyCount == 5)
                            break;

                        if (!filenameIsPlural)
                            break;
                    }

                    return true;
                }
            ),
            renderGraph.fileCopies.end()
        );

        // Add the new file copies in
        renderGraph.fileCopies.insert(renderGraph.fileCopies.end(), newFileCopies.begin(), newFileCopies.end());
        return true;
    }
};

struct ShaderFileDuplicationVisitor
{
    ShaderFileDuplicationVisitor(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(Shader& shader, const std::string& path)
    {
        // Set the destFileName if it hasn't been set yet
        if (shader.destFileName.empty())
            shader.destFileName = shader.fileName;

        // If the file isn't going to be copied, ignore it
        if (!shader.copyFile)
            return true;

        // If this shader file name hasn't been used yet, allow it, and remember that it is being used.
        std::string fileName = std::filesystem::weakly_canonical(shader.destFileName).string();
        auto it = existingShaderFiles.find(fileName);
        if (it == existingShaderFiles.end())
        {
            existingShaderFiles[fileName] = shader.name;
            return true;
        }

        // otherwise, error because the shaders will stomp each other with different shader resources
        // NOTE: warn, not error for now because looping subgraphs (and subgraphs being used multiple times) cause this to trip too. need to sort that out before it can be an error.
        ShowWarningMessage("Shader \"%s\" wants to write out \"%s\" which is also written out by shader \"%s\" so will be stomped.\nIn %s\n", shader.name.c_str(), fileName.c_str(), it->second.c_str(), path.c_str());
        return true;
    }

    RenderGraph& renderGraph;


    std::unordered_map<std::string, std::string> existingShaderFiles;
};

struct ShaderReferenceFixupVisitor
{
    ShaderReferenceFixupVisitor(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(ComputeShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Compute && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find compute shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(WorkGraphShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::WorkGraph && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find work graph shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(RayGenShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::RTRayGen && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find RTRayGen shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(RTClosestHitShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::RTClosestHit && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find RTClosestHit shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(RTClosestHitShaderReferenceOptional& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::RTClosestHit && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find RTClosestHit shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(RTAnyHitShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::RTAnyHit && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find RTAnyHit shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(RTAnyHitShaderReferenceOptional& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::RTAnyHit && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find RTAnyHit shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(RTIntersectionShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::RTIntersection && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find RTIntersection shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(RTIntersectionShaderReferenceOptional& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::RTIntersection && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find RTIntersection shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(VertexShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Vertex && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find vertex shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(PixelShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Pixel && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find pixel shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(AmplificationShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Amplification && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find amplification shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(MeshShaderReference& data, const std::string& path)
    {
        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Mesh && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find mesh shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(VertexShaderReferenceOptional& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Vertex && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find vertex shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(PixelShaderReferenceOptional& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Pixel && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find pixel shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(AmplificationShaderReferenceOptional& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Amplification && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find amplification shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(MeshShaderReferenceOptional& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (int index = 0; index < (int)renderGraph.shaders.size(); ++index)
        {
            if (renderGraph.shaders[index].type == ShaderType::Mesh && !_stricmp(renderGraph.shaders[index].name.c_str(), data.name.c_str()))
            {
                data.shaderIndex = index;
                data.shader = &renderGraph.shaders[index];
                return true;
            }
        }
        GigiAssert(false, "Could not find mesh shader referenced: %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    RenderGraph& renderGraph;
};

struct ReferenceFixupVisitor
{
    ReferenceFixupVisitor(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    {
        visitedNode.resize(renderGraph.nodes.size(), false);
    }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(TextureNodeReference& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        if (!Visit(*(NodeReference*)&data, path))
            return false;

        switch (renderGraph.nodes[data.nodeIndex]._index)
        {
            case RenderGraphNode::c_index_resourceTexture:
            {
                data.textureNode = &renderGraph.nodes[data.nodeIndex].resourceTexture;
                break;
            }
            default:
            {
                GigiAssert(false, "%s was referenced as a texture node, but is not one!\nIn %s\n", data.name.c_str(), path.c_str());
                return false;
            }
        }

        return true;
    }

    bool Visit(BufferNodeReference& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        if (!Visit(*(NodeReference*)&data, path))
            return false;

        switch (renderGraph.nodes[data.nodeIndex]._index)
        {
            case RenderGraphNode::c_index_resourceBuffer:
            {
                data.bufferNode = &renderGraph.nodes[data.nodeIndex].resourceBuffer;
                break;
            }
            default:
            {
                GigiAssert(false, "%s was referenced as a buffer node, but is not one!\nIn %s\n", data.name.c_str(), path.c_str());
                return false;
            }
        }

        return true;
    }

    bool Visit(TextureOrBufferNodeReference& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        if (!Visit(*(NodeReference*)&data, path))
            return false;

        switch (renderGraph.nodes[data.nodeIndex]._index)
        {
            case RenderGraphNode::c_index_resourceTexture:
            {
                data.textureNode = &renderGraph.nodes[data.nodeIndex].resourceTexture;
                break;
            }
            case RenderGraphNode::c_index_resourceBuffer:
            {
                data.bufferNode = &renderGraph.nodes[data.nodeIndex].resourceBuffer;
                break;
            }
            default:
            {
                GigiAssert(false, "%s was referenced as a texture or buffer node, but is neither!\nIn %s\n", data.name.c_str(), path.c_str());
                return false;
            }
        }

        return true;
    }

    bool Visit(NodeReference& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        data.nodeIndex = GetNodeIndexByName(data.name.c_str());
        if (data.nodeIndex != -1)
            return true;

        GigiAssert(false, "Could not find node referenced: %s\nIn %s\n", data.name.c_str(), path.c_str(), path.c_str());
        return false;
    }

    bool Visit(NodePinReference& data, const std::string& path)
    {
        // Get the nodeIndex and nodePinIndex
        bool found = false;
        for (int nodeIndex = 0; nodeIndex < (int)renderGraph.nodes.size(); ++nodeIndex)
        {
            RenderGraphNode& node = renderGraph.nodes[nodeIndex];
            if (!_stricmp(data.node.c_str(), GetNodeName(node).c_str()))
            {
                data.nodeIndex = nodeIndex;
                int pinCount = GetNodePinCount(node);
                for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
                {
                    if (!_stricmp(data.pin.c_str(), GetNodePinName(node, pinIndex).c_str()))
                    {
                        data.nodePinIndex = pinIndex;
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;

                GigiAssert(false, "Could not find pin referenced: %s:%s\nIn %s\n", data.node.c_str(), data.pin.c_str(), path.c_str());
                return false;
            }
        }
        if (!found)
        {
            GigiAssert(false, "Could not find node referenced: %s\nIn %s\n", data.node.c_str(), path.c_str());
            return false;
        }

        // Find the resourceNodeIndex!
        data.resourceNodeIndex = data.nodeIndex;
        int nodePinIndex = data.nodePinIndex;
        while (true)
        {
            RenderGraphNode& node = renderGraph.nodes[data.resourceNodeIndex];

            bool isResourceNode = false;
            ExecuteOnNode(node,
                [&](auto& node)
                {
                    isResourceNode = node.c_isResourceNode;
                }
            );

            if (isResourceNode)
                return true;

            // make sure this node has been visited before get get pin into, since visitation fills it out
            ExecuteOnNode(node,
                [&](auto& node)
                {
                    Visit(node, path);
                }
            );
            InputNodeInfo info = GetNodePinInputNodeInfo(node, nodePinIndex);

            if (info.nodeIndex == -1)
            {
                GigiAssert(false, "Could not get resourceNodeIndex for nodePinReference %s.%s\nIn %s\n", data.node.c_str(), data.pin.c_str(), path.c_str());
                return false;
            }

            data.resourceNodeIndex = info.nodeIndex;
            nodePinIndex = info.pinIndex;
        }

        return true;
    }

    bool Visit(NodePinReferenceOptional& data, const std::string& path)
    {
        if (data.node.empty() || data.pin.empty())
            return true;

        // Get the nodeIndex and nodePinIndex
        bool found = false;
        for (int nodeIndex = 0; nodeIndex < (int)renderGraph.nodes.size(); ++nodeIndex)
        {
            RenderGraphNode& node = renderGraph.nodes[nodeIndex];
            if (!_stricmp(data.node.c_str(), GetNodeName(node).c_str()))
            {
                data.nodeIndex = nodeIndex;
                int pinCount = GetNodePinCount(node);
                for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
                {
                    if (!_stricmp(data.pin.c_str(), GetNodePinName(node, pinIndex).c_str()))
                    {
                        data.nodePinIndex = pinIndex;
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;

                GigiAssert(false, "Could not find pin referenced: %s:%s\nIn %s\n", data.node.c_str(), data.pin.c_str(), path.c_str());
                return false;
            }
        }
        if (!found)
        {
            GigiAssert(false, "Could not find node referenced: %s\nIn %s\n", data.node.c_str(), path.c_str());
            return false;
        }

        // Find the resourceNodeIndex!
        data.resourceNodeIndex = data.nodeIndex;
        int nodePinIndex = data.nodePinIndex;
        while (true)
        {
            RenderGraphNode& node = renderGraph.nodes[data.resourceNodeIndex];

            bool isResourceNode = false;
            ExecuteOnNode(node,
                [&](auto& node)
                {
                    isResourceNode = node.c_isResourceNode;
                }
            );

            if (isResourceNode)
                return true;

            // make sure this node has been visited before get get pin into, since visitation fills it out
            ExecuteOnNode(node,
                [&](auto& node)
                {
                    Visit(node, path);
                }
            );

            InputNodeInfo info = GetNodePinInputNodeInfo(node, nodePinIndex);

            if (info.nodeIndex == -1)
            {
                GigiAssert(false, "Could not get resourceNodeIndex for nodePinReferenceOptional %s.%s\nIn %s\n", data.node.c_str(), data.pin.c_str(), path.c_str());
                return false;
            }

            data.resourceNodeIndex = info.nodeIndex;
            nodePinIndex = info.pinIndex;
        }

        return true;
    }

    bool Visit(RenderGraphNode_Resource_Buffer& data, const std::string& path)
    {
        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        if (data.visibility == ResourceVisibility::Internal)
            return true;

        if (!Visit(data.format.structureType, path))
            return false;

        if (data.format.structureType.structIndex == -1)
            return true;

        renderGraph.structs[data.format.structureType.structIndex].exported = true;

        return true;
    }

    bool Visit(RenderGraphNode_Action_ComputeShader& data, const std::string& path)
    {
        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        // Make sure the indirect buffer is resolved too
        if(data.enableIndirect)
            Visit(data.dispatchSize.indirectBuffer, path);

        int connectionIndex = -1;
        Shader& shader = *data.shader.shader;
        for (NodePinConnection& connection : data.connections)
        {
            connectionIndex++;
            // get the source pin
            int pinCount = (int)shader.resources.size();
            for (int i = 0; i < pinCount; ++i)
            {
                if (!_stricmp(connection.srcPin.c_str(), shader.resources[i].name.c_str()))
                {
                    connection.srcNodePinIndex = i;
                    break;
                }
            }
            if (connection.srcNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find source pin \"%s\" (connections[%i]) in shader node \"%s\"\nIn %s\n", connection.srcPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node
            connection.dstNodeIndex = GetNodeIndexByName(connection.dstNode.c_str());
            if (connection.dstNodeIndex == -1)
            {
                GigiAssert(false, "Could not find dest node \"%s\" (connections[%i]) in shader node \"%s\"\nIn %s\n", connection.dstNode.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node pin
            connection.dstNodePinIndex = GetNodePinIndexByName(renderGraph.nodes[connection.dstNodeIndex], connection.dstPin.c_str());
            if (connection.dstNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find dest pin %s (connections[%i]) in shader node %s\nIn %s\n", connection.dstPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }
        }

        if (data.connections.size() != shader.resources.size())
        {
            GigiAssert(false, "node %s doesn't have the right number of connections for shader %s\nIn %s\n", data.name.c_str(), shader.name.c_str(), path.c_str());
            return false;
        }

        // sort the connections to be in the same order as the shader resources are
        std::sort(data.connections.begin(), data.connections.end(), [](const NodePinConnection& a, const NodePinConnection& b) { return a.srcNodePinIndex < b.srcNodePinIndex; });

        return true;
    }

    bool Visit(RenderGraphNode_Action_RayShader& data, const std::string& path)
    {
        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        // Remember that this render graph uses ray tracing!
        renderGraph.usesRaytracing = true;

        int connectionIndex = -1;
        Shader& shader = *data.shader.shader;
        for (NodePinConnection& connection : data.connections)
        {
            connectionIndex++;
            // get the source pin
            int pinCount = (int)shader.resources.size();
            for (int i = 0; i < pinCount; ++i)
            {
                if (!_stricmp(connection.srcPin.c_str(), shader.resources[i].name.c_str()))
                {
                    connection.srcNodePinIndex = i;
                    break;
                }
            }
            if (connection.srcNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find source pin \"%s\" (connections[%i]) in shader node \"%s\"\nIn %s\n", connection.srcPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node
            connection.dstNodeIndex = GetNodeIndexByName(connection.dstNode.c_str());
            if (connection.dstNodeIndex == -1)
            {
                GigiAssert(false, "Could not find dest node \"%s\" (connections[%i]) in shader node \"%s\"\nIn %s\n", connection.dstNode.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node pin
            connection.dstNodePinIndex = GetNodePinIndexByName(renderGraph.nodes[connection.dstNodeIndex], connection.dstPin.c_str());
            if (connection.dstNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find dest pin %s (connections[%i]) in shader node %s\nIn %s\n", connection.dstPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }
        }

        if (data.connections.size() != shader.resources.size())
        {
            GigiAssert(false, "node %s doesn't have the right number of connections for shader %s\nIn %s\n", data.name.c_str(), shader.name.c_str(), path.c_str());
            return false;
        }

        // sort the connections to be in the same order as the shader resources are
        std::sort(data.connections.begin(), data.connections.end(), [](const NodePinConnection& a, const NodePinConnection& b) { return a.srcNodePinIndex < b.srcNodePinIndex; });

        return true;
    }

    bool Visit(RenderGraphNode_Action_SubGraph& data, const std::string& path)
    {
        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        for (int connectionIndex = 0; connectionIndex < (int)data.connections.size(); ++connectionIndex)
        {
            // set the source pin
            NodePinConnection& connection = data.connections[connectionIndex];
            connection.srcNodePinIndex = connectionIndex;

            // get the dest node
            connection.dstNodeIndex = GetNodeIndexByName(connection.dstNode.c_str());
            if (connection.dstNodeIndex == -1)
            {
                GigiAssert(false, "Could not find dest node \"%s\" (connections[%i]) in SubGraph node \"%s\"\nIn %s\n", connection.dstNode.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node pin
            connection.dstNodePinIndex = GetNodePinIndexByName(renderGraph.nodes[connection.dstNodeIndex], connection.dstPin.c_str());
            if (connection.dstNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find dest pin \"%s\" (connections[%i]) in SubGraph node \"%s\"\nIn %s\n", connection.dstPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }
        }
        return true;
    }

    bool Visit(RenderGraphNode_Action_Barrier& data, const std::string& path)
    {
        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        for (int connectionIndex = 0; connectionIndex < (int)data.connections.size(); ++connectionIndex)
        {
            // set the source pin
            NodePinConnection& connection = data.connections[connectionIndex];
            connection.srcNodePinIndex = connectionIndex;

            // get the dest node
            connection.dstNodeIndex = GetNodeIndexByName(connection.dstNode.c_str());
            if (connection.dstNodeIndex == -1)
            {
                GigiAssert(false, "Could not find dest node \"%s\" (connections[%i]) in Barrier node \"%s\"\nIn %s\n", connection.dstNode.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node pin
            connection.dstNodePinIndex = GetNodePinIndexByName(renderGraph.nodes[connection.dstNodeIndex], connection.dstPin.c_str());
            if (connection.dstNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find dest pin \"%s\" (connections[%i]) in Barrier node \"%s\"\nIn %s\n", connection.dstPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }
        }
        return true;
    }

    bool Visit(RenderGraphNode_Action_WorkGraph& data, const std::string& path) 
    {
        // agilitysdk required for work graphs.
        renderGraph.settings.dx12.AgilitySDKRequired = true;

        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        // Get pin counts
        int connectionIndex = -1;
        Shader* shader = data.entryShader.shader;
        int pinCount = shader ? (int)shader->resources.size() : 0;
        for (NodePinConnection& connection : data.connections)
        {
            connectionIndex++;
            // get the source pin
            for (int i = 0; i < pinCount; ++i)
            {
                if (!_stricmp(connection.srcPin.c_str(), shader->resources[i].name.c_str()))
                {
                    connection.srcNodePinIndex = i;
                    break;
                }
            }

            if (connection.srcNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find source pin \"%s\" (connections[%i]) in work graph node \"%s\"\nIn %s\n", connection.srcPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node
            connection.dstNodeIndex = GetNodeIndexByName(connection.dstNode.c_str());
            if (connection.dstNodeIndex == -1)
            {
                GigiAssert(false, "Could not find dest node \"%s\" (connections[%i]) in work graph node \"%s\"\nIn %s\n", connection.dstNode.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node pin
            connection.dstNodePinIndex = GetNodePinIndexByName(renderGraph.nodes[connection.dstNodeIndex], connection.dstPin.c_str());
            if (connection.dstNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find dest pin %s (connections[%i]) in work graph node %s\nIn %s\n", connection.dstPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }
        }

        // sort the connections to be in the same order as the shader resources are
        std::sort(data.connections.begin(), data.connections.end(), [](const NodePinConnection& a, const NodePinConnection& b) { return a.srcNodePinIndex < b.srcNodePinIndex; });

        Visit(data.records, path + ".recordsBuffer");
        
        return true;
    }
	bool Visit(RenderGraphNode_Reroute& data, const std::string& path)
	{
		if (visitedNode[data.nodeIndex])
			return true;
		visitedNode[data.nodeIndex] = true;

		for (int connectionIndex = 0; connectionIndex < (int)data.connections.size(); ++connectionIndex)
		{
			// set the source pin
			NodePinConnection& connection = data.connections[connectionIndex];
			connection.srcNodePinIndex = connectionIndex;

			// get the dest node
			connection.dstNodeIndex = GetNodeIndexByName(connection.dstNode.c_str());
			if (connection.dstNodeIndex == -1)
			{
				GigiAssert(false, "Could not find dest node \"%s\" (connections[%i]) in Barrier node \"%s\"\nIn %s\n", connection.dstNode.c_str(), connectionIndex, data.name.c_str(), path.c_str());
				return false;
			}

			// get the dest node pin
			connection.dstNodePinIndex = GetNodePinIndexByName(renderGraph.nodes[connection.dstNodeIndex], connection.dstPin.c_str());
			if (connection.dstNodePinIndex == -1)
			{
				GigiAssert(false, "Could not find dest pin \"%s\" (connections[%i]) in Barrier node \"%s\"\nIn %s\n", connection.dstPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
				return false;
			}
		}
		return true;
	}

    bool Visit(RenderGraphNode_Action_CopyResource& data, const std::string& path)
    {
        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        Visit(data.source, path + ".source");
        Visit(data.dest, path + ".dest");
        return true;
    }

    bool Visit(RenderGraphNode_Action_DrawCall& data, const std::string& path)
    {
        if (visitedNode[data.nodeIndex])
            return true;
        visitedNode[data.nodeIndex] = true;

        int connectionIndex = -1;
        Shader* vertexShader = data.vertexShader.shader;
        Shader& pixelShader = *data.pixelShader.shader;
        Shader* amplificationShader = data.amplificationShader.shader;
        Shader* meshShader = data.meshShader.shader;

        if (vertexShader && meshShader)
        {
            GigiAssert(false, "Node %s has both a vertex and mesh shader, which is not allowed.\nIn %s\n", data.name.c_str(), path.c_str());
            return false;
        }

        if (!vertexShader && !meshShader)
        {
            GigiAssert(false, "Node %s has neither a vertex nor a mesh shader, one must be specified.\nIn %s\n", data.name.c_str(), path.c_str());
            return false;
        }

        if (meshShader || amplificationShader)
            renderGraph.settings.dx12.AgilitySDKRequired = true;

        // Get pin counts
        int vertexPinCount = vertexShader ? (int)vertexShader->resources.size() : 0;
        int pixelPinCount = (int)pixelShader.resources.size();
        int amplificationPinCount = amplificationShader ? (int)amplificationShader->resources.size() : 0;
        int meshPinCount = meshShader ? (int)meshShader->resources.size() : 0;

        for (NodePinConnection& connection : data.connections)
        {
            connectionIndex++;
            // get the source pin
            for (int i = 0; i < vertexPinCount; ++i)
            {
                if (!_stricmp(connection.srcPin.c_str(), vertexShader->resources[i].name.c_str()))
                {
                    connection.srcNodePinIndex = i;
                    break;
                }
            }
            for (int i = 0; i < pixelPinCount; ++i)
            {
                if (!_stricmp(connection.srcPin.c_str(), pixelShader.resources[i].name.c_str()))
                {
                    connection.srcNodePinIndex = vertexPinCount + i;
                    break;
                }
            }
            for (int i = 0; i < amplificationPinCount; ++i)
            {
                if (!_stricmp(connection.srcPin.c_str(), amplificationShader->resources[i].name.c_str()))
                {
                    connection.srcNodePinIndex = vertexPinCount + pixelPinCount + i;
                    break;
                }
            }
            for (int i = 0; i < meshPinCount; ++i)
            {
                if (!_stricmp(connection.srcPin.c_str(), meshShader->resources[i].name.c_str()))
                {
                    connection.srcNodePinIndex = vertexPinCount + pixelPinCount + amplificationPinCount + i;
                    break;
                }
            }

            if (connection.srcNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find source pin \"%s\" (connections[%i]) in draw call node \"%s\"\nIn %s\n", connection.srcPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node
            connection.dstNodeIndex = GetNodeIndexByName(connection.dstNode.c_str());
            if (connection.dstNodeIndex == -1)
            {
                GigiAssert(false, "Could not find dest node \"%s\" (connections[%i]) in draw call node \"%s\"\nIn %s\n", connection.dstNode.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }

            // get the dest node pin
            connection.dstNodePinIndex = GetNodePinIndexByName(renderGraph.nodes[connection.dstNodeIndex], connection.dstPin.c_str());
            if (connection.dstNodePinIndex == -1)
            {
                GigiAssert(false, "Could not find dest pin %s (connections[%i]) in draw call node %s\nIn %s\n", connection.dstPin.c_str(), connectionIndex, data.name.c_str(), path.c_str());
                return false;
            }
        }

        if (data.connections.size() != (vertexPinCount + pixelPinCount + amplificationPinCount + meshPinCount))
        {
            GigiAssert(false, "node %s doesn't have the right number of connections for shaders\nIn %s\n", data.name.c_str(), path.c_str());
            return false;
        }

        // sort the connections to be in the same order as the shader resources are
        std::sort(data.connections.begin(), data.connections.end(), [](const NodePinConnection& a, const NodePinConnection& b) { return a.srcNodePinIndex < b.srcNodePinIndex; });

        // Also set the pin index etc for depth and color targets!
        Visit(data.shadingRateImage, path + ".shadingRateImage");
        Visit(data.vertexBuffer, path + ".vertexBuffer");
        Visit(data.indexBuffer, path + ".indexBuffer");
        Visit(data.instanceBuffer, path + ".instanceBuffer");
        Visit(data.depthTarget, path + ".depthTarget");
        for (int i = 0; i < data.colorTargets.size(); ++i)
        {
            char pathBuffer[64];
            sprintf_s(pathBuffer, ".colorTargets[%i]", i);
            Visit(data.colorTargets[i], path + pathBuffer);
        }

        return true;
    }

    bool Visit(Shader& data, const std::string& path)
    {
        // make all constant buffers add a CBV resource so a user doesn't need to do double data entry
        int cbIndex = -1;
        for (ShaderConstantBuffer& cbDesc : data.constantBuffers)
        {
            cbIndex++;

            // Make sure the struct reference is visited first
            char pathBuffer[64];
            sprintf_s(pathBuffer, ".constantBuffers[%i]", cbIndex);
            if (!Visit(cbDesc, path + pathBuffer))
                return false;

            ShaderResource newResource;
            newResource.name = cbDesc.resourceName;
            newResource.type = ShaderResourceType::ConstantBuffer;
            newResource.access = ShaderResourceAccessType::CBV;
            newResource.constantBufferStructIndex = cbDesc.structIndex;
            data.resources.push_back(newResource);
        }

        // compute register indices
        int nextRegisterIndexUAV = 0;
        int nextRegisterIndexSRV = 0;
        int nextRegisterIndexCBV = 0;
        for (ShaderResource& resource : data.resources)
        {
            switch (resource.access)
            {
                case ShaderResourceAccessType::UAV: resource.registerIndex = nextRegisterIndexUAV++; break;
                case ShaderResourceAccessType::RTScene:
                {
                    if (renderGraph.configFromBackend.RTSceneTakesSRVSlot)
                        resource.registerIndex = nextRegisterIndexSRV++;
                    break;
                }
                case ShaderResourceAccessType::SRV: resource.registerIndex = nextRegisterIndexSRV++; break;
                case ShaderResourceAccessType::CBV: resource.registerIndex = nextRegisterIndexCBV++; break;
                default:
                {
                    GigiAssert(false, "Unhandled shader resource access type %s (%i). Shader = \"%s\", Resource = \"%s\".\nIn %s\n", EnumToString(resource.access), resource.access, data.name.c_str(), resource.name.c_str(), path.c_str());
                    return false;
                }
            }
        }
        for (size_t samplerIndex = 0; samplerIndex < data.samplers.size(); ++samplerIndex)
            data.samplers[samplerIndex].registerIndex = (int)samplerIndex;

        return true;
    }

    bool Visit(Struct& data, const std::string& path)
    {
        if (data.forceHostVisible)
            data.exported = true;

        size_t bytes = 0;
        for (StructField& field : data.fields)
        {
            field.sizeInBytes = DataFieldTypeToSize(field.type);
            bytes += field.sizeInBytes;
        }

        data.sizeInBytes = bytes;
        return true;
    }

    bool Visit(RenderGraph& data, const std::string& path)
    {
        // let each node know it's index
        for (size_t index = 0; index < data.nodes.size(); ++index)
            ExecuteOnNode(data.nodes[index], [index](auto& node) {node.nodeIndex = (int)index; });
        return true;
    }

    bool Visit(VariableReference& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (size_t index = 0; index < renderGraph.variables.size(); ++index)
        {
            const Variable& variable = renderGraph.variables[index];

            if (!_stricmp(data.name.c_str(), variable.name.c_str()))
            {
                data.variableIndex = (int)index;
                return true;
            }
        }

        GigiAssert(data.variableIndex != -1, "Could not find variable %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(VariableReferenceNoConst& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (size_t index = 0; index < renderGraph.variables.size(); ++index)
        {
            const Variable& variable = renderGraph.variables[index];

            if (!_stricmp(data.name.c_str(), variable.name.c_str()))
            {
                data.variableIndex = (int)index;
                return true;
            }
        }

        GigiAssert(data.variableIndex != -1, "Could not find variable %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(VariableReferenceConstOnly& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (size_t index = 0; index < renderGraph.variables.size(); ++index)
        {
            const Variable& variable = renderGraph.variables[index];

            if (!_stricmp(data.name.c_str(), variable.name.c_str()))
            {
                data.variableIndex = (int)index;
                return true;
            }
        }

        GigiAssert(data.variableIndex != -1, "Could not find variable %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(StructReference& data, const std::string& path)
    {
        if (data.name.empty())
            return true;

        for (size_t index = 0; index < renderGraph.structs.size(); ++index)
        {
            const Struct& s = renderGraph.structs[index];

            if (!_stricmp(data.name.c_str(), s.name.c_str()))
            {
                data.structIndex = (int)index;
                return true;
            }
        }

        GigiAssert(data.structIndex != -1, "Could not find struct %s\nIn %s\n", data.name.c_str(), path.c_str());
        return false;
    }

    bool Visit(ShaderConstantBuffer& data, const std::string& path)
    {
        if (data.structName.empty())
            return true;

        for (size_t index = 0; index < renderGraph.structs.size(); ++index)
        {
            const Struct& s = renderGraph.structs[index];

            if (!_stricmp(data.structName.c_str(), s.name.c_str()))
            {
                data.structIndex = (int)index;
                return true;
            }
        }

        GigiAssert(data.structIndex != -1, "Could not find struct %s\nIn %s\n", data.structName.c_str(), path.c_str());
        return false;
    }

    bool Visit(Condition& data, const std::string& path)
    {
        if (!data.variable1.empty())
        {
            for (size_t index = 0; index < renderGraph.variables.size(); ++index)
            {
                const Variable& variable = renderGraph.variables[index];

                if (!_stricmp(data.variable1.c_str(), variable.name.c_str()))
                {
                    data.variable1Index = (int)index;
                    break;
                }
            }
            GigiAssert(data.variable1Index != -1, "Could not find variable %s\nIn %s\n", data.variable1.c_str(), path.c_str());
        }

        if (!data.variable2.empty())
        {
            for (size_t index = 0; index < renderGraph.variables.size(); ++index)
            {
                const Variable& variable = renderGraph.variables[index];

                if (!_stricmp(data.variable2.c_str(), variable.name.c_str()))
                {
                    data.variable2Index = (int)index;
                    break;
                }
            }
            GigiAssert(data.variable2Index != -1, "Could not find variable %s\nIn %s\n", data.variable2.c_str(), path.c_str());
        }

        return true;
    }

    RenderGraph& renderGraph;
    std::vector<bool> visitedNode;

    // Helpers
    int GetNodeIndexByName(const char* name)
    {
        for (int i = 0; i < (int)renderGraph.nodes.size(); ++i)
        {
            if (!_stricmp(name, GetNodeName(renderGraph.nodes[i]).c_str()))
                return i;
        }
        return -1;
    }

    int GetNodePinIndexByName(RenderGraphNode& node, const char* name)
    {
        int pinCount = GetNodePinCount(node);
        for (int i = 0; i < pinCount; ++i)
        {
            if (!_stricmp(name, GetNodePinName(node, i).c_str()))
                return i;
        }
        return -1;
    }
};

struct ValidationVisitor
{
    ValidationVisitor(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(Shader& shader, const std::string& path)
    {
        switch (shader.type)
        {
            case ShaderType::Compute:
            case ShaderType::RTRayGen:
            case ShaderType::Vertex:
            case ShaderType::Pixel:
            {
                return true;
            }
            case ShaderType::RTClosestHit:
            case ShaderType::RTMiss:
            case ShaderType::RTAnyHit:
            case ShaderType::RTIntersection:
            {
                if (shader.resources.size() != 0)
                {
                    GigiAssert(false, "Shader type \'%s\' has resources defined, but only the ray gen shader can. Ray gen resources are global so are accessible if they are in the same file or if the ray gen file is #included.\nIn %s\n", EnumToString(shader.type), path.c_str());
                    return false;
                }
            }
        }

        return true;
    }

    bool Visit(Condition& condition, const std::string& path)
    {
        int AVarIndex = GetVariableIndex(renderGraph, condition.variable1.c_str());
        if (AVarIndex == -1)
            return true;

        int BVarIndex = GetVariableIndex(renderGraph, condition.variable2.c_str());
        if (BVarIndex != -1 && renderGraph.variables[AVarIndex].type != renderGraph.variables[BVarIndex].type)
        {
            GigiAssert(false, "A condition with variable \"%s %s\" and \"%s %s\" is not possible because they are different types.\nIn %s\n", EnumToString(renderGraph.variables[AVarIndex].type), condition.variable1.c_str(), EnumToString(renderGraph.variables[BVarIndex].type), condition.variable2.c_str(), path.c_str());
            return false;
        }
        return true;
    }

    bool Visit(SetVariable& setVar, const std::string& path)
    {
        // Get the destination variable type
        int destVarIndex = GetVariableIndex(renderGraph, setVar.destination.name.c_str());
        if (destVarIndex == -1)
            return true;
        DataFieldType destVarType = renderGraph.variables[destVarIndex].type;
        if (setVar.destinationIndex != -1)
            destVarType = DataFieldTypeInfo(destVarType).componentType2;

        // If there is an A variable, make sure it's the same type
        int AVarIndex = GetVariableIndex(renderGraph, setVar.AVar.name.c_str());
        if (AVarIndex != -1)
        {
            DataFieldType AVarType = renderGraph.variables[AVarIndex].type;
            if (setVar.AVarIndex != -1)
                AVarType = DataFieldTypeInfo(AVarType).componentType2;

            if (destVarType != AVarType && setVar.op != SetVariableOperator::Noop)
            {
                GigiAssert(false, "Setting the variable \"%s %s\" to an equation involving variable \"%s %s\" is not possible because they are different types.\nIn %s\n", EnumToString(renderGraph.variables[destVarIndex].type), setVar.destination.name.c_str(), EnumToString(renderGraph.variables[AVarIndex].type), setVar.AVar.name.c_str(), path.c_str());
                return false;
            }
        }

        // If there is a B variable, make sure it's the same type
        int BVarIndex = GetVariableIndex(renderGraph, setVar.BVar.name.c_str());
        if (BVarIndex != -1 && renderGraph.variables[destVarIndex].type != renderGraph.variables[BVarIndex].type)
        {
            DataFieldType BVarType = renderGraph.variables[BVarIndex].type;
            if (setVar.BVarIndex != -1)
                BVarType = DataFieldTypeInfo(BVarType).componentType2;

            if (destVarType != BVarType)
            {
                GigiAssert(false, "Setting the variable \"%s %s\" to an equation involving variable \"%s %s\" is not possible because they are different types.\nIn %s\n", EnumToString(renderGraph.variables[destVarIndex].type), setVar.destination.name.c_str(), EnumToString(renderGraph.variables[BVarIndex].type), setVar.BVar.name.c_str(), path.c_str());
                return false;
            }
        }

        return true;
    }

    bool Visit(Variable& variable, const std::string& path)
    {
        // Every variable needs a type
        if (variable.type == DataFieldType::Count)
        {
            ShowErrorMessage("Variable \"%s\" does not have a data type set.", variable.name.c_str());
            return false;
        }

        // Every variable needs a default
        if (variable.Enum.empty())
            ZeroDfltIfEmpty(variable.dflt, variable.type, path);

        // Enum validation
        if (!variable.Enum.empty())
        {
            if (variable.type != DataFieldType::Int)
            {
                GigiAssert(false, "Variable \'%s\' uses enum \'%s\' but is not an integer type. Only integers can use enums.\nIn %s\n", variable.name.c_str(), variable.Enum.c_str(), path.c_str());
                return false;
            }

            int enumIndex = -1;
            for (const Enum& e : renderGraph.enums)
            {
                enumIndex++;
                if (e.scope == variable.scope && e.originalName == variable.Enum)
                {
                    variable.enumIndex = enumIndex;

                    if (variable.dflt.empty())
                    {
                        GigiAssert(renderGraph.enums[enumIndex].items.size() > 0, "Tried to set a dflt for Variable \'%s\' but the enum has no items!\nIn %s\n", variable.name.c_str(), path.c_str());
                        variable.dflt = renderGraph.enums[enumIndex].items[0].label;
                    }

                    return true;
                }
            }

            GigiAssert(false, "Variable \'%s\' uses an undeclared enum \'%s\'.\nIn %s\n", variable.name.c_str(), variable.Enum.c_str(), path.c_str());
            return false;
        }

        if (!variable.onUserChange.name.empty())
        {
            int variableIndex = GetVariableIndex(renderGraph, variable.onUserChange.name.c_str());
            GigiAssert(variableIndex != -1, "Could not find variable \"%s\".\nIn %s\n", variable.onUserChange.name.c_str(), path.c_str());
            const Variable& variable = renderGraph.variables[variableIndex];
            GigiAssert(!variable.Const, "Variable \"%s\" cannot be const.\nIn %s\n", variable.onUserChange.name.c_str(), path.c_str());
            GigiAssert(variable.type == DataFieldType::Bool, "Variable \"%s\" must be a bool.\nIn %s\n", variable.onUserChange.name.c_str(), path.c_str());
        }

        return true;
    }

    bool Visit(Struct& s, const std::string& path)
    {
        for (StructField& field : s.fields)
        {
            // Every field needs a default
            if (field.Enum.empty())
                ZeroDfltIfEmpty(field.dflt, field.type, path);

            if (field.Enum.empty())
                continue;

            if (field.type != DataFieldType::Int)
            {
                GigiAssert(false, "Struct field \'%s\' uses enum \'%s\' but is not an integer type. Only integers can use enums.\nIn %s\n", field.name.c_str(), field.Enum.c_str(), path.c_str());
                return false;
            }

            int enumIndex = -1;
            for (const Enum& e : renderGraph.enums)
            {
                enumIndex++;
                if (e.scope == s.scope && e.originalName == field.Enum)
                {
                    field.enumIndex = enumIndex;
                    if (field.dflt.empty())
                    {
                        GigiAssert(renderGraph.enums[enumIndex].items.size() > 0, "Tried to set a dflt for struct field \'%s\' but the enum has no items!\nIn %s\n", field.name.c_str(), path.c_str());
                        field.dflt = renderGraph.enums[enumIndex].items[0].label;
                    }
                    break;
                }
            }
            if (enumIndex != -1)
                continue;

            GigiAssert(false, "Struct \'%s\' field \'%s\' uses an undeclared enum \'%s\'.\nIn %s\n", s.name.c_str(), field.name.c_str(), field.Enum.c_str(), path.c_str());
            return false;
        }
        return true;
    }

    bool Visit(RenderGraph& renderGraph, const std::string& path)
    {
        // make sure the render graph has a name.
        if (renderGraph.name.empty())
        {
            GigiAssert(false, "The render graph name is empty. That field is used to make namespaces and folder names, so is required.\nIn %s\n", path.c_str());
            return false;
        }

        // Verify that render graph node names are unique
        {
            std::unordered_set<std::string> names;
            size_t currentNodeIndex = 0;
            for (auto& node : renderGraph.nodes)
            {
                // get the lower case version of the node name
                std::string name = GetNodeName(node);
                std::transform(name.begin(), name.end(), name.begin(),
                    [](unsigned char c) { return std::tolower(c); });

                if (names.find(name) != names.end())
                {
                    std::vector<size_t> indices;
                    for (size_t nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
                    {
                        if (GetNodeName(renderGraph.nodes[nodeIndex]) == name)
                            indices.push_back(nodeIndex);
                    }

                    GigiAssert(false, "node name %s appears more than once in the render graph.\nIn %s\n", name.c_str(), path.c_str());
                    return false;
                }
                names.insert(name);
                currentNodeIndex++;
            }
        }

        // Verify that shader names are unique
        {
            std::unordered_set<std::string> names;
            for (const auto& shader : renderGraph.shaders)
            {
                // get the lower case version of the node name
                std::string name = shader.name;
                std::transform(name.begin(), name.end(), name.begin(),
                    [](unsigned char c) { return std::tolower(c); });

                if (names.find(name) != names.end())
                {
                    GigiAssert(false, "shader name %s appears more than once in the render graph.\nIn %s\n", name.c_str(), path.c_str());
                    return false;
                }
                names.insert(name);
            }
        }

        return true;
    }

    RenderGraph& renderGraph;
};

struct SanitizeVisitor
{
    SanitizeVisitor(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    // The goal of this function is to take an arbitrary string s
    // and make a string which could be used as a C++ identifier
    // that is still reasonable as human readable.
    static void Sanitize(std::string& s)
    {
        StringReplaceAll(s, " ", "_");
        StringReplaceAll(s, "+", "plus");
        StringReplaceAll(s, "-", "_"); // could be minus, but is sometimes a dash
        StringReplaceAll(s, "*", "_");
        StringReplaceAll(s, "/", "_");
        StringReplaceAll(s, "=", "equals");
        StringReplaceAll(s, "#", "number");
        StringReplaceAll(s, ":", "_");
        StringReplaceAll(s, ";", "_");
        StringReplaceAll(s, ".", "_");
        StringReplaceAll(s, ",", "_");
        StringReplaceAll(s, "(", "_");
        StringReplaceAll(s, ")", "_");
        StringReplaceAll(s, "[", "_");
        StringReplaceAll(s, "]", "_");
        StringReplaceAll(s, "{", "_");
        StringReplaceAll(s, "}", "_");
        StringReplaceAll(s, "<", "_");
        StringReplaceAll(s, ">", "_");
        StringReplaceAll(s, "\\", "_");
        StringReplaceAll(s, "\'", "_");
        StringReplaceAll(s, "\"", "_");
        StringReplaceAll(s, "?", "_");
        StringReplaceAll(s, "!", "_");
        StringReplaceAll(s, "@", "_");
        StringReplaceAll(s, "$", "_");
        StringReplaceAll(s, "%", "_");
        StringReplaceAll(s, "^", "_");
        StringReplaceAll(s, "&", "and");
        StringReplaceAll(s, "|", "_");
        StringReplaceAll(s, "`", "_");
        StringReplaceAll(s, "~", "_");
    }

    void Sanitize(std::string& s, std::string& originalName)
    {
        if (originalName.empty())
            originalName = s;

        Sanitize(s);
    }

    bool Visit(RenderGraph& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(Variable& data, const std::string& path)
    {
        Sanitize(data.name, data.originalName);
        Sanitize(data.Enum);

        if (!data.Enum.empty())
            Sanitize(data.dflt);

        return true;
    }

    bool Visit(Struct& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(StructField& data, const std::string& path)
    {
        Sanitize(data.name);
        Sanitize(data.Enum);
        return true;
    }

    // Sanitize enum names and all the item labels
    bool Visit(Enum& data, const std::string& path)
    {
        Sanitize(data.name, data.originalName);
        for (EnumItem& item : data.items)
        {
            item.displayLabel = item.label;
            Sanitize(item.label);
        }
        return true;
    }

    // Sanitize node names
    bool Visit(RenderGraphNode_Base& node, const std::string& path)
    {
        Sanitize(node.name, node.originalName);
        return true;
    }

    // sanitize shader names
    bool Visit(Shader& data, const std::string& path)
    {
        Sanitize(data.name, data.originalName);
        for (std::string& s : data.Used_RTHitGroupIndex)
            Sanitize(s);
        for (std::string& s : data.Used_RTMissIndex)
            Sanitize(s);
        return true;
    }

    // sanitize Condition variable names
    bool Visit(Condition& data, const std::string& path)
    {
        Sanitize(data.variable1);
        Sanitize(data.variable2);
        return true;
    }

    // sanitize ComputeShaderReference shader names
    bool Visit(ComputeShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize WorkGraphShaderReference shader names
    bool Visit(WorkGraphShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize RayGenShaderReference shader names
    bool Visit(RayGenShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize RTClosestHitShaderReference shader names
    bool Visit(RTClosestHitShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize RTAnyHitShaderReference shader names
    bool Visit(RTAnyHitShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize RTIntersectionShaderReference shader names
    bool Visit(RTIntersectionShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize RTClosestHitShaderReferenceOptional shader names
    bool Visit(RTClosestHitShaderReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize RTAnyHitShaderReferenceOptional shader names
    bool Visit(RTAnyHitShaderReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize RTIntersectionShaderReferenceOptional shader names
    bool Visit(RTIntersectionShaderReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize VertexShaderReference shader names
    bool Visit(VertexShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }
    bool Visit(VertexShaderReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize PixelShaderReference shader names
    bool Visit(PixelShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }
    bool Visit(PixelShaderReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize MeshShaderReference shader names
    bool Visit(MeshShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }
    bool Visit(MeshShaderReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize AmplificationShaderReference shader names
    bool Visit(AmplificationShaderReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }
    bool Visit(AmplificationShaderReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    // sanitize NodePinConnection nodes and pin names
    bool Visit(NodePinConnection& data, const std::string& path)
    {
        Sanitize(data.srcPin);
        Sanitize(data.dstNode);
        Sanitize(data.dstPin);
        return true;
    }

    // sanitize ShaderResource names
    bool Visit(ShaderResource& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(TextureOrBufferNodeReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(TextureNodeReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(BufferNodeReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(VariableReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(VariableReferenceNoConst& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(VariableReferenceConstOnly& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(NodePinReference& data, const std::string& path)
    {
        Sanitize(data.node);
        Sanitize(data.pin);
        return true;
    }

    bool Visit(NodePinReferenceOptional& data, const std::string& path)
    {
        Sanitize(data.node);
        Sanitize(data.pin);
        return true;
    }

    bool Visit(StructReference& data, const std::string& path)
    {
        Sanitize(data.name);
        return true;
    }

    bool Visit(RTHitGroup& data, const std::string& path)
    {
        Sanitize(data.name, data.originalName);
        return true;
    }

    // remove connections which have a src pin, but no dest node
    bool Visit(RenderGraphNode_Action_ComputeShader& data, const std::string& path)
    {
        data.connections.erase(
            std::remove_if(
                data.connections.begin(), data.connections.end(),
                [](const NodePinConnection& connection)
                {
                    return connection.srcPin.empty() || connection.dstNode.empty() || connection.dstPin.empty();
                }
            ),
            data.connections.end()
        );
        return true;
    }
    bool Visit(RenderGraphNode_Action_RayShader& data, const std::string& path)
    {
        data.connections.erase(
            std::remove_if(
                data.connections.begin(), data.connections.end(),
                [](const NodePinConnection& connection)
                {
                    return connection.srcPin.empty() || connection.dstNode.empty() || connection.dstPin.empty();
                }
            ),
            data.connections.end()
                    );
        return true;
    }

    bool Visit(RenderGraphNode_Action_WorkGraph& data, const std::string& path)
    {
        data.connections.erase(
            std::remove_if(
                data.connections.begin(), data.connections.end(),
                [](const NodePinConnection& connection)
                {
                    return connection.srcPin.empty() || connection.dstNode.empty() || connection.dstPin.empty();
                }
            ),
            data.connections.end()
        );
        return true;
    }

    bool Visit(RenderGraphNode_Action_DrawCall& data, const std::string& path)
    {
        data.connections.erase(
            std::remove_if(
                data.connections.begin(), data.connections.end(),
                [](const NodePinConnection& connection)
                {
                    return connection.srcPin.empty() || connection.dstNode.empty() || connection.dstPin.empty();
                }
            ),
            data.connections.end()
        );
        return true;
    }
    bool Visit(RenderGraphNode_Action_SubGraph& data, const std::string& path)
    {
        data.connections.erase(
            std::remove_if(
                data.connections.begin(), data.connections.end(),
                [](const NodePinConnection& connection)
                {
                    return connection.srcPin.empty() || connection.dstNode.empty() || connection.dstPin.empty();
                }
            ),
            data.connections.end()
        );
        return true;
    }
    bool Visit(RenderGraphNode_Action_Barrier& data, const std::string& path)
    {
        data.connections.erase(
            std::remove_if(
                data.connections.begin(), data.connections.end(),
                [](const NodePinConnection& connection)
                {
                    return connection.srcPin.empty() || connection.dstNode.empty() || connection.dstPin.empty();
                }
            ),
            data.connections.end()
        );
        return true;
    }
	bool Visit(RenderGraphNode_Reroute& data, const std::string& path)
	{
		data.connections.erase(
			std::remove_if(
				data.connections.begin(), data.connections.end(),
				[](const NodePinConnection& connection)
				{
					return connection.srcPin.empty() || connection.dstNode.empty() || connection.dstPin.empty();
				}
			),
			data.connections.end()
		);
		return true;
	}

    bool Visit(WebGPU_RWTextureSplit& data, const std::string& path)
    {
        Sanitize(data.nodeName);
        Sanitize(data.shaderName);
        Sanitize(data.pinName);
        Sanitize(data.pinNameR);
        return true;
    }

    RenderGraph& renderGraph;
};

struct ShaderAssertsVisitor
{
    constexpr static std::string_view AssertUavSuffix = "__GigiAssertUAV";

    ShaderAssertsVisitor(RenderGraph& renderGraph_)
        :renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(RenderGraphNode& node, const std::string& path)
    {
        std::vector<Shader*> shadersWithAsserts;
        std::string actionNodeName;

        if (node._index == RenderGraphNode::c_index_actionComputeShader)
        {
            RenderGraphNode_Action_ComputeShader& shaderNode = node.actionComputeShader;
            actionNodeName = shaderNode.name;

            if (!ProcessNodeShader(shaderNode.shader.shader, shadersWithAsserts, path))
                return false;
        }
        else if (node._index == RenderGraphNode::c_index_actionWorkGraph)
        {
            RenderGraphNode_Action_WorkGraph& shaderNode = node.actionWorkGraph;
            actionNodeName = shaderNode.name;

            if (!ProcessNodeShader(shaderNode.entryShader.shader, shadersWithAsserts, path))
                return false;
        }
        else if (node._index == RenderGraphNode::c_index_actionDrawCall)
        {
            RenderGraphNode_Action_DrawCall& shaderNode = node.actionDrawCall;
            actionNodeName = shaderNode.name;

            if (!ProcessNodeShader(shaderNode.amplificationShader.shader, shadersWithAsserts, path))
                return false;

            if (!ProcessNodeShader(shaderNode.meshShader.shader, shadersWithAsserts, path))
                return false;

            if (!ProcessNodeShader(shaderNode.vertexShader.shader, shadersWithAsserts, path))
                return false;

            if (!ProcessNodeShader(shaderNode.pixelShader.shader, shadersWithAsserts, path))
                return false;
        }

        if (!shadersWithAsserts.empty())
        {
            const size_t currentNodeId = std::distance(renderGraph.nodes.data(), &node);
            const StructReference& structRef = InsertAssertUAVStruct();

            for (Shader* shader : shadersWithAsserts)
            {
                RenderGraphNode* newUAVBufNode = InsertAssertsUAVNode(*shader, actionNodeName, structRef);
                if (!AddResourceReference(renderGraph.nodes[currentNodeId], shader, newUAVBufNode->resourceBuffer))
                    return false;
            }
        }

        return true;
    }

    bool ProcessNodeShader(Shader* shader, std::vector<Shader*>& shaderWithAssert, const std::string& path)
    {
        if (shader)
        {
            const auto [isValid, hasAsserts] = ProcessShader(*shader, path);
            if (!isValid)
                return false;

            if (hasAsserts)
                shaderWithAssert.push_back(shader);
        }

        return true;
    }

    struct ShaderProcessResult
    {
        bool isValid = false;
        bool hasAsserts = false;
    };

    struct ShaderParsingResult
    {
        bool isValid = false;
        std::unordered_map<std::string, std::string> uniqueAssertsCalls;
    };

    ShaderProcessResult ProcessShader(Shader& shader, const std::string& path)
    {
        auto [isValid, uniqueAssertsCalls] = ParseShader(shader, path);
        if (!isValid)
            return {};

        const bool hasAsserts = !uniqueAssertsCalls.empty();
        if (hasAsserts)
            InsertStringReplacements(shader, uniqueAssertsCalls);

        return { true, hasAsserts };
    }

    ShaderParsingResult ParseShader(Shader& shader, const std::string& path)
    {
        std::string fileName = (std::filesystem::path(renderGraph.baseDirectory) / shader.fileName).string();

        std::vector<unsigned char> fileContents;
        if (!LoadFile(fileName, fileContents))
        {
            GigiAssert(false, "Could not load file %s\nIn %s\n", fileName.c_str(), path.c_str());
            return {};
        }
        fileContents.push_back(0);

        auto BeginsWith = [](const char* haystack, const char* needle) -> bool
        {
            size_t needleLen = strlen(needle);
            if (_strnicmp(haystack, needle, needleLen) != 0)
                return false;
            return true;
        };

        auto GetCondition = [](std::string_view& content)
        {
            const size_t commaPos = content.find_first_of(',');
            const bool isLastArg = commaPos == content.npos;

            const size_t argLen = isLastArg
                ? content.length()
                : commaPos;

            std::string_view res = content.substr(0, argLen);
            content = content.substr(argLen);

            return res;
        };

        auto GetFmt = [](std::string_view& content)
        {
            const size_t firstQuotaPos = content.find_first_of('"');

            std::string_view res;
            if (firstQuotaPos != content.npos)
            {
                const size_t lastQuotaPos = content.find_first_of('"', firstQuotaPos + 1);
                const size_t substrLen = lastQuotaPos != content.npos ? lastQuotaPos - firstQuotaPos + 1 : content.npos;

                std::string_view res = content.substr(firstQuotaPos, substrLen);
                content = content.substr(firstQuotaPos + substrLen);

                return res;
            }

            return std::string_view{};
        };

        auto GetFmtArgs = [](const std::string_view content)
        {
            const size_t commaPos = content.find(',');
            if (commaPos != content.npos)
                return content.substr(commaPos + 1);

            return std::string_view{};
        };

        auto CountFmtArgs = [](const std::string_view fmt, const std::string_view args)
        {
            size_t requestedFmtArgsCount = 0;
            for (const char ch : fmt)
                if (ch == '%')
                    ++requestedFmtArgsCount;

            size_t providedFmtArgsCount = 0;
            size_t hasChars = 0;
            for (const char ch : args)
            {
                hasChars |= !!iswalnum(ch);
                if (ch == ',')
                    ++providedFmtArgsCount;
            }
            providedFmtArgsCount += hasChars;

            return std::pair{ requestedFmtArgsCount, providedFmtArgsCount };
        };

        auto AcquireFormatStringId = [this](const std::string_view fmtStr)
        {
            std::string str(fmtStr);

            const auto it = fmtStrToId.find(str);
            if (it != fmtStrToId.end())
                return it->second;

            const size_t newId = renderGraph.assertsFormatStrings.size();
            renderGraph.assertsFormatStrings.push_back(str);
            fmtStrToId.insert({ str, newId });

            return newId;
        };

        bool hasValidDeclarations = true;
        std::unordered_map<std::string, std::string> uniqueAssertsCalls;

        ForEachToken((char*)fileContents.data(),
            [&](const std::string& tokenStr, const char* stringStart, const char* cursor)
            {
                const auto declError = [&](const char* error)
                {
                    ShowErrorMessage("'%s' has an invalid declaration, %s", tokenStr.c_str(), error);
                    hasValidDeclarations = false;
                };

                std::string_view token(tokenStr);
                const std::string_view prefix("/*$(GigiAssert:");
                const std::string_view suffix(")*/");

                if (!BeginsWith(token.data(), prefix.data()))
                    return;

                std::string_view arguments = token.substr(prefix.length(), token.length() - prefix.length() - suffix.length());
                if (arguments.empty())
                {
                    declError("missed arguments");
                    return;
                }

                const std::string_view condition = GetCondition(arguments);

                if (condition.empty() || condition.find_first_of('"') != condition.npos)
                {
                    declError("invalid condition");
                    return;
                }

                const std::string_view fmtStr = GetFmt(arguments);
                if (!fmtStr.empty() && (fmtStr.back() != '"'))
                {
                    declError("format string misses closing \"");
                    return;
                }

                const std::string_view fmtArgs = !fmtStr.empty() ? GetFmtArgs(arguments) : "";
                const auto [requestedArgsCount, providedArgsCount] = CountFmtArgs(fmtStr, fmtArgs);
                if (requestedArgsCount != providedArgsCount)
                {
                    declError("invalid number of the format arguments");
                    return;
                }

                //store assert token to the function call replacement
                {
                    std::string replValue = "__gigiGigiAssert(";
                    replValue += condition;

                    const std::string formatIdStr = std::to_string(AcquireFormatStringId(fmtStr.empty() ? condition : fmtStr));
                    replValue += ", " + formatIdStr;

                    if (!fmtStr.empty() && !fmtArgs.empty())
                    {
                        replValue += ", ";
                        replValue += fmtArgs;
                    }

                    replValue += ");";
                    uniqueAssertsCalls.insert({ std::string(token), std::move(replValue) });
                }
            });

        return { hasValidDeclarations, std::move(uniqueAssertsCalls) };
    }

    std::string GetShaderAssertUAVName(const Shader shader)
    {
        return "__" + shader.name + AssertUavSuffix.data();
    }

    void InsertStringReplacements(Shader& shader, std::unordered_map<std::string, std::string>& uniqueAssertsCalls)
    {
        for (const auto& [tokenName, tokenReplacement] : uniqueAssertsCalls)
        {
            TokenReplacement replacement;
            replacement.name = tokenName;
            replacement.value = tokenReplacement;
            shader.tokenReplacements.push_back(std::move(replacement));
        }

        const std::string assertBufName = GetShaderAssertUAVName(shader);

        TokenReplacement replacement;
        replacement.name = "/*$(ShaderResources)*/";
        replacement.value =
            "\nvoid __gigiGigiAssert(bool condition, uint fmtId,"
            "\n  float v1 = 0, float v2 = 0, float v3 = 0,"
            "\n  float v4 = 0, float v5 = 0, float v6 = 0)"
            "\n{"
            "\n  if (!condition)"
            "\n  {"
            "\n     uint wasFired;"
            "\n     InterlockedExchange(" + assertBufName + "[0].isFired, 1, wasFired); "
            "\n     if (wasFired)"
            "\n       return;"
            "\n"
            "\n     Struct_GigiAssert newAssert = (Struct_GigiAssert)0;"
            "\n     newAssert.isFired = 1;"
            "\n     newAssert.fmtId = fmtId;"
            "\n     newAssert.v1 = v1;"
            "\n     newAssert.v2 = v2;"
            "\n     newAssert.v3 = v3;"
            "\n     newAssert.v4 = v4;"
            "\n     newAssert.v5 = v5;"
            "\n     newAssert.v6 = v6;"
            "\n     " + assertBufName + "[0] = newAssert; "
            "\n     "
            "\n  }"
            "\n}";
        shader.tokenReplacements.push_back(std::move(replacement));
    }

    const StructReference& InsertAssertUAVStruct()
    {
        if (assertStructRef.structIndex >= 0)
            return assertStructRef;

        Struct assertStruct;
        assertStruct.name = "GigiAssert";

        auto makeStructField = [](const char* name, DataFieldType type)
            {
                StructField newField;
                newField.name = name;
                newField.type = type;
                return newField;
            };

        assertStruct.fields.push_back(makeStructField("isFired", DataFieldType::Uint));
        assertStruct.fields.push_back(makeStructField("fmtId", DataFieldType::Uint));
        assertStruct.fields.push_back(makeStructField("v1", DataFieldType::Float));
        assertStruct.fields.push_back(makeStructField("v2", DataFieldType::Float));
        assertStruct.fields.push_back(makeStructField("v3", DataFieldType::Float));
        assertStruct.fields.push_back(makeStructField("v4", DataFieldType::Float));
        assertStruct.fields.push_back(makeStructField("v5", DataFieldType::Float));
        assertStruct.fields.push_back(makeStructField("v6", DataFieldType::Float));

        assertStructRef.name = assertStruct.name;
        assertStructRef.structIndex = (int)renderGraph.structs.size();

        renderGraph.structs.push_back(std::move(assertStruct));

        return assertStructRef;
    }

    RenderGraphNode* InsertAssertsUAVNode(const Shader& shader, const std::string& actionNodeName, const StructReference& assertStructRef)
    {
        BufferFormatDesc format;
        format.structureType = assertStructRef;

        BufferCountDesc countDesc;
        countDesc.multiply = 1;

        RenderGraphNode newBufferNode;
        newBufferNode._index = RenderGraphNode::c_index_resourceBuffer;
        newBufferNode.resourceBuffer.visibility = ResourceVisibility::Internal;
        newBufferNode.resourceBuffer.format = format;
        newBufferNode.resourceBuffer.count = countDesc;
        newBufferNode.resourceBuffer.transient = true;
        newBufferNode.resourceBuffer.name = actionNodeName + "__GigiAssertUAV_" + shader.name;
        newBufferNode.resourceBuffer.originalName = newBufferNode.resourceBuffer.name;

        renderGraph.nodes.push_back(newBufferNode);

        return &renderGraph.nodes.back();
    }

    bool AddResourceReference(RenderGraphNode& node, Shader* shader, RenderGraphNode_Resource_Buffer& resourceBuffer)
    {
        ShaderResource newResource;
        newResource.name = GetShaderAssertUAVName(*shader);
        newResource.type = ShaderResourceType::Buffer;
        newResource.access = ShaderResourceAccessType::UAV;
        newResource.buffer.typeStruct = resourceBuffer.format.structureType;

        NodePinConnection newConnection;
        newConnection.srcPin = newResource.name;
        newConnection.dstNode = resourceBuffer.name;
        newConnection.dstPin = "resource";

        shader->resources.push_back(std::move(newResource));

        if (node._index == RenderGraphNode::c_index_actionComputeShader)
            node.actionComputeShader.connections.push_back(std::move(newConnection));
        else if (node._index == RenderGraphNode::c_index_actionDrawCall)
            node.actionDrawCall.connections.push_back(std::move(newConnection));
        else if (node._index == RenderGraphNode::c_index_actionWorkGraph)
            node.actionWorkGraph.connections.push_back(std::move(newConnection));
        else
        {
            ShowErrorMessage("Shaders GigiAssert: failed to add node connection: unsupported node type '%d'", node._index);
            return false;
        }

        return true;
    }

    RenderGraph& renderGraph;

    StructReference assertStructRef;
    std::unordered_map<std::string, size_t> fmtStrToId;
};

struct ShaderDataVisitor
{
    ShaderDataVisitor(RenderGraph& renderGraph_, Backend& backend_)
        : renderGraph(renderGraph_)
        , backend(backend_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool GatherRayShadersUsed(Shader& shader, const std::string& path)
    {
        // Load the file
        std::string fileName = (std::filesystem::path(renderGraph.baseDirectory) / shader.fileName).string();
        std::vector<unsigned char> fileContents;
        if (!LoadFile(fileName, fileContents))
        {
            GigiAssert(false, "Could not load file %s\nIn %s\n", fileName.c_str(), path.c_str());
            return false;
        }
        fileContents.push_back(0);

        auto BeginsWith = [](const char* haystack, int& index, const char* needle) -> bool
        {
            size_t needleLen = strlen(needle);
            if (_strnicmp(&haystack[index], needle, needleLen) != 0)
                return false;
            index += (int)needleLen;
            return true;
        };

        // Gather RTHitGroupIndex and RTMissIndex uses
        std::unordered_set<std::string> RTHitGroupIndexes, RTMissIndexes;
        ForEachToken((char*)fileContents.data(),
            [&](const std::string& tokenStr, const char* stringStart, const char* cursor)
            {
                const char* token = tokenStr.c_str();
                int tokenIndex = 0;
                if (!BeginsWith(token, tokenIndex, "/*$("))
                    return;

                if (BeginsWith(token, tokenIndex, "RTHitGroupIndex:"))
                {
                    std::string name = shader.scope + tokenStr.substr(tokenIndex, tokenStr.length() - (tokenIndex + 3));
                    SanitizeVisitor::Sanitize(name);
                    shader.Used_RTHitGroupIndex.push_back(name);
                }
                else if (BeginsWith(token, tokenIndex, "RTMissIndex:"))
                {
                    std::string name = shader.scope + tokenStr.substr(tokenIndex, tokenStr.length() - (tokenIndex + 3));
                    SanitizeVisitor::Sanitize(name);
                    shader.Used_RTMissIndex.push_back(name);
                }
                else
                {
                    return;
                }
            }
        );

        return true;
    }

    bool HookupVariables(Shader& shader, const std::string& path)
    {
        // Don't scan these type of shaders for variables
        switch (shader.type)
        {
            case ShaderType::RTClosestHit:
            case ShaderType::RTAnyHit:
            case ShaderType::RTIntersection:
            case ShaderType::RTMiss:
                return true;
        }

        // Gather the variables referenced in this shader
        std::string fileName = (std::filesystem::path(renderGraph.baseDirectory) / shader.fileName).string();

        std::vector<unsigned char> fileContents;
        if (!LoadFile(fileName, fileContents))
        {
            GigiAssert(false, "Could not load file %s\nIn %s\n", fileName.c_str(), path.c_str());
            return false;
        }
        fileContents.push_back(0);

        auto BeginsWith = [](const char* haystack, int& index, const char* needle) -> bool
        {
            size_t needleLen = strlen(needle);
            if (_strnicmp(&haystack[index], needle, needleLen) != 0)
                return false;
            index += (int)needleLen;
            return true;
        };

        std::unordered_set<std::string> variablesAccessedUnsorted;
        std::unordered_set<std::string> variableAliasesAccessedUnsorted;
        ForEachToken((char*)fileContents.data(),
            [&](const std::string& tokenStr, const char* stringStart, const char* cursor)
            {
                const char* token = tokenStr.c_str();
                int tokenIndex = 0;
                if (!BeginsWith(token, tokenIndex, "/*$("))
                    return;

                int oldTokenIndex = tokenIndex;

                if (BeginsWith(token, tokenIndex, "Variable:"))
                {
                    std::string variableName = tokenStr.substr(tokenIndex, tokenStr.length() - (tokenIndex + 3));

                    int variableIndex = GetScopedVariableIndex(renderGraph, (shader.scope + variableName).c_str());
                    if (variableIndex == -1)
                    {
                        GigiAssert(false, "Could not find variable \"%s\" referenced in shader file \"%s\".\nIn %s\n", variableName.c_str(), shader.fileName.c_str(), path.c_str());
                        return;
                    }

                    // constants don't need to go through a constant buffer, they can just be written into the shader
                    if (renderGraph.variables[variableIndex].Const)
                        return;

                    variablesAccessedUnsorted.insert(renderGraph.variables[variableIndex].name);
                }
                else if (BeginsWith(token, tokenIndex, "VariableAlias:"))
                {
                    std::string aliasName = tokenStr.substr(tokenIndex, tokenStr.length() - (tokenIndex + 3));

                    int aliasIndex = GetVariableAliasIndex(shader, aliasName.c_str());
                    if (aliasIndex == -1)
                    {
                        GigiAssert(false, "Could not find variable alias \"%s\" referenced in shader file \"%s\".\nIn %s\n", aliasName.c_str(), shader.fileName.c_str(), path.c_str());
                        return;
                    }

                    variableAliasesAccessedUnsorted.insert(aliasName);
                }
                else if (
                    BeginsWith(token, tokenIndex, "Image:") || BeginsWith(token, tokenIndex, "Image2D:") ||
                    BeginsWith(token, tokenIndex, "Image2DArray:") || BeginsWith(token, tokenIndex, "Image3D:") ||
                    BeginsWith(token, tokenIndex, "ImageCube:"))
                {
                    // Remember the dimensionality
                    TextureDimensionType dimensionType = TextureDimensionType::Texture2D;
                    if (BeginsWith(token, oldTokenIndex, "Image2DArray:"))
                        dimensionType = TextureDimensionType::Texture2DArray;
                    else if (BeginsWith(token, oldTokenIndex, "Image3D:"))
                        dimensionType = TextureDimensionType::Texture3D;
                    else if (BeginsWith(token, oldTokenIndex, "ImageCube:"))
                        dimensionType = TextureDimensionType::TextureCube;

                    // Get the file name string
                    const char* fileNameBegin = &token[tokenIndex];
                    const char* fileNameEnd = fileNameBegin;
                    while (*fileNameEnd && *fileNameEnd != ':' && *fileNameEnd != ')')
                        fileNameEnd++;
                    std::string fileName(fileNameBegin, fileNameEnd);
                    if (fileName.empty())
                    {
                        GigiAssert(false, "filename is empty.\nIn %s\n", path.c_str());
                        return;
                    }

                    // Get the texture format string
                    const char* textureFormatBegin = (*fileNameEnd) ? fileNameEnd + 1 : fileNameEnd;
                    const char* textureFormatEnd = textureFormatBegin;
                    while (*textureFormatEnd && *textureFormatEnd != ':' && *textureFormatEnd != ')')
                        textureFormatEnd++;
                    std::string textureFormatStr(textureFormatBegin, textureFormatEnd);

                    // Get the view type format string
                    const char* viewTypeBegin = (*textureFormatEnd) ? textureFormatEnd + 1 : textureFormatEnd;
                    const char* viewTypeEnd = viewTypeBegin;
                    while (*viewTypeEnd && *viewTypeEnd != ':' && *viewTypeEnd != ')')
                        viewTypeEnd++;
                    std::string viewTypeStr(viewTypeBegin, viewTypeEnd);

                    // Get the loadFileNameAsSRGB string
                    const char* loadFileNameAsSRGBBegin = (*viewTypeEnd) ? viewTypeEnd + 1 : viewTypeEnd;
                    const char* loadFileNameAsSRGBEnd = loadFileNameAsSRGBBegin;
                    while (*loadFileNameAsSRGBEnd && *loadFileNameAsSRGBEnd != ':' && *loadFileNameAsSRGBEnd != ')')
                        loadFileNameAsSRGBEnd++;
                    std::string loadFileNameAsSRGBStr(loadFileNameAsSRGBBegin, loadFileNameAsSRGBEnd);

                    // Get the makeMips string. optional.
                    const char* makeMipsBegin = (*loadFileNameAsSRGBEnd) ? loadFileNameAsSRGBEnd + 1 : loadFileNameAsSRGBEnd;
                    const char* makeMipsEnd = makeMipsBegin;
                    while (*makeMipsEnd && *makeMipsEnd != ':' && *makeMipsEnd != ')')
                        makeMipsEnd++;
                    std::string makeMipsStr(makeMipsBegin, makeMipsEnd);
                    if (makeMipsStr == "*/")
                        makeMipsStr = "";

                    // Get the texture format
                    TextureFormat textureFormat;
                    if (!StringToEnum(textureFormatStr.c_str(), textureFormat))
                    {
                        GigiAssert(false, "Unknown texture format: %s.\nIn %s\n", textureFormatStr.c_str(), path.c_str());
                        return;
                    }

                    // Get the view type
                    TextureViewType viewType;
                    if (!StringToEnum(viewTypeStr.c_str(), viewType))
                    {
                        GigiAssert(false, "Unknown texture view type: %s.\nIn %s\n", viewTypeStr.c_str(), path.c_str());
                        return;
                    }

                    // Get loadFileNameAsSRGB
                    bool loadFileNameAsSRGB = true;
                    if (!_stricmp(loadFileNameAsSRGBStr.c_str(), "true") || !_stricmp(loadFileNameAsSRGBStr.c_str(), "1"))
                    {
                        loadFileNameAsSRGB = true;
                    }
                    else if (!_stricmp(loadFileNameAsSRGBStr.c_str(), "false") || !_stricmp(loadFileNameAsSRGBStr.c_str(), "0"))
                    {
                        loadFileNameAsSRGB = false;
                    }
                    else
                    {
                        GigiAssert(false, "Couldn't read loadFileNameAsSRGB: %s.\nIn %s\n", loadFileNameAsSRGBStr.c_str(), path.c_str());
                        return;
                    }

                    // Get makeMips
                    bool makeMips = false;
                    if (!makeMipsStr.empty())
                    {
                        if (!_stricmp(makeMipsStr.c_str(), "true") || !_stricmp(makeMipsStr.c_str(), "1"))
                        {
                            makeMips = true;
                        }
                        else if (!_stricmp(makeMipsStr.c_str(), "false") || !_stricmp(makeMipsStr.c_str(), "0"))
                        {
                            makeMips = false;
                        }
                        else
                        {
                            GigiAssert(false, "Couldn't read makeMips: %s.\nIn %s\n", makeMipsStr.c_str(), path.c_str());
                            return;
                        }
                    }

                    // see if we've already made a load texture node for this texture in this format, so we can use that node if so
                    char textureLoadNodeName[256];
                    bool createNewTexture = true;
                    for (const RenderGraphNode& node : renderGraph.nodes)
                    {
                        if (node._index != RenderGraphNode::c_index_resourceTexture)
                            continue;

                        int desiredMips = makeMips ? 0 : 1;

                        const RenderGraphNode_Resource_Texture& textureNode = node.resourceTexture;
                        if (textureNode.loadFileName == fileName && textureNode.format.format == textureFormat && textureNode.dimension == dimensionType && textureNode.loadFileNameAsSRGB == loadFileNameAsSRGB && textureNode.numMips == desiredMips)
                        {
                            strcpy_s(textureLoadNodeName, textureNode.name.c_str());
                            createNewTexture = false;
                            break;
                        }
                    }

                    // create a new node if we should
                    if (createNewTexture)
                    {
                        // Make the name of the new node
                        sprintf_s(textureLoadNodeName, "_loadedTexture_%i", nextLoadedTextureIndex);
                        nextLoadedTextureIndex++;

                        // Add an internal, non transient render graph resource texture node for it.
                        {
                            RenderGraphNode newTextureNode;
                            newTextureNode._index = RenderGraphNode::c_index_resourceTexture;
                            newTextureNode.resourceTexture.visibility = ResourceVisibility::Internal;
                            newTextureNode.resourceTexture.format.format = textureFormat;
                            newTextureNode.resourceTexture.dimension = dimensionType;
                            newTextureNode.resourceTexture.loadFileName = fileName.c_str();
                            newTextureNode.resourceTexture.loadFileNameAsSRGB = loadFileNameAsSRGB;
                            newTextureNode.resourceTexture.numMips = makeMips ? 0 : 1;
                            newTextureNode.resourceTexture.name = textureLoadNodeName;
                            newTextureNode.resourceTexture.originalName = newTextureNode.resourceTexture.name;
                            newTextureNode.resourceTexture.transient = false;
                            renderGraph.nodes.push_back(newTextureNode);
                        }

                        // make asset file copies
                        {
                            FileCopy newFileCopy;
                            newFileCopy.fileName = fileName;
                            newFileCopy.type = FileCopyType::Asset;
                            newFileCopy.binary = true;
                            newFileCopy.plural = (dimensionType != TextureDimensionType::Texture2D);
                            renderGraph.fileCopies.push_back(newFileCopy);
                        }
                    }

                    // See if we have previously added this resource to this shader.
                    // This can happen if a shader references the same texture in the same format multiple times in a shader.
                    bool resourceAlreadyAdded = false;
                    for (const ShaderResource& resource : shader.resources)
                    {
                        if (resource.name == textureLoadNodeName)
                        {
                            resourceAlreadyAdded = true;
                            break;
                        }
                    }

                    // If the resource hasn't already been added to this shader, we need to add it
                    if (!resourceAlreadyAdded)
                    {
                        // Add the resource
                        ShaderResource newResource;
                        newResource.name = textureLoadNodeName;
                        newResource.type = ShaderResourceType::Texture;
                        newResource.access = ShaderResourceAccessType::SRV;
                        newResource.texture.viewType = viewType;
                        newResource.texture.dimension = dimensionType;

                        shader.resources.push_back(newResource);

                        // Hook up this resource node for all uses of this shader
                        for (RenderGraphNode& node : renderGraph.nodes)
                        {
                            switch (node._index)
                            {
                                case RenderGraphNode::c_index_actionComputeShader:
                                {
                                    RenderGraphNode_Action_ComputeShader& shaderNode = node.actionComputeShader;
                                    if (shaderNode.shader.name != shader.name)
                                        continue;

                                    NodePinConnection newConnection;
                                    newConnection.srcPin = textureLoadNodeName;
                                    newConnection.dstNode = textureLoadNodeName;
                                    newConnection.dstPin = "resource";
                                    shaderNode.connections.push_back(newConnection);
                                    break;
                                }
                                case RenderGraphNode::c_index_actionWorkGraph:
                                {
                                    RenderGraphNode_Action_WorkGraph& shaderNode = node.actionWorkGraph;
                                    if (shaderNode.entryShader.name != shader.name)
                                        continue;

                                    NodePinConnection newConnection;
                                    newConnection.srcPin = textureLoadNodeName;
                                    newConnection.dstNode = textureLoadNodeName;
                                    newConnection.dstPin = "resource";
                                    shaderNode.connections.push_back(newConnection);
                                    break;
                                }
                                case RenderGraphNode::c_index_actionRayShader:
                                {
                                    RenderGraphNode_Action_RayShader& shaderNode = node.actionRayShader;
                                    if (shaderNode.shader.name != shader.name)
                                        continue;

                                    NodePinConnection newConnection;
                                    newConnection.srcPin = textureLoadNodeName;
                                    newConnection.dstNode = textureLoadNodeName;
                                    newConnection.dstPin = "resource";
                                    shaderNode.connections.push_back(newConnection);
                                    break;
                                }
                                case RenderGraphNode::c_index_actionDrawCall:
                                {
                                    RenderGraphNode_Action_DrawCall& shaderNode = node.actionDrawCall;
                                    if (shaderNode.vertexShader.name != shader.name 
                                        && shaderNode.pixelShader.name != shader.name)
                                        continue;

                                    NodePinConnection newConnection;
                                    newConnection.srcPin = textureLoadNodeName;
                                    newConnection.dstNode = textureLoadNodeName;
                                    newConnection.dstPin = "resource";
                                    shaderNode.connections.push_back(newConnection);
                                    break;
                                }
                            }
                        }

                        // Remmeber that we've loaded this texture for this shader
                        {
                            LoadedTextureReference shaderTextureLoaded;
                            shaderTextureLoaded.token = token;
                            shaderTextureLoaded.resourceName = textureLoadNodeName;
                            shader.loadedTextureRefs.push_back(shaderTextureLoaded);
                        }
                    }
                }
            }
        );

        // On WebGPU, RTRayGen shaders  need extra variables.
        if (shader.type == ShaderType::RTRayGen && backend == Backend::WebGPU)
        {
            // A uint3 variable for dispatchSize to emulate DispatchRaysDimensions()
            char variableName[1024];
            sprintf_s(variableName, "_dispatchSize_%s", shader.name.c_str());

            Variable newVariable;
            newVariable.name = variableName;
            newVariable.type = DataFieldType::Uint3;
            newVariable.transient = true;
            renderGraph.variables.push_back(newVariable);
            variablesAccessedUnsorted.insert(variableName);
        }

        // if no variables referenced, we are done
        if (variablesAccessedUnsorted.empty() && variableAliasesAccessedUnsorted.empty())
            return true;

        // make a sorted list of accessed variables, for determinism
        std::vector<std::string> variablesAccessed;
        for (auto it : variablesAccessedUnsorted)
            variablesAccessed.push_back(it);
        std::sort(variablesAccessed.begin(), variablesAccessed.end());
        std::vector<std::string> variableAliasessAccessed;
        for (auto it : variableAliasesAccessedUnsorted)
            variableAliasessAccessed.push_back(it);
        std::sort(variableAliasessAccessed.begin(), variableAliasessAccessed.end());

        // add a struct for this constant buffer
        {
            Struct newStruct;
            newStruct.isForShaderConstants = true;
            newStruct.name = "_" + shader.name + "CB";

            for (const std::string& variableName : variablesAccessed)
            {
                int variableIndex = GetVariableIndex(renderGraph, variableName.c_str());
                GigiAssert(variableIndex >= 0, "Could not find variable %s.\nIn %s\n", variableName.c_str(), path.c_str());
                const Variable& variable = renderGraph.variables[variableIndex];

                StructField newField;
                newField.name = variableName;
                newField.type = variable.type;
                newField.dflt = variable.dflt;
                newField.Enum = variable.Enum;
                newField.comment = variable.comment;

                // make sure there is a dflt
                if (newField.dflt.empty())
                {
                    if (newField.Enum.empty())
                    {
                        ZeroDfltIfEmpty(newField.dflt, newField.type, path);
                    }
                    else
                    {
                        int enumIndex = GetEnumIndex(renderGraph, variable.scope.c_str(), variable.Enum.c_str());
                        GigiAssert(enumIndex >= 0, "Could not find enum \"%s\"\nIn %s\n", variable.Enum.c_str(), path.c_str());
                        GigiAssert(renderGraph.enums[enumIndex].items.size() > 0, "Tried to set a dflt for field \'%s\' but the enum has no items!\nIn %s\n", newField.name.c_str(), path.c_str());
                        newField.dflt = renderGraph.enums[enumIndex].items[0].label;
                        newField.enumIndex = enumIndex;
                    }
                }

                newStruct.fields.push_back(newField);
            }

            for (const std::string& aliasName : variableAliasessAccessed)
            {
                int aliasIndex = GetVariableAliasIndex(shader, aliasName.c_str());
                GigiAssert(aliasIndex >= 0, "Could not find variable alias %s.\nIn %s\n", aliasName.c_str(), path.c_str());

                StructField newField;
                newField.name = std::string("_alias_") + aliasName;
                newField.type = shader.variableAliases[aliasIndex].type;

                ZeroDfltIfEmpty(newField.dflt, newField.type, path);

                newStruct.fields.push_back(newField);

                // Also remember that this alias is actually used
                shader.variableAliases[aliasIndex].usedInShader = true;
            }

            // re-arrange and/or pad the struct fields to conform to alignment rules
            switch (backend)
            {
                case Backend::WebGPU: AdjustStructForAlignment_WebGPU(newStruct, path, true); break;
                default: AdjustUniformStructForAlignment_DX12(newStruct, path); break;
            }

            renderGraph.structs.push_back(newStruct);
        }

        // Use this struct as a constant buffer input for this node
        {
            ShaderConstantBuffer newCB;
            newCB.structName = "_" + shader.name + "CB";
            newCB.resourceName = "_" + shader.name + "CB";
            shader.constantBuffers.push_back(newCB);
        }

        // Make a constant buffer resource node that is set from the vars
        RenderGraphNode newCBNode;
        {
            RenderGraphNode_Resource_ShaderConstants newCB;
            newCB.name = "_" + shader.name + "CB";
            newCB.originalName = newCB.name;
            newCB.structure.name = "_" + shader.name + "CB";
            for (const std::string& variableName : variablesAccessed)
            {
                SetCBFromVar newSetFromVar;
                newSetFromVar.field = variableName;
                newSetFromVar.variable.name = variableName;
                newCB.setFromVar.push_back(newSetFromVar);
            }
            newCBNode._index = RenderGraphNode::c_index_resourceShaderConstants;
            newCBNode.resourceShaderConstants = newCB;
        }

        // Hook up this constant buffer to every action node that uses this shader.
        std::vector<RenderGraphNode> nodesToAdd;
        int nodeHookupIndex = 0;
        for (RenderGraphNode& node : renderGraph.nodes)
        {
            const ShaderVariableAliases* nodeVariableAliases = nullptr;

            // compute shader
            if (node._index == RenderGraphNode::c_index_actionComputeShader)
            {
                if (node.actionComputeShader.shader.name == shader.name)
                    nodeVariableAliases = &node.actionComputeShader.shaderVariableAliases;
            }
            else if (node._index == RenderGraphNode::c_index_actionWorkGraph)
            {
                if (node.actionWorkGraph.entryShader.name != shader.name)
                    nodeVariableAliases = &node.actionWorkGraph.shaderVariableAliases;
            }
            // ray shader
            else if (node._index == RenderGraphNode::c_index_actionRayShader)
            {
                if (node.actionRayShader.shader.name == shader.name)
                    nodeVariableAliases = &node.actionRayShader.shaderVariableAliases;
            }
            // draw call
            else if (node._index == RenderGraphNode::c_index_actionDrawCall)
            {
                if (node.actionDrawCall.vertexShader.name == shader.name)
                    nodeVariableAliases = &node.actionDrawCall.vertexShaderVariableAliases;
                else if (node.actionDrawCall.pixelShader.name == shader.name)
                    nodeVariableAliases = &node.actionDrawCall.pixelShaderVariableAliases;
                else if (node.actionDrawCall.amplificationShader.name == shader.name)
                    nodeVariableAliases = &node.actionDrawCall.amplificationShaderVariableAliases;
                else if (node.actionDrawCall.meshShader.name == shader.name)
                    nodeVariableAliases = &node.actionDrawCall.meshShaderVariableAliases;
            }

            // Unknown node or shader type
            if (!nodeVariableAliases)
                continue;

            // Add a constant buffer resource.
            // Only add it once, unless there are variable aliases, in which case we need one per node.
            // Add it delayed though since we are looping through the nodes right now.
            std::string cbNodeName = "_" + shader.name + "CB";
            if (nodeHookupIndex == 0 || variableAliasessAccessed.size() > 0)
            {
                if (variableAliasessAccessed.size() > 0)
                {
                    char buffer[1024];
                    sprintf_s(buffer, "_%sCB_%i", shader.name.c_str(), nodeHookupIndex);
                    cbNodeName = buffer;
                }

                RenderGraphNode newCBNodeCopy = newCBNode;

                // Use the variable aliases to set up some more setvars
                for (const ShaderVariableAlias& alias : nodeVariableAliases->aliases)
                {
                    // only make setvars for the aliases actually used
                    int aliasIndex = GetVariableAliasIndex(shader, alias.name.c_str());
                    if (aliasIndex == -1 || !shader.variableAliases[aliasIndex].usedInShader)
                        continue;

                    SetCBFromVar newSetFromVar;
                    newSetFromVar.field = std::string("_alias_") + alias.name;
                    newSetFromVar.variable = alias.variable;
                    newCBNodeCopy.resourceShaderConstants.setFromVar.push_back(newSetFromVar);
                }

                newCBNodeCopy.resourceShaderConstants.name = cbNodeName;
                newCBNodeCopy.resourceShaderConstants.originalName = cbNodeName;
                nodesToAdd.push_back(newCBNodeCopy);
                nodeHookupIndex++;
            }

            NodePinConnection newConnection;
            newConnection.dstPin = "resource";
            newConnection.dstNode = cbNodeName;
            newConnection.srcPin = "_" + shader.name + "CB";

            switch (node._index)
            {
                case RenderGraphNode::c_index_actionComputeShader: node.actionComputeShader.connections.push_back(newConnection); break;
                case RenderGraphNode::c_index_actionWorkGraph: node.actionWorkGraph.connections.push_back(newConnection); break;
                case RenderGraphNode::c_index_actionRayShader: node.actionRayShader.connections.push_back(newConnection); break;
                case RenderGraphNode::c_index_actionDrawCall: node.actionDrawCall.connections.push_back(newConnection); break;
            }
        }

        // Add the nodes now that we are done looping
        for (const RenderGraphNode& newCB : nodesToAdd)
            renderGraph.nodes.push_back(newCB);

        return true;
    }

    bool CheckForUnusedResources(Shader& shader, const std::string& path)
    {
        if (shader.resources.size() == 0)
            return true;

        if (std::find(renderGraph.buildSettings.disableWarnings.begin(), renderGraph.buildSettings.disableWarnings.end(), GigiCompileWarning::ShaderUnusedResource) != renderGraph.buildSettings.disableWarnings.end())
            return true;

        std::string fileName = (std::filesystem::path(renderGraph.baseDirectory) / shader.fileName).string();

        std::vector<unsigned char> fileContents_;
        if (!LoadFile(fileName, fileContents_))
        {
            GigiAssert(false, "Could not load file %s.\nIn %s\n", fileName.c_str(), path.c_str());
            return false;
        }
        fileContents_.push_back(0);
        std::string fileContents = (char*)fileContents_.data();

        for (const ShaderResource& resource : shader.resources)
        {
            if (resource.name.find(ShaderAssertsVisitor::AssertUavSuffix) != std::string::npos)
                continue;

            if (fileContents.find(resource.name) == std::string::npos)
            {
                bool isLoadedTexture = false;
                for (const LoadedTextureReference& loadedTexture : shader.loadedTextureRefs)
                {
                    if (loadedTexture.resourceName == resource.name)
                    {
                        isLoadedTexture = true;
                        break;
                    }
                }
                if (!isLoadedTexture)
                {
                    // Don't warn about read only pins of texture splits not being used.
                    // We added those and are responsible for using them as needed.
                    bool isTextureSplit = false;
                    for (const WebGPU_RWTextureSplit& textureSplit : renderGraph.backendData.webGPU.RWTextureSplits)
                    {
                        if (textureSplit.pinNameR == resource.name)
                        {
                            isTextureSplit = true;
                            break;
                        }
                    }

                    if (!isTextureSplit)
                    {
                        ShowWarningMessage("shader %s (%s) does not seem to actually use resource %s.\n[%s] %s\n\n", shader.name.c_str(), fileName.c_str(), resource.name.c_str(), EnumToString(GigiCompileWarning::ShaderUnusedResource), EnumToDescription(GigiCompileWarning::ShaderUnusedResource));
                    }
                }
            }
        }

        return true;
    }

    bool Visit(Shader& shader, const std::string& path)
    {
        shader.entryPointW = ToWideString(shader.entryPoint.c_str());

        if (!HookupVariables(shader, path))
        {
            return false;
        }

        if (!GatherRayShadersUsed(shader, path))
        {
            return false;
        }

        if (!CheckForUnusedResources(shader, path))
        {
            return false;
        }

        return true;
    }

    RenderGraph& renderGraph;
    Backend backend;

    int nextLoadedTextureIndex = 0;
};

struct ResolveBackendRestrictions
{
    ResolveBackendRestrictions(RenderGraph& renderGraph_)
        : renderGraph(renderGraph_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(BackendRestriction& data, const std::string& path)
    {
        if (data.backends.empty())
        {
            data.backendFlags = (unsigned int)-1;
        }
        else
        {
            data.backendFlags = 0;
            for (Backend b : data.backends)
                data.backendFlags |= (1 << (unsigned int)b);
        }

        if (!data.isWhiteList)
            data.backendFlags = ~data.backendFlags;

        return true;
    }

    RenderGraph& renderGraph;
};

