///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Shared.h"
#include "GigiCompilerLib/ProcessSlang.h"

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::filesystem::path GetExePath()
{
	char exePath[_MAX_PATH + 1];
	GetModuleFileNameA(NULL, exePath, _MAX_PATH);
	std::filesystem::path ret(exePath);
	ret.replace_filename("");
	return ret;
}

bool ConvertShaderSourceCode(std::string& source, const char* fileName, const char* shaderModel, const char* stage, const char* entryPoint, const std::vector<std::string>& includeDirectories, ShaderLanguage sourceLanguage, ShaderLanguage destinationLanguage, std::string& errorMessage, const std::vector<ShaderDefine>& shaderDefines, const SlangOptions& slangOptions)
{
	// If no conversion needed, we are done
	if (sourceLanguage == destinationLanguage)
		return true;

	// If the source language is slang, slang can output the language specified
	if (sourceLanguage == ShaderLanguage::Slang)
		return ProcessWithSlang(source, fileName, destinationLanguage, stage, entryPoint, shaderModel, errorMessage, includeDirectories, shaderDefines, slangOptions);

	// If the source language is HLSL btu we want WGSL, have slang convert for us
	if (sourceLanguage == ShaderLanguage::HLSL && destinationLanguage == ShaderLanguage::WGSL)
		return ProcessWithSlang(source, fileName, destinationLanguage, stage, entryPoint, shaderModel, errorMessage, includeDirectories, shaderDefines, slangOptions);

	// Otherwise, conversion not currently implemented
	return false;
}

bool LoadAndPreprocessTextFile(const std::string& fileName, std::vector<char>& fileContents, const RenderGraph& renderGraph, std::vector<std::string>& embeddedFiles)
{
    if (!LoadFile(fileName, fileContents))
        return false;

    // Convert to a string for processing
    fileContents.push_back(0);
    std::string str = fileContents.data();

    // Gigi preprocessor - handle embeds
    while(1)
    {
        size_t tagBegin = StringFindCaseInsensitive(str, "/*$(Embed:", 0);
        if (tagBegin == std::string::npos)
            break;

        size_t tagEnd = StringFindCaseInsensitive(str, ")*/", tagBegin);
        if (tagBegin == std::string::npos)
            return false;

        // Make the filename
        size_t fileNameBegin = tagBegin + 10;
        size_t fileNameEnd = tagEnd;

        if (str[fileNameBegin] == '\"')
        {
            fileNameBegin++;
            fileNameEnd--;
        }

        std::string embededFileName = str.substr(fileNameBegin, fileNameEnd - fileNameBegin);
        embededFileName = (std::filesystem::path(fileName).remove_filename() / std::filesystem::path(embededFileName)).string();

        // Get the contents of the embeded file
        std::vector<char> embededFileContents;
        if (!LoadAndPreprocessTextFile(embededFileName, embededFileContents, renderGraph, embeddedFiles))
            return false;

        // remember that this shader is based on this embeded file name
        embeddedFiles.push_back(embededFileName);

        // erase the tag
        str.erase(tagBegin, tagEnd + 3 - tagBegin);

        // put the embeded file contents in
        str.insert(str.begin() + tagBegin, embededFileContents.begin(), embededFileContents.end());
    }

    // Gigi preprocessor - handle IFs
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

    // Convert back to a vector
    fileContents.assign(str.begin(), str.end());

    return true;
}
