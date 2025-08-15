///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Schemas/Types.h"
#include "GigiCompilerLib/Backends/Shared.h"
#include "GigiCompilerLib/Backends/GraphViz.h"
#include "GigiCompilerLib/Backends/ProcessShader.h"
#include "GigiCompilerLib/GigiBuildFlavor.h"

#include <process.h>

#define SAVE_RWSPLIT_GRAPHS() false

struct BackendWebGPU : public BackendBase
{
    static const char* DataFieldTypeToVertexFormat(DataFieldType dataFieldType)
    {
        switch (dataFieldType)
        {
            case DataFieldType::Int: return "sint32";
            case DataFieldType::Int2: return "sint32x2";
            case DataFieldType::Int3: return "sint32x3";
            case DataFieldType::Int4: return "sint32x4";
            case DataFieldType::Uint: return "uint32";
            case DataFieldType::Uint2: return "uint32x2";
            case DataFieldType::Uint3: return "uint32x3";
            case DataFieldType::Uint4: return "uint32x4";
            case DataFieldType::Float: return "float32";
            case DataFieldType::Float2: return "float32x2";
            case DataFieldType::Float3: return "float32x3";
            case DataFieldType::Float4: return "float32x4";
            case DataFieldType::Bool: return "uint32";
            case DataFieldType::Uint_16: return "uint16";
            case DataFieldType::Float_16: return "float16_t";
            //case DataFieldType::Float4x4: return "float32x4x4"; // Doesn't actually exist
        }

        Assert(false, "Unhandled DataFieldType: %s (%i)", EnumToString(dataFieldType), (int)dataFieldType);
        return __FUNCTION__ " unknown DataFieldType";
    }

    static const char* DrawBlendModeToFactor(DrawBlendMode drawBlendMode)
    {
        switch (drawBlendMode)
        {
            case DrawBlendMode::Zero: return "zero";
            case DrawBlendMode::One: return "one";
            case DrawBlendMode::SrcColor: return "src";
            case DrawBlendMode::InvSrcColor: return "one-minus-src";
            case DrawBlendMode::SrcAlpha: return "src-alpha";
            case DrawBlendMode::InvSrcAlpha: return "one-minus-src-alpha";
            case DrawBlendMode::DestAlpha: return "dst-alpha";
            case DrawBlendMode::InvDestAlpha: return "one-minus-dst-alpha";
            case DrawBlendMode::DestColor: return "dst";
            case DrawBlendMode::InvDestColor: return "one-minus-dst";
        }

        Assert(false, "Unhandled DrawBlendMode: %s (%i)", EnumToString(drawBlendMode), (int)drawBlendMode);
        return __FUNCTION__ " unknown DrawBlendMode";
    }

    static const char* StencilOpToStencilOp(StencilOp stencilOp)
    {
        switch (stencilOp)
        {
            case StencilOp::Keep: return "keep";
            case StencilOp::Zero: return "zero";
            case StencilOp::Replace: return "replace";
            case StencilOp::IncrementSaturate: return "increment-clamp";
            case StencilOp::DecrimentSaturate: return "decrement-clamp";
            case StencilOp::Invert: return "invert";
            case StencilOp::Increment: return "increment-wrap";
            case StencilOp::Decriment: return "decrement-wrap";
        }

        Assert(false, "Unhandled StencilOp: %s (%i)", EnumToString(stencilOp), (int)stencilOp);
        return __FUNCTION__ " unknown StencilOp";
    }

    static const char* DepthTestFunctionToDepthCompare(DepthTestFunction depthTestFunction)
    {
        switch (depthTestFunction)
        {
            case DepthTestFunction::Never: return "never";
            case DepthTestFunction::Less: return "less";
            case DepthTestFunction::Equal: return "equal";
            case DepthTestFunction::LessEqual: return "less-equal";
            case DepthTestFunction::Greater: return "greater";
            case DepthTestFunction::NotEqual: return "not-equal";
            case DepthTestFunction::GreaterEqual: return "greater-equal";
            case DepthTestFunction::Always: return "always";
        }

        Assert(false, "Unhandled DepthTestFunction: %s (%i)", EnumToString(depthTestFunction), (int)depthTestFunction);
        return __FUNCTION__ " unknown DepthTestFunction";
    }

    static const char* GeometryTypeToTopology(GeometryType geometryType)
    {
        switch (geometryType)
        {
            case GeometryType::TriangleList: return "triangle-list";
            case GeometryType::LineList: return "line-list";
            case GeometryType::PointList: return "point-list";
        }

        Assert(false, "Unhandled GeometryType: %s (%i)", EnumToString(geometryType), (int)geometryType);
        return __FUNCTION__ " unknown GeometryType";
    }

    static const char* DrawCullModeToCullMode(DrawCullMode drawCullMode)
    {
        switch (drawCullMode)
        {
            case DrawCullMode::None: return "none";
            case DrawCullMode::Front: return "front";
            case DrawCullMode::Back: return "back";
        }

        Assert(false, "Unhandled DrawCullMode: %s (%i)", EnumToString(drawCullMode), (int)drawCullMode);
        return __FUNCTION__ " unknown DrawCullMode";
    }

    static const char* SamplerFilterToMagFilter(SamplerFilter filter)
    {
        switch (filter)
        {
            case SamplerFilter::MinMagMipPoint: return "nearest";
            case SamplerFilter::MinMagLinear_MipPoint: return "linear";
            case SamplerFilter::MinMagMipLinear: return "linear";
        }
        Assert(false, "Unhandled SamplerFilter: %s (%i)", EnumToString(filter), (int)filter);
        return __FUNCTION__ " unknown SamplerFilter";
    }

    static const char* SamplerFilterToMinFilter(SamplerFilter filter)
    {
        switch (filter)
        {
            case SamplerFilter::MinMagMipPoint: return "nearest";
            case SamplerFilter::MinMagLinear_MipPoint: return "linear";
            case SamplerFilter::MinMagMipLinear: return "linear";
        }
        Assert(false, "Unhandled SamplerFilter: %s (%i)", EnumToString(filter), (int)filter);
        return __FUNCTION__ " unknown SamplerFilter";
    }

    static const char* SamplerFilterToMipMapFilter(SamplerFilter filter)
    {
        switch (filter)
        {
            case SamplerFilter::MinMagMipPoint: return "nearest";
            case SamplerFilter::MinMagLinear_MipPoint: return "nearest";
            case SamplerFilter::MinMagMipLinear: return "linear";
        }
        Assert(false, "Unhandled SamplerFilter: %s (%i)", EnumToString(filter), (int)filter);
        return __FUNCTION__ " unknown SamplerFilter";
    }

    static const char* SamplerAddressModeToAddressMode(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
            case SamplerAddressMode::Clamp: return "clamp-to-edge";
            case SamplerAddressMode::Wrap: return "repeat";
            // NOTE: border mode is not supported
            //case SamplerAddressMode::Border:
        }

