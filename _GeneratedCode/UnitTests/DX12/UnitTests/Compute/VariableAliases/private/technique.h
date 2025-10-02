#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace VariableAliases
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

        struct Struct__SetChannel_0CB
        {
            int _alias_Channel = 0;
            float _alias_Value = 0.f;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        struct Struct__SetChannel_1CB
        {
            int _alias_Channel = 0;
            float _alias_Value = 0.f;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        Struct__SetChannel_0CB constantBuffer__SetChannel_0CB_0_cpu;
        ID3D12Resource* constantBuffer__SetChannel_0CB_0 = nullptr;

        static ID3D12PipelineState* computeShader_Set_Red_pso;
        static ID3D12RootSignature* computeShader_Set_Red_rootSig;

        Struct__SetChannel_0CB constantBuffer__SetChannel_0CB_1_cpu;
        ID3D12Resource* constantBuffer__SetChannel_0CB_1 = nullptr;

        static ID3D12PipelineState* computeShader_Set_Green_pso;
        static ID3D12RootSignature* computeShader_Set_Green_rootSig;

        Struct__SetChannel_1CB constantBuffer__SetChannel_1CB_0_cpu;
        ID3D12Resource* constantBuffer__SetChannel_1CB_0 = nullptr;

        static ID3D12PipelineState* computeShader_Set_Blue_pso;
        static ID3D12RootSignature* computeShader_Set_Blue_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
