#pragma once

#include <d3d12.h>

#include <vector>
#include <string>

#include "DX12Utils/ShaderCompilerCommon.h"
#include "DX12Utils/logfn.h"

namespace DX12Utils
{
    bool MakeComputePSO_DXC(
        ID3D12Device* device,
		const ShaderCompilationInfo& shaderInfo,
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState** pso,
        TLogFn logFn);

    bool MakeComputePSO_FXC(
        ID3D12Device* device,
		const ShaderCompilationInfo& shaderInfo,
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState** pso,
        TLogFn logFn);

    std::vector<unsigned char> CompileShaderToByteCode_DXC(
		const ShaderCompilationInfo& shaderInfo,
        TLogFn logFn);

    std::vector<unsigned char> CompileShaderToByteCode_FXC(
		const ShaderCompilationInfo& shaderInfo,
        TLogFn logFn);
}