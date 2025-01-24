/*$(CopyrightHeader)*/// This file was created manually for DX12 unit tests, to write the logic for each unit test

#pragma once

#include <string>

namespace DX12Utils
{

namespace ParseCSV
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
		while (string[i] && !IsNewLine(string[i]))
			i++;

		// Skip forward until not a new line
		while (IsNewLine(string[i]))
			i++;

		return std::string_view(&string[i]);
	}

	inline bool GetNextToken(std::string_view& string, std::string_view& token)
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
		while (GetNextToken(csvData, token))
		{
			tokenS = token;
			if (!lambda(tokenIndex, tokenS.c_str()))
				return false;
			tokenIndex++;
		}

		return true;
	}
}

}; // namespace DX12Utils