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

//shader
#include "MipComputeHLSL.h"
#include <DX12Utils/ShaderCompilerCommon.h>
#include <DX12Utils/CompileShaders.h>
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

bool RuntimeTypes::RenderGraphNode_Resource_Texture::GetDSV(ID3D12Device2* device, D3D12_CPU_DESCRIPTOR_HANDLE& handle, HeapAllocationTracker& DSVHeapAllocationTracker, TextureDimensionType dimension, int arrayIndex, int mipLevel, int sampleCount, const char* resourceName)
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
	sprintf_s(DSVName, "%s DSV (%i,%i,s:%i)", resourceName, arrayIndex, mipLevel, sampleCount);
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
			assert(sampleCount == 1);
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = mipLevel;
			break;
		}
		case TextureDimensionType::Texture2DArray:
		{
			assert(sampleCount == 1);
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.MipSlice = mipLevel;
			dsvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
			dsvDesc.Texture2DArray.ArraySize = 1;
			break;
		}
		case TextureDimensionType::Texture2DMS:
		{
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
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

bool RuntimeTypes::RenderGraphNode_Resource_Texture::GetRTV(ID3D12Device2* device, D3D12_CPU_DESCRIPTOR_HANDLE& handle, HeapAllocationTracker& RTVHeapAllocationTracker, TextureDimensionType dimension, int arrayIndex, int mipLevel, int sampleCount, const char* resourceName)
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
	sprintf_s(DSVName, "%s RTV (%i,%i,s:%i)", resourceName, arrayIndex, mipLevel, sampleCount);
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
			assert(sampleCount == 1);
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = mipLevel;
			rtvDesc.Texture2D.PlaneSlice = 0;
			break;
		}
		case TextureDimensionType::Texture2DArray:
		{
			assert(sampleCount == 1);
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = mipLevel;
			rtvDesc.Texture2DArray.PlaneSlice = 0;
			rtvDesc.Texture2DArray.ArraySize = 1;
			rtvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
			break;
		}
		case TextureDimensionType::Texture3D:
		{
			assert(sampleCount == 1);
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			rtvDesc.Texture3D.MipSlice = mipLevel;
			rtvDesc.Texture3D.WSize = 1;
			rtvDesc.Texture3D.FirstWSlice = arrayIndex;
			break;
		}
		case TextureDimensionType::Texture2DMS:
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			break;
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

    if (m_rootSignature)
    {
        interpreter.m_delayedRelease.Add(m_rootSignature);
        m_rootSignature = nullptr;
    }

    if (m_pso2d)
    {
        interpreter.m_delayedRelease.Add(m_pso2d);
        m_pso2d = nullptr;
    }

    if (m_pso3d)
    {
        interpreter.m_delayedRelease.Add(m_pso3d);
        m_pso3d = nullptr;
    }

	for (auto it : m_dsvIndices)
		interpreter.m_DSVHeapAllocationTracker.Free(it.second);
	m_dsvIndices.clear();

	for (auto it : m_rtvIndices)
		interpreter.m_RTVHeapAllocationTracker.Free(it.second);
	m_rtvIndices.clear();

	m_failed = false;
}

