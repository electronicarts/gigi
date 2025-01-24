///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include <string>
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include "Nodes/nodes.h"
#include "GigiAssert.h"
#include "external/timyxml2/tinyxml2.h"
#include "Schemas/JSON.h"
// clang-format on

#define ALIGN(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

struct BackendBase
{
    static void MakeStringReplacementGlobal(std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const RenderGraph& renderGraph)
    {
        int numActionNodes = 0;
        int numResourceNodes = 0;
        for (const RenderGraphNode& node : renderGraph.nodes)
        {
            if (GetNodeIsResourceNode(node))
                numResourceNodes++;
            else
                numActionNodes++;
        }

        stringReplacementMap["/*$(Platform)*/"] << EnumToString(renderGraph.backend);

        stringReplacementMap["/*$(Name)*/"] << renderGraph.name;
        stringReplacementMap["/*$(NumNodes)*/"] << renderGraph.nodes.size();
        stringReplacementMap["/*$(NumActionNodes)*/"] << numActionNodes;
        stringReplacementMap["/*$(NumResourceNodes)*/"] << numResourceNodes;

        // constant variables
        for (const Variable& variable : renderGraph.variables)
        {
            if (!variable.Const)
                continue;

            std::string key = "/*$(Variable:" + variable.originalName + ")*/";
            stringReplacementMap[key] << variable.dflt;
        }
    }
};

std::filesystem::path GetExePath();

inline bool StringEndsWith(const char* haystack, const char* needle)
{
    int lenHaystack = (int)strlen(haystack);
    int lenNeedle = (int)strlen(needle);

    if (lenHaystack < lenNeedle)
        return false;

    return !strcmp(&haystack[lenHaystack - lenNeedle], needle);
}

inline void SkipNextNewline(const char*& ptr)
{
    if (ptr)
    {
        while (*ptr && ptr[0] != '\n')
            ptr++;

        if (*ptr)
            ptr++;
    }
}

inline bool SkipNewlinesWhiteSpaceAndComments(const char*& ptr)
{
    if (!ptr)
        return ptr;

    auto IsWhiteSpace = [](char c)
    {
        return c == ' ' || c == '\t';
    };

    auto IsNewLine = [](char c)
    {
        return c == '\r' || c == '\n';
    };

    while (*ptr)
    {
        bool adjustedPtr = false;

        // handle white space and newline
        while (*ptr && (IsWhiteSpace(*ptr) || IsNewLine(*ptr)))
        {
            adjustedPtr = true;
            ptr++;
        }

        // handle single line comment
        if (ptr[0] == '/' && ptr[1] == '/')
        {
            adjustedPtr = true;
            ptr += 2;
            SkipNextNewline(ptr);
        }

        // block comment
        if (ptr[0] == '/' && ptr[1] == '*')
        {
            adjustedPtr = true;
            ptr += 2;
            while (*ptr && ptr[0] != '*')
                ptr++;

            // parse error if we couldn't find the end of the block comment
            if (ptr[0] != '*' || ptr[1] != '/')
                return false;
            ptr += 2;
        }

        // if we didn't find a new line, a single line comment, or a block comment, we are done
        if (!adjustedPtr)
            break;
    }
    return true;
}

inline bool GetClosingXMLTag(const char*& ptr, std::string& tagName)
{
    if (!ptr)
        return false;

    std::string closingTag = std::string("</") + tagName + std::string(">");

    if (std::strncmp(closingTag.c_str(), ptr, closingTag.length()))
        return false;

    ptr += closingTag.length();

    return true;
}

inline bool GetOpeningXMLTag(const char*& ptr, std::string& tagName)
{
    if (!ptr)
        return false;

    if (ptr[0] != '<')
        return false;

    if (ptr[1] == '/')
        return false;

    ptr++;
    const char* found = strchr(ptr, '>');
    if (!found)
        return false;

    tagName = std::string_view(ptr, found - ptr);
    ptr = found + 1;

    return true;
}

inline bool SkipStringIfPresent(const char*& ptr, const char* string)
{
    size_t strlenPtr = strlen(ptr);
    size_t strlenString = strlen(string);

    if (strlenPtr < strlenString)
        return false;

    if (memcmp(ptr, string, strlenString) != 0)
        return false;

    ptr += strlenString;
    return true;
}

