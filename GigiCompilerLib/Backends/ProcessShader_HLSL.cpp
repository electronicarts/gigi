///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ProcessShader.h"

#include "Schemas/Types.h"
#include "GigiCompilerLib/Backends/Shared.h"

// Replace single character constants with their integral ascii value instead.
// Neither slang nor WGSL support single character literals, even though HLSL does.
// I put in a feature request for slang: https://github.com/shader-slang/slang/issues/6984
void ProcessShader_ReplaceSingleCharacterConstants(std::string& shaderCode)
{
    size_t cursor = 0;
    size_t startPos = 0;
    while ((startPos = shaderCode.find("'", cursor)) != std::string::npos)
    {
        size_t endPos = shaderCode.find("'", startPos + 1);
        if (endPos == std::string::npos)
            return;

        if (endPos - startPos != 2)
        {
            cursor = endPos;
            continue;
        }

        char intValue[64];
        sprintf_s(intValue, "%u", (unsigned int)shaderCode[startPos + 1]);

        std::string leftString = shaderCode.substr(0, startPos);
        std::string rightString = shaderCode.substr(endPos + 1);
        shaderCode = leftString + intValue + rightString;

        cursor = leftString.length() + strlen(intValue);
    }
}

void ProcessShaderOptions_HLSL::WriteVariableReference_InStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const Variable& variable)
{
    stream << "_" + options.m_shader.name + "CB." + variable.name;
}

void ProcessShaderOptions_HLSL::WriteVariableReference_NotInStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const Variable& variable)
{
    stream << "cb_" + options.m_shader.name + "CB_" + variable.name;
}

void ProcessShaderOptions_HLSL::WriteVariableAlias_InStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderVariableAliasDeclaration& alias)
{
    stream << "_" + options.m_shader.name + "CB._alias_" + alias.name;
}

void ProcessShaderOptions_HLSL::WriteVariableAlias_NotInStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderVariableAliasDeclaration& alias)
{
    stream << "cb_" + options.m_shader.name + "CB__alias_" + alias.name;
}

void ProcessShaderOptions_HLSL::WriteSamplerDefinition_Register(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderSampler& sampler)
{
    stream << "\nSamplerState " << sampler.name << " : register(s" << sampler.registerIndex << sampler.registerSpaceString << ");";
}

void ProcessShaderOptions_HLSL::WriteSamplerDefinition_NoRegister(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderSampler& sampler)
{
    stream << "\nSamplerState " << sampler.name << ";";
}