bool ConvertPixelData(TextureCache::Texture& texture, DXGI_FORMAT newFormat)
{
	// If same format, nothing to do
	if (texture.format == newFormat || newFormat == DXGI_FORMAT_FORCE_UINT)
		return true;

    DXGI_FORMAT_Info textureFormatInfo = Get_DXGI_FORMAT_Info(texture.format);
    DXGI_FORMAT_Info newFormatInfo = Get_DXGI_FORMAT_Info(newFormat);

    // if either format is unknown, we can't do it
    if (textureFormatInfo.format == DXGI_FORMAT_UNKNOWN || newFormatInfo.format == DXGI_FORMAT_UNKNOWN)
        return false;

	// We don't do conversion on compressed image formats
	// We do allow implicit conversion between compatible compressed types though.
	if (textureFormatInfo.isCompressed || newFormatInfo.isCompressed)
	{
		DXGI_FORMAT fmtA = min(texture.format, newFormat);
		DXGI_FORMAT fmtB = max(texture.format, newFormat);

        if (fmtA == DXGI_FORMAT_BC1_UNORM && fmtB == DXGI_FORMAT_BC1_UNORM_SRGB)
            return true;

		if (fmtA == DXGI_FORMAT_BC7_UNORM && fmtB == DXGI_FORMAT_BC7_UNORM_SRGB)
			return true;

		if (fmtA == DXGI_FORMAT_BC4_UNORM && fmtB == DXGI_FORMAT_BC4_SNORM)
			return true;

		if (fmtA == DXGI_FORMAT_BC5_UNORM && fmtB == DXGI_FORMAT_BC5_SNORM)
			return true;

		if (fmtA == DXGI_FORMAT_BC6H_UF16 && fmtB == DXGI_FORMAT_BC6H_SF16)
			return true;

		return false;
	}

	// Process each slice
	for (int iz = 0; iz < texture.images.size(); ++iz)
	{
		// Convert to doubles
		std::vector<double> textureDoubles;
		if (!ConvertToDoubles(texture.images[iz].pixels, textureFormatInfo.channelType, textureDoubles))
			return false;

		// do sRGB conversion if we should
		if (textureFormatInfo.sRGB != newFormatInfo.sRGB)
		{
			int channelsToConvert = min(textureFormatInfo.channelCount, 3); // don't convert alpha

			if (textureFormatInfo.sRGB)
			{
				size_t index = 0;
				while (index < textureDoubles.size())
				{
					for (int channel = 0; channel < channelsToConvert; ++channel)
						textureDoubles[index + channel] = SRGBToLinear((float)textureDoubles[index + channel]);
					index += textureFormatInfo.channelCount;
				}
			}
			else
			{
				size_t index = 0;
				while (index < textureDoubles.size())
				{
					for (int channel = 0; channel < channelsToConvert; ++channel)
						textureDoubles[index + channel] = LinearTosRGB((float)textureDoubles[index + channel]);
					index += textureFormatInfo.channelCount;
				}
			}
		}

		// Add or remove channels as needed
		if (textureFormatInfo.channelCount != newFormatInfo.channelCount)
		{
			size_t pixelCount = textureDoubles.size() / textureFormatInfo.channelCount;
			std::vector<double> padded(pixelCount * newFormatInfo.channelCount);

			const double* src = textureDoubles.data();
			double* dest = padded.data();

			for (size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
			{
				if (textureFormatInfo.channelCount < newFormatInfo.channelCount)
				{
					memcpy(dest, src, sizeof(double) * textureFormatInfo.channelCount);
					for (int channelIndex = textureFormatInfo.channelCount; channelIndex < newFormatInfo.channelCount; ++channelIndex)
						dest[channelIndex] = (channelIndex == 3) ? 1.0 : 0.0;
				}
				else
				{
					memcpy(dest, src, sizeof(double) * newFormatInfo.channelCount);
				}
				src += textureFormatInfo.channelCount;
				dest += newFormatInfo.channelCount;
			}

			textureDoubles = padded;
		}

		// Convert from doubles to the destination format
		if (!ConvertFromDoubles(textureDoubles, newFormatInfo.channelType, texture.images[iz].pixels))
			return false;
	}

	// Set the new format and return success
	texture.format = newFormat;
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

static void MakeMip(GigiInterpreterPreviewWindowDX12& interpreter, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, TextureDimensionType dimension)
{
    const int srcDims[3] = { runtimeData.m_size[0], runtimeData.m_size[1], runtimeData.m_size[2] };

    //make root sig
    if (!runtimeData.m_rootSignature)
    {
        // Descriptor table
        //[0] = input(u0), [1] = output(u1), [2] = CB(b0)
        D3D12_DESCRIPTOR_RANGE ranges[3] = {};

        ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        ranges[0].NumDescriptors = 1;
        ranges[0].BaseShaderRegister = 0;
        ranges[0].RegisterSpace = 0;
        ranges[0].OffsetInDescriptorsFromTableStart = 0;

        ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        ranges[1].NumDescriptors = 1;
        ranges[1].BaseShaderRegister = 1;
        ranges[1].RegisterSpace = 0;
        ranges[1].OffsetInDescriptorsFromTableStart = 1;

        ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        ranges[2].NumDescriptors = 1;
        ranges[2].BaseShaderRegister = 0;
        ranges[2].RegisterSpace = 0;
        ranges[2].OffsetInDescriptorsFromTableStart = 2;

        // Root parameter
        D3D12_ROOT_PARAMETER rootParam;
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParam.DescriptorTable.NumDescriptorRanges = 3;
        rootParam.DescriptorTable.pDescriptorRanges = ranges;

        D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
        rootDesc.NumParameters = 1;
        rootDesc.pParameters = &rootParam;
        rootDesc.NumStaticSamplers = 0;
        rootDesc.pStaticSamplers = nullptr;
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        // Create it
        ID3DBlob* sig = nullptr;
        ID3DBlob* error = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &error);
        if (FAILED(hr))
        {
            const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
            if (errorMsg)
                interpreter.GetLogFn()(LogLevel::Error, "Could not serialize root signature: %s", errorMsg);
            if (sig) sig->Release();
            if (error) error->Release();
            return;
        }

        hr = device->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&runtimeData.m_rootSignature));
        if (FAILED(hr))
        {
            const char* errorMsg = (error ? (const char*)error->GetBufferPointer() : nullptr);
            if (errorMsg)
                interpreter.GetLogFn()(LogLevel::Error, "Could not create root signature: %s", errorMsg);
            if (sig) sig->Release();
            if (error) error->Release();
            return;
        }

        if (sig)
            sig->Release();

        if (error)
            error->Release();

        runtimeData.m_rootSignature->SetName(L"MipCompute Root Signature");
    }

    //make pso2d
    if (!runtimeData.m_pso2d)
    {
        std::string fullFileName = (std::filesystem::path(interpreter.GetTempDirectory()) / "__GIGI__" / "MipCompute.hlsl").string();

        // write the shader to a temporary file, since we don't have an interface to compile a string yet
        std::filesystem::create_directories(std::filesystem::path(fullFileName).remove_filename());
        FILE* file = nullptr;
        fopen_s(&file, fullFileName.c_str(), "wb");
        if (!file)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not open file for writing: \"%s\"", fullFileName.c_str());
            return;
        }
        fwrite(s_MipComputeHLSL, 1, strlen(s_MipComputeHLSL), file);
        fclose(file);

        // Shader compilation depends on which shader compiler they opted to use for this technique
        ShaderCompilationInfo shaderCompilationInfo;
        shaderCompilationInfo.fileName = fullFileName;
        shaderCompilationInfo.entryPoint = "main2D";
        shaderCompilationInfo.shaderModel = "cs_6_1";
        shaderCompilationInfo.debugName = "__FUNCTION__";

        shaderCompilationInfo.flags |= ShaderCompilationFlags::WarningsAsErrors;

        // Turn on for debugging
        if (false)
        {
            shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.flags |= ShaderCompilationFlags::CreatePDBsAndBinaries;
        }

        MakeComputePSO_dxc(device, shaderCompilationInfo, runtimeData.m_rootSignature, &runtimeData.m_pso2d, interpreter.GetLogFn());

        if (!runtimeData.m_pso2d)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not create PSO2D in " __FUNCTION__);
            return;
        }

        runtimeData.m_pso2d->SetName(L"MipCompute PSO2D");
    }

    //make pso3d
    if (!runtimeData.m_pso3d)
    {
        std::string fullFileName = (std::filesystem::path(interpreter.GetTempDirectory()) / "__GIGI__" / "MipCompute.hlsl").string();

        // write the shader to a temporary file, since we don't have an interface to compile a string yet
        std::filesystem::create_directories(std::filesystem::path(fullFileName).remove_filename());
        FILE* file = nullptr;
        fopen_s(&file, fullFileName.c_str(), "wb");
        if (!file)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not open file for writing: \"%s\"", fullFileName.c_str());
            return;
        }
        fwrite(s_MipComputeHLSL, 1, strlen(s_MipComputeHLSL), file);
        fclose(file);

        // Shader compilation depends on which shader compiler they opted to use for this technique
        ShaderCompilationInfo shaderCompilationInfo;
        shaderCompilationInfo.fileName = fullFileName;
        shaderCompilationInfo.entryPoint = "main3D";
        shaderCompilationInfo.shaderModel = "cs_6_1";
        shaderCompilationInfo.debugName = "__FUNCTION__";

        shaderCompilationInfo.flags |= ShaderCompilationFlags::WarningsAsErrors;

        // Turn on for debugging
        if (false)
        {
            shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.flags |= ShaderCompilationFlags::CreatePDBsAndBinaries;
        }

        MakeComputePSO_dxc(device, shaderCompilationInfo, runtimeData.m_rootSignature, &runtimeData.m_pso3d, interpreter.GetLogFn());

        if (!runtimeData.m_pso3d)
        {
            interpreter.GetLogFn()(LogLevel::Error, "Could not create PSO3D in " __FUNCTION__);
            return;
        }

        runtimeData.m_pso3d->SetName(L"MipCompute PSO3D");
    }

    // Convert from TextureDimensionType to ResourceType
    DescriptorTableCache::ResourceType resourceType = DescriptorTableCache::ResourceType::Texture2D;
    switch (dimension)
    {
        case TextureDimensionType::Texture2D: resourceType = DescriptorTableCache::ResourceType::Texture2D; break;
        case TextureDimensionType::Texture2DArray: resourceType = DescriptorTableCache::ResourceType::Texture2DArray; break;
        case TextureDimensionType::Texture3D: resourceType = DescriptorTableCache::ResourceType::Texture3D; break;
        case TextureDimensionType::TextureCube: resourceType = DescriptorTableCache::ResourceType::TextureCube; break;
        case TextureDimensionType::Texture2DMS: resourceType = DescriptorTableCache::ResourceType::Texture2DMS; break;
        default:
        {
            interpreter.GetLogFn()(LogLevel::Error, "Unhandled texture dimension type \"%s\" in " __FUNCTION__, EnumToString(dimension));
            return;
        }
    }

    bool is3D = (dimension == TextureDimensionType::Texture3D);

    TransitionTracker& transitionTracker = interpreter.GetTransitionsNonConst();
    MipCompute_CBStruct cb;
    size_t numArrayIndices = is3D ? 1 : runtimeData.m_size[2];
    for (size_t arrayIndex = 0; arrayIndex < numArrayIndices; arrayIndex++)
    {
        for (uint8_t mipLevel = 1; mipLevel < runtimeData.m_numMips; mipLevel++)
        {
            transitionTracker.Transition(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, "MakeMip m_resourceInitialState");
            transitionTracker.Flush(commandList);

            cb.isSRGBFormat = Get_DXGI_FORMAT_Info(runtimeData.m_format).sRGB;

            UploadBufferTracker::Buffer* cbBuffer = interpreter.getUploadBufferTracker().GetBufferT<MipCompute_CBStruct>(device, true, cb);

            DescriptorTableCache::ResourceDescriptor descriptors[3] = {};

            descriptors[0].m_resource = runtimeData.m_resourceInitialState;
            descriptors[0].m_format = runtimeData.m_format;
            descriptors[0].m_access = DescriptorTableCache::AccessType::UAV;
            descriptors[0].m_resourceType = resourceType;
            descriptors[0].m_UAVMipIndex = mipLevel - 1;
            descriptors[0].m_UAVFirstArraySlice = (UINT)arrayIndex;
            descriptors[0].m_count = runtimeData.m_size[2]; //3d

            descriptors[1].m_resource = runtimeData.m_resourceInitialState;
            descriptors[1].m_format = runtimeData.m_format;
            descriptors[1].m_access = DescriptorTableCache::AccessType::UAV;
            descriptors[1].m_resourceType = resourceType;
            descriptors[1].m_UAVMipIndex = mipLevel;
            descriptors[1].m_UAVFirstArraySlice = (UINT)arrayIndex;
            descriptors[1].m_count = runtimeData.m_size[2]; //3d

            descriptors[2].m_resource = cbBuffer->buffer;
            descriptors[2].m_format = DXGI_FORMAT_UNKNOWN;
            descriptors[2].m_access = DescriptorTableCache::AccessType::CBV;
            descriptors[2].m_resourceType = DescriptorTableCache::ResourceType::Buffer;
            descriptors[2].m_stride = (UINT)cbBuffer->size;
            descriptors[2].m_count = 1;

            //descriptor table
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = {};
            std::string error;
            if (!interpreter.getDescriptorTableCache().GetDescriptorTable(device, interpreter.getSRVHeapAllocationTracker(), descriptors, _countof(descriptors), descriptorTable, error, HEAP_DEBUG_TEXT()))
            {
                interpreter.GetLogFn()(LogLevel::Error, "Could not allocate descriptor table in MakeMip : %s", error.c_str());
                return;
            }

            //setup pipeline and table
            commandList->SetComputeRootSignature(runtimeData.m_rootSignature);
            commandList->SetPipelineState(is3D ? runtimeData.m_pso3d : runtimeData.m_pso2d);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            //dispatch based on dst size
            uint32_t dstW = max(1, runtimeData.m_size[0] >> mipLevel);
            uint32_t dstH = max(1, runtimeData.m_size[1] >> mipLevel);
            uint32_t dstD = max(1, runtimeData.m_size[2] >> mipLevel);

            auto ceilDiv = [](uint32_t a, uint32_t b) { return (a + b - 1u) / b; };
            uint32_t dispatchX = ceilDiv(dstW, 8);
            uint32_t dispatchY = ceilDiv(dstH, 8);
            uint32_t dispatchZ = is3D ? dstD : 1; // because numthreads.z = 1

            commandList->Dispatch(dispatchX, dispatchY, dispatchZ);
        }
    }

    transitionTracker.Transition(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, "MakeMip m_resourceInitialState");
}