// Example:
// string = "/*$(_closesthit:ClosestHitFn)*/"
// token = "_closesthit"
// this function will return true because it found the token, and parameter will be "ClosestHitFn"
// This function returns false if the token is not found in the string, or if the token is malformed (E.g. no colon)
inline bool GetTokenParameter(const char* string, const char* token, std::string& parameter)
{
    const char* ptr = string;
    if (!SkipStringIfPresent(ptr, "/*$(") || !SkipStringIfPresent(ptr, token) || *ptr != ':')
        return false;
    ptr++;

    size_t strlenPtr = strlen(ptr);
    if (strlenPtr < 3)
        return false;

    parameter = std::string(ptr, &ptr[strlenPtr - 3]);
    return true;
}

template <typename LAMBDA>
inline void ForEachToken(const std::string& fileContents, const LAMBDA& lambda)
{
    size_t processed = 0;

    while (1)
    {
        // first look for a "/*$("
        size_t start = fileContents.find("/*$(", processed);
        if (start == std::string::npos)
            return;

        // then look for a ")*/" after it
        size_t end = fileContents.find(")*/", start + 4);
        if (end == std::string::npos)
            return;

        // call the lambda
        lambda(fileContents.substr(start, end - start + 3), &fileContents[0], &fileContents[start]);

        // move past this token
        processed = end + 3;
    }
}

inline size_t CountLineNumber(const char* start, const char* cursor)
{
    size_t ret = 1;
    while (*start && start < cursor)
    {
        if (*start == '\n')
            ret++;
        start++;
    }
    return ret;
}

inline size_t StringFindCaseInsensitive(const std::string& haystack, const std::string& needle, size_t offset)
{
    auto it = std::search(
        haystack.begin() + offset, haystack.end(),
        needle.begin(), needle.end(),
        [](char a, char b) { return std::toupper(a) == std::toupper(b); }
    );

    if (it == haystack.end())
        return std::string::npos;

    return it - haystack.begin();
}

inline void EnsureAllTokensEaten(const std::string& fileContents, std::unordered_map<std::string, std::ostringstream>& stringReplacementMap)
{
    // make sure all tokens appears in the map, even if there's nothing to replace them with.
    ForEachToken(fileContents,
        [&] (const std::string& token, const char* stringStart, const char* cursor)
        {
            stringReplacementMap[token] << "";
        }
    );
}

inline void StringReplaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

inline void AddTemplateFile(std::unordered_map<std::string, std::string>& files, const char* folderName, const char* fileName, const char* templateText)
{
    char fullFileName[4096];
    sprintf_s(fullFileName, "%s/%s", folderName, fileName);
    files[fullFileName] = templateText;
}

inline void RemoveTemplateFile(std::unordered_map<std::string, std::string>& files, const char* folderName, const char* fileName)
{
    char fullFileName[4096];
    sprintf_s(fullFileName, "%s/%s", folderName, fileName);
    files.erase(fullFileName);
}

inline bool FileExists(const std::string& fileName)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName.c_str(), "rb");
    if (!file)
        return false;
    fclose(file);
    return true;
}

inline bool LoadFile(const std::string& fileName, std::vector<unsigned char>& data)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName.c_str(), "rb");
    if (!file)
        return false;

    fseek(file, 0, SEEK_END);
    data.resize(ftell(file));
    fseek(file, 0, SEEK_SET);
    fread(data.data(), data.size(), 1, file);

    fclose(file);
    return true;
}

inline void WriteFileIfDifferent(const std::string& fileName, const std::string& contents)
{
    // make sure the directory exists
    size_t lastindex = fileName.find_last_of("/\\");
    std::string dirName = fileName.substr(0, lastindex);
    std::filesystem::create_directories(dirName);

    // If the file already exists and is the same, don't do anything
    FILE* file = nullptr;
    fopen_s(&file, fileName.c_str(), "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        std::vector<unsigned char> oldFileContents(ftell(file));
        fseek(file, 0, SEEK_SET);
        fread(oldFileContents.data(), oldFileContents.size(), 1, file);
        fclose(file);
        if (contents.size() == oldFileContents.size())
        {
            bool different = false;
            for (size_t i = 0; i < oldFileContents.size(); ++i)
            {
                if (contents[i] != oldFileContents[i])
                {
                    different = true;
                    break;
                }
            }
            if (!different)
                return;
        }
    }

    // write the file
    fopen_s(&file, fileName.c_str(), "wb");
    fwrite(contents.data(), contents.size(), 1, file);
    fclose(file);
}

