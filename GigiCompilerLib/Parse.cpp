///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Parse.h"

bool isWhiteSpaceOrLF(const Char c) {
	// todo: refine
	return c != 0 && c <= ' ';
}


void parseWhiteSpaceOrLF(const Char* &p)
{
	while(isWhiteSpaceOrLF(*p))
		++p;
}

void parseWhiteSpaceNoLF(const Char* &p)
{
	for(;;++p)
	{
		Char c = *p;

		if (c == 0)
		{
			break;
		}
		
		if (!(c == ' ' || c == '\t'))
		{
			break;
		}
	}
}

bool parseStartsWith(const Char* &_p, const char* Token)
{
	const Char* p = _p;
	const Char* t = (const Char*)Token;

	while(*t)
	{
		if(*p != *t)
		{
			return false;
		}
		++p;
		++t;
	}

	_p = p;
	return true;
}

// @return true if a return was found
bool parseToEndOfLine(const Char* &p)
{
	while (*p)
	{
		if (*p == 13)		// CR
		{
			++p;

			if (*p == 10)	// CR+LF
				++p;

			return true;
		}
		if (*p == 10)		// LF
		{
			++p;
			return true;
		}
		++p;
	}

	return false;
}

bool parseLineFeed(const Char*& p) 
{
	if (*p == 13)		// CR
	{
		++p;

		if (*p == 10)	// CR+LF
			++p;
		return true;
	}
	if (*p == 10)		// LF
	{
		++p;
		return true;
	}
	return false;
}

bool parseLine(const Char*& p, std::string& Out)
{
	bool ret = false;
	Out.clear();

	const Char* start = p;
	const Char* end = p;

	// can be optimized, does a lot of resize
	for(;;)
	{
		if (*p == 0)
		{
			end = p;
			break;
		}

		if(*p == 13)		// CR
		{
			end = p;
			++p;

			if (*p == 10)	// CR+LF
				++p;

			break;
		}
		if(*p == 10)		// LF
		{
			end = p;
			++p;
			break;
		}

		p++;
	}
	Out = std::string((const char*)start, end - start);

	return !Out.empty();
}

// @param c charcter we look for
// @param str null terminated set of charcters, e.g. ";'?", must not be 0
// @return true if in set
bool isInSet(const Char c, const char *str)
{
	assert(str);

	const char* p = str;

	while(*p)
	{
		if (*p++ == c)
			return true;
	}

	return false;
}

bool parseUntil(const Char*& p, std::string& Out, const char* endCharacterSet)
{
	bool ret = false;
	Out.clear();

	const Char* start = p;
	const Char* end = p;

	// can be optimized, does a lot of resize
	while (*p)
	{
		if (isInSet(*p, endCharacterSet))
		{
			end = p;
			break;
		}

		p++;
	}
	Out = std::string((const char*)start, end - start);

	return !Out.empty();
}

// without digits
bool isNameCharacter(const Char Value)
{
	return (Value >= 'a' && Value <= 'z') || (Value >= 'A' && Value <= 'Z') || Value == '_';
}

bool isDigitCharacter(const Char Value)
{
	return Value >= '0' && Value <= '9';
}

int64_t stringToInt64(const char* str, int64_t fail)
{
	if(!str)
		return fail;

	const Char* p = (const Char*)str;

	int64_t ret = -1;

	parseInt64(p, ret);
	
	return ret;
}

bool parseInt(const Char*& p, int& outValue) 
{
	int64_t value64;
	bool ok = parseInt64(p, value64);
	if(ok)
		outValue = (int)value64;
	return ok;
}

bool parseInt64(const Char*& p, int64_t &outValue)
{
	const Char* Backup = p;
	bool bNegate = false;

	if (*p == '-')
	{
		bNegate = true;
		++p;
	}

	if (*p < '0' || *p > '9')
	{
		p = Backup;
		return false;
	}

	outValue = 0;

	while (*p >= '0' && *p <= '9')
	{
		outValue = outValue * 10 + (*p - '0');

		++p;
	}

	if (bNegate)
	{
		outValue = -outValue;
	}

	return true;
}

bool parseName(const Char* &p, std::string &Out)
{
	bool Ret = false;

	Out.clear();

	// can be optimized, does a lot of resize

	if (isNameCharacter(*p))
	{
		Out += *p++;
		Ret = true;
	}

	while(isNameCharacter(*p ) || isDigitCharacter(*p))
	{
		Out += *p++;
	}

	return Ret;
}

