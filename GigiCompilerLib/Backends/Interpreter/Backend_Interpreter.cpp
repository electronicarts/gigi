///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Schemas/Types.h"
#include "GigiCompilerLib/Backends/Shared.h"
#include "GigiCompilerLib/Backends/GraphViz.h"
#include "GigiCompilerLib/GigiBuildFlavor.h"
#include <filesystem>

void PostLoad_Interpreter(RenderGraph& renderGraph)
{

}

struct BackendInterpreter : public BackendBase
{
};

extern void CopyShaderFileDX12(Shader& shader, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const char* outFolder, const RenderGraph& renderGraph);

void RunBackend_Interpreter(GigiBuildFlavor buildFlavor, RenderGraph& renderGraph, GGUserFileLatest& ggUserFile)
{
    const char* outFolder = renderGraph.outputDirectory.c_str();

    if (renderGraph.generateGraphVizFlag) {
        // make the graphviz
        std::string fullOutFolder = std::string(outFolder) + "/GraphViz/";
        MakeRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
        MakeFlattenedRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
        MakeSummaryRenderGraphGraphViz(renderGraph, fullOutFolder.c_str());
    }

    // gather the templates for the build flavor chosen.
    std::unordered_map<std::string, std::string> files;
    std::vector<InternalTemplateFile> internalTemplateFiles = ProcessTemplateFolder(renderGraph, files, outFolder, "./GigiCompilerLib/Backends/Interpreter/templates/");

    // Make the files
    std::unordered_map<std::string, std::ostringstream> stringReplacementMap = MakeStringReplacement<BackendInterpreter>(renderGraph, ggUserFile);
    MakeFiles(files, renderGraph, stringReplacementMap);

    // Make file copies for any InternalShader files from the templates
    for (const InternalTemplateFile& internalTemplateFile : internalTemplateFiles)
    {
        if (internalTemplateFile.type != BackendTemplateFileType::InternalShader)
            continue;

        FileCopy newFileCopy;
        newFileCopy.fileName = std::filesystem::weakly_canonical(internalTemplateFile.absoluteFileName).string();
        newFileCopy.type = FileCopyType::Shader;
        newFileCopy.destFileName = std::filesystem::weakly_canonical(internalTemplateFile.relativeFileName).string();

        renderGraph.fileCopies.push_back(newFileCopy);
    }

    // copy any file copies that should happen (assets, and shader file headers)
    for (const FileCopy& fileCopy : renderGraph.fileCopies)
    {
        // load the file into memory
        std::vector<unsigned char> data;
        if (!LoadFile((std::filesystem::path(renderGraph.baseDirectory) / fileCopy.fileName).string(), data))
        {
            Assert(false, "Could not read file %s", fileCopy.fileName.c_str());
        }

        // get the folder to copy to
        const char* destFolder = "";
        switch (fileCopy.type)
        {
            case FileCopyType::Private:
            {
                destFolder = "private/";
                break;
            }
            case FileCopyType::Shader:
            {
                destFolder = "shaders/";
                break;
            }
            case FileCopyType::Asset:
            {
                destFolder = "assets/";
                break;
            }
            default:
            {
                Assert(false, "Unhandled file copy type");
                break;
            }
        }

        // Make the full file name
        char fullFileName[4096];
        std::string destFileName;
        if (fileCopy.destFileName.empty())
            destFileName = fileCopy.fileName;
        else
            destFileName = fileCopy.destFileName;
        sprintf_s(fullFileName, "%s/%s%s", outFolder, destFolder, destFileName.c_str());

        // Replace the strings and write the file out
        if (fileCopy.binary)
        {
            WriteFileIfDifferent(fullFileName, data);
        }
        else
        {
            data.push_back(0);
            std::string fileContents = (char*)data.data();
            ProcessStringReplacement(fileContents, stringReplacementMap, renderGraph);
            WriteFileIfDifferent(fullFileName, fileContents);
        }
    }

    // Copy the shader files
    for (Shader& shader : renderGraph.shaders)
    {
        if (shader.copyFile)
            CopyShaderFileDX12(shader, stringReplacementMap, outFolder, renderGraph);
    }
}
