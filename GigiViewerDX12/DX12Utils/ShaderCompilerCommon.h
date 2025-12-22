#pragma once

#include "Utils.h"

#include <filesystem>

enum class ShaderCompilationFlags : uint32_t
{
	None = 0,
	Debug = 1 << 0,
	WarningsAsErrors = 1 << 1,
	HLSL2021 = 1 << 2,
	CreatePDBsAndBinaries = 1 << 3,
	Enable16BitTypes = 1 << 4,
    BetterShaderErrors = 1 << 5 
};

SETUP_ENUM_CLASS_OPERATORS(ShaderCompilationFlags)

struct ShaderCompilationInfo
{
    // e.g. "C:/gitlab/gigiexperiments/IntersectExperiment", no "\\"
    std::string rootDirectory;
    // relative path e.g. "FinalRenderCS.hlsl"
    std::string sourceFileName;
    // full path to temp filename e.g. "C:\Users\someUser\AppData\Local\Temp\1\Gigi\3340\shaders\FinalRenderCS.hlsl"
	std::filesystem::path fileName;
	std::string entryPoint;
	std::string shaderModel;
	std::string debugName;
	std::vector<ShaderDefine> defines;

	ShaderCompilationFlags flags = ShaderCompilationFlags::None;
};