        Assert(false, "Unhandled SamplerAddressMode: %s (%i)", EnumToString(addressMode), (int)addressMode);
        return __FUNCTION__ " unknown SamplerAddressMode";
    }

    static const char* TextureDimensionTypeToViewDimension(TextureDimensionType dimension)
    {
        switch (dimension)
        {
            case TextureDimensionType::Texture2D: return "2d";
            case TextureDimensionType::Texture2DArray: return "2d-array";
            case TextureDimensionType::Texture3D: return "3d";
            case TextureDimensionType::TextureCube: return "cube";
        }

        Assert(false, "Unhandled TextureDimensionType: %s (%i)", EnumToString(dimension), (int)dimension);
        return __FUNCTION__ " unknown TextureDimensionType";
    }

    static const char* TextureFormatToGPUTextureFormat(TextureFormat format)
    {
        // WebGPU formats: https://gpuweb.github.io/gpuweb/#enumdef-gputextureformat
        switch (format)
        {
            case TextureFormat::R8_Unorm: return "r8unorm";
            case TextureFormat::RG8_Unorm: return "rg8unorm";
            case TextureFormat::RGBA8_Unorm: return "rgba8unorm";
            case TextureFormat::RGBA8_Unorm_sRGB: return "rgba8unorm-srgb";

            case TextureFormat::R8_Snorm: return "r8snorm";
            case TextureFormat::RG8_Snorm: return "rg8snorm";
            case TextureFormat::RGBA8_Snorm: return "rgba8snorm";

            case TextureFormat::R8_Uint: return "r8uint";
            case TextureFormat::RG8_Uint: return "rg8uint";
            case TextureFormat::RGBA8_Uint: return "rgba8uint";

            case TextureFormat::R8_Sint: return "r8sint";
            case TextureFormat::RG8_Sint: return "rg8sint";
            case TextureFormat::RGBA8_Sint: return "rgba8sint";

            case TextureFormat::R16_Float: return "r16float";
            case TextureFormat::RG16_Float: return "rg16float";
            case TextureFormat::RGBA16_Float: return "rgba16float";

            case TextureFormat::R32_Float: return "r32float";
            case TextureFormat::RG32_Float: return "rg32float";
            case TextureFormat::RGBA32_Float: return "rgba32float";

            case TextureFormat::R32_Uint: return "r32uint";
            case TextureFormat::RGBA32_Uint: return "rgba32uint";

            case TextureFormat::R11G11B10_Float: return "rg11b10ufloat";

            case TextureFormat::D32_Float: return "depth32float";
            case TextureFormat::D16_Unorm: return "depth16unorm";

            case TextureFormat::D32_Float_S8: return "depth32float-stencil8";
            case TextureFormat::D24_Unorm_S8: return "depth24plus-stencil8";

            case TextureFormat::BC1_Unorm: return "bc1-rgb-unorm";

            case TextureFormat::BC4_Unorm: return "bc4-r-unorm";
            case TextureFormat::BC4_Snorm: return "bc4-r-snorm";
            case TextureFormat::BC5_Unorm: return "bc5-rg-unorm";
            case TextureFormat::BC5_Snorm: return "bc5-rg-snorm";

            case TextureFormat::BC7_Unorm: return "bc7-rgba-unorm";
            case TextureFormat::BC7_Unorm_sRGB: return "bc7-rgba-unorm-srgb";

            case TextureFormat::BC6_UF16: return "bc6h-rgb-ufloat";
            case TextureFormat::BC6_SF16: return "bc6h-rgb-float";

            // No webgpu version
            case TextureFormat::RGBA16_Unorm:
            case TextureFormat::RGBA16_Snorm:
            {
                Assert(false, "WebGPU does not support textureFormat: %s (%i)", EnumToString(format), (int)format);
                return __FUNCTION__ " unsupported TextureFormat";
                break;
            }
        }

        Assert(false, "Unhandled TextureFormat: %s (%i)", EnumToString(format), (int)format);
        return __FUNCTION__ " unknown TextureFormat";
    }

    static std::string GetVariableDflt(const RenderGraph& renderGraph, const Variable& var)
    {
        if (var.enumIndex == -1)
            return var.dflt;

        return "this.constructor.Enum_" + renderGraph.enums[var.enumIndex].name + std::string(".") + var.dflt;
    }

    static void WriteCondition(const RenderGraph& renderGraph, std::ostringstream& stream, const char* indent, const Condition& condition)
    {
        if (condition.comparison == ConditionComparison::Count)
            return;

        stream << indent << "if (";

        if (condition.alwaysFalse)
        {
            stream << "false)\n";
            return;
        }

        std::string value1;
        if (condition.variable1Index != -1)
            value1 = std::string("this.variable_") + renderGraph.variables[condition.variable1Index].name;

        std::string value2;
        if (condition.variable2Index != -1)
            value2 = std::string("this.variable_") + renderGraph.variables[condition.variable2Index].name;
        else
        {
            value2 = condition.value2;

            if (condition.variable1Index != -1 && renderGraph.variables[condition.variable1Index].enumIndex != -1)
            {
                std::string enumValueScope = renderGraph.enums[renderGraph.variables[condition.variable1Index].enumIndex].name + "::";
                if (!StringBeginsWith(value2.c_str(), enumValueScope.c_str()))
                    value2 = enumValueScope + value2;
            }
        }

        switch (condition.comparison)
        {
            case ConditionComparison::IsFalse: stream << "!" << value1; break;
            case ConditionComparison::IsTrue: stream << value1; break;
            case ConditionComparison::Equals: stream << value1 << " == " << value2; break;
            case ConditionComparison::NotEquals: stream << value1 << " != " << value2; break;
            case ConditionComparison::LT: stream << value1 << " < " << value2; break;
            case ConditionComparison::LTE: stream << value1 << " <= " << value2; break;
            case ConditionComparison::GT: stream << value1 << " > " << value2; break;
            case ConditionComparison::GTE: stream << value1 << " >= " << value2; break;
            default:
            {
                Assert(false, "Unhandled condition comparison");
                return;
            }
        }

        stream << ")\n";
    }

    static void WriteSetVar(const RenderGraph& renderGraph, std::ostringstream& stream, const SetVariable& setVar)
    {
        const char* indent = "    ";
        bool wroteCondition = false;
        if (setVar.condition.comparison != ConditionComparison::Count)
            WriteCondition(renderGraph, stream, indent, setVar.condition);

        stream << indent << "{\n";

        // Write out that this variable changed
        stream << indent << "    this.variableChanged_" << setVar.destination.name;
        if (setVar.destinationIndex != -1)
            stream << "[" << setVar.destinationIndex << "]";
        stream << " = true;\n";

        // Write out the destination assignment
        {
            stream << indent << "    this.variable_" << setVar.destination.name;
            if (setVar.destinationIndex != -1)
                stream << "[" << setVar.destinationIndex << "]";
            stream << " = ";
        }

        // Handle operations which require a prefix to the operands
        if (setVar.op == SetVariableOperator::BitwiseNot)
        {
            const Variable& destVar = renderGraph.variables[setVar.destination.variableIndex];
            if (destVar.type == DataFieldType::Bool)
                stream << "!";
            else
                stream << "~";
        }
        else if (setVar.op == SetVariableOperator::PowerOf2GE)
        {
            stream << "Pow2GE(";
        }
        else if (setVar.op == SetVariableOperator::Minimum)
        {
            stream << "Math.min(";
        }
        else if (setVar.op == SetVariableOperator::Maximum)
        {
            stream << "Math.max(";
        }

        // Write out the first operand
        if (setVar.AVar.variableIndex != -1)
        {
            stream << "this.variable_" << renderGraph.variables[setVar.AVar.variableIndex].name;
            if (setVar.AVarIndex != -1)
                stream << "[" << setVar.AVarIndex << "]";
        }
        else if (setVar.ANode.nodeIndex != -1)
        {
            const char* resourcePrefix = "";
            switch (renderGraph.nodes[setVar.ANode.nodeIndex]._index)
            {
                case RenderGraphNode::c_index_resourceBuffer: resourcePrefix = "this.buffer_"; break;
                case RenderGraphNode::c_index_resourceTexture: resourcePrefix = "this.texture_"; break;
                default:
                {
                    Assert(false, "Unhandled resource node type!");
                }
            }

            stream << resourcePrefix << GetNodeName(renderGraph.nodes[setVar.ANode.nodeIndex]) << "_size";
            if (setVar.AVarIndex != -1)
                stream << "[" << setVar.AVarIndex << "]";
        }
        else
        {
            stream << setVar.ALiteral;
        }

        // Handle the operator and second operand, if there should be one
        if (setVar.op != SetVariableOperator::Noop && setVar.op != SetVariableOperator::BitwiseNot && setVar.op != SetVariableOperator::PowerOf2GE)
        {
            bool destVarIsBool = renderGraph.variables[setVar.destination.variableIndex].type == DataFieldType::Bool;

            // Handle the operator
            switch (setVar.op)
            {
                case SetVariableOperator::Add: stream << " + "; break;
                case SetVariableOperator::Subtract: stream << " - "; break;
                case SetVariableOperator::Multiply: stream << " * "; break;
                case SetVariableOperator::Divide: stream << " / "; break;
                case SetVariableOperator::Modulo: stream << " % "; break;

                case SetVariableOperator::Minimum:
                case SetVariableOperator::Maximum: stream << ", "; break;

                case SetVariableOperator::BitwiseOr: stream << (destVarIsBool ? " || " : " | "); break;
                case SetVariableOperator::BitwiseAnd: stream << (destVarIsBool ? " && " : " & "); break;
                case SetVariableOperator::BitwiseXor: stream << " ^ "; break;
            }

            // Write out the second operand
            if (setVar.BVar.variableIndex != -1)
            {
                stream << indent << "this.variable_" << renderGraph.variables[setVar.BVar.variableIndex].name;
                if (setVar.BVarIndex != -1)
                    stream << "[" << setVar.BVarIndex << "]";
            }
            else if (setVar.BNode.nodeIndex != -1)
            {
                const char* resourcePrefix = "";
                switch (renderGraph.nodes[setVar.BNode.nodeIndex]._index)
                {
                    case RenderGraphNode::c_index_resourceBuffer: resourcePrefix = "this.buffer_"; break;
                    case RenderGraphNode::c_index_resourceTexture: resourcePrefix = "this.texture_"; break;
                    default:
                    {
                        Assert(false, "Unhandled resource node type!");
                    }
                }

                stream << resourcePrefix << GetNodeName(renderGraph.nodes[setVar.BNode.nodeIndex]) << "_size";
                if (setVar.BVarIndex != -1)
                    stream << "[" << setVar.BVarIndex << "]";
            }
            else
            {
                stream << setVar.BLiteral;
            }

            if (setVar.op == SetVariableOperator::Minimum || setVar.op == SetVariableOperator::Maximum)
            {
                stream << ")";
            }
        }

        stream << ";\n" << indent << "}\n\n";
    }

    #include "nodes/nodes.inl"

    static inline bool g_processingShaders = false;

    static void MakeStringReplacementGlobal(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph, GGUserFileLatest& ggUserFile)
    {
        BackendBase::MakeStringReplacementGlobal(stringReplacementMap, renderGraph, ggUserFile);

        if (g_processingShaders)
            return;

        stringReplacementMap["/*$(Description)*/"] << "                    <b>" << renderGraph.name << "</b><br/>\n";

        if (!renderGraph.comment.empty())
            stringReplacementMap["/*$(Description)*/"] << "                    <pre>" << renderGraph.comment << "</pre>\n";

        // required features and limits
        {
            // Features
            {
                if (renderGraph.settings.webGPU.features.float32Filterable)
                    stringReplacementMap["/*$(RequiredFeatures)*/"] << ", \'float32-filterable\'";

                if (renderGraph.settings.webGPU.features.subgroups)
                    stringReplacementMap["/*$(RequiredFeatures)*/"] << ", \'subgroups\'";

                std::string old = stringReplacementMap["/*$(RequiredFeatures)*/"].str();
                if (!old.empty())
                {
                    stringReplacementMap["/*$(RequiredFeatures)*/"] = std::ostringstream();
                    stringReplacementMap["/*$(RequiredFeatures)*/"] << old.substr(2);
                }
            }

            // Limits
            {
                if (renderGraph.settings.webGPU.limits.maxStorageTexturesPerShaderStage > 0)
                    stringReplacementMap["/*$(RequiredLimits)*/"] << ", maxStorageTexturesPerShaderStage: " << renderGraph.settings.webGPU.limits.maxStorageTexturesPerShaderStage;

                if (renderGraph.settings.webGPU.limits.maxStorageBuffersPerShaderStage > 0)
                    stringReplacementMap["/*$(RequiredLimits)*/"] << ", maxStorageBuffersPerShaderStage: " << renderGraph.settings.webGPU.limits.maxStorageBuffersPerShaderStage;

                if (renderGraph.settings.webGPU.limits.maxComputeWorkgroupStorageSize > 0)
                    stringReplacementMap["/*$(RequiredLimits)*/"] << ", maxComputeWorkgroupStorageSize: " << renderGraph.settings.webGPU.limits.maxComputeWorkgroupStorageSize;

                std::string old = stringReplacementMap["/*$(RequiredLimits)*/"].str();
                if (!old.empty())
                {
                    stringReplacementMap["/*$(RequiredLimits)*/"] = std::ostringstream();
                    stringReplacementMap["/*$(RequiredLimits)*/"] << old.substr(2);
                }
            }
        }

        // Fill out the command encoder for each node, in flattened node list order. Also let the node fill out storage etc that are not order dependent.
        for (size_t stepIndex = 0; stepIndex < renderGraph.flattenedNodeList.size(); ++stepIndex)
        {
            // Let the node do work
            int nodeIndex = renderGraph.flattenedNodeList[stepIndex];
            RenderGraphNode& node = renderGraph.nodes[nodeIndex];
            ExecuteOnNode(node,
                [&](auto& node)
                {
                    stringReplacementMap["/*$(Execute)*/"] << "    encoder.pushDebugGroup(\"" << renderGraph.name << "." << node.name << "\");\n\n";
                    MakeStringReplacementForNode(stringReplacementMap, renderGraph, node);
                    stringReplacementMap["/*$(Execute)*/"] << "    encoder.popDebugGroup(); // \"" << renderGraph.name << "." << node.name << "\"\n\n";
                }
            );
        }

        // Declare Variables
        for (const Variable& variable : renderGraph.variables)
        {
            const char* storageToken = (variable.visibility == VariableVisibility::Internal) ? "/*$(VariablesPrivate)*/" : "/*$(VariablesPublic)*/";

            DataFieldTypeInfoStruct dataFieldInfo = DataFieldTypeInfo(variable.type);

            // Write out the variable declaration
            {
                stringReplacementMap[storageToken] << "variable_" << variable.name << " = ";
                if (dataFieldInfo.componentCount == 1)
                    stringReplacementMap[storageToken] << GetVariableDflt(renderGraph, variable) << ";";
                else
                    stringReplacementMap[storageToken] << "[ " << GetVariableDflt(renderGraph, variable) << " ];";

                if (!variable.comment.empty())
                    stringReplacementMap[storageToken] << "  // " << variable.comment;

                stringReplacementMap[storageToken] << "\n";
            }

            // write out the default variable value
            {
                stringReplacementMap[storageToken] << "variableDefault_" << variable.name << " = ";
                if (dataFieldInfo.componentCount == 1)
                    stringReplacementMap[storageToken] << GetVariableDflt(renderGraph, variable) << ";";
                else
                    stringReplacementMap[storageToken] << "[ " << GetVariableDflt(renderGraph, variable) << " ];";

                if (!variable.comment.empty())
                    stringReplacementMap[storageToken] << "  // " << variable.comment;

                stringReplacementMap[storageToken] << "\n";
            }

            // write out the variable changed values
            {
                if (dataFieldInfo.componentCount == 1)
                {
                    stringReplacementMap[storageToken] << "variableChanged_" << variable.name << " = false;\n";
                }
                else
                {
                    stringReplacementMap[storageToken] << "variableChanged_" << variable.name << " = [ ";

                    for (int componentIndex = 0; componentIndex < dataFieldInfo.componentCount; ++componentIndex)
                    {
                        if (componentIndex > 0)
                            stringReplacementMap[storageToken] << ", false";
                        else
                            stringReplacementMap[storageToken] << "false";
                    }

                    stringReplacementMap[storageToken] << " ];\n";
                }
            }
        }

        // Copy the viewed texture
        {
            char sourceTexturename[1024];
            sprintf_s(sourceTexturename, "%s[\"texture_\" + viewingTextureName]", renderGraph.name.c_str());

            stringReplacementMap["/*$(CopyPrimaryOutput)*/"] <<
                "                if (viewingTextureName.length != 0 && " << renderGraph.name << "[\"texture_\" + viewingTextureName] !== null)\n"
                "                {\n"
                "                    // Update size of the canvas\n"
                "                    document.getElementById(\"mainCanvas\").width = " << sourceTexturename << ".width;\n"
                "                    document.getElementById(\"mainCanvas\").height = " << sourceTexturename << ".height;\n"
                "\n"
                "                    if (!Shared.CopyCompatibleFormats(" << sourceTexturename << ".format, canvasFormat))\n"
                "                    {\n"
                "                        Shared.CopyTextureLinearToSRGB(device, encoder, " << sourceTexturename << ", context.getCurrentTexture());\n"
                "                    }\n"
                "                    else\n"
                "                    {\n"
                "                        encoder.copyTextureToTexture(\n"
                "                            { texture: " << sourceTexturename << " },\n"
                "                            { texture: context.getCurrentTexture() },\n"
                "                            {\n"
                "                                width: Math.min(" << sourceTexturename << ".width, context.getCurrentTexture().width),\n"
                "                                height: Math.min(" << sourceTexturename << ".height, context.getCurrentTexture().height),\n"
                "                                depthOrArrayLayers: Math.min(" << sourceTexturename << ".depthOrArrayLayers, context.getCurrentTexture().depthOrArrayLayers)\n"
                "                            }\n"
                "                        );\n"
                "                    }\n"
                "                }\n"
                "\n"
                ;
        }

        // TODOs regarding host and user variables
        {
            bool firstVar = true;
            for (const Variable& variable : renderGraph.variables)
            {
                if (variable.visibility != VariableVisibility::Host)
                    continue;

                if (firstVar)
                {
                    stringReplacementMap["/*$(TODO_NeedUserInput)*/"] << "\n                Host Variables:";
                    firstVar = false;
                }

                stringReplacementMap["/*$(TODO_NeedUserInput)*/"] <<
                    "\n                    " << renderGraph.name << ".variable_" << variable.name << " = " << GetVariableDflt(renderGraph, variable) << "; // (" << EnumToString(variable.type) << ")";

                if (!variable.comment.empty())
                    stringReplacementMap["/*$(TODO_NeedUserInput)*/"] << " " << variable.comment;
            }

            firstVar = true;
            for (const Variable& variable : renderGraph.variables)
            {
                if (variable.visibility != VariableVisibility::User)
                    continue;

                if (firstVar)
                {
                    stringReplacementMap["/*$(TODO_NeedUserInput)*/"] << "\n                User Variables (These show in the UI):";
                        firstVar = false;
                }

                stringReplacementMap["/*$(TODO_NeedUserInput)*/"] <<
                    "\n                    " << renderGraph.name << ".variable_" << variable.name << " = " << GetVariableDflt(renderGraph, variable) << "; // (" << EnumToString(variable.type) << ")";

                if (!variable.comment.empty())
                    stringReplacementMap["/*$(TODO_NeedUserInput)*/"] << " " << variable.comment;
            }
        }

        // write out the saved variables
        if (ggUserFile.snapshot.savedVariables.size() > 0)
        {
            stringReplacementMap["/*$(SetGGUserFileVariableValues)*/"] << "            // Set variables to what they were saved as in the gguser file\n";
            for (const auto& savedVariable : ggUserFile.snapshot.savedVariables)
            {
                int varIndex = GetVariableIndex(renderGraph, savedVariable.name.c_str());
                if (varIndex == -1)
                    continue;

                const Variable& variable = renderGraph.variables[varIndex];

                if (DataFieldTypeInfo(variable.type).componentCount > 1)
                    stringReplacementMap["/*$(SetGGUserFileVariableValues)*/"] << "            " << renderGraph.name << ".variable_" << savedVariable.name << " = [" << savedVariable.value << "];\n";
                else
                    stringReplacementMap["/*$(SetGGUserFileVariableValues)*/"] << "            " << renderGraph.name << ".variable_" << savedVariable.name << " = " << savedVariable.value << ";\n";
            }
            stringReplacementMap["/*$(SetGGUserFileVariableValues)*/"] << "\n";
        }

        // Set variables
        for (const SetVariable& setVar : renderGraph.setVars)
        {
            const char* location = (setVar.setBefore) ? "/*$(SetVarsPre)*/" : "/*$(SetVarsPost)*/";
            WriteSetVar(renderGraph, stringReplacementMap[location], setVar);
        }

        // Write out struct vertex buffer attributes
        for (const Struct& s : renderGraph.structs)
        {
            if (s.isForShaderConstants)
                continue;

            // Wite out all fields of all structs, even if they don't have semantics. An instance buffer doesn't have semantics.
            /*
            bool hasSemantics = false;
            for (const StructField& field : s.fields)
            {
                if (field.semantic != StructFieldSemantic::Count)
                {
                    hasSemantics = true;
                    break;
                }
            }
            if (!hasSemantics)
                continue;
            */

            stringReplacementMap["/*$(StructVBAttribs)*/"] <<
                "// Vertex buffer attributes for struct " << s.name << "\n"
                "static StructVertexBufferAttributes_" << s.name << " =\n"
                "[\n"
                ;

            size_t byteOffset = 0;
            int shaderLocation = 0;
            for (const StructField& field : s.fields)
            {
                //if (field.semantic != StructFieldSemantic::Count)
                if (!field.isPadding)
                {
                    stringReplacementMap["/*$(StructVBAttribs)*/"] <<
                        "    // " << field.name << "\n"
                        "    {\n"
                        "        format: \"" << DataFieldTypeToVertexFormat(field.type) << "\",\n"
                        "        offset: " << byteOffset << ",\n"
                        "        shaderLocation: " << shaderLocation << ",\n"
                        "    },\n"
                        ;
                    shaderLocation++;
                }
                byteOffset += field.sizeInBytes;
            }


            stringReplacementMap["/*$(StructVBAttribs)*/"] <<
                "];\n"
                "\n"
                ;
        }

        // Write out enums
        for (const Enum& e : renderGraph.enums)
        {
            stringReplacementMap["/*$(Enums)*/"] <<
                "static Enum_" << e.name << " =\n"
                "{\n"
                ;

            for (size_t itemIndex = 0; itemIndex < e.items.size(); ++itemIndex)
            {
                const EnumItem& item = e.items[itemIndex];

                stringReplacementMap["/*$(Enums)*/"] <<
                    "    " << item.label << ": " << itemIndex << ",\n"
                    ;
            }

            stringReplacementMap["/*$(Enums)*/"] <<
                "    _count: " << e.items.size() << ",\n"
                "}\n"
                "\n"
                ;
        }

        // Write out structs
        for (const Struct& s : renderGraph.structs)
        {
            //if (!s.exported)
                //continue;

            stringReplacementMap["/*$(Structs)*/"] <<
                "static StructOffsets_" << s.name << " =\n"
                "{\n"
                ;

            size_t offset = 0;
            for (const StructField& field : s.fields)
            {
                DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(field.type);
                for (int componentIndex = 0; componentIndex < typeInfo.componentCount; ++componentIndex)
                {
                    stringReplacementMap["/*$(Structs)*/"] << "    " << field.name;

                    if (typeInfo.componentCount > 1)
                        stringReplacementMap["/*$(Structs)*/"] << "_" << componentIndex;

                    stringReplacementMap["/*$(Structs)*/"] << ": " << offset << ",\n";

                    offset += typeInfo.componentBytes;
                }
            }

            stringReplacementMap["/*$(Structs)*/"] <<
                "    _size: " << offset << ",\n"
                "}\n"
                "\n"
                ;
        }

        // UI
        {
            std::ostringstream resetVariablesStream;
            std::ostringstream setUIToVariablesStream;
            std::ostringstream streams[(int)VariableVisibility::Count];
            for (const Variable& variable : renderGraph.variables)
            {
                std::string disabledStr = variable.Const ? " disabled=\"true\"" : "";
                std::ostringstream& HTMLUIStream = streams[(int)variable.visibility];

                std::string label = variable.name + "&nbsp;";
                if (variable.type == DataFieldType::Bool && variable.UISettings.UIHint == VariableUIHint::Button)
                    label = "";

                HTMLUIStream <<
                    "                        <tr>\n"
                    "                            <td valign=\"top\">"
                    ;

                // Tooltip
                if (!variable.comment.empty())
                    HTMLUIStream << "<div class=\"tooltip\">" << label << "<span class=\"tooltiptext\">" << variable.comment << "</span></div>";
                else
                    HTMLUIStream << label;

                HTMLUIStream << "</td>\n"
                    "                            <td>\n"
                    ;

                if (variable.type == DataFieldType::Bool)
                {
                    if (variable.UISettings.UIHint == VariableUIHint::Button)
                    {
                        HTMLUIStream <<
                            "                                <button type=\"button\" id=\"inputBox_" << variable.name << "\"" << disabledStr << ">" << variable.name << "</button>\n"
                            ;

                        stringReplacementMap["/*$(InitOneTime)*/"] <<
                            "            document.getElementById('inputBox_" << variable.name << "').addEventListener('click',(event) => {" << renderGraph.name << ".variable_" << variable.name << "=true;});\n"
                            ;

                        stringReplacementMap["/*$(ReconcileVariableUI)*/"] <<
                            "                " << renderGraph.name << ".variable_" << variable.name << "=false;\n"
                            "\n"
                            ;
                    }
                    else
                    {
                        HTMLUIStream <<
                            "                                <input type=\"checkbox\" id=\"inputBox_" << variable.name << "\"" << disabledStr << ">\n"
                            ;

                        resetVariablesStream <<
                            "                    " << renderGraph.name << ".variable_" << variable.name << " = " << renderGraph.name << ".variableDefault_" << variable.name << ";\n"
                            ;

                        setUIToVariablesStream <<
                            "                    document.getElementById('inputBox_" << variable.name << "').checked = Boolean(" << renderGraph.name << ".variable_" << variable.name << ");\n"
                            ;

                        stringReplacementMap["/*$(InitOneTime)*/"] <<
                            "            document.getElementById('inputBox_" << variable.name << "').addEventListener('change',(event) => {" << renderGraph.name << ".variable_" << variable.name << "=event.target.checked;});\n"
                            ;

                        stringReplacementMap["/*$(ReconcileVariableUI)*/"] <<
                            "                if (" << renderGraph.name << ".variableChanged_" << variable.name << ")\n"
                            "                {\n"
                            "                    document.getElementById('inputBox_" << variable.name << "').checked=Boolean(" << renderGraph.name << ".variable_" << variable.name << ");\n"
                            "                    " << renderGraph.name << ".variableChanged_" << variable.name << " = false;\n"
                            "                }\n"
                            "\n"
                            ;
                    }
                }
                else if (variable.type == DataFieldType::Int && variable.enumIndex != -1)
                {
                    HTMLUIStream <<
                        "                                <select id=\"inputBox_" << variable.name << "\"" << disabledStr << ">\n"
                        ;

                    const Enum& e = renderGraph.enums[variable.enumIndex];

                    for (size_t itemIndex = 0; itemIndex < e.items.size(); ++itemIndex)
                    {
                        const EnumItem& item = e.items[itemIndex];
                        HTMLUIStream <<
                            "                                    <option value=\"" << itemIndex << "\">" << item.displayLabel << "</option>\n"
                            ;
                    }

                    HTMLUIStream <<
                        "                                </select>\n"
                        ;

                    resetVariablesStream <<
                        "                    " << renderGraph.name << ".variable_" << variable.name << " = " << renderGraph.name << ".variableDefault_" << variable.name << ";\n"
                        ;

                    setUIToVariablesStream <<
                        "                    document.getElementById('inputBox_" << variable.name << "').value = " << renderGraph.name << ".variable_" << variable.name << ";\n"
                        ;

                    stringReplacementMap["/*$(InitOneTime)*/"] <<
                        "            document.getElementById('inputBox_" << variable.name << "').addEventListener('change',(event) => {event.target.value=event.target.value; " << renderGraph.name << ".variable_" << variable.name << "=event.target.value;});\n"
                        ;

                    stringReplacementMap["/*$(ReconcileVariableUI)*/"] <<
                        "                if (" << renderGraph.name << ".variableChanged_" << variable.name << ")\n"
                        "                {\n"
                        "                    document.getElementById('inputBox_" << variable.name << "').value = " << renderGraph.name << ".variable_" << variable.name << ";\n"
                        "                    " << renderGraph.name << ".variableChanged_" << variable.name << " = false;\n"
                        "                }\n"
                        "\n"
                        ;
                }
                else
                {
                    DataFieldTypeInfoStruct typeInfo = DataFieldTypeInfo(variable.type);
                    const char* parseFunction = "";
                    switch (typeInfo.componentType)
                    {
                        case DataFieldComponentType::_int: parseFunction = "parseInt"; break;
                        case DataFieldComponentType::_uint16_t: parseFunction = "parseUint"; break;
                        case DataFieldComponentType::_uint32_t: parseFunction = "parseUint"; break;
                        case DataFieldComponentType::_float: parseFunction = "parseFloat"; break;
                        default:
                        {
                            ShowErrorMessage("Unhandled variable component type: %i", typeInfo.componentType);
                            continue;
                        }
                    }

                    for (int componentIndex = 0; componentIndex < typeInfo.componentCount; ++componentIndex)
                    {
                        if (variable.type == DataFieldType::Float4x4 && componentIndex > 0 && (componentIndex % 4 == 0))
                            HTMLUIStream << "                                <br>\n";

                        char indexString[256];
                        char indexStringNaked[256];
                        if (typeInfo.componentCount > 1)
                        {
                            sprintf_s(indexString, "[%i]", componentIndex);
                            sprintf_s(indexStringNaked, "%i", componentIndex);
                        }
                        else
                        {
                            indexString[0] = 0;
                            indexStringNaked[0] = 0;
                        }

                        HTMLUIStream <<
                            "                                <input type=\"text\" id=\"inputBox_" << variable.name << indexStringNaked << "\"" << disabledStr << ">\n"
                            ;

                        resetVariablesStream <<
                            "                    " << renderGraph.name << ".variable_" << variable.name << indexString << " = " << renderGraph.name << ".variableDefault_" << variable.name << indexString << ";\n"
                            ;

                        setUIToVariablesStream <<
                            "                    document.getElementById('inputBox_" << variable.name << indexStringNaked << "').value = " << renderGraph.name << ".variable_" << variable.name << indexString << ";\n"
                            ;

                        stringReplacementMap["/*$(InitOneTime)*/"] <<
                            "            document.getElementById('inputBox_" << variable.name << indexStringNaked << "').addEventListener('change',(event) => {event.target.value=" << parseFunction << "(event.target.value); " << renderGraph.name << ".variable_" << variable.name << indexString << "=event.target.value;});\n"
                            ;

                        stringReplacementMap["/*$(ReconcileVariableUI)*/"] <<
                            "                if (" << renderGraph.name << ".variableChanged_" << variable.name << indexString << ")\n"
                            "                {\n"
                            "                    document.getElementById('inputBox_" << variable.name << indexStringNaked << "').value = " << renderGraph.name << ".variable_" << variable.name << indexString << ";\n"
                            "                    " << renderGraph.name << ".variableChanged_" << variable.name << indexString << " = false;\n"
                            "                }\n"
                            "\n"
                            ;
                    }
                }

                HTMLUIStream <<
                    "                            </td>\n"
                    "                        </tr>\n"
                    ;
            }

            const VariableVisibility visibilitiesShown[] = { VariableVisibility::User, VariableVisibility::Host, VariableVisibility::Internal };
            for (VariableVisibility visibility : visibilitiesShown)
            {
                std::string str = streams[(int)visibility].str();
                if (str.empty())
                    continue;

                std::string style;
                if (visibility == VariableVisibility::User)
                    style = " style=\"display:block\"";
                else
                    style = " style=\"display:none\"";

                stringReplacementMap["/*$(HTMLUI)*/"] <<
                    "                    <button type=\"button\" class=\"collapsible\">" << EnumToString(visibility) << " Variables</button>\n"
                    "                    <table cellspacing=\"0\" cellpadding=\"0\" class=\"collapsiblecontent\"" << style << ">\n"
                    << str <<
                    "                    </table>\n"
                    "                    <br><br>\n"
                    ;
            }

            if (!resetVariablesStream.str().empty() || !setUIToVariablesStream.str().empty())
            {
                stringReplacementMap["/*$(InitOneTime)*/"] <<
                    "\n"
                    "            function ResetVariables()\n"
                    "            {\n"
                    << resetVariablesStream.str() <<
                    "            }\n"
                    "\n"
                    "            function SetUIToVariables()\n"
                    "            {\n"
                    << setUIToVariablesStream.str() <<
                    "            }\n"
                    "\n"
                    "            // Set variables to their defaults\n"
                    "            ResetVariables();\n"
                    "\n"
                    "            document.getElementById('ResetVariables').addEventListener('click',(event) => { ResetVariables(); SetUIToVariables(); });\n"
                    ;

                stringReplacementMap["/*$(InitOneTime2)*/"] <<
                    "            // Make sure the UI shows the correct variable values\n"
                    "            SetUIToVariables();\n"
                    "\n"
                    ;
            }
        }

        // Imported texture input boxes
        {
            std::ostringstream importedTextures;
            std::ostringstream importedTexturesEvents;
            std::ostringstream importedTexturesCheck;

            for (const RenderGraphNode& nodeBase : renderGraph.nodes)
            {
                if (nodeBase._index != RenderGraphNode::c_index_resourceTexture)
                    continue;

                const RenderGraphNode_Resource_Texture& node = nodeBase.resourceTexture;

                if (node.visibility != ResourceVisibility::Imported)
                    continue;

                // The UI
                {
                    importedTextures << "                        <tr><td>Texture URL: ";

                    if (!node.comment.empty())
                        importedTextures << "<div class=\"tooltip\">" << node.name << "<span class=\"tooltiptext\">" << node.comment << "</span></div>";
                    else
                        importedTextures << node.name;

                    importedTextures << "&nbsp;</td><td><input type=\"text\" id=\"importedResourceURL_" + node.name + "\"></td></tr>\n";
                }

                // The javascript when it changes
                {
                    importedTexturesEvents <<
                        "            document.getElementById('importedResourceURL_" << node.name << "').addEventListener('change', async (event) =>\n"
                        "                {\n"
                        "                    if (typeof " << renderGraph.name << ".importedResourceURL_" << node.name << " === 'undefined' || " << renderGraph.name << ".importedResourceURL_" << node.name << " != event.target.value)\n"
                        "                    {\n"
                        "                        " << renderGraph.name << ".importedResourceURL_" << node.name << " = event.target.value;\n"
                        "                        const loadedTexture = await Shared.CreateTextureWithPNG(device, event.target.value, " << renderGraph.name << ".texture_" << node.name << "_usageFlags);\n"
                        "                        if (loadedTexture !== null)\n"
                        "                        {\n"
                        "                            " << renderGraph.name << ".texture_" << node.name << " = loadedTexture.texture;\n"
                        "                            " << renderGraph.name << ".texture_" << node.name << "_size = loadedTexture.size;\n"
                        "                            " << renderGraph.name << ".texture_" << node.name << "_format = loadedTexture.format;\n"
                        "                        }\n"
                        "                    }\n"
                        "                }\n"
                        "            );\n"
                        ;
                }

                // Gate technique execution on whether we have the imported resources we need or not
                importedTexturesCheck << " || " << renderGraph.name << ".texture_" << node.name << " === null";
            }

            if (!importedTextures.str().empty())
            {
                std::string style;
                if (true)
                    style = " style=\"display:block\"";
                else
                    style = " style=\"display:none\"";

                stringReplacementMap["/*$(ImportedResourcesUI)*/"] <<
                    "                    <button type=\"button\" class=\"collapsible\">Imported Resources</button>\n"
                    "                    <table cellspacing=\"0\" cellpadding=\"0\" class=\"collapsiblecontent\"" << style << ">\n"
                    << importedTextures.str() <<
                    "                    </table>\n"
                    "                    <br><br>\n"
                    ;

                stringReplacementMap["/*$(InitOneTimeImportedResources)*/"] <<
                    "            // Hook up the on change events for imported resources\n"
                    << importedTexturesEvents.str() <<
                    "\n"
                    ;

                stringReplacementMap["/*$(ImportedResourceCheck)*/"] <<
                    "// Don't run the technique if any imported resources are missing\n"
                    "                if ( " << importedTexturesCheck.str().substr(4) << " )\n"
                    "                {\n"
                    "                    requestAnimationFrame(RenderFrame);\n"
                    "                    return;\n"
                    "                }\n"
                    "\n                "
                    ;
            }
        }

        // Deal with gguser file settings
        {
            stringReplacementMap["/*$(StartingCameraPos)*/"] << "[ " << ggUserFile.systemVars.camera.startingCameraPos[0] << ", " << ggUserFile.systemVars.camera.startingCameraPos[1] << ", " << ggUserFile.systemVars.camera.startingCameraPos[2] << " ]";
            stringReplacementMap["/*$(StartingCameraAltitudeAzimuth)*/"] << "[ " << ggUserFile.systemVars.camera.startingCameraAltitudeAzimuth[0] << ", " << ggUserFile.systemVars.camera.startingCameraAltitudeAzimuth[1] << " ]";
            stringReplacementMap["/*$(CameraLeftHanded)*/"] << (ggUserFile.systemVars.camera.leftHanded ? "true" : "false");
            stringReplacementMap["/*$(CameraFlySpeed)*/"] << ggUserFile.systemVars.camera.flySpeed;
            stringReplacementMap["/*$(CameraMouseSensitivity)*/"] << ggUserFile.systemVars.camera.mouseSensitivity;

            // iFrame
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.iFrame_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Set the frame index\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iFrame_varName << " = frameIndex;\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iFrame_varName << " = true;\n"
                    "\n"
                    ;
            }

            // WindowSize
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.WindowSize_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Set the window size\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.WindowSize_varName << "[0] = canvas.width;\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.WindowSize_varName << "[0] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.WindowSize_varName << "[1] = canvas.height;\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.WindowSize_varName << "[1] = true;\n"
                    "\n"
                    ;
            }

            // iResolution
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.iResolution_varName.c_str()) != -1 &&
                FrontEndNodesNoCaching::GetNodeIndexByName(renderGraph, ggUserFile.systemVars.iResolution_textureName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Set the resolution\n"
                    "                if (" << renderGraph.name << ".texture_" << ggUserFile.systemVars.iResolution_textureName << " !== null)\n"
                    "                {\n"
                    "                    " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iResolution_varName << "[0] = " << renderGraph.name << ".texture_" << ggUserFile.systemVars.iResolution_textureName << ".width;\n"
                    "                    " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iResolution_varName << "[0] = true;\n"
                    "                    " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iResolution_varName << "[1] = " << renderGraph.name << ".texture_" << ggUserFile.systemVars.iResolution_textureName << ".height;\n"
                    "                    " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iResolution_varName << "[1] = true;\n"
                    "                    " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iResolution_varName << "[2] = 1;\n"
                    "                    " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iResolution_varName << "[2] = true;\n"
                    "                }\n"
                    "\n"
                    ;
            }

            // iTime
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.iTime_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Time in seconds since this started running\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iTime_varName << " = (currentTimeMs - startTimeMs) / 1000;\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iTime_varName << " = true;\n"
                    "\n"
                    ;
            }

            // iTimeDelta
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.iTimeDelta_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // How long RenderFrame() took last frame, in seconds\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iTimeDelta_varName << " = (currentTimeMs - lastTimeMs) / 1000;\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iTimeDelta_varName << " = true;\n"
                    "\n"
                    ;
            }

            // iFrameRate
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.iFrameRate_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Frames per second\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iFrameRate_varName << " = 1000 / (currentTimeMs - lastTimeMs);\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iFrameRate_varName << " = true;\n"
                    "\n"
                    ;
            }

            // iMouse
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.iMouse_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Mouse: xy is where user last clicked. zw is the click location for the single frame when a user clicks, else is zero.\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iMouse_varName << "[0] = iMouse[0];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iMouse_varName << "[0] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iMouse_varName << "[1] = iMouse[1];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iMouse_varName << "[1] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iMouse_varName << "[2] = iMouse[2];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iMouse_varName << "[2] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.iMouse_varName << "[3] = iMouse[3];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.iMouse_varName << "[3] = true;\n"
                    "\n"
                    ;
            }

            // Mouse State
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.MouseState_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Mouse State: xy is mouse position. z is left mouse button state. w is right mouse button state.\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseState_varName << "[0] = MouseState[0];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseState_varName << "[0] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseState_varName << "[1] = MouseState[1];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseState_varName << "[1] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseState_varName << "[2] = MouseState[2];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseState_varName << "[2] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseState_varName << "[3] = MouseState[3];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseState_varName << "[3] = true;\n"
                    "\n"
                    ;
            }

            // Mouse State Last Frame
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.MouseStateLastFrame_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Mouse State Last Frame: xy is mouse position. z is left mouse button state. w is right mouse button state.\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[0] = MouseStateLastFrame[0];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[0] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[1] = MouseStateLastFrame[1];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[1] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[2] = MouseStateLastFrame[2];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[2] = true;\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[3] = MouseStateLastFrame[3];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.MouseStateLastFrame_varName << "[3] = true;\n"
                    "\n"
                    ;
            }

            // View Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.ViewMtx_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // View matrix\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.ViewMtx_varName << " = Shared.GetViewMatrix(CameraPos, CameraAltitudeAzimuth);\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.ViewMtx_varName << " = new Array(16).fill(true);\n"
                    "\n"
                    ;
            }

            // Inv View Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.InvViewMtx_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Inverse view matrix\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.InvViewMtx_varName << " = Shared.GetInvViewMatrix(CameraPos, CameraAltitudeAzimuth);\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.InvViewMtx_varName << " = new Array(16).fill(true);\n"
                    "\n"
                    ;
            }

            // Projection Matrix (resolution) Texture
            if (!ggUserFile.systemVars.ProjMtx_textureName.empty())
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Projection matrix resolution texture\n"
                    "                const projMtxResolution = [ " << renderGraph.name << ".texture_" << ggUserFile.systemVars.ProjMtx_textureName << "_size[0], " << renderGraph.name << ".texture_" << ggUserFile.systemVars.ProjMtx_textureName << "_size[1] ];\n"
                    "\n"
                    ;
            }
            else
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Projection matrix resolution texture\n"
                    "                const projMtxResolution = [ 1, 1 ];\n"
                    "\n"
                    ;
            }

            // Proj Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.ProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Proj matrix\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.ProjMtx_varName << " = Shared.GetProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.ProjMtx_varName << " = new Array(16).fill(true);\n"
                    "\n"
                    ;
            }

            // Jittered Proj Mtx
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.JitteredProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Jittered proj matrix\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.JitteredProjMtx_varName << " = Shared.GetProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.JitteredProjMtx_varName << " = new Array(16).fill(true);\n"
                    "\n"
                    ;
            }

            // Inv Proj Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.InvProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Inverse Proj matrix\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.InvProjMtx_varName << " = Shared.GetInvProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.InvProjMtx_varName << " = new Array(16).fill(true);\n"
                    "\n"
                    ;
            }

            // Inv Jittered Proj Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.InvJitteredProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Inverse Jittered Proj matrix\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.InvJitteredProjMtx_varName << " = Shared.GetInvProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.InvJitteredProjMtx_varName << " = new Array(16).fill(true);\n"
                    "\n"
                    ;
            }

            // View Proj Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.ViewProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // View Proj matrix\n"
                    "                {\n"
                    "                    const viewMtx = Shared.GetViewMatrix(CameraPos, CameraAltitudeAzimuth);\n"
                    "                    const projMtx = Shared.GetProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                    " << renderGraph.name << ".variable_" << ggUserFile.systemVars.ViewProjMtx_varName << " = Shared.MatMul(viewMtx, projMtx);\n"
                    "                    " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.ViewProjMtx_varName << " = new Array(16).fill(true);\n"
                    "                }\n"
                    "\n"
                    ;
            }

            // Jittered View Proj Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.JitteredViewProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Jittered View Proj matrix\n"
                    "                {\n"
                    "                    const viewMtx = Shared.GetViewMatrix(CameraPos, CameraAltitudeAzimuth);\n"
                    "                    const projMtx = Shared.GetProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                    " << renderGraph.name << ".variable_" << ggUserFile.systemVars.JitteredViewProjMtx_varName << " = Shared.MatMul(viewMtx, projMtx);\n"
                    "                    " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.JitteredViewProjMtx_varName << " = new Array(16).fill(true);\n"
                    "                }\n"
                    "\n"
                    ;
            }

            // Inverse View Proj Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.InvViewProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Inverse View Proj matrix\n"
                    "                {\n"
                    "                    const viewMtx = Shared.GetInvViewMatrix(CameraPos, CameraAltitudeAzimuth);\n"
                    "                    const projMtx = Shared.GetInvProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                    " << renderGraph.name << ".variable_" << ggUserFile.systemVars.InvViewProjMtx_varName << " = Shared.MatMul(projMtx, viewMtx);\n"
                    "                    " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.InvViewProjMtx_varName << " = new Array(16).fill(true);\n"
                    "                }\n"
                    "\n"
                    ;
            }

            // Inverse Jittered View Proj Matrix
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.InvJitteredViewProjMtx_varName.c_str()) != -1)
            {
                const GGUserFile_Camera& camera = ggUserFile.systemVars.camera;

                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Inverse Jittered View Proj matrix\n"
                    "                {\n"
                    "                    const viewMtx = Shared.GetInvViewMatrix(CameraPos, CameraAltitudeAzimuth);\n"
                    "                    const projMtx = Shared.GetInvProjMatrix("
                    << camera.FOV << ", projMtxResolution, " << camera.nearPlane << ", " << camera.farPlane << ", " << (camera.reverseZ ? "true" : "false")
                    << ", " << (camera.perspective ? "true" : "false") << ", CameraLeftHanded);\n"
                    "                    " << renderGraph.name << ".variable_" << ggUserFile.systemVars.InvJitteredViewProjMtx_varName << " = Shared.MatMul(projMtx, viewMtx);\n"
                    "                    " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.InvJitteredViewProjMtx_varName << " = new Array(16).fill(true);\n"
                    "                }\n"
                    "\n"
                    ;
            }

            // Camera Pos
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.CameraPos_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Camera Position\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.CameraPos_varName << " = CameraPos.slice();\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.CameraPos_varName << " = new Array(3).fill(true);\n"
                    "\n"
                    ;
            }

            // Altitude Azimuth
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.CameraAltitudeAzimuth_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Camera Orientation\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.CameraAltitudeAzimuth_varName << " = CameraAltitudeAzimuth.slice();\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.CameraAltitudeAzimuth_varName << " = new Array(2).fill(true);\n"
                    "\n"
                    ;
            }

            // Camera Changed
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.CameraChanged_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Camera Changed\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.CameraChanged_varName << " = CameraChanged;\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.CameraChanged_varName << " = true;\n"
                    "\n"
                    ;
            }

            // Camera Jitter
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.CameraJitter_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Camera Jitter\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.CameraJitter_varName << " = [ 0, 0 ];\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.CameraJitter_varName << " = new Array(2).fill(true);\n"
                    "\n"
                    ;
            }

            // Camera FOV
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.CameraFOV_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Camera FOV\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.CameraFOV_varName << " = " << ggUserFile.systemVars.camera.FOV << ";\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.CameraFOV_varName << " = true;\n"
                    "\n"
                    ;
            }

            // Camera Near Plane
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.CameraNearPlane_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Camera Near Plane\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.CameraNearPlane_varName << " = " << ggUserFile.systemVars.camera.nearPlane << ";\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.CameraNearPlane_varName << " = true;\n"
                    "\n"
                    ;
            }

            // Camera Far Plane
            if (GetVariableIndex(renderGraph, ggUserFile.systemVars.CameraFarPlane_varName.c_str()) != -1)
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Camera Far Plane\n"
                    "                " << renderGraph.name << ".variable_" << ggUserFile.systemVars.CameraFarPlane_varName << " = " << ggUserFile.systemVars.camera.farPlane << ";\n"
                    "                " << renderGraph.name << ".variableChanged_" << ggUserFile.systemVars.CameraFarPlane_varName << " = true;\n"
                    "\n"
                    ;
            }

            // Key State Buffer
            if (!ggUserFile.systemVars.KeyState_bufferName.empty())
            {
                stringReplacementMap["/*$(IndexHTMLPreExecute)*/"] <<
                    "                // Key States Buffer\n"
                    "                if (" << renderGraph.name << ".buffer_" << ggUserFile.systemVars.KeyState_bufferName << " !== null)\n"
                    "                {\n"
                    "                    let KeyStatesU32 = new Uint32Array(KeyStates);\n"
                    "                    let KeyStatesLastFrameU32 = new Uint32Array(KeyStatesLastFrame);\n"
                    "                    device.queue.writeBuffer(" << renderGraph.name << ".buffer_" << ggUserFile.systemVars.KeyState_bufferName << ", 0, KeyStatesU32);\n"
                    "                    device.queue.writeBuffer(" << renderGraph.name << ".buffer_" << ggUserFile.systemVars.KeyState_bufferName << ", 1024, KeyStatesLastFrameU32);\n"
                    "                }\n"
                    "\n"
                    ;
            }
        }

        // warn about vertex shader input location ordering if we should
        {
            for (const Shader& shader : renderGraph.shaders)
            {
                if (shader.type == ShaderType::Vertex)
                {
                    stringReplacementMap["/*$(VertexBufferWarn)*/"] <<
                        "    " << shader.name << "\n"
                        ;
                }
            }

            if (!stringReplacementMap["/*$(VertexBufferWarn)*/"].str().empty())
            {
                std::string old = stringReplacementMap["/*$(VertexBufferWarn)*/"].str();
                stringReplacementMap["/*$(VertexBufferWarn)*/"] = std::ostringstream();
                stringReplacementMap["/*$(VertexBufferWarn)*/"] <<
                    "\n\nGenerating vertex shaders from slang can arbitrarily re-number the @location of each field in a vertex buffer declared in a shader.\n"
                    "Please make sure the \"vertexBufferAttributes\" have \"shaderLocation\" values that match what is used in the vertex shader.\n"
                    "Shaders potentially affected:\n"
                    << old << 
                    "Please see UserDocumentation/WebGPU.pdf for more information."
                    ;

                ShowWarningMessage("%s", stringReplacementMap["/*$(VertexBufferWarn)*/"].str().c_str());
            }
        }

        // View list
        {
            // 4 lists: Primary Output, exported, imported, internal
            std::vector<std::string> textureViewList[4];

            if (!renderGraph.PrimaryOutput.name.empty())
                textureViewList[0].push_back(renderGraph.PrimaryOutput.name);

            for (const RenderGraphNode& nodeBase : renderGraph.nodes)
            {
                if (nodeBase._index != RenderGraphNode::c_index_resourceTexture)
                    continue;

                const RenderGraphNode_Resource_Texture& node = nodeBase.resourceTexture;
                switch (node.visibility)
                {
                    case ResourceVisibility::Exported: textureViewList[1].push_back(node.name); break;
                    case ResourceVisibility::Imported: textureViewList[2].push_back(node.name); break;
                    case ResourceVisibility::Internal: textureViewList[3].push_back(node.name); break;
                }
            }

            // sort the lists alphabetically
            for (int i = 1; i < 4; ++i)
                std::sort(textureViewList[i].begin(), textureViewList[i].end());

            // Help make sure textureViewList[0] has an item in it. This is the default texture shown.
            if (textureViewList[0].empty())
            {
                for (int i = 1; i < 4; ++i)
                {
                    if (!textureViewList[i].empty())
                    {
                        textureViewList[0].push_back(textureViewList[i][0]);
                        break;
                    }
                }
            }

            // Make the selection options
            const char* labels[4] =
            {
                "",
                "----- Exported Textures -----",
                "----- Imported Textures -----",
                "----- Internal Textures -----",
            };

            for (int i = 0; i < 4; ++i)
            {
                if (textureViewList[i].empty())
                    continue;

                if (labels[i][0] != 0)
                    stringReplacementMap["/*$(ViewingList)*/"] << "                        <option value=\"\">" << labels[i] << "</option><br/>\n";

                for (std::string& textureName : textureViewList[i])
                    stringReplacementMap["/*$(ViewingList)*/"] << "                        <option value=\"" << textureName << "\">" << textureName << "</option><br/>\n";
            }

            if (!textureViewList[0].empty())
                stringReplacementMap["/*$(DefaultViewingTexture)*/"] << textureViewList[0][0];
        }

        // Spify up the output as needed
        if (!stringReplacementMap["/*$(ViewingList)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(ViewingList)*/"].str();
            stringReplacementMap["/*$(ViewingList)*/"] = std::ostringstream();
            stringReplacementMap["/*$(ViewingList)*/"] <<
                "\n"
                "                    <br/><br/>\n"
                "                    <label for=\"Viewing\">Viewing</label>\n"
                "                    <select name=\"Viewing\" id=\"Viewing\">\n"
                << old <<
                "                    </select>"
                ;

            stringReplacementMap["/*$(ViewingListEvent)*/"] <<
                "\n"
                "            document.getElementById(\"Viewing\").addEventListener('change', (event) => { if (event.target.value.length != 0) viewingTextureName = event.target.value; });"
                ;
        }

        if (!stringReplacementMap["/*$(HTMLUI)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(HTMLUI)*/"].str();
            stringReplacementMap["/*$(HTMLUI)*/"] = std::ostringstream();
            stringReplacementMap["/*$(HTMLUI)*/"] <<
                old <<
                "                    <button type=\"button\" id=\"ResetVariables\">Reset Variables</button>\n"
                "                    <br/>\n"
                "                    <br/>\n"
                ;
        }

        if (!stringReplacementMap["/*$(ReconcileVariableUI)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(ReconcileVariableUI)*/"].str();
            stringReplacementMap["/*$(ReconcileVariableUI)*/"] = std::ostringstream();
            stringReplacementMap["/*$(ReconcileVariableUI)*/"] << "                // If the technique changed the variables, update the UI\n" << old << "\n";
        }

        if (!stringReplacementMap["/*$(InitOneTime)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(InitOneTime)*/"].str();
            stringReplacementMap["/*$(InitOneTime)*/"] = std::ostringstream();

            if (renderGraph.variables.size() > 0)
                stringReplacementMap["/*$(InitOneTime)*/"] << "            // Hook up the on change events for UI\n";

            stringReplacementMap["/*$(InitOneTime)*/"] << old << "\n";
        }

        if (!stringReplacementMap["/*$(MembersImported)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(MembersImported)*/"].str();
            stringReplacementMap["/*$(MembersImported)*/"] = std::ostringstream();
            stringReplacementMap["/*$(MembersImported)*/"] << "// -------------------- Imported Members\n\n" << old;
        }

        if (!stringReplacementMap["/*$(MembersInternal)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(MembersInternal)*/"].str();
            stringReplacementMap["/*$(MembersInternal)*/"] = std::ostringstream();
            stringReplacementMap["/*$(MembersInternal)*/"] << "// -------------------- Private Members\n\n" << old;
        }

        if (!stringReplacementMap["/*$(MembersExported)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(MembersExported)*/"].str();
            stringReplacementMap["/*$(MembersExported)*/"] = std::ostringstream();
            stringReplacementMap["/*$(MembersExported)*/"] << "// -------------------- Exported Members\n\n" << old;
        }

        if (!stringReplacementMap["/*$(TODO_NeedUserInput)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(TODO_NeedUserInput)*/"].str();
            stringReplacementMap["/*$(TODO_NeedUserInput)*/"] = std::ostringstream();
            stringReplacementMap["/*$(TODO_NeedUserInput)*/"] << ":" << old;

            stringReplacementMap["/*$(TODO_NeedUserInput)*/"] <<
                "\n"
                "\n"
                "                An example of how to load a texture:\n"
                "                    const loadedTexture = await Shared.CreateTextureWithPNG(device, \"cabinsmall.png\", " << renderGraph.name << ".texture_someTexture_usageFlags);\n"
                "                    " << renderGraph.name << ".texture_someTexture = loadedTexture.texture;\n"
                "                    " << renderGraph.name << ".texture_someTexture_size = loadedTexture.size;\n"
                "                    " << renderGraph.name << ".texture_someTexture_format = loadedTexture.format;\n"
                "\n"
                "                See _GeneratedCode\\UnitTests\\WebGPU\\UnitTestLogic.js for more examples of loading files of various kinds."
                ;
        }

        if (!stringReplacementMap["/*$(VariablesPrivate)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(VariablesPrivate)*/"].str();
            stringReplacementMap["/*$(VariablesPrivate)*/"] = std::ostringstream();
            stringReplacementMap["/*$(VariablesPrivate)*/"] << "// -------------------- Private Variables\n\n" << old;
        }

        if (!stringReplacementMap["/*$(VariablesPublic)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(VariablesPublic)*/"].str();
            stringReplacementMap["/*$(VariablesPublic)*/"] = std::ostringstream();
            stringReplacementMap["/*$(VariablesPublic)*/"] << "// -------------------- Public Variables\n\n" << old << "\n";
        }

        if (!stringReplacementMap["/*$(Structs)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(Structs)*/"].str();
            stringReplacementMap["/*$(Structs)*/"] = std::ostringstream();
            stringReplacementMap["/*$(Structs)*/"] << "// -------------------- Structs\n\n" << old;
        }

        if (!stringReplacementMap["/*$(StructVBAttribs)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(StructVBAttribs)*/"].str();
            stringReplacementMap["/*$(StructVBAttribs)*/"] = std::ostringstream();
            stringReplacementMap["/*$(StructVBAttribs)*/"] << "// -------------------- Struct Vertex Buffer Attributes\n\n" << old;
        }

        if (!stringReplacementMap["/*$(Enums)*/"].str().empty())
        {
            std::string old = stringReplacementMap["/*$(Enums)*/"].str();
            stringReplacementMap["/*$(Enums)*/"] = std::ostringstream();
            stringReplacementMap["/*$(Enums)*/"] << "// -------------------- Enums\n\n" << old;
        }
    }
};

