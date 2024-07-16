///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <assert.h>


// could changed for unicode
typedef unsigned char Char;

bool isWhiteSpaceOrLF(const Char c);
bool isNameCharacter(const Char Value);
bool isDigitCharacter(const Char Value);

bool parseLineFeed(const Char*& p);

// tab, space, LF/CR
void parseWhiteSpaceOrLF(const Char* &p);

void parseWhiteSpaceNoLF(const Char* &p);

bool parseStartsWith(const Char* &p, const char* Token);

// @return true if a return was found
bool parseToEndOfLine(const Char* &p);

// @return true if Out has content
bool parseLine(const Char*& p, std::string& Out);
// does not jump over character form endCharacterSet
// @param endCharacterSet e.g. "\r\n;,"
bool parseUntil(const Char*& p, std::string& Out, const char* endCharacterSet);

// uses C++ (variable) name definition
// @return true if Out has content
bool parseName(const Char* &p, std::string &Out);

// outValue is not changed if return is false
bool parseInt64(const Char*& p, int64_t &outValue);
// outValue is not changed if return is false
bool parseInt(const Char*& p, int& outValue);
// @param str if 0 fail is used, no whitespace handling before, string parsing ends with end of integer
int64_t stringToInt64(const char* str, int64_t fail = -1);

// useful for error / warnings, assumes fileStart points to 0 terminated string and p is inside this string
// @param fileStart must not be 0
// @param where must not be 0
// @param outLine starts with 1
// @param outColumn starts with 1
// @return success (can fail if p is not in fileStart string before null termination, if false outLine and outColumn are 0
bool computeLocationInFile(const Char* fileStart, const Char* where, uint32_t& outLine, uint32_t& outColumn, uint32_t tabSize = 4);

// using Using KMP Algorithm (Efficient), case sensitive
const char* strstrOptimized(const char* X, const char* Y, int m, int n);
// using Using KMP Algorithm (Efficient), case insensitive
const char* stristrOptimized(const char* X, const char* Y, int m, int n);


// parse date in format %m/%d/%Y %H:%M:%S
// @return success
//bool parseDate(const Char*& p, tm &out);

template <int TSize>
struct SPushStringA
{
	// constructor
	SPushStringA() : End(dat)
	{
		*dat = 0;
	}

	// WhiteSpace are all characters <=' '
	void trimWhiteSpaceFromRight()
	{
		--End;

		while (End >= dat)
		{
			if (*End > ' ')
			{
				break;
			}

			--End;
		}

		++End;
		*End = 0;
	}

	size_t size() const
	{
		return End - dat;
	}

	char &operator[](const size_t index)
	{
		assert(index < size());
		return dat[index];
	}

	const char *c_str() const
	{
		return dat;
	}

	// @return success
	bool push(const char c)
	{
		assert(c != 0);

		if (End >= dat + TSize - 1)
		{
			assert(0);					// intended?
			return false;
		}

		*End++ = c;
		*End = 0;

		return true;
	}

	bool empty() const
	{
		return End == dat;
	}

	void pop()
	{
		assert(!empty());

		--End;
		*End = 0;
	}

	void clear()
	{
		End = dat;
		*dat = 0;
	}

private: // -----------------------------------------

	char			dat[TSize];
	// points to /0 in dat[], is always 0
	char *			End;
};


// useful for parsing HLSL to extract #include
// MAX_PATH is 260
SPushStringA<260> parsePath(const Char* &p);
