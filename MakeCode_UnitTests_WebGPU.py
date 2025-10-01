#///////////////////////////////////////////////////////////////////////////////
#//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
#//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
#///////////////////////////////////////////////////////////////////////////////

import os
import glob
import subprocess
import shutil

requiredTestPrefix = ""
#requiredTestPrefix = "Compute\\"
#requiredTestPrefix = "Raster\\NoVertex_NoIndex_NoInstance_NoImports"

unsupportedTests = [

    # Just need to make these work. They want the generated code to be able to generate mips.
    "Textures\\Mips_Imported_2D",
    "Textures\\Mips_Imported_2DArray",
    "Textures\\Mips_Imported_3D",
    "Textures\\Mips_Imported_Cube",
    "Textures\\Mips_ShaderToken_2D",
    "Textures\\Mips_ShaderToken_2DArray",
    "Textures\\Mips_ShaderToken_3D",
    "Textures\\Mips_ShaderToken_Cube",

    # A bug in slang prevents this from working
    # https://github.com/shader-slang/slang/issues/6842
    "Textures\\Mips_RGS_2D",

    # This writes to a uav from a vertex shader, which is not allowed in webgpu.
    # This is replaced by Textures\\Mips_VSPS_2D_WebGPU.gg which doesn't
    "Textures\\Mips_VSPS_2D",

    # This uses raw byte buffers, which are not supported in webgpu
    # This is replaced by a Buffers\\buffertest_webgpu test which doesn't.
    "Buffers\\buffertest",

    # Variable rate shading is not supported in webgpu
    "Raster\\VRS",

    # Mesh shaders are not supported in webgpu
    "MeshShaders\\Mesh",
    "MeshShaders\\MeshAmplification",
    "MeshShaders\\MeshAmplificationLines",

    # Writing to cube maps from shaders is not supported in webgpu
    "Textures\\TextureCubeRW_CS",
    "Textures\\TextureCubeRW_PS",
    "Textures\\TextureCubeRW_RGS",
    "Textures\\Mips_CS_Cube",

    # This should work, but dawn (chrome) gives a failure
    # "depthSlice (4294967295) for a 3D attachment is undefined" 4/10/25
    "Textures\\Texture3DRW_PS",

    # Viewer only tests
    "Buffers\\MultipleUVMesh",
    "Buffers\\uint16",
    "Python\\profiling",
    "Python\\GPUWrite",
    "Data\\ply_cube_uv_struct",
    "Data\\ply_cube_binary_type",
    "Data\\binaryTexU8",
    "Data\\binaryTexF32",
    "ShaderAssert\\assertsTest",
    "Textures\\Load_Tex2DArray",
    "Textures\\Load_Tex3D",
    "Textures\\Save",
    "Compute\\OutputToMultipleInputs",
    "SubGraph\\SetVarNode",
    "Buffers\\bufferViewDescriptorTable",

    # Frostbite only
    "CopyResource\\CopyResourceTest_FB",

    # Need to implement buffer view begin / size
    "Buffers\\buffertest_viewOffsetCS",

    # Just need to make it work
    "CopyResource\\CopyPartialBuffer",
    "Raster\\simpleRasterMSAA",
    "Buffers\\Strides",
]

# ==================== GENERATE CODE FOR TECHNIQUES

UnitTests = []

for fileName in glob.glob(os.getcwd() + "/Techniques/UnitTests/**/*.py", recursive = True):
    relFileName = os.path.relpath(fileName, os.getcwd() + "/Techniques/UnitTests/")
    relFileNameNoExtension = os.path.splitext(relFileName)[0]
    relFileName = relFileName + ".gg"

    fileNameNoExtension = os.path.splitext(fileName)[0]
    fileName = fileNameNoExtension + ".gg"

    if relFileNameNoExtension == "RunTests":
        #print("Skipping: " + relFileNameNoExtension)
        continue

    # unsupported unit tests
    if relFileNameNoExtension in unsupportedTests:
        #print("Skipping: " + relFileNameNoExtension)
        continue

    if not relFileNameNoExtension.startswith(requiredTestPrefix):
        #print("Skipping: " + relFileNameNoExtension)
        continue

    unitTest = {
        "module": "_GeneratedCode\\UnitTests\\WebGPU\\UnitTests\\" + relFileNameNoExtension,
        "relPath": relFileNameNoExtension,
        "moduleName" : relFileNameNoExtension.split(os.sep)[-1],
    }

    UnitTests.append(unitTest)

    outDirName = "_GeneratedCode/UnitTests/WebGPU/UnitTests/" + relFileNameNoExtension
    print(relFileNameNoExtension)
    print(".\\GigiCompiler.exe WebGPU_Application " + fileName + " " + outDirName)
    returncode = subprocess.run(".\\GigiCompiler.exe WebGPU_Application " + fileName + " " + outDirName, shell=True, check=True).returncode
    print("returned " + str(returncode))

    print("")

