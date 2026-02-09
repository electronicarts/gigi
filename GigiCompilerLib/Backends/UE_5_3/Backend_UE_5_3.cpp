///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Schemas/Types.h"
#include "GigiCompilerLib/Backends/Shared.h"
#include "GigiCompilerLib/Backends/GraphViz.h"
#include "GigiCompilerLib/Backends/ProcessShader.h"
#include "GigiCompilerLib/GigiBuildFlavor.h"
#include "GigiCompilerLib/ParseCSV.h"

#include <unordered_set>
#include <process.h>

struct BackendUE_5_3 : public BackendBase
{
    struct BufferNodeType
    {
        // If the buffer is a specific type
        DataFieldType type = DataFieldType::Count;

        // If the buffer uses a struct
        int structIndex = -1;

        // If the buffer uses the type of an imported resource (so, could be anything)
        int importedNodeIndex = -1;
    };

    struct TextureNodeFormat
    {
        // If the texture is a specific type
        TextureFormat format = TextureFormat::Any;

        // If the texture uses a variable
        int variableIndex = -1;

        // If the texture uses the type of an imported resource (so, could be anything)
        int importedNodeIndex = -1;
    };

    static std::string StringEscape(const std::string& s)
    {
        std::string ret = s;
        StringReplaceAll(ret, "\\", "\\\\");
        return ret;
    }

    static BufferNodeType GetBufferNodeType(const RenderGraph& renderGraph, const RenderGraphNode_Resource_Buffer& node)
    {
        BufferNodeType ret;

        const RenderGraphNode_Resource_Buffer* buffNode = &node;
        while (1)
        {
            if (buffNode->format.structureType.structIndex != -1)
            {
                ret.structIndex = buffNode->format.structureType.structIndex;
                return ret;
            }
            else if (buffNode->visibility == ResourceVisibility::Imported)
            {
                ret.importedNodeIndex = buffNode->nodeIndex;
                return ret;
            }
            else if (buffNode->format.node.nodeIndex != -1)
            {
                const RenderGraphNode& buffNodeBase = renderGraph.nodes[buffNode->format.node.nodeIndex];
                GigiAssert(buffNodeBase._index == RenderGraphNode::c_index_resourceBuffer, "Must be a buffer!");
                buffNode = &buffNodeBase.resourceBuffer;
            }
            else
            {
                ret.type = buffNode->format.type;
                return ret;
            }
        }
    }

    static TextureNodeFormat GetTextureNodeFormat(const RenderGraph& renderGraph, const RenderGraphNode_Resource_Texture& node)
    {
        TextureNodeFormat ret;

        const RenderGraphNode_Resource_Texture* texNode = &node;
        while (1)
        {
            if (texNode->visibility == ResourceVisibility::Imported)
            {
                ret.importedNodeIndex = texNode->nodeIndex;
                return ret;
            }
            else if (texNode->format.node.nodeIndex != -1)
            {
                const RenderGraphNode& texNodeBase = renderGraph.nodes[texNode->format.node.nodeIndex];
                GigiAssert(texNodeBase._index == RenderGraphNode::c_index_resourceTexture, "Must be a texture!");
                texNode = &texNodeBase.resourceTexture;
            }
            else if (texNode->format.variable.variableIndex != -1)
            {
                ret.variableIndex = texNode->format.variable.variableIndex;
                return ret;
            }
            else
            {
                ret.format = texNode->format.format;
                return ret;
            }
        }
    }

    static std::string MakeShaderDefines(const Shader& shader)
    {
        std::ostringstream ret;

        bool firstDefine = true;
        for (const ShaderDefine& shaderDefine : shader.defines)
        {
            ret << (firstDefine ? "\n" : "") << "\n        OutEnvironment.SetDefine(TEXT(\"" << shaderDefine.name << "\"), TEXT(\"" << shaderDefine.value << "\"));";
            firstDefine = false;
        }

        return ret.str();
    }