static TextureCache::Texture LoadTextureFromBinaryFile(FileCache& fileCache, const char* fileName_, TextureDimensionType dimension, int width, int height, int depth, TextureFormat textureFormat)
{
	// normalize the filename by making it canonical and making it lower case
	std::filesystem::path p = std::filesystem::weakly_canonical(fileName_);
	std::string s = p.string();
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	const char* fileName = s.c_str();

	// init the texture object to an invalid result
	TextureCache::Texture newTexture;
	newTexture.fileName = fileName;

	// Get the file data off disk
	FileCache::File fileData = fileCache.Get(s.c_str());
	if (!fileData.Valid())
		return newTexture;

	// validate format
	DXGI_FORMAT format = TextureFormatToDXGI_FORMAT(textureFormat);
	if (format == DXGI_FORMAT_UNKNOWN || format == DXGI_FORMAT_FORCE_UINT)
		return newTexture;

	// Calculate unpadded sizes
	DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(format);
	const int c_rowSize = width * formatInfo.bytesPerPixel;
	const int c_sliceSize = c_rowSize * height;
	const int c_imageSize = c_sliceSize * depth;

	// Calculate what the size of the image would be with mips, and how many mips there would be
	bool is3D = (dimension == TextureDimensionType::Texture3D);
	int imageSizeWithoutMips = c_imageSize;
	int imageSizeWithMips = 0;
	int mipCount = 0;
	{
		if (!is3D)
		{
			int mipDims[2] = { width, height };
			while (1)
			{
				mipCount++;
				imageSizeWithMips += mipDims[1] * mipDims[0] * formatInfo.bytesPerPixel;

				if (mipDims[0] == 1 && mipDims[1] == 1)
					break;

				mipDims[0] = max(mipDims[0] / 2, 1);
				mipDims[1] = max(mipDims[1] / 2, 1);
			}

			imageSizeWithMips *= depth;
		}
		else
		{
			int mipDims[3] = { width, height, depth };
			while (1)
			{
				mipCount++;
				imageSizeWithMips += mipDims[2] * mipDims[1] * mipDims[0] * formatInfo.bytesPerPixel;

				if (mipDims[0] == 1 && mipDims[1] == 1 && mipDims[2] == 1)
					break;

				mipDims[0] = max(mipDims[0] / 2, 1);
				mipDims[1] = max(mipDims[1] / 2, 1);
				mipDims[2] = max(mipDims[2] / 2, 1);
			}
		}
	}

	// The file size on disk either has to be the size of the image with mips, or without mips.
	if (fileData.GetSize() == imageSizeWithoutMips)
		mipCount = 1;
	else if (fileData.GetSize() != imageSizeWithMips)
		return newTexture;

	// Fill out the texture data
	newTexture.width = width;
	newTexture.height = height;
	newTexture.depth = depth;
	newTexture.mips = mipCount;
	newTexture.format = format;

	int arrayCount = is3D ? 1 : depth;

	// copy pixel data
	const char* src = fileData.GetBytes();
	int imageCount = 0;
	newTexture.images.resize(newTexture.depth * mipCount);
	for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
	{
		int mipDims[3] = { width, height, is3D ? depth : 1 };
		for (int mipIndex = 0; mipIndex < mipCount; ++mipIndex)
		{
			int mipSliceSize = mipDims[0] * mipDims[1] * formatInfo.bytesPerPixel;

			TextureCache::Image& image = newTexture.images[imageCount];
			imageCount++;

			for (size_t iz = 0; iz < mipDims[2]; ++iz)
			{
				size_t copyOffset = image.pixels.size();

				image.pixels.resize(copyOffset + mipSliceSize);
				memcpy(&image.pixels[copyOffset], src, mipSliceSize);

				src += mipSliceSize;
			}

			mipDims[0] = max(mipDims[0] / 2, 1);
			mipDims[1] = max(mipDims[1] / 2, 1);

			if (is3D)
				mipDims[2] = max(mipDims[2] / 2, 1);
		}
	}

	// I'm being lazy about calculating how many 2d slices a 3d texture with mips needs
	if (imageCount < newTexture.images.size())
		newTexture.images.resize(imageCount);

	// return success
	return newTexture;
}