static void FixupShaderUAVDeclarations(const RenderGraph& renderGraph, const Shader& shader, std::string& shaderCode)
{
    for (const WebGPU_RWTextureSplit& textureSplit : renderGraph.backendData.webGPU.RWTextureSplits)
    {
        if (textureSplit.shaderName != shader.name)
            continue;

        // Handle the write access
        {
            // find the variable declaration
            // If it isn't found, it could be that the read is never actually used, so is deadstripped away, and is not actually an error.
            char buffer[1024];
            sprintf_s(buffer, "var %s :", textureSplit.pinName.c_str());
            size_t offset = shaderCode.find(buffer);
            if (offset != std::string::npos)
            {
                // find the <
                size_t bracketOpen = shaderCode.find('<', offset);
                if (bracketOpen == std::string::npos)
                {
                    ShowErrorMessage("Could not find open bracket for \"%s\" in shader \"%s\"", textureSplit.pinName.c_str(), shader.name.c_str());
                    continue;
                }

                // find the >
                size_t bracketClose = shaderCode.find('>', bracketOpen);
                if (bracketClose == std::string::npos)
                {
                    ShowErrorMessage("Could not find close bracket for \"%s\" in shader \"%s\"", textureSplit.pinName.c_str(), shader.name.c_str());
                    continue;
                }

                // Replace
                sprintf_s(buffer, "/*(%s_format)*/", textureSplit.pinName.c_str());
                shaderCode =
                    shaderCode.substr(0, bracketOpen + 1) +
                    buffer +
                    ", write" +
                    shaderCode.substr(bracketClose)
                    ;
            }
        }

        // Handle the read access
        {
            // find the variable declaration
            // If it isn't found, it could be that the read is never actually used, so is deadstripped away, and is not actually an error.
            char buffer[1024];
            sprintf_s(buffer, "var %s :", textureSplit.pinNameR.c_str());
            size_t offset = shaderCode.find(buffer);
            if (offset != std::string::npos)
            {
                // find the <
                size_t bracketOpen = shaderCode.find('<', offset);
                if (bracketOpen == std::string::npos)
                {
                    ShowErrorMessage("Could not find open bracket for \"%s\" in shader \"%s\"", textureSplit.pinNameR.c_str(), shader.name.c_str());
                    continue;
                }

                // find the >
                size_t bracketClose = shaderCode.find('>', bracketOpen);
                if (bracketClose == std::string::npos)
                {
                    ShowErrorMessage("Could not find close bracket for \"%s\" in shader \"%s\"", textureSplit.pinNameR.c_str(), shader.name.c_str());
                    continue;
                }

                // Replace
                sprintf_s(buffer, "/*(%s_format)*/", textureSplit.pinNameR.c_str());
                shaderCode =
                    shaderCode.substr(0, bracketOpen + 1) +
                    buffer +
                    ", read" +
                    shaderCode.substr(bracketClose)
                ;
            }
        }
    }
}

