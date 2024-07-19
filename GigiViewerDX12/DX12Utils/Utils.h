///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <f16.h>

struct half
{
	uint16_t data;

	half(double d)
	{
		data = f32tof16((float)d);
	}

	operator double() const
	{
		return double(float(*this));
	}

	operator float() const
	{
		return half_to_float_fast5(data);
	}
};

struct DXGI_FORMAT_Info
{
	enum class NormType
	{
		None,
		UNorm,
		SNorm,
	};

	enum class ChannelType
	{
		_uint8_t,
		_uint16_t,
		_uint32_t,

		_int8_t,
		_int16_t,
		_int32_t,

		_half,
		_float,
	};

	DXGI_FORMAT_Info() {}

	DXGI_FORMAT_Info(DXGI_FORMAT _format, const char* _name, int _bytesPerChannel, int _channelCount, ChannelType _channelType, bool _sRGB, bool _isStencil, bool _isDepth, int _planeIndex, int _planeCount, NormType _normType, bool _isCompressed)
	{
		format = _format;
		name = _name;
		bytesPerChannel = _bytesPerChannel;
		channelCount = _channelCount;
		bytesPerPixel = channelCount * bytesPerChannel;
		channelType = _channelType;
		sRGB = _sRGB;
		isStencil = _isStencil;
		isDepth = _isDepth;
		planeIndex = _planeIndex;
		planeCount = _planeCount;
		normType = _normType;
		isCompressed = _isCompressed;
	}

	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	const char* name = nullptr;
	int bytesPerChannel = 0;
	int channelCount = 0;
	int bytesPerPixel = 0;
	ChannelType channelType = ChannelType::_uint8_t;
	bool sRGB = false;
	bool isStencil = false;
	bool isDepth = false;
	int planeIndex = 0;
	int planeCount = 1;
	NormType normType = NormType::None;
	bool isCompressed = false;
};

#define DXGI_FORMAT_INFO(type, channelCount, sRGB) DXGI_FORMAT_Info(DXGI_FORMAT_UNKNOWN, "", sizeof(type), channelCount, DXGI_FORMAT_Info::ChannelType::_##type, sRGB, false, false, 0, 1, DXGI_FORMAT_Info::NormType::None, false)
#define DXGI_FORMAT_INFO_CASE(name, type, channelCount, sRGB, isStencil, isDepth, planeIndex, planeCount, normType, isCompressed) case name: return DXGI_FORMAT_Info(name, #name, sizeof(type), channelCount, DXGI_FORMAT_Info::ChannelType::_##type, sRGB, isStencil, isDepth, planeIndex, planeCount, DXGI_FORMAT_Info::NormType::##normType, isCompressed);

inline DXGI_FORMAT_Info Get_DXGI_FORMAT_Info(DXGI_FORMAT format)
{
	switch (format)
	{
		// Unsigned integers
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_UNORM, uint8_t, 1, false, false, false, 0, 1, UNorm, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_UNORM, uint8_t, 2, false, false, false, 0, 1, UNorm, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_UNORM, uint8_t, 4, false, false, false, 0, 1, UNorm, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, uint8_t, 4, true, false, false, 0, 1, UNorm, false);

		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_UINT, uint8_t, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_UINT, uint8_t, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_UINT, uint8_t, 4, false, false, false, 0, 1, None, false);

		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_UINT, uint16_t, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_UNORM, uint16_t, 1, false, false, false, 0, 1, UNorm, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16_UINT, uint16_t, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_UINT, uint16_t, 4, false, false, false, 0, 1, None, false);

		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_UINT, uint32_t, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32_UINT, uint32_t, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32_UINT, uint32_t, 3, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32A32_UINT, uint32_t, 4, false, false, false, 0, 1, None, false);

		// Signed integers
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_SNORM, int8_t, 1, false, false, false, 0, 1, SNorm, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_SNORM, int8_t, 2, false, false, false, 0, 1, SNorm, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_SNORM, int8_t, 4, false, false, false, 0, 1, SNorm, false);

		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8_SINT, int8_t, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8_SINT, int8_t, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R8G8B8A8_SINT, int8_t, 4, false, false, false, 0, 1, None, false);

		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_SINT, int16_t, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16_SINT, int16_t, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_SINT, int16_t, 4, false, false, false, 0, 1, None, false);

		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_SINT, int32_t, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32_SINT, int32_t, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32_SINT, int32_t, 3, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32A32_SINT, int32_t, 4, false, false, false, 0, 1, None, false);

		// 16 bit
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16_FLOAT, half, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16_FLOAT, half, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_FLOAT, half, 4, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_UNORM, uint16_t, 4, false, false, false, 0, 1, UNorm, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R16G16B16A16_SNORM, int16_t, 4, false, false, false, 0, 1, SNorm, false);

		// 32 bit float
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_FLOAT, float, 1, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32_FLOAT, float, 2, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32_FLOAT, float, 3, false, false, false, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32G32B32A32_FLOAT, float, 4, false, false, false, 0, 1, None, false);

		// Other
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R11G11B10_FLOAT, uint32_t, 1, false, false, false, 0, 1, None, false);  // Treat as 1 x u32

		// Depth
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D32_FLOAT, float, 1, false, false, true, 0, 1, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D16_UNORM, uint16_t, 1, false, false, true, 0, 1, UNorm, false);

		// Depth Stencil
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, uint32_t, 2, false, true, true, 0, 2, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, float, 1, false, true, true, 0, 2, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, uint8_t, 1, false, true, true, 1, 2, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_D24_UNORM_S8_UINT, uint32_t, 1, false, true, true, 0, 2, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, uint32_t, 1, false, true, true, 0, 2, None, false);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_X24_TYPELESS_G8_UINT, uint8_t, 1, false, true, true, 1, 2, None, false);

		// Block compressed formats
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC7_UNORM, uint8_t, 4, false, false, false, 0, 1, UNorm, true);
		DXGI_FORMAT_INFO_CASE(DXGI_FORMAT_BC7_UNORM_SRGB, uint8_t, 4, true, false, false, 0, 1, UNorm, true);

		default:
		{
			Assert(false, "Unhandled DXGI_FORMAT");
			return DXGI_FORMAT_INFO(uint8_t, 0, false);
		}
	}
}

