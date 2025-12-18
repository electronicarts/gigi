///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

struct GGUserFile_AMD_FidelityFXSDK_FrameInterpolation;
class GigiInterpreterPreviewWindowDX12;
struct IDXGISwapChain4;
struct ID3D12CommandQueue;
struct ID3D12Device14;
struct ID3D12GraphicsCommandList;
struct GGUserFile_SystemVars;

class Camera;

namespace AMDFrameInterpolation
{
    struct Desc
    {
        ID3D12Device14* device = nullptr;
        ID3D12GraphicsCommandList* commandList = nullptr;
        int frameIndex = 0;
        float jitterOffset[2] = { 0.5f, 0.5f };
        float frameTimeMS = 16.67f;
        int imagePosition[2] = { 0, 0 };
        int imageSize[2] = { 0, 0 };
        int imageClipMin[2] = { 0, 0 };
        int imageClipMax[2] = { 0, 0 };
    };

    struct Version
    {
        uint64_t versionId = 0;
        std::string versionName;
    };

    void Init(const GigiInterpreterPreviewWindowDX12& interpreter, ID3D12Device14* device, IDXGISwapChain4*& swapChain, ID3D12CommandQueue* commandQueue);
    void Release();

    void Tick(const GGUserFile_AMD_FidelityFXSDK_FrameInterpolation& settings, const GGUserFile_SystemVars& sysSettigns, GigiInterpreterPreviewWindowDX12& interpreter, IDXGISwapChain4*& swapChain, const Camera& cameraState, const Desc& desc);

    const std::vector<Version>& GetVersions(ID3D12Device14* device);
};