#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace CopyPartialBuffer
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

        static ID3D12PipelineState* computeShader_Init_pso;
        static ID3D12RootSignature* computeShader_Init_rootSig;

        ID3D12Resource* buffer_Partial_Copy_source_Copy_0 = nullptr;
        DXGI_FORMAT buffer_Partial_Copy_source_Copy_0_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
        unsigned int buffer_Partial_Copy_source_Copy_0_stride = 0; // For structured buffers, the size of the structure
        unsigned int buffer_Partial_Copy_source_Copy_0_count = 0; // How many items there are
        const D3D12_RESOURCE_STATES c_buffer_Partial_Copy_source_Copy_0_endingState = D3D12_RESOURCE_STATE_COPY_SOURCE;

        static const D3D12_RESOURCE_FLAGS c_buffer_Partial_Copy_source_Copy_0_flags =  D3D12_RESOURCE_FLAG_NONE; // Flags the buffer needs to have been created with

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
