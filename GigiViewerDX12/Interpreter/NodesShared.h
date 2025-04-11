///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include <d3d12.h>
#include <stdint.h>
#include <array>
// clang-format on

typedef std::array<int, 3> IVec3;

inline bool ShadingRateToD3D12_SHADING_RATE(ShadingRate shadingRate, D3D12_SHADING_RATE& d3d12ShadingRate)
{
	switch (shadingRate)
	{
		case ShadingRate::_1x1: d3d12ShadingRate = D3D12_SHADING_RATE_1X1; return true;
		case ShadingRate::_1x2: d3d12ShadingRate = D3D12_SHADING_RATE_1X2; return true;
		case ShadingRate::_2x1: d3d12ShadingRate = D3D12_SHADING_RATE_2X1; return true;
		case ShadingRate::_2x2: d3d12ShadingRate = D3D12_SHADING_RATE_2X2; return true;
		case ShadingRate::_2x4: d3d12ShadingRate = D3D12_SHADING_RATE_2X4; return true;
		case ShadingRate::_4x2: d3d12ShadingRate = D3D12_SHADING_RATE_4X2; return true;
		case ShadingRate::_4x4: d3d12ShadingRate = D3D12_SHADING_RATE_4X4; return true;
		default: return false;
	}
	return false;
}

inline bool ShadingRateCombinerToD3D12_SHADING_RATE_COMBINER(ShadingRateCombiner shadingRateCombiner, D3D12_SHADING_RATE_COMBINER& d3d12ShadingRateCombiner)
{
	switch (shadingRateCombiner)
	{
		case ShadingRateCombiner::PassThrough: d3d12ShadingRateCombiner = D3D12_SHADING_RATE_COMBINER_PASSTHROUGH; return true;
		case ShadingRateCombiner::Override: d3d12ShadingRateCombiner = D3D12_SHADING_RATE_COMBINER_OVERRIDE; return true;
		case ShadingRateCombiner::Min: d3d12ShadingRateCombiner = D3D12_SHADING_RATE_COMBINER_MIN; return true;
		case ShadingRateCombiner::Max: d3d12ShadingRateCombiner = D3D12_SHADING_RATE_COMBINER_MAX; return true;
		case ShadingRateCombiner::Sum: d3d12ShadingRateCombiner = D3D12_SHADING_RATE_COMBINER_SUM; return true;
		default: return false;
	}
	return false;
}

inline bool SamplerFilterToD3D12Filter(SamplerFilter filter, D3D12_FILTER& d3d12Filter)
{
	switch (filter)
	{
		case SamplerFilter::MinMagMipPoint: d3d12Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; return true;
		case SamplerFilter::MinMagLinear_MipPoint: d3d12Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT; return true;
		case SamplerFilter::MinMagMipLinear: d3d12Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; return true;
		default: return false;
	}
	return false;
}

inline bool SamplerAddressModeToD3D12AddressMode(SamplerAddressMode mode, D3D12_TEXTURE_ADDRESS_MODE& d3d12Mode)
{
	switch (mode)
	{
		case SamplerAddressMode::Clamp: d3d12Mode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP; return true;
		case SamplerAddressMode::Wrap: d3d12Mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP; return true;
		case SamplerAddressMode::Border: d3d12Mode = D3D12_TEXTURE_ADDRESS_MODE_BORDER; return true;
		default: return false;
	}
	return false;
}

