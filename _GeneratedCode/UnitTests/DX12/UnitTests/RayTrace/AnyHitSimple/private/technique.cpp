#include "../public/technique.h"
#include "DX12Utils/dxutils.h"
#include "DX12Utils/DelayedReleaseTracker.h"
#include "DX12Utils/HeapAllocationTracker.h"
#include "DX12Utils/TextureCache.h"

#include <vector>
#include <chrono>

namespace AnyHitSimple
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

    ID3D12StateObject* ContextInternal::rayShader_DoRT_rtso = nullptr;
    ID3D12RootSignature* ContextInternal::rayShader_DoRT_rootSig = nullptr;
    ID3D12Resource* ContextInternal::rayShader_DoRT_shaderTableRayGen = nullptr;
    unsigned int    ContextInternal::rayShader_DoRT_shaderTableRayGenSize = 0;
    ID3D12Resource* ContextInternal::rayShader_DoRT_shaderTableMiss = nullptr;
    unsigned int    ContextInternal::rayShader_DoRT_shaderTableMissSize = 0;
    ID3D12Resource* ContextInternal::rayShader_DoRT_shaderTableHitGroup = nullptr;
    unsigned int    ContextInternal::rayShader_DoRT_shaderTableHitGroupSize = 0;

    template <typename T>
    T Pow2GE(const T& A)
    {
        float f = std::log2(float(A));
        f = std::ceilf(f);
        return (T)std::pow(2.0f, f);
    }

    bool CreateShared(ID3D12Device* device)
    {

        ID3D12Device5* dxrDevice = nullptr;
        device->QueryInterface(IID_PPV_ARGS(&dxrDevice));

        // Ray Shader: DoRT
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // output
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // scene
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // _RayGenCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::rayShader_DoRT_rootSig, (c_debugNames ? L"DoRT" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "MAX_RECURSION_DEPTH", "3" },
                { "RT_HIT_GROUP_COUNT", "1" },
                { nullptr, nullptr }
            };

            // Compile shaders
            std::vector<unsigned char> shaderCode[4];

            // Compile RTMiss : AnyHitSimple.hlsl Miss()
            shaderCode[0] = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/AnyHitSimple.hlsl", "", "lib_6_3", defines, c_debugShaders, Context::LogFn);
            if (shaderCode[0].empty())
                return false;

            // Compile RTClosestHit : AnyHitSimple.hlsl ClosestHit()
            shaderCode[1] = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/AnyHitSimple.hlsl", "", "lib_6_3", defines, c_debugShaders, Context::LogFn);
            if (shaderCode[1].empty())
                return false;

            // Compile RTAnyHit : AnyHitSimple.hlsl AnyHit()
            shaderCode[2] = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/AnyHitSimple.hlsl", "", "lib_6_3", defines, c_debugShaders, Context::LogFn);
            if (shaderCode[2].empty())
                return false;

            // Compile RTRayGen : AnyHitSimple.hlsl RayGen()
            shaderCode[3] = DX12Utils::CompileShaderToByteCode_DXC(Context::s_techniqueLocation.c_str(), L"shaders/AnyHitSimple.hlsl", "", "lib_6_3", defines, c_debugShaders, Context::LogFn);
            if (shaderCode[3].empty())
                return false;

            // Make the state object
            D3D12_STATE_SUBOBJECT subObjects[9];

            D3D12_STATE_OBJECT_DESC soDesc;
            soDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
            soDesc.NumSubobjects = 9;
            soDesc.pSubobjects = subObjects;

            // DXIL Library for RTMiss : AnyHitSimple.hlsl Miss()
            {
                static D3D12_EXPORT_DESC exportDesc;
                exportDesc.Name = L"Miss_0";
                exportDesc.ExportToRename = L"Miss";
                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

                static D3D12_DXIL_LIBRARY_DESC libDesc;
                libDesc.DXILLibrary.BytecodeLength = shaderCode[0].size();
                libDesc.DXILLibrary.pShaderBytecode = shaderCode[0].data();
                libDesc.NumExports = 1;
                libDesc.pExports = &exportDesc;

                subObjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                subObjects[0].pDesc = &libDesc;
            }

            // DXIL Library for RTClosestHit : AnyHitSimple.hlsl ClosestHit()
            {
                static D3D12_EXPORT_DESC exportDesc;
                exportDesc.Name = L"ClosestHit_1";
                exportDesc.ExportToRename = L"ClosestHit";
                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

                static D3D12_DXIL_LIBRARY_DESC libDesc;
                libDesc.DXILLibrary.BytecodeLength = shaderCode[1].size();
                libDesc.DXILLibrary.pShaderBytecode = shaderCode[1].data();
                libDesc.NumExports = 1;
                libDesc.pExports = &exportDesc;

                subObjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                subObjects[1].pDesc = &libDesc;
            }

            // DXIL Library for RTAnyHit : AnyHitSimple.hlsl AnyHit()
            {
                static D3D12_EXPORT_DESC exportDesc;
                exportDesc.Name = L"AnyHit_2";
                exportDesc.ExportToRename = L"AnyHit";
                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

                static D3D12_DXIL_LIBRARY_DESC libDesc;
                libDesc.DXILLibrary.BytecodeLength = shaderCode[2].size();
                libDesc.DXILLibrary.pShaderBytecode = shaderCode[2].data();
                libDesc.NumExports = 1;
                libDesc.pExports = &exportDesc;

                subObjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                subObjects[2].pDesc = &libDesc;
            }

            // DXIL Library for RTRayGen : AnyHitSimple.hlsl RayGen()
            {
                static D3D12_EXPORT_DESC exportDesc;
                exportDesc.Name = L"RayGen_3";
                exportDesc.ExportToRename = L"RayGen";
                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

                static D3D12_DXIL_LIBRARY_DESC libDesc;
                libDesc.DXILLibrary.BytecodeLength = shaderCode[3].size();
                libDesc.DXILLibrary.pShaderBytecode = shaderCode[3].data();
                libDesc.NumExports = 1;
                libDesc.pExports = &exportDesc;

                subObjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                subObjects[3].pDesc = &libDesc;
            }

            // Make the hit group sub objects
            D3D12_HIT_GROUP_DESC hitGroupDescs[1];

            // Hit group: HitGroup
            {
                D3D12_HIT_GROUP_DESC& hitGroupDesc = hitGroupDescs[0];
                hitGroupDesc.HitGroupExport = L"hitgroup0";
                hitGroupDesc.AnyHitShaderImport = L"AnyHit_2";
                hitGroupDesc.ClosestHitShaderImport = L"ClosestHit_1";
                hitGroupDesc.IntersectionShaderImport = nullptr;
                hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
                subObjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
                subObjects[4].pDesc = &hitGroupDesc;
            }

            // Payload
            D3D12_RAYTRACING_SHADER_CONFIG payloadDesc;
            payloadDesc.MaxPayloadSizeInBytes = 64;
            payloadDesc.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;

            subObjects[5].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
            subObjects[5].pDesc = &payloadDesc;

            // Associate payload with shaders
            const WCHAR* shaderExports[] = { L"Miss_0", L"ClosestHit_1", L"AnyHit_2", L"RayGen_3" };

            D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
            shaderPayloadAssociation.NumExports = 4;
            shaderPayloadAssociation.pExports = shaderExports;
            shaderPayloadAssociation.pSubobjectToAssociate = &subObjects[5];

            subObjects[6].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
            subObjects[6].pDesc = &shaderPayloadAssociation;

            // Pipeline Config
            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
            pipelineConfig.MaxTraceRecursionDepth = 3;

            subObjects[7].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
            subObjects[7].pDesc = &pipelineConfig;

            // Global Root Signature
            subObjects[8].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
            subObjects[8].pDesc = &ContextInternal::rayShader_DoRT_rootSig;
            if (FAILED(dxrDevice->CreateStateObject(&soDesc, IID_PPV_ARGS(&ContextInternal::rayShader_DoRT_rtso))))
                return false;

            if (c_debugNames)
                ContextInternal::rayShader_DoRT_rtso->SetName(L"DoRT state object");

            // Create the shader tables
            {
                ID3D12StateObjectProperties* soprops = nullptr;
                if(FAILED(ContextInternal::rayShader_DoRT_rtso->QueryInterface(IID_PPV_ARGS(&soprops))))
                    return false;

                // make the ray gen shader table and fill it out
                {
                    ContextInternal::rayShader_DoRT_shaderTableRayGenSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, (unsigned int)(1 * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));
                    ContextInternal::rayShader_DoRT_shaderTableRayGen = DX12Utils::CreateBuffer(device, ContextInternal::rayShader_DoRT_shaderTableRayGenSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (c_debugNames ? L"DoRT shader table ray gen" : nullptr), nullptr);

                    unsigned char* shaderTableBytes = nullptr;
                    D3D12_RANGE readRange = { 0, 0 };
                    ContextInternal::rayShader_DoRT_shaderTableRayGen->Map(0, &readRange, (void**)&shaderTableBytes);

                    memcpy(shaderTableBytes, soprops->GetShaderIdentifier(L"RayGen_3"), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
                    shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

                    ContextInternal::rayShader_DoRT_shaderTableRayGen->Unmap(0, nullptr);
                }

                // make the miss shader table and fill it out
                {
                    ContextInternal::rayShader_DoRT_shaderTableMissSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, (unsigned int)(1 * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));
                    ContextInternal::rayShader_DoRT_shaderTableMiss = DX12Utils::CreateBuffer(device, ContextInternal::rayShader_DoRT_shaderTableMissSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (c_debugNames ? L"DoRT shader table miss" : nullptr), nullptr);

                    unsigned char* shaderTableBytes = nullptr;
                    D3D12_RANGE readRange = { 0, 0 };
                    ContextInternal::rayShader_DoRT_shaderTableMiss->Map(0, &readRange, (void**)&shaderTableBytes);

                    memcpy(shaderTableBytes, soprops->GetShaderIdentifier(L"Miss_0"), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
                    shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

                    ContextInternal::rayShader_DoRT_shaderTableMiss->Unmap(0, nullptr);
                }

                // make the hit group shader table and fill it out
                {
                    ContextInternal::rayShader_DoRT_shaderTableHitGroupSize = ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, (unsigned int)(1 * D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));
                    ContextInternal::rayShader_DoRT_shaderTableHitGroup = DX12Utils::CreateBuffer(device, ContextInternal::rayShader_DoRT_shaderTableHitGroupSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, (c_debugNames ? L"DoRT shader table hit group" : nullptr), nullptr);

                    unsigned char* shaderTableBytes = nullptr;
                    D3D12_RANGE readRange = { 0, 0 };
                    ContextInternal::rayShader_DoRT_shaderTableHitGroup->Map(0, &readRange, (void**)&shaderTableBytes);

                    memcpy(shaderTableBytes, soprops->GetShaderIdentifier(L"hitgroup0"), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
                    shaderTableBytes += D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

                    ContextInternal::rayShader_DoRT_shaderTableHitGroup->Unmap(0, nullptr);
                }

                soprops->Release();
            }
        }

        dxrDevice->Release();

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

        if(ContextInternal::rayShader_DoRT_rtso)
        {
            s_delayedRelease.Add(ContextInternal::rayShader_DoRT_rtso);
            ContextInternal::rayShader_DoRT_rtso = nullptr;
        }

        if(ContextInternal::rayShader_DoRT_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::rayShader_DoRT_rootSig);
            ContextInternal::rayShader_DoRT_rootSig = nullptr;
        }

        if(ContextInternal::rayShader_DoRT_shaderTableRayGen)
        {
            s_delayedRelease.Add(ContextInternal::rayShader_DoRT_shaderTableRayGen);
            ContextInternal::rayShader_DoRT_shaderTableRayGen = nullptr;
        }

        if(ContextInternal::rayShader_DoRT_shaderTableMiss)
        {
            s_delayedRelease.Add(ContextInternal::rayShader_DoRT_shaderTableMiss);
            ContextInternal::rayShader_DoRT_shaderTableMiss = nullptr;
        }

        if(ContextInternal::rayShader_DoRT_shaderTableHitGroup)
        {
            s_delayedRelease.Add(ContextInternal::rayShader_DoRT_shaderTableHitGroup);
            ContextInternal::rayShader_DoRT_shaderTableHitGroup = nullptr;
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
        return nullptr;
    }

    D3D12_RESOURCE_STATES Context::GetPrimaryOutputTextureState()
    {
        return D3D12_RESOURCE_STATE_COMMON;
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
        range.End = ((1 + 1) * 2) * sizeof(uint64_t);

        uint64_t* timeStampBuffer = nullptr;
        m_internal.m_TimestampReadbackBuffer->Map(0, &range, (void**)&timeStampBuffer);

        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // ray shader: DoRT
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

        if(m_output.texture_Output)
        {
            s_delayedRelease.Add(m_output.texture_Output);
            m_output.texture_Output = nullptr;
        }

        // _RayGenCB
        if (m_internal.constantBuffer__RayGenCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__RayGenCB);
            m_internal.constantBuffer__RayGenCB = nullptr;
        }
    }

    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        // reset the timer index
        s_timerIndex = 0;

        ScopedPerfEvent scopedPerf("AnyHitSimple", commandList, 4);

        std::chrono::high_resolution_clock::time_point startPointCPUTechnique;
        if(context->m_profile)
        {
            startPointCPUTechnique = std::chrono::high_resolution_clock::now();
            if(context->m_internal.m_TimestampQueryHeap == nullptr)
            {
                D3D12_QUERY_HEAP_DESC QueryHeapDesc;
                QueryHeapDesc.Count = (1+1) * 2;
                QueryHeapDesc.NodeMask = 1;
                QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&context->m_internal.m_TimestampQueryHeap));
                if (c_debugNames)
                    context->m_internal.m_TimestampQueryHeap->SetName(L"AnyHitSimple Time Stamp Query Heap");

                context->m_internal.m_TimestampReadbackBuffer = DX12Utils::CreateBuffer(device, sizeof(uint64_t) * (1+1) * 2, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (c_debugNames ? L"AnyHitSimple Time Stamp Query Heap" : nullptr), nullptr);
            }
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
        }

        ID3D12Device5* dxrDevice = nullptr;
        ID3D12GraphicsCommandList4* dxrCommandList = nullptr;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxrDevice))) || FAILED(commandList->QueryInterface(IID_PPV_ARGS(&dxrCommandList))))
        {
            Context::LogFn(LogLevel::Error, "Could not get dxr command list. Ray tracing not supported, but is required.");
            return;
        }

        if (!context->m_input.buffer_Scene_blas)
        {
            Context::LogFn(LogLevel::Error, "AnyHitSimple: Imported buffer \"Scene\" needs a BLAS/TLAS. Use CreateManagedTLAS to create them.\n");
            return;
        }

        if (!context->m_input.buffer_Scene)
        {
            Context::LogFn(LogLevel::Error, "AnyHitSimple: Imported buffer \"Scene\" is null.\n");
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

            if(context->m_input.buffer_Scene_state != D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_Scene;
                barriers[barrierCount].Transition.StateBefore = context->m_input.buffer_Scene_state;
                barriers[barrierCount].Transition.StateAfter = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        // Shader Constants: _RayGenCB
        {
            context->m_internal.constantBuffer__RayGenCB_cpu.CameraPos = context->m_input.variable_CameraPos;
            context->m_internal.constantBuffer__RayGenCB_cpu.InvViewProjMtx = context->m_input.variable_InvViewProjMtx;
            context->m_internal.constantBuffer__RayGenCB_cpu.depthNearPlane = context->m_input.variable_depthNearPlane;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__RayGenCB, context->m_internal.constantBuffer__RayGenCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].UAV.pResource = context->m_output.texture_Output;

            commandList->ResourceBarrier(1, barriers);
        }

        // Ray Shader: DoRT
        {
            ScopedPerfEvent scopedPerf("Ray Shader: DoRT", commandList, 2);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::rayShader_DoRT_rootSig);
            dxrCommandList->SetPipelineState1(ContextInternal::rayShader_DoRT_rtso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_Output, context->m_output.texture_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_input.buffer_Scene, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::RTScene, false, context->m_input.buffer_Scene_tlasSize, 1, 0 },
                { context->m_internal.constantBuffer__RayGenCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_Output_size[0],
                context->m_output.texture_Output_size[1],
                context->m_output.texture_Output_size[2]
            };

            D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
            dispatchDesc.Width = ((baseDispatchSize[0] + 0) * 1) / 1 + 0;
            dispatchDesc.Height = ((baseDispatchSize[1] + 0) * 1) / 1 + 0;
            dispatchDesc.Depth = ((baseDispatchSize[2] + 0) * 1) / 1 + 0;
            dispatchDesc.RayGenerationShaderRecord.StartAddress = ContextInternal::rayShader_DoRT_shaderTableRayGen->GetGPUVirtualAddress();
            dispatchDesc.RayGenerationShaderRecord.SizeInBytes = ContextInternal::rayShader_DoRT_shaderTableRayGenSize;
            if (ContextInternal::rayShader_DoRT_shaderTableMiss)
                dispatchDesc.MissShaderTable.StartAddress = ContextInternal::rayShader_DoRT_shaderTableMiss->GetGPUVirtualAddress();
            dispatchDesc.MissShaderTable.SizeInBytes = ContextInternal::rayShader_DoRT_shaderTableMissSize;
            dispatchDesc.MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
            if (ContextInternal::rayShader_DoRT_shaderTableHitGroup)
                dispatchDesc.HitGroupTable.StartAddress = ContextInternal::rayShader_DoRT_shaderTableHitGroup->GetGPUVirtualAddress();
            dispatchDesc.HitGroupTable.SizeInBytes = ContextInternal::rayShader_DoRT_shaderTableHitGroupSize;
            dispatchDesc.HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

            dxrCommandList->DispatchRays(&dispatchDesc);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "DoRT";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Make sure imported resources are put back in the state they were given to us in
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[1];

            if(context->m_input.buffer_Scene_state != D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_Scene;
                barriers[barrierCount].Transition.StateBefore = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
                barriers[barrierCount].Transition.StateAfter = context->m_input.buffer_Scene_state;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
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

        dxrDevice->Release();
        dxrCommandList->Release();
    }

    void Context::EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        bool dirty = false;

        // Output
        {
            unsigned int baseSize[3] = { 1, 1, 1 };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 512) / 1 + 0,
                ((baseSize[1] + 0) * 512) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            if(!m_output.texture_Output ||
               m_output.texture_Output_size[0] != desiredSize[0] ||
               m_output.texture_Output_size[1] != desiredSize[1] ||
               m_output.texture_Output_size[2] != desiredSize[2] ||
               m_output.texture_Output_numMips != desiredNumMips ||
               m_output.texture_Output_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_Output)
                    s_delayedRelease.Add(m_output.texture_Output);

                m_output.texture_Output = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_Output_flags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"Output" : nullptr), Context::LogFn);
                m_output.texture_Output_size[0] = desiredSize[0];
                m_output.texture_Output_size[1] = desiredSize[1];
                m_output.texture_Output_size[2] = desiredSize[2];
                m_output.texture_Output_numMips = desiredNumMips;
                m_output.texture_Output_format = desiredFormat;
            }
        }

        // _RayGenCB
        if (m_internal.constantBuffer__RayGenCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__RayGenCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_RayGenCB" : nullptr), Context::LogFn);
        }
        EnsureDrawCallPSOsCreated(device, dirty);
    }

    bool Context::EnsureDrawCallPSOsCreated(ID3D12Device* device, bool dirty)
    {
        return true;
    }
};
