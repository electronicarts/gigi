#///////////////////////////////////////////////////////////////////////////////
#//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
#//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
#///////////////////////////////////////////////////////////////////////////////

import os
import glob
import subprocess
import shutil

requiredTestPrefix = ""
#requiredTestPrefix = "Compute\\"
#requiredTestPrefix = "Raster\\NoVertex_NoIndex_NoInstance_NoImports"

unsupportedTests = [

    # Viewer only
    "Buffers\\MultipleUVMesh",
    "Data\\ply_cube_uv_struct",
    "Data\\ply_cube_binary_type",
    "Data\\binaryTexU8",
    "Data\\binaryTexF32",
    "Python\\GPUWrite",
    "Python\\profiling",
    "ShaderAssert\\assertsTest",
    "Textures\\Save",
    "Compute\\OutputToMultipleInputs",

    # Viewer Only - These make sure the viewer will make mips of imported textures when asked
    "Textures\\Mips_Imported_2D",
    "Textures\\Mips_Imported_2DArray",
    "Textures\\Mips_Imported_3D",
    "Textures\\Mips_Imported_Cube",
    "Textures\\Load_Tex2DArray",
    "Textures\\Load_Tex3D",
    "SubGraph\\SetVarNode",

    # Just need to make work
    "RayTrace\\simpleRTDynamic",

    # WebGPU only
    "Buffers\\buffertest_webgpu",
    "Textures\\Mips_VSPS_2D_WebGPU",
]

# ==================== GENERATE CODE FOR TECHNIQUES

UnitTests = []

print(".\\GigiCompiler.exe DX12_Application ./Techniques/UnitTests/UnitTests.gg _GeneratedCode/UnitTests/DX12/")
subprocess.run(".\\GigiCompiler.exe DX12_Application ./Techniques/UnitTests/UnitTests.gg _GeneratedCode/UnitTests/DX12/")
print("")

for fileName in glob.glob(os.getcwd() + "/Techniques/UnitTests/**/*.py", recursive = True):
    relFileName = os.path.relpath(fileName, os.getcwd() + "/Techniques/UnitTests/")
    relFileNameNoExtension = os.path.splitext(relFileName)[0]
    relFileName = relFileName + ".gg"

    fileNameNoExtension = os.path.splitext(fileName)[0]
    fileName = fileNameNoExtension + ".gg"

    if relFileNameNoExtension == "RunTests":
        continue

    # Unsupported unit tests
    if relFileNameNoExtension in unsupportedTests:
        continue

    if not relFileNameNoExtension.startswith(requiredTestPrefix):
        continue

    unitTest = {
        "module": "_GeneratedCode\\UnitTests\\DX12\\UnitTests\\" + relFileNameNoExtension,
        "relPath": relFileNameNoExtension,
        "moduleName" : relFileNameNoExtension.split(os.sep)[-1],
    }

    UnitTests.append(unitTest)

    outDirName = "_GeneratedCode/UnitTests/DX12/UnitTests/" + relFileNameNoExtension
    print(relFileNameNoExtension)
    print(".\\GigiCompiler.exe DX12_Module " + fileName + " " + outDirName)
    subprocess.run(".\\GigiCompiler.exe DX12_Module " + fileName + " " + outDirName, shell=True, check=True)
    print("")

def ReplaceSection(data, labelStart, labelEnd, newText):
    start = data.find(labelStart) + len(labelStart)
    end = data.find(labelEnd, start)
    newData = data[:start] + newText + data[end:]
    #print(start)
    #print(end)
    return newData

def AppendSection(data, labelStart, labelEnd, newText):
    start = data.find(labelStart) + len(labelStart)
    end = data.find(labelEnd, start)
    newData = data[:end] + newText + data[end:]
    #print(start)
    #print(end)
    return newData

# ==================== Modify main.cpp to get all unit tests in there

path = ".\\_GeneratedCode\\UnitTests\\DX12\\"

# Read the file data in
data = open(path + "main.cpp", "r").read()

# Make the list of techniques
techniqueList = []
for fileName in glob.glob(path + "UnitTests\\**\\readme.txt", recursive = True):
    fileName = '\\'.join(fileName.split('\\')[0:-1])
    dirName = os.path.relpath(fileName, path)
    techniqueName = os.path.basename(dirName)
    if "\\DXC" not in dirName:
        techniqueList.append([dirName, techniqueName])

# // Gigi Modification Begin - Includes And Context
newString1 = ""
newString2 = ""
newString3 = "\nbool g_doSubsetTest = false; // If true, it will only test the techniques set to true below. Good for testing single techniques.\n"
for technique in techniqueList:
    newString1 += "\n#include \"" + technique[0] + "\\public\\technique.h\""
    newString1 += "\n#include \"" + technique[0] + "\\public\\imgui.h\""
    newString1 += "\n#include \"" + technique[0] + "\\private\\technique.h\""
    newString2 += "\n" + technique[1] + "::Context* m_" + technique[1] + " = nullptr;"
    newString3 += "\nbool g_doTest_" + technique[1] + " = false;"
newString2 += "\n\n#include \"UnitTestLogic.h\""
data = ReplaceSection(data, "// Gigi Modification Begin - Includes And Context", "// Gigi Modification End", newString1 + "\n" + newString2 + "\n" + newString3 + "\n")

