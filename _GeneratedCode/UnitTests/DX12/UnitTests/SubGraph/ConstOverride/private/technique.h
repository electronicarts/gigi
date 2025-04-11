#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace ConstOverride
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

        // Variables
        const uint variable___literal_0 = 50;  // Made to replace variable "MinX" with a constant value in subgraph node "Left"
        const uint variable___literal_1 = 100;  // Made to replace variable "MaxX" with a constant value in subgraph node "Left"
        const uint variable___literal_2 = 150;  // Made to replace variable "MinX" with a constant value in subgraph node "Right"
        const uint variable___literal_3 = 200;  // Made to replace variable "MaxX" with a constant value in subgraph node "Right"

        static ID3D12PipelineState* computeShader_Clear_pso;
        static ID3D12RootSignature* computeShader_Clear_rootSig;

        static ID3D12PipelineState* computeShader_Left_WriteColor_pso;
        static ID3D12RootSignature* computeShader_Left_WriteColor_rootSig;

        static ID3D12PipelineState* computeShader_Right_WriteColor_pso;
        static ID3D12RootSignature* computeShader_Right_WriteColor_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