void ProcessShaderOptions_HLSL::WriteResourceDefinition(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource, bool writeRegister)
{
    switch (resource.access)
    {
        case ShaderResourceAccessType::RTScene:
        {
            stream << "\nRaytracingAccelerationStructure " << resource.name;
            if (writeRegister)
                stream << " : register(t" << resource.registerIndex << resource.registerSpaceString << ")";
            stream << ";";
            break;
        }
        case ShaderResourceAccessType::UAV:
        case ShaderResourceAccessType::SRV:
        {
            const char* typePrefix = (resource.access == ShaderResourceAccessType::UAV) ? "RW" : "";
            const char* registerType = (resource.access == ShaderResourceAccessType::UAV) ? "u" : "t";

            switch (resource.type)
            {
                case ShaderResourceType::Texture:
                {
                    const char* variablePrefix = (resource.access == ShaderResourceAccessType::UAV && resource.texture.globallyCoherent) ? "globallycoherent " : "";

                    const char* textureType = "";
                    switch (resource.texture.dimension)
                    {
                        case TextureDimensionType::Texture2D: textureType = "Texture2D"; break;
                        case TextureDimensionType::Texture2DArray: textureType = "Texture2DArray"; break;
                        case TextureDimensionType::Texture3D: textureType = "Texture3D"; break;
                        case TextureDimensionType::TextureCube:
                        {
                            textureType = (resource.access == ShaderResourceAccessType::UAV) ? "Texture2DArray" : "TextureCube";
                            break;
                        }
                        case TextureDimensionType::Texture2DMS: textureType = "Texture2DMS"; break;
                        default:
                        {
                            GigiAssert(false, "Unhandled TextureDimensionType: %s (%i)", EnumToString(resource.texture.dimension), (int)resource.texture.dimension);
                        }
                    }

                    DataFieldType viewDataFieldType;
                    if (!EnumToEnum(resource.texture.viewType, viewDataFieldType))
                    {
                        GigiAssert(false, "Could not convert TextureViewType to DataFieldType");
                    }

                    std::string viewTypePrefix;
                    std::string viewTypePostfix;
                    if (renderGraph.backend == Backend::WebGPU && resource.allowAtomicOps)
                    {
                        viewTypePrefix = "Atomic<";
                        viewTypePostfix = ">";
                    }

                    stream << "\n" << variablePrefix << typePrefix << textureType << "<" << viewTypePrefix << DataFieldTypeToShaderType(viewDataFieldType) << viewTypePostfix << "> " << resource.name;
                    if (writeRegister)
                        stream << " : register(" << registerType << resource.registerIndex << resource.registerSpaceString << ")";
                    stream << ";";

                    break;
                }
                case ShaderResourceType::Buffer:
                {
                    const char* variablePrefix = (resource.access == ShaderResourceAccessType::UAV && resource.buffer.globallyCoherent) ? "globallycoherent " : "";

                    std::string viewTypePrefix;
                    std::string viewTypePostfix;
                    if (renderGraph.backend == Backend::WebGPU && resource.allowAtomicOps)
                    {
                        viewTypePrefix = "Atomic<";
                        viewTypePostfix = ">";
                    }

                    if (resource.buffer.raw)
                    {
                        stream << "\n" << variablePrefix << typePrefix << "ByteAddressBuffer " << resource.name;
                        if (writeRegister)
                            stream << " : register(" << registerType << resource.registerIndex << resource.registerSpaceString << ")";
                        stream << ";";
                    }
                    else if (resource.buffer.typeStruct.structIndex != -1)
                    {
                        stream << "\n" << variablePrefix << typePrefix << "StructuredBuffer<" << viewTypePrefix << "Struct_" << renderGraph.structs[resource.buffer.typeStruct.structIndex].name << viewTypePostfix << "> " << resource.name;
                        if (writeRegister)
                            stream << " : register(" << registerType << resource.registerIndex << resource.registerSpaceString << ")";
                        stream << ";";
                    }
                    else
                    {
                        if (!ShaderResourceBufferIsStructuredBuffer(resource.buffer))
                        {
                            stream << "\n" << variablePrefix << typePrefix << "Buffer<" << viewTypePrefix << DataFieldTypeToShaderType(resource.buffer.type) << viewTypePostfix << "> " << resource.name;
                            if (writeRegister)
                                stream << " : register(" << registerType << resource.registerIndex << resource.registerSpaceString << ")";
                            stream << ";";
                        }
                        else
                        {
                            stream << "\n" << variablePrefix << typePrefix << "StructuredBuffer<" << viewTypePrefix << DataFieldTypeToShaderType(resource.buffer.type) << viewTypePostfix << "> " << resource.name;
                            if (writeRegister)
                                stream << " : register(" << registerType << resource.registerIndex << resource.registerSpaceString << ")";
                            stream << ";";
                        }
                    }
                    break;
                }
                default:
                {
                    GigiAssert(false, "Unhandled resource type: %i (%s) in shader %s", resource.type, EnumToString(resource.type), options.m_shader.originalName.c_str());
                    break;
                }
            }
            break;
        }
        case ShaderResourceAccessType::CBV:
        {
            options.m_writeConstantBufferDefinition(options, stream, renderGraph, resource);
            break;
        }
        default:
        {
            GigiAssert(false, "Unhandled resource access type: %s (%i)", EnumToString(resource.access), resource.access);
            break;
        }
    }
}

void ProcessShaderOptions_HLSL::WriteConstantBufferDefinition_NotInStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource)
{
    const Struct& structDesc = renderGraph.structs[resource.constantBufferStructIndex];
    for (const StructField& field : structDesc.fields)
    {
        stream << "\n" <<
            DataFieldTypeToShaderType(field.type) << " cb" << structDesc.name << "_" << field.name << ";";
    }
}