inline DXGI_FORMAT TextureFormatToDXGI_FORMAT(TextureFormat textureFormat)
{
	switch (textureFormat)
	{
		case TextureFormat::Any: return DXGI_FORMAT_UNKNOWN;
		case TextureFormat::R8_Unorm: return DXGI_FORMAT_R8_UNORM;
		case TextureFormat::RG8_Unorm: return DXGI_FORMAT_R8G8_UNORM;
		case TextureFormat::RGBA8_Unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA8_Unorm_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case TextureFormat::BGRA8_Unorm: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::R8_Snorm: return DXGI_FORMAT_R8_SNORM;
		case TextureFormat::RG8_Snorm: return DXGI_FORMAT_R8G8_SNORM;
		case TextureFormat::RGBA8_Snorm: return DXGI_FORMAT_R8G8B8A8_SNORM;
		case TextureFormat::R8_Uint: return DXGI_FORMAT_R8_UINT;
		case TextureFormat::RG8_Uint: return DXGI_FORMAT_R8G8_UINT;
		case TextureFormat::RGBA8_Uint: return DXGI_FORMAT_R8G8B8A8_UINT;
		case TextureFormat::R8_Sint: return DXGI_FORMAT_R8_SINT;
		case TextureFormat::RG8_Sint: return DXGI_FORMAT_R8G8_SINT;
		case TextureFormat::RGBA8_Sint: return DXGI_FORMAT_R8G8B8A8_SINT;
		case TextureFormat::R16_Float: return DXGI_FORMAT_R16_FLOAT;
		case TextureFormat::RG16_Float: return DXGI_FORMAT_R16G16_FLOAT;
		case TextureFormat::RGBA16_Float: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TextureFormat::RGBA16_Unorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
		case TextureFormat::RGBA16_Snorm: return DXGI_FORMAT_R16G16B16A16_SNORM;
		case TextureFormat::RG16_Uint: return DXGI_FORMAT_R16G16_UINT;
		case TextureFormat::R32_Float: return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::RG32_Float: return DXGI_FORMAT_R32G32_FLOAT;
		case TextureFormat::RGBA32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TextureFormat::R32_Uint: return DXGI_FORMAT_R32_UINT;
		case TextureFormat::RG32_Uint: return DXGI_FORMAT_R32G32_UINT;
		case TextureFormat::RGBA32_Uint: return DXGI_FORMAT_R32G32B32A32_UINT;
		case TextureFormat::R11G11B10_Float: return DXGI_FORMAT_R11G11B10_FLOAT;
		case TextureFormat::D32_Float: return DXGI_FORMAT_D32_FLOAT;
		case TextureFormat::D16_Unorm: return DXGI_FORMAT_D16_UNORM;
		case TextureFormat::D32_Float_S8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case TextureFormat::D24_Unorm_S8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::BC4_Unorm: return DXGI_FORMAT_BC4_UNORM;
		case TextureFormat::BC4_Snorm: return DXGI_FORMAT_BC4_SNORM;
		case TextureFormat::BC5_Unorm: return DXGI_FORMAT_BC5_UNORM;
		case TextureFormat::BC5_Snorm: return DXGI_FORMAT_BC5_SNORM;
		case TextureFormat::BC7_Unorm: return DXGI_FORMAT_BC7_UNORM;
		case TextureFormat::BC7_Unorm_sRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
		case TextureFormat::BC6_UF16: return DXGI_FORMAT_BC6H_UF16;
		case TextureFormat::BC6_SF16: return DXGI_FORMAT_BC6H_SF16;
	}

	Assert(false, "Unhandled TextureFormat");
	return DXGI_FORMAT_FORCE_UINT;
}

// Returns "Any" if no valid format found
inline TextureFormat DXGI_FORMATToTextureFormat(DXGI_FORMAT format)
{
	for (size_t i = 0; i < EnumCount<TextureFormat>(); ++i)
	{
		if (TextureFormatToDXGI_FORMAT((TextureFormat)i) == format)
			return (TextureFormat)i;
	}
	return TextureFormat::Any;
}

inline D3D12_CULL_MODE DrawCullModeToD3D12_CULL_MODE(DrawCullMode cullMode)
{
	switch (cullMode)
	{
		case DrawCullMode::None: return D3D12_CULL_MODE_NONE;
		case DrawCullMode::Front: return D3D12_CULL_MODE_FRONT;
		case DrawCullMode::Back: return D3D12_CULL_MODE_BACK;
		default:
		{
			Assert(false, "Unhandled DrawCullMode");
			return D3D12_CULL_MODE_NONE;
		}
	}
}

