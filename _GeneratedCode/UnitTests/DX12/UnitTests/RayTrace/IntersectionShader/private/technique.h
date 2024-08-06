#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace IntersectionShader
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

        struct Struct__Ray_GenCB
        {
            float3 CameraPos = {0.0f, 0.0f, 0.0f};
            float _padding0 = 0.000000f;  // Padding
            float4x4 InvViewProjMtx = {1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f};
        };

        Struct__Ray_GenCB constantBuffer__Ray_GenCB_cpu;
        ID3D12Resource* constantBuffer__Ray_GenCB = nullptr;

        static ID3D12StateObject* rayShader_Do_RT_rtso;
        static ID3D12RootSignature* rayShader_Do_RT_rootSig;
        static ID3D12Resource* rayShader_Do_RT_shaderTableRayGen;
        static unsigned int    rayShader_Do_RT_shaderTableRayGenSize;
        static ID3D12Resource* rayShader_Do_RT_shaderTableMiss;
        static unsigned int    rayShader_Do_RT_shaderTableMissSize;
        static ID3D12Resource* rayShader_Do_RT_shaderTableHitGroup;
        static unsigned int    rayShader_Do_RT_shaderTableHitGroupSize;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
