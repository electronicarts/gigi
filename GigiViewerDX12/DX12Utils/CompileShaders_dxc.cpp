///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// Portions of this software were based on https://simoncoenen.com/blog/programming/graphics/DxcCompiling

#include "CompileShaders.h"

#include <d3d12.h>

#include <dxcapi.h>
#include <vector>
#include <filesystem>

#pragma comment(lib, "dxcompiler.lib")

class IncludeHandlerDXC : public IDxcIncludeHandler
{
public:
    IncludeHandlerDXC(const char* directory)
    {
        HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
        m_utils->CreateDefaultIncludeHandler(&m_defaultIncludeHandler);
        m_directory = ToWideString(directory);
    }

    ~IncludeHandlerDXC()
    {
        m_utils->Release();
        m_defaultIncludeHandler->Release();
    }

    HRESULT LoadSource(LPCWSTR pFileName, IDxcBlob** ppIncludeSource) override
    {
        IDxcBlobEncoding* encoding = nullptr;
        std::wstring fullFileName = std::filesystem::path(m_directory) / pFileName;
        m_includeFiles.push_back(FromWideString(fullFileName.c_str()));
        HRESULT hr = m_utils->LoadFile(fullFileName.c_str(), nullptr, &encoding);

        if (SUCCEEDED(hr))
            *ppIncludeSource = encoding;
        else
            *ppIncludeSource = nullptr;

        return hr;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
    {
        return m_defaultIncludeHandler->QueryInterface(riid, ppvObject);
    }

    ULONG AddRef(void) override { return 0; }
    ULONG Release(void) override { return 0; }

    std::vector<std::string> m_includeFiles;

    std::wstring m_directory;
    IDxcUtils* m_utils = nullptr;
    IDxcIncludeHandler* m_defaultIncludeHandler = nullptr;
};

static IDxcBlob* CompileShaderToByteCode_Private(
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines_,
    bool debugShaders,
    bool HV2021,
    LogFn logFn,
    std::vector<std::string>* allFiles)
{
    IDxcLibrary* library = nullptr;
    HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    if (FAILED(hr))
    {
        logFn(LogLevel::Error, "Could not create a IDxcLibrary");
        return nullptr;
    }

    IDxcCompiler3* compiler = nullptr;
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    if (FAILED(hr))
    {
        logFn(LogLevel::Error, "Could not create a IDxcCompiler3");
        return nullptr;
    }

    std::string shaderDir = std::filesystem::path(fileName).parent_path().string();

    IncludeHandlerDXC include(shaderDir.c_str());
    uint32_t codePage = CP_UTF8;
    IDxcBlobEncoding* sourceBlob = nullptr;
    std::wstring fullFileName = ToWideString(fileName);
    hr = library->CreateBlobFromFile(fullFileName.c_str(), &codePage, &sourceBlob);

    if (FAILED(hr))
    {
        logFn(LogLevel::Error, "Could not load shader file \"%s\"", fileName);
        return nullptr;
    }

    std::vector<LPCWSTR> arguments;

    arguments.push_back(fullFileName.c_str()); // 1st positional argument is the source file name

    std::wstring entryPointW = ToWideString(entryPoint);
    std::wstring shaderModelW = ToWideString(shaderModel);

    if (entryPoint && entryPoint[0])
    {
        arguments.push_back(L"-E");
        arguments.push_back(entryPointW.c_str());
    }

    arguments.push_back(L"-T");
    arguments.push_back(shaderModelW.c_str());

    if (debugShaders)
    {
        arguments.push_back(DXC_ARG_DEBUG);
        arguments.push_back(DXC_ARG_DEBUG_NAME_FOR_SOURCE);
        arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
        arguments.push_back(L"-Qembed_debug");
    }

    if (HV2021)
        arguments.push_back(L"-HV 2021");

    std::vector<std::wstring> defineWStrings;
    if (defines_)
    {
        int i = 0;
        while (defines_[i].Name)
        {
            defineWStrings.push_back(ToWideString(defines_[i].Name) + L"=" + ToWideString(defines_[i].Definition));
            i++;
        }

        i = 0;
        while (defines_[i].Name)
        {
            arguments.push_back(L"-D");
            arguments.push_back(defineWStrings[i].c_str());
            i++;
        }
    }

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = 0;

    IDxcResult* result = nullptr;
    hr = compiler->Compile(
        &sourceBuffer,
        arguments.data(), (UINT32)arguments.size(),
        &include,
        IID_PPV_ARGS(&result));

    if (allFiles)
    {
        allFiles->push_back(fileName);
        for (const std::string& fileName : include.m_includeFiles)
            allFiles->push_back(fileName);
    }

    if (SUCCEEDED(hr))
        result->GetStatus(&hr);

    if (FAILED(hr))
    {
        if (result)
        {
            IDxcBlobEncoding* errorsBlob = nullptr;
            hr = result->GetErrorBuffer(&errorsBlob);
            if (SUCCEEDED(hr) && errorsBlob)
                logFn(LogLevel::Error, "Shader %ls failed to compile with errors:\n%s\n", fullFileName.c_str(), (const char*)errorsBlob->GetBufferPointer());
        }
        else
        {
            logFn(LogLevel::Error, "Shader %ls failed to compile\n", fullFileName.c_str());
        }
        return nullptr;
    }

    IDxcBlob* code = nullptr;
    result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&code), nullptr);

    /*
    // Save the PDBs.
    if (debugShaders)
    {
        IDxcBlob* pPDB = nullptr;
        IDxcBlobUtf16* pPDBName = nullptr;
        result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
        {
            std::wstring fullPDBFileName = std::wstring(L"./ShaderPDBs/") + pPDBName->GetStringPointer();
            FILE* fp = NULL;
            _wfopen_s(&fp, fullPDBFileName.c_str(), L"wb");
            fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp);
            fclose(fp);
        }

        pPDBName->Release();
        pPDB->Release();
    }
    */

    library->Release();
    compiler->Release();
    sourceBlob->Release();

    return code;
}


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
    std::vector<std::string>* allFiles)
{
    IDxcBlob* code = CompileShaderToByteCode_Private(fileName, entryPoint, shaderModel, defines, debugShaders, HV2021, logFn, allFiles);
    if (!code)
        return false;

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSig;
    psoDesc.CS.BytecodeLength = code->GetBufferSize();
    psoDesc.CS.pShaderBytecode = code->GetBufferPointer();

    HRESULT  hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pso));
    if (FAILED(hr))
        return false;

    code->Release();

    if (debugName)
        (*pso)->SetName(ToWideString(debugName).c_str());

    return true;
}

std::vector<unsigned char> CompileShaderToByteCode_dxc(
    const char* fileName,
    const char* entryPoint,
    const char* shaderModel,
    const D3D_SHADER_MACRO* defines,
    bool debugShaders,
    bool HV2021,
    LogFn logFn,
    std::vector<std::string>* allFiles)
{
    std::vector<unsigned char> ret;

    IDxcBlob* code = CompileShaderToByteCode_Private(fileName, entryPoint, shaderModel, defines, debugShaders, HV2021, logFn, allFiles);
    if (!code)
        return ret;

    ret.resize(code->GetBufferSize());
    memcpy(ret.data(), code->GetBufferPointer(), ret.size());

    code->Release();
    return ret;
}
