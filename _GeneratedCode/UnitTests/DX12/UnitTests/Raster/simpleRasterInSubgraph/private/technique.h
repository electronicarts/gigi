#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace simpleRasterInSubgraph
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

        struct Struct__DoSimpleRaster_VertexShaderCB
        {
            float4x4 DoSimpleRaster_ViewProjMtx = {1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f};
        };

        // This is here for the benefit of simpleRasterInSubgraph. Need a reference to a struct type in a subgraph.
        ID3D12Resource* buffer_DoSimpleRaster_VBCopy = nullptr;
        DXGI_FORMAT buffer_DoSimpleRaster_VBCopy_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
        unsigned int buffer_DoSimpleRaster_VBCopy_stride = 0; // For structured buffers, the size of the structure
        unsigned int buffer_DoSimpleRaster_VBCopy_count = 0; // How many items there are
        const D3D12_RESOURCE_STATES c_buffer_DoSimpleRaster_VBCopy_endingState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

        static const D3D12_RESOURCE_FLAGS c_buffer_DoSimpleRaster_VBCopy_flags =  D3D12_RESOURCE_FLAG_NONE; // Flags the buffer needs to have been created with

        Struct__DoSimpleRaster_VertexShaderCB constantBuffer__DoSimpleRaster_VertexShaderCB_cpu;
        ID3D12Resource* constantBuffer__DoSimpleRaster_VertexShaderCB = nullptr;

        ID3D12PipelineState* drawCall_DoSimpleRaster_Rasterize_pso = nullptr;
        ID3D12RootSignature* drawCall_DoSimpleRaster_Rasterize_rootSig = nullptr;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
