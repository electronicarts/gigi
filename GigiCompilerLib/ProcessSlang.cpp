///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ProcessSlang.h"

#include "external/slang/include/slang.h"
#include "external/slang/include/slang-com-helper.h"

#include <filesystem>
#include <vector>
#include <unordered_map>
#include "Backends/Shared.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// A probably incorrectly implemented include handler for slang.
// I wish there was an example of how to make one, or some better documentation.
// spAddSearchPath looks to do the trick though.
#if 0
class SlangFileContents : public ISlangBlob
{
public:
    // ISlangUnknown
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (auto intf = getInterface(uuid))
        {
            *outObject = intf;
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }

    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE
    {
        return m_data.data();
    }

    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE
    {
        return m_data.size();
    }

    ISlangUnknown* getInterface(const SlangUUID& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() ||
            guid == ISlangBlob::getTypeGuid())
        {
            return static_cast<ISlangBlob*>(this);
        }
        return nullptr;
    }

    std::vector<unsigned char> m_data;
};

class SlangIncludeHandler : public ISlangFileSystem
{
public:

    SlangIncludeHandler()
    {
    }

    void SetWorkingDirectory(const char* workingDirectory)
    {
        m_workingDirectory = workingDirectory;
    }

    void Release()
    {
        for (auto it : m_files)
            delete it.second;
    }

    // ISlangUnknown
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (auto intf = getInterface(uuid))
        {
            *outObject = intf;
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        if (auto intf = getInterface(guid))
        {
            return intf;
        }
        return getObject(guid);
    }

    void* getObject(const SlangUUID& guid)
    {
        SLANG_UNUSED(guid);
        return nullptr;
    }

    ISlangUnknown* getInterface(const SlangUUID& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() ||
            guid == ISlangFileSystem::getTypeGuid())
        {
            return static_cast<ISlangFileSystem*>(this);
        }
        if (guid == ISlangCastable::getTypeGuid())
        {
            return static_cast<ISlangCastable*>(this);
        }

        return nullptr;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
        char const* path,
        ISlangBlob** outBlob) override final
    {
        std::string fullFileName = std::filesystem::weakly_canonical(std::filesystem::path(m_workingDirectory) / path).string();

        auto it = m_files.find(fullFileName);
        if (it != m_files.end())
        {
            *outBlob = it->second;
            return SLANG_OK;
        }

        SlangFileContents* newContents = new SlangFileContents;

        if (!LoadFile(fullFileName, newContents->m_data))
        {
            delete newContents;
            return SLANG_FAIL;
        }

        m_files[fullFileName] = newContents;

        *outBlob = newContents;
        return SLANG_OK;
    }

    const char* m_workingDirectory = nullptr;

    std::unordered_map<std::string, SlangFileContents*> m_files;
};
#endif

// https://github.com/shader-slang/slang/tree/master/docs#readme
bool ProcessWithSlang(std::string& source, const char* fileName, ShaderLanguage destinationLanguage, const char* stage, const char* entryPoint, const char* profile, std::string& errorMessage, const std::vector<std::string>& includeDirectories, const SlangOptions& options)
{
    SlangCompileTarget targetLanguage = SlangCompileTarget::SLANG_HLSL;
    switch (destinationLanguage)
    {
        case ShaderLanguage::HLSL: targetLanguage = SlangCompileTarget::SLANG_HLSL; break;
        case ShaderLanguage::WGSL: targetLanguage = SlangCompileTarget::SLANG_WGSL; break;
        default:
        {
            errorMessage = std::string("ProcessWithSlang() is unable to output shader language \"") + EnumToString(destinationLanguage) + "\"";
            return false;
        }
    }

    bool ret = true;
    char errorBuffer[1024];

    // Create a session and request
    SlangSession* session = spCreateSession(NULL);
    SlangCompileRequest* request = spCreateCompileRequest(session);

    if (options.noNameMangling)
        request->setCompileFlags(SLANG_COMPILE_FLAG_NO_MANGLING);

    switch (options.optimizationLevel)
    {
        case GigiSlangOptimizationLevel::None: request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_NONE); break;
        case GigiSlangOptimizationLevel::Default: request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_DEFAULT); break;
        case GigiSlangOptimizationLevel::High: request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_HIGH); break;
        case GigiSlangOptimizationLevel::Maximum: request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL); break;
    }

    request->setLineDirectiveMode(options.lineDirectives ? SLANG_LINE_DIRECTIVE_MODE_STANDARD : SLANG_LINE_DIRECTIVE_MODE_NONE);

    {
        unsigned int diagnosticsFlags = 0;
        if (options.warningsAsErrors)
            diagnosticsFlags |= SLANG_DIAGNOSTIC_FLAG_TREAT_WARNINGS_AS_ERRORS;
        if (options.verbosePaths)
            diagnosticsFlags |= SLANG_DIAGNOSTIC_FLAG_VERBOSE_PATHS;
        request->setDiagnosticFlags(diagnosticsFlags);
    }

    // Set what type of thing we want to come out of the slang compiler
    spSetCodeGenTarget(request, targetLanguage);

    switch (options.floatingPointMode)
    {
        case GigiSlangFloatingPointMode::Default: request->setTargetFloatingPointMode(0, SLANG_FLOATING_POINT_MODE_DEFAULT); break;
        case GigiSlangFloatingPointMode::Fast: request->setTargetFloatingPointMode(0, SLANG_FLOATING_POINT_MODE_FAST); break;
        case GigiSlangFloatingPointMode::Precise: request->setTargetFloatingPointMode(0, SLANG_FLOATING_POINT_MODE_PRECISE); break;
    }

    for (const std::string& includeDirectory : includeDirectories)
        spAddSearchPath(request, includeDirectory.c_str());

    int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "");

    // set the source code
    spAddTranslationUnitSourceString(request, translationUnitIndex, fileName, source.data());

    if (profile)
        spSetTargetProfile(request, 0, spFindProfile(session, profile));

    SlangStage stageEnum = SlangStage::SLANG_STAGE_NONE;
    if (!_stricmp(stage, "fragment"))
        stageEnum = SlangStage::SLANG_STAGE_FRAGMENT;
    else if (!_stricmp(stage, "vertex"))
        stageEnum = SlangStage::SLANG_STAGE_VERTEX;
    else if (!_stricmp(stage, "compute"))
        stageEnum = SlangStage::SLANG_STAGE_COMPUTE;

    // Add an entry point
    int entryPointIndex = spAddEntryPoint(
        request,
        translationUnitIndex,
        entryPoint,
        stageEnum);

    int anyErrors = spCompile(request);

    if (anyErrors != 0)
    {
        sprintf_s(errorBuffer, "spCompile: ERROR %i\n", anyErrors);
        errorMessage += errorBuffer;
        ret = false;
    }

    // Output diagnostics if there were problems
    char const* diagnostics = spGetDiagnosticOutput(request);
    if (diagnostics)
        errorMessage += diagnostics;

    // set the compiled output
    if (ret)
    {
        size_t dataSize = 0;
        source = (const char*)spGetEntryPointCode(request, entryPointIndex, &dataSize);
    }

    // Clean up
    spDestroyCompileRequest(request);
    spDestroySession(session);

    if (!errorMessage.empty())
        errorMessage = std::string("[Slang] ") + errorMessage;

    return ret;
}
