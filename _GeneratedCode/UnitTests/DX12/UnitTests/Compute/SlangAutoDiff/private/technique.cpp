#include "../public/technique.h"
#include "DX12Utils/dxutils.h"
#include "DX12Utils/DelayedReleaseTracker.h"
#include "DX12Utils/HeapAllocationTracker.h"
#include "DX12Utils/TextureCache.h"

#include <vector>
#include <chrono>

namespace SlangAutoDiff
{
    static std::vector<Context*> s_allContexts;

    static DX12Utils::Heap                  s_srvHeap;
    static DX12Utils::Heap                  s_rtvHeap;
    static DX12Utils::Heap                  s_dsvHeap;
    static DX12Utils::UploadBufferTracker   s_ubTracker;
    static DX12Utils::DelayedReleaseTracker s_delayedRelease;
    static DX12Utils::HeapAllocationTracker s_heapAllocationTrackerRTV;
    static DX12Utils::HeapAllocationTracker s_heapAllocationTrackerDSV;

    TLogFn Context::LogFn = [] (LogLevel level, const char* msg, ...) {};
    TPerfEventBeginFn Context::PerfEventBeginFn = [] (const char* name, ID3D12GraphicsCommandList* commandList, int index) {};
    TPerfEventEndFn Context::PerfEventEndFn = [] (ID3D12GraphicsCommandList* commandList) {};

    std::wstring Context::s_techniqueLocation = L"./";
    static unsigned int s_timerIndex = 0;

    ID3D12CommandSignature* ContextInternal::s_commandSignatureDispatch = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Initialize_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Initialize_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_GradientDescend_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_GradientDescend_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Render_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Render_rootSig = nullptr;

    template <typename T>
    T Pow2GE(const T& A)
    {
        float f = std::log2(float(A));
        f = std::ceilf(f);
        return (T)std::pow(2.0f, f);
    }

