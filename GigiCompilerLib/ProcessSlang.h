///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>

enum class ShaderLanguage;
struct SlangOptions;

bool ProcessWithSlang(std::string& source, const char* fileName, ShaderLanguage destinationLanguage, const char* stage, const char* entryPoint, const char* profile, std::string& errorMessage, const std::vector<std::string>& includeDirectories, const SlangOptions& options);