static void ProcessShaderFile(const std::string& nodeName, const Shader& shader, std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const char* outFolder, const RenderGraph& renderGraph)
{
    if (shader.language == ShaderLanguage::WGSL)
    {
        Assert(false, "WGSL is not yet supported as a source language for webgpu code generation. Shader \"%s\"", shader.name.c_str());
        return;
    }

    if (shader.language != ShaderLanguage::HLSL && shader.language != ShaderLanguage::Slang)
    {
        Assert(false, "Unsupported shader source language encountered for shader \"%s\": %s", shader.name.c_str(), EnumToString(shader.language));
        return;
    }

    bool usesRT = false;
    for (const ShaderResource& resource : shader.resources)
    {
        if (resource.originalAccess == ShaderResourceAccessType::RTScene)
        {
            usesRT = true;
            break;
        }
    }

    // shaders that use RT need a version that uses BVHs and a version that does not
    for (int shaderTypeIndex = 0; shaderTypeIndex < (usesRT ? 2 : 1); ++shaderTypeIndex)
    {
        bool isBVHShader = (shaderTypeIndex == 1);
        bool isNonBVHShader = (usesRT && shaderTypeIndex == 0);

        ProcessShaderOptions_HLSL options(shader);

        options.m_writeSamplerDefinition =
            [](const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderSampler& sampler)
            {
                stream << "\n[[vk::binding(" << sampler.registerIndex << ")]]";
                ProcessShaderOptions_HLSL::WriteSamplerDefinition_Register(options, stream, sampler);
            }
        ;

        options.m_writeResourceDefinition =
            [](const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource)
            {
                stream << "\n[[vk::binding(" << resource.registerIndex << ")]]";
                ProcessShaderOptions_HLSL::WriteResourceDefinition_Register(options, stream, renderGraph, resource);
            }
        ;

        options.m_handleShaderToken =
            [](ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const std::string& token, size_t lineNumber, const RenderGraph& renderGraph, const std::string& declareLineNumber)
            {
                // Only rw texture READS need to be updated. Writes have the same resource name so can go through the default behavior.
                std::string param;
                if (GetTokenParameter(token.c_str(), "RWTextureR", param))
                {
                    // Do replacement of texture to buffer if we should
                    for (const WebGPU_RWTextureSplit& textures : renderGraph.backendData.webGPU.RWTextureSplits)
                    {
                        if (textures.shaderName == options.m_shader.name && textures.pinName == param)
                        {
                            stream = std::ostringstream();
                            stream << textures.pinNameR;
                            return true;
                        }
                    }

                    // Default behavior if we got here. no replacement occured.
                    return false;
                }
                else if (GetTokenParameter(token.c_str(), "_raygeneration", param))
                {
                    stream = std::ostringstream();
                    stream <<
                        "[numthreads(" << options.m_shader.NumThreads[0] << ", " << options.m_shader.NumThreads[1] << ", " << options.m_shader.NumThreads[2] << ")]\n"
                        << declareLineNumber <<
                        "void " << param << "(uint3 DTid : SV_DispatchThreadID)"
                        ;
                }
                else if (GetTokenParameter(token.c_str(), "_miss", param))
                {
                    stream = std::ostringstream();
                    stream <<
                        "// Ray miss shader\n"
                        << declareLineNumber <<
                        "void " << param << "(inout Payload payload)";
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

                    stream = std::ostringstream();
                    stream <<
                        "// Ray closest hit\n"
                        << declareLineNumber <<
                        "void " << entryPoint << "(inout Payload payload, in " << attribStruct << " intersection)";

                    options.m_attribStructsUsed.insert(attribStruct);
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

                    stream = std::ostringstream();
                    stream <<
                        "// Ray any hit\n"
                        << declareLineNumber <<
                        "void " << entryPoint << "(inout Payload payload, in " << attribStruct << " attr, inout uint _anyHitResult, in RayDesc _ray, in float _RayTCurrent, in uint _PrimitiveIndex)";

                    options.m_attribStructsUsed.insert(attribStruct);
                }
                else if (GetTokenParameter(token.c_str(), "_intersection", param))
                {
                    stream = std::ostringstream();
                    stream <<
                        "// Ray intersection\n"
                        << declareLineNumber <<
                        "void " << param << "(in RayDesc _ray, inout float _RayTCurrent, in uint _PrimitiveIndex, inout uint _HitPrimitiveIndex, in uint _hitGroupIndex, inout Payload _payload, inout GIGI_INTERSECTIONINFOOBJECTTYPE _attributes)";
                }
                else
                {
                    return false;
                }

                return true;
            }
        ;

        options.m_handleGeneralStringReplacement =
            [isBVHShader](std::string& shaderCode, std::unordered_map<std::string, std::ostringstream>& shaderSpecificStringReplacementMap, const ProcessShaderOptions_HLSL& options, const RenderGraph& renderGraph)
            {
                // Append the shader defines as #defines
                {
                    std::ostringstream defines;
                    for (const ShaderDefine& define : options.m_shader.defines)
                        defines << "#define " << define.name << " " << define.value << "\n";

                    // GIGI_SCENE_IS_BVH define
                    defines << "#define GIGI_SCENE_IS_BVH " << (isBVHShader ? 1 : 0) << "\n";

                    shaderCode = defines.str() + shaderCode;
                }

                // ReportHit()
                StringReplaceAll(shaderCode, "ReportHit(", "IntersectShaderReportHit(_ray, _RayTCurrent, _hitGroupIndex, _payload, _PrimitiveIndex, _HitPrimitiveIndex, _attributes, ");

                // DispatchRaysDimensions()
                {
                    char variableName[1024];
                    sprintf_s(variableName, "_dispatchSize_%s", options.m_shader.name.c_str());
                    int varIndex = GetVariableIndex(renderGraph, variableName);
                    if (varIndex != -1)
                    {
                        const Variable& variable = renderGraph.variables[varIndex];

                        std::ostringstream varText;
                        options.m_writeVariableReference(options, varText, variable);

                        StringReplaceAll(shaderCode, "DispatchRaysDimensions()", varText.str());
                    }
                }

                // Gather information about an RTScene parameter, if it exists
                bool hasRTScene = false;
                std::string rtSceneName = "";
                std::string rtSceneTypeName = "float3";
                std::string rtScenePositionFieldName = "";
                {
                    int index = -1;
                    for (const ShaderResource& resource : options.m_shader.resources)
                    {
                        index++;
                        if (resource.originalAccess == ShaderResourceAccessType::RTScene)
                        {
                            hasRTScene = true;
                            rtSceneName = resource.name;

                            if (resource.buffer.typeStruct.structIndex != -1)
                            {
                                const Struct& s = renderGraph.structs[resource.buffer.typeStruct.structIndex];
                                rtSceneTypeName = std::string("Struct_") + s.name;

                                int positionFieldIndex = -1;
                                for (int fieldIndex = 0; fieldIndex < (int)s.fields.size(); ++fieldIndex)
                                {
                                    if (s.fields[fieldIndex].semantic == StructFieldSemantic::Position)
                                    {
                                        positionFieldIndex = fieldIndex;
                                        break;
                                    }
                                }

                                Assert(positionFieldIndex != -1, "Could not find position field index in struct \"%s\" in shader \"%s\"", s.name.c_str(), options.m_shader.name.c_str());
                                rtScenePositionFieldName = std::string(".") + s.fields[positionFieldIndex].name;
                            }
                            else
                            {
                                rtSceneTypeName = DataFieldTypeToShaderType(resource.buffer.type);
                            }
                        }
                    }
                }

                // Raytracing common - needed for both inline, and dxr raytracing
                // https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
                // Note: you can make a raygen shader without an RTScene
                if (hasRTScene)
                {
                    Assert(options.m_attribStructsUsed.size() <= 1, "Multiple intersection attribute structs used in the same shader.");
                    std::string attributeStruct = "BuiltInTriangleIntersectionAttributes";
                    for (const std::string& a : options.m_attribStructsUsed)
                    {
                        attributeStruct = a;
                        break;
                    }
                    bool customIntersectionInfo = attributeStruct != "BuiltInTriangleIntersectionAttributes";

                    shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                        // This turns into a function call to SCENE_IS_CUBES(), which doesn't exist. We add that function to the wgsl shader in the javascript,
                        // to configure at runtime.
                        "property bool SCENE_IS_AABBS {\n"
                        "  get{ __intrinsic_asm \"SCENE_IS_AABBS\"; }\n"
                        "}\n"
                        "\n"
                        "#define GIGI_RTSCENENAME " << rtSceneName << "\n"
                        "#define GIGI_RTSCENETYPENAME " << rtSceneTypeName << "\n"
                        "#define GIGI_RTSCENEPOSFIELDNAME " << rtScenePositionFieldName << "\n"
                        "#define GIGI_ISRAYGENSHADER " << (options.m_shader.type == ShaderType::RTRayGen ? "1" : "0") << "\n"
                        "#define GIGI_INTERSECTIONINFOOBJECTTYPE " << attributeStruct << "\n"
                        "#define GIGI_INTERSECTIONINFOCUSTOM " << (customIntersectionInfo ? "1" : "0") << "\n"
                        "\n"
                        "#include \"GigiRayTracingEmulation.hlsli\"\n"
                        "\n"
                        ;

                    // Ray gen shaders need more
                    if (options.m_shader.type == ShaderType::RTRayGen)
                    {
                        Assert(options.m_attribStructsUsed.size() <= 1, "Multiple intersection attribute structs used in the same shader.");
                        std::string attributeStruct = "BuiltInTriangleIntersectionAttributes";
                        for (const std::string& a : options.m_attribStructsUsed)
                        {
                            attributeStruct = a;
                            break;
                        }

                        // RunIntersectionShader() - Call intersection based on hitGroupIndex
                        {
                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "void RunIntersectionShader(in RayDesc Ray, inout float minT, in uint primitiveIndex, inout uint hitIndex, in uint hitGroupIndex, inout Payload payload, inout GIGI_INTERSECTIONINFOOBJECTTYPE intersectionInfo)\n"
                                "{\n"
                                "    switch (hitGroupIndex)\n"
                                "    {\n"
                                ;

                            for (int usedRTHitGroupIndex = 0; usedRTHitGroupIndex < (int)options.m_shader.Used_RTHitGroupIndex.size(); ++usedRTHitGroupIndex)
                            {
                                const std::string& hitGroupName = options.m_shader.Used_RTHitGroupIndex[usedRTHitGroupIndex];
                                int hitGroupIndex = GetHitGroupIndex(renderGraph, hitGroupName.c_str());
                                Assert(hitGroupIndex != -1, "Could not find hit group %s", hitGroupName.c_str());
                                const RTHitGroup& hitGroup = renderGraph.hitGroups[hitGroupIndex];

                                if (hitGroup.intersection.shaderIndex == -1)
                                    continue;

                                const Shader& shader = renderGraph.shaders[hitGroup.intersection.shaderIndex];
                                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                    "        case " << usedRTHitGroupIndex << ": " << shader.entryPoint << "(Ray, minT, primitiveIndex, hitIndex, hitGroupIndex, payload, intersectionInfo); break;\n"
                                    ;
                            }

                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "    }\n"
                                "}\n"
                                "\n"
                                ;
                        }

                        // RunAnyHitShader() - Call any hit based on hitGroupIndex
                        {
                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "void RunAnyHitShader(inout Payload payload, inout GIGI_INTERSECTIONINFOOBJECTTYPE intersectionInfo, inout uint anyHitResult, in RayDesc Ray, in float minT, in uint primitiveIndex, in uint hitGroupIndex)\n"
                                "{\n"
                                "    switch (hitGroupIndex)\n"
                                "    {\n"
                                ;

                            for (int usedRTHitGroupIndex = 0; usedRTHitGroupIndex < (int)options.m_shader.Used_RTHitGroupIndex.size(); ++usedRTHitGroupIndex)
                            {
                                const std::string& hitGroupName = options.m_shader.Used_RTHitGroupIndex[usedRTHitGroupIndex];
                                int hitGroupIndex = GetHitGroupIndex(renderGraph, hitGroupName.c_str());
                                Assert(hitGroupIndex != -1, "Could not find hit group %s", hitGroupName.c_str());
                                const RTHitGroup& hitGroup = renderGraph.hitGroups[hitGroupIndex];

                                if (hitGroup.anyHit.shaderIndex == -1)
                                    continue;

                                const Shader& shader = renderGraph.shaders[hitGroup.anyHit.shaderIndex];
                                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                    "        case " << usedRTHitGroupIndex << ": " << shader.entryPoint << "(payload, intersectionInfo, anyHitResult, Ray, minT, primitiveIndex); break;\n"
                                    ;
                            }

                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "    }\n"
                                "}\n"
                                "\n"
                                ;
                        }

                        // RunMissShader() - Call miss shader based on MissShaderIndex
                        {
                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "void RunMissShader(inout Payload payload, in uint missShaderIndex)\n"
                                "{\n"
                                "    switch (missShaderIndex)\n"
                                "    {\n"
                                ;

                            for (int usedRTMissIndex = 0; usedRTMissIndex < (int)options.m_shader.Used_RTMissIndex.size(); ++usedRTMissIndex)
                            {
                                const std::string& missShaderName = options.m_shader.Used_RTMissIndex[usedRTMissIndex];
                                int missShaderIndex = GetShaderIndex(renderGraph, missShaderName.c_str());
                                Assert(missShaderIndex != -1, "Could not find miss shader %s", missShaderName.c_str());

                                const Shader& shader = renderGraph.shaders[missShaderIndex];
                                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                    "        case " << usedRTMissIndex << ": " << shader.entryPoint << "(payload); break;\n"
                                    ;
                            }

                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "    }\n"
                                "}\n"
                                "\n"
                                ;
                        }

                        // RunClosestHitShader() - Call closest hit based on hitGroupIndex
                        {
                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "void RunClosestHitShader(inout Payload payload, inout GIGI_INTERSECTIONINFOOBJECTTYPE intersectionInfo, in uint hitGroupIndex, in uint _PrimitiveIndex)\n"
                                "{\n"
                                "    switch (hitGroupIndex)\n"
                                "    {\n"
                                ;

                            for (int usedRTHitGroupIndex = 0; usedRTHitGroupIndex < (int)options.m_shader.Used_RTHitGroupIndex.size(); ++usedRTHitGroupIndex)
                            {
                                const std::string& hitGroupName = options.m_shader.Used_RTHitGroupIndex[usedRTHitGroupIndex];
                                int hitGroupIndex = GetHitGroupIndex(renderGraph, hitGroupName.c_str());
                                Assert(hitGroupIndex != -1, "Could not find hit group %s", hitGroupName.c_str());
                                const RTHitGroup& hitGroup = renderGraph.hitGroups[hitGroupIndex];

                                if (hitGroup.closestHit.shaderIndex == -1)
                                    continue;

                                const Shader& shader = renderGraph.shaders[hitGroup.closestHit.shaderIndex];
                                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                    "        case " << usedRTHitGroupIndex << ": " << shader.entryPoint << "(payload, intersectionInfo); break;\n"
                                    ;
                            }

                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "    }\n"
                                "}\n"
                                "\n"
                                ;
                        }
                    }
                }

                // Some RGS emulation.
                if (options.m_shader.type == ShaderType::RTRayGen)
                {
                    shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                        "#define DispatchRaysIndex() DTid\n"
                        "#define IgnoreHit() { _anyHitResult = AnyHitResult_IgnoreHit; return; }\n"
                        "#define AcceptHitAndEndSearch() { _anyHitResult = AnyHitResult_AcceptHitAndEndSearch; return; }\n"
                        "#define ObjectRayOrigin() _ray.Origin\n"
                        "#define ObjectRayDirection() _ray.Direction\n"
                        "#define RayTCurrent() _RayTCurrent\n"
                        "#define RayTMin() _ray.TMin\n"
                        "#define PrimitiveIndex() _PrimitiveIndex\n"
                        "#define ReportHit() _ray.Direction\n"
                        ;
                }
            }
        ;

        options.m_rayTraceFnName = "RayVsScene";

        // shader includes are in a temp directory because they have some processing done to them
        std::string shaderIncludeDirectory = std::filesystem::temp_directory_path().string();
        {
            shaderIncludeDirectory += "Gigi\\";
            char pid[256];
            sprintf_s(pid, "%i", _getpid());
            shaderIncludeDirectory += pid;
            shaderIncludeDirectory += "\\shaders\\";
        }

        std::string includeDirectory = (std::filesystem::path(shaderIncludeDirectory) / shader.destFileName).replace_filename("").string();
        std::string includeDirectoryRoot = std::filesystem::path(shaderIncludeDirectory).string();
        std::string shaderCode;

        // Don't emit line numbers, try not to mangle the names, and be explicit about paths for errors.
        Shader shaderCopy = shader;
        shaderCopy.slangOptions.lineDirectives = false;
        shaderCopy.slangOptions.noNameMangling = true;
        shaderCopy.slangOptions.verbosePaths = true;

        // For the BVH version of the shader, the RTScene buffer needs to be uint
        if (isBVHShader)
        {
            for (ShaderResource& resource : shaderCopy.resources)
            {
                if (resource.originalAccess == ShaderResourceAccessType::RTScene)
                {
                    resource.buffer.typeStruct.name = "";
                    resource.buffer.typeStruct.structIndex = -1;
                    resource.buffer.type = DataFieldType::Uint;
                }
            }
        }

        options.m_shader = shaderCopy;
        std::vector<std::string> includeDirectories;
        includeDirectories.push_back(includeDirectory);
        includeDirectories.push_back(includeDirectoryRoot);
        ProcessShaderToMemory_HLSL(shaderCopy, shaderCopy.entryPoint.c_str(), ShaderLanguage::WGSL, stringReplacementMap, renderGraph, options, includeDirectories, shaderCode);

        // Fix up UAV declarations
        FixupShaderUAVDeclarations(renderGraph, shader, shaderCode);

        std::string shaderType;
        std::string shaderTypeDescription;
        if (usesRT)
        {
            shaderType = (isBVHShader) ? "_BVH" : "_NonBVH";
            shaderTypeDescription = (isBVHShader) ? "BVH Variation of " : "Non BVH Variation of ";
        }

        stringReplacementMap["/*$(Shaders)*/"]
            << "// Shader code for " << shaderTypeDescription << EnumToString(shader.type) << " shader \"" << shader.name << "\", node \"" << nodeName << "\"\n"
            << "static ShaderCode_" << nodeName << "_" << shader.name << shaderType << " = `\n" << shaderCode << "`;\n\n";
    }
}

