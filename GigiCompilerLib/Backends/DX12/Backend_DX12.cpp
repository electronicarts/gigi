///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Schemas/Types.h"
#include "GigiCompilerLib/Backends/Shared.h"
#include "GigiCompilerLib/Backends/GraphViz.h"
#include "GigiCompilerLib/GigiBuildFlavor.h"
#include "GigiCompilerLib/ProcessSlang.h"

#include <unordered_set>

struct BackendDX12 : public BackendBase
{
    static const std::string GetResourceNodePathInContext(ResourceVisibility visibility)
    {
        switch (visibility)
        {
            case ResourceVisibility::Imported: return "m_input.";
            case ResourceVisibility::Internal: return "m_internal.";
            case ResourceVisibility::Exported: return "m_output.";
        }
        Assert(false, "Unhandled ResourceVisibility: %i", (int)visibility);
        return __FUNCTION__ " unknown ResourceVisibility";
    }

    // Used by buffers
    // When this isn't good enough, have a look at DataFieldTypeInfoStructDX12 and perhaps try to combine the usage cases
    static const std::string DataFieldTypeToDXGIFormat(DataFieldType type)
    {
        switch (type)
        {
            case DataFieldType::Float: return "DXGI_FORMAT_R32_FLOAT";
            case DataFieldType::Float3: return "DXGI_FORMAT_R32G32B32_FLOAT";
            case DataFieldType::Uint: return "DXGI_FORMAT_R32_UINT";
            case DataFieldType::Uint_16: return "DXGI_FORMAT_R16_UINT";
            default:
            {
                Assert(false, "Unhandled DataFieldType: %i", (int)type);
                return __FUNCTION__ " unknown data field type";
            }
        }
    }

    static std::string SamplerFilterToD3D12_FILTER(SamplerFilter filter)
    {
        switch (filter)
        {
            case SamplerFilter::MinMagMipPoint: return "D3D12_FILTER_MIN_MAG_MIP_POINT";
            case SamplerFilter::MinMagLinear_MipPoint: return "D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT";
            case SamplerFilter::MinMagMipLinear: return "D3D12_FILTER_MIN_MAG_MIP_LINEAR";
            default:
            {
                Assert(false, "Unhandled SamplerFilter: %i", (int)filter);
                return __FUNCTION__ " unhandled SamplerFilter";
            }
        }
    }

    static std::string DrawCullModeToD3D12_CULL_MODE(DrawCullMode drawCullMode)
    {
        switch (drawCullMode)
        {
            case DrawCullMode::None: return "D3D12_CULL_MODE_NONE";
            case DrawCullMode::Front: return "D3D12_CULL_MODE_FRONT";
            case DrawCullMode::Back: return "D3D12_CULL_MODE_BACK";
            default:
            {
                Assert(false, "Unhandled DrawCullMode: %i (%s)", (int)drawCullMode, EnumToString(drawCullMode));
                return __FUNCTION__ " unhandled DrawCullMode";
            }
        }
    }

    static std::string DrawBlendModeToD3D12_BLEND(DrawBlendMode mode)
    {
        switch (mode)
        {
            case DrawBlendMode::Zero: return "D3D12_BLEND_ZERO";
            case DrawBlendMode::One: return "D3D12_BLEND_ONE";
            case DrawBlendMode::SrcColor: return "D3D12_BLEND_SRC_COLOR";
            case DrawBlendMode::InvSrcColor: return "D3D12_BLEND_INV_SRC_COLOR";
            case DrawBlendMode::SrcAlpha: return "D3D12_BLEND_SRC_ALPHA";
            case DrawBlendMode::InvSrcAlpha: return "D3D12_BLEND_INV_SRC_ALPHA";
            case DrawBlendMode::DestAlpha: return "D3D12_BLEND_DEST_ALPHA";
            case DrawBlendMode::InvDestAlpha: return "D3D12_BLEND_INV_DEST_ALPHA";
            case DrawBlendMode::DestColor: return "D3D12_BLEND_DEST_COLOR";
            case DrawBlendMode::InvDestColor: return "D3D12_BLEND_INV_DEST_COLOR";
            default:
            {
                Assert(false, "Unhandled DrawBlendMode: %i (%s)", (int)mode, EnumToString(mode));
                return __FUNCTION__ " unhandled DrawBlendMode";
            }
        }
    }

    static std::string TextureDimensionTypeToD3D12_RTV_DIMENSION(TextureDimensionType textureDimensionType)
    {
        switch (textureDimensionType)
        {
            case TextureDimensionType::Texture2D: return "D3D12_RTV_DIMENSION_TEXTURE2D";
            case TextureDimensionType::Texture2DArray: return "D3D12_RTV_DIMENSION_TEXTURE2DARRAY";
            case TextureDimensionType::Texture3D: return "D3D12_RTV_DIMENSION_TEXTURE3D";
            case TextureDimensionType::TextureCube: return "D3D12_RTV_DIMENSION_TEXTURE2DARRAY";
            default:
            {
                Assert(false, "Unhandled TextureDimensionType: %i (%s)", (int)textureDimensionType, EnumToString(textureDimensionType));
                return __FUNCTION__ " unhandled TextureDimensionType";
            }
        }
    }

    static std::string TextureDimensionTypeToD3D12_DSV_DIMENSION(TextureDimensionType textureDimensionType)
    {
        switch (textureDimensionType)
        {
            case TextureDimensionType::Texture2D: return "D3D12_DSV_DIMENSION_TEXTURE2D";
            case TextureDimensionType::Texture2DArray: return "D3D12_DSV_DIMENSION_TEXTURE2DARRAY";
            case TextureDimensionType::TextureCube: return "D3D12_DSV_DIMENSION_TEXTURE2DARRAY";
            default:
            {
                Assert(false, "Unhandled TextureDimensionType: %i (%s)", (int)textureDimensionType, EnumToString(textureDimensionType));
                return __FUNCTION__ " unhandled TextureDimensionType";
            }
        }
    }

    static std::string GeometryTypeToD3D12_PRIMITIVE_TOPOLOGY(GeometryType geometryType)
    {
        switch (geometryType)
        {
            case GeometryType::TriangleList: return "D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST";
            case GeometryType::LineList: return "D3D_PRIMITIVE_TOPOLOGY_LINELIST";
            case GeometryType::PointList: return "D3D_PRIMITIVE_TOPOLOGY_POINTLIST";
            default:
            {
                Assert(false, "Unhandled GeometryType: %i (%s)", (int)geometryType, EnumToString(geometryType));
                return __FUNCTION__ " unhandled GeometryType";
            }
        }
    }

    static std::string GeometryTypeToD3D12_PRIMITIVE_TOPOLOGY_TYPE(GeometryType geometryType)
    {
        switch (geometryType)
	    {
		    case GeometryType::TriangleList: return "D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE";
		    case GeometryType::LineList: return "D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE";
		    case GeometryType::PointList: return "D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT";
		    default:
		    {
                Assert(false, "Unhandled GeometryType: %i (%s)", (int)geometryType, EnumToString(geometryType));
                return __FUNCTION__ " unhandled GeometryType";
		    }
	    }
    }

    static std::string DepthTestFunctionToD3D12_COMPARISON_FUNC(DepthTestFunction function)
    {
        switch (function)
        {
            case DepthTestFunction::Never: return "D3D12_COMPARISON_FUNC_NEVER";
            case DepthTestFunction::Less: return "D3D12_COMPARISON_FUNC_LESS";
            case DepthTestFunction::Equal: return "D3D12_COMPARISON_FUNC_EQUAL";
            case DepthTestFunction::LessEqual: return "D3D12_COMPARISON_FUNC_LESS_EQUAL";
            case DepthTestFunction::Greater: return "D3D12_COMPARISON_FUNC_GREATER";
            case DepthTestFunction::NotEqual: return "D3D12_COMPARISON_FUNC_NOT_EQUAL";
            case DepthTestFunction::GreaterEqual: return "D3D12_COMPARISON_FUNC_GREATER_EQUAL";
            case DepthTestFunction::Always: return "D3D12_COMPARISON_FUNC_ALWAYS";
            default:
            {
                Assert(false, "Unhandled DepthTestFunction: %i (%s)", (int)function, EnumToString(function));
                return __FUNCTION__ " unhandled DepthTestFunction";
            }
        }
    }

    static std::string StencilOpToD3D12_STENCIL_OP(StencilOp op)
    {
        switch (op)
        {
            case StencilOp::Keep: return "D3D12_STENCIL_OP_KEEP";
            case StencilOp::Zero: return "D3D12_STENCIL_OP_ZERO";
            case StencilOp::Replace: return "D3D12_STENCIL_OP_REPLACE";
            case StencilOp::IncrementSaturate: return "D3D12_STENCIL_OP_INCR_SAT";
            case StencilOp::DecrimentSaturate: return "D3D12_STENCIL_OP_DECR_SAT";
            case StencilOp::Invert: return "D3D12_STENCIL_OP_INVERT";
            case StencilOp::Increment: return "D3D12_STENCIL_OP_INCR";
            case StencilOp::Decriment: return "D3D12_STENCIL_OP_DECR";
        }

        Assert(false, "Unhandled StencilOp: %i (%s)", (int)op, EnumToString(op));
        return __FUNCTION__ " unhandled StencilOp";
    }

