#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace YesVertexStruct_NoIndex_NoInstance
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

    enum class ViewMode: int
    {
        Normal,
        UV,
        Solid,
    };

    inline const char* EnumToString(ViewMode value, bool displayString = false)
    {
        switch(value)
        {
            case ViewMode::Normal: return displayString ? "Normal" : "Normal";
            case ViewMode::UV: return displayString ? "UV" : "UV";
            case ViewMode::Solid: return displayString ? "Solid" : "Solid";
            default: return nullptr;
        }
    }

    struct ContextInternal
    {
        ID3D12QueryHeap* m_TimestampQueryHeap = nullptr;
        ID3D12Resource* m_TimestampReadbackBuffer = nullptr;

        static ID3D12CommandSignature* s_commandSignatureDispatch;

        struct Struct__VertexShaderCB
        {
            float4x4 ViewProjMtx = {1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f};
        };

        struct Struct__PixelShaderCB
        {
            int viewMode = (int)ViewMode::Normal;
            float3 _padding0 = {0.000000f, 0.000000f, 0.000000f};  // Padding
        };

        Struct__VertexShaderCB constantBuffer__VertexShaderCB_cpu;
        ID3D12Resource* constantBuffer__VertexShaderCB = nullptr;

        Struct__PixelShaderCB constantBuffer__PixelShaderCB_cpu;
        ID3D12Resource* constantBuffer__PixelShaderCB = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_rootSig = nullptr;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
