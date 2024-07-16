///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include "GigiCompilerLib/Utils.h"

bool MakeComputePSO_dxc(
    ID3D12Device* device,
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState** pso,
    bool debugShaders,
    const char* debugName,
    bool HV2021,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);

bool MakeComputePSO_fxc(
    ID3D12Device* device,
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState** pso,
    bool debugShaders,
    const char* debugName,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);

std::vector<unsigned char> CompileShaderToByteCode_dxc(
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    bool debugShaders,
    bool HV2021,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);

std::vector<unsigned char> CompileShaderToByteCode_fxc(
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    bool debugShaders,
    LogFn logFn,
    std::vector<std::string>* allFiles = nullptr);