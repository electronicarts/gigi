#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace SlangAutoDiff
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

        struct Struct__InitCB
        {
            int FrameIndex = 0;
            float3 _padding0 = {0.0f,0.0f,0.0f};  // Padding
            float4 MouseState = {0.0f,0.0f,0.0f,0.0f};
            float4 MouseStateLastFrame = {0.0f,0.0f,0.0f,0.0f};
            int NumGaussians = 10;
            float3 iResolution = {0.0f,0.0f,0.0f};
            unsigned int initialized = false;
            float3 _padding1 = {0.0f,0.0f,0.0f};  // Padding
        };

        struct Struct__RenderCB
        {
            int NumGaussians = 10;
            unsigned int QuantizeDisplay = false;
            float2 _padding0 = {0.0f,0.0f};  // Padding
        };

        struct Struct__DescendCB
        {
            float LearningRate = 0.100000f;
            float MaximumStepSize = 0.010000f;
            int NumGaussians = 10;
            unsigned int UseBackwardAD = true;
        };

        // first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
        ID3D12Resource* buffer_Data = nullptr;
        DXGI_FORMAT buffer_Data_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
        unsigned int buffer_Data_stride = 0; // For structured buffers, the size of the structure
        unsigned int buffer_Data_count = 0; // How many items there are
        const D3D12_RESOURCE_STATES c_buffer_Data_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        static const D3D12_RESOURCE_FLAGS c_buffer_Data_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

        Struct__InitCB constantBuffer__InitCB_cpu;
        ID3D12Resource* constantBuffer__InitCB = nullptr;

        static ID3D12PipelineState* computeShader_Initialize_pso;
        static ID3D12RootSignature* computeShader_Initialize_rootSig;

        Struct__RenderCB constantBuffer__RenderCB_cpu;
        ID3D12Resource* constantBuffer__RenderCB = nullptr;

        Struct__DescendCB constantBuffer__DescendCB_cpu;
        ID3D12Resource* constantBuffer__DescendCB = nullptr;

        static ID3D12PipelineState* computeShader_GradientDescend_pso;
        static ID3D12RootSignature* computeShader_GradientDescend_rootSig;

        static ID3D12PipelineState* computeShader_Render_pso;
        static ID3D12RootSignature* computeShader_Render_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