inline void WriteFileIfDifferent(const std::string& fileName, const std::vector<unsigned char>& contents)
{
    // make sure the directory exists
    size_t lastindex = fileName.find_last_of("/\\");
    std::string dirName = fileName.substr(0, lastindex);
    std::filesystem::create_directories(dirName);

    // If the file already exists and is the same, don't do anything
    FILE* file = nullptr;
    fopen_s(&file, fileName.c_str(), "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        std::vector<unsigned char> oldFileContents(ftell(file));
        fseek(file, 0, SEEK_SET);
        fread(oldFileContents.data(), oldFileContents.size(), 1, file);
        fclose(file);
        if (contents.size() == oldFileContents.size())
        {
            bool different = false;
            for (size_t i = 0; i < oldFileContents.size(); ++i)
            {
                if (contents[i] != oldFileContents[i])
                {
                    different = true;
                    break;
                }
            }
            if (!different)
                return;
        }
    }

    // write the file
    fopen_s(&file, fileName.c_str(), "wb");
    fwrite(contents.data(), contents.size(), 1, file);
    fclose(file);
}

inline void ProcessStringReplacement(std::string& str, std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const RenderGraph& renderGraph)
{
    // Gigi preprocessor
    size_t offset = 0;
    while (1)
    {
        // Get the next conditional
        size_t conditionBeginStart = 0;
        size_t conditionBeginEnd = 0;
        std::string condition, value;
        {
            offset = StringFindCaseInsensitive(str, "/*$(if:", offset);
            if (offset == std::string::npos)
                break;
            conditionBeginStart = offset;
            offset += 7;

            size_t conditionIndex = offset;

            size_t valueIndex = StringFindCaseInsensitive(str, ":", conditionIndex);
            if (valueIndex == std::string::npos)
                break;
            condition = str.substr(conditionIndex, valueIndex - conditionIndex);
            valueIndex++;

            offset = StringFindCaseInsensitive(str, ")*/", valueIndex);
            if (offset == std::string::npos)
                break;
            value = str.substr(valueIndex, offset - valueIndex);
            offset += 3;
            conditionBeginEnd = offset;
        }

        // find the endif corresponding to this conditional
        size_t conditionEndStart = 0;
        size_t conditionEndEnd = 0;
        {
            offset = StringFindCaseInsensitive(str, "/*$(Endif)*/", offset);
            if (offset == std::string::npos)
                break;
            conditionEndStart = offset;
            offset += 12;
            conditionEndEnd = offset;
        }

        // Evaluate the condition
        bool conditionIsTrue = false;
        if (!_stricmp(condition.c_str(), "Platform"))
        {
            Backend backend;
            StringToEnum(value.c_str(), backend);
            conditionIsTrue = (backend == renderGraph.backend);
        }
        else if (!_stricmp(condition.c_str(), "PlatformNot"))
        {
            Backend backend;
            StringToEnum(value.c_str(), backend);
            conditionIsTrue = (backend != renderGraph.backend);
        }
        else if (!_stricmp(condition.c_str(), "DX12.AgilitySDKRequired"))
        {
            bool compareValue = false;
            if (!_stricmp(value.c_str(), "true"))
                compareValue = true;
            conditionIsTrue = (compareValue == renderGraph.settings.dx12.AgilitySDKRequired);
        }

        // if the condition is true, we want to remove the conditional statements
        if (conditionIsTrue)
        {
            str.erase(conditionEndStart, conditionEndEnd - conditionEndStart);
            str.erase(conditionBeginStart, conditionBeginEnd - conditionBeginStart);
        }
        // else we want to remove the conditional statements and everything in between them
        else
        {
            str.erase(conditionBeginStart, conditionEndEnd - conditionBeginStart);
        }

        // continue searching the string where we removed stuff
        offset = conditionBeginStart;
    }

    EnsureAllTokensEaten(str, stringReplacementMap);
    for (std::pair<const std::string, std::ostringstream>& replacement : stringReplacementMap)
        StringReplaceAll(str, replacement.first, replacement.second.str());
}

inline void ProcessStringReplacement(std::string& str, std::unordered_map<std::string, std::ostringstream>& stringReplacementMap1, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap2, const RenderGraph& renderGraph)
{
    ProcessStringReplacement(str, stringReplacementMap1, renderGraph);
    for (const std::pair<const std::string, std::ostringstream>& replacement : stringReplacementMap2)
        StringReplaceAll(str, replacement.first, replacement.second.str());
}

