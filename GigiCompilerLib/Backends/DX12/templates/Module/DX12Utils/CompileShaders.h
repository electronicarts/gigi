/*$(CopyrightHeader)*/#pragma once

#include <d3d12.h>

#include <vector>
#include <string>

#include "DX12Utils/logfn.h"

namespace DX12Utils
{
    bool MakeComputePSO_DXC(
        ID3D12Device* device,
        LPCWSTR shaderDir,
        LPCWSTR shaderFile,
        const char* entryPoint,
        const char* shaderModel,
        const D3D_SHADER_MACRO* defines,
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState** pso,
        bool debugShaders,
        LPCWSTR debugName,
        TLogFn logFn);

    bool MakeComputePSO_FXC(
        ID3D12Device* device,
        LPCWSTR shaderDir,
        LPCWSTR shaderFile,
        const char* entryPoint,
        const char* shaderModel,
        const D3D_SHADER_MACRO* defines,
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState** pso,
        bool debugShaders,
        LPCWSTR debugName,
        TLogFn logFn);

    std::vector<unsigned char> CompileShaderToByteCode_DXC(
        LPCWSTR shaderDir,
        LPCWSTR shaderFile,
        const char* entryPoint,
        const char* shaderModel,
        const D3D_SHADER_MACRO* defines,
        bool debugShaders,
        TLogFn logFn);

    std::vector<unsigned char> CompileShaderToByteCode_FXC(
        LPCWSTR shaderDir,
        LPCWSTR shaderFile,
        const char* entryPoint,
        const char* shaderModel,
        const D3D_SHADER_MACRO* defines,
        bool debugShaders,
        TLogFn logFn);
}