static void DoSlangDataFixups(RenderGraph& renderGraph)
{
    // Slang doesn't renumber registers, so it generates code where u0 and b0 overlap, both using register 0.
    // To get around this problem, for every shader that is not wgsl, we auto number the registers so that they have unique numbers.
    // Vertex shaders and pixel shaders also must have unique register numbers when combined together, so we add 100 to pixel shaders.
    for (Shader& shader : renderGraph.shaders)
    {
        if (shader.language == ShaderLanguage::WGSL)
            continue;

        int offset = (shader.type == ShaderType::Pixel) ? 100 : 0;

        for (size_t index = 0; index < shader.resources.size(); ++index)
            shader.resources[index].registerIndex = int(index) + offset;

        for (size_t index = 0; index < shader.samplers.size(); ++index)
            shader.samplers[index].registerIndex = int(index) + offset + int(shader.resources.size());
    }

    // In 2025.3.2, slang outputs texture / texture_storage for Buffers, but does regular buffer storage for structured buffers.
    // Make sure all resources use structured buffers.
    for (Shader& shader : renderGraph.shaders)
    {
        for (ShaderResource& resource : shader.resources)
        {
            resource.buffer.PODAsStructuredBuffer = true;
        }
    }
}

void RunBackend_WebGPU(GigiBuildFlavor buildFlavor, RenderGraph& renderGraph, GGUserFileLatest& ggUserFile)
{
    for (const Shader& shader : renderGraph.shaders)
    {
        if (shader.type == ShaderType::Amplification)
        {
            Assert(false, "Amplification shaders are not supported by WebGPU. (shader = \"%s\")", shader.name.c_str());
        }
        else if (shader.type == ShaderType::Mesh)
        {
            Assert(false, "Mesh shaders are not supported by WebGPU. (shader = \"%s\")", shader.name.c_str());
        }
    }

    for (const RenderGraphNode& nodeBase : renderGraph.nodes)
    {
        if (nodeBase._index != RenderGraphNode::c_index_actionDrawCall)
            continue;

        const RenderGraphNode_Action_DrawCall& node = nodeBase.actionDrawCall;
        const RenderGraphNode_Action_DrawCall defaultNode;

        if (node.shadingRate != defaultNode.shadingRate ||
            node.shadingRateCombiner1 != defaultNode.shadingRateCombiner1 ||
            node.shadingRateCombiner2 != defaultNode.shadingRateCombiner2 ||
            node.shadingRateImage.nodeIndex != -1)
        {
            Assert(false, "variable rate shading is not supported by WebGPU. (node = \"%s\")", node.name.c_str());
        }
    }

    // Do any data fixups needed for slang
    DoSlangDataFixups(renderGraph);

    const char* outFolder = renderGraph.outputDirectory.c_str();

    if(renderGraph.generateGraphVizFlag){
        // make the graphviz
        std::string fullOutFolder = std::string(outFolder) +"/GraphViz/";
        MakeRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
        MakeFlattenedRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
        MakeSummaryRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
    }

    // gather the templates for the build flavor chosen.
    std::unordered_map<std::string, std::string> files;
    std::vector<InternalTemplateFile> internalTemplateFiles;
    {
        // Module files are always present
        std::vector<InternalTemplateFile> templateFiles = ProcessTemplateFolder(renderGraph, files, outFolder, "./GigiCompilerLib/Backends/WebGPU/templates/Module/");
        internalTemplateFiles.insert(internalTemplateFiles.end(), templateFiles.begin(), templateFiles.end());

        // Other flavors add application code that calls into the module code
        switch (buildFlavor)
        {
            case GigiBuildFlavor::WebGPU_Module: break;
            case GigiBuildFlavor::WebGPU_Application:
            {
                std::vector<InternalTemplateFile> templateFiles = ProcessTemplateFolder(renderGraph, files, outFolder, "./GigiCompilerLib/Backends/WebGPU/templates/Application/");
                internalTemplateFiles.insert(internalTemplateFiles.end(), templateFiles.begin(), templateFiles.end());
                break;
            }
            default:
            {
                Assert(false, "Unhandled build flavor: %s", EnumToString(buildFlavor));
                return;
            }
        }
    }

    // Get a temporary directory
    std::string tempDirectory = std::filesystem::temp_directory_path().string();
    {
        tempDirectory += "Gigi\\";
        char pid[256];
        sprintf_s(pid, "%i", _getpid());
        tempDirectory += pid;
        tempDirectory += "\\";
    }

    // Make string replacement only for things the shaders will want.
    BackendWebGPU::g_processingShaders = true;
    std::unordered_map<std::string, std::ostringstream> stringReplacementMap = MakeStringReplacement<BackendWebGPU>(renderGraph, ggUserFile);
    BackendWebGPU::g_processingShaders = false;

    // Copy shader header files into a temp folder and give them treatment.
    for (const FileCopy& fileCopy : renderGraph.fileCopies)
    {
        if (fileCopy.type != FileCopyType::Shader)
            continue;

        // load the file into memory
        std::vector<unsigned char> data;
        if (!LoadFile(renderGraph.baseDirectory + fileCopy.fileName, data))
        {
            Assert(false, "Could not read file %s", fileCopy.fileName.c_str());
        }
        data.push_back(0);

        std::string fileContents = (char*)data.data();

        // Make the full file name
        char fullFileName[4096];
        std::string destFileName;
        if (fileCopy.destFileName.empty())
            destFileName = fileCopy.fileName;
        else
            destFileName = fileCopy.destFileName;
        sprintf_s(fullFileName, "%s/shaders/%s", tempDirectory.c_str(), destFileName.c_str());

        // Process the shader to be ok for slang
        ProcessShader_ReplaceSingleCharacterConstants(fileContents);

        // Do string replacement if we should
        if (!fileCopy.binary)
            ProcessStringReplacement(fileContents, stringReplacementMap, renderGraph);

        WriteFileIfDifferent(fullFileName, fileContents);
    }

    // Write any InternalShader files to the shaders directory too
    for (const InternalTemplateFile& internalTemplateFile : internalTemplateFiles)
    {
        if (internalTemplateFile.type != BackendTemplateFileType::InternalShader)
            continue;

        // load the file into memory
        std::vector<unsigned char> fileContents;
        if (!LoadFile(internalTemplateFile.absoluteFileName, fileContents))
        {
            Assert(false, "Could not read file %s", internalTemplateFile.absoluteFileName.c_str());
        }

        char fullFileName[4096];
        sprintf_s(fullFileName, "%s/shaders/%s", tempDirectory.c_str(), internalTemplateFile.relativeFileName.c_str());

        WriteFileIfDifferent(fullFileName, fileContents);
    }

    // Make the string replacement
    stringReplacementMap = MakeStringReplacement<BackendWebGPU>(renderGraph, ggUserFile);

    // Process shaders because they end up in the string replacement map
    for (const RenderGraphNode& nodeBase : renderGraph.nodes)
    {
        switch (nodeBase._index)
        {
            case RenderGraphNode::c_index_actionComputeShader:
            {
                const RenderGraphNode_Action_ComputeShader& node = nodeBase.actionComputeShader;
                if (node.shader.shader->copyFile)
                    ProcessShaderFile(node.name, *node.shader.shader, stringReplacementMap, outFolder, renderGraph);
                break;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                const RenderGraphNode_Action_RayShader& node = nodeBase.actionRayShader;
                ProcessShaderFile(node.name, *node.shader.shader, stringReplacementMap, outFolder, renderGraph);

                for (const RTHitGroup& hitGroup : renderGraph.hitGroups)
                {
                    if (hitGroup.closestHit.shader && hitGroup.closestHit.shader->copyFile)
                        ProcessShaderFile(node.name, *hitGroup.closestHit.shader, stringReplacementMap, outFolder, renderGraph);

                    if (hitGroup.anyHit.shader && hitGroup.anyHit.shader->copyFile)
                        ProcessShaderFile(node.name, *hitGroup.anyHit.shader, stringReplacementMap, outFolder, renderGraph);

                    if (hitGroup.intersection.shader && hitGroup.intersection.shader->copyFile)
                        ProcessShaderFile(node.name, *hitGroup.intersection.shader, stringReplacementMap, outFolder, renderGraph);
                }

                for (const Shader& shader : renderGraph.shaders)
                {
                    if (shader.type == ShaderType::RTMiss && shader.copyFile)
                        ProcessShaderFile(node.name, shader, stringReplacementMap, outFolder, renderGraph);
                }

                break;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                const RenderGraphNode_Action_DrawCall& node = nodeBase.actionDrawCall;

                if(node.amplificationShader.shader && node.amplificationShader.shader->copyFile)
                    ProcessShaderFile(node.name, *node.amplificationShader.shader, stringReplacementMap, outFolder, renderGraph);

                if (node.meshShader.shader && node.meshShader.shader->copyFile)
                    ProcessShaderFile(node.name, *node.meshShader.shader, stringReplacementMap, outFolder, renderGraph);

                if (node.vertexShader.shader && node.vertexShader.shader->copyFile)
                    ProcessShaderFile(node.name, *node.vertexShader.shader, stringReplacementMap, outFolder, renderGraph);

                if (node.pixelShader.shader && node.pixelShader.shader->copyFile)
                    ProcessShaderFile(node.name, *node.pixelShader.shader, stringReplacementMap, outFolder, renderGraph);

                break;
            }

            // Nothing to do for nodes that don't use shaders
            case RenderGraphNode::c_index_actionCopyResource:
            case RenderGraphNode::c_index_actionSubGraph:
            case RenderGraphNode::c_index_actionBarrier:
            {
                break;
            }

            // Nothing to do for resource nodes
            case RenderGraphNode::c_index_resourceBuffer:
            case RenderGraphNode::c_index_resourceShaderConstants:
            case RenderGraphNode::c_index_resourceTexture:
            {
                break;
            }

            default:
            {
                Assert(false, "Unhandled node type");
                break;
            }
        }
    }

    // Make the output files
    MakeFiles(files, renderGraph, stringReplacementMap);

    // copy any file copies that should happen
    for (const FileCopy& fileCopy : renderGraph.fileCopies)
    {
        // no shader file copies needed. slang inlines the headers.
        if (fileCopy.type == FileCopyType::Shader)
            continue;

        // load the file into memory
        std::vector<unsigned char> data;
        if (!LoadFile(renderGraph.baseDirectory + fileCopy.fileName, data))
        {
            Assert(false, "Could not read file %s", fileCopy.fileName.c_str());
        }

        // get the folder to copy to
        const char* destFolder = "";
        switch (fileCopy.type)
        {
            case FileCopyType::Private:
            {
                destFolder = "private/";
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
                Assert(false, "Unhandled file copy type");
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

static bool TextureFormatAllowedForReadWriteStorage(TextureFormat format)
{
    // reflecting information from https://gpuweb.github.io/gpuweb/#plain-color-formats
    switch (format)
    {
        case TextureFormat::R32_Float:
        case TextureFormat::R32_Uint:
        // case TextureFormat::R32_Sint: // r32 sint is supported, if we add it to Gigi
        {
            return true;
        }
    }

    return false;
}

void PostLoad_WebGPU(RenderGraph& renderGraph)
{
    // WebGPU has very limited texture UAV (storage texture) capabilities in read-write mode: only r32float, r32sint and r32uint are allowed.
    // If you use those formats as UAVs, and Gigi can know that at Gigi compile time, they will be allowed without modification.
    // Otherwise, the source texture will be copied as a read only copy, and the original texture will be write only.
    // A pin will be added to the shader so that there are two pins, one for read only and the other for write only.
    // Use the /*$(RWTextureR:resource:location)*/ tag for reads and /*$(RWTextureW:resource:location)*/ for writes, to allow them to be
    // renamed in the shader as needed.
    //
    // This means that in general, shared writes won't be able to affect shader reads within the same dispatch.
    // The read only and write only formats are also more limited than other platforms, so in webgpu you are more likely to get an
    // error regarding this and have to change your source technique if you want it to also work on webgpu.
    //
    // This can be revisited in the future as WebGPU matures!

    // Gather up the node pins that are UAV textures with possibly unsupported texture formats
    struct NodePin
    {
        std::string node;
        std::string pin;
    };
    std::vector<NodePin> unsupportedTextureUAVs;
    for (RenderGraphNode& nodeBase : renderGraph.nodes)
    {
        if (GetNodeIsResourceNode(nodeBase))
            continue;

        int pinIndex = -1;
        std::vector<FrontEndNodesNoCaching::PinInfo> pinInfos = FrontEndNodesNoCaching::GetPinInfo(renderGraph, nodeBase);
        for (FrontEndNodesNoCaching::PinInfo& pinInfo : pinInfos)
        {
            pinIndex++;

            if (pinInfo.access != ShaderResourceAccessType::UAV)
                continue;

            int resourceNodeIndex = FrontEndNodesNoCaching::GetRootNodeIndex(renderGraph, GetNodeName(nodeBase), pinInfo.srcPin);
            if (resourceNodeIndex == -1)
                continue;

            const RenderGraphNode& resourceNodeBase = renderGraph.nodes[resourceNodeIndex];
            if (resourceNodeBase._index != RenderGraphNode::c_index_resourceTexture)
                continue;

            // TODO: This commented out code doesn't make a copy if the texture is known to be a supported format. This can be problematic
            // because the same shader may be used multiple times, and sometimes want a separate pin, other times not.
            // We would have to rework this code so that if ANY uses of a shader wanted a copy, they all copy a copy, else avoid the copy.
            //TextureFormat compileTimeTextureFormat = FrontEndNodesNoCaching::GetCompileTimeTextureFormat(renderGraph, resourceNodeBase);
            //if (TextureFormatAllowedForReadWriteStorage(compileTimeTextureFormat))
                //continue;

            // If we got here, it means we are accessing a texture with UAV access, and we can't be sure that it's an ok format for read/write, so add it to the list
            unsupportedTextureUAVs.push_back({ GetNodeName(nodeBase), pinInfo.srcPin });
        }
    }

    #if SAVE_RWSPLIT_GRAPHS()
    int versionIndex = 0;
    #endif

    // split each access into a read only and write only access
    for (const NodePin& nodePin : unsupportedTextureUAVs)
    {
        #if SAVE_RWSPLIT_GRAPHS()
        char fileName[1024];
        sprintf_s(fileName, "_RWSplit_v%i.gg", versionIndex);
        WriteToJSONFile(renderGraph, fileName);
        versionIndex++;
        #endif

        auto theNodeIndex = FrontEndNodesNoCaching::GetNodeIndexByName(renderGraph, nodePin.node.c_str());

        // Get node nvolved
        RenderGraphNode& nodeBase = renderGraph.nodes[FrontEndNodesNoCaching::GetNodeIndexByName(renderGraph, nodePin.node.c_str())];

        // Get pin involved
        std::vector<FrontEndNodesNoCaching::PinInfo> pinInfos = FrontEndNodesNoCaching::GetPinInfo(renderGraph, nodeBase);
        int pinIndex = FrontEndNodesNoCaching::GetPinIndexByName(pinInfos, nodePin.pin.c_str());
        FrontEndNodesNoCaching::PinInfo& pinInfo = pinInfos[pinIndex];

        // Get the texture node plugged into this pin
        int resourceNodeIndex = FrontEndNodesNoCaching::GetRootNodeIndex(renderGraph, GetNodeName(nodeBase), pinInfo.srcPin);
        const RenderGraphNode& textureNodeBase = renderGraph.nodes[resourceNodeIndex];

        // Make a new texture node that is a clone of the other one.
        // Give it a unique name and make sure it's internal and transient though.
        // Also explicitly set the size and format to the other node since it might be imported.
        RenderGraphNode newTextureNodeBase = textureNodeBase;
        RenderGraphNode_Resource_Texture& newTextureNode = newTextureNodeBase.resourceTexture;
        {
            char baseName[1024];
            sprintf_s(baseName, "%s_%s_ReadOnly", GetNodeName(nodeBase).c_str(), pinInfo.srcPin.c_str());
            newTextureNode.name = FrontEndNodesNoCaching::GetUniqueNodeName(renderGraph, baseName);
            newTextureNode.visibility = ResourceVisibility::Internal;
            newTextureNode.transient = true;
            newTextureNode.format.node.name = textureNodeBase.resourceTexture.name;

            newTextureNode.size = TextureSizeDesc();
            newTextureNode.size.node.name = textureNodeBase.resourceTexture.name;

            // Make a meaningful comment
            newTextureNode.comment = "This is a read only texture copy to help get around read-write storage texture format limitations.";
        }

        // Make the texture copy node
        RenderGraphNode copyTextureNodeBase;
        RenderGraphNode_Action_CopyResource& copyTextureNode = copyTextureNodeBase.actionCopyResource;
        {
            copyTextureNodeBase._index = RenderGraphNode::c_index_actionCopyResource;

            copyTextureNode.source.node = *pinInfo.dstNode;
            copyTextureNode.source.pin = *pinInfo.dstPin;

            copyTextureNode.dest.node = newTextureNode.name;
            copyTextureNode.dest.pin = "resource";

            // make a unique name
            {
                char baseName[1024];
                sprintf_s(baseName, "Copy_%s_%s", GetNodeName(nodeBase).c_str(), pinInfo.srcPin.c_str());
                copyTextureNode.name = FrontEndNodesNoCaching::GetUniqueNodeName(renderGraph, baseName);
            }
        }

        // Update the current node pin to be connected to the src pin of the copy texture to buffer node.
        {
            *pinInfo.dstNode = copyTextureNode.name;
            *pinInfo.dstPin = "source";
        }

        WebGPU_RWTextureSplit rwTextureSplit;
        rwTextureSplit.nodeName = GetNodeName(nodeBase);
        rwTextureSplit.pinName = pinInfo.srcPin;

        auto HandleUAVSplit = [&renderGraph, &pinInfo, &rwTextureSplit, &copyTextureNode, pinIndex](auto& node, const std::string& shaderName) -> bool
        {
            if (shaderName.empty())
                return false;

            int shaderIndex = -1;
            for (int i = 0; i < renderGraph.shaders.size(); ++i)
            {
                if (renderGraph.shaders[i].name == shaderName)
                {
                    shaderIndex = i;
                    break;
                }
            }
            Assert(shaderIndex != -1, "Could not find shader");

            const Shader& shader = renderGraph.shaders[shaderIndex];

            // If this shader doesn't contain this pin, return failure
            bool found = false;
            for (const ShaderResource& resource : shader.resources)
            {
                if (resource.name == pinInfo.srcPin)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;

            rwTextureSplit.shaderName = shader.name.c_str();

            // Get a unique name for the read only version of the pin if we haven't already.
            // If we have, use the pin we already made.
            {
                // See if we've already made a pin
                for (const WebGPU_RWTextureSplit& split : renderGraph.backendData.webGPU.RWTextureSplits)
                {
                    if (split.shaderName == rwTextureSplit.shaderName && split.pinName == rwTextureSplit.pinName)
                    {
                        rwTextureSplit.pinNameR = split.pinNameR;
                        break;
                    }
                }

                // If not, make one
                if (rwTextureSplit.pinNameR.empty())
                {
                    int shaderIndex = FrontEndNodesNoCaching::GetShaderIndexByName(renderGraph, ShaderType::Count, rwTextureSplit.shaderName.c_str());
                    Assert(shaderIndex >= 0, "Could not find shader \"%s\"", rwTextureSplit.shaderName.c_str());
                    Shader& shader = renderGraph.shaders[shaderIndex];

                    rwTextureSplit.pinNameR = FrontEndNodesNoCaching::GetUniqueShaderResourceName(shader.resources, (pinInfo.srcPin + "ReadOnly").c_str());

                    const ShaderResource* srcResource = FrontEndNodesNoCaching::GetShaderResourceByName(renderGraph, ShaderType::Count, rwTextureSplit.shaderName.c_str(), pinInfo.srcPin.c_str());
                    Assert(srcResource != nullptr, "Could not find source shader resource to copy");

                    // Add a new pin for the read only access
                    ShaderResource newResource = *srcResource;
                    newResource.name = rwTextureSplit.pinNameR;
                    shader.resources.push_back(newResource);
                }
            }

            // connect the copy node dest pin to the read only pin
            NodePinConnection newConnection;
            newConnection.srcPin = rwTextureSplit.pinNameR;
            newConnection.dstNode = copyTextureNode.name;
            newConnection.dstPin = "dest";
            node.connections.push_back(newConnection);

            // Make sure it's dealing with the same mip
            node.linkProperties.resize(node.connections.size());
            node.linkProperties.rbegin()->UAVMipIndex = node.linkProperties[pinIndex].UAVMipIndex;

            return true;
        };

        // We need to add a new pin that is the same as this one. This pin should be write only, and a new pin should be read only.
        bool UAVSplitHandled = false;
        {
            switch (nodeBase._index)
            {
                case RenderGraphNode::c_index_actionComputeShader:
                {
                    RenderGraphNode_Action_ComputeShader& node = nodeBase.actionComputeShader;
                    UAVSplitHandled = HandleUAVSplit(node, node.shader.name);
                    break;
                }
                case RenderGraphNode::c_index_actionRayShader:
                {
                    RenderGraphNode_Action_RayShader& node = nodeBase.actionRayShader;
                    UAVSplitHandled = HandleUAVSplit(node, node.shader.name);
                    break;
                }
                case RenderGraphNode::c_index_actionDrawCall:
                {
                    RenderGraphNode_Action_DrawCall& node = nodeBase.actionDrawCall;
                    UAVSplitHandled =
                        HandleUAVSplit(node, node.vertexShader.name) ||
                        HandleUAVSplit(node, node.pixelShader.name) ||
                        HandleUAVSplit(node, node.meshShader.name) ||
                        HandleUAVSplit(node, node.amplificationShader.name);
                    break;
                }
                default: 
                {
                    Assert(false, "Unhandled node type in " __FUNCTION__);
                    break;
                }
            }
        }

        Assert(UAVSplitHandled, "Cound not find UAV node to split into R and W in " __FUNCTION__);

        renderGraph.backendData.webGPU.RWTextureSplits.push_back(rwTextureSplit);

        // Move the nodes to be at the bottom of the graph
        float nodesAABB[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // min xy, max xy.
        {
            bool firstSample = true;
            for (const RenderGraphNode& node : renderGraph.nodes)
            {
                ExecuteOnNode(node,
                    [&firstSample, &nodesAABB](auto& node)
                    {
                        if (firstSample)
                        {
                            nodesAABB[0] = nodesAABB[2] = node.editorPos[0];
                            nodesAABB[1] = nodesAABB[3] = node.editorPos[1];
                            firstSample = false;
                        }
                        else
                        {
                            nodesAABB[0] = std::min(nodesAABB[0], node.editorPos[0]);
                            nodesAABB[1] = std::min(nodesAABB[1], node.editorPos[1]);
                            nodesAABB[2] = std::max(nodesAABB[2], node.editorPos[0]);
                            nodesAABB[3] = std::max(nodesAABB[3], node.editorPos[1]);
                        }
                    }
                );
            }
        }
        newTextureNodeBase.resourceTexture.editorPos[1] = nodesAABB[3] + 100.0f;
        copyTextureNodeBase.actionCopyResource.editorPos[0] = 750.0f;
        copyTextureNodeBase.actionCopyResource.editorPos[1] = nodesAABB[3] + 100.0f;

        // Add the new nodes to the graph
        renderGraph.nodes.push_back(newTextureNodeBase);
        renderGraph.nodes.push_back(copyTextureNodeBase);
    }

    #if SAVE_RWSPLIT_GRAPHS()
    char fileName[1024];
    sprintf_s(fileName, "_RWSplit_v%i.gg", versionIndex);
    WriteToJSONFile(renderGraph, fileName);
    versionIndex++;
    #endif

    // If we had to split any texture UAVs, report this information
    if (!renderGraph.backendData.webGPU.RWTextureSplits.empty())
    {
        std::ostringstream warning;
        warning <<
            "WebGPU is limited in what texture formats supported read/write access.\n"
            "Supported formats: r32float, r32uint, r32sint.\n"
            "To help get around this problem, the following <node:pin> pairs have been split into a read only\n"
            "pin and write only pin:\n"
            "\n"
            ;

        for (const WebGPU_RWTextureSplit& texture : renderGraph.backendData.webGPU.RWTextureSplits)
        {
            warning << "    <" << texture.nodeName << ":" << texture.pinName << "> (shader \"" << texture.shaderName << "\")\n";
        }

        warning <<
            "\n"
            "An extra texture resource was made for each of these as well as a texture copy. If you use the\n"
            "Gigi shader tokens /*$(RWTextureR)*/ and /*$(RWTextureW)*/, the correct resources will be accessed\n"
            "automatically. Note that this means writes cannot affect reads during the same dispatch which might\n"
            "change functionality.\n"
            ;

        ShowWarningMessage("%s", warning.str().c_str());
    }

    // There is no support for ray gen shaders. We emulate them with compute shaders.
    // So, all RTScene accesses should be SRV accesses (read only buffers).
    for (Shader& shader : renderGraph.shaders)
    {
        for (ShaderResource& resource : shader.resources)
        {
            if (resource.access == ShaderResourceAccessType::RTScene)
            {
                resource.originalAccess = resource.access;
                resource.access = ShaderResourceAccessType::SRV;

                // Also make sure that the buffer has a type set
                Assert(resource.buffer.type != DataFieldType::Count || !resource.buffer.typeStruct.name.empty(), "WebGPU requires a buffer type given for RTScene buffer resource \"%s\" in shader \"%s\"", shader.name.c_str(), resource.name.c_str());
            }
        }
    }
}