# // Gigi Modification Begin - Create Context
newString = ""
for technique in techniqueList:
    newString += "\n    if (!g_doSubsetTest || g_doTest_" + technique[1] + ")"
    newString += "\n    {"
    newString += "\n        " + technique[1] + "::Context::LogFn = &LogFunction;"
    newString += "\n        " + technique[1] + "::Context::PerfEventBeginFn = &PerfEventBeginFn;"
    newString += "\n        " + technique[1] + "::Context::PerfEventEndFn = &PerfEventEndFn;"
    newString += "\n        " + technique[1] + "::Context::s_techniqueLocation = L\".\\\\" + technique[0].replace("\\", "\\\\") + "\\\\\";"
    newString += "\n        m_" + technique[1] + " = " + technique[1] + "::CreateContext(g_pd3dDevice);"
    newString += "\n        if (!m_" + technique[1] + ")"
    newString += "\n        {"
    newString += "\n            printf(\"Could not create m_" + technique[1] + " context\");"
    newString += "\n            return 1;"
    newString += "\n        }"
    newString += "\n    }"
    newString += "\n"
data = ReplaceSection(data, "// Gigi Modification Begin - Create Context", "    // Gigi Modification End", newString + "\n")

# // Gigi Modification Begin - UI
newString = ""
for technique in techniqueList:
    newString += "\n        if (m_" + technique[1] + " && ImGui::CollapsingHeader(\"" + technique[1] + "\"))"
    newString += "\n            " + technique[1] + "::MakeUI(m_" + technique[1] + ", g_pd3dCommandQueue);"
data = ReplaceSection(data, "// Gigi Modification Begin - UI", "        // Gigi Modification End", newString + "\n")

# // Gigi Modification Begin - OnNewFrame and Execute
newString1 = ""
newString2 = ""
newString3 = ""
newString4 = ""
for technique in techniqueList:
    newString1 += "\n        if (m_" + technique[1] + ")"
    newString1 += "\n            " + technique[1] + "::OnNewFrame(NUM_FRAMES_IN_FLIGHT);"
    newString2 += "\n        if (m_" + technique[1] + ")"
    newString2 += "\n            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_" + technique[1] + ", UnitTestEvent::PreExecute);"
    newString3 += "\n        if (m_" + technique[1] + ")"
    newString3 += "\n            " + technique[1] + "::Execute(m_" + technique[1] + ", g_pd3dDevice, g_pd3dCommandList);"
    newString4 += "\n        if (m_" + technique[1] + ")"
    newString4 += "\n            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_" + technique[1] + ", UnitTestEvent::PostExecute);"
data = ReplaceSection(data, "// Gigi Modification Begin - OnNewFrame and Execute", "        // Gigi Modification End", newString1 + "\n" + newString2 + "\n" + newString3 + "\n"+ newString4 + "\n")

# // Gigi Modification Begin - Destroy Contexts
newString = ""
for technique in techniqueList:
    newString += "\n    if (m_" + technique[1] + ")"
    newString += "\n    {"
    newString += "\n        " + technique[1] + "::DestroyContext(m_" + technique[1] + ");"
    newString += "\n        m_" + technique[1] + " = nullptr;"
    newString += "\n    }"
data = ReplaceSection(data, "// Gigi Modification Begin - Destroy Contexts", "    // Gigi Modification End", newString + "\n")

# Write the file data out
with open(path + 'main.cpp', 'w') as f:
    f.write(data)

# ==================== Delete all extraneous DX12Utils and AgilitySDK folders. Only need one copy.

for dirName in glob.glob(os.getcwd() + "/_GeneratedCode/UnitTests/DX12/**/DX12Utils/", recursive = True):
    relDirName = os.path.relpath(dirName, os.getcwd() + "/_GeneratedCode/UnitTests/DX12/")
    if (relDirName != "DX12Utils"):
        shutil.rmtree(dirName)

for dirName in glob.glob(os.getcwd() + "/_GeneratedCode/UnitTests/DX12/**/AgilitySDK/", recursive = True):
    relDirName = os.path.relpath(dirName, os.getcwd() + "/_GeneratedCode/UnitTests/DX12/")
    if (relDirName != "AgilitySDK"):
        shutil.rmtree(dirName)

for dirName in glob.glob(os.getcwd() + "/_GeneratedCode/UnitTests/DX12/**/DXC/", recursive = True):
    relDirName = os.path.relpath(dirName, os.getcwd() + "/_GeneratedCode/UnitTests/DX12/")
    if (relDirName != "DXC"):
        shutil.rmtree(dirName)

for dirName in glob.glob(os.getcwd() + "/_GeneratedCode/UnitTests/DX12/**/dxcompiler.dll", recursive = True):
    relDirName = os.path.relpath(dirName, os.getcwd() + "/_GeneratedCode/UnitTests/DX12/")
    if (relDirName != "dxcompiler.dll"):
        os.remove(dirName)

for dirName in glob.glob(os.getcwd() + "/_GeneratedCode/UnitTests/DX12/**/dxil.dll", recursive = True):
    relDirName = os.path.relpath(dirName, os.getcwd() + "/_GeneratedCode/UnitTests/DX12/")
    if (relDirName != "dxil.dll"):
        os.remove(dirName)

# ==================== Modify public\\technique.h files

for index, unitTest in enumerate(UnitTests):
    fileName = unitTest["module"] + "\\public\\technique.h"
    #print(fileName)
    #print(unitTest)
    #print(unitTest["relPath"].split("\\"))
    data = open(fileName, "r").read()

    data = data.replace("static const bool c_debugShaders = true;","static const bool c_debugShaders = false;")

    # Write the file data out
    with open(fileName, 'w') as f:
        f.write(data)