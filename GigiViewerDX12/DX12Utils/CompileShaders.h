///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include "GigiCompilerLib/Utils.h"
#include "ShaderCompilerCommon.h"

bool MakeComputePSO_dxc(
    ID3D12Device* device,
    const ShaderCompilationInfo& shaderInfo,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState** pso,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);

bool MakeComputePSO_fxc(
    ID3D12Device* device,
	const ShaderCompilationInfo& shaderInfo,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState** pso,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);

std::vector<unsigned char> CompileShaderToByteCode_dxc(
	const ShaderCompilationInfo& shaderInfo,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);

std::vector<unsigned char> CompileShaderToByteCode_fxc(
	const ShaderCompilationInfo& shaderInfo,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);