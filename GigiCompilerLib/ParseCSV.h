///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ParseText.h"

namespace ParseCSV
{
	using namespace ParseText;

	inline bool GetNextCSVToken(std::string_view& string, std::string_view& token)
	{
		if (string.length() == 0)
			return false;

		// go until newline, comma, or null
		int tokenEnd = 0;
		while (tokenEnd < string.length() && string[tokenEnd] != ',' && !IsNewLine(string[tokenEnd]))
			tokenEnd++;

		// Get the token
		token = string.substr(0, tokenEnd);

		// If we hit a comma, skip it.  If we hit a newline, skip until a non newline.
		if (tokenEnd < string.length())
		{
			if (string[tokenEnd] == ',')
			{
				tokenEnd++;
			}
			else if (IsNewLine(string[tokenEnd]))
			{
				while (tokenEnd < string.length() && IsNewLine(string[tokenEnd]))
					tokenEnd++;
			}
		}

		// remove the token from the string
		string.remove_prefix(tokenEnd);
		
		// Remove any white space from the front of the token
		// Also remove a quote if present
		{
			int i = 0;
			while (i < token.length() && IsWhiteSpace(token[i]))
				i++;
			if (i < token.length() && IsQuote(token[i]))
				i++;
			token.remove_prefix(i);
		}

		// Remove any white space from the back of the token
		// Also remove a quote if present
		{
			int i = (int)token.length() - 1;
			while (i > 0 && IsWhiteSpace(token[i]))
				i--;

			if (i > 0 && IsQuote(token[i]))
				i--;

			token.remove_suffix(token.length() - 1 - i);
		}

		return true;
	}

	template <typename LAMBDA>
	bool ForEachValue(const char* csv, bool skipHeader, const LAMBDA& lambda)
	{
		if (!csv)
			return true;

		std::string_view csvData = csv;
		if (skipHeader)
			csvData = SkipToNewLine(csvData);

		std::string_view token;
		std::string tokenS;
		int tokenIndex = 0;
		while (GetNextCSVToken(csvData, token))
		{
			tokenS = token;
			if (!lambda(tokenIndex, tokenS.c_str()))
				return false;
			tokenIndex++;
		}

		return true;
	}
}