    static std::string SamplerAddressModeToD3D12_TEXTURE_ADDRESS_MODE(SamplerAddressMode addressMode)
    {
        switch (addressMode)
        {
            case SamplerAddressMode::Clamp: return "D3D12_TEXTURE_ADDRESS_MODE_CLAMP";
            case SamplerAddressMode::Wrap: return "D3D12_TEXTURE_ADDRESS_MODE_WRAP";
            case SamplerAddressMode::Border: return "D3D12_TEXTURE_ADDRESS_MODE_BORDER";
            default:
            {
                Assert(false, "Unhandled SamplerAddressMode: %i", (int)addressMode);
                return __FUNCTION__ " unhandled SamplerAddressMode";
            }
        }
    }

    static const std::string TextureFormatToDXGIFormat(TextureFormat textureFormat)
    {
        switch (textureFormat)
        {
            case TextureFormat::Any:
            {
                Assert(false, "TextureFormat::Any is not an acceptable texture format to convert to a DXGI format.");
                break;
            }
            case TextureFormat::R8_Unorm: return "DXGI_FORMAT_R8_UNORM";
            case TextureFormat::RG8_Unorm: return "DXGI_FORMAT_R8G8_UNORM";
            case TextureFormat::RGBA8_Unorm: return "DXGI_FORMAT_R8G8B8A8_UNORM";
            case TextureFormat::RGBA8_Unorm_sRGB: return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
            case TextureFormat::R8_Snorm: return "DXGI_FORMAT_R8_SNORM";
            case TextureFormat::RG8_Snorm: return "DXGI_FORMAT_R8G8_SNORM";
            case TextureFormat::RGBA8_Snorm: return "DXGI_FORMAT_R8G8B8A8_SNORM";
            case TextureFormat::R8_Uint: return "DXGI_FORMAT_R8_UINT";
            case TextureFormat::RG8_Uint: return "DXGI_FORMAT_R8G8_UINT";
            case TextureFormat::RGBA8_Uint: return "DXGI_FORMAT_R8G8B8A8_UINT";
            case TextureFormat::R8_Sint: return "DXGI_FORMAT_R8_SINT";
            case TextureFormat::RG8_Sint: return "DXGI_FORMAT_R8G8_SINT";
            case TextureFormat::RGBA8_Sint: return "DXGI_FORMAT_R8G8B8A8_SINT";
            case TextureFormat::R16_Float: return "DXGI_FORMAT_R16_FLOAT";
            case TextureFormat::RG16_Float: return "DXGI_FORMAT_R16G16_FLOAT";
            case TextureFormat::RGBA16_Float: return "DXGI_FORMAT_R16G16B16A16_FLOAT";
            case TextureFormat::RGBA16_Unorm: return "DXGI_FORMAT_R16G16B16A16_UNORM";
            case TextureFormat::RGBA16_Snorm: return "DXGI_FORMAT_R16G16B16A16_SNORM";
            case TextureFormat::R32_Float: return "DXGI_FORMAT_R32_FLOAT";
            case TextureFormat::RG32_Float: return "DXGI_FORMAT_R32G32_FLOAT";
            case TextureFormat::RGBA32_Float: return "DXGI_FORMAT_R32G32B32A32_FLOAT";
            case TextureFormat::R32_Uint: return "DXGI_FORMAT_R32_UINT";
            case TextureFormat::RGBA32_Uint: return "DXGI_FORMAT_R32G32B32A32_UINT";
            case TextureFormat::R11G11B10_Float: return "DXGI_FORMAT_R11G11B10_FLOAT";
            case TextureFormat::D32_Float: return "DXGI_FORMAT_D32_FLOAT";
            case TextureFormat::D16_Unorm: return "DXGI_FORMAT_D16_UNORM";
            case TextureFormat::D32_Float_S8: return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
            case TextureFormat::D24_Unorm_S8: return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
            case TextureFormat::BC4_Unorm: return "DXGI_FORMAT_BC4_UNORM";
            case TextureFormat::BC4_Snorm: return "DXGI_FORMAT_BC4_SNORM";
            case TextureFormat::BC5_Unorm: return "DXGI_FORMAT_BC5_UNORM";
            case TextureFormat::BC5_Snorm: return "DXGI_FORMAT_BC5_SNORM";
            case TextureFormat::BC7_Unorm: return "DXGI_FORMAT_BC7_UNORM";
            case TextureFormat::BC7_Unorm_sRGB: return "DXGI_FORMAT_BC7_UNORM_SRGB";
            case TextureFormat::BC6_UF16: return "DXGI_FORMAT_BC6H_UF16";
            case TextureFormat::BC6_SF16: return "DXGI_FORMAT_BC6H_SF16";
        }

        Assert(false, "Unhandled TextureFormat: %s (%i)", EnumToString(textureFormat), (int)textureFormat);
        return __FUNCTION__ " unknown texture format";
    }

    static const std::string ShaderResourceTypeToDX12ResourceState(ShaderResourceAccessType resourceType)
    {
        switch (resourceType)
        {
            case ShaderResourceAccessType::UAV: return "D3D12_RESOURCE_STATE_UNORDERED_ACCESS";
            case ShaderResourceAccessType::SRV: return "D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE";
            case ShaderResourceAccessType::CopySource: return "D3D12_RESOURCE_STATE_COPY_SOURCE";
            case ShaderResourceAccessType::CopyDest: return "D3D12_RESOURCE_STATE_COPY_DEST";
            case ShaderResourceAccessType::CBV: return "D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER";
            case ShaderResourceAccessType::Indirect: return "D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT";
            case ShaderResourceAccessType::RTScene: return "D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE";
            case ShaderResourceAccessType::RenderTarget: return "D3D12_RESOURCE_STATE_RENDER_TARGET";
            case ShaderResourceAccessType::DepthTarget: return "D3D12_RESOURCE_STATE_DEPTH_WRITE";
            case ShaderResourceAccessType::VertexBuffer: return "D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER";
            case ShaderResourceAccessType::IndexBuffer: return "D3D12_RESOURCE_STATE_INDEX_BUFFER";
            case ShaderResourceAccessType::ShadingRate: return "D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE";
        }

        Assert(false, "Unhandled ShaderResourceType: %s (%i)", EnumToString(resourceType), (int)resourceType);
        return __FUNCTION__ " unknown resource type";
    }

    static const std::string ShadingRateToD3D12_SHADING_RATE(ShadingRate shadingRate)
    {
        switch (shadingRate)
        {
            case ShadingRate::_1x1: return "D3D12_SHADING_RATE_1X1";
            case ShadingRate::_1x2: return "D3D12_SHADING_RATE_1X2";
            case ShadingRate::_2x1: return "D3D12_SHADING_RATE_2X1";
            case ShadingRate::_2x2: return "D3D12_SHADING_RATE_2X2";
            case ShadingRate::_2x4: return "D3D12_SHADING_RATE_2X4";
            case ShadingRate::_4x2: return "D3D12_SHADING_RATE_4X2";
            case ShadingRate::_4x4: return "D3D12_SHADING_RATE_4X4";
        }

        Assert(false, "Unhandled ShadingRate: %s (%i)", EnumToString(shadingRate), (int)shadingRate);
        return __FUNCTION__ " unknown ShadingRate";
    }

    static const std::string ShadingRateCombinerToD3D12_SHADING_RATE_COMBINER(ShadingRateCombiner shadingRateCombiner)
    {
        switch (shadingRateCombiner)
        {
            case ShadingRateCombiner::PassThrough: return "D3D12_SHADING_RATE_COMBINER_PASSTHROUGH";
            case ShadingRateCombiner::Override: return "D3D12_SHADING_RATE_COMBINER_OVERRIDE";
            case ShadingRateCombiner::Min: return "D3D12_SHADING_RATE_COMBINER_MIN";
            case ShadingRateCombiner::Max: return "D3D12_SHADING_RATE_COMBINER_MAX";
            case ShadingRateCombiner::Sum: return "D3D12_SHADING_RATE_COMBINER_SUM";
        }

        Assert(false, "Unhandled ShadingRateCombiner: %s (%i)", EnumToString(shadingRateCombiner), (int)shadingRateCombiner);
        return __FUNCTION__ " unknown ShadingRateCombiner";
    }

    inline static std::string VariableToStringInsideContext(const Variable& variable, const RenderGraph& renderGraph)
    {
        // get the path to the variable
        std::string path;
        switch (variable.visibility)
        {
            case VariableVisibility::Internal:
            {
                path = "m_internal.";
                break;
            }
            case VariableVisibility::User:
            case VariableVisibility::Host:
            {
                path = "m_input.";
                break;
            }
            default:
            {
                Assert(false, "Unhandled variable visibility");
            }
        }

        // return the path plus the variable name
        return path + "variable_" + variable.name;
    }

    inline static std::string VariableToString(const Variable& variable, const RenderGraph& renderGraph)
    {
        return std::string("context->") + VariableToStringInsideContext(variable, renderGraph);
    }

    inline static std::string ResourceToStringInsideContext(const RenderGraphNode& node)
    {
        // get the path to the variable
        std::string path;
        switch (GetNodeResourceVisibility(node))
        {
            case ResourceVisibility::Imported: path = "m_input."; break;
            case ResourceVisibility::Internal: path = "m_internal."; break;
            case ResourceVisibility::Exported: path = "m_output."; break;
            default:
            {
                Assert(false, "Unhandled resource visibility");
            }
        }

        switch (node._index)
        {
            case RenderGraphNode::c_index_resourceBuffer: return path + "buffer_" + GetNodeName(node);
            case RenderGraphNode::c_index_resourceTexture: return path + "texture_" + GetNodeName(node);
            default:
            {
                Assert(false, "Unhandled resource node type");
            }
        }
        return "<error>";
    }

