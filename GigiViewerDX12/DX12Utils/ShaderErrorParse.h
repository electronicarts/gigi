///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////
#include <assert.h>

static const char* g_testInput =
"\n"
"## 1. sourceFile0 DXC&VisualStudio jumpsToSourceFileLine\n"
"hlsl.hlsl:15:2: error: use of undeclared identifier 'numthreads'\n"
"[numthreads(8, 8, 1)]\n"
" ^\n"
"In file included from hlsl.hlsl:10:\n"
"## 2. includeFile DXC&VisualStudio jumpsToSourceFileLine\n"
"./../../include/s2h.hlsl:1169:1: error: HLSL requires a type specifier for all declarations\n"
"\n"
"## 3. sourceFile1 DXC&VisualStudio jumpsToSourceFileLine\n"
"D:/dev/github/ShaderToHuman/examples/HelloWorld/HelloWorld.hlsl(10,1): error: unknown type name 'ShaderError01'\n"
"\n"
"## 4. sourceFile FXC&VisualStudio jumpsToSourceFile\n"
//"@" // debug marker
"C:\\Users\\mmittring\\AppData\\Local\\Temp\\2\\Gigi\\55064\\shaders\\HelloWorld.hlsl(10,1-13): error X3000: unrecognized identifier 'ShaderError01'\n"
"\n"
"## 5. includeFile FXC&VisualStudio jumpsToSourceFile\n"
"../../include/s2h.hlsl(26,1-13): error X3000: unrecognized identifier 'ShaderError02'\n"
"\n"
"## 6. sourceFile FXC in temp jumpsToTempFile (no processing)\n"
"C:\\Users\\mmittring\\AppData\\Local\\Temp\\2\\Gigi\\52680\\shaders\\HelloWorld.hlsl(27,1-14):\n";

static const char* g_testReferenceOutput =
"\n"
"## 1. sourceFile0 DXC&VisualStudio jumpsToSourceFileLine\n"
"D:/dev/github/ShaderToHuman/examples/HelloWorld/HelloWorld.hlsl(15,2): error: use of undeclared identifier 'numthreads'\n"
"[numthreads(8, 8, 1)]\n"
" ^\n"
"In file included from D:/dev/github/ShaderToHuman/examples/HelloWorld/HelloWorld.hlsl(10):\n"
"## 2. includeFile DXC&VisualStudio jumpsToSourceFileLine\n"
"D:/dev/github/ShaderToHuman/examples/HelloWorld/../../include/s2h.hlsl(1169,1): error: HLSL requires a type specifier for all declarations\n"
"\n"
"## 3. sourceFile1 DXC&VisualStudio jumpsToSourceFileLine\n"
"D:/dev/github/ShaderToHuman/examples/HelloWorld/HelloWorld.hlsl(10,1): error: unknown type name 'ShaderError01'\n"
"\n"
"## 4. sourceFile FXC&VisualStudio jumpsToSourceFile\n"
"C:\\Users\\mmittring\\AppData\\Local\\Temp\\2\\Gigi\\55064\\shaders\\HelloWorld.hlsl(10,1): error X3000: unrecognized identifier 'ShaderError01'\n"
"\n"
"## 5. includeFile FXC&VisualStudio jumpsToSourceFile\n"
"D:/dev/github/ShaderToHuman/examples/HelloWorld/../../include/s2h.hlsl(26,1): error X3000: unrecognized identifier 'ShaderError02'\n"
"\n"
"## 6. sourceFile FXC in temp jumpsToTempFile (no processing)\n"
"C:\\Users\\mmittring\\AppData\\Local\\Temp\\2\\Gigi\\52680\\shaders\\HelloWorld.hlsl(27,1):\n";

typedef unsigned char Char;

inline bool isDigitCharacter(const Char Value)
{
    return Value >= '0' && Value <= '9';
}

inline bool isFilePathCharacter(const Char Value)
{
    return Value != '<'
        && Value != '>'
        && Value != '|'
        && Value != '?'
        && Value != '*'
        && Value >= ' '
        && Value != 0;
}

