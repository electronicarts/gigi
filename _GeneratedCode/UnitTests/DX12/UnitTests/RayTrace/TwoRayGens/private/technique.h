#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace TwoRayGens
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

        struct Struct_VertexBuffer
        {
            float3 Color = {};
            float3 Position = {};
        };

        struct Struct_VertexBufferSimple
        {
            float3 Position = {};
        };

        struct Struct_VertexBufferFull
        {
            float3 Position = {};
            float3 Color = {};
            float3 Normal = {};
            float4 Tangent = {};
            float2 UV = {};
            int MaterialID = 0;
        };

        struct Struct__TwoRayGens1CB
        {
            float3 cameraPos = {};
            float _padding0 = 0.000000f;  // Padding
            float4x4 clipToWorld = {};
            float depthNearPlane = 0.000000f;  // The depth value for the near plane.
            float3 _padding1 = {};  // Padding
        };

        struct Struct__TwoRayGens2CB
        {
            float3 cameraPos = {};
            float _padding0 = 0.000000f;  // Padding
            float4x4 clipToWorld = {};
            float depthNearPlane = 0.000000f;  // The depth value for the near plane.
            float3 _padding1 = {};  // Padding
        };

        Struct__TwoRayGens1CB constantBuffer__TwoRayGens1CB_cpu;
        ID3D12Resource* constantBuffer__TwoRayGens1CB = nullptr;

        static ID3D12StateObject* rayShader_DoRT1_rtso;
        static ID3D12RootSignature* rayShader_DoRT1_rootSig;
        static ID3D12Resource* rayShader_DoRT1_shaderTableRayGen;
        static unsigned int    rayShader_DoRT1_shaderTableRayGenSize;
        static ID3D12Resource* rayShader_DoRT1_shaderTableMiss;
        static unsigned int    rayShader_DoRT1_shaderTableMissSize;
        static ID3D12Resource* rayShader_DoRT1_shaderTableHitGroup;
        static unsigned int    rayShader_DoRT1_shaderTableHitGroupSize;

        Struct__TwoRayGens2CB constantBuffer__TwoRayGens2CB_cpu;
        ID3D12Resource* constantBuffer__TwoRayGens2CB = nullptr;

        static ID3D12StateObject* rayShader_DoRT2_rtso;
        static ID3D12RootSignature* rayShader_DoRT2_rootSig;
        static ID3D12Resource* rayShader_DoRT2_shaderTableRayGen;
        static unsigned int    rayShader_DoRT2_shaderTableRayGenSize;
        static ID3D12Resource* rayShader_DoRT2_shaderTableMiss;
        static unsigned int    rayShader_DoRT2_shaderTableMissSize;
        static ID3D12Resource* rayShader_DoRT2_shaderTableHitGroup;
        static unsigned int    rayShader_DoRT2_shaderTableHitGroupSize;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