template <typename TBACKEND>
std::unordered_map<std::string, std::ostringstream> MakeFiles(std::unordered_map<std::string, std::string>& files, RenderGraph& renderGraph)
{
    // make the string replacement
    std::unordered_map<std::string, std::ostringstream> stringReplacementMap;
    for (const CustomGigiToken& token : renderGraph.customTokens)
        stringReplacementMap[std::string("/*$(") + token.key + std::string(")*/")] << token.value;
    stringReplacementMap["\r\n"] << "\n"; // normalize line endings
    TBACKEND::MakeStringReplacementGlobal(stringReplacementMap, renderGraph);

    // do string replacement
    for (std::pair<const std::string, std::string>& filePair : files)
        ProcessStringReplacement(filePair.second, stringReplacementMap, renderGraph);

    // write the files only if they are different.  This avoids unnecessary code rebuilds and such.
    for (std::pair<const std::string, std::string>& filePair : files)
        WriteFileIfDifferent(filePair.first, filePair.second);

    return stringReplacementMap;
}

inline int GetResourceNodeForPin(const RenderGraph& renderGraph, const RenderGraphNode& node, int pinIndex)
{
    InputNodeInfo connectionInfo = GetNodePinInputNodeInfo(node, pinIndex);

    while (connectionInfo.nodeIndex != -1 && !GetNodeIsResourceNode(renderGraph.nodes[connectionInfo.nodeIndex]))
        connectionInfo = GetNodePinInputNodeInfo(renderGraph.nodes[connectionInfo.nodeIndex], connectionInfo.pinIndex);

    return connectionInfo.nodeIndex;
}

inline int GetResourceNodeForPin(const RenderGraph& renderGraph, int nodeIndex, int pinIndex)
{
    return GetResourceNodeForPin(renderGraph, renderGraph.nodes[nodeIndex], pinIndex);
}

inline bool ShaderResourceTypeIsReadOnly(ShaderResourceAccessType access)
{
    switch (access)
    {
        case ShaderResourceAccessType::UAV: return false;
        case ShaderResourceAccessType::RTScene: return true;
        case ShaderResourceAccessType::SRV: return true;
        case ShaderResourceAccessType::CopySource: return true;
        case ShaderResourceAccessType::CopyDest: return false;
        case ShaderResourceAccessType::CBV: return true;
        case ShaderResourceAccessType::Indirect: return true;
        case ShaderResourceAccessType::VertexBuffer: return true;
        case ShaderResourceAccessType::RenderTarget: return false;
        case ShaderResourceAccessType::DepthTarget: return false;
        case ShaderResourceAccessType::Barrier: return false;
        case ShaderResourceAccessType::ShadingRate: return true;
    }

    Assert(false, "Unhandled ShaderResourceType: %i", access);
    return false;
}

inline bool AccessIsReadOnly(unsigned int accessedAs)
{
    for (unsigned int i = 0; i < (unsigned int)ShaderResourceAccessType::Count; ++i)
    {
        if ((accessedAs & (1 << i)) == 0)
            continue;

        if (!ShaderResourceTypeIsReadOnly((ShaderResourceAccessType)i))
            return false;
    }

    return true;
}

inline size_t DataFieldTypeToSize(DataFieldType type)
{
    switch(type)
    {
        case DataFieldType::Int: return 4;
        case DataFieldType::Int2: return 4 * 2;
        case DataFieldType::Int3: return 4 * 3;
        case DataFieldType::Int4: return 4 * 4;
        case DataFieldType::Uint: return 4;
        case DataFieldType::Uint2: return 4 * 2;
        case DataFieldType::Uint3: return 4 * 3;
        case DataFieldType::Uint4: return 4 * 4;
        case DataFieldType::Float: return 4;
        case DataFieldType::Float2: return 4 * 2;
        case DataFieldType::Float3: return 4 * 3;
        case DataFieldType::Float4: return 4 * 4;
        case DataFieldType::Bool: return 4;
        case DataFieldType::Float4x4: return 4 * 4 * 4;
        case DataFieldType::Uint_16: return 2;
        default:
        {
            Assert(false, "Unknown data field type: %i", type);
            return 0;
        }
    }
}

inline size_t DataFieldTypeComponentCount(DataFieldType type)
{
    switch(type)
    {
        case DataFieldType::Int: return 1;
        case DataFieldType::Int2: return 2;
        case DataFieldType::Int3: return 3;
        case DataFieldType::Int4: return 4;
        case DataFieldType::Uint: return 1;
        case DataFieldType::Uint2: return 2;
        case DataFieldType::Uint3: return 3;
        case DataFieldType::Uint4: return 4;
        case DataFieldType::Float: return 1;
        case DataFieldType::Float2: return 2;
        case DataFieldType::Float3: return 3;
        case DataFieldType::Float4: return 4;
        case DataFieldType::Bool: return 1;
        case DataFieldType::Float4x4: return 16;
        case DataFieldType::Uint_16: return 1;
        default:
        {
            Assert(false, "Unknown data field type: %i (%s)", type, EnumToString(type));
            return 0;
        }
    }
}