bool GigiInterpreterPreviewWindowDX12::LoadTexture(std::vector<TextureCache::Texture>& loadedTextures, const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, std::string fileName, bool fileIsSRGB, const ImportedTextureBinaryDesc& binaryDesc, DXGI_FORMAT desiredFormat)
{
	if (!FileNameSafe(fileName.c_str()))
		return false;

	bool useCubeMapNames = (fileName.find("%s") != std::string::npos);

	int textureIndex = -1;
	while (1)
	{
		// make the next file name
		textureIndex++;

		// If we are supposed to use cube map names, but ran out of cube map names, we are done.
		if (useCubeMapNames && textureIndex >= 6)
			break;

		// make the next file name
		char indexedFileName[1024];
		if (useCubeMapNames)
			sprintf_s(indexedFileName, fileName.c_str(), c_cubeMapNames[textureIndex]);
		else
			sprintf_s(indexedFileName, fileName.c_str(), textureIndex);

		// Load the file
		TextureCache::Texture& texture = m_textures.Get(m_files, indexedFileName);
		if (texture.Valid())
		{
			// We need to combine slices of 3d textures, or split apart slices of 2d texture arrays
			if (node.dimension == TextureDimensionType::Texture3D)
			{
				loadedTextures.push_back(texture);
				loadedTextures[0].Ensure3D();
			}
			else
			{
				bool success = texture.SplitByIndex([&loadedTextures](const TextureCache::Texture& texture)
					{
						loadedTextures.push_back(texture);
					}
				);
				if (!success)
					return false;
			}
		}
		// If the file couldn't be loaded, try as a binary file
		else if (textureIndex == 0)
		{
			if (binaryDesc.format != TextureFormat::Any)
			{
				TextureCache::Texture binaryTexture = LoadTextureFromBinaryFile(m_files, indexedFileName, node.dimension, binaryDesc.size[0], binaryDesc.size[1], binaryDesc.size[2], binaryDesc.format);
				if (!binaryTexture.Valid())
					return false;

				// We need to combine slices of 3d textures, or split apart slices of 2d texture arrays
				if (node.dimension == TextureDimensionType::Texture3D)
				{
					loadedTextures.push_back(binaryTexture);
					loadedTextures[0].Ensure3D();
				}
				else
				{
					bool success = binaryTexture.SplitByIndex([&loadedTextures](const TextureCache::Texture& texture)
						{
							loadedTextures.push_back(texture);
						}
					);

					if (!success)
						return false;
				}
			}
			else
				return false;
		}
		// it's ok to fail after the first image, since for indexed files, the numbers have to stop at some point!
		else
		{
			break;
		}

		// Watch this file for changes
		m_fileWatcher.Add(indexedFileName, FileWatchOwner::TextureCache);

		// Verify that all loaded textures are the same size and format
		if (textureIndex > 0)
		{
			if (loadedTextures[0].width != texture.width || loadedTextures[0].height != texture.height && texture.format != loadedTextures[0].format)
			{
				m_logFn(LogLevel::Error, "Texture \"%s\" is the wrong size or type", indexedFileName);
				return false;
			}
		}

		// If there is no %i or %s in the filename, break. We have found the only file.
		if (fileName.find("%i") == std::string::npos && fileName.find("%s") == std::string::npos)
			break;
	}

	// Ensure that cube maps have 6 images loaded
	if (node.dimension == TextureDimensionType::TextureCube)
	{
		if (loadedTextures.size() != 6)
		{
			m_logFn(LogLevel::Error, "Cube map \"%s\" does not have 6 images, it has %i", node.name.c_str(), (int)loadedTextures.size());
			return false;
		}
	}

	// Force Texture2D to have one texture loaded max. This can happen if you load a dds into a texture 2d, since a dds can have multiple images inside of it,
	// that get flattened into multiple loadedTextures
	if (node.dimension == TextureDimensionType::Texture2D && loadedTextures.size() > 1)
		loadedTextures.resize(1);

	// Translate format types based on sRGB flag
	{
		DXGI_FORMAT format = loadedTextures[0].format;
		if (fileIsSRGB)
		{
			switch (format)
			{
				case DXGI_FORMAT_R8G8B8A8_UNORM: format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
				case DXGI_FORMAT_BC7_UNORM: format = DXGI_FORMAT_BC7_UNORM_SRGB; break;

			}
		}
		else
		{
			switch (format)
			{
				case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
				case DXGI_FORMAT_BC7_UNORM_SRGB: format = DXGI_FORMAT_BC7_UNORM; break;
			}
		}
		for (TextureCache::Texture& texture : loadedTextures)
			texture.format = format;
	}

	// Do format conversion if needed
	for (TextureCache::Texture& texture : loadedTextures)
	{
        if (!ConvertPixelData(texture, desiredFormat))
        {
            DXGI_FORMAT_Info textureFormatInfo = Get_DXGI_FORMAT_Info(texture.format);
            DXGI_FORMAT_Info newFormatInfo = Get_DXGI_FORMAT_Info(desiredFormat);
            m_logFn(LogLevel::Error, "Could not convert texture from format \"%s\" to \"%s\"", textureFormatInfo.name, newFormatInfo.name);
            
            return false;
        }
	}

	// Smoosh all the slices of a 3d texture together if needed.
	// This happens if you load a 3d texture from indexed images (filenames with %i in them)
	if ((node.dimension == TextureDimensionType::Texture3D) && loadedTextures.size() > 1)
	{
		TextureCache::Texture& firstTexture = loadedTextures[0];
		for (size_t textureIndex = 1; textureIndex < loadedTextures.size(); ++textureIndex)
		{
			const TextureCache::Texture& texture = loadedTextures[textureIndex];

			for (size_t imageIndex = 0; imageIndex < firstTexture.images.size(); ++imageIndex)
			{
				TextureCache::Image& firstImage = firstTexture.images[imageIndex];
				const TextureCache::Image& image = texture.images[imageIndex];

				size_t copyOffset = firstImage.pixels.size();
				size_t copySize = image.pixels.size();

				firstImage.pixels.resize(copyOffset + copySize);
				memcpy(&firstImage.pixels[copyOffset], image.pixels.data(), copySize);
			}

			firstTexture.depth += texture.depth;
		}
		loadedTextures.resize(1);
	}

	return true;
}

