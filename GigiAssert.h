///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define Assert(X, MSG, ...) \
{ \
	if ((X) == false) \
	{ \
		static bool ignoreThis = false; \
		if (!ignoreThis) \
		{ \
			if(!ShowErrorMessage( __FUNCTION__ "() Line " TOSTRING(__LINE__) ":\n\nExpression:\n" #X "\n\n" MSG, __VA_ARGS__)) \
				ignoreThis = true; \
		} \
	} \
}

enum class MessageType
{
	Info,
	Warn,
	Error
};

using GigiPrintMessageFn = void (*)(MessageType messageType, const char* msg);

void SetGigiPrintMessage(const GigiPrintMessageFn & printMessageFn);
void SetGigiHeadlessMode(bool headless);

void ShowInfoMessage(const char* msg, ...);
bool ShowErrorMessage(const char* msg, ...);
void ShowWarningMessage(const char* msg, ...);
bool AskForConfirmation(const char* msg, ...);
void ShowMessageBox(const char* msg, ...);