    bool CreateShared(ID3D12Device* device)
    {

        // Compute Shader: Initialize
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[2];

            // Data
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // _Init_0CB
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            if(!DX12Utils::MakeRootSig(device, ranges, 2, samplers, 0, &ContextInternal::computeShader_Initialize_rootSig, (c_debugNames ? L"Initialize" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SlangAutoDiff_Initialize.hlsl";
            shaderCompilationInfo.entryPoint = "csmain";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "Initialize" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_Initialize_rootSig, &ContextInternal::computeShader_Initialize_pso, Context::LogFn))
                return false;
        }

        // Compute Shader: GradientDescend
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[2];

            // Data
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // _Descend_0CB
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            if(!DX12Utils::MakeRootSig(device, ranges, 2, samplers, 0, &ContextInternal::computeShader_GradientDescend_rootSig, (c_debugNames ? L"GradientDescend" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SlangAutoDiff_Descend_0.hlsl";
            shaderCompilationInfo.entryPoint = "csmain";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "GradientDescend" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("SCREEN_MIN","0.001f");
            shaderCompilationInfo.defines.emplace_back("SCREEN_MAX","0.99f");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_GradientDescend_rootSig, &ContextInternal::computeShader_GradientDescend_pso, Context::LogFn))
                return false;
        }

        // Compute Shader: Render
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Data
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // Output
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // _Render_0CB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_Render_rootSig, (c_debugNames ? L"Render" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SlangAutoDiff_Render.hlsl";
            shaderCompilationInfo.entryPoint = "csmain";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "Render" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_Render_rootSig, &ContextInternal::computeShader_Render_pso, Context::LogFn))
                return false;
        }

        // Create heaps
        if (c_numSRVDescriptors > 0 && !CreateHeap(s_srvHeap, device, c_numSRVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, Context::LogFn))
            return false;

        if (c_numRTVDescriptors > 0 && !CreateHeap(s_rtvHeap, device, c_numRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, Context::LogFn))
            return false;

        if (c_numDSVDescriptors > 0 && !CreateHeap(s_dsvHeap, device, c_numDSVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, Context::LogFn))
            return false;

        s_heapAllocationTrackerRTV.Init(s_rtvHeap.m_heap, c_numRTVDescriptors, (int)device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
        s_heapAllocationTrackerDSV.Init(s_dsvHeap.m_heap, c_numDSVDescriptors, (int)device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

        // create indirect dispatch command
        {
            D3D12_INDIRECT_ARGUMENT_DESC dispatchArg = {};
            dispatchArg.Type						 = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

            D3D12_COMMAND_SIGNATURE_DESC dispatchDesc = {};
            dispatchDesc.ByteStride					  = sizeof(uint32_t) * 3;
            dispatchDesc.NumArgumentDescs			  = 1;
            dispatchDesc.pArgumentDescs				  = &dispatchArg;
            dispatchDesc.NodeMask					  = 0x0;

            device->CreateCommandSignature(
                &dispatchDesc,
                nullptr,
                IID_PPV_ARGS(&ContextInternal::s_commandSignatureDispatch));
        }

        return true;
    }

    void DestroyShared()
    {

        if(ContextInternal::computeShader_Initialize_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Initialize_pso);
            ContextInternal::computeShader_Initialize_pso = nullptr;
        }

        if(ContextInternal::computeShader_Initialize_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Initialize_rootSig);
            ContextInternal::computeShader_Initialize_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_GradientDescend_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_GradientDescend_pso);
            ContextInternal::computeShader_GradientDescend_pso = nullptr;
        }

        if(ContextInternal::computeShader_GradientDescend_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_GradientDescend_rootSig);
            ContextInternal::computeShader_GradientDescend_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_Render_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Render_pso);
            ContextInternal::computeShader_Render_pso = nullptr;
        }

        if(ContextInternal::computeShader_Render_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Render_rootSig);
            ContextInternal::computeShader_Render_rootSig = nullptr;
        }

        // Clear out heap trackers
        s_heapAllocationTrackerRTV.Release();
        s_heapAllocationTrackerDSV.Release();

        // Destroy Heaps
        DestroyHeap(s_srvHeap);
        DestroyHeap(s_rtvHeap);
        DestroyHeap(s_dsvHeap);

        // Destroy any upload buffers
        s_ubTracker.Release();

        // Finish any delayed release
        s_delayedRelease.Release();

        // Destroy indirect dispatch command
        if (ContextInternal::s_commandSignatureDispatch)
        {
            ContextInternal::s_commandSignatureDispatch->Release();
            ContextInternal::s_commandSignatureDispatch = nullptr;
        }
    }

    Context* CreateContext(ID3D12Device* device)
    {
        if (s_allContexts.size() == 0)
        {
            if (!CreateShared(device))
                return nullptr;
        }

        Context* ret = new Context;
        s_allContexts.push_back(ret);
        return ret;
    }

    void DestroyContext(Context* context)
    {
        s_allContexts.erase(std::remove(s_allContexts.begin(), s_allContexts.end(), context), s_allContexts.end());
        delete context;
        if (s_allContexts.size() == 0)
            DestroyShared();
    }

    ID3D12Resource* Context::GetPrimaryOutputTexture()
    {
        return m_input.texture_Output;
    }

    D3D12_RESOURCE_STATES Context::GetPrimaryOutputTextureState()
    {
        return m_input.texture_Output_state;
    }

    void OnNewFrame(int framesInFlight)
    {
        s_delayedRelease.OnNewFrame(framesInFlight);
        s_ubTracker.OnNewFrame(framesInFlight);
        s_heapAllocationTrackerRTV.OnNewFrame(framesInFlight);
        s_heapAllocationTrackerDSV.OnNewFrame(framesInFlight);
    }

    int Context::GetContextCount()
    {
        return (int)s_allContexts.size();
    }

    Context* Context::GetContext(int index)
    {
        if (index >= 0 && index < GetContextCount())
            return s_allContexts[index];
        else
            return nullptr;
    }

    ID3D12Resource* Context::CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const void* data, size_t size, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Make a buffer and have the context manage it
        ID3D12Resource* ret = DX12Utils::CreateBuffer(
            device,
            (unsigned int)size,
            flags,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_HEAP_TYPE_DEFAULT,
            debugName,
            Context::LogFn
        );
        AddManagedResource(ret);

        // Copy the data to the buffer if we should
        if (data != nullptr && size > 0)
            UploadBufferData(device, commandList, ret, D3D12_RESOURCE_STATE_COPY_DEST, data, (unsigned int)size);

        // Do a resource transition if we should
        if (desiredState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = ret;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = desiredState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // return the resource
        return ret;
    }

    ID3D12Resource* Context::CreateManagedTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, const void* initialData, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Create a texture
        ID3D12Resource* ret = DX12Utils::CreateTexture(device, size, numMips, format, flags, D3D12_RESOURCE_STATE_COPY_DEST, resourceType, debugName, Context::LogFn);
        AddManagedResource(ret);

        // copy initial data in, if we should
        if (initialData != nullptr)
        {
            DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
            UploadTextureData(device, commandList, ret, D3D12_RESOURCE_STATE_COPY_DEST, initialData, size[0] * formatInfo.bytesPerPixel);
        }

        // Put the resource into the desired state
        if (desiredState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = ret;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = desiredState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        return ret;
    }

    ID3D12Resource* Context::CreateManagedTextureAndClear(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, void* clearValue, size_t clearValueSize, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Make sure the clear value is the correct size
        DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
        if (clearValue != nullptr && clearValueSize > 0 && clearValueSize != formatInfo.bytesPerPixel)
            return nullptr;

        // Copy data into the resource
        std::vector<unsigned char> expandedClearValue;
        void* initialData = nullptr;
        if (clearValue != nullptr && clearValueSize > 0)
        {
            expandedClearValue.resize(size[0] * size[1] * size[2] * formatInfo.bytesPerPixel);
            unsigned char* dest = expandedClearValue.data();
            for (size_t i = 0; i < size[0] * size[1] * size[2]; ++i)
            {
                memcpy(dest, clearValue, formatInfo.bytesPerPixel);
                dest += formatInfo.bytesPerPixel;
            }
            initialData = expandedClearValue.data();
        }

        // make and return the texture
        return CreateManagedTexture(device, commandList, flags, format, size, numMips, resourceType, initialData, debugName, desiredState);
    }

    ID3D12Resource* Context::CreateManagedTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, DX12Utils::ResourceType resourceType, const char* fileName, bool sourceIsSRGB, unsigned int size[3], const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Get the desired channel type
        DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
        DX12Utils::TextureCache::Type desiredChannelType = DX12Utils::TextureCache::Type::U8;
        if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
            desiredChannelType = DX12Utils::TextureCache::Type::U8;
        else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)
            desiredChannelType = DX12Utils::TextureCache::Type::F32;
        else
            return nullptr;

        if (resourceType == DX12Utils::ResourceType::Texture2D)
        {
            // Load the texture and convert as necessary
            DX12Utils::TextureCache::Texture texture = DX12Utils::TextureCache::GetAs(fileName, sourceIsSRGB, desiredChannelType, formatInfo.sRGB, formatInfo.channelCount);
            if (!texture.Valid())
                return nullptr;

            // store off image properties
            size[0] = texture.width;
            size[1] = texture.height;
            size[2] = 1;

            // make and return the texture
            return CreateManagedTexture(device, commandList, flags, format, size, 1, resourceType, texture.pixels.data(), debugName, desiredState);
        }
        else if (resourceType == DX12Utils::ResourceType::Texture2DArray ||
                 resourceType == DX12Utils::ResourceType::Texture3D ||
                 resourceType == DX12Utils::ResourceType::TextureCube)
        {
            static const char* c_cubeMapNames[] =
            {
                "Right",
                "Left",
                "Up",
                "Down",
                "Front",
                "Back"
            };

            bool useCubeMapNames = (resourceType == DX12Utils::ResourceType::TextureCube && strstr(fileName, "%s") != nullptr);
            bool hasPercentI = strstr(fileName, "%i") != nullptr;
            if (!useCubeMapNames && !hasPercentI)
                return nullptr;

            std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;

            // Load multiple textures
            int textureIndex = -1;
            while (1)
            {
                textureIndex++;
                char indexedFileName[1024];

                if (useCubeMapNames)
                    sprintf_s(indexedFileName, fileName, c_cubeMapNames[textureIndex]);
                else
                    sprintf_s(indexedFileName, fileName, textureIndex);

                // Load the texture and convert as necessary
                DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, sourceIsSRGB, desiredChannelType, formatInfo.sRGB, formatInfo.channelCount);
                if (!loadedTextureSlice.Valid())
                {
                    if (textureIndex == 0)
                        return nullptr;
                    break;
                }

                // make sure the textures are the same size
                if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))
                    return nullptr;

                loadedTextureSlices.push_back(loadedTextureSlice);
            }

            // store the texture size
            size[0] = loadedTextureSlices[0].width;
            size[1] = loadedTextureSlices[0].height;
            size[2] = (unsigned int)loadedTextureSlices.size();

            // gather up all pixels into a contiguous chunk of memory
            std::vector<unsigned char> allPixels;
            for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)
                allPixels.insert(allPixels.end(), texture.pixels.begin(), texture.pixels.end());

            // make and return the texture
            return CreateManagedTexture(device, commandList, flags, format, size, 1, resourceType, allPixels.data(), debugName, desiredState);
        }
        else
            return nullptr;
    }

    void Context::UploadTextureData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* texture, D3D12_RESOURCE_STATES textureState, const void* data, unsigned int unalignedPitch)
    {
        // Get information about the texture
        int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);
        D3D12_RESOURCE_DESC textureDesc = texture->GetDesc();

        // transition the resource to copy dest if it isn't already
        if (textureState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = texture;
            barrier.Transition.StateBefore = textureState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // 3d textures do a single copy because it's a single sub resource.
        if (textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            // Get the upload buffer
            DX12Utils::UploadBufferTracker::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * textureDesc.Height * textureDesc.DepthOrArraySize, Context::LogFn, false);

            // Map, copy, unmap
            {
                unsigned char* dest = nullptr;
                D3D12_RANGE readRange = { 0, 0 };
                HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);
                if (FAILED(hr))
                {
                    Context::LogFn(LogLevel::Error, "Could not map upload buffer.");
                }
                else
                {
                    const unsigned char* src = (const unsigned char*)data;
                    for (int iz = 0; iz < textureDesc.DepthOrArraySize; ++iz)
                    {
                        for (int iy = 0; iy < (int)textureDesc.Height; ++iy)
                        {
                            memcpy(dest, src, unalignedPitch);
                            src += unalignedPitch;
                            dest += alignedPitch;
                        }
                    }

                    uploadBuffer->buffer->Unmap(0, nullptr);
                }
            }

            // copy the upload buffer into the texture
            {
                unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;
                device->GetCopyableFootprints(&textureDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

                D3D12_TEXTURE_COPY_LOCATION src = {};
                src.pResource = uploadBuffer->buffer;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = *layout;

                D3D12_TEXTURE_COPY_LOCATION dest = {};
                dest.pResource = texture;
                dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dest.SubresourceIndex = 0;

                commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
            }
        }
        // 2d array textures do a copy for each slice
        else if (textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            for (int iz = 0; iz < textureDesc.DepthOrArraySize; ++iz)
            {
                // Get the upload buffer
                DX12Utils::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * textureDesc.Height, Context::LogFn, false);

                // Map, copy, unmap
                {
                    unsigned char* dest = nullptr;
                    D3D12_RANGE readRange = { 0, 0 };
                    HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);
                    if (FAILED(hr))
                    {
                        Context::LogFn(LogLevel::Error, "Could not map upload buffer.");
                    }
                    else
                    {
                        const unsigned char* src = &((const unsigned char*)data)[unalignedPitch * textureDesc.Height * iz];
                        for (int iy = 0; iy < (int)textureDesc.Height; ++iy)
                        {
                            memcpy(dest, src, unalignedPitch);
                            src += unalignedPitch;
                            dest += alignedPitch;
                        }

                        uploadBuffer->buffer->Unmap(0, nullptr);
                    }
                }

                 // copy the upload buffer into the texture
                 {
                     unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
                     D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;
                     device->GetCopyableFootprints(&textureDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

                     D3D12_TEXTURE_COPY_LOCATION src = {};
                     src.pResource = uploadBuffer->buffer;
                     src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                     src.PlacedFootprint = *layout;

                     D3D12_TEXTURE_COPY_LOCATION dest = {};
                     dest.pResource = texture;
                     dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                     dest.SubresourceIndex = iz;

                     commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
                 }
            }
        }
        else
        {
            Context::LogFn(LogLevel::Error, "Unhandled texture dimension.");
        }

        // transition the resource back to what it was
        if (textureState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = texture;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = textureState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }
    }

    void Context::UploadBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* buffer, D3D12_RESOURCE_STATES bufferState, const void* data, unsigned int dataSize)
    {
        // Get the upload buffer
        DX12Utils::UploadBufferTracker::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, dataSize, Context::LogFn, false);

        // copy cpu data to the upload buffer
        {
            void* start = nullptr;
            HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&start));
            if(hr)
            {
                Context::LogFn(LogLevel::Error, "Could not map upload buffer");
                return;
            }

            memcpy(start, data, dataSize);

            uploadBuffer->buffer->Unmap(0, nullptr);
        }

        // transition the resource to copy dest if it isn't already
        if (bufferState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = buffer;
            barrier.Transition.StateBefore = bufferState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // copy the resource
        commandList->CopyResource(buffer, uploadBuffer->buffer);

        // transition the resource back to what it was
        if (bufferState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = buffer;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = bufferState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }
    }

    int Context::GetRTV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_RTV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName)
    {
        // Make the key
        DX12Utils::SubResourceHeapAllocationInfo key;
        key.resource = resource;
        key.arrayIndex = arrayIndex;
        key.mipIndex = mipIndex;

        // If it already exists, use it
        auto it = m_internal.m_RTVCache.find(key);
        if (it != m_internal.m_RTVCache.end())
            return it->second;

        // Allocate an RTV index
        int rtvIndex = -1;
        if (!s_heapAllocationTrackerRTV.Allocate(rtvIndex, debugName))
            return -1;

        // Create the RTV
        if (!DX12Utils::CreateRTV(device, resource, s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), format, dimension, arrayIndex, mipIndex))
        {
            s_heapAllocationTrackerRTV.Free(rtvIndex);
            return -1;
        }

        // store the result
        m_internal.m_RTVCache[key] = rtvIndex;
        return rtvIndex;
    }

    int Context::GetDSV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_DSV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName)
    {
	    // Make the key
        DX12Utils::SubResourceHeapAllocationInfo key;
        key.resource = resource;
        key.arrayIndex = arrayIndex;
        key.mipIndex = mipIndex;

	    // If it already exists, use it
	    auto it = m_internal.m_DSVCache.find(key);
	    if (it != m_internal.m_DSVCache.end())
            return it->second;

        // Allocate a DSV index
        int dsvIndex = -1;
        if (!s_heapAllocationTrackerDSV.Allocate(dsvIndex, debugName))
            return -1;

        // Create the DSV
        if (!DX12Utils::CreateDSV(device, resource, s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), format, dimension, arrayIndex, mipIndex))
        {
            s_heapAllocationTrackerDSV.Free(dsvIndex);
            return -1;
        }

        // store the result
        m_internal.m_DSVCache[key] = dsvIndex;
        return dsvIndex;
    }

    const ProfileEntry* Context::ReadbackProfileData(ID3D12CommandQueue* commandQueue, int& numItems)
    {
        numItems = 0;

        if (!m_profile || !m_internal.m_TimestampReadbackBuffer)
            return nullptr;

        uint64_t GPUFrequency;
        commandQueue->GetTimestampFrequency(&GPUFrequency);
        double GPUTickDelta = 1.0 / static_cast<double>(GPUFrequency);

        D3D12_RANGE range;
        range.Begin = 0;
        range.End = ((3 + 1) * 2) * sizeof(uint64_t);

        uint64_t* timeStampBuffer = nullptr;
        m_internal.m_TimestampReadbackBuffer->Map(0, &range, (void**)&timeStampBuffer);

        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Initialize
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: GradientDescend
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Render
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+1] - timeStampBuffer[0])); numItems++; // GPU total

        D3D12_RANGE emptyRange = {};
        m_internal.m_TimestampReadbackBuffer->Unmap(0, &emptyRange);

        return m_profileData;
    }

    Context::~Context()
    {
        for (const auto& pair : m_internal.m_RTVCache)
            s_heapAllocationTrackerRTV.Free(pair.second);
        m_internal.m_RTVCache.clear();

        for (const auto& pair : m_internal.m_DSVCache)
            s_heapAllocationTrackerDSV.Free(pair.second);
        m_internal.m_DSVCache.clear();

        for (ID3D12Resource* resource : m_internal.m_managedResources)
            resource->Release();
        m_internal.m_managedResources.clear();

        if(m_internal.m_TimestampQueryHeap)
        {
            m_internal.m_TimestampQueryHeap->Release();
            m_internal.m_TimestampQueryHeap = nullptr;
        }

        if(m_internal.m_TimestampReadbackBuffer)
        {
            m_internal.m_TimestampReadbackBuffer->Release();
            m_internal.m_TimestampReadbackBuffer = nullptr;
        }

        // first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
        if(m_internal.buffer_Data)
        {
            s_delayedRelease.Add(m_internal.buffer_Data);
            m_internal.buffer_Data = nullptr;
        }

        // _Init_0CB
        if (m_internal.constantBuffer__Init_0CB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__Init_0CB);
            m_internal.constantBuffer__Init_0CB = nullptr;
        }

        // _Render_0CB
        if (m_internal.constantBuffer__Render_0CB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__Render_0CB);
            m_internal.constantBuffer__Render_0CB = nullptr;
        }

        // _Descend_0CB
        if (m_internal.constantBuffer__Descend_0CB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__Descend_0CB);
            m_internal.constantBuffer__Descend_0CB = nullptr;
        }
    }

    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        // reset the timer index
        s_timerIndex = 0;

        ScopedPerfEvent scopedPerf("SlangAutoDiff", commandList, 8);

        std::chrono::high_resolution_clock::time_point startPointCPUTechnique;
        if(context->m_profile)
        {
            startPointCPUTechnique = std::chrono::high_resolution_clock::now();
            if(context->m_internal.m_TimestampQueryHeap == nullptr)
            {
                D3D12_QUERY_HEAP_DESC QueryHeapDesc;
                QueryHeapDesc.Count = (3+1) * 2;
                QueryHeapDesc.NodeMask = 1;
                QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&context->m_internal.m_TimestampQueryHeap));
                if (c_debugNames)
                    context->m_internal.m_TimestampQueryHeap->SetName(L"SlangAutoDiff Time Stamp Query Heap");

                context->m_internal.m_TimestampReadbackBuffer = DX12Utils::CreateBuffer(device, sizeof(uint64_t) * (3+1) * 2, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (c_debugNames ? L"SlangAutoDiff Time Stamp Query Heap" : nullptr), nullptr);
            }
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
        }

        if (!context->m_input.texture_Output)
        {
            Context::LogFn(LogLevel::Error, "SlangAutoDiff: Imported texture \"Output\" is null.\n");
            return;
        }

        // Make sure internally owned resources are created and are the right size and format
        context->EnsureResourcesCreated(device, commandList);

        // set the heaps
        ID3D12DescriptorHeap* heaps[] =
        {
            s_srvHeap.m_heap,
        };
        commandList->SetDescriptorHeaps(_countof(heaps), heaps);

        // Make sure imported resources are in the correct state
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[1];

            if(context->m_input.texture_Output_state != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.texture_Output;
                barriers[barrierCount].Transition.StateBefore = context->m_input.texture_Output_state;
                barriers[barrierCount].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }
            else
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].UAV.pResource = context->m_input.texture_Output;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        // Shader Constants: _Init_0CB
        {
            context->m_internal.constantBuffer__Init_0CB_cpu.FrameIndex = context->m_input.variable_FrameIndex;
            context->m_internal.constantBuffer__Init_0CB_cpu.MouseState = context->m_input.variable_MouseState;
            context->m_internal.constantBuffer__Init_0CB_cpu.MouseStateLastFrame = context->m_input.variable_MouseStateLastFrame;
            context->m_internal.constantBuffer__Init_0CB_cpu.NumGaussians = context->m_input.variable_NumGaussians;
            context->m_internal.constantBuffer__Init_0CB_cpu.iResolution = context->m_input.variable_iResolution;
            context->m_internal.constantBuffer__Init_0CB_cpu.initialized = context->m_input.variable_initialized;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__Init_0CB, context->m_internal.constantBuffer__Init_0CB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_internal.buffer_Data;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, barriers);
        }

        // Compute Shader: Initialize
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Initialize", commandList, 1);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Initialize_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Initialize_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_internal.buffer_Data, context->m_internal.buffer_Data_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Buffer, false, context->m_internal.buffer_Data_stride, context->m_internal.buffer_Data_count, 0, 0, 0, false },
                { context->m_internal.constantBuffer__Init_0CB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0, 0, 0, false }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 2, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = { 1, 1, 1 };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 1 - 1) / 1,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 1 - 1) / 1,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Initialize";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _Render_0CB
        {
            context->m_internal.constantBuffer__Render_0CB_cpu.NumGaussians = context->m_input.variable_NumGaussians;
            context->m_internal.constantBuffer__Render_0CB_cpu.QuantizeDisplay = context->m_input.variable_QuantizeDisplay;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__Render_0CB, context->m_internal.constantBuffer__Render_0CB_cpu, Context::LogFn);
        }

        // Shader Constants: _Descend_0CB
        {
            context->m_internal.constantBuffer__Descend_0CB_cpu.LearningRate = context->m_input.variable_LearningRate;
            context->m_internal.constantBuffer__Descend_0CB_cpu.MaximumStepSize = context->m_input.variable_MaximumStepSize;
            context->m_internal.constantBuffer__Descend_0CB_cpu.NumGaussians = context->m_input.variable_NumGaussians;
            context->m_internal.constantBuffer__Descend_0CB_cpu.UseBackwardAD = context->m_input.variable_UseBackwardAD;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__Descend_0CB, context->m_internal.constantBuffer__Descend_0CB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].UAV.pResource = context->m_internal.buffer_Data;

            commandList->ResourceBarrier(1, barriers);
        }

        // Compute Shader: GradientDescend
        {
            ScopedPerfEvent scopedPerf("Compute Shader: GradientDescend", commandList, 4);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_GradientDescend_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_GradientDescend_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_internal.buffer_Data, context->m_internal.buffer_Data_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Buffer, false, context->m_internal.buffer_Data_stride, context->m_internal.buffer_Data_count, 0, 0, 0, false },
                { context->m_internal.constantBuffer__Descend_0CB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0, 0, 0, false }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 2, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = { 1, 1, 1 };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 1 - 1) / 1,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 1 - 1) / 1,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "GradientDescend";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_internal.buffer_Data;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, barriers);
        }

        // Compute Shader: Render
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Render", commandList, 2);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Render_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Render_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_internal.buffer_Data, context->m_internal.buffer_Data_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Buffer, false, context->m_internal.buffer_Data_stride, context->m_internal.buffer_Data_count, 0, 0, 0, false },
                { context->m_input.texture_Output, context->m_input.texture_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0, 0, 0, false },
                { context->m_internal.constantBuffer__Render_0CB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0, 0, 0, false }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_input.texture_Output_size[0],
                context->m_input.texture_Output_size[1],
                context->m_input.texture_Output_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Render";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Make sure imported resources are put back in the state they were given to us in
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[1];

            if(context->m_input.texture_Output_state != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.texture_Output;
                barriers[barrierCount].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                barriers[barrierCount].Transition.StateAfter = context->m_input.texture_Output_state;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }
            else
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].UAV.pResource = context->m_input.texture_Output;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        if(context->m_profile)
        {
            context->m_profileData[(s_timerIndex-1)/2].m_label = "Total";
            context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPUTechnique).count();
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            commandList->ResolveQueryData(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, s_timerIndex, context->m_internal.m_TimestampReadbackBuffer, 0);
        }

        // Set variables
        context->m_input.variable_initialized = !context->m_input.variable_Reset;
        if(!context->m_input.variable_initialized)
            context->m_input.variable_Reset = false;
    }

    void Context::EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        bool dirty = false;

        // Data
        // first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
        {

            unsigned int baseCount = (unsigned int)m_input.variable_NumGaussians;
            unsigned int desiredCount = ((baseCount + 0 ) * 5) / 1 + 4;
            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R32_FLOAT;
            unsigned int desiredStride = 0;

            if(!m_internal.buffer_Data ||
               m_internal.buffer_Data_count != desiredCount ||
               m_internal.buffer_Data_format != desiredFormat ||
               m_internal.buffer_Data_stride != desiredStride)
            {
                dirty = true;
                if(m_internal.buffer_Data)
                    s_delayedRelease.Add(m_internal.buffer_Data);

                unsigned int desiredSize = desiredCount * ((desiredStride > 0) ? desiredStride : DX12Utils::Get_DXGI_FORMAT_Info(desiredFormat, Context::LogFn).bytesPerPixel);

                m_internal.buffer_Data = DX12Utils::CreateBuffer(device, desiredSize, m_internal.c_buffer_Data_flags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"Data" : nullptr), Context::LogFn);
                m_internal.buffer_Data_count = desiredCount;
                m_internal.buffer_Data_format = desiredFormat;
                m_internal.buffer_Data_stride = desiredStride;
            }
        }

        // _Init_0CB
        if (m_internal.constantBuffer__Init_0CB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__Init_0CB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_Init_0CB" : nullptr), Context::LogFn);
        }

        // _Render_0CB
        if (m_internal.constantBuffer__Render_0CB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__Render_0CB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_Render_0CB" : nullptr), Context::LogFn);
        }

        // _Descend_0CB
        if (m_internal.constantBuffer__Descend_0CB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__Descend_0CB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_Descend_0CB" : nullptr), Context::LogFn);
        }
        EnsureDrawCallPSOsCreated(device, dirty);
    }

    bool Context::EnsureDrawCallPSOsCreated(ID3D12Device* device, bool dirty)
    {
        return true;
    }
};
