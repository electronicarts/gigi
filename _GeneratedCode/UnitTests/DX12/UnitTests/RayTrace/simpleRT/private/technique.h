#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace simpleRT
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

        struct Struct_VertexBufferSimple
        {
            float3 Position = {0.f, 0.f, 0.f};
        };

        struct Struct_VertexBufferFull
        {
            float3 Position = {0.f, 0.f, 0.f};
            float3 Color = {0.f, 0.f, 0.f};
            float3 Normal = {0.f, 0.f, 0.f};
            float4 Tangent = {0.f, 0.f, 0.f, 0.f};
            float2 UV = {0.f, 0.f};
            int MaterialID = 0;
        };

        struct Struct__SimpleRTRayGenCB
        {
            float3 cameraPos = {0.f, 0.f, 0.f};
            float _padding0 = 0.f;  // Padding
            float4x4 clipToWorld = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float depthNearPlane = 0.f;  // The depth value for the near plane.
            float3 hitColor = {0.f, 1.f, 0.f};
            float3 missColor = {1.f, 0.f, 0.f};
            float _padding1 = 0.f;  // Padding
        };

        Struct__SimpleRTRayGenCB constantBuffer__SimpleRTRayGenCB_cpu;
        ID3D12Resource* constantBuffer__SimpleRTRayGenCB = nullptr;

        static ID3D12StateObject* rayShader_DoRT_rtso;
        static ID3D12RootSignature* rayShader_DoRT_rootSig;
        static ID3D12Resource* rayShader_DoRT_shaderTableRayGen;
        static unsigned int    rayShader_DoRT_shaderTableRayGenSize;
        static ID3D12Resource* rayShader_DoRT_shaderTableMiss;
        static unsigned int    rayShader_DoRT_shaderTableMissSize;
        static ID3D12Resource* rayShader_DoRT_shaderTableHitGroup;
        static unsigned int    rayShader_DoRT_shaderTableHitGroupSize;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
