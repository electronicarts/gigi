#pragma once

#include <d3d12.h>
#include <array>
#include <vector>

namespace buffertest
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

        struct Struct__BufferTestCB
        {
            float alpha1 = 1.000000f;  // Adjusts the contribution of sample n-1
            float alpha2 = 0.000000f;  // Adjusts the contribution of sample n-2
            float gain = 0.500000f;  // Overall Volume Adjustment
            float _padding0 = 0.000000f;  // Padding
        };

        Struct__BufferTestCB constantBuffer__BufferTestCB_cpu;
        ID3D12Resource* constantBuffer__BufferTestCB = nullptr;

        // BufferTest compute shader
        static ID3D12PipelineState* computeShader_BufferTest_pso;
        static ID3D12RootSignature* computeShader_BufferTest_rootSig;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;

        std::vector<int> m_managedRTVs;
        std::vector<int> m_managedDSVs;
    };
};