inline D3D12_PRIMITIVE_TOPOLOGY GeometryTypeToD3D12_PRIMITIVE_TOPOLOGY(GeometryType geometryType)
{
	switch (geometryType)
	{
		case GeometryType::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case GeometryType::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case GeometryType::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		default:
		{
			Assert(false, "Unhandled GeometryType");
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}
}

inline D3D12_PRIMITIVE_TOPOLOGY_TYPE GeometryTypeToD3D12_PRIMITIVE_TOPOLOGY_TYPE(GeometryType geometryType)
{
	switch (geometryType)
	{
		case GeometryType::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case GeometryType::LineList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case GeometryType::PointList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		default:
		{
			Assert(false, "Unhandled GeometryType");
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}
}

inline D3D12_COMPARISON_FUNC DepthTestFunctionToD3D12_COMPARISON_FUNC(DepthTestFunction function)
{
	switch (function)
	{
		case DepthTestFunction::Never: return D3D12_COMPARISON_FUNC_NEVER;
		case DepthTestFunction::Less: return D3D12_COMPARISON_FUNC_LESS;
		case DepthTestFunction::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
		case DepthTestFunction::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case DepthTestFunction::Greater: return D3D12_COMPARISON_FUNC_GREATER;
		case DepthTestFunction::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case DepthTestFunction::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case DepthTestFunction::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
		default:
		{
			Assert(false, "Unhandled DepthTestFunction");
			return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}
}

inline D3D12_BLEND DrawBlendModeToD3D12_BLEND(DrawBlendMode mode)
{
	switch (mode)
	{
		case DrawBlendMode::Zero: return D3D12_BLEND_ZERO;
		case DrawBlendMode::One: return D3D12_BLEND_ONE;
		case DrawBlendMode::SrcColor: return D3D12_BLEND_SRC_COLOR;
		case DrawBlendMode::InvSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
		case DrawBlendMode::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
		case DrawBlendMode::InvSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
		case DrawBlendMode::DestAlpha: return D3D12_BLEND_DEST_ALPHA;
		case DrawBlendMode::InvDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
		case DrawBlendMode::DestColor: return D3D12_BLEND_DEST_COLOR;
		case DrawBlendMode::InvDestColor: return D3D12_BLEND_INV_DEST_COLOR;
		default:
		{
			Assert(false, "Unhandled DrawBlendMode");
			return D3D12_BLEND_ONE;
		}
	}
}

struct DataFieldTypeInfoStructDX12
{
	DataFieldTypeInfoStructDX12(DataFieldComponentType type, int bytes, int count, DXGI_FORMAT _componentFormat, DXGI_FORMAT _typeFormat, int _typeFormatCount)
	{
		componentType = type;
		componentBytes = bytes;
		componentCount = count;

		componentFormat = _componentFormat;
		typeFormat = _typeFormat;
		typeFormatCount = _typeFormatCount;

		typeBytes = bytes * count;
	}

	DataFieldComponentType componentType = DataFieldComponentType::_int;
	int componentBytes = sizeof(int);
	int componentCount = 0;

	// A DXGI_FORMAT that would express a single channel value
	DXGI_FORMAT componentFormat = DXGI_FORMAT_FORCE_UINT;

	// A DXGI_FORMAT that would express an entire value.
	// Some types require multiple entries though, such as a float4x4 requiring 4 float4s, so typeFormatCount would be 4 in that case.
	DXGI_FORMAT typeFormat = DXGI_FORMAT_FORCE_UINT;
	int typeFormatCount = 1;

	int typeBytes = 0;
};

#define DATA_FIELD_TYPE_INFO_DX12(type, count, componentFormat, typeFormat, typeFormatCount) DataFieldTypeInfoStructDX12(DataFieldComponentType::_##type, (int)sizeof(type), count, componentFormat, typeFormat, typeFormatCount)

inline DataFieldTypeInfoStructDX12 DataFieldTypeInfoDX12(DataFieldType type)
{
    switch(type)
    {
        case DataFieldType::Int: return DATA_FIELD_TYPE_INFO_DX12(int, 1, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_SINT, 1);
        case DataFieldType::Int2: return DATA_FIELD_TYPE_INFO_DX12(int, 2, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT, 1);
        case DataFieldType::Int3: return DATA_FIELD_TYPE_INFO_DX12(int, 3, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32B32_SINT, 1);
        case DataFieldType::Int4: return DATA_FIELD_TYPE_INFO_DX12(int, 4, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32B32A32_SINT, 1);
        case DataFieldType::Uint: return DATA_FIELD_TYPE_INFO_DX12(uint32_t, 1, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, 1);
        case DataFieldType::Uint2: return DATA_FIELD_TYPE_INFO_DX12(uint32_t, 2, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, 1);
        case DataFieldType::Uint3: return DATA_FIELD_TYPE_INFO_DX12(uint32_t, 3, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32B32_UINT, 1);
        case DataFieldType::Uint4: return DATA_FIELD_TYPE_INFO_DX12(uint32_t, 4, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32B32A32_UINT, 1);
        case DataFieldType::Float: return DATA_FIELD_TYPE_INFO_DX12(float, 1, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, 1);
        case DataFieldType::Float2: return DATA_FIELD_TYPE_INFO_DX12(float, 2, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, 1);
        case DataFieldType::Float3: return DATA_FIELD_TYPE_INFO_DX12(float, 3, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, 1);
        case DataFieldType::Float4: return DATA_FIELD_TYPE_INFO_DX12(float, 4, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, 1);
        case DataFieldType::Bool: return DATA_FIELD_TYPE_INFO_DX12(uint32_t, 1, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_UINT, 1);
        case DataFieldType::Float4x4: return DATA_FIELD_TYPE_INFO_DX12(float, 16, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, 16);
        case DataFieldType::Uint_16: return DATA_FIELD_TYPE_INFO_DX12(uint16_t, 1, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16_UINT, 1);
        case DataFieldType::Int_64: return DATA_FIELD_TYPE_INFO_DX12(int64_t, 1, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_SINT, 1);
        case DataFieldType::Uint_64: return DATA_FIELD_TYPE_INFO_DX12(uint64_t, 1, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_UINT, 1);
        default:
        {
            Assert(false, "Unknown data field type: %i", type);
			return DATA_FIELD_TYPE_INFO_DX12(int, 0, DXGI_FORMAT_FORCE_UINT, DXGI_FORMAT_FORCE_UINT, 0);
        }
    }
}

inline DXGI_FORMAT GetDesiredFormat(const GigiInterpreterPreviewWindowDX12& interpreter, const RenderGraphNode_Resource_Texture& node, int rootNodeId = -1)
{
	// protect against dependency loops
	if (rootNodeId == -1)
		rootNodeId = node.nodeIndex;
	else if (rootNodeId == node.nodeIndex)
		return DXGI_FORMAT_FORCE_UINT;

	// Get desired format
	if (node.format.variable.variableIndex != -1)
	{
		// Note: the variable is meant to hold the host API's enum value for format
		GigiInterpreterPreviewWindowDX12::RuntimeVariable rtVar = interpreter.GetRuntimeVariable(node.format.variable.variableIndex);
		switch (rtVar.variable->type)
		{
			case DataFieldType::Int: return static_cast<DXGI_FORMAT>(*(int*)rtVar.storage.value);
			default: return DXGI_FORMAT_FORCE_UINT;
		}
	}
	else if (node.format.node.textureNode)
		return GetDesiredFormat(interpreter, *node.format.node.textureNode, rootNodeId);
	else
	{
		if (node.format.format != TextureFormat::Any)
			return TextureFormatToDXGI_FORMAT(node.format.format);

		bool exists = true;
		DXGI_FORMAT ret = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(node.name.c_str(), exists).m_format;
		if (exists)
			return ret;
		else
			return DXGI_FORMAT_FORCE_UINT;
	}
}

struct BufferFormatInfo
{
	BufferFormatInfo() {}

	BufferFormatInfo(DXGI_FORMAT _format, int _formatCount, int _structIndex)
	{
		format = _format;
		formatCount = _formatCount;
		structIndex = _structIndex;
	}

	BufferFormatInfo(DXGI_FORMAT _format, int _formatCount)
	{
		format = _format;
		formatCount = _formatCount;
	}

	BufferFormatInfo(int _structIndex)
	{
		structIndex = _structIndex;
	}

	DXGI_FORMAT format = DXGI_FORMAT_FORCE_UINT;
	int formatCount = 0;

	int structIndex = -1;
};

inline BufferFormatInfo GetBufferFormatInfo(const GigiInterpreterPreviewWindowDX12& interpreter, const RenderGraphNode_Resource_Buffer& node, int rootNodeId = -1)
{
	// If this is an imported buffer, the format is whatever the format has been set to
	if (node.visibility == ResourceVisibility::Imported)
	{
		const RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(node.name.c_str());
		return BufferFormatInfo(runtimeData.m_format, runtimeData.m_formatCount, runtimeData.m_structIndex);
	}

	// protect against dependency loops
	if (rootNodeId == -1)
		rootNodeId = node.nodeIndex;
	else if (rootNodeId == node.nodeIndex)
		return BufferFormatInfo();

	// Get desired format
	if (node.format.node.bufferNode)
		return GetBufferFormatInfo(interpreter, *node.format.node.bufferNode, rootNodeId);

	if (node.format.type == DataFieldType::Count)
	{
		return BufferFormatInfo(DXGI_FORMAT_UNKNOWN, 1, node.format.structureType.structIndex);
	}
	else
	{
		DataFieldTypeInfoStructDX12 typeInfo = DataFieldTypeInfoDX12(node.format.type);
		return BufferFormatInfo(typeInfo.typeFormat, typeInfo.typeFormatCount, node.format.structureType.structIndex);
	}
}

inline IVec3 GetDesiredSize(const GigiInterpreterPreviewWindowDX12& interpreter, const RenderGraphNode_Resource_Texture& node, int rootNodeId = -1)
{
	// If this is an imported texture, the size is whatever it has been set to
	IVec3 ret = { 1, 1, 1 };
	if (node.visibility == ResourceVisibility::Imported)
	{
		const int* v = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(node.name.c_str()).m_size;
		ret[0] = (int)v[0];
		ret[1] = (int)v[1];
		ret[2] = (int)v[2];
		return ret;
	}

	// protect against dependency loops
	if (rootNodeId == -1)
		rootNodeId = node.nodeIndex;
	else if (rootNodeId == node.nodeIndex)
		return IVec3{ 0, 0, 0 };

	// Get desired size

	if (node.size.variable.variableIndex != -1)
	{
		GigiInterpreterPreviewWindowDX12::RuntimeVariable rtVar = interpreter.GetRuntimeVariable(node.size.variable.variableIndex);

		DataFieldTypeInfoStruct varTypeInfo = DataFieldTypeInfo(rtVar.variable->type);

		int componentCount = varTypeInfo.componentCount;
		if (componentCount > 3)
			componentCount = 3;

		switch(varTypeInfo.componentType)
		{
			case DataFieldComponentType::_int:
			{
				for (int i = 0; i < componentCount; ++i)
					ret[i] = (int)((int*)rtVar.storage.value)[i];
				break;
			}
			case DataFieldComponentType::_uint16_t:
			{
				for (int i = 0; i < componentCount; ++i)
					ret[i] = (int)((uint16_t*)rtVar.storage.value)[i];
				break;
			}
			case DataFieldComponentType::_uint32_t:
			{
				for (int i = 0; i < componentCount; ++i)
					ret[i] = (int)((uint32_t*)rtVar.storage.value)[i];
				break;
			}
			case DataFieldComponentType::_float:
			{
				for (int i = 0; i < componentCount; ++i)
					ret[i] = (int)((float*)rtVar.storage.value)[i];
				break;
			}
		}
	}
	else if (node.size.node.textureNode)
	{
		ret = GetDesiredSize(interpreter, *node.size.node.textureNode, rootNodeId);
	}

	// apply the fixed function formula
	for (int i = 0; i < 3; ++i)
		ret[i] = (((ret[i] + node.size.preAdd[i]) * node.size.multiply[i]) / node.size.divide[i]) + node.size.postAdd[i];

	// Cube maps always want to be size 6 on z
	if (node.dimension == TextureDimensionType::TextureCube)
		ret[2] = 6;

	// return the final size;
	return ret;
}

inline int GetDesiredCount(const GigiInterpreterPreviewWindowDX12& interpreter, const RenderGraphNode_Resource_Buffer& node, int rootNodeId = -1)
{
	// If this is an imported buffer, the count is whatever it has been set to
	if (node.visibility == ResourceVisibility::Imported)
	{
		const RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(node.name.c_str());
		return runtimeData.m_count;
	}

	// protect against dependency loops
	if (rootNodeId == -1)
		rootNodeId = node.nodeIndex;
	else if (rootNodeId == node.nodeIndex)
		return 0;

	// Get desired size
	int ret = 1;
	if (node.count.node.bufferNode)
		ret = GetDesiredCount(interpreter, *node.count.node.bufferNode, rootNodeId);
	else if (node.count.variable.variableIndex != -1)
	{
		const RenderGraph& renderGraph = interpreter.GetRenderGraph();
		const GigiInterpreterPreviewWindowDX12::RuntimeVariable& rtVar = interpreter.GetRuntimeVariable(node.count.variable.variableIndex);

		DataFieldTypeInfoStruct varTypeInfo = DataFieldTypeInfo(rtVar.variable->type);

		switch(varTypeInfo.componentType)
		{
			case DataFieldComponentType::_int: ret = *(int*)rtVar.storage.value; break;
			case DataFieldComponentType::_uint16_t: ret = int(*(uint16_t*)rtVar.storage.value); break;
			case DataFieldComponentType::_uint32_t: ret = int(*(uint32_t*)rtVar.storage.value); break;
			case DataFieldComponentType::_float: ret = int(*(float*)rtVar.storage.value); break;
		}
	}

	// apply the fixed function formula
	ret = (((ret + node.count.preAdd) * node.count.multiply) / node.count.divide) + node.count.postAdd;

	// return the final count;
	return ret;
}

inline D3D12_RESOURCE_FLAGS ShaderResourceAccessToD3D12_RESOURCE_FLAGs(unsigned int accessedAs)
{
	D3D12_RESOURCE_FLAGS ret = D3D12_RESOURCE_FLAG_NONE;

	if (accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV))
		ret |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	if (accessedAs & (1 << (unsigned int)ShaderResourceAccessType::RenderTarget))
		ret |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	if (accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
		ret |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	return ret;
}

inline ResourceType TextureDimensionTypeToResourceType(TextureDimensionType type)
{
	switch (type)
	{
		case TextureDimensionType::Texture2D: return ResourceType::Texture2D;
		case TextureDimensionType::Texture2DArray: return ResourceType::Texture2DArray;
		case TextureDimensionType::Texture3D: return ResourceType::Texture3D;
		case TextureDimensionType::TextureCube: return ResourceType::TextureCube;
	}
	return ResourceType::Texture2D;
}