inline bool DataFieldTypeIsPOD(DataFieldType type)
{
    return DataFieldTypeComponentCount(type) == 1;
}

inline std::string GetNodeTypeString(const RenderGraphNode& node)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: return #_NAME;
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
    Assert(false, "Unhandled node type");
    return "";
}

inline bool StringBeginsWith(const char* hayStack, const char* needle)
{
    size_t hayStackLen = strlen(hayStack);
    size_t needleLen = strlen(needle);

    if (needleLen > hayStackLen)
        return false;

    return std::strncmp(hayStack, needle, needleLen) == 0;
}

template <typename TVARIABLETOSTRING>
inline std::string ConditionToString(const Condition& condition, const RenderGraph& renderGraph, const TVARIABLETOSTRING& VariableToStringFn)
{
    if (condition.alwaysFalse)
        return "false";

    std::string value1;
    if (condition.variable1Index != -1)
        value1 = VariableToStringFn(renderGraph.variables[condition.variable1Index], renderGraph);

    std::string value2;
    if (condition.variable2Index != -1)
        value2 = VariableToStringFn(renderGraph.variables[condition.variable2Index], renderGraph);
    else
    {
        value2 = condition.value2;

        if (condition.variable1Index != -1 && renderGraph.variables[condition.variable1Index].enumIndex != -1)
        {
            std::string enumValueScope = renderGraph.enums[renderGraph.variables[condition.variable1Index].enumIndex].name + "::";
            if (!StringBeginsWith(value2.c_str(), enumValueScope.c_str()))
                value2 = enumValueScope + value2;
        }
    }

    switch (condition.comparison)
    {
        case ConditionComparison::IsFalse: return "!" + value1;
        case ConditionComparison::IsTrue: return value1;
        case ConditionComparison::Equals: return value1 + " == " + value2;
        case ConditionComparison::NotEquals: return value1 + " != " + value2;
        case ConditionComparison::LT: return value1 + " < " + value2;
        case ConditionComparison::LTE: return value1 + " <= " + value2;
        case ConditionComparison::GT: return value1 + " > " + value2;
        case ConditionComparison::GTE: return value1 + " >= " + value2;
        default:
        {
            Assert(false, "Unhandled condition comparison");
        }
    }
    return "";
}

inline bool ResourceNodeIsUsed(const RenderGraphNode_ResourceBase& resource)
{
    return resource.finalState != ShaderResourceAccessType::Count;
}

inline bool ResourceNodeIsUsed(RenderGraphNode& resource)
{
    return GetResourceNodeFinalState(resource) != ShaderResourceAccessType::Count;
}

static std::string DataFieldTypeToShaderType(DataFieldType type)
{
    switch (type)
    {
        case DataFieldType::Int: return "int";
        case DataFieldType::Int2: return "int2";
        case DataFieldType::Int3: return "int3";
        case DataFieldType::Int4: return "int4";
        case DataFieldType::Uint: return "uint";
        case DataFieldType::Uint2: return "uint2";
        case DataFieldType::Uint3: return "uint3";
        case DataFieldType::Uint4: return "uint4";
        case DataFieldType::Float: return "float";
        case DataFieldType::Float2: return "float2";
        case DataFieldType::Float3: return "float3";
        case DataFieldType::Float4: return "float4";
        // bools are differently sized on cpu and gpu
        // There is also some oddities when using them in structured buffers and using 1 and 0
        // for true and false instead of 0xffffffff and 0. better to dodge the problem.
        case DataFieldType::Bool: return "uint";
        case DataFieldType::Float4x4: return "float4x4";
        case DataFieldType::Uint_16: return "uint";
        case DataFieldType::Count:
        {
            Assert(false, "Invalid data field type: Count");
            return __FUNCTION__ " invalid field type";
        }
        default:
        {
            Assert(false, "Unhandled data field type: %i", type);
            return __FUNCTION__ " unknown field type";
        }
    }
}

template <typename ENUM_A, typename ENUM_B>
inline bool EnumToEnum(ENUM_A a, ENUM_B& b)
{
    return StringToEnum(EnumToString(a), b);
}

