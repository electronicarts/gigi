#include "../public/technique.h"
#include "DX12Utils/dxutils.h"
#include "DX12Utils/DelayedReleaseTracker.h"
#include "DX12Utils/HeapAllocationTracker.h"
#include "DX12Utils/TextureCache.h"

#include <vector>
#include <chrono>

namespace SubGraphLoops
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

    ID3D12PipelineState* ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_rootSig = nullptr;

    template <typename T>
    T Pow2GE(const T& A)
    {
        float f = std::log2(float(A));
        f = std::ceilf(f);
        return (T)std::pow(2.0f, f);
    }

    bool CreateShared(ID3D12Device* device)
    {

        // Compute Shader: FilterSub_Iteration_0_DoBlur
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Input
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

            // _FilterSub_Iteration_0_BlurCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_rootSig, (c_debugNames ? L"FilterSub_Iteration_0_DoBlur" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SubGraphLoopsInner_FilterSub Iteration 0/SubGraphLoopsBlur.hlsl";
            shaderCompilationInfo.entryPoint = "main";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "FilterSub_Iteration_0_DoBlur" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_rootSig, &ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_pso, Context::LogFn))
                return false;
        }

        // Compute Shader: FilterSub_Iteration_1_DoBlur
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Input
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

            // _FilterSub_Iteration_1_BlurCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_rootSig, (c_debugNames ? L"FilterSub_Iteration_1_DoBlur" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SubGraphLoopsInner_FilterSub Iteration 1/SubGraphLoopsBlur.hlsl";
            shaderCompilationInfo.entryPoint = "main";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "FilterSub_Iteration_1_DoBlur" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_rootSig, &ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_pso, Context::LogFn))
                return false;
        }

        // Compute Shader: FilterSub_Iteration_2_DoBlur
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Input
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

            // _FilterSub_Iteration_2_BlurCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_rootSig, (c_debugNames ? L"FilterSub_Iteration_2_DoBlur" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SubGraphLoopsInner_FilterSub Iteration 2/SubGraphLoopsBlur.hlsl";
            shaderCompilationInfo.entryPoint = "main";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "FilterSub_Iteration_2_DoBlur" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_rootSig, &ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_pso, Context::LogFn))
                return false;
        }

        // Compute Shader: FilterSub_Iteration_3_DoBlur
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Input
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

            // _FilterSub_Iteration_3_BlurCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_rootSig, (c_debugNames ? L"FilterSub_Iteration_3_DoBlur" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SubGraphLoopsInner_FilterSub Iteration 3/SubGraphLoopsBlur.hlsl";
            shaderCompilationInfo.entryPoint = "main";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "FilterSub_Iteration_3_DoBlur" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_rootSig, &ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_pso, Context::LogFn))
                return false;
        }

        // Compute Shader: FilterSub_Iteration_4_DoBlur
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Input
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

            // _FilterSub_Iteration_4_BlurCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_rootSig, (c_debugNames ? L"FilterSub_Iteration_4_DoBlur" : nullptr), Context::LogFn))
                return false;

            ShaderCompilationInfo shaderCompilationInfo;
            shaderCompilationInfo.fileName = std::filesystem::path(Context::s_techniqueLocation) / "shaders" / "SubGraphLoopsInner_FilterSub Iteration 4/SubGraphLoopsBlur.hlsl";
            shaderCompilationInfo.entryPoint = "main";
            shaderCompilationInfo.shaderModel = "cs_6_1";
            shaderCompilationInfo.debugName = (c_debugNames ? "FilterSub_Iteration_4_DoBlur" : "");
            if (c_debugShaders) shaderCompilationInfo.flags |= ShaderCompilationFlags::Debug;
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchMultiply","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchDivide","uint3(1,1,1)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPreAdd","uint3(0,0,0)");
            shaderCompilationInfo.defines.emplace_back("__GigiDispatchPostAdd","uint3(0,0,0)");

            if(!DX12Utils::MakeComputePSO_DXC(device, shaderCompilationInfo,
               ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_rootSig, &ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_pso, Context::LogFn))
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

        if(ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_pso);
            ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_pso = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_rootSig);
            ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_pso);
            ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_pso = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_rootSig);
            ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_pso);
            ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_pso = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_rootSig);
            ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_pso);
            ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_pso = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_rootSig);
            ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_pso);
            ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_pso = nullptr;
        }

        if(ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_rootSig);
            ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_rootSig = nullptr;
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
        return m_output.texture_FilterSub_Iteration_4_Output;
    }

    D3D12_RESOURCE_STATES Context::GetPrimaryOutputTextureState()
    {
        return m_output.c_texture_FilterSub_Iteration_4_Output_endingState;
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
        range.End = ((10 + 1) * 2) * sizeof(uint64_t);

        uint64_t* timeStampBuffer = nullptr;
        m_internal.m_TimestampReadbackBuffer->Map(0, &range, (void**)&timeStampBuffer);

        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: FilterSub_Iteration_0_DoBlur
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // copy resource: FilterSub_Iteration_0_Copy_back
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: FilterSub_Iteration_1_DoBlur
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // copy resource: FilterSub_Iteration_1_Copy_back
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: FilterSub_Iteration_2_DoBlur
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // copy resource: FilterSub_Iteration_2_Copy_back
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: FilterSub_Iteration_3_DoBlur
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // copy resource: FilterSub_Iteration_3_Copy_back
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: FilterSub_Iteration_4_DoBlur
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // copy resource: FilterSub_Iteration_4_Copy_back
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

        if(m_output.texture_FilterSub_Iteration_0_Output)
        {
            s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_0_Output);
            m_output.texture_FilterSub_Iteration_0_Output = nullptr;
        }

        if(m_output.texture_FilterSub_Iteration_1_Output)
        {
            s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_1_Output);
            m_output.texture_FilterSub_Iteration_1_Output = nullptr;
        }

        if(m_output.texture_FilterSub_Iteration_2_Output)
        {
            s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_2_Output);
            m_output.texture_FilterSub_Iteration_2_Output = nullptr;
        }

        if(m_output.texture_FilterSub_Iteration_3_Output)
        {
            s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_3_Output);
            m_output.texture_FilterSub_Iteration_3_Output = nullptr;
        }

        if(m_output.texture_FilterSub_Iteration_4_Output)
        {
            s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_4_Output);
            m_output.texture_FilterSub_Iteration_4_Output = nullptr;
        }

        // _FilterSub_Iteration_0_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB);
            m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB = nullptr;
        }

        // _FilterSub_Iteration_1_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB);
            m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB = nullptr;
        }

        // _FilterSub_Iteration_2_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB);
            m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB = nullptr;
        }

        // _FilterSub_Iteration_3_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB);
            m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB = nullptr;
        }

        // _FilterSub_Iteration_4_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB);
            m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB = nullptr;
        }
    }

    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        // reset the timer index
        s_timerIndex = 0;

        ScopedPerfEvent scopedPerf("SubGraphLoops", commandList, 21);

        std::chrono::high_resolution_clock::time_point startPointCPUTechnique;
        if(context->m_profile)
        {
            startPointCPUTechnique = std::chrono::high_resolution_clock::now();
            if(context->m_internal.m_TimestampQueryHeap == nullptr)
            {
                D3D12_QUERY_HEAP_DESC QueryHeapDesc;
                QueryHeapDesc.Count = (10+1) * 2;
                QueryHeapDesc.NodeMask = 1;
                QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&context->m_internal.m_TimestampQueryHeap));
                if (c_debugNames)
                    context->m_internal.m_TimestampQueryHeap->SetName(L"SubGraphLoops Time Stamp Query Heap");

                context->m_internal.m_TimestampReadbackBuffer = DX12Utils::CreateBuffer(device, sizeof(uint64_t) * (10+1) * 2, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (c_debugNames ? L"SubGraphLoops Time Stamp Query Heap" : nullptr), nullptr);
            }
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
        }

        if (!context->m_input.texture_Input)
        {
            Context::LogFn(LogLevel::Error, "SubGraphLoops: Imported texture \"Input\" is null.\n");
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

            if(context->m_input.texture_Input_state != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.texture_Input;
                barriers[barrierCount].Transition.StateBefore = context->m_input.texture_Input_state;
                barriers[barrierCount].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        // Shader Constants: _FilterSub_Iteration_0_BlurCB
        {
            context->m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB_cpu.FilterSub_Iteration_0_sRGB = context->m_input.variable_FilterSub_Iteration_0_sRGB;
            context->m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB_cpu.__loopIndexValue_0 = context->m_internal.variable___loopIndexValue_0;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB, context->m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.texture_FilterSub_Iteration_0_Output;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, barriers);
        }

        // Compute Shader: FilterSub_Iteration_0_DoBlur
        {
            ScopedPerfEvent scopedPerf("Compute Shader: FilterSub_Iteration_0_DoBlur", commandList, 1);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_FilterSub_Iteration_0_DoBlur_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_input.texture_Input, context->m_input.texture_Input_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_FilterSub_Iteration_0_Output, context->m_output.texture_FilterSub_Iteration_0_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_input.texture_Input_size[0],
                context->m_input.texture_Input_size[1],
                context->m_input.texture_Input_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_0_DoBlur";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_0_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Copy Resource: FilterSub_Iteration_0_Copy_back
        {
            ScopedPerfEvent scopedPerf("Copy Resource: FilterSub_Iteration_0_Copy_back", commandList, 3);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            // Even if two buffers have the same stride and count, one could be padded for alignment differently based on use
            unsigned int srcSize = context->m_output.texture_FilterSub_Iteration_0_Output->GetDesc().Width;
            unsigned int destSize = context->m_input.texture_Input->GetDesc().Width;
            if (srcSize == destSize)
                commandList->CopyResource(context->m_input.texture_Input, context->m_output.texture_FilterSub_Iteration_0_Output);
            else
                commandList->CopyBufferRegion(context->m_input.texture_Input, 0, context->m_output.texture_FilterSub_Iteration_0_Output, 0, min(srcSize, destSize));

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_0_Copy_back";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _FilterSub_Iteration_1_BlurCB
        {
            context->m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB_cpu.FilterSub_Iteration_1_sRGB = context->m_input.variable_FilterSub_Iteration_1_sRGB;
            context->m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB_cpu.__loopIndexValue_1 = context->m_internal.variable___loopIndexValue_1;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB, context->m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_1_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: FilterSub_Iteration_1_DoBlur
        {
            ScopedPerfEvent scopedPerf("Compute Shader: FilterSub_Iteration_1_DoBlur", commandList, 4);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_FilterSub_Iteration_1_DoBlur_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_input.texture_Input, context->m_input.texture_Input_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_FilterSub_Iteration_1_Output, context->m_output.texture_FilterSub_Iteration_1_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_input.texture_Input_size[0],
                context->m_input.texture_Input_size[1],
                context->m_input.texture_Input_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_1_DoBlur";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_1_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Copy Resource: FilterSub_Iteration_1_Copy_back
        {
            ScopedPerfEvent scopedPerf("Copy Resource: FilterSub_Iteration_1_Copy_back", commandList, 6);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            // Even if two buffers have the same stride and count, one could be padded for alignment differently based on use
            unsigned int srcSize = context->m_output.texture_FilterSub_Iteration_1_Output->GetDesc().Width;
            unsigned int destSize = context->m_input.texture_Input->GetDesc().Width;
            if (srcSize == destSize)
                commandList->CopyResource(context->m_input.texture_Input, context->m_output.texture_FilterSub_Iteration_1_Output);
            else
                commandList->CopyBufferRegion(context->m_input.texture_Input, 0, context->m_output.texture_FilterSub_Iteration_1_Output, 0, min(srcSize, destSize));

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_1_Copy_back";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _FilterSub_Iteration_2_BlurCB
        {
            context->m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB_cpu.FilterSub_Iteration_2_sRGB = context->m_input.variable_FilterSub_Iteration_2_sRGB;
            context->m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB_cpu.__loopIndexValue_2 = context->m_internal.variable___loopIndexValue_2;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB, context->m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_2_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: FilterSub_Iteration_2_DoBlur
        {
            ScopedPerfEvent scopedPerf("Compute Shader: FilterSub_Iteration_2_DoBlur", commandList, 7);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_FilterSub_Iteration_2_DoBlur_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_input.texture_Input, context->m_input.texture_Input_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_FilterSub_Iteration_2_Output, context->m_output.texture_FilterSub_Iteration_2_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_input.texture_Input_size[0],
                context->m_input.texture_Input_size[1],
                context->m_input.texture_Input_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_2_DoBlur";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_2_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Copy Resource: FilterSub_Iteration_2_Copy_back
        {
            ScopedPerfEvent scopedPerf("Copy Resource: FilterSub_Iteration_2_Copy_back", commandList, 9);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            // Even if two buffers have the same stride and count, one could be padded for alignment differently based on use
            unsigned int srcSize = context->m_output.texture_FilterSub_Iteration_2_Output->GetDesc().Width;
            unsigned int destSize = context->m_input.texture_Input->GetDesc().Width;
            if (srcSize == destSize)
                commandList->CopyResource(context->m_input.texture_Input, context->m_output.texture_FilterSub_Iteration_2_Output);
            else
                commandList->CopyBufferRegion(context->m_input.texture_Input, 0, context->m_output.texture_FilterSub_Iteration_2_Output, 0, min(srcSize, destSize));

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_2_Copy_back";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _FilterSub_Iteration_3_BlurCB
        {
            context->m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB_cpu.FilterSub_Iteration_3_sRGB = context->m_input.variable_FilterSub_Iteration_3_sRGB;
            context->m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB_cpu.__loopIndexValue_3 = context->m_internal.variable___loopIndexValue_3;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB, context->m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_3_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: FilterSub_Iteration_3_DoBlur
        {
            ScopedPerfEvent scopedPerf("Compute Shader: FilterSub_Iteration_3_DoBlur", commandList, 10);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_FilterSub_Iteration_3_DoBlur_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_input.texture_Input, context->m_input.texture_Input_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_FilterSub_Iteration_3_Output, context->m_output.texture_FilterSub_Iteration_3_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_input.texture_Input_size[0],
                context->m_input.texture_Input_size[1],
                context->m_input.texture_Input_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_3_DoBlur";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_3_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Copy Resource: FilterSub_Iteration_3_Copy_back
        {
            ScopedPerfEvent scopedPerf("Copy Resource: FilterSub_Iteration_3_Copy_back", commandList, 12);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            // Even if two buffers have the same stride and count, one could be padded for alignment differently based on use
            unsigned int srcSize = context->m_output.texture_FilterSub_Iteration_3_Output->GetDesc().Width;
            unsigned int destSize = context->m_input.texture_Input->GetDesc().Width;
            if (srcSize == destSize)
                commandList->CopyResource(context->m_input.texture_Input, context->m_output.texture_FilterSub_Iteration_3_Output);
            else
                commandList->CopyBufferRegion(context->m_input.texture_Input, 0, context->m_output.texture_FilterSub_Iteration_3_Output, 0, min(srcSize, destSize));

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_3_Copy_back";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _FilterSub_Iteration_4_BlurCB
        {
            context->m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB_cpu.FilterSub_Iteration_4_sRGB = context->m_input.variable_FilterSub_Iteration_4_sRGB;
            context->m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB_cpu.__loopIndexValue_4 = context->m_internal.variable___loopIndexValue_4;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB, context->m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_4_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: FilterSub_Iteration_4_DoBlur
        {
            ScopedPerfEvent scopedPerf("Compute Shader: FilterSub_Iteration_4_DoBlur", commandList, 13);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_FilterSub_Iteration_4_DoBlur_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_input.texture_Input, context->m_input.texture_Input_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_FilterSub_Iteration_4_Output, context->m_output.texture_FilterSub_Iteration_4_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_input.texture_Input_size[0],
                context->m_input.texture_Input_size[1],
                context->m_input.texture_Input_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_4_DoBlur";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_input.texture_Input;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_FilterSub_Iteration_4_Output;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Copy Resource: FilterSub_Iteration_4_Copy_back
        {
            ScopedPerfEvent scopedPerf("Copy Resource: FilterSub_Iteration_4_Copy_back", commandList, 15);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            // Even if two buffers have the same stride and count, one could be padded for alignment differently based on use
            unsigned int srcSize = context->m_output.texture_FilterSub_Iteration_4_Output->GetDesc().Width;
            unsigned int destSize = context->m_input.texture_Input->GetDesc().Width;
            if (srcSize == destSize)
                commandList->CopyResource(context->m_input.texture_Input, context->m_output.texture_FilterSub_Iteration_4_Output);
            else
                commandList->CopyBufferRegion(context->m_input.texture_Input, 0, context->m_output.texture_FilterSub_Iteration_4_Output, 0, min(srcSize, destSize));

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "FilterSub_Iteration_4_Copy_back";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Make sure imported resources are put back in the state they were given to us in
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[1];

            if(context->m_input.texture_Input_state != D3D12_RESOURCE_STATE_COPY_DEST)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.texture_Input;
                barriers[barrierCount].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barriers[barrierCount].Transition.StateAfter = context->m_input.texture_Input_state;
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
    }

    void Context::EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        bool dirty = false;

        // FilterSub_Iteration_0_Output
        {
            unsigned int baseSize[3] = {
                m_input.texture_Input_size[0],
                m_input.texture_Input_size[1],
                m_input.texture_Input_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = m_input.texture_Input_format;

            if(!m_output.texture_FilterSub_Iteration_0_Output ||
               m_output.texture_FilterSub_Iteration_0_Output_size[0] != desiredSize[0] ||
               m_output.texture_FilterSub_Iteration_0_Output_size[1] != desiredSize[1] ||
               m_output.texture_FilterSub_Iteration_0_Output_size[2] != desiredSize[2] ||
               m_output.texture_FilterSub_Iteration_0_Output_numMips != desiredNumMips ||
               m_output.texture_FilterSub_Iteration_0_Output_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_FilterSub_Iteration_0_Output)
                    s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_0_Output);

                m_output.texture_FilterSub_Iteration_0_Output = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_FilterSub_Iteration_0_Output_flags, D3D12_RESOURCE_STATE_COPY_SOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"FilterSub_Iteration_0_Output" : nullptr), Context::LogFn);
                m_output.texture_FilterSub_Iteration_0_Output_size[0] = desiredSize[0];
                m_output.texture_FilterSub_Iteration_0_Output_size[1] = desiredSize[1];
                m_output.texture_FilterSub_Iteration_0_Output_size[2] = desiredSize[2];
                m_output.texture_FilterSub_Iteration_0_Output_numMips = desiredNumMips;
                m_output.texture_FilterSub_Iteration_0_Output_format = desiredFormat;
            }
        }

        // FilterSub_Iteration_1_Output
        {
            unsigned int baseSize[3] = {
                m_input.texture_Input_size[0],
                m_input.texture_Input_size[1],
                m_input.texture_Input_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = m_input.texture_Input_format;

            if(!m_output.texture_FilterSub_Iteration_1_Output ||
               m_output.texture_FilterSub_Iteration_1_Output_size[0] != desiredSize[0] ||
               m_output.texture_FilterSub_Iteration_1_Output_size[1] != desiredSize[1] ||
               m_output.texture_FilterSub_Iteration_1_Output_size[2] != desiredSize[2] ||
               m_output.texture_FilterSub_Iteration_1_Output_numMips != desiredNumMips ||
               m_output.texture_FilterSub_Iteration_1_Output_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_FilterSub_Iteration_1_Output)
                    s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_1_Output);

                m_output.texture_FilterSub_Iteration_1_Output = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_FilterSub_Iteration_1_Output_flags, D3D12_RESOURCE_STATE_COPY_SOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"FilterSub_Iteration_1_Output" : nullptr), Context::LogFn);
                m_output.texture_FilterSub_Iteration_1_Output_size[0] = desiredSize[0];
                m_output.texture_FilterSub_Iteration_1_Output_size[1] = desiredSize[1];
                m_output.texture_FilterSub_Iteration_1_Output_size[2] = desiredSize[2];
                m_output.texture_FilterSub_Iteration_1_Output_numMips = desiredNumMips;
                m_output.texture_FilterSub_Iteration_1_Output_format = desiredFormat;
            }
        }

        // FilterSub_Iteration_2_Output
        {
            unsigned int baseSize[3] = {
                m_input.texture_Input_size[0],
                m_input.texture_Input_size[1],
                m_input.texture_Input_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = m_input.texture_Input_format;

            if(!m_output.texture_FilterSub_Iteration_2_Output ||
               m_output.texture_FilterSub_Iteration_2_Output_size[0] != desiredSize[0] ||
               m_output.texture_FilterSub_Iteration_2_Output_size[1] != desiredSize[1] ||
               m_output.texture_FilterSub_Iteration_2_Output_size[2] != desiredSize[2] ||
               m_output.texture_FilterSub_Iteration_2_Output_numMips != desiredNumMips ||
               m_output.texture_FilterSub_Iteration_2_Output_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_FilterSub_Iteration_2_Output)
                    s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_2_Output);

                m_output.texture_FilterSub_Iteration_2_Output = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_FilterSub_Iteration_2_Output_flags, D3D12_RESOURCE_STATE_COPY_SOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"FilterSub_Iteration_2_Output" : nullptr), Context::LogFn);
                m_output.texture_FilterSub_Iteration_2_Output_size[0] = desiredSize[0];
                m_output.texture_FilterSub_Iteration_2_Output_size[1] = desiredSize[1];
                m_output.texture_FilterSub_Iteration_2_Output_size[2] = desiredSize[2];
                m_output.texture_FilterSub_Iteration_2_Output_numMips = desiredNumMips;
                m_output.texture_FilterSub_Iteration_2_Output_format = desiredFormat;
            }
        }

        // FilterSub_Iteration_3_Output
        {
            unsigned int baseSize[3] = {
                m_input.texture_Input_size[0],
                m_input.texture_Input_size[1],
                m_input.texture_Input_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = m_input.texture_Input_format;

            if(!m_output.texture_FilterSub_Iteration_3_Output ||
               m_output.texture_FilterSub_Iteration_3_Output_size[0] != desiredSize[0] ||
               m_output.texture_FilterSub_Iteration_3_Output_size[1] != desiredSize[1] ||
               m_output.texture_FilterSub_Iteration_3_Output_size[2] != desiredSize[2] ||
               m_output.texture_FilterSub_Iteration_3_Output_numMips != desiredNumMips ||
               m_output.texture_FilterSub_Iteration_3_Output_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_FilterSub_Iteration_3_Output)
                    s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_3_Output);

                m_output.texture_FilterSub_Iteration_3_Output = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_FilterSub_Iteration_3_Output_flags, D3D12_RESOURCE_STATE_COPY_SOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"FilterSub_Iteration_3_Output" : nullptr), Context::LogFn);
                m_output.texture_FilterSub_Iteration_3_Output_size[0] = desiredSize[0];
                m_output.texture_FilterSub_Iteration_3_Output_size[1] = desiredSize[1];
                m_output.texture_FilterSub_Iteration_3_Output_size[2] = desiredSize[2];
                m_output.texture_FilterSub_Iteration_3_Output_numMips = desiredNumMips;
                m_output.texture_FilterSub_Iteration_3_Output_format = desiredFormat;
            }
        }

        // FilterSub_Iteration_4_Output
        {
            unsigned int baseSize[3] = {
                m_input.texture_Input_size[0],
                m_input.texture_Input_size[1],
                m_input.texture_Input_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = m_input.texture_Input_format;

            if(!m_output.texture_FilterSub_Iteration_4_Output ||
               m_output.texture_FilterSub_Iteration_4_Output_size[0] != desiredSize[0] ||
               m_output.texture_FilterSub_Iteration_4_Output_size[1] != desiredSize[1] ||
               m_output.texture_FilterSub_Iteration_4_Output_size[2] != desiredSize[2] ||
               m_output.texture_FilterSub_Iteration_4_Output_numMips != desiredNumMips ||
               m_output.texture_FilterSub_Iteration_4_Output_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_FilterSub_Iteration_4_Output)
                    s_delayedRelease.Add(m_output.texture_FilterSub_Iteration_4_Output);

                m_output.texture_FilterSub_Iteration_4_Output = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_FilterSub_Iteration_4_Output_flags, D3D12_RESOURCE_STATE_COPY_SOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"FilterSub_Iteration_4_Output" : nullptr), Context::LogFn);
                m_output.texture_FilterSub_Iteration_4_Output_size[0] = desiredSize[0];
                m_output.texture_FilterSub_Iteration_4_Output_size[1] = desiredSize[1];
                m_output.texture_FilterSub_Iteration_4_Output_size[2] = desiredSize[2];
                m_output.texture_FilterSub_Iteration_4_Output_numMips = desiredNumMips;
                m_output.texture_FilterSub_Iteration_4_Output_format = desiredFormat;
            }
        }

        // _FilterSub_Iteration_0_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__FilterSub_Iteration_0_BlurCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_FilterSub_Iteration_0_BlurCB" : nullptr), Context::LogFn);
        }

        // _FilterSub_Iteration_1_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__FilterSub_Iteration_1_BlurCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_FilterSub_Iteration_1_BlurCB" : nullptr), Context::LogFn);
        }

        // _FilterSub_Iteration_2_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__FilterSub_Iteration_2_BlurCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_FilterSub_Iteration_2_BlurCB" : nullptr), Context::LogFn);
        }

        // _FilterSub_Iteration_3_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__FilterSub_Iteration_3_BlurCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_FilterSub_Iteration_3_BlurCB" : nullptr), Context::LogFn);
        }

        // _FilterSub_Iteration_4_BlurCB
        if (m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__FilterSub_Iteration_4_BlurCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_FilterSub_Iteration_4_BlurCB" : nullptr), Context::LogFn);
        }
        EnsureDrawCallPSOsCreated(device, dirty);
    }

    bool Context::EnsureDrawCallPSOsCreated(ID3D12Device* device, bool dirty)
    {
        return true;
    }
};
