///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "CompileShaders.h"
#include "ShaderErrorParse.h"

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
	const ShaderCompilationInfo& shaderInfo,
    LogFn logFn,
    std::vector<std::string>* allFiles
    )
{
    // Compile Shaders
    ID3DBlob* error = nullptr;
    ID3DBlob* shader = nullptr;
    UINT compileFlags = ((shaderInfo.flags & ShaderCompilationFlags::Debug) != ShaderCompilationFlags::None) ? D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION : 0;

    if ((shaderInfo.flags & ShaderCompilationFlags::WarningsAsErrors) != ShaderCompilationFlags::None)
    {
        compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
    }

    // compile the shader from file
    std::string shaderDir = shaderInfo.fileName.parent_path().string();
    IncludeHandlerFXC include(shaderDir.c_str());
    std::wstring fullFileName = shaderInfo.fileName;

    std::vector<D3D_SHADER_MACRO> d3dMacros;
    d3dMacros.reserve(shaderInfo.defines.size()+1);
    for (const auto& shaderDefine : shaderInfo.defines)
    {
        if (!shaderDefine.name.empty() && !shaderDefine.value.empty())
        {
            D3D_SHADER_MACRO& macro = d3dMacros.emplace_back();
            macro.Name = shaderDefine.name.c_str();
            macro.Definition = shaderDefine.value.c_str();
        }
    }

    D3D_SHADER_MACRO& macro = d3dMacros.emplace_back();
    macro.Name = nullptr;
    macro.Definition = nullptr;

    HRESULT hr = D3DCompileFromFile(fullFileName.c_str(), d3dMacros.data(), &include, shaderInfo.entryPoint.c_str(), shaderInfo.shaderModel.c_str(), compileFlags, 0, &shader, &error);

    std::string fileNameStr = shaderInfo.fileName.string();

    if (allFiles)
    {
        allFiles->push_back((std::filesystem::path(shaderInfo.rootDirectory) / shaderInfo.sourceFileName).string());
        allFiles->push_back(shaderInfo.fileName.string());
        for (const std::string& fileName : include.m_includeFiles)
            allFiles->push_back(fileName);
    }

    if (FAILED(hr))
    {
        _com_error err(hr);
        if (error)
        {
            // fxc.exe error output is already like VisualStudio wants it with line number in round brackets but the path is to the temp file.
            const char* errorText = (const char*)error->GetBufferPointer();

            // If Visual Studio or some other tool is attached we can double click on the line to jump to the error.

            OutputDebugStringA("\nFXC shader compile error: =======================================================");

            if ((shaderInfo.flags & ShaderCompilationFlags::BetterShaderErrors) != ShaderCompilationFlags::None)
            {
                OutputDebugStringA(" BetterShaderErrors=1:\n");
                // If Visual Studio or some other tool is attached we can double click on the line to jump to the error.
                std::string errorString = fixupHLSLErrors((const Char*)errorText, fileNameStr.c_str(), shaderInfo.rootDirectory, shaderInfo.sourceFileName);
                OutputDebugStringA(errorString.c_str());
                logFn(LogLevel::Error, "Could not compile shader %s:\n%s\n%s", fileNameStr.c_str(), FromWideString(err.ErrorMessage()).c_str(), errorString.c_str());
            }
            else
            {
                logFn(LogLevel::Error, "Could not compile shader %s:\n%s\n%s", fileNameStr.c_str(), FromWideString(err.ErrorMessage()).c_str(), errorText);
                OutputDebugStringA(" BetterShaderErrors=0:\n");
                OutputDebugStringA(errorText);
            }

            OutputDebugStringA("\n");

            error->Release();
            error = nullptr;
        }
        else
        {
            logFn(LogLevel::Error, "Could not compile shader %s:\n%s", fileNameStr.c_str(), FromWideString(err.ErrorMessage()).c_str());
        }
        return nullptr;
    }
    else if (error)
    {
        const char* errorMsg = (const char*)error->GetBufferPointer();
        logFn(LogLevel::Warn, "Shader compilation warning %s:\n%s", fileNameStr, errorMsg);
        error->Release();
        error = nullptr;
    }

    return shader;
}

bool WrapperCreateComputePipelineState(ID3D12Device * device, const D3D12_COMPUTE_PIPELINE_STATE_DESC & psoDesc, ID3D12PipelineState * *pso);

bool MakeComputePSO_fxc(
    ID3D12Device* device,
	const ShaderCompilationInfo& shaderInfo,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState** pso,
    LogFn logFn,
    std::vector<std::string>* allFiles)
{
    ID3DBlob* shader = CompileShaderToByteCode_Private(shaderInfo, logFn, allFiles);
    if (!shader)
        return false;

    // Put shader bytecode into PSO
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSig;
    psoDesc.CS.pShaderBytecode = shader->GetBufferPointer();
    psoDesc.CS.BytecodeLength = shader->GetBufferSize();
    
    if (!WrapperCreateComputePipelineState(device, psoDesc, pso))
    {
        logFn(LogLevel::Error, "Could not create PSO for shader %s", shaderInfo.fileName.string());
        return false;
    }

    if (shader) shader->Release();
    shader = nullptr;

    if (!shaderInfo.debugName.empty())
        (*pso)->SetName(ToWideString(shaderInfo.debugName.c_str()).c_str());

    return true;
}

std::vector<unsigned char> CompileShaderToByteCode_fxc(
	const ShaderCompilationInfo& shaderInfo,
    LogFn logFn,
    std::vector<std::string>* allFiles)
{
    std::vector<unsigned char> ret;

    ID3DBlob* shader = CompileShaderToByteCode_Private(shaderInfo, logFn, allFiles);
    if (!shader)
        return ret;

    ret.resize(shader->GetBufferSize());
    memcpy(ret.data(), shader->GetBufferPointer(), ret.size());

    shader->Release();
    return ret;
}