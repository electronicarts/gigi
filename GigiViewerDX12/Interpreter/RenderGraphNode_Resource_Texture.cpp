///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"
#include "DX12Utils/CreateResources.h"
#include "DX12Utils/Utils.h"
#include "DX12Utils/SRGB.h"
#include <d3dx12/d3dx12.h>
// clang-format on

static const char* c_cubeMapNames[] =
{
	"Right",
	"Left",
	"Up",
	"Down",
	"Front",
	"Back"
};

static inline double Lerp(double A, double B, double t)
{
	return A * (1.0 - t) + B * t;
}

// Return false if there are invalid % format markers
static bool FileNameSafe(const char* fileName)
{
	if (!fileName)
		return true;

	const char* c = fileName;
	while (*c)
	{
		if (*c == '%')
		{
			c++;
			switch (*c)
			{
				case 'i':
				case 's':
				{
					break;
				}
				default: return false;
			}
		}
		c++;
	}

	return true;
}

bool RuntimeTypes::RenderGraphNode_Resource_Texture::GetDSV(ID3D12Device2* device, D3D12_CPU_DESCRIPTOR_HANDLE& handle, HeapAllocationTracker& DSVHeapAllocationTracker, TextureDimensionType dimension, int arrayIndex, int mipLevel, const char* resourceName)
{
	// Make the key
	SubResourceKey key;
	key.arrayIndex = arrayIndex;
	key.mipLevel = mipLevel;

	// If it already exists, use it
	auto it = m_dsvIndices.find(key);
	if (it != m_dsvIndices.end())
	{
		int dsvIndex = it->second;
		handle = DSVHeapAllocationTracker.GetCPUHandle(dsvIndex);
		return true;
	}

	// Allocate a DSV index and store this index in the indices tracked for this resource
	char DSVName[1024];
	sprintf_s(DSVName, "%s DSV (%i,%i)", resourceName, arrayIndex, mipLevel);
	int dsvIndex = -1;
	if (!DSVHeapAllocationTracker.Allocate(dsvIndex, DSVName))
		return false;
	m_dsvIndices[key] = dsvIndex;

	// Create the DSV
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DSV_Safe_DXGI_FORMAT(m_format);
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	switch (dimension)
	{
		case TextureDimensionType::Texture2D:
		{
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = mipLevel;
			break;
		}
		case TextureDimensionType::Texture2DArray:
		{
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.MipSlice = mipLevel;
			dsvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
			dsvDesc.Texture2DArray.ArraySize = 1;
			break;
		}
		default:
		{
			return false;
		}
	}

	device->CreateDepthStencilView(m_resource, &dsvDesc, DSVHeapAllocationTracker.GetCPUHandle(dsvIndex));

	handle = DSVHeapAllocationTracker.GetCPUHandle(dsvIndex);
	return true;
}

bool RuntimeTypes::RenderGraphNode_Resource_Texture::GetRTV(ID3D12Device2* device, D3D12_CPU_DESCRIPTOR_HANDLE& handle, HeapAllocationTracker& RTVHeapAllocationTracker, TextureDimensionType dimension, int arrayIndex, int mipLevel, const char* resourceName)
{
	// Make the key
	SubResourceKey key;
	key.arrayIndex = arrayIndex;
	key.mipLevel = mipLevel;

	// If it already exists, use it
	auto it = m_rtvIndices.find(key);
	if (it != m_rtvIndices.end())
	{
		int rtvIndex = it->second;
		handle = RTVHeapAllocationTracker.GetCPUHandle(rtvIndex);
		return true;
	}

	// Allocate an RTV index and store this index in the indices tracked for this resource
	char DSVName[1024];
	sprintf_s(DSVName, "%s RTV (%i,%i)", resourceName, arrayIndex, mipLevel);
	int rtvIndex = -1;
	if (!RTVHeapAllocationTracker.Allocate(rtvIndex, DSVName))
		return false;
	m_rtvIndices[key] = rtvIndex;

	// Create the RTV
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = m_format;
	switch (dimension)
	{
		case TextureDimensionType::Texture2D:
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = mipLevel;
			rtvDesc.Texture2D.PlaneSlice = 0;
			break;
		}
		case TextureDimensionType::Texture2DArray:
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = mipLevel;
			rtvDesc.Texture2DArray.PlaneSlice = 0;
			rtvDesc.Texture2DArray.ArraySize = 1;
			rtvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
			break;
		}
		case TextureDimensionType::Texture3D:
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			rtvDesc.Texture3D.MipSlice = mipLevel;
			rtvDesc.Texture3D.WSize = 1;
			rtvDesc.Texture3D.FirstWSlice = arrayIndex;
		}
		default:
		{
			return false;
		}
	}

	device->CreateRenderTargetView(m_resource, &rtvDesc, RTVHeapAllocationTracker.GetCPUHandle(rtvIndex));

	handle = RTVHeapAllocationTracker.GetCPUHandle(rtvIndex);

	return true;
}

void RuntimeTypes::RenderGraphNode_Resource_Texture::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
	RenderGraphNode_Base::Release(interpreter);

	// free resources
	if (m_resourceInitialState)
	{
		interpreter.m_transitions.Untrack(m_resourceInitialState);
		interpreter.m_delayedRelease.Add(m_resourceInitialState);
		m_resourceInitialState = nullptr;
	}

	if (m_resource)
	{
		interpreter.m_transitions.Untrack(m_resource);
		interpreter.m_delayedRelease.Add(m_resource);
		m_resource = nullptr;
	}

	for (auto it : m_dsvIndices)
		interpreter.m_DSVHeapAllocationTracker.Free(it.second);
	m_dsvIndices.clear();

	for (auto it : m_rtvIndices)
		interpreter.m_RTVHeapAllocationTracker.Free(it.second);
	m_rtvIndices.clear();

	m_failed = false;
}

template <typename T>
std::vector<double> ConvertToDoubles(const std::vector<unsigned char>& src, double multiplier)
{
	const size_t valueCount = src.size() / sizeof(T);

	std::vector<double> ret(valueCount, 0);

	const T* srcValues = (const T*)src.data();
	for (size_t index = 0; index < valueCount; ++index)
		ret[index] = multiplier * (double)srcValues[index];

	return ret;
}

template <typename T>
std::vector<double> ConvertToDoubles(const unsigned char* src, size_t valueCount, double multiplier)
{
	std::vector<double> ret(valueCount, 0);

	const T* srcValues = (const T*)src;
	for (size_t index = 0; index < valueCount; ++index)
		ret[index] = multiplier * (double)srcValues[index];

	return ret;
}

