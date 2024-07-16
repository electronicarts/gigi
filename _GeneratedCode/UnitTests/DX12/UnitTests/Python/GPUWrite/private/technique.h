#pragma once

#include <d3d12.h>
#include <array>
#include <vector>

namespace GPUWrite
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

        struct Struct_BufferStruct
        {
            int theInt = 0;
            float theFloat = 0.000000f;
        };

        ID3D12Resource* texture_Texture2D = nullptr;
        unsigned int texture_Texture2D_size[3] = { 0, 0, 0 };
        DXGI_FORMAT texture_Texture2D_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_Texture2D_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        const D3D12_RESOURCE_STATES c_texture_Texture2D_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        ID3D12Resource* texture_Texture2DArray = nullptr;
        unsigned int texture_Texture2DArray_size[3] = { 0, 0, 0 };
        DXGI_FORMAT texture_Texture2DArray_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_Texture2DArray_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        const D3D12_RESOURCE_STATES c_texture_Texture2DArray_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        ID3D12Resource* texture_Texture3D = nullptr;
        unsigned int texture_Texture3D_size[3] = { 0, 0, 0 };
        DXGI_FORMAT texture_Texture3D_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_Texture3D_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        const D3D12_RESOURCE_STATES c_texture_Texture3D_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        ID3D12Resource* buffer_FloatBuffer = nullptr;
        DXGI_FORMAT buffer_FloatBuffer_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
        unsigned int buffer_FloatBuffer_stride = 0; // For structured buffers, the size of the structure
        unsigned int buffer_FloatBuffer_count = 0; // How many items there are
        const D3D12_RESOURCE_STATES c_buffer_FloatBuffer_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        static const D3D12_RESOURCE_FLAGS c_buffer_FloatBuffer_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

        ID3D12Resource* buffer_StructBuffer = nullptr;
        DXGI_FORMAT buffer_StructBuffer_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
        unsigned int buffer_StructBuffer_stride = 0; // For structured buffers, the size of the structure
        unsigned int buffer_StructBuffer_count = 0; // How many items there are
        const D3D12_RESOURCE_STATES c_buffer_StructBuffer_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        static const D3D12_RESOURCE_FLAGS c_buffer_StructBuffer_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

        static ID3D12PipelineState* computeShader_ComputeShader_pso;
        static ID3D12RootSignature* computeShader_ComputeShader_rootSig;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;

        std::vector<int> m_managedRTVs;
        std::vector<int> m_managedDSVs;
    };
};
