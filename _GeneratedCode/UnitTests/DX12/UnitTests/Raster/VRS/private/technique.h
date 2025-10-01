#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace VRS
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

        struct Struct__VertexShaderCB
        {
            float4x4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        };

        struct Struct__QuadVSCB
        {
            float4x4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        };

        // Variables
        uint2 variable_ShadingRateTextureSizeXY = {0, 0};

        ID3D12Resource* texture_ShadingRate = nullptr;
        unsigned int texture_ShadingRate_size[3] = { 0, 0, 0 };
        unsigned int texture_ShadingRate_numMips = 0;
        DXGI_FORMAT texture_ShadingRate_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_ShadingRate_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        const D3D12_RESOURCE_STATES c_texture_ShadingRate_endingState = D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

        static ID3D12PipelineState* computeShader_MakeShadingRate_pso;
        static ID3D12RootSignature* computeShader_MakeShadingRate_rootSig;

        Struct__VertexShaderCB constantBuffer__VertexShaderCB_cpu;
        ID3D12Resource* constantBuffer__VertexShaderCB = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Vignette_pso = nullptr;
        ID3D12RootSignature* drawCall_Vignette_rootSig = nullptr;

        Struct__QuadVSCB constantBuffer__QuadVSCB_cpu;
        ID3D12Resource* constantBuffer__QuadVSCB = nullptr;

        ID3D12PipelineState* drawCall_DrawQuad_pso = nullptr;
        ID3D12RootSignature* drawCall_DrawQuad_rootSig = nullptr;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