void ProcessShaderOptions_HLSL::WriteConstantBufferDefinition_CBuffer(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource)
{
    stream <<
        "\ncbuffer cb" << resource.registerIndex << " : register(b" << resource.registerIndex << ")"
        "\n{"
        "\n	Struct_" << renderGraph.structs[resource.constantBufferStructIndex].name << " " << resource.name << ";"
        "\n};"
        ;
}

void ProcessShaderOptions_HLSL::WriteConstantBufferDefinition_ConstantBuffer(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource)
{
    stream <<
        "\nConstantBuffer<Struct_" << renderGraph.structs[resource.constantBufferStructIndex].name << "> " << resource.name << " : register(b" << resource.registerIndex << resource.registerSpaceString << ");"
        ;
}

bool ProcessShaderToMemory_HLSL(const Shader& shader, const char* entryPoint, ShaderLanguage targetShaderLanguage, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const RenderGraph& renderGraph, ProcessShaderOptions_HLSL& options, const std::vector<std::string>& includeDirectories, std::string& shaderCode)
{
    bool isRayShader = (shader.type == ShaderType::RTRayGen || shader.type == ShaderType::RTClosestHit || shader.type == ShaderType::RTAnyHit || shader.type == ShaderType::RTIntersection || shader.type == ShaderType::RTMiss);

    std::unordered_map<std::string, std::ostringstream> shaderSpecificStringReplacementMap;

    // Handle variables
    for (const Variable& variable : renderGraph.variables)
    {
        if (variable.scope != shader.scope)
            continue;

        std::string key = "/*$(Variable:" + variable.originalName + ")*/";
        shaderSpecificStringReplacementMap[key] = std::ostringstream();
        if (variable.Const)
            shaderSpecificStringReplacementMap[key] << "(" + variable.dflt + ")";
        else
        {
            // cast to bool to avoid the warning about casting from uint to bool
            if (variable.type == DataFieldType::Bool)
                shaderSpecificStringReplacementMap[key] << "(bool)";

            options.m_writeVariableReference(options, shaderSpecificStringReplacementMap[key], variable);
        }
    }

    // Handle Variable Aliases
    for (const ShaderVariableAliasDeclaration& alias : shader.variableAliases)
    {
        if (!alias.usedInShader)
            continue;

        std::string cast = (alias.type == DataFieldType::Bool ? "(bool)" : "");

        shaderSpecificStringReplacementMap["__Post__ShaderResources"] <<
            "\n" <<
            DataFieldTypeToShaderType(alias.type) << " _GetVariableAliasValue_" << alias.name << "()\n"
            "{\n"
            "    #ifdef __GIGI_AlIAS_VARIABLE_CONST_" << alias.name << "\n"
            "        return " << cast << "__GIGI_AlIAS_VARIABLE_CONST_" << alias.name << ";\n"
            "    #else\n"
            "        return " << cast
            ;

        options.m_writeVariableAlias(options, shaderSpecificStringReplacementMap["__Post__ShaderResources"], alias);

        shaderSpecificStringReplacementMap["__Post__ShaderResources"] <<
            ";\n"
            "    #endif\n"
            "}\n"
            ;

        std::string key = "/*$(VariableAlias:" + alias.name + ")*/";
        shaderSpecificStringReplacementMap[key] = std::ostringstream();
        shaderSpecificStringReplacementMap[key] << "_GetVariableAliasValue_" << alias.name << "()";
    }

    // Handle replaced variables
    for (const VariableReplacement& replacement : renderGraph.variableReplacements)
    {
        if (replacement.srcScope != shader.scope)
            continue;

        int variableIndex = GetScopedVariableIndex(renderGraph, replacement.destName.c_str());
        if (variableIndex == -1)
        {
            GigiAssert(false, "Could not find variable %s that replaced variable %s%s", replacement.destName.c_str(), replacement.srcScope.c_str(), replacement.srcName.c_str());
            return false;
        }

        const Variable& variable = renderGraph.variables[variableIndex];

        std::string key = "/*$(Variable:" + replacement.srcName + ")*/";
        shaderSpecificStringReplacementMap[key] = std::ostringstream();
        if (variable.Const)
            shaderSpecificStringReplacementMap[key] << "(" + variable.dflt + ")";
        else
        {
            // cast to bool to avoid the warning about casting from uint to bool
            if (variable.type == DataFieldType::Bool)
                shaderSpecificStringReplacementMap[key] << "(bool)";

            options.m_writeVariableReference(options, shaderSpecificStringReplacementMap[key], variable);
        }
    }

    std::string srcFileName = (std::filesystem::path(renderGraph.baseDirectory) / shader.fileName).string();
    std::vector<char> shaderFile;
    std::vector<std::string> embeddedFiles;
    if (!LoadAndPreprocessTextFile(srcFileName, shaderFile, renderGraph, embeddedFiles))
    {
        GigiAssert(false, "Could not load file %s", srcFileName.c_str());
        return false;
    }
    shaderFile.push_back(0);

    // Handle atomic ops from hlsl that are not available in slang
    if (renderGraph.backend == Backend::WebGPU)
    {
        // Map https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/interlockedadd
        // To https://github.com/shader-slang/spec/blob/main/proposals/003-atomic-t.md
        // Slang atomics: https://docs.shader-slang.org/en/latest/external/core-module-reference/types/atomic-0/index.html
        shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
            "#define InterlockedAdd(value, param, oldValue) oldValue = value.add(param)\n"
            "#define InterlockedAnd(value, param, oldValue) oldValue = value.and(param)\n"
            "#define InterlockedMax(value, param, oldValue) oldValue = value.max(param)\n"
            "#define InterlockedMin(value, param, oldValue) oldValue = value.min(param)\n"
            "#define InterlockedOr(value, param, oldValue) oldValue = value.or(param)\n"
            "#define InterlockedXor(value, param, oldValue) oldValue = value.xor(param)\n"
            // Doesn't exist in slang
            //"#define InterlockedCompareStore(value, param_compare, param_value)"

            // I tried to make 2 parameter versions that don't care about the oldValue.
            // To be able to have overloading based on parameter count, i needed to make them functions instead of macros.
            // It complains about the webgpu generated code when i do the below, saying that atomic variables must have storage or workgroup address space
            /*
            "void InterlockedAdd<T:IArithmeticAtomicable>(inout Atomic<T> value, in T param, inout T oldValue) { oldValue = value.add(param); }\n"
            "void InterlockedAdd<T:IArithmeticAtomicable>(inout Atomic<T> value, in T param) { value.add(param); }\n"
            "\n"
            "void InterlockedMax<T:IArithmeticAtomicable>(inout Atomic<T> value, in T param, inout T oldValue) { oldValue = value.max(param); }\n"
            "void InterlockedMax<T:IArithmeticAtomicable>(inout Atomic<T> value, in T param) { value.max(param); }\n"
            "\n"
            "void InterlockedMin<T:IArithmeticAtomicable>(inout Atomic<T> value, in T param, inout T oldValue) { oldValue = value.min(param); }\n"
            "void InterlockedMin<T:IArithmeticAtomicable>(inout Atomic<T> value, in T param) { value.min(param); }\n"
            "\n"
            "void InterlockedAnd<T:IBitAtomicable>(inout Atomic<T> value, in T param, inout T oldValue) { oldValue = value.and(param); }\n"
            "void InterlockedAnd<T:IBitAtomicable>(inout Atomic<T> value, in T param) { value.and(param); }\n"
            "\n"
            "void InterlockedOr<T:IBitAtomicable>(inout Atomic<T> value, in T param, inout T oldValue) { oldValue = value.or(param); }\n"
            "void InterlockedOr<T:IBitAtomicable>(inout Atomic<T> value, in T param) { value.or(param); }\n"
            "\n"
            "void InterlockedXor<T:IBitAtomicable>(inout Atomic<T> value, in T param, inout T oldValue) { oldValue = value.xor(param); }\n"
            "void InterlockedXor<T:IBitAtomicable>(inout Atomic<T> value, in T param) { value.xor(param); }\n"
            "\n"
            */
            ;
    }

    // write out enums
    {
        for (const auto& e : renderGraph.enums)
        {
            if (e.scope != shader.scope)
                continue;

            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                "\n"
                "\nstruct " << e.originalName <<
                "\n{"
                ;

            int itemIndex = -1;
            for (const auto& item : e.items)
            {
                itemIndex++;
                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                    "\n    static const int " << item.label << " = " << itemIndex << ";"
                    ;

                if (!item.comment.empty())
                    shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << " // " << item.comment;
            }

            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                "\n};"
                ;
        }
    }

    // write out any structs needed
    {
        std::unordered_set<int> structsWritten;
        for (size_t resourceIndex = 0; resourceIndex < shader.resources.size(); ++resourceIndex)
        {
            const ShaderResource& resource = shader.resources[resourceIndex];

            // Write out structs
            int structIndex = -1;
            if (resource.access == ShaderResourceAccessType::CBV)
                structIndex = resource.constantBufferStructIndex;
            else if (resource.type == ShaderResourceType::Buffer)
                structIndex = resource.buffer.typeStruct.structIndex;

            // only write structs out once
            if (structIndex == -1 || structsWritten.count(structIndex) > 0)
                continue;
            structsWritten.insert(structIndex);

            const Struct& s = renderGraph.structs[structIndex];

            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                "\n"
                "\nstruct Struct_" << s.name <<
                "\n{"
                ;

            for (const StructField& field : s.fields)
            {
                std::string viewTypePrefix;
                std::string viewTypePostfix;
                if (renderGraph.backend == Backend::WebGPU && field.allowAtomicOps)
                {
                    viewTypePrefix = "Atomic<";
                    viewTypePostfix = ">";
                }

                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                    "\n    " << viewTypePrefix << DataFieldTypeToShaderType(field.type) << viewTypePostfix << " " << field.name << ";"
                    ;
            }

            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << "\n};";
        }
        shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << "\n";
    }

    // Write out samplers
    for (size_t samplerIndex = 0; samplerIndex < shader.samplers.size(); ++samplerIndex)
        options.m_writeSamplerDefinition(options, shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"], shader.samplers[samplerIndex]);

    // write out resource declarations
    for (const ShaderResource& resource : shader.resources)
        options.m_writeResourceDefinition(options, shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"], renderGraph, resource);
    shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << "\n";

    // Handle string replacement for any references to loaded textures
    for (const LoadedTextureReference& loadedTexture : shader.loadedTextureRefs)
        shaderSpecificStringReplacementMap[loadedTexture.token] << loadedTexture.resourceName;

    // Put custom tokens in, else they will get filled in with an empty string by logic lower down
    for (const CustomGigiToken& token : renderGraph.customTokens)
        shaderSpecificStringReplacementMap[std::string("/*$(") + token.key + std::string(")*/")] << token.value;

    // Handle shader markup
    shaderSpecificStringReplacementMap["/*$(RayTraceFn)*/"] << options.m_rayTraceFnName;
    shaderCode = (char*)shaderFile.data();
    ForEachToken(shaderCode.c_str(),
        [&](const std::string& token, const char* stringStart, const char* cursor)
        {
            size_t lineNumber = CountLineNumber(stringStart, cursor);
            std::string declareLineNumber;
            if (options.m_writeOriginalLineNumbers)
            {
                std::ostringstream stream;
                stream << "#line " << lineNumber << "\n";
                declareLineNumber = stream.str();
            }

            // Give token over-riding first wack at it.
            if (options.m_handleShaderToken(options, shaderSpecificStringReplacementMap[token], token, lineNumber, renderGraph, declareLineNumber))
                return;

            // Otherwise, do common functionality
            std::string param;
            if (token == "/*$(ShaderResources)*/")
            {
                // Apply "__Post__ShaderResources" to "/*$(ShaderResources)*/" and clear it out
                shaderSpecificStringReplacementMap[token] << shaderSpecificStringReplacementMap["__Post__ShaderResources"].str();
                shaderSpecificStringReplacementMap["__Post__ShaderResources"] = std::ostringstream();

                if (options.m_writeOriginalLineNumbers)
                {
                    std::string old = shaderSpecificStringReplacementMap[token].str();
                    shaderSpecificStringReplacementMap[token] = std::ostringstream();
                    shaderSpecificStringReplacementMap[token] <<
                        old << "\n" <<
                        declareLineNumber
                        ;
                }
            }
            else if (GetTokenParameter(token.c_str(), "_workgraph", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    declareLineNumber <<
                    "void " << param;
            }
            else if (GetTokenParameter(token.c_str(), "_compute", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[numthreads(" << shader.NumThreads[0] << ", " << shader.NumThreads[1] << ", " << shader.NumThreads[2] << ")]\n"
                    << declareLineNumber <<
                    "void " << param;
            }
            else if (GetTokenParameter(token.c_str(), "_amplification", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[numthreads(" << shader.NumThreads[0] << ", " << shader.NumThreads[1] << ", " << shader.NumThreads[2] << ")]\n"
                    << declareLineNumber <<
                    "void " << param;
            }
            else if (GetTokenParameter(token.c_str(), "_mesh", param))
            {
                std::string entryPoint = param;
                std::string topology = "triangle";

                // Get the optional attribute structure argument if present
                size_t colonLoc = param.find_first_of(':');
                if (colonLoc != std::string::npos && colonLoc + 1 < param.length() && colonLoc > 0)
                {
                    entryPoint = param.substr(0, colonLoc);
                    topology = param.substr(colonLoc + 1);
                }

                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[numthreads(" << shader.NumThreads[0] << ", " << shader.NumThreads[1] << ", " << shader.NumThreads[2] << ")]\n"
                    "[OutputTopology(\"" << topology << "\")]\n"
                    << declareLineNumber <<
                    "void " << entryPoint;
            }
            else if (GetTokenParameter(token.c_str(), "_raygeneration", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"raygeneration\")]\n"
                    << declareLineNumber <<
                    "void " << param << "()";
            }
            else if (GetTokenParameter(token.c_str(), "_miss", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"miss\")]\n"
                    << declareLineNumber <<
                    "void " << param << "(inout Payload payload : SV_RayPayload)";
            }
            else if (GetTokenParameter(token.c_str(), "_anyhit", param))
            {
                std::string entryPoint = param;
                std::string attribStruct = "BuiltInTriangleIntersectionAttributes";

                // Get the optional attribute structure argument if present
                size_t colonLoc = param.find_first_of(':');
                if (colonLoc != std::string::npos && colonLoc + 1 < param.length() && colonLoc > 0)
                {
                    entryPoint = param.substr(0, colonLoc);
                    attribStruct = param.substr(colonLoc + 1);
                }

                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"anyhit\")]\n"
                    << declareLineNumber <<
                    "void " << entryPoint << "(inout Payload payload, in " << attribStruct << " attr)";
            }
            else if (GetTokenParameter(token.c_str(), "_intersection", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"intersection\")]\n"
                    << declareLineNumber <<
                    "void " << param << "()";
            }
            else if (GetTokenParameter(token.c_str(), "_closesthit", param))
            {
                std::string entryPoint = param;
                std::string attribStruct = "BuiltInTriangleIntersectionAttributes";

                // Get the optional attribute structure argument if present
                size_t colonLoc = param.find_first_of(':');
                if (colonLoc != std::string::npos && colonLoc + 1 < param.length() && colonLoc > 0)
                {
                    entryPoint = param.substr(0, colonLoc);
                    attribStruct = param.substr(colonLoc + 1);
                }

                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"closesthit\")]\n"
                    << declareLineNumber <<
                    "void " << entryPoint << "(inout Payload payload : SV_RayPayload, in " << attribStruct << " intersection : SV_IntersectionAttributes)";
            }
            else if (GetTokenParameter(token.c_str(), "RTHitGroupIndex", param))
            {
                int foundIndex = -1;
                for (int i = 0; i < shader.Used_RTHitGroupIndex.size(); ++i)
                {
                    int HGIndex = GetHitGroupIndex(renderGraph, shader.Used_RTHitGroupIndex[i].c_str());
                    if (HGIndex < 0)
                        continue;

                    const RTHitGroup& hitGroup = renderGraph.hitGroups[HGIndex];
                    if (shader.scope == hitGroup.scope && param == hitGroup.originalName)
                    {
                        foundIndex = i;
                        break;
                    }
                    if (foundIndex != -1)
                        break;
                }

                GigiAssert(foundIndex != -1, "Could not find RTHitGroupIndex for \"%s\" in shader \"%s\"", param.c_str(), shader.name.c_str());
                if (foundIndex != -1)
                {
                    shaderSpecificStringReplacementMap[token] = std::ostringstream();
                    shaderSpecificStringReplacementMap[token] << foundIndex;
                }
            }
            else if (token == "/*$(RTHitGroupCount)*/")
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] << renderGraph.hitGroups.size();
            }
            else if (GetTokenParameter(token.c_str(), "RTMissIndex", param))
            {
                // All others:
                int foundIndex = -1;
                for (int i = 0; i < shader.Used_RTMissIndex.size(); ++i)
                {
                    int MissIndex = GetShaderIndex(renderGraph, shader.Used_RTMissIndex[i].c_str());
                    if (MissIndex < 0)
                        continue;

                    const Shader& missShader = renderGraph.shaders[MissIndex];
                    if (shader.scope == missShader.scope && param == missShader.originalName)
                    {
                        foundIndex = i;
                        break;
                    }
                    if (foundIndex != -1)
                        break;
                }

                GigiAssert(foundIndex != -1, "Could not find RTMissIndex for \"%s\" in shader \"%s\"", param.c_str(), shader.name.c_str());
                if (foundIndex != -1)
                {
                    shaderSpecificStringReplacementMap[token] = std::ostringstream();
                    shaderSpecificStringReplacementMap[token] << foundIndex;
                }
            }
            else if (token == "/*$(NumThreads)*/")
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] << "uint3(" << shader.NumThreads[0] << "," << shader.NumThreads[1] << "," << shader.NumThreads[2] << ")";
            }
            else if (token == "/*$(DispatchMultiply)*/")
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] << "__GigiDispatchMultiply";
            }
            else if (token == "/*$(DispatchDivide)*/")
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] << "__GigiDispatchDivide";
            }
            else if (token == "/*$(DispatchPreAdd)*/")
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] << "__GigiDispatchPreAdd";
            }
            else if (token == "/*$(DispatchPostAdd)*/")
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] << "__GigiDispatchPostAdd";
            }
            else if (GetTokenParameter(token.c_str(), "RWTextureR", param) || GetTokenParameter(token.c_str(), "RWTextureW", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] << param;
            }
        }
    );

    for (const TokenReplacement& replacement : shader.tokenReplacements)
        shaderSpecificStringReplacementMap[replacement.name] << replacement.value;

    // Do any other string replacement that might want to be done
    options.m_handleGeneralStringReplacement(shaderCode, shaderSpecificStringReplacementMap, options, renderGraph);

    // Replace the strings
    ProcessStringReplacement(shaderCode, shaderSpecificStringReplacementMap, stringReplacementMap, renderGraph);

    // Convert to HLSL if it isn't already.
    if (shader.language != targetShaderLanguage)
    {
        // Convert from slang to hlsl
        if (shader.language == ShaderLanguage::Slang && targetShaderLanguage == ShaderLanguage::HLSL)
        {
            const char* shaderModel = nullptr;
            const char* stage = nullptr;
            switch (shader.type)
            {
                case ShaderType::Compute: shaderModel = renderGraph.settings.dx12.shaderModelCs.c_str(); stage = "compute"; break;
                case ShaderType::Vertex: shaderModel = renderGraph.settings.dx12.shaderModelVs.c_str(); stage = "vertex"; break;
                case ShaderType::Pixel: shaderModel = renderGraph.settings.dx12.shaderModelPs.c_str(); stage = "fragment"; break;
                default:
                {
                    GigiAssert(false, "Unhandled shader type (%s) in " __FUNCTION__, EnumToString(shader.type));
                    break;
                }
            }

            if (stage)
            {
                std::string errorMessage;
                if (!ConvertShaderSourceCode(shaderCode, shader.fileName.c_str(), shaderModel, stage, entryPoint, includeDirectories, shader.language, targetShaderLanguage, errorMessage, shader.defines, shader.slangOptions))
                    ShowErrorMessage("\"%s\" converting from \"%s\" to \"%s\"\n%s\n", shader.fileName.c_str(), EnumToString(shader.language), EnumToString(targetShaderLanguage), errorMessage.c_str());
                else if (!errorMessage.empty())
                    ShowWarningMessage("\"%s\" converting from \"%s\" to \"%s\"\n%s\n", shader.fileName.c_str(), EnumToString(shader.language), EnumToString(targetShaderLanguage), errorMessage.c_str());
            }

            // Comment out the code that slang puts in
            // Don't delete it, in case it ever turns out to be useful.
            {
                const char* codeToDelete =
                    "#pragma pack_matrix(row_major)\n"
                    "#ifdef SLANG_HLSL_ENABLE_NVAPI\n"
                    "#include \"nvHLSLExtns.h\"\n"
                    "#endif\n"
                    "\n"
                    "#ifndef __DXC_VERSION_MAJOR\n"
                    "// warning X3557: loop doesn't seem to do anything, forcing loop to unroll\n"
                    "#pragma warning(disable : 3557)\n"
                    "#endif\n"
                    ;

                const char* replacement =
                    "//#pragma pack_matrix(row_major)\n"
                    "//#ifdef SLANG_HLSL_ENABLE_NVAPI\n"
                    "//#include \"nvHLSLExtns.h\"\n"
                    "//#endif\n"
                    "//\n"
                    "//#ifndef __DXC_VERSION_MAJOR\n"
                    "// warning X3557: loop doesn't seem to do anything, forcing loop to unroll\n"
                    "//#pragma warning(disable : 3557)\n"
                    "//#endif\n"
                    ;

                StringReplaceAll(shaderCode, codeToDelete, replacement);
            }
        }
        // convert from slang or hlsl to wgsl
        else if (targetShaderLanguage == ShaderLanguage::WGSL)
        {
            // Process the shader to be ok for slang
            ProcessShader_ReplaceSingleCharacterConstants(shaderCode);

            const char* shaderModel = nullptr;
            const char* stage = nullptr;
            switch (shader.type)
            {
                case ShaderType::Compute: stage = "compute"; break;
                case ShaderType::Vertex: stage = "vertex"; break;
                case ShaderType::Pixel: stage = "fragment"; break;
                case ShaderType::RTRayGen: stage = "compute"; break; // webgpu / wgsl has no ray tracing shaders, so we emulate them with compute.
                default:
                {
                    GigiAssert(false, "Unhandled shader type (%s) in " __FUNCTION__, EnumToString(shader.type));
                    break;
                }
            }

            if (stage)
            {
                std::string errorMessage;
                if (!ConvertShaderSourceCode(shaderCode, shader.fileName.c_str(), shaderModel, stage, entryPoint, includeDirectories, shader.language, targetShaderLanguage, errorMessage, shader.defines, shader.slangOptions))
                    ShowErrorMessage("\"%s\" converting from \"%s\" to \"%s\"\n%s\n", shader.fileName.c_str(), EnumToString(shader.language), EnumToString(targetShaderLanguage), errorMessage.c_str());
                else if (!errorMessage.empty())
                    ShowWarningMessage("\"%s\" converting from \"%s\" to \"%s\"\n%s\n", shader.fileName.c_str(), EnumToString(shader.language), EnumToString(targetShaderLanguage), errorMessage.c_str());
            }
        }
        else
        {
            GigiAssert(false, "Could not convert shader from \"%s\" to \"%s\"", EnumToString(shader.language), EnumToString(targetShaderLanguage));
        }
    }

    // handle obligatory includes. Do it after slang runs, so slang doesn't error when it can't find it
    shaderCode.insert(0, options.m_topOfShader.c_str());

    return true;
}

bool ProcessShader_HLSL(const Shader& shader, const char* entryPoint, ShaderLanguage targetShaderLanguage, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const RenderGraph& renderGraph, ProcessShaderOptions_HLSL& options, const char* outFileName)
{
    std::filesystem::path includeDirectory(outFileName);
    includeDirectory.replace_filename("");

    std::vector<std::string> includeDirectories;
    includeDirectories.push_back(includeDirectory.string());

    // Process the shader code in memory
    std::string shaderCode;
    if (!ProcessShaderToMemory_HLSL(shader, entryPoint, targetShaderLanguage, stringReplacementMap, renderGraph, options, includeDirectories, shaderCode))
        return false;

    // Write the shader file out
    WriteFileIfDifferent(outFileName, shaderCode);
    return true;
}

