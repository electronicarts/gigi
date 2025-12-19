///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Utils.h"

#include <Windows.h>

std::wstring ToWideString(const char* string)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, string, (int)strlen(string), nullptr, 0);
	std::wstring result(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, string, (int)strlen(string), result.data(), size);
	return result;
}

std::string FromWideString(const wchar_t* string)
{
	int size = WideCharToMultiByte(CP_ACP, 0, string, -1, nullptr, 0, nullptr, nullptr);
	std::string result(size, 0);
	WideCharToMultiByte(CP_ACP, 0, string, -1, result.data(), size, nullptr, nullptr);
	return result;
}

std::string FromWideStringUTF8(const wchar_t* string)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, string, -1, nullptr, 0, nullptr, nullptr);
	std::string result(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, string, -1, result.data(), size, nullptr, nullptr);
	return result;
}