inline int GetVariableIndex(const RenderGraph& renderGraph, const char* variableName)
{
    int index = -1;
    for (const Variable& variable : renderGraph.variables)
    {
        index++;
        if (!_stricmp(variable.name.c_str(), variableName))
            return index;
    }
    return -1;
}

inline void GetScopeFromVariable(const char* variableName, std::string& scope, std::string& name)
{
    scope = "";
    name = variableName;

    size_t dotPos = name.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        scope = name.substr(0, dotPos + 1);
        name = name.substr(dotPos + 1, std::string::npos);
    }
}

// Shaders reference the original name of variables, before subgraph flattening and similar.
// So, we need a special routine to find the variable mentioned.
// All other varibale references should be updated to the correct variables.
inline int GetScopedVariableIndex(const RenderGraph& renderGraph, const char* variableName)
{
    // extract the variable scope and name
    std::string searchScope;
    std::string searchVariableName;
    GetScopeFromVariable(variableName, searchScope, searchVariableName);

    // Go through variable replacements to find the variable this actually represents, before searching for it.
    // Do this in a loop, as there may be a chain of renames
    {
        int loopCount = 0;
        bool didRename = false;
        do
        {
            Assert(loopCount < 1000, "Variable rename loop detected while loop for variable \"%s\"", variableName);
            didRename = false;

            for (const VariableReplacement& replacement : renderGraph.variableReplacements)
            {
                if (_stricmp(replacement.srcScope.c_str(), searchScope.c_str()))
                    continue;

                if (!_stricmp(replacement.srcName.c_str(), searchVariableName.c_str()))
                {
                    searchScope = "";
                    searchVariableName = replacement.destName;
                    didRename = true;

                    // extract the variable scope and name
                    GetScopeFromVariable(searchVariableName.c_str(), searchScope, searchVariableName);

                    break;
                }
            }
            loopCount++;
        }
        while (didRename);
    }

    // Look through the list
    int index = -1;
    for (const Variable& variable : renderGraph.variables)
    {
        index++;

        if (_stricmp(variable.scope.c_str(), searchScope.c_str()))
            continue;

        if (!_stricmp(variable.originalName.c_str(), searchVariableName.c_str()))
            return index;
    }
    return -1;
}

enum class DataFieldComponentType
{
	_int,
	_uint16_t,
	_uint32_t,
	_float,
};

struct DataFieldTypeInfoStruct
{
	DataFieldTypeInfoStruct(DataFieldComponentType type, int bytes, int count, DataFieldType type2)
	{
		componentType = type;
        componentType2 = type2;
		componentBytes = bytes;
		componentCount = count;

		typeBytes = bytes * count;
	}

	DataFieldComponentType componentType = DataFieldComponentType::_int;
    DataFieldType componentType2 = DataFieldType::Int;
	int componentBytes = sizeof(int);
	int componentCount = 0;

    int typeBytes = 0;
};

#define DATA_FIELD_TYPE_INFO(type, count, type2) DataFieldTypeInfoStruct(DataFieldComponentType::_##type, (int)sizeof(type), count, type2)

inline DataFieldTypeInfoStruct DataFieldTypeInfo(DataFieldType type)
{
    switch(type)
    {
        case DataFieldType::Int: return DATA_FIELD_TYPE_INFO(int, 1, DataFieldType::Int);
        case DataFieldType::Int2: return DATA_FIELD_TYPE_INFO(int, 2, DataFieldType::Int);
        case DataFieldType::Int3: return DATA_FIELD_TYPE_INFO(int, 3, DataFieldType::Int);
        case DataFieldType::Int4: return DATA_FIELD_TYPE_INFO(int, 4, DataFieldType::Int);
        case DataFieldType::Uint: return DATA_FIELD_TYPE_INFO(uint32_t, 1, DataFieldType::Uint);
        case DataFieldType::Uint2: return DATA_FIELD_TYPE_INFO(uint32_t, 2, DataFieldType::Uint);
        case DataFieldType::Uint3: return DATA_FIELD_TYPE_INFO(uint32_t, 3, DataFieldType::Uint);
        case DataFieldType::Uint4: return DATA_FIELD_TYPE_INFO(uint32_t, 4, DataFieldType::Uint);
        case DataFieldType::Float: return DATA_FIELD_TYPE_INFO(float, 1, DataFieldType::Float);
        case DataFieldType::Float2: return DATA_FIELD_TYPE_INFO(float, 2, DataFieldType::Float);
        case DataFieldType::Float3: return DATA_FIELD_TYPE_INFO(float, 3, DataFieldType::Float);
        case DataFieldType::Float4: return DATA_FIELD_TYPE_INFO(float, 4, DataFieldType::Float);
        case DataFieldType::Bool: return DATA_FIELD_TYPE_INFO(uint32_t, 1, DataFieldType::Bool);
        case DataFieldType::Float4x4: return DATA_FIELD_TYPE_INFO(float, 16, DataFieldType::Float);
        case DataFieldType::Uint_16: return DATA_FIELD_TYPE_INFO(uint16_t, 1, DataFieldType::Uint_16);
        default:
        {
            Assert(false, "Unknown data field type: %i (%s)", type, EnumToString(type));
			return DATA_FIELD_TYPE_INFO(int, 0, DataFieldType::Int);
        }
    }
}

