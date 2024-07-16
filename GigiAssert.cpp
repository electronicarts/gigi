///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiAssert.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#include "spdlog/spdlog.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

GigiPrintMessageFn g_printMessageFn = [](MessageType messageType, const char* msg)
{
	switch (messageType)
	{
		case MessageType::Info: spdlog::info(msg); break;
		case MessageType::Warn: spdlog::warn(msg); break;
		case MessageType::Error: spdlog::error(msg); break;
	}
};

bool g_headlessMode = false;

void SetGigiPrintMessage(const GigiPrintMessageFn& printMessageFn)
{
	g_printMessageFn = printMessageFn;
}

void SetGigiHeadlessMode(bool headless)
{
	g_headlessMode = headless;
}

bool AskForConfirmation(const char* msg, ...)
{
	if (g_headlessMode)
		return true;

	char buffer[4096];
	va_list args;
	va_start(args, msg);
	vsprintf_s(buffer, msg, args);
	va_end(args);

	return MessageBoxA(nullptr, buffer, "Gigi", MB_OKCANCEL) == IDOK;
}

void ShowMessageBox(const char* msg, ...)
{
	if (!g_headlessMode)
	{
		char buffer[4096];
		va_list args;
		va_start(args, msg);
		vsprintf_s(buffer, msg, args);
		va_end(args);

		MessageBoxA(nullptr, buffer, "Gigi", MB_OK);
	}
}

void ShowInfoMessage(const char* msg, ...)
{
	char buffer[4096];
	{
		va_list args;
		va_start(args, msg);
		vsprintf_s(buffer, msg, args);
		va_end(args);
		g_printMessageFn(MessageType::Info, buffer);
	}
}

void ShowWarningMessage(const char* msg, ...)
{
	char buffer[4096];
	{
		va_list args;
		va_start(args, msg);
		vsprintf_s(buffer, msg, args);
		va_end(args);
		g_printMessageFn(MessageType::Warn, buffer);
	}
}

bool ShowErrorMessage(const char* msg, ...)
{
	char buffer[4096];
	{
		va_list args;
		va_start(args, msg);
		vsprintf_s(buffer, msg, args);
		va_end(args);
		g_printMessageFn(MessageType::Error, buffer);

		// to Visual Studio Output Debug
		// so there is something when press the users presses "Retry"
		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");
	}

	if (!g_headlessMode)
	{
		switch (MessageBoxA(nullptr, buffer, "Gigi Assert", MB_ABORTRETRYIGNORE | MB_DEFBUTTON3 | MB_ICONERROR))
		{
			case IDRETRY:
			{
				DebugBreak();
				exit(100);
				return false;
			}
			case IDIGNORE:
			{
				return false;
			}
			default:
			case IDABORT:
			{
				return true;
			}
		}
	}
	return false;
}