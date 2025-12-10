///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

struct GGUserFile_WebCam;
class GigiInterpreterPreviewWindowDX12;
struct ID3D12GraphicsCommandList;

namespace WebCam
{
	void Init();

	bool PreRender(const GGUserFile_WebCam& settings, GigiInterpreterPreviewWindowDX12& interpreter, ID3D12GraphicsCommandList* commandList);

	std::string GetDeviceName();

	void Shutdown();
};