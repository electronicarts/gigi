///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Parse.h"
#include <vector>

struct Enum;
struct ParseErrorObject;
struct Struct;

struct ParseErrorObject
{
	//
	std::string errorMessage;
	// starting with 1, only meaningful if !text.empty()
	uint32_t line = 0;
	// starting with 1, only meaningful if !text.empty()
	uint32_t column = 0;
	//
	std::string errorLine;
};

// reads cb.definition and generates all fields from that
// without C comments but trailing C++ comments are allowed
// e.g.
// struct MyName
// {
//   int a,b;
//   float c; // comment for c
//   float4 d, e; uint f;
//   float2 more_With_Longer_Names;
//   uint a : SV_COUNT = 12; // with semantic 'SV_COUNT'
// };
// @param definition must not be 0
// @param outError
// @return success
bool reparseStructDefinition(const char* definition, std::vector<Enum>& enums, std::vector<Struct>& structs, ParseErrorObject& parseErrorObject);

bool parseComplexLiteral(const Char*& p, std::string& Out);