bool GigiInterpreterPreviewWindowDX12::CreateAndUploadTextures(const RenderGraphNode_Resource_Texture& node, RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeData, std::vector<TextureCache::Texture>& loadedTextures)
{
    bool imageLoadedFromDisk = (node.visibility == ResourceVisibility::Imported || !node.loadFileName.empty());

	// Validate texture usage
	bool hasMips = loadedTextures[0].images.size() > 1;
	bool wantsMips = runtimeData.m_numMips > 1;
	bool wantsToMakeMips = wantsMips && !hasMips && imageLoadedFromDisk;
	{
		DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(runtimeData.m_format);

		// We don't make mips for compressed texture formats
		if (wantsToMakeMips && formatInfo.isCompressed)
		{
			m_logFn(LogLevel::Error, "Cannot make mips for a compressed texture format. Node \"%s\"", node.name.c_str());
			return false;
		}

		// only let proper depth formats be allowed for use as a depth texture
		if (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::DepthTarget))
		{
			if (!formatInfo.isDepth)
			{
				m_logFn(LogLevel::Error, "Cannot create because texture is used as a depth target, but is not a depth format. Node \"%s\"", node.name.c_str());
				return false;
			}
		}

		// UAV with stencil is not allowed
		if (formatInfo.isStencil && (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV)))
		{
			m_logFn(LogLevel::Error, "Cannot create because texture is a stencil format, but is used with UAV access. This is not allowed. Node \"%s\"", node.name.c_str());
			return false;
		}

		// UAV with compressed texture types is not allowed
		if (formatInfo.isCompressed && (node.accessedAs & (1 << (unsigned int)ShaderResourceAccessType::UAV)))
		{
			m_logFn(LogLevel::Error, "Cannot create because texture is a compressed format, but is used with UAV access. This is not allowed. Node \"%s\"", node.name.c_str());
			return false;
		}

		if (runtimeData.m_size[2] > 1 && node.dimension == TextureDimensionType::Texture2D)
		{
			m_logFn(LogLevel::Error, "Cannot create because size.z is > 1 (%i), but node is a texture 2d. Node \"%s\"", runtimeData.m_size[2], node.name.c_str());
			return false;
		}
	}

	// Make a resource for the "live" texture which may be modified during running, and also for the initial state.
	D3D12_RESOURCE_FLAGS resourceFlags = ShaderResourceAccessToD3D12_RESOURCE_FLAGs(node.accessedAs);

    if (wantsToMakeMips) resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	unsigned int size[3] = { (unsigned int)runtimeData.m_size[0], (unsigned int)runtimeData.m_size[1], (unsigned int)runtimeData.m_size[2] };
	std::string nodeNameInitialState = node.name + " Initial State";
    
    const unsigned int sampleCount = (node.dimension == TextureDimensionType::Texture2DMS) ? node.msaaSettings.sampleCount : 1;

	runtimeData.m_resourceInitialState = CreateTexture(m_device, size, runtimeData.m_numMips, runtimeData.m_format, sampleCount, resourceFlags, D3D12_RESOURCE_STATE_COPY_DEST, TextureDimensionTypeToResourceType(node.dimension), nodeNameInitialState.c_str());
	runtimeData.m_resource = CreateTexture(m_device, size, runtimeData.m_numMips, runtimeData.m_format, sampleCount, resourceFlags, D3D12_RESOURCE_STATE_COPY_DEST, TextureDimensionTypeToResourceType(node.dimension), node.name.c_str());

	// track the new resources for state transitions
	m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resourceInitialState, D3D12_RESOURCE_STATE_COPY_DEST));
	m_transitions.Track(TRANSITION_DEBUG_INFO(runtimeData.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));

    if (!runtimeData.m_resource)
        return false;

    // copy everything to GPU
    {
        D3D12_RESOURCE_DESC resourceDesc = runtimeData.m_resource->GetDesc();

        // MSAA preview is not implemented yet.
        // In DX12 CopyTextureRegion does not work on MSAA textures
        const bool MSAA = (node.dimension == TextureDimensionType::Texture2DMS);

        if (!MSAA)
        {
            int arrayCount = (int)loadedTextures.size();
            int loadedMipCount = (int)loadedTextures[0].mips;
            int resourceMipCount = (int)runtimeData.m_numMips;

            for (int arrayIndex = 0; arrayIndex < arrayCount; ++arrayIndex)
            {
                for (int mipIndex = 0; mipIndex < loadedMipCount; ++mipIndex)
                {
                    // Calculate the subresource index
                    unsigned int subResourceIndex = D3D12CalcSubresource(mipIndex, arrayIndex, 0, resourceMipCount, arrayCount);

                    // gather stats about the current sub resource
                    std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)));
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
                    unsigned int numRows = 0;
                    size_t unalignedPitch = 0;
                    size_t totalSize = 0;
                    m_device->GetCopyableFootprints(&resourceDesc, subResourceIndex, 1, 0, layout, &numRows, &unalignedPitch, &totalSize);
                    size_t alignedPitch = layout->Footprint.RowPitch;
                    size_t bufferSize = numRows * alignedPitch * layout->Footprint.Depth;


                    // Allocate an upload buffer
                    UploadBufferTracker::Buffer* uploadBuffer = m_uploadBufferTracker.GetBuffer(m_device, bufferSize, false);

                    // map the memory
                    D3D12_RANGE readRange;
                    readRange.Begin = 1;
                    readRange.End = 0;
                    unsigned char* destPixels = nullptr;
                    HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, reinterpret_cast<void**>(&destPixels));
                    if (FAILED(hr))
                        return false;

                    // Copy the texture data
                    for (unsigned int iz = 0; iz < layout->Footprint.Depth; ++iz)
                    {
                        for (unsigned int iy = 0; iy < numRows; ++iy)
                        {
                            size_t srcBegin = (iz * numRows + iy) * unalignedPitch;
                            size_t srcEnd = srcBegin + unalignedPitch;

                            if (loadedTextures[arrayIndex].images[mipIndex].pixels.size() < srcEnd)
                                return false;

                            const unsigned char* src = &loadedTextures[arrayIndex].images[mipIndex].pixels[srcBegin];
                            unsigned char* dest = &destPixels[(iz * numRows + iy) * alignedPitch];
                            memcpy(dest, src, unalignedPitch);
                        }
                    }

                    // unmap the memory
                    uploadBuffer->buffer->Unmap(0, nullptr);

                    // Copy the upload buffer into m_resourceInitialState
                    {
                        D3D12_TEXTURE_COPY_LOCATION src = {};
                        src.pResource = uploadBuffer->buffer;
                        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                        src.PlacedFootprint = *layout;

                        D3D12_TEXTURE_COPY_LOCATION dest = {};
                        dest.pResource = runtimeData.m_resourceInitialState;
                        dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                        dest.SubresourceIndex = subResourceIndex;

                        m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
                    }
                }
            }
        }
    }

    // Make the mips if we should
    if (wantsToMakeMips)
        MakeMip(*s_interpreter, m_device, m_commandList, runtimeData, node.dimension);

    return true;
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

		if (desc.state == ImportedResourceState::failed)
		{
			runtimeData.m_renderGraphText = "\nCreation Failed";
            runtimeData.m_inErrorState = true;
			return true;
		}

		// (Re)Create a texture, as necessary
		// if the texture isn't dirty, nothing to do
		// if the format is "any", wait until it isn't
		if (desc.state == ImportedResourceState::dirty && desc.texture.format != TextureFormat::Any)
		{
			// Release any resources which may have previously existed
			runtimeData.Release(*this);

			std::vector<TextureCache::Texture> loadedTextures;

			// Load a file from disk if we have a file name
			if (!desc.texture.fileName.empty())
			{
				// Load the textures
				DXGI_FORMAT format = TextureFormatToDXGI_FORMAT(desc.texture.format);
				if (!LoadTexture(loadedTextures, node, runtimeData, desc.texture.fileName, desc.texture.fileIsSRGB, desc.texture.binaryDesc, format) || loadedTextures.size() == 0)
				{
					m_logFn(LogLevel::Error, "Could not load texture \"%s\" for node \"%s\"", desc.texture.fileName.c_str(), node.name.c_str());
					runtimeData.m_failed = true;
					desc.state = ImportedResourceState::failed;
					return true;
				}
			}
			// else make a solid color texture
			else if (desc.texture.size[0] > 0 && desc.texture.size[1] > 0 && desc.texture.size[2] > 0)
			{
				// make a new 2D texture
				TextureCache::Texture newTexture;
				newTexture.width = desc.texture.size[0];
				newTexture.height = desc.texture.size[1];
				newTexture.mips = 1;
				newTexture.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				newTexture.images.resize(1);

				if ((node.dimension == TextureDimensionType::Texture3D))
				{
					newTexture.depth = desc.texture.size[2];

					// Fill it with solid color
					newTexture.images[0].pixels.resize(newTexture.width * newTexture.height * newTexture.depth * sizeof(float) * 4);
					for (size_t i = 0; i < desc.texture.size[0] * desc.texture.size[1] * desc.texture.size[2]; ++i)
						memcpy(&newTexture.images[0].pixels[i * sizeof(float) * 4], desc.texture.color, sizeof(float) * 4);

					// Convert it to the format we want
					if (!ConvertPixelData(newTexture, TextureFormatToDXGI_FORMAT(desc.texture.format)))
					{
                        DXGI_FORMAT_Info textureFormatInfo = Get_DXGI_FORMAT_Info(newTexture.format);
                        DXGI_FORMAT_Info newFormatInfo = Get_DXGI_FORMAT_Info(TextureFormatToDXGI_FORMAT(desc.texture.format));
                        m_logFn(LogLevel::Error, "Could not load convert texture from format \"%s\" to \"%s\"", textureFormatInfo.name, newFormatInfo.name);

						runtimeData.m_failed = true;
						desc.state = ImportedResourceState::failed;
						return true;
					}

					// Add it
					loadedTextures.push_back(newTexture);
				}
				else
				{
					newTexture.depth = 1;

					// Fill it with solid color
					newTexture.images[0].pixels.resize(newTexture.width * newTexture.height * sizeof(float) * 4);
					for (size_t i = 0; i < desc.texture.size[0] * desc.texture.size[1]; ++i)
						memcpy(&newTexture.images[0].pixels[i * sizeof(float) * 4], desc.texture.color, sizeof(float) * 4);

					// Convert it to the format we want
					if (!ConvertPixelData(newTexture, TextureFormatToDXGI_FORMAT(desc.texture.format)))
					{
                        DXGI_FORMAT_Info textureFormatInfo = Get_DXGI_FORMAT_Info(newTexture.format);
                        DXGI_FORMAT_Info newFormatInfo = Get_DXGI_FORMAT_Info(TextureFormatToDXGI_FORMAT(desc.texture.format));
                        m_logFn(LogLevel::Error, "Could not load convert texture from format \"%s\" to \"%s\"", textureFormatInfo.name, newFormatInfo.name);

						runtimeData.m_failed = true;
						desc.state = ImportedResourceState::failed;
						return true;
					}

					// Repeatedly add this texture to fulfill "depth" aka desc.texture.size[2]
					for (int iz = 0; iz < desc.texture.size[2]; ++iz)
						loadedTextures.push_back(newTexture);
				}
			}
			// otherwise, we have nothing to go on
			else
			{
				runtimeData.m_failed = true;
				desc.state = ImportedResourceState::failed;
				return true;
			}

			// Fill out the runtime data
			bool is3D = (node.dimension == TextureDimensionType::Texture3D);
			runtimeData.m_size[0] = loadedTextures[0].width;
			runtimeData.m_size[1] = loadedTextures[0].height;
			runtimeData.m_size[2] = is3D ? loadedTextures[0].depth : (int)loadedTextures.size();
			runtimeData.m_format = loadedTextures[0].format;
			runtimeData.m_failed = false;

			// Calculate mip count if needed
			runtimeData.m_numMips = (int)loadedTextures[0].mips;
			if (desc.texture.makeMips)
			{
				int maxSize = max(runtimeData.m_size[0], runtimeData.m_size[1]);

				if (node.dimension == TextureDimensionType::Texture3D)
					maxSize = max(maxSize, runtimeData.m_size[2]);

				runtimeData.m_numMips = 1;
				while (maxSize > 1)
				{
					maxSize /= 2;
					runtimeData.m_numMips++;
				}
			}

			if (!CreateAndUploadTextures(node, runtimeData, loadedTextures))
			{
				m_logFn(LogLevel::Error, "Could not create resource for node \"%s\"", node.name.c_str());
				runtimeData.m_failed = true;
				desc.state = ImportedResourceState::failed;
				return false;
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
        bool wantsAutoFormat = IsDesiredFormatAny(*this, node);
		DXGI_FORMAT desiredFormat = GetDesiredFormat(*this, node);
		IVec3 desiredSize = GetDesiredSize(*this, node);

		bool hasSize = desiredSize[0] != 0 && desiredSize[1] != 0 && desiredSize[2] != 0;
		bool hasFileName = !node.loadFileName.empty() && FileNameSafe(node.loadFileName.c_str());
		std::string fullLoadFileName = GetTempDirectory() + "assets\\" + node.loadFileName;

        bool formatOK = (desiredFormat != DXGI_FORMAT_FORCE_UINT) || wantsAutoFormat;

		if (!(formatOK && (hasSize || hasFileName) && !runtimeData.m_failed))
		{
			std::ostringstream ss;
			ss << "Texture can't be created:";

			if (desiredFormat == DXGI_FORMAT_FORCE_UINT)
			{
				ss << "\nCould not determine desired format of texture";
			}

			if (!(hasSize || hasFileName))
			{
				ss << "\nCould not determine size of texture";
				if (!hasFileName && desiredSize[2] == 0)
					ss << "\nThe z component of the texture size is 0, should be > 0.";
			}

			if (runtimeData.m_failed)
			{
				ss << "\nCreation Failed";
			}

			runtimeData.m_renderGraphText = ss.str();
            runtimeData.m_inErrorState = true;

			return true;
		}

		std::vector<TextureCache::Texture> loadedTextures;

		// if we have a filename, our size comes from the image file
		if (hasFileName)
		{
			// If we already have a resource made, we know the size
			if (runtimeData.m_resource)
			{
				desiredSize[0] = runtimeData.m_size[0];
				desiredSize[1] = runtimeData.m_size[1];
				desiredSize[2] = runtimeData.m_size[2];
			}
			// else we load the texture and get the size from that
			else
			{
				// Load the textures
				ImportedTextureBinaryDesc binaryDesc;
                if (!LoadTexture(loadedTextures, node, runtimeData, fullLoadFileName, node.loadFileNameAsSRGB, binaryDesc, desiredFormat) || loadedTextures.size() == 0)
                {
                    std::ostringstream ss;
                    ss << "Could not load texture \"" << fullLoadFileName << "\"";
                    runtimeData.m_renderGraphText = ss.str();
                    runtimeData.m_inErrorState = true;
                    return true;
                }

                if (desiredFormat == DXGI_FORMAT_FORCE_UINT)
                    desiredFormat = loadedTextures[0].format;

				// Set the desired size, now that we know it
				desiredSize[0] = loadedTextures[0].width;
				desiredSize[1] = loadedTextures[0].height;
				desiredSize[2] = (node.dimension == TextureDimensionType::Texture3D) ? loadedTextures[0].depth : (int)loadedTextures.size();
			}
		}

		// Calculate desired mip count
		int desiredMips = (int)node.numMips;
		if (hasFileName)
		{
			if (runtimeData.m_resource)
				desiredMips = runtimeData.m_numMips;
			if (loadedTextures.size() > 0 && loadedTextures[0].images.size() > 1)
				desiredMips = (int)loadedTextures[0].mips;
		}
		if (desiredMips == 0)
		{
			int maxSize = max(desiredSize[0], desiredSize[1]);

			if (node.dimension == TextureDimensionType::Texture3D)
				maxSize = max(maxSize, runtimeData.m_size[2]);

			desiredMips = 1;
			while (maxSize > 1)
			{
				maxSize /= 2;
				desiredMips++;
			}
		}
		const unsigned int desiredSampleCount = (node.dimension == TextureDimensionType::Texture2DMS) ? node.msaaSettings.sampleCount : 1;

		// (re) create the resource if we should
		if (!runtimeData.m_resource || runtimeData.m_format != desiredFormat || runtimeData.m_size[0] != desiredSize[0] || runtimeData.m_size[1] != desiredSize[1] || runtimeData.m_size[2] != desiredSize[2] || runtimeData.m_numMips != desiredMips || runtimeData.sampleCount != desiredSampleCount)
		{
			// Release any resources which may have previously existed
			runtimeData.Release(*this);

			// set the format and size
			runtimeData.m_format = desiredFormat;
			runtimeData.m_size[0] = desiredSize[0];
			runtimeData.m_size[1] = desiredSize[1];
			runtimeData.m_size[2] = desiredSize[2];
			runtimeData.m_numMips = desiredMips;
            runtimeData.sampleCount = desiredSampleCount;

			// If we don't have a filename, make textures to spec and zero initialize
			if (!hasFileName)
			{
				DXGI_FORMAT_Info desiredFormatInfo = Get_DXGI_FORMAT_Info(desiredFormat);

				TextureCache::Texture texture;
				texture.width = desiredSize[0];
				texture.height = desiredSize[1];
				texture.mips = 1;
				texture.format = desiredFormat;
				texture.images.resize(1);

				if (node.dimension == TextureDimensionType::Texture3D)
				{
					texture.depth = desiredSize[2];
					texture.images[0].pixels.resize(texture.width* texture.height* texture.depth* desiredFormatInfo.bytesPerPixel, 0);
					loadedTextures.push_back(texture);
				}
				else
				{
					texture.depth = 1;
					texture.images[0].pixels.resize(texture.width * texture.height * desiredFormatInfo.bytesPerPixel, 0);
					for (int iz = 0; iz < desiredSize[2]; ++iz)
						loadedTextures.push_back(texture);
				}
			}

			if (!CreateAndUploadTextures(node, runtimeData, loadedTextures))
			{
				m_logFn(LogLevel::Error, "Could not create resource for node \"%s\"", node.name.c_str());
				runtimeData.m_failed = true;
				return true;
			}

			// Note that the resource wants to be reset to the initial state.
			runtimeData.m_resourceWantsReset = true;
		}

        if (!node.loadFileName.empty())
        {
            std::ostringstream ss;
            ss << "Loaded from \"" << node.loadFileName << "\"";
            runtimeData.m_renderGraphText = ss.str();
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
