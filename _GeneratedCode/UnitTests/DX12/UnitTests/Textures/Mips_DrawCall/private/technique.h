#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace Mips_DrawCall
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
            float4x4 ViewProjMtx = {1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f};
        };

        struct Struct__VertexShaderSphereCB
        {
            float4x4 ViewProjMtx = {1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f};
        };

        struct Struct__PixelShaderSphereCB
        {
            unsigned int UseMips = true;
            float3 _padding0 = {0.0f,0.0f,0.0f};  // Padding
        };

        ID3D12Resource* texture_Color_Buffer = nullptr;
        unsigned int texture_Color_Buffer_size[3] = { 0, 0, 0 };
        unsigned int texture_Color_Buffer_numMips = 0;
        DXGI_FORMAT texture_Color_Buffer_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_Color_Buffer_flags =  D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        const D3D12_RESOURCE_STATES c_texture_Color_Buffer_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        ID3D12Resource* texture_Depth_Buffer = nullptr;
        unsigned int texture_Depth_Buffer_size[3] = { 0, 0, 0 };
        unsigned int texture_Depth_Buffer_numMips = 0;
        DXGI_FORMAT texture_Depth_Buffer_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_Depth_Buffer_flags =  D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        const D3D12_RESOURCE_STATES c_texture_Depth_Buffer_endingState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

        Struct__VertexShaderCB constantBuffer__VertexShaderCB_cpu;
        ID3D12Resource* constantBuffer__VertexShaderCB = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_0_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_0_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_1_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_1_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_2_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_2_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_3_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_3_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_4_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_4_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_5_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_5_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_6_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_6_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_7_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_7_rootSig = nullptr;

        ID3D12PipelineState* drawCall_Rasterize_Mip_8_pso = nullptr;
        ID3D12RootSignature* drawCall_Rasterize_Mip_8_rootSig = nullptr;

        Struct__VertexShaderSphereCB constantBuffer__VertexShaderSphereCB_cpu;
        ID3D12Resource* constantBuffer__VertexShaderSphereCB = nullptr;

        Struct__PixelShaderSphereCB constantBuffer__PixelShaderSphereCB_cpu;
        ID3D12Resource* constantBuffer__PixelShaderSphereCB = nullptr;

        ID3D12PipelineState* drawCall_Draw_Sphere_pso = nullptr;
        ID3D12RootSignature* drawCall_Draw_Sphere_rootSig = nullptr;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
