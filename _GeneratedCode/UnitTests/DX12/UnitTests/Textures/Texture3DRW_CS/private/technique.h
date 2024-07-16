#pragma once

#include <d3d12.h>
#include <array>
#include <vector>

namespace Texture3DRW_CS
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

        ID3D12Resource* texture__loadedTexture_0 = nullptr;
        unsigned int texture__loadedTexture_0_size[3] = { 0, 0, 0 };
        DXGI_FORMAT texture__loadedTexture_0_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture__loadedTexture_0_flags =  D3D12_RESOURCE_FLAG_NONE;
        const D3D12_RESOURCE_STATES c_texture__loadedTexture_0_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        static ID3D12PipelineState* computeShader_RW_pso;
        static ID3D12RootSignature* computeShader_RW_rootSig;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;

        std::vector<int> m_managedRTVs;
        std::vector<int> m_managedDSVs;
    };
};
