///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

struct GGUserFile_Audio;
class GigiInterpreterPreviewWindowDX12;
struct ID3D12GraphicsCommandList;

namespace Audio
{
    bool Init(int framesInFlight);
    void Shutdown(GigiInterpreterPreviewWindowDX12& interpreter);

    bool PreRender(const GGUserFile_Audio& audioSettings, GigiInterpreterPreviewWindowDX12& interpreter, ID3D12GraphicsCommandList* commandList);
    void PostRender(const GGUserFile_Audio& audioSettings, GigiInterpreterPreviewWindowDX12& interpreter, ID3D12GraphicsCommandList* commandList, int maxFramesInFlight, bool forceSilence);

    std::string GetInputDeviceName();
    std::string GetOutputDeviceName();
};