    inline static std::string ResourceToString(const RenderGraphNode& node)
    {
        return std::string("context->") + ResourceToStringInsideContext(node);
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

    static std::string VariableDefaultValue(const Variable& var, const RenderGraph& renderGraph)
    {
        if (var.enumIndex == -1)
            return DataFieldTypeValue(var.type, var.dflt);

        return renderGraph.enums[var.enumIndex].name + std::string("::") + var.dflt;
    }

    static std::string StructFieldDefaultValue(const StructField& field, const RenderGraph& renderGraph)
    {
        if (field.enumIndex == -1)
            return DataFieldTypeValue(field.type, field.dflt);

        return std::string("(int)") + renderGraph.enums[field.enumIndex].name + std::string("::") + field.dflt;
    }


    static std::string DataFieldTypeToCPPType(DataFieldType type)
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
            case DataFieldType::Bool: return "bool";
            case DataFieldType::Float4x4: return "float4x4";
            default:
            {
                Assert(false, "Unhandled data field type: %s (%i)", EnumToString(type), type);
                return __FUNCTION__ " unknown field type";
            }
        }
    }

    static std::string VariableTypeToCPPType(const Variable& variable, const RenderGraph& renderGraph, bool allowEnums = true)
    {
        if (!allowEnums || variable.enumIndex == -1)
            return DataFieldTypeToCPPType(variable.type);

        return renderGraph.enums[variable.enumIndex].name;
    }

    static std::string DataFieldTypeToCPPBufferType(DataFieldType type)
    {
        // bools are differently sized on cpu and gpu
        if (type == DataFieldType::Bool)
            return "unsigned int";
        else
            return DataFieldTypeToCPPType(type);
    }

    #include "nodes/nodes.inl"

    static void EmitTransitions(RenderGraph& renderGraph, std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const std::vector<ResourceTransition>& transitions)
    {
        if (transitions.empty())
            return;

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n        // Transition resources for the next action"
            "\n        {"
            "\n            D3D12_RESOURCE_BARRIER barriers[" << transitions.size() << "];"
        ;

        size_t transitionIndex = 0;
        for (size_t index = 0; index < transitions.size(); ++index)
        {
            const ResourceTransition& transition = transitions[index];

            const char* resourcePrefix = "";
            switch (renderGraph.nodes[transition.nodeIndex]._index)
            {
                case RenderGraphNode::c_index_resourceBuffer: resourcePrefix = "buffer_"; break;
                case RenderGraphNode::c_index_resourceTexture: resourcePrefix = "texture_"; break;
                default: Assert(false, "Unhandled resource node type"); break;
            }

            if (transition.oldState == ShaderResourceAccessType::UAV && transition.newState == ShaderResourceAccessType::UAV)
            {
                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            barriers[" << transitionIndex << "].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;"
                    "\n            barriers[" << transitionIndex << "].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;"
                    "\n            barriers[" << transitionIndex << "].UAV.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(renderGraph.nodes[transition.nodeIndex])) << resourcePrefix << GetNodeName(renderGraph.nodes[transition.nodeIndex]) << ";"
                ;
            }
            else
            {
                std::string namePrefix = (transition.newState == ShaderResourceAccessType::CBV)
                    ? "constantBuffer_"
                    : resourcePrefix
                ;

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            barriers[" << transitionIndex << "].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;"
                    "\n            barriers[" << transitionIndex << "].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;"
                    "\n            barriers[" << transitionIndex << "].Transition.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(renderGraph.nodes[transition.nodeIndex])) << namePrefix << GetNodeName(renderGraph.nodes[transition.nodeIndex]) << ";"
                    "\n            barriers[" << transitionIndex << "].Transition.StateBefore = " << ShaderResourceTypeToDX12ResourceState(transition.oldState) << ";"
                    "\n            barriers[" << transitionIndex << "].Transition.StateAfter = " << ShaderResourceTypeToDX12ResourceState(transition.newState) << ";"
                    "\n            barriers[" << transitionIndex << "].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;"
                ;
            }
            transitionIndex++;
        }

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n            commandList->ResourceBarrier(" << transitions.size() << ", barriers);"
            "\n        }"
        ;
    }

    static void MakeStringReplacementGlobal(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, RenderGraph& renderGraph)
    {
        BackendBase::MakeStringReplacementGlobal(stringReplacementMap, renderGraph);

        // Simple things
        {
            stringReplacementMap["/*$(DXShaderCompilerIsDXC)*/"] << ((renderGraph.settings.dx12.shaderCompiler == DXShaderCompiler::DXC) ? "true" : "false");

            if (renderGraph.PrimaryOutput.nodeIndex == -1 || renderGraph.nodes[renderGraph.PrimaryOutput.nodeIndex]._index != RenderGraphNode::c_index_resourceTexture)
            {
                stringReplacementMap["/*$(CopyPrimaryOutput)*/"] <<
                    "// Copy the primary output to the screen\n"
                    "            // if (m_" << renderGraph.name << "->GetPrimaryOutputTexture())\n"
                    "            //     CopyTextureToTexture(g_pd3dCommandList, m_" << renderGraph.name << "->GetPrimaryOutputTexture(), m_" << renderGraph.name << "->GetPrimaryOutputTextureState(), g_mainRenderTargetResource[backBufferIdx], D3D12_RESOURCE_STATE_RENDER_TARGET);\n";

                stringReplacementMap["/*$(GetPrimaryOutputTexture)*/"] << "return nullptr;";
                stringReplacementMap["/*$(GetPrimaryOutputTextureState)*/"] << "return D3D12_RESOURCE_STATE_COMMON;";
            }
            else
            {
                const RenderGraphNode& node = renderGraph.nodes[renderGraph.PrimaryOutput.nodeIndex];
                ResourceVisibility nodeVisibility = GetNodeResourceVisibility(node);

                if (nodeVisibility == ResourceVisibility::Imported)
                    stringReplacementMap["/*$(GetPrimaryOutputTextureState)*/"] << "return " << GetResourceNodePathInContext(nodeVisibility) << "texture_" << GetNodeName(node) << "_state;";
                else
                    stringReplacementMap["/*$(GetPrimaryOutputTextureState)*/"] << "return " << GetResourceNodePathInContext(nodeVisibility) << "c_texture_" << GetNodeName(node) << "_endingState;";

                stringReplacementMap["/*$(GetPrimaryOutputTexture)*/"] << "return " << GetResourceNodePathInContext(nodeVisibility) << "texture_" << GetNodeName(node) << ";";


                stringReplacementMap["/*$(CopyPrimaryOutput)*/"] <<
                    "// Copy the primary output to the screen\n"
                    "            if (m_" << renderGraph.name << "->GetPrimaryOutputTexture())\n"
                    "                CopyTextureToTexture(g_pd3dCommandList, m_" << renderGraph.name << "->GetPrimaryOutputTexture(), m_" << renderGraph.name << "->GetPrimaryOutputTextureState(), g_mainRenderTargetResource[backBufferIdx], D3D12_RESOURCE_STATE_RENDER_TARGET);\n"
                    ;
            }
        }

        // Imported resources
        {
            for (const RenderGraphNode& node : renderGraph.nodes)
            {
                if (!GetNodeIsResourceNode(node) || GetNodeResourceVisibility(node) != ResourceVisibility::Imported)
                    continue;

                std::string nodeName = GetNodeName(node);

                switch (node._index)
                {
                    case RenderGraphNode::c_index_resourceBuffer:
                    {
                        stringReplacementMap["/*$(FirstPreExecute)*/"] <<
                            "\n"
                            "                // Buffer " << nodeName << "\n"
                            "                {\n"
                            "                    // TODO: Set up buffer " << nodeName << "\n"
                            "                    /*\n"
                            "                    unsigned char* initialData = nullptr;\n"
                            "                    size_t initialDataSize = 0;\n"
                            "                    m_" << renderGraph.name << "->m_input.buffer_" << nodeName << "_format = DXGI_FORMAT_UNKNOWN;\n"
                            "                    m_" << renderGraph.name << "->m_input.buffer_" << nodeName << "_stride = 0;\n"
                            "                    m_" << renderGraph.name << "->m_input.buffer_" << nodeName << "_count = 0;\n"
                            "                    m_" << renderGraph.name << "->m_input.buffer_" << nodeName << "_state = D3D12_RESOURCE_STATE_COMMON;\n"
                            "                    m_" << renderGraph.name << "->m_input.buffer_" << nodeName << " = m_" << renderGraph.name << "->CreateManagedBuffer(\n"
                            "                        g_pd3dDevice,\n"
                            "                        g_pd3dCommandList,\n"
                            "                        m_" << renderGraph.name << "->m_input.c_buffer_" << nodeName << "_flags,\n"
                            "                        (void*)initialData, initialDataSize,\n"
                            "                        L\"" << renderGraph.name << "." << nodeName << "\"\n"
                            "                        m_" << renderGraph.name << "->m_input.buffer_" << nodeName << "_state\n"
                            "                    );\n"
                            "                    */\n"
                            "                }\n"
                            ;
                        break;
                    }
                    case RenderGraphNode::c_index_resourceTexture:
                    {
                        stringReplacementMap["/*$(FirstPreExecute)*/"] <<
                            "\n"
                            "                // Texture " << nodeName << "\n"
                            "                {\n"
                            "                    // TODO: Set up texture " << nodeName << "\n"
                            "                    // Note: Use CreateManagedTexture() to create a texture with specific initial data, instead of loading it from disk\n"
                            "                    /*\n"
                            "                    const char* fileName = \"image.png\";\n"
                            "                    bool fileIsSRGB = true;\n"
                            "                    m_" << renderGraph.name << "->m_input.texture_" << nodeName << "_format = DXGI_FORMAT_UNKNOWN;\n"
                            "                    m_" << renderGraph.name << "->m_input.texture_" << nodeName << "_state = D3D12_RESOURCE_STATE_COMMON;\n"
                            "                    m_" << renderGraph.name << "->m_input.texture_" << nodeName << " = m_" << renderGraph.name << "->CreateManagedTextureFromFile(\n"
                            "                        g_pd3dDevice,\n"
                            "                        g_pd3dCommandList,\n"
                            "                        m_" << renderGraph.name << "->m_input.texture_" << nodeName << "_flags,\n"
                            "                        m_" << renderGraph.name << "->m_input.texture_" << nodeName << "_format,\n"
                            "                        DX12Utils::ResourceType::Texture2D,\n"
                            "                        fileName,\n"
                            "                        fileIsSRGB,\n"
                            "                        m_" << renderGraph.name << "->m_input.texture_" << nodeName << "_size,\n"
                            "                        L\"" << renderGraph.name << "." << nodeName << "\",\n"
                            "                        m_" << renderGraph.name << "->m_input.texture_" << nodeName << "_state\n"
                            "                    );\n"
                            "                    */\n"
                            "                }\n"
                            ;
                        break;
                    }
                }
            }
        }

        // Make the enums
        {
            // C++ enum definitions
            for (int enumIndex = 0; enumIndex < (int)renderGraph.enums.size(); ++enumIndex)
            {
                const Enum& e = renderGraph.enums[enumIndex];

                if (!e.comment.empty())
                    stringReplacementMap["/*$(CppEnums)*/"] << "\n\n    // " << e.comment;
                else
                    stringReplacementMap["/*$(CppEnums)*/"] << "\n";

                stringReplacementMap["/*$(CppEnums)*/"] <<
                    "\n    enum class " << e.name << ": int"
                    "\n    {"
                    ;

                for (const auto& item : e.items)
                {
                    stringReplacementMap["/*$(CppEnums)*/"] << "\n        " << item.label << ",";

                    if (!item.comment.empty())
                        stringReplacementMap["/*$(CppEnums)*/"] << " // " << item.comment;
                }

                stringReplacementMap["/*$(CppEnums)*/"] <<
                    "\n    };"
                    ;
            }

            // C++ enum to string
            for (int enumIndex = 0; enumIndex < (int)renderGraph.enums.size(); ++enumIndex)
            {
                const Enum& e = renderGraph.enums[enumIndex];

                stringReplacementMap["/*$(CppEnums)*/"] <<
                    "\n"
                    "\n    inline const char* EnumToString(" << e.name << " value, bool displayString = false)"
                    "\n    {"
                    "\n        switch(value)"
                    "\n        {"
                    ;

                for (const auto& item : e.items)
                {
                    stringReplacementMap["/*$(CppEnums)*/"] <<
                        "\n            case " << e.name << "::" << item.label << ": return displayString ? \"" << item.displayLabel <<  "\" : \"" << item.label << "\";"
                        ;
                }

                stringReplacementMap["/*$(CppEnums)*/"] <<
                    "\n            default: return nullptr;"
                    "\n        }"
                    "\n    }"
                    ;

            }

            // Python interface
            bool firstEnum = true;
            for (int enumIndex = 0; enumIndex < (int)renderGraph.enums.size(); ++enumIndex)
            {
                const Enum& e = renderGraph.enums[enumIndex];
                int itemIndex = -1;
                for (const auto& item : e.items)
                {
                    itemIndex++;
                    stringReplacementMap["/*$(PythonCreateModule)*/"] << "\n        PyModule_AddIntConstant(module, \"" << e.name << "_" << item.label << "\", " << itemIndex << ");";
                }

                stringReplacementMap["/*$(PythonFunctionDeclarations)*/"] << 
                    "\n        {\"" << e.name << "ToString\", " << e.name << "ToString, METH_VARARGS, \"" << e.comment << "\"},";

                if (!firstEnum)
                    stringReplacementMap["/*$(PythonFunctionDefinitions)*/"] << "\n";
                firstEnum = false;

                stringReplacementMap["/*$(PythonFunctionDefinitions)*/"] <<
                    "\n    inline PyObject* " << e.name << "ToString(PyObject* self, PyObject* args)"
                    "\n    {"
                    "\n        int value;"
                    "\n        if (!PyArg_ParseTuple(args, \"i:" << e.name << "ToString\", &value))"
                    "\n            return PyErr_Format(PyExc_TypeError, \"type error\");"
                    "\n"
                    "\n        switch((" << e.name << ")value)"
                    "\n        {"
                    ;

                for (const auto& item : e.items)
                {
                    stringReplacementMap["/*$(PythonFunctionDefinitions)*/"] <<
                        "\n            case " << e.name << "::" << item.label << ": return Py_BuildValue(\"s\", \"" << item.label << "\");"
                        ;
                }

                stringReplacementMap["/*$(PythonFunctionDefinitions)*/"] <<
                    "\n            default: return Py_BuildValue(\"s\", \"<invalid " << e.name << " value>\");"
                    "\n        }"
                    "\n    }"
                    ;
            }
        }

        // compile time settings
        stringReplacementMap["/*$(TechniqueSettings)*/"] <<
            "\n    static const int c_numSRVDescriptors = " << renderGraph.settings.dx12.numSRVDescriptors << ";  // If 0, no heap will be created. One heap shared by all contexts of this technique."
            "\n    static const int c_numRTVDescriptors = " << renderGraph.settings.dx12.numRTVDescriptors << ";  // If 0, no heap will be created. One heap shared by all contexts of this technique."
            "\n    static const int c_numDSVDescriptors = " << renderGraph.settings.dx12.numDSVDescriptors << ";  // If 0, no heap will be created. One heap shared by all contexts of this technique."
            "\n    static const bool c_debugShaders = " << (renderGraph.settings.common.debugShaders ? "true" : "false") << "; // If true, will compile shaders with debug info enabled."
            "\n    static const bool c_debugNames = " << (renderGraph.settings.common.debugNames ? "true" : "false") << "; // If true, will set debug names on objects. If false, debug names should be deadstripped from the executable."
            "\n"
            "\n    // Information about the technique"
            "\n    static const bool c_requiresRaytracing = " << (renderGraph.usesRaytracing ? "true" : "false") << "; // If true, this technique will not work without raytracing support";

        // get a dxr command list if raytracing is used
        if (renderGraph.usesRaytracing)
        {
            stringReplacementMap["/*$(ExecuteBegin)*/"] <<
                "\n"
                "\n        ID3D12Device5* dxrDevice = nullptr;"
                "\n        ID3D12GraphicsCommandList4* dxrCommandList = nullptr;"
                "\n        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxrDevice))) || FAILED(commandList->QueryInterface(IID_PPV_ARGS(&dxrCommandList))))"
                "\n        {"
                "\n            Context::LogFn(LogLevel::Error, \"Could not get dxr command list. Ray tracing not supported, but is required.\");"
                "\n            return;"
                "\n        }";

            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n        ID3D12Device5* dxrDevice = nullptr;"
                "\n        device->QueryInterface(IID_PPV_ARGS(&dxrDevice));";

            stringReplacementMap["/*$(ExecuteEnd)*/"] <<
                "\n"
                "\n        dxrDevice->Release();"
                "\n        dxrCommandList->Release();";
        }

        // Python interface of user facing variables
        {
            bool first = (renderGraph.enums.size() == 0);
            for (const Variable& variable : renderGraph.variables)
            {
                if (variable.visibility != VariableVisibility::User || variable.Const)
                    continue;

                const char* varSymbols = "";
                const char* varRefs = "&value";
                switch (variable.type)
                {
                    case DataFieldType::Int: varSymbols = "i"; break;
                    case DataFieldType::Int2: varSymbols = "ii"; varRefs ="&value[0], &value[1]"; break;
                    case DataFieldType::Int3: varSymbols = "iii"; varRefs = "&value[0], &value[1], &value[2]"; break;
                    case DataFieldType::Int4: varSymbols = "iiii"; varRefs = "&value[0], &value[1], &value[2], &value[3]"; break;
                    case DataFieldType::Uint: varSymbols = "I"; break;
                    case DataFieldType::Uint2: varSymbols = "II"; varRefs = "&value[0], &value[1]"; break;
                    case DataFieldType::Uint3: varSymbols = "III"; varRefs = "&value[0], &value[1], &value[2]"; break;
                    case DataFieldType::Uint4: varSymbols = "IIII"; varRefs = "&value[0], &value[1], &value[2], &value[3]"; break;
                    case DataFieldType::Float: varSymbols = "f"; break;
                    case DataFieldType::Float2: varSymbols = "ff"; varRefs = "&value[0], &value[1]"; break;
                    case DataFieldType::Float3: varSymbols = "fff"; varRefs = "&value[0], &value[1], &value[2]"; break;
                    case DataFieldType::Float4: varSymbols = "ffff"; varRefs = "&value[0], &value[1], &value[2], &value[3]"; break;
                    case DataFieldType::Bool: varSymbols = "b"; break;
                    case DataFieldType::Float4x4: varSymbols = "ffffffffffffffff"; varRefs = "&value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9], &value[10], &value[11], &value[12], &value[13], &value[14], &value[15]"; break;
                    case DataFieldType::Uint_16: varSymbols = "I"; break;
                    default: Assert(false, "Unhandled Variable Type: %i", variable.type); break;
                }

                // Put the function in the list of functions exposed to python
                stringReplacementMap["/*$(PythonFunctionDeclarations)*/"] <<
                    "\n        {\"Set_" << variable.name << "\", Set_" << variable.name << ", METH_VARARGS, \"" << variable.comment << "\"},";

                // Write the body of the function
                if (!first)
                    stringReplacementMap["/*$(PythonFunctionDefinitions)*/"] << "\n";
                first = false;

                stringReplacementMap["/*$(PythonFunctionDefinitions)*/"] <<
                    "\n    inline PyObject* Set_" << variable.name << "(PyObject* self, PyObject* args)"
                    "\n    {"
                    "\n        int contextIndex;"
                    "\n        " << VariableTypeToCPPType(variable, renderGraph, false) << " value;"
                    ;

                std::string cast;
                if (variable.enumIndex != -1)
                    cast = std::string("(") + renderGraph.enums[variable.enumIndex].name + std::string(")");

                stringReplacementMap["/*$(PythonFunctionDefinitions)*/"] <<
                    "\n"
                    "\n        if (!PyArg_ParseTuple(args, \"i" << varSymbols << ":Set_" << variable.name << "\", &contextIndex, " << varRefs << "))"
                    "\n            return PyErr_Format(PyExc_TypeError, \"type error\");"
                    "\n"
                    "\n        Context* context = Context::GetContext(contextIndex);"
                    "\n        if (!context)"
                    "\n            return PyErr_Format(PyExc_IndexError, __FUNCTION__, \"() : index % i is out of range(count = % i)\", contextIndex, Context::GetContextCount());"
                    "\n"
                    "\n        context->m_input.variable_" << variable.name << " = " << cast << "value;"
                    "\n"
                    "\n        Py_INCREF(Py_None);"
                    "\n        return Py_None;"
                    "\n    }"
                    ;
            }
        }

        // imgui of user facing variables and profiling data
        {
            stringReplacementMap["/*$(IMGUI)*/"] <<
                "\n        ImGui::PushID(\"gigi_" << renderGraph.name << "\");";

            // variables
            bool first = true;
            for (const Variable& variable : renderGraph.variables)
            {
                if (variable.visibility != VariableVisibility::User || variable.Const)
                    continue;

                if (first)
                {
                    first = false;
                    stringReplacementMap["/*$(IMGUI)*/"] << "\n";
                }

                switch (variable.type)
                {
                    case DataFieldType::Bool:
                    {
                        if (variable.UISettings.UIHint == VariableUIHint::Button)
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        " << VariableToString(variable, renderGraph) << " = ImGui::Button(\"" << variable.name << "\");"
                                ;

                            if (!variable.comment.empty())
                            {
                                stringReplacementMap["/*$(IMGUI)*/"] <<
                                    "\n        ShowToolTip(\"" << variable.comment << "\");"
                                    ;
                            }
                        }
                        else
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        ImGui::Checkbox(\"" << variable.name << "\", &" << VariableToString(variable, renderGraph) << ");"
                                ;

                            if (!variable.comment.empty())
                            {
                                stringReplacementMap["/*$(IMGUI)*/"] <<
                                    "\n        ShowToolTip(\"" << variable.comment << "\");"
                                    ;
                            }
                        }
                        break;
                    }
                    case DataFieldType::Int:
                    {
                        if (variable.enumIndex == -1)
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        ImGui::InputInt(\"" << variable.name << "\", &" << VariableToString(variable, renderGraph) << ", 0);"
                                ;

                            if (!variable.comment.empty())
                            {
                                stringReplacementMap["/*$(IMGUI)*/"] <<
                                    "\n        ShowToolTip(\"" << variable.comment << "\");"
                                    ;
                            }
                        }
                        else
                        {
                            const Enum& e = renderGraph.enums[variable.enumIndex];

                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        {"
                                "\n            static const char* labels[] = {"
                                ;

                            for (const auto& item : e.items)
                                stringReplacementMap["/*$(IMGUI)*/"] << "\n                \"" << item.displayLabel << "\",";

                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n            };"
                                "\n            ImGui::Combo(\"" << variable.name << "\", (int*)&" << VariableToString(variable, renderGraph) << ", labels, " << e.items.size() << ");"
                                ;

                            if (!variable.comment.empty())
                            {
                                stringReplacementMap["/*$(IMGUI)*/"] <<
                                    "\n            ShowToolTip(\"" << variable.comment << "\");"
                                    ;
                            }

                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        }"
                                ;
                        }

                        break;
                    }
                    case DataFieldType::Uint:
                    {
                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        {"
                            "\n            int localVar = (int)" << VariableToString(variable, renderGraph) << ";"
                            "\n            if(ImGui::InputInt(\"" << variable.name << "\", &localVar, 0))"
                            "\n                " << VariableToString(variable, renderGraph) << " = (unsigned int)localVar;"
                            ;

                        if (!variable.comment.empty())
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n            ShowToolTip(\"" << variable.comment << "\");"
                                ;
                        }

                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        }";
                        break;
                    }
                    case DataFieldType::Uint2:
                    {
                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        {"
                            "\n            float width = ImGui::GetContentRegionAvail().x / 4.0f;"
                            "\n            ImGui::PushID(\"" << variable.name << "\");"
                            "\n            ImGui::PushItemWidth(width);"
                            "\n            int localVarX = (int)" << VariableToString(variable, renderGraph) << "[0];"
                            "\n            if(ImGui::InputInt(\"##X\", &localVarX, 0))"
                            "\n                " << VariableToString(variable, renderGraph) << "[0] = (unsigned int)localVarX;"
                            "\n            ImGui::SameLine();"
                            "\n            int localVarY = (int)" << VariableToString(variable, renderGraph) << "[1];"
                            "\n            if(ImGui::InputInt(\"##Y\", &localVarY, 0))"
                            "\n                " << VariableToString(variable, renderGraph) << "[1] = (unsigned int)localVarY;"
                            "\n            ImGui::SameLine();"
                            "\n            ImGui::Text(\"" << variable.name << "\");"
                            "\n            ImGui::PopItemWidth();"
                            "\n            ImGui::PopID();"
                            ;

                        if (!variable.comment.empty())
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n            ShowToolTip(\"" << variable.comment << "\");"
                                ;
                        }

                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        }";
                        break;
                    }
                    case DataFieldType::Uint3:
                    {
                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        {"
                            "\n            float width = ImGui::GetContentRegionAvail().x / 4.0f;"
                            "\n            ImGui::PushID(\"" << variable.name << "\");"
                            "\n            ImGui::PushItemWidth(width);"
                            "\n            int localVarX = (int)" << VariableToString(variable, renderGraph) << "[0];"
                            "\n            if(ImGui::InputInt(\"##X\", &localVarX, 0))"
                            "\n                " << VariableToString(variable, renderGraph) << "[0] = (unsigned int)localVarX;"
                            "\n            ImGui::SameLine();"
                            "\n            int localVarY = (int)" << VariableToString(variable, renderGraph) << "[1];"
                            "\n            if(ImGui::InputInt(\"##Y\", &localVarY, 0))"
                            "\n                " << VariableToString(variable, renderGraph) << "[1] = (unsigned int)localVarY;"
                            "\n            ImGui::SameLine();"
                            "\n            int localVarZ = (int)" << VariableToString(variable, renderGraph) << "[2];"
                            "\n            if(ImGui::InputInt(\"##Z\", &localVarZ, 0))"
                            "\n                " << VariableToString(variable, renderGraph) << "[2] = (unsigned int)localVarZ;"
                            "\n            ImGui::SameLine();"
                            "\n            ImGui::Text(\"" << variable.name << "\");"
                            "\n            ImGui::PopItemWidth();"
                            "\n            ImGui::PopID();"
                            ;

                        if (!variable.comment.empty())
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n            ShowToolTip(\"" << variable.comment << "\");"
                                ;
                        }

                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        }";
                        break;
                    }
                    case DataFieldType::Float:
                    {
                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        ImGui::InputFloat(\"" << variable.name << "\", &" << VariableToString(variable, renderGraph) << ");"
                            ;

                        if (!variable.comment.empty())
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        ShowToolTip(\"" << variable.comment << "\");"
                                ;
                        }
                        break;
                    }
                    case DataFieldType::Float2:
                    {
                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        {"
                            "\n            float width = ImGui::GetContentRegionAvail().x / 4.0f;"
                            "\n            ImGui::PushID(\"" << variable.name << "\");"
                            "\n            ImGui::PushItemWidth(width);"
                            "\n            ImGui::InputFloat(\"##X\", &" << VariableToString(variable, renderGraph) << "[0]);"
                            "\n            ImGui::SameLine();"
                            "\n            ImGui::InputFloat(\"##Y\", &" << VariableToString(variable, renderGraph) << "[1]);"
                            "\n            ImGui::SameLine();"
                            "\n            ImGui::Text(\"" << variable.name << "\");"
                            "\n            ImGui::PopItemWidth();"
                            "\n            ImGui::PopID();"
                            ;

                        if (!variable.comment.empty())
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n            ShowToolTip(\"" << variable.comment << "\");"
                                ;
                        }

                        stringReplacementMap["/*$(IMGUI)*/"] <<
                            "\n        }"
                            ;
                        break;
                    }
                    case DataFieldType::Float3:
                    {
                        if (variable.UISettings.UIHint == VariableUIHint::Color)
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        ImGui::ColorEdit3(\"" << variable.name << "\", &" << VariableToString(variable, renderGraph) << "[0]);"
                                ;
                        }
                        else
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        {"
                                "\n            float width = ImGui::GetContentRegionAvail().x / 5.0f;"
                                "\n            ImGui::PushID(\"" << variable.name << "\");"
                                "\n            ImGui::PushItemWidth(width);"
                                "\n            ImGui::InputFloat(\"##X\", &" << VariableToString(variable, renderGraph) << "[0]);"
                                "\n            ImGui::SameLine();"
                                "\n            ImGui::InputFloat(\"##Y\", &" << VariableToString(variable, renderGraph) << "[1]);"
                                "\n            ImGui::SameLine();"
                                "\n            ImGui::InputFloat(\"##Z\", &" << VariableToString(variable, renderGraph) << "[2]);"
                                "\n            ImGui::SameLine();"
                                "\n            ImGui::Text(\"" << variable.name << "\");"
                                "\n            ImGui::PopItemWidth();"
                                "\n            ImGui::PopID();"
                                ;

                            if (!variable.comment.empty())
                            {
                                stringReplacementMap["/*$(IMGUI)*/"] <<
                                    "\n            ShowToolTip(\"" << variable.comment << "\");"
                                    ;
                            }

                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        }"
                                ;
                        }
                        break;
                    }
                    case DataFieldType::Float4:
                    {
                        if (variable.UISettings.UIHint == VariableUIHint::Color)
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        ImGui::ColorEdit4(\"" << variable.name << "\", &" << VariableToString(variable, renderGraph) << "[0]);"
                                ;
                        }
                        else
                        {
                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        {"
                                "\n            float width = ImGui::GetContentRegionAvail().x / 6.0f;"
                                "\n            ImGui::PushID(\"" << variable.name << "\");"
                                "\n            ImGui::PushItemWidth(width);"
                                "\n            ImGui::InputFloat(\"##X\", &" << VariableToString(variable, renderGraph) << "[0]);"
                                "\n            ImGui::SameLine();"
                                "\n            ImGui::InputFloat(\"##Y\", &" << VariableToString(variable, renderGraph) << "[1]);"
                                "\n            ImGui::SameLine();"
                                "\n            ImGui::InputFloat(\"##Z\", &" << VariableToString(variable, renderGraph) << "[2]);"
                                "\n            ImGui::SameLine();"
                                "\n            ImGui::InputFloat(\"##W\", &" << VariableToString(variable, renderGraph) << "[3]);"
                                "\n            ImGui::SameLine();"
                                "\n            ImGui::Text(\"" << variable.name << "\");"
                                "\n            ImGui::PopItemWidth();"
                                "\n            ImGui::PopID();"
                                ;

                            if (!variable.comment.empty())
                            {
                                stringReplacementMap["/*$(IMGUI)*/"] <<
                                    "\n            ShowToolTip(\"" << variable.comment << "\");"
                                    ;
                            }

                            stringReplacementMap["/*$(IMGUI)*/"] <<
                                "\n        }"
                                ;
                        }
                        break;
                    }
                    default:
                    {
                        Assert(false, "Unhandled Variable Type: %i", variable.type);
                        break;
                    }
                }
            }

            // imgui of profiling data
            {
                stringReplacementMap["/*$(IMGUI)*/"] <<
                    "\n"
                    "\n        ImGui::Checkbox(\"Profile\", &context->m_profile);"
                    "\n        if (context->m_profile)"
                    "\n        {"
                    "\n            int numEntries = 0;"
                    "\n            const ProfileEntry* entries = context->ReadbackProfileData(commandQueue, numEntries);"
                    "\n            if (ImGui::BeginTable(\"profiling\", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))"
                    "\n            {"
                    "\n                ImGui::TableSetupColumn(\"Label\");"
                    "\n                ImGui::TableSetupColumn(\"CPU ms\");"
                    "\n                ImGui::TableSetupColumn(\"GPU ms\");"
                    "\n                ImGui::TableHeadersRow();"
                    "\n                float totalCpu = 0.0f;"
                    "\n                float totalGpu = 0.0f;"
                    "\n                for (int entryIndex = 0; entryIndex < numEntries; ++entryIndex)"
                    "\n                {"
                    "\n                    ImGui::TableNextRow();"
                    "\n                    ImGui::TableNextColumn();"
                    "\n                    ImGui::TextUnformatted(entries[entryIndex].m_label);"
                    "\n                    ImGui::TableNextColumn();"
                    "\n                    ImGui::Text(\"%0.3f\", entries[entryIndex].m_cpu * 1000.0f);"
                    "\n                    ImGui::TableNextColumn();"
                    "\n                    ImGui::Text(\"%0.3f\", entries[entryIndex].m_gpu * 1000.0f);"
                    "\n                    totalCpu += entries[entryIndex].m_cpu;"
                    "\n                    totalGpu += entries[entryIndex].m_gpu;"
                    "\n                }"
                    "\n                ImGui::EndTable();"
                    "\n            }"
                    "\n        }"
                    ;
            }

            stringReplacementMap["/*$(IMGUI)*/"] <<
                "\n"
                "\n        ImGui::PopID();";
        }

        // make the structs for the constant buffers of shaders
        {
            for (const Struct& cb : renderGraph.structs)
            {
                const char* location = cb.exported ? "/*$(Public)*/" : "/*$(ContextInternal)*/";
                const char* indent = cb.exported ? "    " : "        ";

                stringReplacementMap[location] <<
                    "\n"
                    "\n" << indent << "struct Struct_" << cb.name <<
                    "\n" << indent << "{";

                for (const StructField& field : cb.fields)
                {
                    stringReplacementMap[location] <<
                        "\n" << indent << "    " << DataFieldTypeToCPPBufferType(field.type) << " " << field.name << " = " << StructFieldDefaultValue(field, renderGraph) << ";";

                    if (!field.comment.empty())
                        stringReplacementMap[location] << "  // " << field.comment;
                }

                stringReplacementMap[location] <<
                    "\n" << indent << "};";
            }
        }

        // Make the setVars
        {
            bool firstSetVar[2] = { true, true };

            for (const SetVariable& setVar : renderGraph.setVars)
            {
                if (setVar.destination.variableIndex == -1)
                    continue;

                const char* destinationString = setVar.setBefore ? "/*$(ExecuteBegin)*/" : "/*$(ExecuteEnd)*/";

                // Show a comment for setting variables
                if (firstSetVar[!!setVar.setBefore])
                {
                    firstSetVar[!!setVar.setBefore] = false;
                    stringReplacementMap[destinationString] <<
                        "\n"
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
                    stringReplacementMap[destinationString] << BackendDX12::ResourceToString(renderGraph.nodes[setVar.ANode.nodeIndex]) << "_size";
                    if (setVar.AVarIndex != -1)
                        stringReplacementMap[destinationString] << "[" << setVar.AVarIndex << "]";
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
                        stringReplacementMap[destinationString] << BackendDX12::ResourceToString(renderGraph.nodes[setVar.BNode.nodeIndex]) << "_size";
                        if (setVar.BVarIndex != -1)
                            stringReplacementMap[destinationString] << "[" << setVar.BVarIndex << "]";
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

        // make the starting imported texture transitions
        int importedResourcesCount = 0;
        {
            // count how many imported resources there are
            for (RenderGraphNode& node : renderGraph.nodes)
            {
                if (GetNodeIsResourceNode(node) && GetNodeResourceVisibility(node) == ResourceVisibility::Imported)
                    importedResourcesCount++;
            }

            if (importedResourcesCount > 0)
            {
                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n        // Make sure imported resources are in the correct state"
                    "\n        {"
                    "\n            int barrierCount = 0;"
                    "\n            D3D12_RESOURCE_BARRIER barriers[" << importedResourcesCount << "];"
                    ;

                for (RenderGraphNode& node : renderGraph.nodes)
                {
                    if (!GetNodeIsResourceNode(node) || GetNodeResourceVisibility(node) != ResourceVisibility::Imported || !ResourceNodeIsUsed(node))
                        continue;

                    const char* resourcePrefix = "";
                    switch (node._index)
                    {
                        case RenderGraphNode::c_index_resourceBuffer: resourcePrefix = "buffer_"; break;
                        case RenderGraphNode::c_index_resourceTexture: resourcePrefix = "texture_"; break;
                        default: Assert(false, "Unhandled resource node type"); break;
                    }

                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n"
                        "\n            if(context->m_input." << resourcePrefix << GetNodeName(node) << "_state != " << ShaderResourceTypeToDX12ResourceState(GetResourceNodeStartingState(node)) << ")"
                        "\n            {"
                        "\n                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;"
                        "\n                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;"
                        "\n                barriers[barrierCount].Transition.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(node)) << resourcePrefix << GetNodeName(node) << ";"
                        "\n                barriers[barrierCount].Transition.StateBefore = context->m_input." << resourcePrefix << GetNodeName(node) << "_state;"
                        "\n                barriers[barrierCount].Transition.StateAfter = " << ShaderResourceTypeToDX12ResourceState(GetResourceNodeStartingState(node)) << ";"
                        "\n                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;"
                        "\n                barrierCount++;"
                        "\n            }"
                        ;

                    if (GetResourceNodeStartingState(node) == ShaderResourceAccessType::UAV)
                    {
                        stringReplacementMap["/*$(Execute)*/"] <<
                            "\n            else"
                            "\n            {"
                            "\n                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;"
                            "\n                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;"
                            "\n                barriers[barrierCount].UAV.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(node)) << resourcePrefix << GetNodeName(node) << ";"
                            "\n                barrierCount++;"
                            "\n            }"
                            ;
                    }
                }

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            if(barrierCount > 0)"
                    "\n                commandList->ResourceBarrier(barrierCount, barriers);"
                    "\n        }"
                    ;
            }
        }

        // Variable storage
        {
            bool firstVarOfType[(int)VariableVisibility::Count];
            std::fill_n(firstVarOfType, (int)VariableVisibility::Count, true);
            for (const Variable& variable : renderGraph.variables)
            {
                VariableVisibility visibility = variable.visibility;

                // Just to keep this logic simpler
                if (visibility == VariableVisibility::Host)
                    visibility = VariableVisibility::User;

                const char* location = nullptr;
                const char* indent = nullptr;
                const char* cppScope = nullptr;
                switch (visibility)
                {
                    case VariableVisibility::Internal:
                    {
                        location = "/*$(ContextInternal)*/";
                        indent = "\n        ";
                        cppScope = "ContextInternal::";
                        break;
                    }
                    case VariableVisibility::Host:
                    case VariableVisibility::User:
                    {
                        location = "/*$(ContextInput)*/";
                        indent = "\n            ";
                        cppScope = "Context::ContextInput::";
                        break;
                    }
                    default:
                    {
                        Assert(false, "Unhandled variable visibility");
                    }
                }

                if (firstVarOfType[(int)visibility])
                {
                    firstVarOfType[(int)visibility] = false;
                    stringReplacementMap[location] << "\n" << indent << "// Variables";
                }

                if (variable.Static)
                {
                    stringReplacementMap[location] << indent << "static " << (variable.Const ? "const " : "") << VariableTypeToCPPType(variable, renderGraph) << " variable_" << variable.name << ";";
                    if (!variable.comment.empty())
                        stringReplacementMap[location] << "  // " << variable.comment;

                    stringReplacementMap["/*$(StaticVariables)*/"] << "\n    " << (variable.Const ? "const " : "") << VariableTypeToCPPType(variable, renderGraph) << " " << cppScope << "variable_" << variable.name << " = " << VariableDefaultValue(variable, renderGraph) << ";";
                }
                else
                {
                    stringReplacementMap[location] << indent << (variable.Const ? "const " : "") << VariableTypeToCPPType(variable, renderGraph) << " variable_" << variable.name << " = " << VariableDefaultValue(variable, renderGraph) << ";";
                    if (!variable.comment.empty())
                        stringReplacementMap[location] << "  // " << variable.comment;
                }
            }
        }

        // Fill out the command list for each node, in flattened node list order. Also let the node fill out storage etc that are not order dependent.
        for (size_t stepIndex = 0; stepIndex < renderGraph.flattenedNodeList.size(); ++stepIndex)
        {
            // Do transitions needed before this node
            if (renderGraph.transitions[stepIndex].transitions.size() > 0)
                EmitTransitions(renderGraph, stringReplacementMap, renderGraph.transitions[stepIndex].transitions);

            // Let the node do work
            int nodeIndex = renderGraph.flattenedNodeList[stepIndex];
            RenderGraphNode& node = renderGraph.nodes[nodeIndex];
            ExecuteOnNode(node, [&](auto& node) { MakeStringReplacementForNode(stringReplacementMap, renderGraph, node); });
        }

        // make the ending imported texture transitions
        if (importedResourcesCount > 0)
        {
            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n        // Make sure imported resources are put back in the state they were given to us in"
                "\n        {"
                "\n            int barrierCount = 0;"
                "\n            D3D12_RESOURCE_BARRIER barriers[" << importedResourcesCount << "];"
            ;

            for (RenderGraphNode& node : renderGraph.nodes)
            {
                if (!GetNodeIsResourceNode(node) || GetNodeResourceVisibility(node) != ResourceVisibility::Imported || !ResourceNodeIsUsed(node))
                    continue;

                const char* resourcePrefix = "";
                switch (node._index)
                {
                    case RenderGraphNode::c_index_resourceBuffer: resourcePrefix = "buffer_"; break;
                    case RenderGraphNode::c_index_resourceTexture: resourcePrefix = "texture_"; break;
                    default: Assert(false, "Unhandled resource node type"); break;
                }

                stringReplacementMap["/*$(Execute)*/"] <<
                    "\n"
                    "\n            if(context->m_input." << resourcePrefix << GetNodeName(node) << "_state != " << ShaderResourceTypeToDX12ResourceState(GetResourceNodeFinalState(node)) << ")"
                    "\n            {"
                    "\n                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;"
                    "\n                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;"
                    "\n                barriers[barrierCount].Transition.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(node)) << resourcePrefix << GetNodeName(node) << ";"
                    "\n                barriers[barrierCount].Transition.StateBefore = " << ShaderResourceTypeToDX12ResourceState(GetResourceNodeFinalState(node)) << ";"
                    "\n                barriers[barrierCount].Transition.StateAfter = context->m_input." << resourcePrefix << GetNodeName(node) << "_state;"
                    "\n                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;"
                    "\n                barrierCount++;"
                    "\n            }"
                ;

                if (GetResourceNodeFinalState(node) == ShaderResourceAccessType::UAV)
                {
                    stringReplacementMap["/*$(Execute)*/"] <<
                        "\n            else"
                        "\n            {"
                        "\n                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;"
                        "\n                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;"
                        "\n                barriers[barrierCount].UAV.pResource = context->" << GetResourceNodePathInContext(GetNodeResourceVisibility(node)) << resourcePrefix << GetNodeName(node) << ";"
                        "\n                barrierCount++;"
                        "\n            }";
                }
            }

            stringReplacementMap["/*$(Execute)*/"] <<
                "\n"
                "\n            if(barrierCount > 0)"
                "\n                commandList->ResourceBarrier(barrierCount, barriers);"
                "\n        }"
            ;
        }

        stringReplacementMap["/*$(Execute)*/"] <<
            "\n"
            "\n        if(context->m_profile)"
            "\n        {"
            "\n            context->m_profileData[(s_timerIndex-1)/2].m_label = \"Total\";"
            "\n            context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPUTechnique).count();"
            "\n            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);"
            "\n            commandList->ResolveQueryData(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, s_timerIndex, context->m_internal.m_TimestampReadbackBuffer, 0);"
            "\n        }"
            ;

        stringReplacementMap["/*$(ReadbackProfileData)*/"] <<
            "\n        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+1] - timeStampBuffer[0])); numItems++; // GPU total";

        if (renderGraph.usesRaytracing)
        {
            stringReplacementMap["/*$(CreateShared)*/"] <<
                "\n"
                "\n        dxrDevice->Release();";
        }
    }
};

void CopyShaderFileDX12(const Shader& shader, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const char* outFolder, const RenderGraph& renderGraph)
{
    std::unordered_map<std::string, std::ostringstream> shaderSpecificStringReplacementMap;

    // Handle variables
    for (const Variable& variable: renderGraph.variables)
    {
        if (variable.scope != shader.scope)
            continue;

        std::string key = "/*$(Variable:" + variable.originalName + ")*/";
        shaderSpecificStringReplacementMap[key] = std::ostringstream();
        if (variable.Const)
            shaderSpecificStringReplacementMap[key] << "(" + variable.dflt + ")";
        else
            shaderSpecificStringReplacementMap[key] << "_" + shader.name + "CB." + variable.name;
    }

    // Handle replaced variables
    for (const VariableReplacement& replacement : renderGraph.variableReplacements)
    {
        if (replacement.srcScope != shader.scope)
            continue;

        int variableIndex = GetScopedVariableIndex(renderGraph, replacement.destName.c_str());
        if (variableIndex == -1)
        {
            Assert(false, "Could not find variable %s that replaced variable %s%s", replacement.destName.c_str(), replacement.srcScope.c_str(), replacement.srcName.c_str());
            return;
        }

        const Variable& variable = renderGraph.variables[variableIndex];

        std::string key = "/*$(Variable:" + replacement.srcName + ")*/";
        shaderSpecificStringReplacementMap[key] = std::ostringstream();
        if (variable.Const)
            shaderSpecificStringReplacementMap[key] << "(" + variable.dflt + ")";
        else
            shaderSpecificStringReplacementMap[key] << "_" + shader.name + "CB." + variable.name;
    }

    std::string srcFileName = (std::filesystem::path(renderGraph.baseDirectory) / shader.fileName).string();
    std::vector<unsigned char> shaderFile;
    if (!LoadFile(srcFileName, shaderFile))
    {
        Assert(false, "Could not load file %s", srcFileName.c_str());
        return;
    }
    shaderFile.push_back(0);

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

            int structIndex = -1;

            switch (resource.access)
            {
                case ShaderResourceAccessType::CBV: structIndex = resource.constantBufferStructIndex; break;
                default:
                {
                    if (resource.type == ShaderResourceType::Buffer)
                        structIndex = resource.buffer.typeStruct.structIndex;
                    break;
                }
            }

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
                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                    "\n    " << DataFieldTypeToShaderType(field.type) << " " << field.name << ";"
                    ;
            }

            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << "\n};";
        }
        shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << "\n";
    }

    // Write out samplers
    for (size_t samplerIndex = 0; samplerIndex < shader.samplers.size(); ++samplerIndex)
    {
        const ShaderSampler& sampler = shader.samplers[samplerIndex];

        std::string spaceString = "";
        if (shader.type == ShaderType::Pixel)
            spaceString = ", space1";

        shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
            "\nSamplerState " << sampler.name << " : register(s" << samplerIndex << spaceString << ");";
    }

    // write out resource declarations
    for (const ShaderResource& resource : shader.resources)
    {
        switch (resource.access)
        {
            case ShaderResourceAccessType::RTScene:
            {
                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << "\nRaytracingAccelerationStructure " << resource.name << " : register(t" << resource.registerIndex << ");";
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
                            default:
                            {
                                Assert(false, "Unhandled TextureDimensionType: %s (%i)", EnumToString(resource.texture.dimension), (int)resource.texture.dimension);
                            }
                        }

                        DataFieldType viewDataFieldType;
                        if (!EnumToEnum(resource.texture.viewType, viewDataFieldType))
                        {
                            Assert(false, "Could not convert TextureViewType to DataFieldType");
                        }

                        shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                            "\n" << variablePrefix << typePrefix << textureType << "<" << BackendDX12::DataFieldTypeToCPPType(viewDataFieldType) << "> " << resource.name << " : register(" << registerType << resource.registerIndex << ");";
                        break;
                    }
                    case ShaderResourceType::Buffer:
                    {
						const char* variablePrefix = (resource.access == ShaderResourceAccessType::UAV && resource.buffer.globallyCoherent) ? "globallycoherent " : "";

                        if (resource.buffer.raw)
                        {
                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "\n" << variablePrefix << typePrefix << "ByteAddressBuffer " << resource.name << " : register(" << registerType << resource.registerIndex << ");"
                                ;
                        }
                        else if (resource.buffer.typeStruct.structIndex != -1)
                        {
                            shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                "\n" << variablePrefix << typePrefix << "StructuredBuffer<Struct_" << renderGraph.structs[resource.buffer.typeStruct.structIndex].name << "> " << resource.name << " : register(" << registerType << resource.registerIndex << ");"
                                ;
                        }
                        else
                        {
                            if (DataFieldTypeIsPOD(resource.buffer.type) && !resource.buffer.PODAsStructuredBuffer)
                            {
                                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                    "\n" << variablePrefix << typePrefix << "Buffer<" << DataFieldTypeToShaderType(resource.buffer.type) << "> " << resource.name << " : register(" << registerType << resource.registerIndex << ");";
                            }
                            else
                            {
                                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                                    "\n" << variablePrefix << typePrefix << "StructuredBuffer<" << DataFieldTypeToShaderType(resource.buffer.type) << "> " << resource.name << " : register(" << registerType << resource.registerIndex << ");";
                            }
                        }
                        break;
                    }
                    default:
                    {
                        Assert(false, "Unhandled resource type: %i (%s) in shader %s", resource.type, EnumToString(resource.type), shader.originalName.c_str());
                        break;
                    }
                }
                break;
            }
            case ShaderResourceAccessType::CBV:
            {
                shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] <<
                    "\nConstantBuffer<Struct_" << renderGraph.structs[resource.constantBufferStructIndex].name << "> " << resource.name << " : register(b" << resource.registerIndex << ");"
                ;
                break;
            }
            default:
            {
                Assert(false, "Unhandled resource access type: %i", resource.access);
                break;
            }
        }
    }
    shaderSpecificStringReplacementMap["/*$(ShaderResources)*/"] << "\n";

    // Handle string replacement for any references to loaded textures
    for (const LoadedTextureReference& loadedTexture : shader.loadedTextureRefs)
        shaderSpecificStringReplacementMap[loadedTexture.token] << loadedTexture.resourceName;

    // Handle shader markup
    shaderSpecificStringReplacementMap["/*$(RayTraceFn)*/"] << "TraceRay";
    std::string shaderFileContents = (char*)shaderFile.data();
    ForEachToken(shaderFileContents.c_str(),
        [&](const std::string& token, const char* stringStart, const char* cursor)
        {
            size_t lineNumber = CountLineNumber(stringStart, cursor);

            std::string param;
            if (token == "/*$(ShaderResources)*/")
            {
                std::string old = shaderSpecificStringReplacementMap[token].str();
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    old << "\n" <<
                    "#line " << lineNumber << "\n"
                    ;
            }
            else if (GetTokenParameter(token.c_str(), "_compute", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[numthreads(" << shader.NumThreads[0] << ", " << shader.NumThreads[1] << ", " << shader.NumThreads[2] << ")]\n"
                    "#line " << lineNumber << "\n"
                    "void " << param;
            }
            else if (GetTokenParameter(token.c_str(), "_amplification", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[numthreads(" << shader.NumThreads[0] << ", " << shader.NumThreads[1] << ", " << shader.NumThreads[2] << ")]\n"
                    "#line " << lineNumber << "\n"
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
                    "#line " << lineNumber << "\n"
                    "void " << entryPoint;
            }
            else if (GetTokenParameter(token.c_str(), "_raygeneration", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"raygeneration\")]\n"
                    "#line " << lineNumber << "\n"
                    "void " << param << "()";
            }
            else if (GetTokenParameter(token.c_str(), "_miss", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"miss\")]\n"
                    "#line " << lineNumber << "\n"
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
                    "#line " << lineNumber << "\n"
                    "void " << entryPoint << "(inout Payload payload, in " << attribStruct << " attr)";
            }
            else if (GetTokenParameter(token.c_str(), "_intersection", param))
            {
                shaderSpecificStringReplacementMap[token] = std::ostringstream();
                shaderSpecificStringReplacementMap[token] <<
                    "[shader(\"intersection\")]\n"
                    "#line " << lineNumber << "\n"
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
                    "#line " << lineNumber << "\n"
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

                Assert(foundIndex != -1, "Could not find RTHitGroupIndex for \"%s\" in shader \"%s\"", param.c_str(), shader.name.c_str());
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

                Assert(foundIndex != -1, "Could not find RTMissIndex for \"%s\" in shader \"%s\"", param.c_str(), shader.name.c_str());
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
        }
    );

    for (const TokenReplacement& replacement : shader.tokenReplacements)
        shaderSpecificStringReplacementMap[replacement.name] << replacement.value;

    // Replace the strings
    ProcessStringReplacement(shaderFileContents, shaderSpecificStringReplacementMap, stringReplacementMap, renderGraph);

    // Process with slang, if we are supposed to
    if (shader.slangOptions.process)
    {
        const char* shaderModel = nullptr;
        const char* stage = nullptr;
        switch (shader.type)
        {
            case ShaderType::Compute: shaderModel = renderGraph.settings.dx12.shaderModelCs.c_str(); stage = "compute"; break;
            case ShaderType::Vertex: shaderModel = renderGraph.settings.dx12.shaderModelVs.c_str(); stage = "vertex"; break;
            case ShaderType::Pixel: shaderModel = renderGraph.settings.dx12.shaderModelPs.c_str(); stage = "fragment"; break;
        }

        std::string workingDirectory = (std::filesystem::path(outFolder) / "shaders" / "").string();
        std::string slangErrorMessage;
        if (!ProcessWithSlang(shaderFileContents, shader.fileName.c_str(), stage, shader.entryPoint.c_str(), shaderModel, slangErrorMessage, workingDirectory.c_str()))
        {
            ShowErrorMessage("Slang:%s\n%s\n", shader.fileName.c_str(), slangErrorMessage.c_str());
        }
        else if (!slangErrorMessage.empty())
        {
            ShowWarningMessage("Slang:%s\n%s\n", shader.fileName.c_str(), slangErrorMessage.c_str());
        }
    }

    // Write the shader file out
    std::string fullFileName = (std::filesystem::path(outFolder) / "shaders" / shader.destFileName).string();
    WriteFileIfDifferent(fullFileName, shaderFileContents);
}

void RunBackend_DX12(GigiBuildFlavor buildFlavor, RenderGraph& renderGraph)
{
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
    {
        // Module files are always present
        ProcessTemplateFolder(renderGraph, files, outFolder, "./GigiCompilerLib/Backends/DX12/templates/Module/");

        // Other flavors add application code that calls into the module code
        switch (buildFlavor)
        {
            case GigiBuildFlavor::DX12_Module: break;
            case GigiBuildFlavor::DX12_Application:
            {
                ProcessTemplateFolder(renderGraph, files, outFolder, "./GigiCompilerLib/Backends/DX12/templates/Application/");
                break;
            }
            default:
            {
                Assert(false, "Unhandled build flavor: %s", EnumToString(buildFlavor));
                return;
            }
        }
    }

    // Make the files
    std::unordered_map<std::string, std::ostringstream> stringReplacementMap = MakeFiles<BackendDX12>(files, renderGraph);

    // string replacement to help the custom functions
    {
        // parameters
        stringReplacementMap["/*$(CustomFunctionParams)*/"] << "Context* context";
        stringReplacementMap["/*$(CustomFunctionFileHeader)*/"] << "#include \"../public/technique.h\"\n\nnamespace " << renderGraph.name << "\n{";
        stringReplacementMap["/*$(CustomFunctionFileFooter)*/"] << "}\n";

        // for variable usage
        for (const Variable& variable : renderGraph.variables)
        {
            std::string key = "/*$(Variable:" + variable.name + ")*/";
            stringReplacementMap[key] << BackendDX12::VariableToString(variable, renderGraph);
        }

        // size of resources
        for (const RenderGraphNode& node : renderGraph.nodes)
        {
            if (!GetNodeIsResourceNode(node) || node._index == RenderGraphNode::c_index_resourceShaderConstants)
                continue;

            std::string key = "/*$(ResourceSize:" + GetNodeName(node) + ")*/";
            stringReplacementMap[key] << BackendDX12::ResourceToString(node) << "_size";
        }
    }

    // copy any file copies that should happen
    for (const FileCopy& fileCopy : renderGraph.fileCopies)
    {
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

    // Copy the shader files
    for (const Shader& shader : renderGraph.shaders)
    {
        if (shader.copyFile)
            CopyShaderFileDX12(shader, stringReplacementMap, outFolder, renderGraph);
    }
}

void PostLoad_DX12(RenderGraph& renderGraph)
{
}