/*
bool parseDate(const Char*& p, tm& out) 
{
	const Char* backup = p;

	int mo, d, y, h, mi, s;

	// %m/%d/%Y %H:%M:%S
	parseWhiteSpaceNoLF(p);
	if(	!parseInt(p, mo) || 
		!parseStartsWith(p, "/") ||
		!parseInt(p, d) ||
		!parseStartsWith(p, "/") ||
		!parseInt(p, y) ||
		!parseStartsWith(p, " ") ||
		!parseInt(p, h) ||
		!parseStartsWith(p, ":") ||
		!parseInt(p, mi) ||
		!parseStartsWith(p, ":") ||
		!parseInt(p, s)
		)
	{
		p = backup;
		return false;
	}
	
	out = { 0 };

	out.tm_year = y - 1900;
	out.tm_mon = mo - 1;
	out.tm_mday = d;
	out.tm_hour = h - 1;
	out.tm_min = mi;
	out.tm_sec = s;

	return true;
}
*/

SPushStringA<260> parsePath(const Char* &p)
{
	SPushStringA<260> Ret;

	for (;;)
	{
		Char c = *p++;

		if ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			c == '\\' ||
			c == '/' ||
			c == ' ' ||
			c == '@' ||
			c == '.' ||
			c == '_')
		{
			Ret.push(c);
		}
		else
		{
			break;
		}
	}
	--p;

	return Ret;
}

bool computeLocationInFile(const Char* fileStart, const Char* where, uint32_t & outLine, uint32_t & outColumn, uint32_t tabSize)
{
	assert(fileStart);
	assert(where);

	const Char* p = fileStart;

	outLine = 1;
	outColumn = 1;

	if (where >= fileStart)
	for(;;)
	{
		if(p >= where)
		{
			return true;
		}

		if(parseLineFeed(p))
		{
			++outLine;
			outColumn = 1;
			continue;
		}

		if(*p=='\t')
		{
			++p;
			outColumn += tabSize;
			continue;
		}

		if(*p == 0)
			break;

		++p;
		++outColumn;
	};

	outLine = 0;
	outColumn = 0;
	return false;
}


// Portions of this software were based on https://www.techiedelight.com/implement-strstr-function-c-iterative-recursive/
// Function to implement `strstr()` function using KMP algorithm
// modified to stristr() with table to avoid costly localization functions
const char* strstrOptimized(const char* X, const char* Y, int m, int n)
{
	// base case 1: `Y` is NULL or empty
	if (*Y == '\0' || n == 0) {
		return X;
	}

	// base case 2: `X` is NULL, or X's length is less than Y's
	if (*X == '\0' || n > m) {
		return NULL;
	}

	// `next[i]` stores the index of the next best partial match
//	int next[n + 1];
	assert(n < 1024);
	if(n >= 1024)
		return 0;
	int next[1024];

	for (int i = 0; i < n + 1; i++) {
		next[i] = 0;
	}

	for (int i = 1; i < n; i++)
	{
		int j = next[i + 1];

		while (j > 0 && Y[j] != Y[i]) {
			j = next[j];
		}

		if (j > 0 || Y[j] == Y[i]) {
			next[i + 1] = j + 1;
		}
	}

	for (int i = 0, j = 0; i < m; i++)
	{
		if (*(X + i) == *(Y + j))
		{
			if (++j == n) {
				return (X + i - j + 1);
			}
		}
		else if (j > 0)
		{
			j = next[j];
			i--;    // since `i` will be incremented in the next iteration
		}
	}

	return NULL;
}

const char* stristrOptimized(const char* X, const char* Y, int m, int n)
{
	static char tab[256];
	if (!tab[1])
	{
		// build table to avoid costly localization functions
		for (int i = 1; i < 256; ++i)
			tab[i] = toupper(i);
	}

	// base case 1: `Y` is NULL or empty
	if (*Y == '\0' || n == 0) {
		return X;
	}

	// base case 2: `X` is NULL, or X's length is less than Y's
	if (*X == '\0' || n > m) {
		return NULL;
	}

	// `next[i]` stores the index of the next best partial match
//	int next[n + 1];
	assert(n < 1024);
	if (n >= 1024)
		return 0;
	int next[1024];

	for (int i = 0; i < n + 1; i++) {
		next[i] = 0;
	}

	for (int i = 1; i < n; i++)
	{
		int j = next[i + 1];

		while (j > 0 && tab[Y[j]] != tab[Y[i]]) {
			j = next[j];
		}

		if (j > 0 || tab[Y[j]] == tab[Y[i]]) {
			next[i + 1] = j + 1;
		}
	}

	for (int i = 0, j = 0; i < m; i++)
	{
		if (tab[*(X + i)] == tab[*(Y + j)])
		{
			if (++j == n) {
				return (X + i - j + 1);
			}
		}
		else if (j > 0)
		{
			j = next[j];
			i--;    // since `i` will be incremented in the next iteration
		}
	}

	return NULL;
}


