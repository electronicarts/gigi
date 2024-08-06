#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace boxblur
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

        struct Struct__BoxBlurCB
        {
            int radius = 2;  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
            unsigned int sRGB = true;
            float2 _padding0 = {};  // Padding
        };

        // An internal texture used during the blurring process
        ID3D12Resource* texture_PingPongTexture = nullptr;
        unsigned int texture_PingPongTexture_size[3] = { 0, 0, 0 };
        unsigned int texture_PingPongTexture_numMips = 0;
        DXGI_FORMAT texture_PingPongTexture_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_PingPongTexture_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        const D3D12_RESOURCE_STATES c_texture_PingPongTexture_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        Struct__BoxBlurCB constantBuffer__BoxBlurCB_cpu;
        ID3D12Resource* constantBuffer__BoxBlurCB = nullptr;

        // Horizontal blur pass
        static ID3D12PipelineState* computeShader_BlurH_pso;
        static ID3D12RootSignature* computeShader_BlurH_rootSig;

        // Vertical blur pass
        static ID3D12PipelineState* computeShader_BlurV_pso;
        static ID3D12RootSignature* computeShader_BlurV_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
