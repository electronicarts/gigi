///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Shared.h"

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
