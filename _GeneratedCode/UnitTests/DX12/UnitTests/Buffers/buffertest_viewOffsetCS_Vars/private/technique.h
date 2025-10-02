#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace buffertest_viewOffsetCS_Vars
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

        struct Struct__BufferTest_0CB
        {
            float alpha1 = 1.f;  // Adjusts the contribution of sample n-1
            float alpha2 = 0.f;  // Adjusts the contribution of sample n-2
            float gain = 0.5f;  // Overall Volume Adjustment
            float _padding0 = 0.f;  // Padding
        };

        // Variables
        int variable_InputTypedBufferBegin = 0;
        int variable_InputTypedBufferSize = 0;
        int variable_OutputTypedBufferBegin = 1;
        int variable_OutputTypedBufferSize = 0;
        int variable_InputTypedStructBufferBegin = 0;
        int variable_InputTypedStructBufferSize = 0;
        int variable_OutputTypedStructBufferBegin = 4;
        int variable_OutputTypedStructBufferSize = 0;
        int variable_InputStructuredBufferBegin = 0;
        int variable_InputStructuredBufferSize = 0;
        int variable_OutputStructuredBufferBegin = 1;
        int variable_OutputStructuredBufferSize = 0;
        int variable_InputTypedBufferRawBegin = 0;
        int variable_InputTypedBufferRawSize = 0;
        int variable_OutputTypedBufferRawBegin = 16;
        int variable_OutputTypedBufferRawSize = 0;

        Struct__BufferTest_0CB constantBuffer__BufferTest_0CB_cpu;
        ID3D12Resource* constantBuffer__BufferTest_0CB = nullptr;

        // BufferTest compute shader
        static ID3D12PipelineState* computeShader_BufferTest_pso;
        static ID3D12RootSignature* computeShader_BufferTest_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
