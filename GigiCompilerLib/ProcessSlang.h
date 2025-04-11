///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

struct SlangOptions;

bool ProcessWithSlang(std::string& source, const char* fileName, const char* stage, const char* entryPoint, const char* profile, std::string& errorMessage, const char* workingDirectory, const SlangOptions& options);
