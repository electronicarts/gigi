#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace IndirectDispatch
{
    using uint = unsigned int;
    using uint2 = std::array<uint, 2>;
    using uint3 = std::array<uint, 3>;
    using uint4 = std::array<uint, 4>;

    using int2 = std::array<int, 2>;
    using int3 = std::array<int, 3>;
    using int4 = std::array<int, 4>;
    using float2 = std::array<float, 2>;
    using float3 = std::array<float, 3>;
    using float4 = std::array<float, 4>;
    using float4x4 = std::array<std::array<float, 4>, 4>;

    struct ContextInternal
    {
        ID3D12QueryHeap* m_TimestampQueryHeap = nullptr;
        ID3D12Resource* m_TimestampReadbackBuffer = nullptr;

        static ID3D12CommandSignature* s_commandSignatureDispatch;

        struct Struct__Fill_Indirect_Dispatch_Count_0CB
        {
            uint3 Dispatch_Count_1 = {50, 50, 1};
            float _padding0 = 0.f;  // Padding
            uint3 Dispatch_Count_2 = {100, 100, 1};
            float _padding1 = 0.f;  // Padding
        };

        ID3D12Resource* buffer_Indirect_Dispatch_Count = nullptr;
        DXGI_FORMAT buffer_Indirect_Dispatch_Count_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
        unsigned int buffer_Indirect_Dispatch_Count_stride = 0; // For structured buffers, the size of the structure
        unsigned int buffer_Indirect_Dispatch_Count_count = 0; // How many items there are
        const D3D12_RESOURCE_STATES c_buffer_Indirect_Dispatch_Count_endingState = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

        static const D3D12_RESOURCE_FLAGS c_buffer_Indirect_Dispatch_Count_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

        static ID3D12PipelineState* computeShader_Clear_Render_Target_pso;
        static ID3D12RootSignature* computeShader_Clear_Render_Target_rootSig;

        Struct__Fill_Indirect_Dispatch_Count_0CB constantBuffer__Fill_Indirect_Dispatch_Count_0CB_cpu;
        ID3D12Resource* constantBuffer__Fill_Indirect_Dispatch_Count_0CB = nullptr;

        static ID3D12PipelineState* computeShader_Fill_Indirect_Dispatch_Count_pso;
        static ID3D12RootSignature* computeShader_Fill_Indirect_Dispatch_Count_rootSig;

        static ID3D12PipelineState* computeShader_Do_Indirect_Dispatch_1_pso;
        static ID3D12RootSignature* computeShader_Do_Indirect_Dispatch_1_rootSig;

        static ID3D12PipelineState* computeShader_Do_Indirect_Dispatch_2_pso;
        static ID3D12RootSignature* computeShader_Do_Indirect_Dispatch_2_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
