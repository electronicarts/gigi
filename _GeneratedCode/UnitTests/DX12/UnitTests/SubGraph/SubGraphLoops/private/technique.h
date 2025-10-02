#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace SubGraphLoops
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

        struct Struct__FilterSub_Iteration_0_Blur_0CB
        {
            unsigned int FilterSub_Iteration_0_sRGB = true;
            int __loopIndexValue_0 = 0;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        struct Struct__FilterSub_Iteration_1_Blur_0CB
        {
            unsigned int FilterSub_Iteration_1_sRGB = true;
            int __loopIndexValue_1 = 1;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        struct Struct__FilterSub_Iteration_2_Blur_0CB
        {
            unsigned int FilterSub_Iteration_2_sRGB = true;
            int __loopIndexValue_2 = 2;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        struct Struct__FilterSub_Iteration_3_Blur_0CB
        {
            unsigned int FilterSub_Iteration_3_sRGB = true;
            int __loopIndexValue_3 = 3;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        struct Struct__FilterSub_Iteration_4_Blur_0CB
        {
            unsigned int FilterSub_Iteration_4_sRGB = true;
            int __loopIndexValue_4 = 4;
            float2 _padding0 = {0.f, 0.f};  // Padding
        };

        // Variables
        int variable___loopIndexValue_0 = 0;
        int variable___loopIndexValue_1 = 1;
        int variable___loopIndexValue_2 = 2;
        int variable___loopIndexValue_3 = 3;
        int variable___loopIndexValue_4 = 4;

        Struct__FilterSub_Iteration_0_Blur_0CB constantBuffer__FilterSub_Iteration_0_Blur_0CB_cpu;
        ID3D12Resource* constantBuffer__FilterSub_Iteration_0_Blur_0CB = nullptr;

        static ID3D12PipelineState* computeShader_FilterSub_Iteration_0_DoBlur_pso;
        static ID3D12RootSignature* computeShader_FilterSub_Iteration_0_DoBlur_rootSig;

        Struct__FilterSub_Iteration_1_Blur_0CB constantBuffer__FilterSub_Iteration_1_Blur_0CB_cpu;
        ID3D12Resource* constantBuffer__FilterSub_Iteration_1_Blur_0CB = nullptr;

        static ID3D12PipelineState* computeShader_FilterSub_Iteration_1_DoBlur_pso;
        static ID3D12RootSignature* computeShader_FilterSub_Iteration_1_DoBlur_rootSig;

        Struct__FilterSub_Iteration_2_Blur_0CB constantBuffer__FilterSub_Iteration_2_Blur_0CB_cpu;
        ID3D12Resource* constantBuffer__FilterSub_Iteration_2_Blur_0CB = nullptr;

        static ID3D12PipelineState* computeShader_FilterSub_Iteration_2_DoBlur_pso;
        static ID3D12RootSignature* computeShader_FilterSub_Iteration_2_DoBlur_rootSig;

        Struct__FilterSub_Iteration_3_Blur_0CB constantBuffer__FilterSub_Iteration_3_Blur_0CB_cpu;
        ID3D12Resource* constantBuffer__FilterSub_Iteration_3_Blur_0CB = nullptr;

        static ID3D12PipelineState* computeShader_FilterSub_Iteration_3_DoBlur_pso;
        static ID3D12RootSignature* computeShader_FilterSub_Iteration_3_DoBlur_rootSig;

        Struct__FilterSub_Iteration_4_Blur_0CB constantBuffer__FilterSub_Iteration_4_Blur_0CB_cpu;
        ID3D12Resource* constantBuffer__FilterSub_Iteration_4_Blur_0CB = nullptr;

        static ID3D12PipelineState* computeShader_FilterSub_Iteration_4_DoBlur_pso;
        static ID3D12RootSignature* computeShader_FilterSub_Iteration_4_DoBlur_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