inline void ProcessTemplateFolder(RenderGraph& renderGraph, std::unordered_map<std::string, std::string>& files, const char* outFolder, const char* templateDir_)
{
    // todo: templatedir needs to be relative to exe, not working directory!
    std::filesystem::path templateDir = GetExePath() / templateDir_;

    // read the configuration
    BackendTemplateConfig& templateConfig = renderGraph.templateConfig;
    {
        std::string ggconfigFileName = (templateDir / "ggconfig.json").string();
        ReadFromJSONFile(templateConfig, ggconfigFileName.c_str(), false);
    }

    // process the rename files
    for (BackendTemplateFileProperties& fileProperties : templateConfig.fileProperties)
        StringReplaceAll(fileProperties.renameTo, "/*$(Name)*/", renderGraph.name);

    // Expand file properties that are for directories
    for (size_t filePropertyIndex = 0; filePropertyIndex < templateConfig.fileProperties.size(); ++filePropertyIndex)
    {
        BackendTemplateFileProperties& fileProperties = templateConfig.fileProperties[filePropertyIndex];
        if (!fileProperties.isDirectory)
            continue;

        std::filesystem::path p = std::filesystem::weakly_canonical(templateDir / fileProperties.fileName);

        for (const std::filesystem::directory_entry it : std::filesystem::recursive_directory_iterator(p))
        {
            if (!it.is_regular_file())
                continue;

            BackendTemplateFileProperties newProperties = templateConfig.fileProperties[filePropertyIndex];
            newProperties.isDirectory = false;
            newProperties.fileName = std::filesystem::proximate(it.path(), templateDir).string();
            templateConfig.fileProperties.push_back(newProperties);
        }
    }

    // Erase all the isDirectory entries now that we've acted on them
    templateConfig.fileProperties.erase(std::remove_if(templateConfig.fileProperties.begin(), templateConfig.fileProperties.end(),
        [](const BackendTemplateFileProperties& fileProperties)
        {
            return fileProperties.isDirectory;
        }),
        templateConfig.fileProperties.end()
    );

    // Process the template files
    for (const std::filesystem::directory_entry it : std::filesystem::recursive_directory_iterator(templateDir))
    {
        if (!it.is_regular_file())
            continue;

        // Ignore this file if the ggconfig file says to ignore it
        {
            bool ignoreFile = false;
            std::string absoluteFileName = std::filesystem::weakly_canonical(it.path()).string();
            for (const std::string& ignoreDir : templateConfig.ignoreDirectories)
            {
                // ignore the file if the canonical path begins with the canonical path of the ignoreDir
                std::string absoluteIgnoreDir = std::filesystem::weakly_canonical((templateDir / std::filesystem::path(ignoreDir))).string();
                if (!std::strncmp(absoluteIgnoreDir.c_str(), absoluteFileName.c_str(), absoluteIgnoreDir.length()))
                {
                    ignoreFile = true;
                    break;
                }
            }
            if (ignoreFile)
                continue;
        }

        // Get the relative file name
        std::string relativeFileName = std::filesystem::proximate(it.path(), templateDir).string();

        if (!_stricmp(relativeFileName.c_str(), "ggconfig.json"))
            continue;

        // Detect if the file is binary. Binary files don't go through string replacement etc.
        bool isBinary = false;
        {
            std::string extensionString = it.path().extension().string();
            const char* extension = extensionString.c_str();

            isBinary =
                !_stricmp(extension, ".dll") ||
                !_stricmp(extension, ".lib") ||
                !_stricmp(extension, ".pdb") ||
                !_stricmp(extension, ".exe") ||
                !_stricmp(extension, ".p7s") ||
                !_stricmp(extension, ".nupkg") ||
                !_stricmp(extension, ".png")
                ;
        }

        // Load the file data up
        std::vector<unsigned char> fileData;
        if (!LoadFile(it.path().string().c_str(), fileData))
        {
            Assert(false, "Could not load template file %s", it.path().string().c_str());
            return;
        }

        // Apply file properties from the template config
        bool skipFile = false;
        for (BackendTemplateFileProperties& fileProperties : templateConfig.fileProperties)
        {
            if (_stricmp(relativeFileName.c_str(), fileProperties.fileName.c_str()))
                continue;

            if (!fileProperties.renameTo.empty())
                relativeFileName = fileProperties.renameTo;

            if (fileProperties.onlyIncludeIfRaytracing && !renderGraph.usesRaytracing)
                skipFile = true;

            if (fileProperties.onlyIncludeIfDX12AgilitySDKRequired && !renderGraph.settings.dx12.AgilitySDKRequired)
                skipFile = true;

            break;
        }
        if (skipFile)
            continue;

        // Binary files get copied to the outFolder without modification
        if (isBinary)
        {
            char fullFileName[4096];
            sprintf_s(fullFileName, "%s/%s", outFolder, relativeFileName.c_str());
            WriteFileIfDifferent(fullFileName, fileData);
        }
        // Text files get a null terminator and go through the template system
        else
        {
            fileData.push_back(0);
            AddTemplateFile(files, outFolder, relativeFileName.c_str(), (char*)fileData.data());
        }
    }

    // Process nodeTemplateFiles
    for (const std::string& nodeTemplateFile : templateConfig.nodeTemplateFiles)
    {
        std::string fullFileName = (templateDir / std::filesystem::path(nodeTemplateFile)).string();

        tinyxml2::XMLDocument doc;
        doc.LoadFile(fullFileName.c_str());
        if (doc.Error())
        {
            Assert(false, "Could not load xml file \"%s\" : %s", fullFileName.c_str(), doc.ErrorStr());
            return;
        }

        auto rootElement = doc.RootElement();
        if (!rootElement)
        {
            Assert(false, "Could not load xml file \"%s\" : no root element", fullFileName.c_str());
            return;
        }

        while (rootElement)
        {
            // seperate the xml node's name into the node type and tag
            const char* elementName = rootElement->Name();
            const char* split = strchr(elementName, ':');
            if (!split)
            {
                Assert(false, "Could not process xml file \"%s\": xml tag <%s> should be of the format <nodeType:tag>.", fullFileName.c_str(), elementName);
                return;
            }

            // remove the first newline from the template text if present, for convinience.
            const char* elementText = rootElement->GetText();
            if (!elementText)
                elementText = "";
            if (elementText[0] == '\n')
                elementText++;
            else if (elementText[0] == '\r' && elementText[1] == '\n')
                elementText += 2;
            std::string elementTextStr = elementText;
            /*
            if (StringEndsWith(elementTextStr.c_str(), "\n"))
            {
                elementTextStr.pop_back();
            }
            else if (StringEndsWith(elementTextStr.c_str(), "\r\n"))
            {
                elementTextStr.pop_back();
                elementTextStr.pop_back();
            }
            */

            // Store this template text
            BackendTemplateNodeTemplate newTemplateEntry;
            newTemplateEntry.nodeType = std::string_view(elementName, split - elementName);
            newTemplateEntry.tag = std::string("/*$(") + &split[1] + std::string(")*/");
            newTemplateEntry.text = elementTextStr;
            templateConfig.nodeTemplates.push_back(newTemplateEntry);

            rootElement = rootElement->NextSiblingElement();
        }
    }
}

template <typename LAMBDA>
void DispatchLambdaActionInner(RenderGraphNode_ActionBase& node, const LAMBDA& lambda)
{
    lambda(node);
}

template <typename LAMBDA>
void DispatchLambdaActionInner(const RenderGraphNode_ActionBase& node, const LAMBDA& lambda)
{
    lambda(node);
}

template <typename LAMBDA>
void DispatchLambdaActionInner(RenderGraphNode_ResourceBase& node, const LAMBDA& lambda)
{
}

template <typename LAMBDA>
void DispatchLambdaActionInner(const RenderGraphNode_ResourceBase& node, const LAMBDA& lambda)
{
}

template <typename LAMBDA>
void DispatchLambdaAction(RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                DispatchLambdaActionInner(node.##_NAME, lambda); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

template <typename LAMBDA>
void DispatchLambdaAction(const RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                DispatchLambdaAction(node.##_NAME, lambda); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}