static bool ConvertToDoubles(const std::vector<unsigned char>& src, DXGI_FORMAT_Info::ChannelType type, std::vector<double>& doubles)
{
	switch (type)
	{
		case DXGI_FORMAT_Info::ChannelType::_uint8_t: doubles = ConvertToDoubles<uint8_t>(src, 1.0 / 255.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint16_t: doubles = ConvertToDoubles<uint16_t>(src, 1.0 / 65535.0); break;
		case DXGI_FORMAT_Info::ChannelType::_int16_t: doubles = ConvertToDoubles<int16_t>(src, 1.0 / 32767.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint32_t: doubles = ConvertToDoubles<uint32_t>(src, 1.0 / 4294967296.0); break;
		case DXGI_FORMAT_Info::ChannelType::_half: doubles = ConvertToDoubles<half>(src, 1.0); break;
		case DXGI_FORMAT_Info::ChannelType::_float: doubles = ConvertToDoubles<float>(src, 1.0); break;
		default: return false;
	}
	return true;
}

static bool ConvertToDoubles(const unsigned char* src, size_t valueCount, DXGI_FORMAT_Info::ChannelType type, std::vector<double>& doubles)
{
	switch (type)
	{
		case DXGI_FORMAT_Info::ChannelType::_uint8_t: doubles = ConvertToDoubles<uint8_t>(src, valueCount, 1.0 / 255.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint16_t: doubles = ConvertToDoubles<uint16_t>(src, valueCount, 1.0 / 65535.0); break;
		case DXGI_FORMAT_Info::ChannelType::_int16_t: doubles = ConvertToDoubles<int16_t>(src, valueCount, 1.0 / 32767.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint32_t: doubles = ConvertToDoubles<uint32_t>(src, valueCount, 1.0 / 4294967296.0); break;
		case DXGI_FORMAT_Info::ChannelType::_half: doubles = ConvertToDoubles<half>(src, valueCount, 1.0); break;
		case DXGI_FORMAT_Info::ChannelType::_float: doubles = ConvertToDoubles<float>(src, valueCount, 1.0); break;
		default: return false;
	}
	return true;
}

template <typename T>
std::vector<unsigned char> ConvertFromDoubles(const std::vector<double>& src, double multiplier, double theMin, double theMax)
{
	const size_t valueCount = src.size();

	std::vector<unsigned char> ret(valueCount * sizeof(T), 0);

	T* destValues = (T*)ret.data();
	for (size_t index = 0; index < valueCount; ++index)
		destValues[index] = (T)(max(min(src[index] * multiplier, theMax), theMin));

	return ret;
}

static bool ConvertFromDoubles(const std::vector<double>& doubles, DXGI_FORMAT_Info::ChannelType type, std::vector<unsigned char>& dest)
{
	switch (type)
	{
		case DXGI_FORMAT_Info::ChannelType::_uint8_t: dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, 0.0, 255.0); break;
		case DXGI_FORMAT_Info::ChannelType::_int8_t: dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, -128.0, 127.0); break;
		case DXGI_FORMAT_Info::ChannelType::_int16_t: dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, -32768.0, 32767.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint16_t: dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, 0.0, 65535.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint32_t: dest = ConvertFromDoubles<uint32_t>(doubles, 4294967296.0, 0.0, 4294967295.0); break;
		case DXGI_FORMAT_Info::ChannelType::_half: dest = ConvertFromDoubles<half>(doubles, 1.0, -65504.0, 65504.0); break;
		case DXGI_FORMAT_Info::ChannelType::_float: dest = ConvertFromDoubles<float>(doubles, 1.0, -FLT_MAX, FLT_MAX); break;
		default: return false;
	}
	return true;
}

static bool ConvertFromDoubles(const std::vector<double>& doubles, DXGI_FORMAT_Info::ChannelType type, unsigned char* dest)
{
	std::vector<unsigned char> _dest;
	switch (type)
	{
		case DXGI_FORMAT_Info::ChannelType::_uint8_t: _dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, 0.0, 255.0); break;
		case DXGI_FORMAT_Info::ChannelType::_int8_t: _dest = ConvertFromDoubles<uint8_t>(doubles, 256.0, -128.0, 127.0); break;
		case DXGI_FORMAT_Info::ChannelType::_int16_t: _dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, -32768.0, 32767.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint16_t: _dest = ConvertFromDoubles<uint16_t>(doubles, 65536.0, 0.0, 65535.0); break;
		case DXGI_FORMAT_Info::ChannelType::_uint32_t: _dest = ConvertFromDoubles<uint32_t>(doubles, 4294967296.0, 0.0, 4294967295.0); break;
		case DXGI_FORMAT_Info::ChannelType::_half: _dest = ConvertFromDoubles<half>(doubles, 1.0, -65504.0, 65504.0); break;
		case DXGI_FORMAT_Info::ChannelType::_float: _dest = ConvertFromDoubles<float>(doubles, 1.0, -FLT_MAX, FLT_MAX); break;
		default: return false;
	}

	memcpy(dest, _dest.data(), _dest.size());

	return true;
}

bool ConvertPixelData(const std::vector<unsigned char>& src, const DXGI_FORMAT_Info& srcFormat_, std::vector<unsigned char>& dest, const DXGI_FORMAT_Info& destFormat)
{
	// We don't do conversion on compressed image formats
	if (srcFormat_.isCompressed || destFormat.isCompressed)
	{
		if (srcFormat_.format == destFormat.format)
		{
			dest = src;
			return true;
		}
		return false;
	}

	// Nothing to do if nothing to convert
	DXGI_FORMAT_Info srcFormat = srcFormat_;
	if (srcFormat.channelType == destFormat.channelType && srcFormat.sRGB == destFormat.sRGB && srcFormat.channelCount == destFormat.channelCount)
	{
		dest = src;
		return true;
	}

	// Convert the source pixels to doubles
	std::vector<double> srcDoubles;
	if (!ConvertToDoubles(src, srcFormat.channelType, srcDoubles))
		return false;

	// convert to the right channel count
	if (srcFormat.channelCount != destFormat.channelCount)
	{
		size_t numPixels = srcDoubles.size() / srcFormat.channelCount;
		std::vector<double> temp(numPixels * destFormat.channelCount);

		const double* srcPixel = srcDoubles.data();
		double* destPixel = temp.data();

		for (size_t i = 0; i < numPixels; ++i)
		{
			memcpy(destPixel, srcPixel, sizeof(double) * min(srcFormat.channelCount, destFormat.channelCount));

			for (size_t c = srcFormat.channelCount; c < destFormat.channelCount; c++)
				destPixel[c] = (c < 3) ? 0.0f : 1.0f;

			srcPixel += srcFormat.channelCount;
			destPixel += destFormat.channelCount;
		}

		// Channel count conversion done
		srcFormat.channelCount = destFormat.channelCount;
		srcDoubles = temp;
	}

	// do sRGB conversion if we should
	if (srcFormat.sRGB != destFormat.sRGB)
	{
		int channelsToConvert = min(srcFormat.channelCount, 3); // don't convert alpha

		if (srcFormat.sRGB)
		{
			size_t index = 0;
			while (index < srcDoubles.size())
			{
				for (int channel = 0; channel < channelsToConvert; ++channel)
					srcDoubles[index + channel] = (float)SRGBToLinear((float)srcDoubles[index + channel]);
				index += srcFormat.channelCount;
			}
		}
		else
		{
			size_t index = 0;
			while (index < srcDoubles.size())
			{
				for (int channel = 0; channel < channelsToConvert; ++channel)
					srcDoubles[index + channel] = (float)LinearTosRGB((float)srcDoubles[index + channel]);
				index += srcFormat.channelCount;
			}
		}

		srcFormat.sRGB = destFormat.sRGB;
	}

	// Convert from doubles to the destination format
	if (!ConvertFromDoubles(srcDoubles, destFormat.channelType, dest))
		return false;

	return true;
}

static void BilinearSample(const unsigned char* src, const DXGI_FORMAT_Info& formatInfo, const int dims[3], float U, float V, unsigned char* dest)
{
	float srcX = U * float(dims[0]) - 0.5f;
	float srcY = V * float(dims[1]) - 0.5f;

	int x1 = int(srcX);
	int y1 = int(srcY);
	int x2 = (x1 + 1) % dims[0];
	int y2 = (y1 + 1) % dims[1];

	float fracX = std::fmod(srcX, 1.0f);
	float fracY = std::fmod(srcY, 1.0f);

	// read the source pixels
	std::vector<double> P00Values, P01Values, P10Values, P11Values;
	ConvertToDoubles(&src[(y1 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P00Values);
	ConvertToDoubles(&src[(y1 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P10Values);
	ConvertToDoubles(&src[(y2 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P01Values);
	ConvertToDoubles(&src[(y2 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P11Values);

	// Calculate the result
	std::vector<double> result(formatInfo.channelCount);
	for (size_t channelIndex = 0; channelIndex < formatInfo.channelCount; ++channelIndex)
	{
		double resultx0 = Lerp(P00Values[channelIndex], P10Values[channelIndex], fracX);
		double resultx1 = Lerp(P01Values[channelIndex], P11Values[channelIndex], fracX);
		result[channelIndex] = Lerp(resultx0, resultx1, fracY);
	}

	// Write the dest pixel
	ConvertFromDoubles(result, formatInfo.channelType, dest);
}

static void TrilinearSample(const unsigned char* src, const DXGI_FORMAT_Info& formatInfo, const int dims[3], float U, float V, float W, unsigned char* dest)
{
	float srcX = U * float(dims[0]) - 0.5f;
	float srcY = V * float(dims[1]) - 0.5f;
	float srcZ = W * float(dims[2]) - 0.5f;

	int x1 = int(srcX);
	int y1 = int(srcY);
	int z1 = int(srcZ);
	int x2 = (x1 + 1) % dims[0];
	int y2 = (y1 + 1) % dims[1];
	int z2 = (z1 + 1) % dims[2];

	float fracX = std::fmod(srcX, 1.0f);
	float fracY = std::fmod(srcY, 1.0f);
	float fracZ = std::fmod(srcZ, 1.0f);

	// read the source pixels
	std::vector<double> P000Values, P010Values, P100Values, P110Values;
	std::vector<double> P001Values, P011Values, P101Values, P111Values;
	ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y1 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P000Values);
	ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y1 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P100Values);
	ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y2 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P010Values);
	ConvertToDoubles(&src[(z1 * dims[1] * dims[0] + y2 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P110Values);
	ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y1 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P001Values);
	ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y1 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P101Values);
	ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y2 * dims[0] + x1) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P011Values);
	ConvertToDoubles(&src[(z2 * dims[1] * dims[0] + y2 * dims[0] + x2) * formatInfo.bytesPerPixel], formatInfo.channelCount, formatInfo.channelType, P111Values);

	// Calculate the result
	std::vector<double> result(formatInfo.channelCount);
	for (size_t channelIndex = 0; channelIndex < formatInfo.channelCount; ++channelIndex)
	{
		double resultx00 = Lerp(P000Values[channelIndex], P100Values[channelIndex], fracX);
		double resultx10 = Lerp(P010Values[channelIndex], P110Values[channelIndex], fracX);

		double resultx01 = Lerp(P001Values[channelIndex], P101Values[channelIndex], fracX);
		double resultx11 = Lerp(P011Values[channelIndex], P111Values[channelIndex], fracX);

		double resultxy0 = Lerp(resultx00, resultx10, fracY);
		double resultxy1 = Lerp(resultx01, resultx11, fracY);

		result[channelIndex] = Lerp(resultxy0, resultxy1, fracZ);
	}

	// Write the dest pixel
	ConvertFromDoubles(result, formatInfo.channelType, dest);
}

static void MakeMip(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const DXGI_FORMAT_Info& formatInfo, TextureDimensionType dimension, const int srcDims[3])
{
	// calculate the size of the destination dims
	bool is3D = (dimension == TextureDimensionType::Texture3D);
	int destDims[3] = { max(srcDims[0] / 2, 1), max(srcDims[1] / 2, 1), is3D ? max(srcDims[2] / 2, 1) : srcDims[2] };

	// Allocate space for the mip
	dest.resize(destDims[0] * destDims[1] * destDims[2] * formatInfo.bytesPerPixel, 0);

	int srcUnalignedPitch = srcDims[0] * formatInfo.bytesPerPixel;
	int destUnalignedPitch = destDims[0] * formatInfo.bytesPerPixel;

	if (is3D)
	{
		// For each z slize
		for (size_t destZ = 0; destZ < destDims[2]; ++destZ)
		{
			float W = (float(destZ) + 0.5f) / float(destDims[2]);

			size_t destArrayOffset = destZ * destDims[1] * destUnalignedPitch;

			// for each pixel
			for (size_t destY = 0; destY < destDims[1]; ++destY)
			{
				float V = (float(destY) + 0.5f) / float(destDims[1]);

				size_t destOffset = destArrayOffset + destY * destUnalignedPitch;

				for (size_t destX = 0; destX < destDims[0]; ++destX)
				{
					float U = (float(destX) + 0.5f) / float(destDims[0]);

					unsigned char* destPx = &dest[destOffset + destX * formatInfo.bytesPerPixel];

					// trilinear sample src[U, V, W] and write to dest
					TrilinearSample(src.data(), formatInfo, srcDims, U, V, W, destPx);
				}
			}
		}
	}
	else
	{
		// For each array slize
		for (size_t iZ = 0; iZ < srcDims[2]; ++iZ)
		{
			size_t srcArrayOffset = iZ * srcDims[1] * srcUnalignedPitch;
			size_t destArrayOffset = iZ * destDims[1] * destUnalignedPitch;

			// for each pixel
			for (size_t destY = 0; destY < destDims[1]; ++destY)
			{
				float V = (float(destY) + 0.5f) / float(destDims[1]);

				size_t destOffset = destArrayOffset + destY * destUnalignedPitch;

				for (size_t destX = 0; destX < destDims[0]; ++destX)
				{
					float U = (float(destX) + 0.5f) / float(destDims[0]);

					unsigned char* destPx = &dest[destOffset + destX * formatInfo.bytesPerPixel];

					// bilinear sample src[U, V] and write to dest
					BilinearSample(&src[srcArrayOffset], formatInfo, srcDims, U, V, destPx);
				}
			}
		}
	}
}

static TextureCache::Texture LoadTextureFromBinaryFile(FileCache& fileCache, const char* fileName_, int dims[2], int channelCount, TextureCache::Type dataType)
{
	size_t dataTypeSize = (dataType == TextureCache::Type::F32) ? sizeof(float) : sizeof(uint8_t);

	// normalize the string by making it canonical and making it lower case
	std::filesystem::path p = std::filesystem::weakly_canonical(fileName_);
	std::string s = p.string();
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	const char* fileName = s.c_str();

	// Get the file data off disk
	FileCache::File fileData = fileCache.Get(s.c_str());

	// init the texture object
	TextureCache::Texture newTexture;
	newTexture.fileName = fileName;

	// bail out if there aren't the right number of bytes
	if (fileData.GetSize() != dims[0] * dims[1] * channelCount * dataTypeSize)
		return newTexture;

	// fill out the other texture data
	newTexture.width = dims[0];
	newTexture.height = dims[1];
	newTexture.channels = channelCount;
	newTexture.type = dataType;

	// copy pixel data
	newTexture.pixels.resize(fileData.GetSize());
	memcpy(newTexture.pixels.data(), fileData.GetBytes(), fileData.GetSize());

	return newTexture;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeActionImported(const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, NodeAction nodeAction)
{
	// If this resource is imported, add it to the list of imported resources.
	// Let resource creation happen during execute, since the desired format or size can change due to runtime conditions.
	if (nodeAction == NodeAction::Init)
	{
		if (m_importedResources.count(node.name) == 0)
			m_importedResources[node.name] = ImportedResourceDesc(true);
		m_importedResources[node.name].stale = false;
		m_importedResources[node.name].nodeIndex = node.nodeIndex;
		m_importedResources[node.name].resourceIndex = 0; // corresponds to the initial state resource being the first viewable resource
		m_importedResources[node.name].texture.textureType = TextureDimensionTypeToViewableResourceType(node.dimension);
		return true;
	}

	if (nodeAction == NodeAction::Execute)
	{
		ImportedResourceDesc& desc = m_importedResources[node.name];

		// (Re)Create a texture, as necessary
		// if the texture isn't dirty, nothing to do
		// if the format is "any", wait until it isn't
		if (desc.state == ImportedResourceState::dirty && desc.texture.format != TextureFormat::Any)
		{
			// Release any resources which may have previously existed
			runtimeData.Release(*this);

			// fill out the pixels - either from a file on disk, or a solid color;
			std::vector<unsigned char> allPixels;

			runtimeData.m_format = TextureFormatToDXGI_FORMAT(desc.texture.format);
			DXGI_FORMAT_Info pixelsFormatInfo = Get_DXGI_FORMAT_Info(runtimeData.m_format);

			// if we have a file to load
			if (!desc.texture.fileName.empty())
			{
				if (!FileNameSafe(desc.texture.fileName.c_str()))
					return true;

				std::vector<TextureCache::Texture> loadedTextures;

				bool useCubeMapNames = (desc.texture.fileName.find("%s") != std::string::npos);

				int textureIndex = -1;
				bool loadedBinary = false;
				while (1)
				{
					// make the next file name
					textureIndex++;
					char indexedFileName[1024];
					if (useCubeMapNames)
						sprintf_s(indexedFileName, desc.texture.fileName.c_str(), c_cubeMapNames[textureIndex]);
					else
						sprintf_s(indexedFileName, desc.texture.fileName.c_str(), textureIndex);

					TextureCache::Texture& texture = m_textures.Get(m_files, indexedFileName);
					if (!texture.Valid())
					{
						if (textureIndex == 0)
						{
							// try as a binary file
							int binaryDims2D[2] = { desc.texture.binaryDims[0], desc.texture.binaryDims[1] * desc.texture.binaryDims[2] };
							TextureCache::Type dataType = (desc.texture.binaryType == GGUserFile_ImportedTexture_BinaryType::Float) ? TextureCache::Type::F32 : TextureCache::Type::U8;
							TextureCache::Texture texture = LoadTextureFromBinaryFile(m_files, indexedFileName, binaryDims2D, desc.texture.binaryChannels, dataType);

							// If still invalid, bail out
							if (!texture.Valid())
							{
								if (FileExists(indexedFileName))
									m_logFn(LogLevel::Error, "Could not load texture. Unsupported type or format. \"%s\"", indexedFileName);
								else
									m_logFn(LogLevel::Error, "Could not load texture. File not found. \"%s\"", indexedFileName);
								runtimeData.m_failed = true;
								desc.state = ImportedResourceState::failed;
								return false;
							}

							// finalize
							loadedTextures.push_back(texture);
							m_fileWatcher.Add(indexedFileName, FileWatchOwner::TextureCache);

							// otherwise we are done. binary files only load a single file
							loadedBinary = true;
							break;
						}
						else
						{
							break;
						}
					}

					loadedTextures.push_back(texture);
					m_fileWatcher.Add(indexedFileName, FileWatchOwner::TextureCache);

					if (textureIndex > 0)
					{
						if (loadedTextures[0].width != texture.width || loadedTextures[0].height != texture.height && texture.type != loadedTextures[0].type || texture.channels != loadedTextures[0].channels)
						{
							m_logFn(LogLevel::Error, "Texture \"%s\" is the wrong size or type", indexedFileName);
							runtimeData.m_failed = true;
							desc.state = ImportedResourceState::failed;
							return false;
						}
					}

					// only one texture allowed in a texture2D
					if (node.dimension == TextureDimensionType::Texture2D)
						break;

					// If there is no %i or %s in the filename, break
					if (desc.texture.fileName.find("%i") == std::string::npos && desc.texture.fileName.find("%s") == std::string::npos)
						break;

					// cube maps are only allowed 6 textures
					if (useCubeMapNames && textureIndex == 5)
						break;
				}

				// if loaded as binary, break the singularly loaded image into multiple images if we should
				if (loadedBinary && desc.texture.binaryDims[2] > 1)
				{
					int dataByteSize = (int)(loadedTextures[0].type == TextureCache::Type::F32) ? sizeof(float) : sizeof(uint8_t);
					int imageSizeBytes = desc.texture.binaryDims[0] * desc.texture.binaryDims[1] * desc.texture.binaryChannels * dataByteSize;

					for (int z = 1; z < desc.texture.binaryDims[2]; ++z)
					{
						TextureCache::Texture newTexture = loadedTextures[0];
						newTexture.height = desc.texture.binaryDims[1];
						newTexture.pixels.resize(imageSizeBytes);

						unsigned char* dest = newTexture.pixels.data();
						const unsigned char* src = &loadedTextures[0].pixels[z * imageSizeBytes];
						memcpy(dest, src, imageSizeBytes);

						loadedTextures.push_back(newTexture);
					}

					loadedTextures[0].height = desc.texture.binaryDims[1];
					loadedTextures[0].pixels.resize(imageSizeBytes);
				}

				// Ensure that cube maps have 6 images loaded
				if (node.dimension == TextureDimensionType::TextureCube)
				{
					if (loadedTextures.size() != 6)
					{
						m_logFn(LogLevel::Error, "Cube map \"%s\" does not have 6 images, it has %i", node.name.c_str(), (int)loadedTextures.size());
						runtimeData.m_failed = true;
						desc.state = ImportedResourceState::failed;
						return false;
					}
				}

				// make a DXGI_FORMAT_Info describing our loaded texture data
				DXGI_FORMAT_Info textureFormatInfo;
				switch (loadedTextures[0].type)
				{
					case TextureCache::Type::U8: textureFormatInfo = DXGI_FORMAT_INFO(uint8_t, loadedTextures[0].channels, desc.texture.fileIsSRGB); break;
					case TextureCache::Type::F32: textureFormatInfo = DXGI_FORMAT_INFO(float, loadedTextures[0].channels, desc.texture.fileIsSRGB); break;
					case TextureCache::Type::BC7: textureFormatInfo = desc.texture.fileIsSRGB ? Get_DXGI_FORMAT_Info(DXGI_FORMAT_BC7_UNORM_SRGB) : Get_DXGI_FORMAT_Info(DXGI_FORMAT_BC7_UNORM); break;
					default:
					{
						desc.state = ImportedResourceState::failed;
						return true;
					}
				}

				// tint the image if we should
				std::vector<unsigned char> allTintedPixels;
				if (desc.texture.color[0] != 1.0f || desc.texture.color[1] != 1.0f || desc.texture.color[2] != 1.0f || desc.texture.color[3] != 1.0f)
				{
					if (textureFormatInfo.isCompressed)
					{
						m_logFn(LogLevel::Error, "Texture \"%s\": cannot tint a compressed texture format", node.name.c_str());
						desc.state = ImportedResourceState::failed;
						return false;
					}

					std::vector<unsigned char> tintedPixels(loadedTextures[0].pixels.size());
					for (const TextureCache::Texture& texture : loadedTextures)
					{
						switch (texture.type)
						{
							case TextureCache::Type::U8:
							{
								const unsigned char* srcPixel = texture.pixels.data();
								unsigned char* destPixel = tintedPixels.data();
								for (int pixelIndex = 0; pixelIndex < texture.width * texture.height; ++pixelIndex)
								{
									for (int channelIndex = 0; channelIndex < texture.channels; ++channelIndex)
									{
										*destPixel = (unsigned char)(float(*srcPixel) * desc.texture.color[channelIndex]);
										srcPixel++;
										destPixel++;
									}
								}
								break;
							}
							case TextureCache::Type::F32:
							{
								const float* srcPixel = (float*)texture.pixels.data();
								float* destPixel = (float*)tintedPixels.data();
								for (int pixelIndex = 0; pixelIndex < texture.width * texture.height; ++pixelIndex)
								{
									for (int channelIndex = 0; channelIndex < texture.channels; ++channelIndex)
									{
										*destPixel = *srcPixel * desc.texture.color[channelIndex];
										srcPixel++;
										destPixel++;
									}
								}
								break;
							}
							default:
							{
								desc.state = ImportedResourceState::failed;
								return true;
							}
						}
						allTintedPixels.insert(allTintedPixels.end(), tintedPixels.begin(), tintedPixels.end());
					}
				}
				else
				{
					for (const TextureCache::Texture& texture : loadedTextures)
						allTintedPixels.insert(allTintedPixels.end(), texture.pixels.begin(), texture.pixels.end());
				}

				// convert to the correct format
				if (!ConvertPixelData(allTintedPixels, textureFormatInfo, allPixels, pixelsFormatInfo))
				{
					desc.state = ImportedResourceState::failed;
					return true;
				}
				runtimeData.m_size[0] = loadedTextures[0].width;
				runtimeData.m_size[1] = loadedTextures[0].height;
				runtimeData.m_size[2] = (int)loadedTextures.size();
			}
			// else if we have a size
			else if (desc.texture.size[0] > 0 && desc.texture.size[1] > 0)
			{
				// make the initialization value for a single pixel
				std::vector<float> initValue(pixelsFormatInfo.channelCount);
				for (int i = 0; i < pixelsFormatInfo.channelCount; ++i)
					initValue[i] = desc.texture.color[i];

				// repeat that to make the whole texture.
				std::vector<unsigned char> srcPixels(desc.texture.size[0] * desc.texture.size[1] * desc.texture.size[2] * pixelsFormatInfo.channelCount * sizeof(float));
				for (int i = 0; i < desc.texture.size[0] * desc.texture.size[1] * desc.texture.size[2]; ++i)
					memcpy(&srcPixels[i * pixelsFormatInfo.channelCount * sizeof(float)], initValue.data(), initValue.size() * sizeof(initValue[0]));

				// convert to the correct format
				if (!ConvertPixelData(srcPixels, DXGI_FORMAT_INFO(float, pixelsFormatInfo.channelCount, desc.texture.fileIsSRGB), allPixels, pixelsFormatInfo))
				{
					desc.state = ImportedResourceState::failed;
					return true;
				}

				runtimeData.m_size[0] = desc.texture.size[0];
				runtimeData.m_size[1] = desc.texture.size[1];
				runtimeData.m_size[2] = desc.texture.size[2];
			}
			// otherwise, we have nothing to go on
			else
			{
				desc.state = ImportedResourceState::failed;
				return true;
			}

			// Calculate mip count if needed
			runtimeData.m_numMips = 1;
			if (desc.texture.makeMips)
			{
				int maxSize = max(runtimeData.m_size[0], runtimeData.m_size[1]);
				while (maxSize > 1)
				{
					maxSize /= 2;
					runtimeData.m_numMips++;
				}
			}

			// We don't currently make mips for compressed texture formats
			if (runtimeData.m_numMips > 1 && pixelsFormatInfo.isCompressed)
			{
				m_logFn(LogLevel::Error, "Texture \"%s\": Cannot make mips for a compressed texture format", node.name.c_str());
				desc.state = ImportedResourceState::failed;
				return false;
			}

			// only let proper depth formats be allowed for use as a depth texture
			if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
			{
				if (!Get_DXGI_FORMAT_Info(runtimeData.m_format).isDepth)
				{
					runtimeData.m_renderGraphText = "Cannot create because texture is used as a depth target, but is not a depth format";
					return true;
				}
			}

			// UAV with stencil is not allowed
			if (pixelsFormatInfo.isStencil && (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV)))
			{
				runtimeData.m_renderGraphText = "Cannot create because texture is a stencil format, but is used with UAV access. This is not allowed.";
				return true;
			}

			// UAV with compressed texture types is not allowed
			if (pixelsFormatInfo.isCompressed && (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV)))
			{
				runtimeData.m_renderGraphText = "Cannot create because texture is a compressed format, but is used with UAV access. This is not allowed.";
				return true;
			}

			// Make a resource for the "live" texture which may be modified during running, and also for the initial state.
			D3D12_RESOURCE_FLAGS resourceFlags = ShaderResourceAccessToD3D12_RESOURCE_FLAGs(node.accessedAs);
			unsigned int size[3] = { (unsigned int)runtimeData.m_size[0], (unsigned int)runtimeData.m_size[1], (unsigned int)runtimeData.m_size[2] };
			std::string nodeNameInitialState = node.name + " Initial State";
			runtimeData.m_resourceInitialState = CreateTexture(m_device, size, runtimeData.m_numMips, runtimeData.m_format, resourceFlags, D3D12_RESOURCE_STATE_COPY_DEST, TextureDimensionTypeToResourceType(node.dimension), nodeNameInitialState.c_str());
			runtimeData.m_resource = CreateTexture(m_device, size, runtimeData.m_numMips, runtimeData.m_format, resourceFlags, D3D12_RESOURCE_STATE_COPY_DEST, TextureDimensionTypeToResourceType(node.dimension), node.name.c_str());

			// track the new resources for state transitions
			m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COPY_DEST));
			m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));

			// Make the mips
			std::vector<std::vector<unsigned char>> allPixelsMips(runtimeData.m_numMips > 1 ? runtimeData.m_numMips - 1 : 0);
			{
				int mipDims[3] = { runtimeData.m_size[0], runtimeData.m_size[1], runtimeData.m_size[2] };
				for (int mipIndex = 0; mipIndex < runtimeData.m_numMips - 1; ++mipIndex)
				{
					if (mipIndex == 0)
						MakeMip(allPixels, allPixelsMips[0], pixelsFormatInfo, node.dimension, mipDims);
					else
						MakeMip(allPixelsMips[mipIndex - 1], allPixelsMips[mipIndex], pixelsFormatInfo, node.dimension, mipDims);

					mipDims[0] = max(mipDims[0] / 2, 1);
					mipDims[1] = max(mipDims[1] / 2, 1);
					if (node.dimension == TextureDimensionType::Texture3D)
						mipDims[2] = max(mipDims[2] / 2, 1);
				}
			}

			// copy everything to GPU
			if (pixelsFormatInfo.isCompressed)
			{
				// Make an upload buffer
				int unalignedPitch = (int)allPixels.size();
				int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);
				UploadBufferTracker::Buffer* uploadBuffer = m_uploadBufferTracker.GetBuffer(m_device, alignedPitch, false);

				// write the pixels into the upload buffer
				{
					unsigned char* destPixels = nullptr;
					HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&destPixels));
					if (hr)
						return false;

					memcpy(destPixels, allPixels.data(), allPixels.size());

					uploadBuffer->buffer->Unmap(0, nullptr);
				}

				// Copy the upload buffer into m_resourceInitialState
				{
					UINT subresourceIndex = D3D12CalcSubresource(0, 0, 0, 1, 1);

					D3D12_RESOURCE_DESC resourceDesc = runtimeData.m_resourceInitialState->GetDesc();
					std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
					D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
					m_device->GetCopyableFootprints(&resourceDesc, subresourceIndex, 1, 0, layout, nullptr, nullptr, nullptr);

					D3D12_TEXTURE_COPY_LOCATION src = {};
					src.pResource = uploadBuffer->buffer;
					src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					src.PlacedFootprint = *layout;

					D3D12_TEXTURE_COPY_LOCATION dest = {};
					dest.pResource = runtimeData.m_resourceInitialState;
					dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dest.SubresourceIndex = subresourceIndex;

					m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
				}
			}
			else
			{
				int mipDims[3] = { runtimeData.m_size[0], runtimeData.m_size[1], runtimeData.m_size[2] };
				for (int mipIndex = 0; mipIndex < runtimeData.m_numMips; ++mipIndex)
				{
					std::vector<unsigned char>& srcPixels = (mipIndex > 0)
						? allPixelsMips[mipIndex - 1]
						: allPixels;

					// Set up variables to handle 3d textures (single sub resource) vs other times (a sub resource per 2d texture)
					int arrayCount = 0;
					int iyCount = mipDims[1];
					int izCount = 0;
					if (node.dimension == TextureDimensionType::Texture3D)
					{
						arrayCount = 1;
						izCount = mipDims[2];
					}
					else
					{
						arrayCount = mipDims[2];
						izCount = 1;
					}

					int allPixelsStride = (int)srcPixels.size() / arrayCount;

					for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
					{
						unsigned int arrayIndexStart = allPixelsStride * arrayIndex;

						// Make an upload buffer
						int unalignedPitch = mipDims[0] * pixelsFormatInfo.bytesPerPixel;
						int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);
						UploadBufferTracker::Buffer* uploadBuffer = m_uploadBufferTracker.GetBuffer(m_device, alignedPitch * iyCount * izCount, false);

						// write the pixels into the upload buffer
						{
							unsigned char* destPixels = nullptr;
							HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&destPixels));
							if (hr)
								return false;

							for (int iz = 0; iz < izCount; ++iz)
							{
								for (int iy = 0; iy < iyCount; ++iy)
								{
									const unsigned char* src = &srcPixels[arrayIndexStart + (iz * iyCount + iy) * unalignedPitch];
									unsigned char* dest = &destPixels[(iz * iyCount + iy) * alignedPitch];
									memcpy(dest, src, unalignedPitch);
								}
							}

							uploadBuffer->buffer->Unmap(0, nullptr);
						}

						// Copy the upload buffer into m_resourceInitialState
						{
							UINT subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, runtimeData.m_numMips, arrayCount);

							D3D12_RESOURCE_DESC resourceDesc = runtimeData.m_resourceInitialState->GetDesc();
							std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
							D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
							m_device->GetCopyableFootprints(&resourceDesc, subresourceIndex, 1, 0, layout, nullptr, nullptr, nullptr);

							D3D12_TEXTURE_COPY_LOCATION src = {};
							src.pResource = uploadBuffer->buffer;
							src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
							src.PlacedFootprint = *layout;

							D3D12_TEXTURE_COPY_LOCATION dest = {};
							dest.pResource = runtimeData.m_resourceInitialState;
							dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
							dest.SubresourceIndex = subresourceIndex;

							m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
						}
					}

					mipDims[0] = max(mipDims[0] / 2, 1);
					mipDims[1] = max(mipDims[1] / 2, 1);
					if (node.dimension == TextureDimensionType::Texture3D)
						mipDims[2] = max(mipDims[2] / 2, 1);
				}
			}

			// Note that the resource wants to be reset to the initial state.
			runtimeData.m_resourceWantsReset = true;

			// All is well
			desc.state = ImportedResourceState::clean;
		}
	}

	return true;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeActionNotImported(const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, NodeAction nodeAction)
{
	if (nodeAction == NodeAction::Execute)
	{
		// failing to get the format and size could be a transient problem
		DXGI_FORMAT desiredFormat = GetDesiredFormat(*this, node);
		IVec3 desiredSize = GetDesiredSize(*this, node);

		bool hasSize = desiredSize[0] != 0 && desiredSize[1] != 0 && desiredSize[2] != 0;
		bool hasFileName = !node.loadFileName.empty() && FileNameSafe(node.loadFileName.c_str());
		std::string fullLoadFileName = m_tempDirectory + "assets\\" + node.loadFileName;

		if (desiredFormat != DXGI_FORMAT_FORCE_UINT && (hasSize || hasFileName) && !runtimeData.m_failed)
		{
			std::vector<TextureCache::Texture> loadedTextures;

			if (hasFileName)
			{
				if (runtimeData.m_resource)
				{
					desiredSize[0] = runtimeData.m_size[0];
					desiredSize[1] = runtimeData.m_size[1];
					desiredSize[2] = runtimeData.m_size[2];
				}
				else
				{
					bool useCubeMapNames = (fullLoadFileName.find("%s") != std::string::npos);

					int textureIndex = -1;
					while (1)
					{
						// make the next file name
						textureIndex++;
						char indexedFileName[1024];
						if (useCubeMapNames)
							sprintf_s(indexedFileName, fullLoadFileName.c_str(), c_cubeMapNames[textureIndex]);
						else
							sprintf_s(indexedFileName, fullLoadFileName.c_str(), textureIndex);

						TextureCache::Texture& texture = m_textures.Get(m_files, indexedFileName);
						if (!texture.Valid())
						{
							if (textureIndex == 0)
							{
								m_logFn(LogLevel::Error, "Could not load texture \"%s\"", indexedFileName);
								runtimeData.m_failed = true;
								return false;
							}
							else
							{
								break;
							}
						}
						loadedTextures.push_back(texture);
						m_fileWatcher.Add(indexedFileName, FileWatchOwner::TextureCache);

						if (textureIndex > 0)
						{
							if (desiredSize[0] != texture.width || desiredSize[1] != texture.height && texture.type != loadedTextures[0].type || texture.channels != loadedTextures[0].channels)
							{
								m_logFn(LogLevel::Error, "Texture \"%s\" is the wrong size or type", indexedFileName);
								runtimeData.m_failed = true;
								return false;
							}
						}
						else
						{
							desiredSize[0] = texture.width;
							desiredSize[1] = texture.height;
						}

						// only one texture allowed in a texture2D
						if (node.dimension == TextureDimensionType::Texture2D)
							break;

						// If there is no %i or %s in the filename, break
						if (fullLoadFileName.find("%i") == std::string::npos && fullLoadFileName.find("%s") == std::string::npos)
							break;

						// cube maps are only allowed 6 textures
						if (useCubeMapNames && textureIndex == 5)
							break;
					}
					desiredSize[2] = (int)loadedTextures.size();

					// Ensure that cube maps have 6 images loaded
					if (node.dimension == TextureDimensionType::TextureCube)
					{
						if (loadedTextures.size() != 6)
						{
							m_logFn(LogLevel::Error, "Cube map \"%s\" does not have 6 images, it has %i", node.name.c_str(), (int)loadedTextures.size());
							runtimeData.m_failed = true;
							return false;
						}
					}
				}
			}

			// Calculate desired mip count
			int desiredMips = node.numMips;
			if (desiredMips == 0 || (hasFileName && node.loadFileMakeMips))
			{
				int maxSize = max(desiredSize[0], desiredSize[1]);
				desiredMips = 1;
				while (maxSize > 1)
				{
					maxSize /= 2;
					desiredMips++;
				}
			}

			// (re) create the resource if we should
			if (!runtimeData.m_resource || runtimeData.m_format != desiredFormat || runtimeData.m_size[0] != desiredSize[0] || runtimeData.m_size[1] != desiredSize[1] || runtimeData.m_size[2] != desiredSize[2] || runtimeData.m_numMips != desiredMips)
			{
				// Release any resources which may have previously existed
				runtimeData.Release(*this);

				// set the format and size
				runtimeData.m_format = desiredFormat;
				runtimeData.m_size[0] = desiredSize[0];
				runtimeData.m_size[1] = desiredSize[1];
				runtimeData.m_size[2] = desiredSize[2];
				runtimeData.m_numMips = desiredMips;

				// UAV with stencil is not allowed
				DXGI_FORMAT_Info pixelsFormatInfo = Get_DXGI_FORMAT_Info(runtimeData.m_format);
				if (pixelsFormatInfo.isStencil && (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV)))
				{
					runtimeData.m_renderGraphText = "Cannot create because texture is a stencil format, but is used with UAV access";
					return true;
				}

				// Make a resource for the "live" texture which may be modified during running, and also for the initial state.
				D3D12_RESOURCE_FLAGS resourceFlags = ShaderResourceAccessToD3D12_RESOURCE_FLAGs(node.accessedAs);
				unsigned int size[3] = { (unsigned int)desiredSize[0], (unsigned int)desiredSize[1], (unsigned int)desiredSize[2] };
				std::string nodeNameInitialState = node.name + " Initial State";
				runtimeData.m_resourceInitialState = CreateTexture(m_device, size, desiredMips, runtimeData.m_format, resourceFlags, D3D12_RESOURCE_STATE_COPY_DEST, TextureDimensionTypeToResourceType(node.dimension), nodeNameInitialState.c_str());
				runtimeData.m_resource = CreateTexture(m_device, size, desiredMips, runtimeData.m_format, resourceFlags, D3D12_RESOURCE_STATE_COPY_DEST, TextureDimensionTypeToResourceType(node.dimension), node.name.c_str());

				// track the new resources for state transitions
				m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));

				if (hasFileName)
				{
					// get the texture info
					TextureCache::Texture& firstTexture = loadedTextures[0];

					// make a DXGI_FORMAT_Info describing our loaded texture data
					DXGI_FORMAT_Info textureFormatInfo;
					switch (firstTexture.type)
					{
						case TextureCache::Type::U8: textureFormatInfo = DXGI_FORMAT_INFO(uint8_t, firstTexture.channels, node.loadFileNameAsSRGB); break;
						case TextureCache::Type::F32: textureFormatInfo = DXGI_FORMAT_INFO(float, firstTexture.channels, node.loadFileNameAsSRGB); break;
						case TextureCache::Type::BC7: textureFormatInfo = node.loadFileNameAsSRGB ? Get_DXGI_FORMAT_Info(DXGI_FORMAT_BC7_UNORM_SRGB) : Get_DXGI_FORMAT_Info(DXGI_FORMAT_BC7_UNORM); break;
						default:
						{
							return false;
						}
					}

					// convert all the textures into a single bufer
					std::vector<unsigned char> allPixels;
					DXGI_FORMAT_Info pixelsFormatInfo = Get_DXGI_FORMAT_Info(runtimeData.m_format);
					for (size_t textureIndex = 0; textureIndex < loadedTextures.size(); ++textureIndex)
					{
						std::vector<unsigned char> pixelsConverted;
						if (!ConvertPixelData(loadedTextures[textureIndex].pixels, textureFormatInfo, pixelsConverted, pixelsFormatInfo))
							return false;

						allPixels.insert(allPixels.end(), pixelsConverted.begin(), pixelsConverted.end());
					}

					// Make the mips
					std::vector<std::vector<unsigned char>> allPixelsMips(desiredMips > 1 ? desiredMips - 1 : 0);
					{
						int mipDims[3] = { runtimeData.m_size[0], runtimeData.m_size[1], runtimeData.m_size[2] };
						for (int mipIndex = 0; mipIndex < desiredMips - 1; ++mipIndex)
						{
							if (mipIndex == 0)
								MakeMip(allPixels, allPixelsMips[0], pixelsFormatInfo, node.dimension, mipDims);
							else
								MakeMip(allPixelsMips[mipIndex - 1], allPixelsMips[mipIndex], pixelsFormatInfo, node.dimension, mipDims);

							mipDims[0] = max(mipDims[0] / 2, 1);
							mipDims[1] = max(mipDims[1] / 2, 1);
							if (node.dimension == TextureDimensionType::Texture3D)
								mipDims[2] = max(mipDims[2] / 2, 1);
						}
					}

					// copy everything to GPU
					int mipDims[3] = { runtimeData.m_size[0], runtimeData.m_size[1], runtimeData.m_size[2] };
					for (int mipIndex = 0; mipIndex < runtimeData.m_numMips; ++mipIndex)
					{
						std::vector<unsigned char>& srcPixels = (mipIndex > 0)
							? allPixelsMips[mipIndex - 1]
							: allPixels;

						// Set up variables to handle 3d textures (single sub resource) vs other times (a sub resource per 2d texture)
						int arrayCount = 0;
						int iyCount = mipDims[1];
						int izCount = 0;
						if (node.dimension == TextureDimensionType::Texture3D)
						{
							arrayCount = 1;
							izCount = mipDims[2];
						}
						else
						{
							arrayCount = mipDims[2];
							izCount = 1;
						}

						int allPixelsStride = (int)srcPixels.size() / arrayCount;

						for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
						{
							unsigned int arrayIndexStart = allPixelsStride * arrayIndex;

							// Make an upload buffer
							int unalignedPitch = mipDims[0] * pixelsFormatInfo.bytesPerPixel;
							int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);
							UploadBufferTracker::Buffer* uploadBuffer = m_uploadBufferTracker.GetBuffer(m_device, alignedPitch * iyCount * izCount, false);

							// write the pixels into the upload buffer
							{
								unsigned char* destPixels = nullptr;
								HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&destPixels));
								if (hr)
									return false;

								for (int iz = 0; iz < izCount; ++iz)
								{
									for (int iy = 0; iy < iyCount; ++iy)
									{
										const unsigned char* src = &srcPixels[arrayIndexStart + (iz * iyCount + iy) * unalignedPitch];
										unsigned char* dest = &destPixels[(iz * iyCount + iy) * alignedPitch];
										memcpy(dest, src, unalignedPitch);
									}
								}

								uploadBuffer->buffer->Unmap(0, nullptr);
							}

							// Copy the upload buffer into m_resourceInitialState
							{
								UINT subresourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, runtimeData.m_numMips, arrayCount);

								D3D12_RESOURCE_DESC resourceDesc = runtimeData.m_resourceInitialState->GetDesc();
								std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
								D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
								m_device->GetCopyableFootprints(&resourceDesc, subresourceIndex, 1, 0, layout, nullptr, nullptr, nullptr);

								D3D12_TEXTURE_COPY_LOCATION src = {};
								src.pResource = uploadBuffer->buffer;
								src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
								src.PlacedFootprint = *layout;

								D3D12_TEXTURE_COPY_LOCATION dest = {};
								dest.pResource = runtimeData.m_resourceInitialState;
								dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
								dest.SubresourceIndex = subresourceIndex;

								m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
							}
						}

						mipDims[0] = max(mipDims[0] / 2, 1);
						mipDims[1] = max(mipDims[1] / 2, 1);
						if (node.dimension == TextureDimensionType::Texture3D)
							mipDims[2] = max(mipDims[2] / 2, 1);
					}
				}

				// Note that the resource wants to be reset to the initial state.
				runtimeData.m_resourceWantsReset = true;
			}
		}
	}

	return true;
}

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, NodeAction nodeAction)
{
	ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, true);

	if (node.visibility == ResourceVisibility::Imported)
	{
		if (!OnNodeActionImported(node, runtimeData, nodeAction))
			return false;
	}
	else
	{
		if (!OnNodeActionNotImported(node, runtimeData, nodeAction))
			return false;
	}

	if (nodeAction == NodeAction::Execute)
	{
		// See if we should reset the resource to the initial state
		bool resourceWantsReset = runtimeData.m_resourceWantsReset;
		runtimeData.m_resourceWantsReset = false;
		if (node.visibility == ResourceVisibility::Imported)
		{
			ImportedResourceDesc& desc = m_importedResources[node.name];
			resourceWantsReset |= desc.resetEveryFrame;
		}
		else
		{
			// A transient resource doesn't have to be transient, but it's safer to force them to be here, for situations on other platforms where they actually will be.
			resourceWantsReset |= node.transient;
		}

		// Do it if so
		if (resourceWantsReset)
		{
			if (runtimeData.m_resource && runtimeData.m_resourceInitialState)
			{
				// transition the resources as needed
				m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COPY_SOURCE));
				m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				m_transitions.Flush(m_commandList);

				// copy
				m_commandList->CopyResource(runtimeData.m_resource, runtimeData.m_resourceInitialState);
			}
		}

		// publish the initial and current state as viewable textures
		runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(node.dimension), (node.name + ".resource" + " - Initial State").c_str(), runtimeData.m_resourceInitialState, runtimeData.m_format, runtimeData.m_size, runtimeData.m_numMips, true, false);
		runtimeData.HandleViewableTexture(*this, TextureDimensionTypeToViewableResourceType(node.dimension), (node.name + ".resource").c_str(), runtimeData.m_resource, runtimeData.m_format, runtimeData.m_size, runtimeData.m_numMips, false, resourceWantsReset);

		m_transitions.Transition(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}

	return true;
}
