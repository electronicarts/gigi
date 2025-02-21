#pragma once

#include "Utils.h"

#include <filesystem>

enum class ShaderCompilationFlags : uint32_t
{
	None = 0,
	Debug = 1 << 0,
	WarningsAsErrors = 1 << 1,
	HLSL2021 = 1 << 2,
	CreatePDBsAndBinaries = 1 << 3
};

SETUP_ENUM_CLASS_OPERATORS(ShaderCompilationFlags)

struct ShaderCompilationInfo
{
	std::filesystem::path fileName;
	std::string entryPoint;
	std::string shaderModel;
	std::string debugName;
	std::vector<ShaderDefine> defines;

	ShaderCompilationFlags flags = ShaderCompilationFlags::None;
};