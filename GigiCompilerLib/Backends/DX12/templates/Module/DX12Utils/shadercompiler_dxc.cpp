#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "shadercompiler.h"
#include "../public/technique.h"
#include <dxcapi.h>
#include <string>
#include <codecvt>

#pragma comment(lib, "dxcompiler.lib")

namespace DX12Utils
{
    static IDxcBlob* CompileShaderToByteCode_Private(
        LPCWSTR shaderDir,
        LPCWSTR shaderFile,
        const char* entryPoint,
        const char* shaderModel,
        const D3D_SHADER_MACRO* defines_,
        bool debugShaders,
        TLogFn logFn)
    {
        IDxcLibrary* library = nullptr;
        HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
        if (FAILED(hr))
        {
            logFn(LogLevel::Error, "Could not create a IDxcLibrary");
            return nullptr;
        }

        IDxcCompiler* compiler = nullptr;
        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        if (FAILED(hr))
        {
            logFn(LogLevel::Error, "Could not create a IDxcCompiler");
            return nullptr;
        }

        IDxcIncludeHandler* includeHandler = nullptr;
        hr = library->CreateIncludeHandler(&includeHandler);
        if (FAILED(hr))
        {
            logFn(LogLevel::Error, "Could not create an include handler");
            return nullptr;
        }

        uint32_t codePage = CP_UTF8;
        IDxcBlobEncoding* sourceBlob = nullptr;
        std::wstring fullFileName = std::wstring(shaderDir) + std::wstring(shaderFile);
        hr = library->CreateBlobFromFile(fullFileName.c_str(), &codePage, &sourceBlob);

        typedef std::codecvt_utf8<wchar_t> convert_type;
        std::wstring_convert<convert_type, wchar_t> converter;
        std::wstring entryPointW = converter.from_bytes(entryPoint);
        std::wstring shaderModelW = converter.from_bytes(shaderModel);

        std::vector<DxcDefine> defines;
        std::vector<std::wstring> wstrings;
        if (defines_)
        {
            int i = 0;
            while (defines_[i].Name)
            {
                wstrings.push_back(converter.from_bytes(defines_[i].Name));
                wstrings.push_back(converter.from_bytes(defines_[i].Definition));
                i++;
            }

            i = 0;
            while (defines_[i].Name)
            {
                DxcDefine define;
                define.Name = wstrings[i * 2 + 0].c_str();
                define.Value = wstrings[i * 2 + 1].c_str();
                defines.push_back(define);
                i++;
            }
        }

        IDxcOperationResult* result = nullptr;
        hr = compiler->Compile(
            sourceBlob, // pSource
            fullFileName.c_str(), // pSourceName
            entryPointW.c_str(), // pEntryPoint
            shaderModelW.c_str(), // pTargetProfile
            NULL, 0, // pArguments, argCount
            defines.data(), (UINT32)defines.size(), // pDefines, defineCount
            includeHandler, // pIncludeHandler
            &result); // ppResult

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
            return nullptr;
        }

        IDxcBlob* code = nullptr;
        result->GetResult(&code);

        library->Release();
        compiler->Release();
        includeHandler->Release();
        sourceBlob->Release();

        return code;
    }

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
        TLogFn logFn)
    {
        IDxcBlob* code = CompileShaderToByteCode_Private(shaderDir, shaderFile, entryPoint, shaderModel, defines, debugShaders, logFn);
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

        return true;
    }

    std::vector<unsigned char> CompileShaderToByteCode_DXC(
        LPCWSTR shaderDir,
        LPCWSTR shaderFile,
        const char* entryPoint,
        const char* shaderModel,
        const D3D_SHADER_MACRO* defines,
        bool debugShaders,
        TLogFn logFn)
    {
        std::vector<unsigned char> ret;

        IDxcBlob* code = CompileShaderToByteCode_Private(shaderDir, shaderFile, entryPoint, shaderModel, defines, debugShaders, logFn);
        if(!code)
            return ret;

        ret.resize(code->GetBufferSize());
        memcpy(ret.data(), code->GetBufferPointer(), ret.size());

        code->Release();
        return ret;
    }
}; // namespace DX12Utils
