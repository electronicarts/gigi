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
#include <comdef.h>

// Note: you can specify include directories by pushing into arguments "-I <path>" which i think would get rid of the need for a custom include handler.

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
	const ShaderCompilationInfo& shaderInfo,
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

    std::string shaderDir = shaderInfo.fileName.parent_path().string();

    IncludeHandlerDXC include(shaderDir.c_str());
    uint32_t codePage = CP_UTF8;
    IDxcBlobEncoding* sourceBlob = nullptr;
    std::wstring fullFileName = shaderInfo.fileName.wstring();
    hr = library->CreateBlobFromFile(fullFileName.c_str(), &codePage, &sourceBlob);

    if (FAILED(hr))
    {
        logFn(LogLevel::Error, "Could not load shader file \"%s\"", shaderInfo.fileName.string().c_str());
        return nullptr;
    }

    std::vector<LPCWSTR> arguments;

    arguments.push_back(fullFileName.c_str()); // 1st positional argument is the source file name

    std::wstring entryPointW = ToWideString(shaderInfo.entryPoint.c_str());
    std::wstring shaderModelW = ToWideString(shaderInfo.shaderModel.c_str());

    if (!shaderInfo.entryPoint.empty())
    {
        arguments.push_back(L"-E");
        arguments.push_back(entryPointW.c_str());
    }

    arguments.push_back(L"-T");
    arguments.push_back(shaderModelW.c_str());

    // PDBs require shader debugging on to work
    if ((shaderInfo.flags & ShaderCompilationFlags::Debug) != ShaderCompilationFlags::None || (shaderInfo.flags & ShaderCompilationFlags::CreatePDBsAndBinaries) != ShaderCompilationFlags::None)
    {
        arguments.push_back(DXC_ARG_DEBUG);
        arguments.push_back(DXC_ARG_DEBUG_NAME_FOR_SOURCE);
        arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
        arguments.push_back(L"-Qembed_debug");
    }

    if ((shaderInfo.flags & ShaderCompilationFlags::WarningsAsErrors) != ShaderCompilationFlags::None)
    {
        arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
    }

    if ((shaderInfo.flags & ShaderCompilationFlags::HLSL2021) != ShaderCompilationFlags::None)
        arguments.push_back(L"-HV 2021");

    if ((shaderInfo.flags & ShaderCompilationFlags::Enable16BitTypes) != ShaderCompilationFlags::None)
        arguments.push_back(L"-enable-16bit-types");

    std::vector<std::wstring> shaderDefinitions;
    shaderDefinitions.reserve(shaderInfo.defines.size());
    for (const auto& shaderDefine : shaderInfo.defines)
    {
        if (!shaderDefine.name.empty() && !shaderDefine.value.empty())
        {
			arguments.push_back(L"-D");

            std::wstring& definition = shaderDefinitions.emplace_back();
            definition = ToWideString(shaderDefine.name.c_str()) + L"=" + ToWideString(shaderDefine.value.c_str());
            arguments.push_back(definition.c_str());
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
        allFiles->push_back(shaderInfo.fileName.string());
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
    hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&code), nullptr);

    if ((shaderInfo.flags & ShaderCompilationFlags::CreatePDBsAndBinaries) != ShaderCompilationFlags::None)
    {
		const std::filesystem::path directoryPath = "./ShaderDebugInfo";

        // PDBs
        {
			IDxcBlob* pPDB = nullptr;
			IDxcBlobUtf16* pPDBName = nullptr;
			hr = result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
			if (SUCCEEDED(hr))
			{
				std::filesystem::path fullPDBFileName = directoryPath / std::filesystem::path(shaderInfo.fileName.stem().string() + ".pdb");

				if (!std::filesystem::exists(directoryPath))
				{
					std::filesystem::create_directories(directoryPath);
				}

				FILE* fp = NULL;
				if (_wfopen_s(&fp, fullPDBFileName.c_str(), L"wb") == 0)
				{
					void* bufferPointer = pPDB->GetBufferPointer();
					size_t bufferSize = pPDB->GetBufferSize();

					fwrite(bufferPointer, bufferSize, 1, fp);
					fclose(fp);
				}
			}
            else
            {
                _com_error err(hr);
                logFn(LogLevel::Warn, "Shader %ls requested PDBs and binaries but we were unable to get them: %ls\n", fullFileName.c_str(), err.ErrorMessage());
            }

            if (pPDBName)
			    pPDBName->Release();

            if(pPDB)
			    pPDB->Release();
        }

        // Binaries
        {
			std::filesystem::path fullBinaryFileName = directoryPath / std::filesystem::path(shaderInfo.fileName.stem().string() + ".cso");

			if (!std::filesystem::exists(directoryPath))
			{
				std::filesystem::create_directories(directoryPath);
			}

			FILE* fp = NULL;
			if (_wfopen_s(&fp, fullBinaryFileName.c_str(), L"wb") == 0)
			{
				void* bufferPointer = code->GetBufferPointer();
				size_t bufferSize = code->GetBufferSize();

				fwrite(bufferPointer, bufferSize, 1, fp);
				fclose(fp);
			}
        }
    }

    library->Release();
    compiler->Release();
    sourceBlob->Release();

    return code;
}


bool MakeComputePSO_dxc(
    ID3D12Device* device,
	const ShaderCompilationInfo& shaderInfo,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState** pso,
    LogFn logFn,
    std::vector<std::string>* allFiles)
{
    IDxcBlob* code = CompileShaderToByteCode_Private(shaderInfo, logFn, allFiles);
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

    if (!shaderInfo.debugName.empty())
        (*pso)->SetName(ToWideString(shaderInfo.debugName.c_str()).c_str());

    return true;
}

std::vector<unsigned char> CompileShaderToByteCode_dxc(
	const ShaderCompilationInfo& shaderInfo,
    LogFn logFn,
    std::vector<std::string>* allFiles)
{
    std::vector<unsigned char> ret;

    IDxcBlob* code = CompileShaderToByteCode_Private(shaderInfo, logFn, allFiles);
    if (!code)
        return ret;

    ret.resize(code->GetBufferSize());
    memcpy(ret.data(), code->GetBufferPointer(), ret.size());

    code->Release();
    return ret;
}
