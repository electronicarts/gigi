/*$(CopyrightHeader)*/#include "CompileShaders.h"

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <vector>
#include <comdef.h>
#include <filesystem>

#include <Windows.h>

static std::wstring ToWideString(const char* string)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, string, (int)strlen(string), nullptr, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, string, (int)strlen(string), result.data(), size);
    return result;
}

static std::string FromWideString(const wchar_t* string)
{
    int size = WideCharToMultiByte(CP_ACP, 0, string, -1, nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_ACP, 0, string, -1, result.data(), size, nullptr, nullptr);
    return result;
}

namespace DX12Utils
{

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
    const std::wstring& fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    bool debugShaders,
    TLogFn logFn
    )
{
    // Compile Shaders
    ID3DBlob* error = nullptr;
    ID3DBlob* shader = nullptr;
    UINT compileFlags = debugShaders ? D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION : 0;

    // compile the shader from file
    std::string shaderDir = std::filesystem::path(fileName).parent_path().string();
    IncludeHandlerFXC include(shaderDir.c_str());
    HRESULT hr = D3DCompileFromFile(fileName.c_str(), defines, &include, entryPoint, shaderModel, compileFlags, 0, &shader, &error);

    if (FAILED(hr))
    {
        _com_error err(hr);
        if (error)
        {
            const char* errorMsg = (const char*)error->GetBufferPointer();
            logFn(LogLevel::Error, "Could not compile shader %ls:\n%s\n%s", fileName.c_str(), err.ErrorMessage(), errorMsg);
            error->Release();
            error = nullptr;
        }
        else
        {
            logFn(LogLevel::Error, "Could not compile shader %ls:\n%s", fileName.c_str(), err.ErrorMessage());
        }
        return nullptr;
    }
    else if (error)
    {
        const char* errorMsg = (const char*)error->GetBufferPointer();
        logFn(LogLevel::Warn, "Shader compilation warning %ls:\n%s", fileName.c_str(), errorMsg);
        error->Release();
        error = nullptr;
    }

    return shader;
}

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
    TLogFn logFn)
{
    std::wstring fileName = std::filesystem::path(shaderDir) / std::filesystem::path(shaderFile).wstring();

    ID3DBlob* shader = CompileShaderToByteCode_Private(fileName, entryPoint, shaderModel, defines, debugShaders, logFn);
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
        (*pso)->SetName(debugName);

    return true;
}

std::vector<unsigned char> CompileShaderToByteCode_FXC(
    LPCWSTR shaderDir,
    LPCWSTR shaderFile,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    bool debugShaders,
    TLogFn logFn)
{
    std::vector<unsigned char> ret;

    std::wstring fileName = std::filesystem::path(shaderDir) / std::filesystem::path(shaderFile).wstring();

    ID3DBlob* shader = CompileShaderToByteCode_Private(fileName, entryPoint, shaderModel, defines, debugShaders, logFn);
    if (!shader)
        return ret;

    ret.resize(shader->GetBufferSize());
    memcpy(ret.data(), shader->GetBufferPointer(), ret.size());

    shader->Release();
    return ret;
}

}