inline bool FormatsCompatibleForCopyResource(DXGI_FORMAT a, DXGI_FORMAT b)
{
	// This function needs to be expanded to handle all cases correctly.  Maybe Get_DXGI_FORMAT_Info() can be used somehow.
	if (a == b)
		return true;

	if (a > b)
	{
		DXGI_FORMAT temp = a;
		a = b;
		b = temp;
	}

	if (a == DXGI_FORMAT_D32_FLOAT && b == DXGI_FORMAT_R32_FLOAT)
		return true;

	return false;
}

inline DXGI_FORMAT SRV_Safe_DXGI_FORMAT(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_D32_FLOAT:return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_D16_UNORM:return DXGI_FORMAT_R16_UNORM;
		default: return format;
	}
}

inline DXGI_FORMAT DSV_Safe_DXGI_FORMAT(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default: return format;
	}
}

inline D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS GGUserFile_TLASBuildFlagsToD3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(GGUserFile_TLASBuildFlags flags)
{
	switch (flags)
	{
		case GGUserFile_TLASBuildFlags::None: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		case GGUserFile_TLASBuildFlags::AllowUpdate: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		case GGUserFile_TLASBuildFlags::AllowCompaction: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
		case GGUserFile_TLASBuildFlags::PreferFastTrace: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		case GGUserFile_TLASBuildFlags::PreferFastBuild: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
		case GGUserFile_TLASBuildFlags::MinimizeMemory: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
		default:
		{
			Assert(false, "Unhandled GGUserFile_TLASBuildFlags");
			return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		}
	}
}

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads
inline bool FormatSupportedForUAV(ID3D12Device* device, DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_UNKNOWN:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
			return true;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData;
	ZeroMemory(&FeatureData, sizeof(FeatureData));
	HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData));
	if (FAILED(hr) || !FeatureData.TypedUAVLoadAdditionalFormats)
		return false;

	switch (format)
	{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_R8_TYPELESS:
			return true;
	}

	D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = { format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
	hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
	return (SUCCEEDED(hr) && (FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0);
}

inline D3D12_STENCIL_OP StencilOpToD3D12_STENCIL_OP(StencilOp op)
{
	switch (op)
	{
		case StencilOp::Keep: return D3D12_STENCIL_OP_KEEP;
		case StencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
		case StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
		case StencilOp::IncrementSaturate: return D3D12_STENCIL_OP_INCR_SAT;
		case StencilOp::DecrimentSaturate: return D3D12_STENCIL_OP_DECR_SAT;
		case StencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
		case StencilOp::Increment: return D3D12_STENCIL_OP_INCR;
		case StencilOp::Decriment: return D3D12_STENCIL_OP_DECR;
	}

	Assert(false, "Unhandled StencilOp");
	return D3D12_STENCIL_OP_KEEP;
}