# ==================== Delete Files

# The unit tests are not meant to be standalone things, either for web or node.
# So, remove the things that make them standalone, and set up the webgpu folder
# to be the main hub.

first = True
for unitTest in UnitTests:

    if first:
        shutil.copy(unitTest["module"] + "\\Node_GetPackages.bat", "_GeneratedCode\\UnitTests\\WebGPU\\")
        shutil.copy(unitTest["module"] + "\\Web_Open.bat", "_GeneratedCode\\UnitTests\\WebGPU\\")
        shutil.copy(unitTest["module"] + "\\Web_Start.bat", "_GeneratedCode\\UnitTests\\WebGPU\\")
        shutil.copy(unitTest["module"] + "\\.gitignore", "_GeneratedCode\\UnitTests\\WebGPU\\")
        shutil.copy(unitTest["module"] + "\\jquery-csv.js", "_GeneratedCode\\UnitTests\\WebGPU\\")
        shutil.copy(unitTest["module"] + "\\Shared.js", "_GeneratedCode\\UnitTests\\WebGPU\\")
        shutil.copy(unitTest["module"] + "\\style.css", "_GeneratedCode\\UnitTests\\WebGPU\\")
        shutil.copy(unitTest["module"] + "\\favicon.ico", "_GeneratedCode\\UnitTests\\WebGPU\\")
        first = False

    os.remove(unitTest["module"] + "\\Node_GetPackages.bat")
    os.remove(unitTest["module"] + "\\Node_Run.bat")
    os.remove(unitTest["module"] + "\\Web_Open.bat")
    os.remove(unitTest["module"] + "\\Web_Start.bat")
    os.remove(unitTest["module"] + "\\.gitignore")
    os.remove(unitTest["module"] + "\\Readme.txt")
    os.remove(unitTest["module"] + "\\Shared.js")
    os.remove(unitTest["module"] + "\\jquery-csv.js")
    os.remove(unitTest["module"] + "\\style.css")
    os.remove(unitTest["module"] + "\\favicon.ico")

# Download the packages
cwd = os.getcwd()
os.chdir("_GeneratedCode/UnitTests/WebGPU/")
print(".\\Node_GetPackages.bat")
returncode = subprocess.run(".\\Node_GetPackages.bat").returncode
print("returned " + str(returncode))
os.chdir(cwd)

# ==================== File modification helpers

def PrependText(data, label, newText):
    return data.replace(label, newText + label)

def AppendText(data, label, newText):
    return data.replace(label, label + newText)

def ReplaceTextSection(data, labelStart, labelEnd, newText):
    start = data.find(labelStart)
    end = data.find(labelEnd, start) + len(labelEnd)
    newData = data[:start] + newText + data[end:]
    #print(start)
    #print(end)
    return newData

# ==================== Modify index.js files

indexHTML = open("_GeneratedCode\\UnitTests\\WebGPU\\index.html", "w")
indexHTML.write("<html>\n<head><title>Gigi WebGPU Unit Tests</title></head>\n<body>\n<h1>Gigi WebGPU Unit Tests</h1>\n\n")

