#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace StructuredBuffer
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

    enum class Trinary: int
    {
        True,
        False,
        Maybe,
    };

    inline const char* EnumToString(Trinary value, bool displayString = false)
    {
        switch(value)
        {
            case Trinary::True: return displayString ? "True" : "True";
            case Trinary::False: return displayString ? "False" : "False";
            case Trinary::Maybe: return displayString ? "Maybe" : "Maybe";
            default: return nullptr;
        }
    }

    struct ContextInternal
    {
        ID3D12QueryHeap* m_TimestampQueryHeap = nullptr;
        ID3D12Resource* m_TimestampReadbackBuffer = nullptr;

        static ID3D12CommandSignature* s_commandSignatureDispatch;

        struct Struct__csmain_0CB
        {
            float frameDeltaTime = 0.f;
            int frameIndex = 0;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        Struct__csmain_0CB constantBuffer__csmain_0CB_cpu;
        ID3D12Resource* constantBuffer__csmain_0CB = nullptr;

        static ID3D12PipelineState* computeShader_csmain_pso;
        static ID3D12RootSignature* computeShader_csmain_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
