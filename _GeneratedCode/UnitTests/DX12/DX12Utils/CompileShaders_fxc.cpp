#include "CompileShaders.h"

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
	const ShaderCompilationInfo& shaderInfo,
    TLogFn logFn
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

	std::vector<D3D_SHADER_MACRO> d3dMacros;
	d3dMacros.reserve(shaderInfo.defines.size());
	for (const auto& shaderDefine : shaderInfo.defines)
	{
		if (!shaderDefine.name.empty() && !shaderDefine.value.empty())
		{
			D3D_SHADER_MACRO& macro = d3dMacros.emplace_back();
			macro.Name = shaderDefine.name.c_str();
			macro.Definition = shaderDefine.value.c_str();
		}
	}

    HRESULT hr = D3DCompileFromFile(shaderInfo.fileName.c_str(), d3dMacros.data(), &include, shaderInfo.entryPoint.c_str(), shaderInfo.shaderModel.c_str(), compileFlags, 0, &shader, &error);

	std::string fileNameStr = shaderInfo.fileName.string();

    if (FAILED(hr))
    {
        _com_error err(hr);
        if (error)
        {
            const char* errorMsg = (const char*)error->GetBufferPointer();
            logFn(LogLevel::Error, "Could not compile shader %ls:\n%s\n%s", fileNameStr.c_str(), err.ErrorMessage(), errorMsg);
            error->Release();
            error = nullptr;
        }
        else
        {
            logFn(LogLevel::Error, "Could not compile shader %ls:\n%s", fileNameStr.c_str(), err.ErrorMessage());
        }
        return nullptr;
    }
    else if (error)
    {
        const char* errorMsg = (const char*)error->GetBufferPointer();
        logFn(LogLevel::Warn, "Shader compilation warning %ls:\n%s", fileNameStr.c_str(), errorMsg);
        error->Release();
        error = nullptr;
    }

    return shader;
}

bool MakeComputePSO_FXC(
    ID3D12Device* device,
	const ShaderCompilationInfo& shaderInfo,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState** pso,
    TLogFn logFn)
{
    ID3DBlob* shader = CompileShaderToByteCode_Private(shaderInfo, logFn);
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
        logFn(LogLevel::Error, "Could not create PSO for shader %s", shaderInfo.fileName.string().c_str());
        return false;
    }

    if (shader) shader->Release();
    shader = nullptr;

    if (!shaderInfo.debugName.empty())
        (*pso)->SetName(ToWideString(shaderInfo.debugName.c_str()).c_str());

    return true;
}

std::vector<unsigned char> CompileShaderToByteCode_FXC(
	const ShaderCompilationInfo& shaderInfo,
    TLogFn logFn)
{
    std::vector<unsigned char> ret;

    ID3DBlob* shader = CompileShaderToByteCode_Private(shaderInfo, logFn);
    if (!shader)
        return ret;

    ret.resize(shader->GetBufferSize());
    memcpy(ret.data(), shader->GetBufferPointer(), ret.size());

    shader->Release();
    return ret;
}

}