for unitTest in UnitTests:
    fileName = unitTest["module"] + "\\index.js"
    #print(fileName)
    #print(unitTest)
    #print(unitTest["relPath"].split("\\"))
    data = open(fileName, "r").read()

    data = data.replace("'./Shared.js'","'../../../Shared.js'")
    data = data.replace("'./jquery-csv.js'","'../../../jquery-csv.js'")

    # import the unit test logic
    testPaths = unitTest["relPath"].split("\\")
    data = PrependText(data, "import { create, globals } from 'webgpu';", "import { test_" + testPaths[0] + "_" + testPaths[1] + " } from \"../../../UnitTestLogic.js\";\n")

    # Init
    data = ReplaceTextSection(data, "/* TODO:\nprovide", "*/", "// initialize the module\nnumExecutions = await test_" + testPaths[0] + "_" + testPaths[1] + ".Init(device, " + testPaths[1] + ");")

    # PreExecute
    data = AppendText(data, "const encoder = device.createCommandEncoder();\n", "    test_" + testPaths[0] + "_" + testPaths[1] + ".PreExecute(device, encoder, " + testPaths[1] + ", frameIndex);\n")

    # PostExecute
    data = PrependText(data, "// Submit the command encoder", "// Do post technique execution work\n    test_" + testPaths[0] + "_" + testPaths[1] + ".NodeOnly_PostExecute(device, encoder, " + testPaths[1] + ", frameIndex);\n\n    ")

    # Validate
    data = AppendText(data, "await device.queue.onSubmittedWorkDone();", "\n\n    // Do validation work\n    await test_" + testPaths[0] + "_" + testPaths[1] + ".NodeOnly_Validate(" + testPaths[1] + ", frameIndex);")

    # Write the file data out
    with open(fileName, 'w') as f:
        f.write(data)

    # Add this test to the index.html file
    indexHTML.write("<a href=\"UnitTests\\" + unitTest["relPath"] + "\\index.html\">" + unitTest["relPath"] + "</a>\n<br><br>\n\n")

indexHTML.write("</body>\n</html>\n")
indexHTML.close()

# ==================== Modify index.html files

for index, unitTest in enumerate(UnitTests):
    fileName = unitTest["module"] + "\\index.html"
    #print(fileName)
    #print(unitTest)
    #print(unitTest["relPath"].split("\\"))
    data = open(fileName, "r").read()

    # Home
    data = PrependText(data, "                    <canvas width=", "                    <a href=\"/index.html\">Index</a>\n")

    # <<
    if index > 0:
        nextUnitTest = UnitTests[index-1]
        data = PrependText(data, "                    <canvas width=", "                    <a href=\"/UnitTests\\" + nextUnitTest["relPath"] + "\\index.html\"> << </a>\n")
    else:
        data = PrependText(data, "                    <canvas width=", "                    <<\n")

    # Current number
    data = PrependText(data, "                    <canvas width=", "                    " + str(index+1) + " / " + str(len(UnitTests)) + "\n")

    # >>
    if index + 1 < len(UnitTests):
        nextUnitTest = UnitTests[index+1]
        data = PrependText(data, "                    <canvas width=", "                    <a href=\"/UnitTests\\" + nextUnitTest["relPath"] + "\\index.html\"> >> </a><br/>\n")
    else:
        data = PrependText(data, "                    <canvas width=", "                    >><br/>\n")

    data = data.replace("'./Shared.js'","'../../../Shared.js'")
    data = data.replace("'./jquery-csv.js'","'../../../jquery-csv.js'")
    data = data.replace("./style.css","../../../style.css")

    # import the unit test logic
    testPaths = unitTest["relPath"].split("\\")
    data = AppendText(data, "<script type=\"module\">\n", "            import { test_" + testPaths[0] + "_" + testPaths[1] + " } from \"../../../UnitTestLogic.js\";\n\n")

    # Init
    data = ReplaceTextSection(data, "/* TODO:\n            provide", "*/", "// Initialize the technique from unit test logic\n            await test_" + testPaths[0] + "_" + testPaths[1] + ".Init(device, " + testPaths[1] + ");")

    # PreExecute
    data = ReplaceTextSection(data, "/* TODO:\n            provide", "*/", "// Do pre execute unit test logic\n                test_" + testPaths[0] + "_" + testPaths[1] + ".PreExecute(device, encoder, " + testPaths[1] + ", frameIndex);")

    # Write the file data out
    with open(fileName, 'w') as f:
        f.write(data)

# ==================== Modify module.js files

for unitTest in UnitTests:
    fileName = unitTest["module"] + "\\" + unitTest["moduleName"] + "_Module.js"
    #print(fileName)
    data = open(fileName, "r").read()

    data = data.replace("'./Shared.js'","'../../../Shared.js'")
    data = data.replace("'./jquery-csv.js'","'../../../jquery-csv.js'")

    # Write the file data out
    with open(fileName, 'w') as f:
        f.write(data)
