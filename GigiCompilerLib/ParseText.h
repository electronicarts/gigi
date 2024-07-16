///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace ParseText
{
	inline bool IsNewLine(char c)
	{
		return c == '\r' || c == '\n';
	}

	inline bool IsWhiteSpace(char c)
	{
		return c == ' ' || c == '\t';
	}

	inline bool IsQuote(char c)
	{
		return c == '\'' || c == '\"';
	}

	inline std::string_view SkipToNewLine(const std::string_view& string)
	{
		// Skip forward until a new line or null
		int i = 0;
		while (i < string.length() && string[i] && !IsNewLine(string[i]))
			i++;

		// Skip forward until not a new line
		while (i < string.length() && IsNewLine(string[i]))
			i++;

		if (i < string.length())
			return std::string_view(&string[i]);
		else
			return std::string_view("");
	}

	inline bool ReadLine(std::string_view& string, std::string_view& line)
	{
		// Get until new line or end of string
		size_t i = 0;
		while (i < string.length() && !IsNewLine(string[i]))
			i++;
		line = string.substr(0, i);

		// skip all new line characters
		while (i < string.length() && IsNewLine(string[i]))
			i++;
		if (i < string.length())
			string = std::string_view(&string[i]);
		else
			string = std::string_view("");

		// return whether or not we got anything
		return line.size() > 0;
	}

	inline bool ReadWord(std::string_view& string, std::string_view& word)
	{
		// skip any white space and newlines
		size_t start = 0;
		while (start < string.length() && (IsWhiteSpace(string[start]) || IsNewLine(string[start])))
			start++;

		// go until white space, newline, or end of string
		size_t end = start;
		while (end < string.length() && !IsWhiteSpace(string[end]) && !IsNewLine(string[end]))
			end++;

		// Get the word
		word = string.substr(start, end - start);

		// remove the word from the string
		if (end < string.length())
			string = std::string_view(&string[end]);
		else
			string = std::string_view("");

		// return whether or not we got anything
		return word.size() > 0;
	}
};