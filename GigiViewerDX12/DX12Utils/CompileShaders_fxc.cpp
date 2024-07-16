///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "CompileShaders.h"

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <vector>
#include <comdef.h>
#include <filesystem>

class IncludeHandlerFXC : public ID3DInclude
{
public:
    IncludeHandlerFXC(const char* directory)
        : m_directory(directory)
    {
    }

    HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
    {
        std::string fulLFileName = (std::filesystem::path(m_directory) / pFileName).string();
        m_includeFiles.push_back(fulLFileName);

        FILE* file = nullptr;
        fopen_s(&file, fulLFileName.c_str(), "rb");
        if (!file)
            return E_FAIL;

        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* data = new char[fileSize];
        fread(data, 1, fileSize, file);
        *ppData = data;
        *pBytes = (UINT)fileSize;

        fclose(file);
        return S_OK;
    }

    HRESULT Close(LPCVOID pData) override
    {
        delete[] pData;
        return S_OK;
    }

    std::string m_directory;
    std::vector<std::string> m_includeFiles;
};

static ID3DBlob* CompileShaderToByteCode_Private(
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    bool debugShaders,
    LogFn logFn,
    std::vector<std::string>* allFiles
    )
{
    // Compile Shaders
    ID3DBlob* error = nullptr;
    ID3DBlob* shader = nullptr;
    UINT compileFlags = debugShaders ? D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION : 0;

    // compile the shader from file
    std::string shaderDir = std::filesystem::path(fileName).parent_path().string();
    IncludeHandlerFXC include(shaderDir.c_str());
    std::wstring fullFileName = ToWideString(fileName);
    HRESULT hr = D3DCompileFromFile(fullFileName.c_str(), defines, &include, entryPoint, shaderModel, compileFlags, 0, &shader, &error);

    if (allFiles)
    {
        allFiles->push_back(fileName);
        for (const std::string& fileName : include.m_includeFiles)
            allFiles->push_back(fileName);
    }

    if (FAILED(hr))
    {
        _com_error err(hr);
        if (error)
        {
            const char* errorMsg = (const char*)error->GetBufferPointer();
            logFn(LogLevel::Error, "Could not compile shader %s:\n%s\n%s", fileName, FromWideString(err.ErrorMessage()).c_str(), errorMsg);
            error->Release();
            error = nullptr;
        }
        else
        {
            logFn(LogLevel::Error, "Could not compile shader %s:\n%s", fileName, FromWideString(err.ErrorMessage()).c_str());
        }
        return nullptr;
    }
    else if (error)
    {
        const char* errorMsg = (const char*)error->GetBufferPointer();
        logFn(LogLevel::Warn, "Shader compilation warning %s:\n%s", fileName, errorMsg);
        error->Release();
        error = nullptr;
    }

    return shader;
}

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
    std::vector<std::string>* allFiles)
{
    ID3DBlob* shader = CompileShaderToByteCode_Private(fileName, entryPoint, shaderModel, defines, debugShaders, logFn, allFiles);
    if (!shader)
        return false;

    // Put shader bytecode into PSO
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = rootSig;
    desc.CS.pShaderBytecode = shader->GetBufferPointer();
    desc.CS.BytecodeLength = shader->GetBufferSize();

    // Make PSO
    HRESULT hr = device->CreateComputePipelineState(&desc, IID_PPV_ARGS(pso));
    if (FAILED(hr))
    {
        logFn(LogLevel::Error, "Could not create PSO for shader %s", fileName);
        return false;
    }

    if (shader) shader->Release();
    shader = nullptr;

    if (debugName)
        (*pso)->SetName(ToWideString(debugName).c_str());

    return true;
}

std::vector<unsigned char> CompileShaderToByteCode_fxc(
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    bool debugShaders,
    LogFn logFn,
    std::vector<std::string>* allFiles)
{
    std::vector<unsigned char> ret;

    ID3DBlob* shader = CompileShaderToByteCode_Private(fileName, entryPoint, shaderModel, defines, debugShaders, logFn, allFiles);
    if (!shader)
        return ret;

    ret.resize(shader->GetBufferSize());
    memcpy(ret.data(), shader->GetBufferPointer(), ret.size());

    shader->Release();
    return ret;
}