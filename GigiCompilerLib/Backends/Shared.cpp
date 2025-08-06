///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Shared.h"
#include "GigiCompilerLib/ProcessSlang.h"

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::filesystem::path GetExePath()
{
	char exePath[_MAX_PATH + 1];
	GetModuleFileNameA(NULL, exePath, _MAX_PATH);
	std::filesystem::path ret(exePath);
	ret.replace_filename("");
	return ret;
}

bool ConvertShaderSourceCode(std::string& source, const char* fileName, const char* shaderModel, const char* stage, const char* entryPoint, const std::vector<std::string>& includeDirectories, ShaderLanguage sourceLanguage, ShaderLanguage destinationLanguage, std::string& errorMessage, const SlangOptions& slangOptions)
{
	// If no conversion needed, we are done
	if (sourceLanguage == destinationLanguage)
		return true;

	// If the source language is slang, slang can output the language specified
	if (sourceLanguage == ShaderLanguage::Slang)
		return ProcessWithSlang(source, fileName, destinationLanguage, stage, entryPoint, shaderModel, errorMessage, includeDirectories, slangOptions);

	// If the source language is HLSL btu we want WGSL, have slang convert for us
	if (sourceLanguage == ShaderLanguage::HLSL && destinationLanguage == ShaderLanguage::WGSL)
		return ProcessWithSlang(source, fileName, destinationLanguage, stage, entryPoint, shaderModel, errorMessage, includeDirectories, slangOptions);

	// Otherwise, conversion not currently implemented
	return false;
}