    template <typename TNode>
    static std::string MakeShaderParams(const RenderGraph& renderGraph, const TNode& node, const std::vector<ResourceDependency>& dependencies, const Shader& shader, size_t depsIndexStart, size_t depsCount)
    {
        std::ostringstream shaderParams;

        for (size_t depIndex = depsIndexStart; depIndex < (depsIndexStart + depsCount); ++depIndex)
        {
            unsigned int bufferViewBegin = 0;
            int bufferViewBeginVarIndex = -1;
            unsigned int bufferViewSize = 0;
            int bufferViewSizeVarIndex = -1;
            bool bufferViewInBytes = false;
            int UAVMipIndex = 0;
            {
                size_t pinIndex = depIndex;
                if (pinIndex < node.linkProperties.size())
                {
                    const LinkProperties& linkProperties = node.linkProperties[pinIndex];
                    UAVMipIndex = linkProperties.UAVMipIndex;
                    bufferViewBegin = linkProperties.bufferViewBegin;
                    bufferViewBeginVarIndex = linkProperties.bufferViewBeginVariable.variableIndex;
                    bufferViewSize = linkProperties.bufferViewSize;
                    bufferViewSizeVarIndex = linkProperties.bufferViewSizeVariable.variableIndex;
                    bufferViewInBytes = (linkProperties.bufferViewUnits == MemoryUnitOfMeasurement::Bytes);
                }
            }

            const ResourceDependency& dep = dependencies[depIndex];
            size_t resourceIndex = depIndex - depsIndexStart;

            RenderGraphNode depNodeBase = renderGraph.nodes[dep.nodeIndex];

            switch (dep.type)
            {
                case ShaderResourceType::Texture:
                {
                    RenderGraphNode_Resource_Texture& depNode = depNodeBase.resourceTexture;

                    if (dep.access == ShaderResourceAccessType::UAV && UAVMipIndex > 0)
                        shaderParams << "        TODO: make the below use UAV Mip " << UAVMipIndex << "\n";

                    std::string macro = "";
                    std::string typePrefix = "";
                    switch (dep.access)
                    {
                        case ShaderResourceAccessType::SRV: macro = "SHADER_PARAMETER_RDG_TEXTURE"; typePrefix = ""; break;
                        case ShaderResourceAccessType::UAV: macro = "SHADER_PARAMETER_RDG_TEXTURE_UAV"; typePrefix = "RW"; break;
                        default:
                        {
                            GigiAssert(false, "Unhandled dependency access in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(dep.access));
                            break;
                        }
                    }

                    std::string type = "";
                    switch (depNode.dimension)
                    {
                        case TextureDimensionType::Texture2D: type = "Texture2D"; break;
                        case TextureDimensionType::Texture2DArray: type = "Texture2DArray"; break;
                        case TextureDimensionType::Texture3D: type = "Texture3D"; break;
                        case TextureDimensionType::TextureCube: type = "TextureCube"; break;
                        default:
                        {
                            GigiAssert(false, "Unhandled texture dimension in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(depNode.dimension));
                            break;
                        }
                    }

                    std::string viewType = "";
                    switch (shader.resources[resourceIndex].texture.viewType)
                    {
                        case TextureViewType::Uint: viewType = "<uint>"; break;
                        case TextureViewType::Uint4: viewType = "<uint4>"; break;
                        case TextureViewType::Float: viewType = "<float>"; break;
                        case TextureViewType::Float2: viewType = "<float2>"; break;
                        case TextureViewType::Float3: viewType = "<float3>"; break;
                        case TextureViewType::Float4: viewType = "<float4>"; break;
                        default:
                        {
                            GigiAssert(false, "Unhandled texture view type in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(shader.resources[resourceIndex].texture.viewType));
                            break;
                        }
                    }

                    shaderParams << "        " << macro << "(" << typePrefix << type << viewType << ", " << shader.resources[resourceIndex].name << ")\n";
                    break;
                }
                case ShaderResourceType::Buffer:
                {
                    RenderGraphNode_Resource_Buffer& depNode = depNodeBase.resourceBuffer;

                    if (bufferViewBegin > 0 || bufferViewSize > 0 || bufferViewBeginVarIndex != -1 || bufferViewSizeVarIndex != -1)
                    {
                        shaderParams << "        TODO: make the below use a view of the buffer starting at ";

                        if (bufferViewBeginVarIndex != -1)
                            shaderParams << VariableToString(renderGraph.variables[bufferViewBeginVarIndex], renderGraph);
                        else
                            shaderParams << bufferViewBegin;

                        shaderParams << " with a size of ";

                        if (bufferViewSizeVarIndex != -1)
                            shaderParams << VariableToString(renderGraph.variables[bufferViewSizeVarIndex], renderGraph);
                        else
                            shaderParams << bufferViewSize;

                        shaderParams << " (this line is described in " << (bufferViewInBytes ? "BYTES" : "ITEM COUNT") << ")\n";
                    }

                    if (dep.access == ShaderResourceAccessType::Indirect)
                    {
                        shaderParams << "        RDG_BUFFER_ACCESS(IndirectDispatchArgsBuffer, ERHIAccess::IndirectArgs)\n";
                    }
                    else if (dep.access == ShaderResourceAccessType::RTScene)
                    {
                        shaderParams << "        SHADER_PARAMETER_RDG_BUFFER_SRV(RaytracingAccelerationStructure, " << shader.resources[resourceIndex].name << ")\n";
                    }
                    else
                    {
                        const ShaderResource& resource = shader.resources[resourceIndex];

                        std::string macro = "";
                        std::string typePrefix = "";
                        switch (dep.access)
                        {
                            case ShaderResourceAccessType::SRV: macro = "SHADER_PARAMETER_RDG_BUFFER_SRV"; typePrefix = ""; break;
                            case ShaderResourceAccessType::UAV: macro = "SHADER_PARAMETER_RDG_BUFFER_UAV"; typePrefix = "RW"; break;
                            default:
                            {
                                GigiAssert(false, "Unhandled dependency access in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(dep.access));
                                break;
                            }
                        }

                        std::string type;
                        std::string viewType;
                        if (resource.buffer.raw)
                        {
                            type = "ByteAddressBuffer";
                        }
                        else if (resource.buffer.typeStruct.structIndex != -1)
                        {
                            type = "StructuredBuffer";
                            viewType = std::string("<") + renderGraph.structs[resource.buffer.typeStruct.structIndex].name + std::string(">");
                        }
                        else
                        {
                            if (!ShaderResourceBufferIsStructuredBuffer(resource.buffer))
                            {
                                type = "Buffer";
                            }
                            else
                            {
                                type = "StructuredBuffer";
                            }

                            switch (shader.resources[resourceIndex].buffer.type)
                            {
                                case DataFieldType::Int: viewType = "<int>"; break;
                                case DataFieldType::Int2: viewType = "<int2>"; break;
                                case DataFieldType::Int3: viewType = "<int3>"; break;
                                case DataFieldType::Int4: viewType = "<int4>"; break;
                                case DataFieldType::Uint: viewType = "<int>"; break;
                                case DataFieldType::Uint2: viewType = "<uint2>"; break;
                                case DataFieldType::Uint3: viewType = "<uint3>"; break;
                                case DataFieldType::Uint4: viewType = "<uint4>"; break;
                                case DataFieldType::Float: viewType = "<float>"; break;
                                case DataFieldType::Float2: viewType = "<float2>"; break;
                                case DataFieldType::Float3: viewType = "<float3>"; break;
                                case DataFieldType::Float4: viewType = "<float4>"; break;
                                case DataFieldType::Float_16: viewType = "<half>"; break;
                                default:
                                {
                                    GigiAssert(false, "Unhandled buffer type in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(shader.resources[resourceIndex].buffer.type));
                                    break;
                                }
                            }
                        }

                        shaderParams << "        " << macro << "(" << typePrefix << type << viewType << ", " << shader.resources[resourceIndex].name << ")\n";
                    }
                    break;
                }
                case ShaderResourceType::ConstantBuffer:
                {
                    RenderGraphNode_Resource_ShaderConstants& depNode = depNodeBase.resourceShaderConstants;
                    const Struct& structDesc = renderGraph.structs[depNode.structure.structIndex];
                    std::string fieldType = "";
                    for (const StructField& field : structDesc.fields)
                    {
                        switch (field.type)
                        {
                            case DataFieldType::Bool: fieldType = "uint32"; break;
                            case DataFieldType::Uint: fieldType = "uint32"; break;
                            case DataFieldType::Uint2: fieldType = "FUintVector2"; break;
                            case DataFieldType::Uint3: fieldType = "FUintVector3"; break;
                            case DataFieldType::Uint4: fieldType = "FUintVector4"; break;
                            case DataFieldType::Int: fieldType = "int32"; break;
                            case DataFieldType::Int2: fieldType = "FIntVector2"; break;
                            case DataFieldType::Int3: fieldType = "FIntVector3"; break;
                            case DataFieldType::Int4: fieldType = "FIntVector4"; break;
                            case DataFieldType::Float: fieldType = "float"; break;
                            case DataFieldType::Float2: fieldType = "FVector2f"; break;
                            case DataFieldType::Float3: fieldType = "FVector3f"; break;
                            case DataFieldType::Float4: fieldType = "FVector4f"; break;
                            case DataFieldType::Float4x4: fieldType = "FMatrix44f"; break;
                            case DataFieldType::Uint_16: fieldType = "uint16"; break;
                            case DataFieldType::Int_64: fieldType = "uint64"; break;
                            case DataFieldType::Uint_64: fieldType = "uint64"; break;
                            case DataFieldType::Float_16: fieldType = "uint16"; break;
                            default:
                            {
                                GigiAssert(false, "Unhandled field type in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(field.type));
                                break;
                            }
                        }
                        shaderParams << "        SHADER_PARAMETER(" << fieldType << ", cb" << depNode.structure.name << "_" << field.name << ")\n";
                    }
                    break;
                }
                default:
                {
                    GigiAssert(false, "Unhandled dependency type in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(dep.type));
                    break;
                }
            }
        }

        return shaderParams.str();
    }

    static std::string FillShaderParams(const RenderGraph& renderGraph, const std::vector<ResourceDependency>& dependencies, const Shader& shader, size_t depsIndexOffset, const std::string& indent, const std::string& structPrefix, const std::string& comment)
    {
        if (shader.resources.size() == 0)
            return "";

        std::ostringstream shaderParams;

        if (!comment.empty())
            shaderParams << indent << comment << "\n";

        for (size_t resourceIndex = 0; resourceIndex < shader.resources.size(); ++resourceIndex)
        {
            size_t depIndex = resourceIndex + depsIndexOffset;

            const ResourceDependency& dep = dependencies[depIndex];

            if (dep.access == ShaderResourceAccessType::RTScene)
                continue;

            RenderGraphNode depNodeBase = renderGraph.nodes[dep.nodeIndex];

            switch (dep.type)
            {
                case ShaderResourceType::Texture:
                {
                    RenderGraphNode_Resource_Texture& depNode = depNodeBase.resourceTexture;
                    std::string prefix, postfix;
                    if (dep.access == ShaderResourceAccessType::UAV)
                    {
                        prefix = "GraphBuilder.CreateUAV(";
                        postfix = ")";
                    }
                    shaderParams
                        << indent
                        << structPrefix
                        << shader.resources[resourceIndex].name
                        << " = " << prefix << "Texture_" << depNode.name << postfix << ";\n"
                        ;
                    break;
                }
                case ShaderResourceType::Buffer:
                {
                    RenderGraphNode_Resource_Buffer& depNode = depNodeBase.resourceBuffer;

                    BufferNodeType bufferNodeType = GetBufferNodeType(renderGraph, depNode);

                    std::string format;
                    if (bufferNodeType.importedNodeIndex != -1)
                    {
                        format = "params.inputs.BufferFormat_" + GetNodeName(renderGraph, bufferNodeType.importedNodeIndex);
                    }
                    else if (bufferNodeType.structIndex != -1)
                    {
                        format = "PF_Unknown";
                    }
                    else
                    {
                        if (DataFieldTypeIsPOD(bufferNodeType.type))
                        {
                            format = DataFieldTypeToEPixelFormat(bufferNodeType.type);
                        }
                        else
                        {
                            format = "PF_Unknown";
                        }
                    }

                    std::string prefix, postfix;
                    if (dep.access == ShaderResourceAccessType::UAV)
                    {
                        prefix = "GraphBuilder.CreateUAV(";
                        postfix = ")";
                    }
                    else
                    {
                        prefix = "GraphBuilder.CreateSRV(";
                        postfix = ")";
                    }

                    shaderParams
                        << indent
                        << structPrefix
                        << shader.resources[resourceIndex].name
                        << " = " << prefix << "Buffer_" << depNode.name << ", " << format << postfix << ";\n"
                        ;
                    break;
                }
                case ShaderResourceType::ConstantBuffer:
                {
                    RenderGraphNode_Resource_ShaderConstants& depNode = depNodeBase.resourceShaderConstants;

                    for (const SetCBFromVar& setVar : depNode.setFromVar)
                    {
                        const Variable& var = renderGraph.variables[setVar.variable.variableIndex];
                        shaderParams
                            << indent
                            << structPrefix
                            << "cb" << depNode.structure.name << "_"
                            << setVar.field
                            << " = "
                            << VariableToString(var, renderGraph)
                            << ";\n";
                    }
                    break;
                }
                default:
                {
                    GigiAssert(false, "Unhandled dependency type in shader \"%s\": \"%s\"", shader.name.c_str(), EnumToString(dep.type));
                    break;
                }
            }
        }

        shaderParams << "\n";

        return shaderParams.str();
    }

    static const char* DepthTestFunctionToECompareFunction(DepthTestFunction func)
    {
        switch (func)
        {
            case DepthTestFunction::Never: return "CF_Never";
            case DepthTestFunction::Less: return "CF_Less";
            case DepthTestFunction::Equal: return "CF_Equal";
            case DepthTestFunction::LessEqual: return "CF_LessEqual";
            case DepthTestFunction::Greater: return "CF_Greater";
            case DepthTestFunction::NotEqual: return "CF_NotEqual";
            case DepthTestFunction::GreaterEqual: return "CF_GreaterEqual";
            case DepthTestFunction::Always: return "CF_Always";
            default:
            {
                GigiAssert(false, "Unhandled DepthTestFunction: \"%s\"", EnumToString(func));
                return nullptr;
            }
        }
    }

    static const char* StencilOpToEStencilOp(StencilOp op)
    {
        switch (op)
        {
            case StencilOp::Keep: return "SO_Keep";
            case StencilOp::Zero: return "SO_Zero";
            case StencilOp::Replace: return "SO_Replace";
            case StencilOp::IncrementSaturate: return "SO_SaturatedIncrement";
            case StencilOp::DecrimentSaturate: return "SO_SaturatedDecrement";
            case StencilOp::Invert: return "SO_Invert";
            case StencilOp::Increment: return "SO_Increment";
            case StencilOp::Decriment: return "SO_Decrement";
            default:
            {
                GigiAssert(false, "Unhandled StencilOp: \"%s\"", EnumToString(op));
                return nullptr;
            }
        }
    }

    static const char* DrawBlendModeToEBlendFactor(DrawBlendMode mode)
    {
        switch (mode)
        {
            case DrawBlendMode::Zero: return "BF_Zero";
            case DrawBlendMode::One: return "BF_One";
            case DrawBlendMode::SrcColor: return "BF_SourceColor";
            case DrawBlendMode::InvSrcColor: return "BF_InverseSourceColor";
            case DrawBlendMode::SrcAlpha: return "BF_SourceAlpha";
            case DrawBlendMode::InvSrcAlpha: return "BF_InverseSourceAlpha";
            case DrawBlendMode::DestAlpha: return "BF_DestAlpha";
            case DrawBlendMode::InvDestAlpha: return "BF_InverseDestAlpha";
            case DrawBlendMode::DestColor: return "BF_DestColor";
            case DrawBlendMode::InvDestColor: return "BF_InverseDestColor";
            default:
            {
                GigiAssert(false, "Unhandled DrawBlendMode: \"%s\"", EnumToString(mode));
                return nullptr;
            }
        }
    }

    static const char* DrawCullModeToERasterizerCullMode(DrawCullMode mode, bool frontIsCounterClockwise)
    {
        switch (mode)
        {
            case DrawCullMode::None: return "CM_None";
            case DrawCullMode::Front: return (frontIsCounterClockwise ? "CM_CCW" : "CM_CW");
            case DrawCullMode::Back: return (frontIsCounterClockwise ? "CM_CW" : "CM_CCW");
            default:
            {
                GigiAssert(false, "Unhandled DrawCullMode: \"%s\"", EnumToString(mode));
                return nullptr;
            }
        }
    }

    static std::string MakeETextureCreateFlags(unsigned int accessedAs, bool sRGB)
    {
        std::ostringstream ret;

        bool firstFlag = true;
        #define HANDLE_FLAG(GIGI_FLAG, UNREAL_FLAG) \
            if (accessedAs & (1 << (unsigned int)GIGI_FLAG)) \
            { \
                if (!firstFlag) \
                    ret << " | "; \
                ret << #UNREAL_FLAG; \
                firstFlag = false; \
            }

        HANDLE_FLAG(ShaderResourceAccessType::UAV, ETextureCreateFlags::UAV);
        HANDLE_FLAG(ShaderResourceAccessType::SRV, ETextureCreateFlags::ShaderResource);
        HANDLE_FLAG(ShaderResourceAccessType::RenderTarget, ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource);
        HANDLE_FLAG(ShaderResourceAccessType::DepthTarget, ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource);

        #undef HANDLE_FLAG

        if (firstFlag)
        {
            // Setting it to none makes it fail in D3D12TransientResourceAllocator.cpp GetInitialResourceState().
            // So, setting it to UAV instead
            ret << "ETextureCreateFlags::UAV";
            //makeDesc << "ETextureCreateFlags::None";
        }

        if (sRGB)
        {
            ret << " | ETextureCreateFlags::SRGB";
        }

        return ret.str();
    }

    static std::string VariableDefaultValueCPP(const Variable& variable, const RenderGraph& renderGraph, bool enumToInt)
    {
        std::ostringstream ret;

        if (variable.type  != DataFieldType::Float4x4)
        {
            // If we should convert enums to ints and this variable uses an enum, try to do that.
            // If this process fails, fall back to writing out the dflt value plainly, as it might already be an int.
            if (enumToInt && variable.enumIndex != -1)
            {
                const Enum& e = renderGraph.enums[variable.enumIndex];
                for (int enumValue = 0; enumValue < (int)e.items.size(); ++enumValue)
                {
                    const std::string& label = e.items[enumValue].label;
                    std::string scopedLabel = e.name + std::string("::") + label;

                    if (!_stricmp(variable.dflt.c_str(), label.c_str()) || !_stricmp(variable.dflt.c_str(), scopedLabel.c_str()))
                    {
                        ret << DataFieldTypeToCPPType(variable.type) << "(" << enumValue << ")";
                        return ret.str();
                    }
                }
            }

            ret << DataFieldTypeToCPPType(variable.type) << "(" << variable.dflt << ")";

            return ret.str();
        }

        ret.precision(std::numeric_limits<float>::max_digits10);

        std::vector<float> values;
        ParseCSV::ForEachValue(variable.dflt.c_str(), false,
            [&](int tokenIndex, const char* token)
            {
                float f;
                sscanf_s(token, "%f", &f);
                values.push_back(f);
                return true;
            }
        );

        ret << DataFieldTypeToCPPType(variable.type) << "(";

        for (int i = 0; i < 4; ++i)
        {
            if (i > 0)
                ret << ", ";

            ret << "FPlane4f(";
            for (int j = 0; j < 4; ++j)
            {
                if (j > 0)
                    ret << ", ";
                ret << std::fixed << values[i * 4 + j] << "f";
            }

            ret << ")";
        }

        ret << ")";

        return ret.str();
    }

    static std::string VariableToString(const Variable& variable, const RenderGraph& renderGraph)
    {
        return std::string("View.ViewState->TechniqueState_") + renderGraph.name + std::string(".Var_") + variable.name;
    }

    static const char* DataFieldTypeToEPixelFormat(DataFieldType type)
    {
        switch (type)
        {
            case DataFieldType::Int: return "PF_R32_SINT";
            case DataFieldType::Uint: return "PF_R32_UINT";
            case DataFieldType::Float: return "PF_R32_FLOAT";
            case DataFieldType::Float_16: return "PF_R16F";
            default:
            {
                GigiAssert(false, "Unhandled DataFieldType: \"%s\"", EnumToString(type));
                return nullptr;
            }
        }
    }

    static const char* TextureFormatToEPixelFormat(TextureFormat format, bool& isSRGB)
    {
        // The commented out formats don't seem to have an equivelent in UE oddly.
        isSRGB = false;
        switch (format)
        {
            case TextureFormat::R8_Unorm: return "PF_R8";
            case TextureFormat::RG8_Unorm: return "PF_R8G8";
            case TextureFormat::RGBA8_Unorm: return "PF_R8G8B8A8";
            case TextureFormat::RGBA8_Unorm_sRGB: isSRGB = true; return "PF_R8G8B8A8"; // need to create with ETextureCreateFlags::SRGB
            //case TextureFormat::R8_Snorm:
            //case TextureFormat::RG8_Snorm:
            case TextureFormat::RGBA8_Snorm: return "PF_R8G8B8A8_SNORM";
            case TextureFormat::R8_Uint: return "PF_R8_UINT";
            case TextureFormat::RG8_Uint: return "PF_R8G8_UINT";
            case TextureFormat::RGBA8_Uint: return "PF_R8G8B8A8_UINT";
            case TextureFormat::R8_Sint: return "PF_R8_SINT";
            //case TextureFormat::RG8_Sint:
            //case TextureFormat::RGBA8_Sint:
            case TextureFormat::R16_Float: return "PF_R16F";
            //case TextureFormat::RG16_Float:
            case TextureFormat::RGBA16_Float: return "PF_FloatRGBA"; // EPixelFormat comment says this is RGBA16F
            case TextureFormat::RGBA16_Unorm: return "PF_R16G16B16A16_UNORM";
            case TextureFormat::R32_Float: return "PF_R32_FLOAT";
            //case TextureFormat::RG32_Float:
            case TextureFormat::RGBA32_Float: return "PF_A32B32G32R32F"; //  https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/D3D12RHI/Private/D3D12RHI.cpp shows that PF_A32B32G32R32F is set to DXGI_FORMAT_R32G32B32A32_FLOAT
            case TextureFormat::R32_Uint: return "PF_R32_UINT";
            case TextureFormat::RGBA32_Uint: return "PF_R32G32B32A32_UINT";
            case TextureFormat::R11G11B10_Float: return "PF_FloatR11G11B10";
            case TextureFormat::D32_Float: return "PF_R32_FLOAT";
            //case TextureFormat::D16_Unorm:
            //case TextureFormat::D32_Float_S8:
            case TextureFormat::D24_Unorm_S8: return "PF_DepthStencil";// "PF_X24_G8";
            default:
            {
                GigiAssert(false, "Unhandled TextureFormat: \"%s\"", EnumToString(format));
                return "<Unhandled TextureFormat>";
            }
        }
    }

    static std::string DataFieldTypeValue(DataFieldType type, const std::string& value)
    {
        switch(type)
        {
            case DataFieldType::Int2:
            case DataFieldType::Int3:
            case DataFieldType::Int4:
            case DataFieldType::Uint2:
            case DataFieldType::Uint3:
            case DataFieldType::Uint4:
            case DataFieldType::Float2:
            case DataFieldType::Float3:
            case DataFieldType::Float4:
            case DataFieldType::Float4x4:
            {
                std::string ret = "{" + value + "}";
                return ret;
            }
            default:
            {
                return value;
            }
        }
    }

    static std::string StructFieldDefaultValue(const StructField& field, const RenderGraph& renderGraph)
    {
        if (field.enumIndex == -1)
            return DataFieldTypeValue(field.type, field.dflt);

        return std::string("(int)") + renderGraph.enums[field.enumIndex].name + std::string("::") + field.dflt;
    }

    static const char* DataFieldTypeToCPPType(DataFieldType type)
    {
        switch (type)
        {
            case DataFieldType::Int: return "int32";
            case DataFieldType::Int2: return "FIntVector2";
            case DataFieldType::Int3: return "FIntVector3";
            case DataFieldType::Int4: return "FIntVector4";
            case DataFieldType::Uint: return "uint32";
            case DataFieldType::Uint2: return "FUintVector2";
            case DataFieldType::Uint3: return "FUintVector3";
            case DataFieldType::Uint4: return "FUintVector4";
            case DataFieldType::Float: return "float";
            case DataFieldType::Float2: return "FVector2f";
            case DataFieldType::Float3: return "FVector3f";
            case DataFieldType::Float4: return "FVector4f";
            case DataFieldType::Bool: return "bool";
            case DataFieldType::Float4x4: return "FMatrix44f";
            case DataFieldType::Uint_16: return "uint16";
            case DataFieldType::Int_64: return "int64_t";
            case DataFieldType::Uint_64: return "uint64_t";
            case DataFieldType::Float_16: return "uint16";
            default:
            {
                GigiAssert(false, "Unhandled data field type: \"%s\"", EnumToString(type));
                return nullptr;
            }
        }
    }

    static const char* DataFieldTypeToShaderType(DataFieldType type)
    {
        switch (type)
        {
            case DataFieldType::Int: return "int";
            case DataFieldType::Int2: return "int2";
            case DataFieldType::Int3: return "int3";
            case DataFieldType::Int4: return "int4";
            case DataFieldType::Uint: return "uint";
            case DataFieldType::Uint2: return "uint2";
            case DataFieldType::Uint3: return "uint3";
            case DataFieldType::Uint4: return "uint4";
            case DataFieldType::Float: return "float";
            case DataFieldType::Float2: return "float2";
            case DataFieldType::Float3: return "float3";
            case DataFieldType::Float4: return "float4";
            case DataFieldType::Bool: return "uint";
            case DataFieldType::Float4x4: return "float4x4";
            case DataFieldType::Uint_16: return "uint16_t";
            case DataFieldType::Int_64: return "int64_t";
            case DataFieldType::Uint_64: return "uint64_t";
            case DataFieldType::Float_16: return "half";
            default:
            {
                GigiAssert(false, "Unhandled data field type: \"%s\"", EnumToString(type));
                return nullptr;
            }
        }
    }

    #include "GigiCompilerLib/Backends/UE_5_3/nodes/allNodes.h"

    static bool ProcessNodeTag(const RenderGraph& renderGraph, const RenderGraphNode& nodeBase, std::string& tag, std::string& text)
    {
        // This function, or child function calls may:
        // say not to use this tag template for anything.
        // change what tag is written to.
        // change what is in the tag text

        // Common string replacement
        StringReplaceAll(text, "/*$(Name)*/", renderGraph.name);
        StringReplaceAll(text, "/*$(Node:Type)*/", GetNodeTypeString(nodeBase));
        ExecuteOnNode(nodeBase, [&](auto& node)
            {
                StringReplaceAll(text, "/*$(Node:Name)*/", node.name);
                StringReplaceAll(text, "/*$(Node:Comment)*/", node.comment);

                if (!node.comment.empty())
                {
                    StringReplaceAll(text, "/*$(Node:CommentAsComment)*/", std::string("  // ") + node.comment);
                    StringReplaceAll(text, "/*$(Node:CommentInComment)*/", std::string(" - ") + node.comment);
                }
                else
                {
                    StringReplaceAll(text, "/*$(Node:CommentAsComment)*/", "");
                    StringReplaceAll(text, "/*$(Node:CommentInComment)*/", "");
                }
            }
        );

        // /*$(ModifyCompilationEnvironment)*/
        {
            std::ostringstream modifyCompilationEnvironment;

            modifyCompilationEnvironment << "\n\n        OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);";

            if (renderGraph.settings.UE_5_3.AllowRealTypes)
                modifyCompilationEnvironment << "\n        OutEnvironment.CompilerFlags.Add(CFLAG_AllowRealTypes);";

            StringReplaceAll(text, "/*$(ModifyCompilationEnvironment)*/", modifyCompilationEnvironment.str());
        }

        // Node type specific string replacement
        bool insertTag = true;
        ExecuteOnNode(nodeBase, [&](auto& node)
            {
                insertTag = ProcessNodeTag(renderGraph, node, tag, text);
            }
        );

        return insertTag;
    }

    static void MakeStringReplacementGlobal(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, GGUserFileLatest& ggUserFile)
    {
        BackendBase::MakeStringReplacementGlobal(stringReplacementMap, renderGraph, ggUserFile);

        // Process the nodes in order
        for (int nodeIndex : renderGraph.flattenedNodeList)
        {
            const RenderGraphNode& nodeBase = renderGraph.nodes[nodeIndex];
            std::string nodeType = GetNodeTypeString(nodeBase);

            for (const BackendTemplateNodeTemplate& t : renderGraph.templateConfig.nodeTemplates)
            {
                if (_stricmp(t.nodeType.c_str(), nodeType.c_str()))
                    continue;

                std::string text = t.text;
                std::string tag = t.tag;
                if (ProcessNodeTag(renderGraph, nodeBase, tag, text))
                    stringReplacementMap[tag] << text;
            }
        }

        // Create the slang warning if we should
        // Also the Vertex Shader semantics warning
        {
            bool firstSlang = true;
            bool firstVS = true;
            for (const Shader& shader : renderGraph.shaders)
            {
                if (shader.type == ShaderType::Vertex)
                {
                    if (firstVS)
                    {
                        firstVS = false;
                        stringReplacementMap["/*Readme_SlangWarning*/"] <<
                            "=====================================================================================================\n"
                            "Modify Vertex Buffer Semantics in Shaders\n"
                            "=====================================================================================================\n"
                            "\n"
                            "If you use semantics like POSITION, NORMAL, TEXCOORD in the inputs to a vertex shader, you need to\n"
                            "update those to use ATTRIBUTE[N] semantics instead.\n"
                            "\n"
                            "For instance:\n"
                            "\n"
                            " struct VSInput\n"
                            " {\n"
                            "	float3 position : POSITION;\n"
                            "	float3 normal : NORMAL;\n"
                            "	float2 uv : TEXCOORD;\n"
                            " };\n"
                            "\n"
                            " Should become:\n"
                            "\n"
                            "  struct VSInput\n"
                            " {\n"
                            "	float3 position : ATTRIBUTE0;\n"
                            "	float3 normal : ATTRIBUTE1;\n"
                            "	float2 uv : ATTRIBUTE2;\n"
                            " };\n"
                            "\n"
                            "Shaders Files Potentially Affected:\n"
                            ;
                    }

                    stringReplacementMap["/*Readme_VSSemantics*/"] << "  * " << std::filesystem::path(shader.destFileName).replace_extension(".usf").string() << "\n";
                }

                if (shader.language == ShaderLanguage::Slang)
                {
                    if (firstSlang)
                    {
                        firstSlang = false;
                        stringReplacementMap["/*Readme_SlangWarning*/"] <<
                            "=====================================================================================================\n"
                            "Slang Warning\n"
                            "=====================================================================================================\n"
                            "\n"
                            "Some shader files were processed by slang and may need some manual fixup. Slang sometimes\n"
                            "renames resources by appending a 0, such as \"Data\" becoming \"Data_0\". Slang also adds register\n"
                            "declarations onto the resources, and puts globally scoped uniform values into a struct.  These things\n"
                            "will make shader compilation errors because the shader does not match the C++ definition of params.\n"
                            "\n"
                            "To fix it, you can remove the register declarations, put the resource names back to what they should be\n"
                            "and you can take the uniform values out of the struct.  To satisfy the slang code that references those\n"
                            "things that you just un-renamed, you can use #defines to make them still accessible in the slang code.\n"
                            "\n"
                            "For instance if slang made it look like this:\n"
                            "\n"
                            "RWBuffer<float > Data_0 : register(u0);\n"
                            "\n"
                            "Change that to this:\n"
                            "\n"
                            "RWBuffer<float> Data;\n"
                            "#define Data_0 Data\n"
                            "\n"
                            "For the constants in structs, you'll need to replace the . of the struct with an underscore or similar.\n"
                            "\n"
                            "Shaders Files Affected:\n"
                            ;
                    }

                    stringReplacementMap["/*Readme_SlangWarning*/"] << "  * " << std::filesystem::path(shader.destFileName).replace_extension(".usf").string() << "\n";
                }
            }
            if (firstSlang)
                stringReplacementMap["/*Readme_SlangWarning*/"] << "";
            else
                stringReplacementMap["/*Readme_SlangWarning*/"] << "\n";

            if (firstVS)
                stringReplacementMap["/*Readme_VSSemantics*/"] << "";
            else
                stringReplacementMap["/*Readme_VSSemantics*/"] << "\n";
        }

        // Explain how to load assets /*Readme_Assets*/
        {
            bool firstAsset = true;

            stringReplacementMap["/*Readme_Assets*/"] << "";

            for (const RenderGraphNode& nodeBase : renderGraph.nodes)
            {
                if (nodeBase._index != RenderGraphNode::c_index_resourceTexture)
                    continue;

                const RenderGraphNode_Resource_Texture& node = nodeBase.resourceTexture;

                if (node.loadFileName.empty())
                    continue;

                if (firstAsset)
                {
                    firstAsset = false;

                    stringReplacementMap["/*Readme_Assets*/"] <<
                        "=====================================================================================================\n"
                        "Asset Setup\n"
                        "=====================================================================================================\n"
                        "\n"
                        "This technique requires additional setup for required assets.\n"
                        "\n"
                        "These assets must be imported into the engine:\n"
                        "\n"
                        ;
                }

                stringReplacementMap["/*Readme_Assets*/"] <<
                    (std::filesystem::path("assets/") / std::filesystem::path(node.loadFileName)).string() <<
                    " (sRGB: " << (node.loadFileNameAsSRGB ? "true" : "false") << ")\n"
                    ;
            }

            if (!firstAsset)
            {
                stringReplacementMap["/*Readme_Assets*/"] <<
                    "\n"
                    << renderGraph.name << "::Initialize() needs to be called from the game thread to load the textures for use on the rendering thread.\n"
                    "\n"
                    "A good place to do that could be in SceneRendering.cpp. Near the top, add an include to your technique:\n"
                    "\n"
                    "#include \"" << renderGraph.name << "Technique.h\"\n"
                    "\n"
                    "Then, add this into the FSceneRenderer constructor, right below where LoadBlueNoiseTexture() is called, near line 2568.\n"
                    "\n"
                    "        {\n"
                    "            " << renderGraph.name << "::FInitParams initParams;\n"
                    "            // TODO: set path to imported assets.\n"
                    "            // example: initParams.TexturePath__loadedTexture_0 = FSoftObjectPath(\"/Engine/EngineMaterials/cabinsmall.cabinsmall\");\n"
                    "            " << renderGraph.name << "::Initialize(initParams);\n"
                    "        }\n"
                    "\n"
                    ;
            }
        }

        // Make the enums
        {
            // C++ enum definitions
            for (int enumIndex = 0; enumIndex < (int)renderGraph.enums.size(); ++enumIndex)
            {
                const Enum& e = renderGraph.enums[enumIndex];

                if (!e.comment.empty())
                    stringReplacementMap["/*$(TechniqueHNamespace)*/"] << "\n    // " << e.comment;

                stringReplacementMap["/*$(TechniqueHNamespace)*/"] <<
                    "\n    enum class " << e.name << ": int"
                    "\n    {"
                    ;

                for (const auto& item : e.items)
                {
                    stringReplacementMap["/*$(TechniqueHNamespace)*/"] << "\n        " << item.label << ",";

                    if (!item.comment.empty())
                        stringReplacementMap["/*$(TechniqueHNamespace)*/"] << " // " << item.comment;
                }

                stringReplacementMap["/*$(TechniqueHNamespace)*/"] <<
                    "\n    };"
                    ;
            }

            // C++ enum to string
            for (int enumIndex = 0; enumIndex < (int)renderGraph.enums.size(); ++enumIndex)
            {
                const Enum& e = renderGraph.enums[enumIndex];

                stringReplacementMap["/*$(TechniqueHNamespace)*/"] <<
                    "\n"
                    "\n    inline const char* EnumToString(" << e.name << " value, bool displayString = false)"
                    "\n    {"
                    "\n        switch(value)"
                    "\n        {"
                    ;

                for (const auto& item : e.items)
                {
                    stringReplacementMap["/*$(TechniqueHNamespace)*/"] <<
                        "\n            case " << e.name << "::" << item.label << ": return displayString ? \"" << item.displayLabel <<  "\" : \"" << item.label << "\";"
                        ;
                }

                stringReplacementMap["/*$(TechniqueHNamespace)*/"] <<
                    "\n            default: return nullptr;"
                    "\n        }"
                    "\n    }"
                    "\n"
                    ;
            }
        }

        // variables
        {
            for (int targetVisibility = 0; targetVisibility < int(VariableVisibility::Count); ++targetVisibility)
            {
                bool firstVariable = true;
                for (const Variable& variable : renderGraph.variables)
                {
                    VariableVisibility visibility = variable.visibility;
                    if (visibility != (VariableVisibility)targetVisibility)
                        continue;

                    if (visibility == VariableVisibility::User)
                    {
                        stringReplacementMap["/*$(TechniqueCPP_CVars)*/"] <<
                            "static bool CVarChanged_" << renderGraph.name << "_" << variable.name << " = false;\n"
                            "TAutoConsoleVariable<" << DataFieldTypeToCPPType(variable.type) << "> CVar_" << renderGraph.name << "_" << variable.name << "(\n"
                            "    TEXT(\"r." << renderGraph.name << "." << variable.name << "\"), " << VariableDefaultValueCPP(variable, renderGraph, true) << ",\n"
                            "    TEXT(\"" << variable.comment << "\"),\n"
                            "    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InVariable)\n"
                            "        {\n"
                            "            CVarChanged_" << renderGraph.name << "_" << variable.name << " = true;\n"
                            "        }),\n"
                            "    ECVF_RenderThreadSafe);\n"
                            "\n";

                        if (stringReplacementMap["/*$(VariableMaintenance)*/"].str().empty())
                        {
                            stringReplacementMap["/*$(VariableMaintenance)*/"] <<
                                "    //We can't modify cvars on render thread, so we keep the real values in view state, which can be modified by code when needed.\n"
                                ;
                        }

                        stringReplacementMap["/*$(VariableMaintenance)*/"] <<
                            "    if (CVarChanged_" << renderGraph.name << "_" << variable.name << ")\n"
                            "    {\n"
                            "        View.ViewState->TechniqueState_" << renderGraph.name << ".Var_" << variable.name << " = CVar_" << renderGraph.name << "_" << variable.name << ".GetValueOnRenderThread();\n"
                            "        CVarChanged_" << renderGraph.name << "_" << variable.name << " = false;\n"
                            "    }\n"
                            "\n"
                            ;
                    }

                    // Comment header of group
                    if (firstVariable)
                    {
                        if (!stringReplacementMap["/*$(FTechniqueState)*/"].str().empty())
                            stringReplacementMap["/*$(FTechniqueState)*/"] << "\n";

                        if (visibility == VariableVisibility::Internal)
                            stringReplacementMap["/*$(FTechniqueState)*/"] << "    // Internal Variables - do not modify these\n";
                        else if (visibility == VariableVisibility::Host)
                            stringReplacementMap["/*$(FTechniqueState)*/"] << "    // Host Variables - feel free to read and write these\n";
                        else
                            stringReplacementMap["/*$(FTechniqueState)*/"] << "    // User Variables - feel free to read and write these\n";
                        firstVariable = false;
                    }

                    // Declare variable
                    stringReplacementMap["/*$(FTechniqueState)*/"] << "    " << DataFieldTypeToCPPType(variable.type) << " Var_" << variable.name;

                    // Default value
                    if (!variable.dflt.empty())
                        stringReplacementMap["/*$(FTechniqueState)*/"] << " = " << VariableDefaultValueCPP(variable, renderGraph, true);

                    // Comment and close
                    stringReplacementMap["/*$(FTechniqueState)*/"] << ";";
                    if (!variable.comment.empty())
                        stringReplacementMap["/*$(FTechniqueState)*/"] << "  // " << variable.comment;
                    stringReplacementMap["/*$(FTechniqueState)*/"] << "\n";
                }
            }
        }

        // Make non transient storage instructions in readme
        if (!stringReplacementMap["/*$(FTechniqueState)*/"].str().empty())
        {
            stringReplacementMap["/*Readme_NonTransientStorage*/"] <<
                "=====================================================================================================\n"
                "Set Up Non Transient Storage\n"
                "=====================================================================================================\n"
                "\n"
                "This technique requires non transient storage.\n"
                "\n"
                "To set that up, put this struct into Engine\\Source\\Runtime\\Renderer\\Private\\ScenePrivate.h:\n"
                "\n"
                "struct FTechniqueState_" << renderGraph.name << "\n"
                "{\n"
                << stringReplacementMap["/*$(FTechniqueState)*/"].str() << "};\n"
                "\n"
                "and put this member variable into FSceneViewState:\n"
                "\n"
                "FTechniqueState_" << renderGraph.name << " TechniqueState_" << renderGraph.name << ";\n"
                "\n"
                ;
        }
        else
        {
            stringReplacementMap["/*Readme_NonTransientStorage*/"] << "";
        }

        // make the structs
        {
            for (const Struct& s : renderGraph.structs)
            {
                if (!s.exported)
                    continue;

                const char* location = "/*$(TechniqueHNamespace)*/";
                const char* indent = "    ";

                stringReplacementMap[location] <<
                    "\n" << indent << "struct Struct_" << s.name <<
                    "\n" << indent << "{";

                for (const StructField& field : s.fields)
                {
                    stringReplacementMap[location] <<
                        "\n" << indent << "    " << DataFieldTypeToCPPType(field.type) << " " << field.name << " = " << StructFieldDefaultValue(field, renderGraph) << ";";

                    if (!field.comment.empty())
                        stringReplacementMap[location] << "  // " << field.comment;
                }

                stringReplacementMap[location] <<
                    "\n" << indent << "};"
                    "\n"
                    ;
            }
        }

        // Make the setVars
        {
            bool firstSetVar[2] = { true, true };

            for (const SetVariable& setVar : renderGraph.setVars)
            {
                if (setVar.destination.variableIndex == -1)
                    continue;

                const char* destinationString = setVar.setBefore ? "/*$(SetVarBefore)*/" : "/*$(SetVarAfter)*/";

                // Show a comment for setting variables
                if (firstSetVar[!!setVar.setBefore])
                {
                    firstSetVar[!!setVar.setBefore] = false;
                    stringReplacementMap[destinationString] <<
                        "\n        // Set variables"
                        ;
                }

                // Handle the condition for this happening
                if (setVar.condition.comparison != ConditionComparison::Count)
                {
                    stringReplacementMap[destinationString] <<
                        "\n        if(" << ConditionToString(setVar.condition, renderGraph, &VariableToString) << ")"
                        "\n            "
                        ;
                }
                else
                {
                    stringReplacementMap[destinationString] <<
                        "\n        "
                        ;
                }

                // Write out the destination assignment
                {
                    stringReplacementMap[destinationString] << VariableToString(renderGraph.variables[setVar.destination.variableIndex], renderGraph);
                    if (setVar.destinationIndex != -1)
                        stringReplacementMap[destinationString] << "[" << setVar.destinationIndex << "]";
                    stringReplacementMap[destinationString] << " = ";
                }

                // Handle operations which require a prefix to the operands
                if (setVar.op == SetVariableOperator::BitwiseNot)
                {
                    const Variable& destVar = renderGraph.variables[setVar.destination.variableIndex];
                    if (destVar.type == DataFieldType::Bool)
                        stringReplacementMap[destinationString] << "!";
                    else
                        stringReplacementMap[destinationString] << "~";
                }
                else if (setVar.op == SetVariableOperator::PowerOf2GE)
                {
                    stringReplacementMap[destinationString] << "Pow2GE(";
                }
                else if (setVar.op == SetVariableOperator::Minimum)
                {
                    stringReplacementMap[destinationString] << "min(";
                }
                else if (setVar.op == SetVariableOperator::Maximum)
                {
                    stringReplacementMap[destinationString] << "max(";
                }

                // Write out the first operand
                if (setVar.AVar.variableIndex != -1)
                {
                    stringReplacementMap[destinationString] << VariableToString(renderGraph.variables[setVar.AVar.variableIndex], renderGraph);
                    if (setVar.AVarIndex != -1)
                        stringReplacementMap[destinationString] << "[" << setVar.AVarIndex << "]";
                }
                else if (setVar.ANode.nodeIndex != -1)
                {
                    GigiAssert(false, "TODO: implement this");
                    //stringReplacementMap[destinationString] << BackendDX12::ResourceToString(renderGraph.nodes[setVar.ANode.nodeIndex]) << "_size";
                    //if (setVar.AVarIndex != -1)
                        //stringReplacementMap[destinationString] << "[" << setVar.AVarIndex << "]";
                }
                else
                {
                    stringReplacementMap[destinationString] << setVar.ALiteral;
                }

                if (setVar.op == SetVariableOperator::PowerOf2GE)
                {
                    stringReplacementMap[destinationString] << ")";
                }

                // Handle the operator and second operand, if there should be one
                if (setVar.op != SetVariableOperator::Noop && setVar.op != SetVariableOperator::BitwiseNot && setVar.op != SetVariableOperator::PowerOf2GE)
                {
                    bool destVarIsBool = renderGraph.variables[setVar.destination.variableIndex].type == DataFieldType::Bool;

                    // Handle the operator
                    switch (setVar.op)
                    {
                        case SetVariableOperator::Add: stringReplacementMap[destinationString] << " + "; break;
                        case SetVariableOperator::Subtract: stringReplacementMap[destinationString] << " - "; break;
                        case SetVariableOperator::Multiply: stringReplacementMap[destinationString] << " * "; break;
                        case SetVariableOperator::Divide: stringReplacementMap[destinationString] << " / "; break;
                        case SetVariableOperator::Modulo: stringReplacementMap[destinationString] << " % "; break;

                        case SetVariableOperator::Minimum:
                        case SetVariableOperator::Maximum: stringReplacementMap[destinationString] << ", "; break;

                        case SetVariableOperator::BitwiseOr: stringReplacementMap[destinationString] << (destVarIsBool ? " || " : " | "); break;
                        case SetVariableOperator::BitwiseAnd: stringReplacementMap[destinationString] << (destVarIsBool ? " && " : " & "); break;
                        case SetVariableOperator::BitwiseXor: stringReplacementMap[destinationString] << " ^ "; break;
                    }

                    // Write out the second operand
                    if (setVar.BVar.variableIndex != -1)
                    {
                        stringReplacementMap[destinationString] << VariableToString(renderGraph.variables[setVar.BVar.variableIndex], renderGraph);
                        if (setVar.BVarIndex != -1)
                            stringReplacementMap[destinationString] << "[" << setVar.BVarIndex << "]";
                    }
                    else if (setVar.BNode.nodeIndex != -1)
                    {
                        GigiAssert(false, "TODO: implement this");
                        //stringReplacementMap[destinationString] << BackendDX12::ResourceToString(renderGraph.nodes[setVar.BNode.nodeIndex]) << "_size";
                        //if (setVar.BVarIndex != -1)
                            //stringReplacementMap[destinationString] << "[" << setVar.BVarIndex << "]";
                    }
                    else
                    {
                        stringReplacementMap[destinationString] << setVar.BLiteral;
                    }

                    if (setVar.op == SetVariableOperator::Minimum || setVar.op == SetVariableOperator::Maximum)
                    {
                        stringReplacementMap[destinationString] << ")";
                    }
                }

                // All done
                stringReplacementMap[destinationString] << ";";
            }
        }

        // FInitParams
        {
            if (!stringReplacementMap["/*$(FInitParams)*/"].str().empty())
            {
                std::string oldstr = stringReplacementMap["/*$(FInitParams)*/"].str();
                stringReplacementMap["/*$(FInitParams)*/"] = std::ostringstream();
                stringReplacementMap["/*$(FInitParams)*/"] <<
                    "    struct FInitParams\n"
                    "    {\n"
                    << oldstr <<
                    "    };\n"
                    "\n"
                    "    // Must be called from game thread, not render thread\n"
                    "    void Initialize(FInitParams& params);\n"
                    "\n"
                    ;
            }

            if (!stringReplacementMap["/*$(InitializeFnCPP)*/"].str().empty())
            {
                std::string oldstr = stringReplacementMap["/*$(InitializeFnCPP)*/"].str();
                stringReplacementMap["/*$(InitializeFnCPP)*/"] = std::ostringstream();
                stringReplacementMap["/*$(InitializeFnCPP)*/"] <<
                    "\n"
                    "void Initialize(FInitParams& params)\n"
                    "{\n"
                    << oldstr <<
                    "}\n"
                    "\n"
                    ;
            }
        }

        // Ray tracing hit groups and miss shaders
        {
            // Miss shaders
            for (const Shader& shader : renderGraph.shaders)
            {
                if (shader.type != ShaderType::RTMiss)
                    continue;

                std::string fileName = std::string("/Engine/Private/" + renderGraph.name + "/") + std::filesystem::path(shader.destFileName).replace_extension(".usf").string();

                stringReplacementMap["/*$(TechniqueCPP_Shaders)*/"] <<
                    "// Miss Shader \"" << shader.name << "\"\n"
                    "class F" << shader.name << "MS : public FGlobalShader\n"
                    "{\n"
                    "    DECLARE_GLOBAL_SHADER(F" << shader.name << "MS);\n"
                    "\n"
                    "    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)\n"
                    "    {\n"
                    "        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);\n"
                    "    }\n"
                    "\n"
                    "    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)\n"
                    "    {\n"
                    "        return ERayTracingPayloadType::Default;\n"
                    "    }\n"
                    "};\n"
                    "\n"
                    "IMPLEMENT_GLOBAL_SHADER(F" << shader.name << "MS, \"" << StringEscape(fileName) << "\", \"" << shader.entryPoint << "\", SF_RayMiss);\n"
                    "\n"
                    ;
            }

            // hit groups
            for (const RTHitGroup& hitGroup : renderGraph.hitGroups)
            {
                std::ostringstream entryPointString;
                std::unordered_set<std::string> shaderFileNames;

                if (hitGroup.closestHit.shader)
                {
                    const Shader* shader = hitGroup.closestHit.shader;
                    if (!entryPointString.str().empty())
                        entryPointString << " ";
                    entryPointString << "closesthit=" << shader->entryPoint;
                    shaderFileNames.insert(shader->destFileName);
                }

                if (hitGroup.anyHit.shader)
                {
                    const Shader* shader = hitGroup.anyHit.shader;
                    if (!entryPointString.str().empty())
                        entryPointString << " ";
                    entryPointString << "anyhit=" << shader->entryPoint;
                    shaderFileNames.insert(shader->destFileName);
                }

                if (hitGroup.intersection.shader)
                {
                    const Shader* shader = hitGroup.intersection.shader;
                    if (!entryPointString.str().empty())
                        entryPointString << " ";
                    entryPointString << "intersection=" << shader->entryPoint;
                    shaderFileNames.insert(shader->destFileName);
                }

                std::ostringstream fileName;
                if (shaderFileNames.size() > 1)
                    fileName << "/Engine/Private/" << renderGraph.name << "/_HG_" << hitGroup.name << ".usf";
                else
                {
                    for (const std::string& f : shaderFileNames)
                        fileName << "/Engine/Private/" << renderGraph.name << "/" << std::filesystem::path(f).replace_extension(".usf").string();
                }

                stringReplacementMap["/*$(TechniqueCPP_Shaders)*/"] <<
                    "// Hit Group \"" << hitGroup.name << "\"\n"
                    "class F" << hitGroup.name << "HG : public FGlobalShader\n"
                    "{\n"
                    "    DECLARE_GLOBAL_SHADER(F" << hitGroup.name << "HG);\n"
                    "\n"
                    "    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)\n"
                    "    {\n"
                    "        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);\n"
                    "    }\n"
                    "\n"
                    "    static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)\n"
                    "    {\n"
                    "        return ERayTracingPayloadType::Default;\n"
                    "    }\n"
                    "};\n"
                    "\n"
                    "IMPLEMENT_GLOBAL_SHADER(F" << hitGroup.name << "HG, \"" << StringEscape(fileName.str()) << "\", \"" << entryPointString.str() << "\", SF_RayHitGroup);\n"
                    "\n"
                    ;
            }
        }
    }
};

static void CopyShaderFile(Shader& shader, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const char* outFolder, const RenderGraph& renderGraph)
{
    if (shader.language != ShaderLanguage::HLSL && shader.language != ShaderLanguage::Slang)
    {
        GigiAssert(false, "Unsupported shader source language encountered for shader \"%s\": %s", shader.name.c_str(), EnumToString(shader.language));
        return;
    }

    bool isRayShader = (shader.type == ShaderType::RTRayGen || shader.type == ShaderType::RTClosestHit || shader.type == ShaderType::RTAnyHit || shader.type == ShaderType::RTIntersection || shader.type == ShaderType::RTMiss);

    std::string outFileName = (std::filesystem::path(outFolder) / "shaders" / shader.destFileName).replace_extension(".usf").string();

    ProcessShaderOptions_HLSL options(shader);

    options.m_writeOriginalLineNumbers = false;

    options.m_handleShaderToken =
        [](ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const std::string& token, size_t lineNumber, const RenderGraph& renderGraph, const std::string& declareLineNumber)
        {
            std::string param;
            if (GetTokenParameter(token.c_str(), "_raygeneration", param))
            {
                stream = std::ostringstream();
                stream <<
                    "RAY_TRACING_ENTRY_RAYGEN(" << param << ")";
            }
            else if (GetTokenParameter(token.c_str(), "_miss", param))
            {
                stream = std::ostringstream();
                stream <<
                    "RAY_TRACING_ENTRY_MISS(" << param << ", Payload, payload)";
            }
            else if (GetTokenParameter(token.c_str(), "_anyhit", param))
            {
                std::string entryPoint = param;
                std::string attribStruct = "FRayTracingIntersectionAttributes";

                // Get the optional attribute structure argument if present
                size_t colonLoc = param.find_first_of(':');
                if (colonLoc != std::string::npos && colonLoc + 1 < param.length() && colonLoc > 0)
                {
                    entryPoint = param.substr(0, colonLoc);
                    attribStruct = param.substr(colonLoc + 1);
                }

                stream = std::ostringstream();
                stream <<
                    "RAY_TRACING_ENTRY_ANY_HIT(" << entryPoint << ", Payload, payload, " << attribStruct << ", attr)"
                    ;
            }
            else if (GetTokenParameter(token.c_str(), "_intersection", param))
            {
                stream = std::ostringstream();
                stream <<
                    "RAY_TRACING_ENTRY_INTERSECTION( " << param << ")"
                    ;
            }
            else if (GetTokenParameter(token.c_str(), "_closesthit", param))
            {
                std::string entryPoint = param;
                std::string attribStruct = "FRayTracingIntersectionAttributes";

                // Get the optional attribute structure argument if present
                size_t colonLoc = param.find_first_of(':');
                if (colonLoc != std::string::npos && colonLoc + 1 < param.length() && colonLoc > 0)
                {
                    entryPoint = param.substr(0, colonLoc);
                    attribStruct = param.substr(colonLoc + 1);
                }

                stream = std::ostringstream();
                stream <<
                    "RAY_TRACING_ENTRY_CLOSEST_HIT(" << entryPoint << ", Payload, payload, " << attribStruct << ", intersection)"
                    ;
            }
            else if (GetTokenParameter(token.c_str(), "RTHitGroupIndex", param))
            {
                int foundIndex = -1;
                for (int i = 0; i < (int)renderGraph.hitGroups.size(); ++i)
                {
                    const RTHitGroup& hitGroup = renderGraph.hitGroups[i];
                    if (options.m_shader.scope == hitGroup.scope && param == hitGroup.originalName)
                    {
                        foundIndex = i;
                        break;
                    }
                    if (foundIndex != -1)
                        break;
                }

                GigiAssert(foundIndex != -1, "Could not find RTHitGroupIndex for \"%s\" in shader \"%s\"", param.c_str(), options.m_shader.name.c_str());
                if (foundIndex != -1)
                {
                    stream = std::ostringstream();
                    stream << foundIndex;
                }
            }
            else
            {
                return false;
            }

            return true;
        }
    ;

    if (isRayShader)
    {
        options.m_topOfShader =
            "#include \"/Engine/Public/Platform.ush\"\n"
            "#include \"/Engine/Private/RayTracing/RayTracingCommon.ush\"\n"
            "\n"
            "#define float16_t half\n"
            "\n"
            ;
    }
    else
    {
        options.m_topOfShader =
            "#include \"/Engine/Public/Platform.ush\"\n"
            "\n"
            "#define float16_t half\n"
            "\n"
            ;
    }

    options.m_writeConstantBufferDefinition = ProcessShaderOptions_HLSL::WriteConstantBufferDefinition_NotInStruct;
    options.m_writeVariableReference = ProcessShaderOptions_HLSL::WriteVariableReference_NotInStruct;
    options.m_writeVariableAlias = ProcessShaderOptions_HLSL::WriteVariableAlias_NotInStruct;
    options.m_writeSamplerDefinition = ProcessShaderOptions_HLSL::WriteSamplerDefinition_NoRegister;
    options.m_writeResourceDefinition = ProcessShaderOptions_HLSL::WriteResourceDefinition_NoRegister;

    ProcessShader_HLSL(shader, shader.entryPoint.c_str(), ShaderLanguage::HLSL, stringReplacementMap, renderGraph, options, outFileName.c_str());
}

void RunBackend_UE_5_3(GigiBuildFlavor buildFlavor, RenderGraph& renderGraph, GGUserFileLatest& ggUserFile)
{
    // Error out if MSAA is used in this graph, because it isn't yet supported
    for (const RenderGraphNode& node : renderGraph.nodes)
    {
        if (node._index != RenderGraphNode::c_index_resourceTexture)
            continue;

        if (node.resourceTexture.dimension == TextureDimensionType::Texture2DMS)
        {
            GigiAssert(false, "Multisampled textures not supported in code generator for ", EnumToString(buildFlavor));
            return;
        }
    }

    const char* outFolder = renderGraph.outputDirectory.c_str();

    if (renderGraph.generateGraphVizFlag) {
        // make the graphviz
        std::string fullOutFolder = std::string(outFolder) + "/GraphViz/";
        MakeRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
        MakeFlattenedRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
        MakeSummaryRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
    }

    // Get a temporary directory based on the process ID so multiple exes can run at once
    std::string tempDirectory = std::filesystem::temp_directory_path().string();
    {
        tempDirectory += "Gigi\\";
        char pid[256];
        sprintf_s(pid, "%i", _getpid());
        tempDirectory += pid;
        tempDirectory += "\\";
    }

    // Any user variable that is an unsupported cvar type needs to be made into a host variable, and we can warn about that
    for (Variable& variable : renderGraph.variables)
    {
        if (variable.visibility != VariableVisibility::User)
            continue;

        if (!DataFieldTypeIsPOD(variable.type))
        {
            variable.visibility = VariableVisibility::Host;
            ShowWarningMessage("Variable \"%s\" had to be changed from user scope to host scope because it is not a cvar supported data type (%s).\n", variable.name.c_str(), EnumToString(variable.type));
        }
    }

    // gather the templates for the build flavor chosen.
    std::unordered_map<std::string, std::string> files;
    ProcessTemplateFolder(renderGraph, files, outFolder, "./GigiCompilerLib/Backends/UE_5_3/templates/");

    // Make the files
    std::unordered_map<std::string, std::ostringstream> stringReplacementMap = MakeStringReplacement<BackendUE_5_3>(renderGraph, ggUserFile);
    MakeFiles(files, renderGraph, stringReplacementMap);

    // Copy the shader files
    for (Shader& shader : renderGraph.shaders)
    {
        if (shader.type == ShaderType::RTAnyHit || shader.type == ShaderType::RTClosestHit || shader.type == ShaderType::RTIntersection)
            CopyShaderFile(shader, stringReplacementMap, tempDirectory.c_str(), renderGraph);
        else if (shader.copyFile)
            CopyShaderFile(shader, stringReplacementMap, outFolder, renderGraph);
    }

    // Make the composite hit group shader files
    {
        for (const RTHitGroup& hitGroup : renderGraph.hitGroups)
        {
            std::unordered_set<std::string> shaderFileNames;

            std::vector<char> shaderFile;

            std::ostringstream outFile;

            // AnyHit
            if (hitGroup.anyHit.shader)
            {
                shaderFileNames.insert(hitGroup.anyHit.shader->destFileName.c_str());

                char fullFileName[4096];
                sprintf_s(fullFileName, "%s/shaders/%s", tempDirectory.c_str(), hitGroup.anyHit.shader->destFileName.c_str());
                if (!LoadTextFile(std::filesystem::path(fullFileName).replace_extension(".usf").string().c_str(), shaderFile))
                    GigiAssert(false, "Could not read file %s", fullFileName);

                outFile <<
                    "// ====================================\n"
                    "// Any Hit: " << hitGroup.anyHit.shader->entryPoint << "() in " << hitGroup.anyHit.shader->destFileName << "\n"
                    "// ====================================\n"
                    "\n"
                    << shaderFile.data() << "\n"
                    "\n"
                    ;
            }

            // ClosestHit
            if (hitGroup.closestHit.shader)
            {
                shaderFileNames.insert(hitGroup.closestHit.shader->destFileName.c_str());

                char fullFileName[4096];
                sprintf_s(fullFileName, "%s/shaders/%s", tempDirectory.c_str(), hitGroup.closestHit.shader->destFileName.c_str());
                if (!LoadTextFile(std::filesystem::path(fullFileName).replace_extension(".usf").string().c_str(), shaderFile))
                    GigiAssert(false, "Could not read file %s", fullFileName);

                outFile <<
                    "// ====================================\n"
                    "// Closest Hit: " << hitGroup.closestHit.shader->entryPoint << "() in " << hitGroup.closestHit.shader->destFileName << "\n"
                    "// ====================================\n"
                    "\n"
                    << shaderFile.data() << "\n"
                    "\n"
                    ;
            }

            // Intersection
            if (hitGroup.intersection.shader)
            {
                shaderFileNames.insert(hitGroup.intersection.shader->destFileName.c_str());

                char fullFileName[4096];
                sprintf_s(fullFileName, "%s/shaders/%s", tempDirectory.c_str(), hitGroup.intersection.shader->destFileName.c_str());
                if (!LoadTextFile(std::filesystem::path(fullFileName).replace_extension(".usf").string().c_str(), shaderFile))
                    GigiAssert(false, "Could not read file %s", fullFileName);

                outFile <<
                    "// ====================================\n"
                    "// Intersection: " << hitGroup.intersection.shader->entryPoint << "() in " << hitGroup.intersection.shader->destFileName << "\n"
                    "// ====================================\n"
                    "\n"
                    << shaderFile.data() << "\n"
                    "\n"
                    ;
            }

            // Write it to disk if there was more than one file. Else we'll just use that one file.
            if (shaderFileNames.size() > 1)
            {
                std::string outFileContents = outFile.str();
                std::ostringstream fileName;
                fileName << "shaders/_HG_" << hitGroup.name << ".usf";
                std::string fullFileName = (std::filesystem::path(outFolder) / std::filesystem::path(fileName.str())).string();
                WriteFileIfDifferent(fullFileName, outFileContents);
            }
        }
    }

    // copy any file copies that should happen
    for (const FileCopy& fileCopy : renderGraph.fileCopies)
    {
        // load the file into memory
        std::vector<char> data;
        if (fileCopy.binary)
        {
            if (!LoadFile(renderGraph.baseDirectory + fileCopy.fileName, data))
            {
                GigiAssert(false, "Could not read file %s", fileCopy.fileName.c_str());
            }
        }
        else
        {
            std::vector<std::string> embeddedFiles;
            if (!LoadAndPreprocessTextFile(renderGraph.baseDirectory + fileCopy.fileName, data, renderGraph, embeddedFiles))
            {
                GigiAssert(false, "Could not read file %s", fileCopy.fileName.c_str());
            }
        }

        // get the folder to copy to
        const char* destFolder = "";
        switch (fileCopy.type)
        {
            case FileCopyType::Private:
            {
                destFolder = "";
                break;
            }
            case FileCopyType::Shader:
            {
                destFolder = "shaders/";
                break;
            }
            case FileCopyType::Asset:
            {
                destFolder = "assets/";
                break;
            }
            default:
            {
                GigiAssert(false, "Unhandled file copy type");
                break;
            }
        }

        // Make the full file name
        char fullFileName[4096];
        std::string destFileName;
        if (fileCopy.destFileName.empty())
            destFileName = fileCopy.fileName;
        else
            destFileName = fileCopy.destFileName;
        sprintf_s(fullFileName, "%s/%s%s", outFolder, destFolder, destFileName.c_str());

        // Replace the strings and write the file out
        if (fileCopy.binary)
        {
            WriteFileIfDifferent(fullFileName, data);
        }
        else
        {
            data.push_back(0);
            std::string fileContents = (char*)data.data();
            ProcessStringReplacement(fileContents, stringReplacementMap, renderGraph);
            WriteFileIfDifferent(fullFileName, fileContents);
        }
    }
}

void PostLoad_UE_5_3(RenderGraph& renderGraph)
{
    // This backend ignores RTScene resources, and just raytraces against the same world
    renderGraph.configFromBackend.RTSceneTakesSRVSlot = false;
}

// How to move forward with raytracing? need to chew on it abit. maybe need to bang head against it a bit.
// could also get raster merged down and deal with ray after

// TODO: what to do about GetRayTracingPayloadType? for hit groups, ray gen, and ray miss. currently doing default always
// TODO: having the hitgroup in a combined file isn't really working... maybe only do that if there are multiple shader files involved in the hit group?
//  * what is broken is that it isn't the only function in the file, and that other function uses constant buffers, but no constant buffer stuff is generated here.
// TODO: when having the miss use the singular ray gen shader file. "Global uniform buffer cannot be used in a RayMiss shader. Global parameters: cb_SimpleRTRayGenCB_missColor."
//  * can we make ray shaders work in UE, or do we need special rules for working in UE? (gross if so but...)
//  * maybe could we use slang to extract just the function we want? but need to deal with gigi tokens, and shader constants that replace them, etc.
//  ? can we make a local root sig instead?


// NOTE: the combined hitgroup setup (_HG_HitGroup0.usf) doesn't work cause the shader constants aren't in there.
//  * maybe it will work when we use the other shader constants setup that writes into the shaders for you?
//  * BUT, the problem is in the ray ray gen / miss / closest hit functions which shouldn't be in there and won't get that info anyways.
//  * only combine into a hitgroup file if there is more than 1 shader file involved, i think.

// Put this in readme if raytracing being used...
// How to get the scene TLAS?
// Scene renderers have a pointer to the scene, which you can do: Scene->RayTracingScene.GetLayerView(ERayTracingSceneLayer::Base);
// In the example using PostProcessing.cpp, you don't have access to a scene
//  BUT, that is called by DeferredShadingSceneRenderer which does, so you just need to pass it through into the AddPostProcessingPasses
// ? how do we get it into the technique though? i think we need a FRDGBufferSRVRef in the input, instead of a FRHIRayTracingScene*?
//  although maybe the one from the view would be ok?
// ACTUALLY!  the view has "GetRayTracingSceneLayerViewChecked". i think that's what we want, instead of changing the post processing api.
// yes maybe, but there is no ray traced scene made.


// I think FDeferredShadingSceneRenderer::bShouldUpdateRayTracingScene needs to be true, or else the tlas doesn't exist
// That wasn't enough.
// AnyRayTracingPassEnabled() needs to return true i think?
// yeah. i think there is a delegate though too.