inline bool parseStartsWith(const Char*& _p, const char* Token)
{
    const Char* p = _p;
    const Char* t = (const Char*)Token;

    while (*t)
    {
        if (*p != *t)
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
inline bool parseToEndOfLine(const Char*& p)
{
    while (*p)
    {
        if (*p == 13) // CR
        {
            ++p;

            if (*p == 10) // CR+LF
                ++p;

            return true;
        }
        if (*p == 10) // LF
        {
            ++p;
            return true;
        }
        ++p;
    }

    return false;
}

/*
// e.g. C:\Users\mmittring\AppData\Local\Temp\2\Gigi\55064\shaders\HelloWorld.hlsl(10, 1 - 13) : error X3000 : unrecognized identifier 'ShaderError01'
// should parse over all until '('
static bool parseFileNameError(const Char*& p)
{
    const Char* backup = p;

    while (*p != 0 && *p != 10 && *p != 13 && *p != '(' && *p != ')' && *p != ':' && *p != '[' && *p != ']')
        ++p;

    if (*p == '(')
    {
        ++p;
        while (isDigitCharacter(*p))
            ++p;
        if(*p == ')')
            return true;
        if (*p == ',')
        {
            ++p;
            while (*p == ' ')
                ++p;
            while (isDigitCharacter(*p))
                ++p;
            while (*p == ' ')
                ++p;
            if (*p == '-')
            {
                ++p;
                while (isDigitCharacter(*p))
                    ++p;
            }
            if (*p == ')')
                return true;
        }
    }

    p = backup;
    return false;
}

// @return 0:end of file, 1:"./", 2:"hlsl.hlsl", 3:fileName
static int parseNextEntryType(const Char*& p, const char* fileName)
{
    while (*p)
    {
        const Char* backup = p;

        if (parseStartsWith(p, "./"))
        {
            p = backup;
            return 1;
        }
        else if (parseStartsWith(p, "hlsl.hlsl"))
        {
            p = backup;
            return 2;
        }
/ *        else if (parseStartsWith(p, fileName))
        {
            p = backup;
            return 3;
        }
* /        else if (parseFileNameError(p))
        {
            p = backup;
            return 4;
        }


        parseToEndOfLine(p);
    }

    return 0;
}
*/

inline bool parseNumber(const Char*& p, std::string& temp)
{
    if (!isDigitCharacter(*p))
        return false;
    while (isDigitCharacter(*p))
        temp += *p++;
    return true;
}

// windows specific
inline bool isAbsolutePath(const std::string& value)
{
    return std::find(value.begin(), value.end(), ':') != value.end();
}

// fix all
//		hlsl.hlsl:<LINE>:<COLUMN>:
// and
//		./some_include:<LINE>:<COLUMN>:
// to
//      pathFileName(Line,COLUMN):
//
// see https://github.com/microsoft/DirectXShaderCompiler/issues/5096
// @param fileName temp file name e.g. "C:\Users\someUser\AppData\Local\Temp\1\Gigi\8936\shaders\FinalRenderCS.hlsl"
// @param sourceFileName e.g. "HelloWorld.hlsl"
inline std::string fixupHLSLErrors(const Char* input, const char* fileName, const std::string absPath, const std::string sourceFileName)
{
    assert(fileName);
    assert(*fileName);

    assert(!absPath.empty());
    assert(absPath.empty() || absPath.back() != '/');
    assert(strstr((const char*)absPath.c_str(), "\\") == 0);

    assert(!sourceFileName.empty());
    assert(sourceFileName.empty() || absPath.back() != '/');
    assert(strstr((const char*)sourceFileName.c_str(), "\\") == 0);

    // e.g. "C:\Users\someUser\AppData\Local\Temp\1\Gigi\8936\shaders"
    const std::string filePathOnly = std::filesystem::path(fileName).parent_path().string();

//    std::wstring fullFileName = std::filesystem::path(absPath) / sourceFileName;
    //    m_includeFiles.push_back(FromWideString(fullFileName.c_str()));

         //    const std::string filePathOnly = FilePath(fileName).getParent();
    //    const std::string absPath = filePathOnly;    // todo

    //    const FilePath& absPath = getRootDirectory();

    std::string ret;

    // avoid memory allocations for better performance
    ret.reserve(strlen((const char*)input));

    const Char* p = input;
    const Char* pLast = p;

    // for every line
    while(*p)
    {
        if (*p == '@')
        {
            ++p;      // for debugging, put breakpoint here
        }

        // simple implementation but wasteful heap allocations, not static to allow for multi threading
        std::string temp;

        if (parseStartsWith(p, "In file included from hlsl.hlsl:"))
        {
            while (isDigitCharacter(*p)) // line
                temp += *p++;
            if (*p == ':')
            {
                ret += "In file included from ";
//                ret += "\n";                        // Visual Studio might need the file name to start with a new line
                ret += absPath;
                ret += '/';
                ret += sourceFileName;

                ret += '(';
                ret += temp;
                ret += ")";
                pLast = p;
                continue;
            }
        }

        while (isFilePathCharacter(*p) && *p != '(' && *p != ':')
            ++p;
        if (*p == '(')   // e.g. D:/dev/github/ShaderToHuman/examples/HelloWorld/HelloWorld.hlsl(10,1):
        {
            const Char* endFilePath = p;
            ++p;

            if (!parseNumber(p, temp))
                continue;

            if (*p == ',') 
            {
                ++p;
                temp += ',';

                if (!parseNumber(p, temp))
                    continue;
            }
            if (*p == '-')  // e.g. FXC (10,1-13):
            {
                ++p;

                std::string dummy;
                if (!parseNumber(p, dummy))
                    continue;
            }

            if (*p == ')')  // e.g. D:/dev/github/ShaderToHuman/examples/HelloWorld/HelloWorld.hlsl(10,1):
            {
                ++p;
                if (*p == ':')  // e.g. D:/dev/github/ShaderToHuman/examples/HelloWorld/HelloWorld.hlsl(10,1):
                {
                    ++p;
                    std::string value((const char*)pLast, endFilePath - pLast);

                    if (!isAbsolutePath(value))
                    {
                        ret += absPath;
                        ret += '/';
                    }
                    // replace filename if needed
                    if (strncmp((const char*)pLast, "hlsl.hlsl", 9) == 0)
                        ret += sourceFileName;
                    else
                    {
                        ret += value;
                    }

                    ret += '(';
                    ret += temp;
                    ret += "):";
                    pLast = p;
                }
            }
        }
        else if (*p == ':')   // e.g. "hlsl.hlsl:15:2:" error: or "/../../include/s2h.hlsl:1169:1: error: "
        {
            const Char* endFilePath = p;
            ++p;

            if (!parseNumber(p, temp))  // line
                continue;

            if (*p == ':')
            {
                ++p;

                temp += ',';
                if (!parseNumber(p, temp))    // column
                    continue;

                if (*p != ':')
                    continue;
                ++p;

                // add filepath
                ret += absPath;
                ret += '/';
                // replace filename if needed
                if(strncmp((const char*)pLast, "hlsl.hlsl", 9) == 0)
                    ret += sourceFileName;
                else
                {
                    parseStartsWith(pLast, "./"); // e.g. ./../../include/s2h.hlsl:1169:1: error: HLSL requires a type specifier for all declarations

                    ret += std::string((const char*)pLast, endFilePath - pLast);
                }
                ret += '(';
                ret += temp;
                ret += "):";
                pLast = p;
            }
        }

        parseToEndOfLine(p);

        size_t characatersInFront = p - pLast;

        if (characatersInFront)
        {
            ret.append((const char*)pLast, characatersInFront);
            //			n -= characatersInFront;
            pLast = p;
        }
/*
        if (entryType == 1) // "./"
        {
            ret += filePathOnly;
            ret += '/';

            // jump over "./"
            p += 2;
            while (*p && *p != ':')
            {
                ret += *p++;
            }
            assert(*p == ':'); // syntax error
            ++p;
            ret += '(';
        }
        else if (entryType == 2 || entryType == 3) // "hlsl.hlsl" or filename
        {
            // jump over "hlsl.hlsl" or temp filename
            while(*p != '(' && *p != ':' && *p != 10 && *p != 13 && *p != 0)
                ++p;
            ret += sourceFileName;
            if (*p == '(' || *p == ':')
                ++p;
            ret += '(';
        }
        else if (entryType == 4)
        {
            // copy potentially relative filename
            while (*p && *p != '(')
            {
                ret += *p++;
            }
            assert(*p == '('); // syntax error
            ++p;
            ret += '(';
        }
        else
            assert(0);

        // line number
        while (isDigitCharacter(*p))
        {
            ret += *p++;
        }

        if (*p == ')')
            ++p;
        else
        {
            // e.g. ./../../include/s2h.hlsl:1169:1: error: unknown type name 'ShaderError02'
            assert(*p == ',' || *p == ':'); // syntax error
            ++p;

            // column number (optional)
            if (isDigitCharacter(*p))
            {
                ret += ',';
                while (isDigitCharacter(*p))
                {
                    ret += *p++;
                }

                // range of columns ? e.g. FXC (10,1-13):
                if (*p == '-')
                {
                    ++p;
                    while (isDigitCharacter(*p))
                        ++p;
                }
                if(*p == ')')
                    ++p;
                assert(*p == ':'); // syntax error
                ++p;
            }
        }

        ret += "):";

        pLast = p;
*/
    }

    // copy remaining characters
    ret += (const char*)pLast;

    return ret;
}

// @return success
static bool testFixupHLSLErrors()
{
    const bool showOutput = true;

    if (showOutput)
    {
        OutputDebugStringA("\n\nTest input: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        OutputDebugStringA(g_testInput);
        OutputDebugStringA(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");

        OutputDebugStringA("\n\nRef output: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        OutputDebugStringA(g_testReferenceOutput);
        OutputDebugStringA(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
    }

    std::string output = fixupHLSLErrors(
        (const Char *)g_testInput,
        "D:\\dev\\github\\ShaderToHuman\\examples\\HelloWorld\\HelloWorld.hlsl",
        "D:/dev/github/ShaderToHuman/examples/HelloWorld",
        "HelloWorld.hlsl");

    if (showOutput)
    {
        OutputDebugStringA("\n\nTest output: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        OutputDebugStringA(output.c_str());
        OutputDebugStringA(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
    }

    bool ret = strcmp(g_testReferenceOutput, output.c_str()) == 0;

    return ret;
}