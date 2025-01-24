///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "processthreadsapi.h"

#include "Schemas/JSON.h"

inline bool IsNewline(char c)
{
	return c == '\r' || c == '\n';
}

inline bool IsWhiteSpace(char c)
{
	return c == ' ' || c == '\t' || IsNewline(c);
}

inline char* ScanUntilWhitespace(char* s)
{
	while (*s && !IsWhiteSpace(*s))
		s++;
	return s;
}

inline char* ScanUntilNotWhitespace(char* s)
{
	while (*s && IsWhiteSpace(*s))
		s++;
	return s;
}

inline char* ScanUntilNewline(char* s)
{
	while (*s && !IsNewline(*s))
		s++;
	return s;
}

inline char* ScanUntilNotNewline(char* s)
{
	while (*s && IsNewline(*s))
		s++;
	return s;
}

inline bool GetStringUntilWhitespace(std::string& dest, char*& start)
{
	char* end = ScanUntilWhitespace(start);
	if (end == start)
		return false;

	bool endOfString = (end[0] == 0);

	end[0] = 0;
	dest = start;

	if (endOfString)
		start = end;
	else
		start = ScanUntilNotNewline(end + 1);

	return true;
}

inline bool GetStringUntilNewline(std::string& dest, char*& start)
{
	char* end = ScanUntilNewline(start);
	if (end == start)
		return false;

	bool endOfString = (end[0] == 0);

	end[0] = 0;
	dest = start;

	if (endOfString)
		start = end;
	else
		start = ScanUntilNotNewline(end + 1);

	return true;
}

inline void RunCommandLine(bool blocking, const char* format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
	va_end(args);

	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;

	CreateProcessA(
		nullptr,
		buffer,
		nullptr,
		nullptr,
		FALSE,
		CREATE_NO_WINDOW,
		nullptr,
		nullptr,
		&si,
		&pi);

	if (blocking)
		WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

inline size_t HashCombine(size_t a, size_t b)
{
	a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
	return a;
}

inline size_t GetSummaryHash(const BrowserTechniqueSummary& summary)
{
	size_t hash1 = std::hash<std::string>()(summary.Repo);
	size_t hash2 = std::hash<std::string>()(summary.Commit);
	size_t hash3 = std::hash<std::string>()(summary.DetailsFile);
	size_t hash4 = std::hash<std::string>()(summary.ListedDate);

	size_t hash12 = HashCombine(hash1, hash2);
	size_t hash34 = HashCombine(hash3, hash4);

	return HashCombine(hash12, hash34);
}

inline std::string GetSummaryHashString(const BrowserTechniqueSummary& summary)
{
	std::stringstream ret;
	ret << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << GetSummaryHash(summary);
	return ret.str();
}
