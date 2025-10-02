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
    "CopyResource\\CopyResourceTest", # Has CopyResourceTest_FB instead. can't copy texture <-> buffer

    # Need to make these work
    "RayTrace\\simpleRT",
    "RayTrace\\simpleRT_inline",
    "RayTrace\\AnyHitSimple",
    "RayTrace\\TwoRayGens",
    "RayTrace\\IntersectionShader",
    "RayTrace\\AnyHit",

    "Textures\\Texture2DArrayRW_RGS",
    "Textures\\Texture2DArrayRW_CS",
    "Textures\\Texture2DArrayRW_PS",
    "Textures\\Texture2DRW_RGS",
    "Textures\\Texture2DRW_CS",
    "Textures\\Texture2DRW_PS",
    "Textures\\Texture3DRW_RGS",
    "Textures\\Texture3DRW_CS",
    "Textures\\Texture3DRW_PS",
    "Textures\\TextureCubeRW_RGS",
    "Textures\\TextureCubeRW_CS",
    "Textures\\TextureCubeRW_PS",
    "Textures\\TextureFormats",

    "Textures\\Mips_DrawCall",
    "Textures\\Mips_Imported_2D",
    "Textures\\Mips_Imported_2DArray",
    "Textures\\Mips_Imported_3D",
    "Textures\\Mips_Imported_Cube",
    "Textures\\Mips_ShaderToken_2D",
    "Textures\\Mips_ShaderToken_2DArray",
    "Textures\\Mips_ShaderToken_3D",
    "Textures\\Mips_ShaderToken_Cube",
    "Textures\\Mips_CS_2D",
    "Textures\\Mips_CS_2DArray",
    "Textures\\Mips_CS_3D",
    "Textures\\Mips_CS_Cube",
    "Textures\\Mips_RGS_2D",
    "Textures\\Mips_VSPS_2D",

    "Raster\\simpleRaster_Points",
    "Raster\\simpleRaster_Lines",
    "Raster\\simpleRaster",
    "Raster\\SimpleRaster2",
    "Raster\\simpleRasterInSubgraph",

    "MeshShaders\\Mesh",
    "MeshShaders\\MeshAmplification",
    "MeshShaders\\MeshAmplificationLines",

    "Raster\\VRS",

    "Buffers\\MultipleUVMesh",

    "SubGraph\\SubGraphLoops",
    "SubGraph\\SubInSub",
    "SubGraph\\ConstOverride",

    "Compute\\IndirectDispatch",

    # Viewer only tests
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

    # Just need to make work
    "RayTrace\\simpleRTDynamic",
    "Compute\\BufferAtomics",
    "CopyResource\\CopyPartialBuffer",

    # Just need to generate the code and test it
    "Compute\\Defines",
    "Raster\\simpleRasterMSAA",

    # WebGPU only
    "Buffers\\buffertest_webgpu",
    "Textures\\Mips_VSPS_2D_WebGPU",

    # Need to implement buffer view begin / size
    "Buffers\\buffertest_viewOffsetCS",
    "Buffers\\buffertest_viewOffsetCS_Vars",

    # Unsure if supported
    "Raster\\IndirectDraw",
]

# ==================== GENERATE CODE FOR TECHNIQUES

for fileName in glob.glob(os.getcwd() + "/Techniques/UnitTests/**/*.py", recursive = True):
    relFileName = os.path.relpath(fileName, os.getcwd() + "/Techniques/UnitTests/")
    relFileNameNoExtension = os.path.splitext(relFileName)[0]
    relFileName = relFileName + ".gg"

    fileNameNoExtension = os.path.splitext(fileName)[0]
    fileName = fileNameNoExtension + ".gg"

    if relFileNameNoExtension == "RunTests" or relFileNameNoExtension == "TestLogic":
        #print("Skipping: " + relFileNameNoExtension)
        continue

    # unsupported unit tests
    if relFileNameNoExtension in unsupportedTests:
        #print("Skipping: " + relFileNameNoExtension)
        continue

    if not relFileNameNoExtension.startswith(requiredTestPrefix):
        #print("Skipping: " + relFileNameNoExtension)
        continue

    outDirName = "_GeneratedCode/UnitTests/UE_5_3/UnitTests/" + relFileNameNoExtension
    print(relFileNameNoExtension)
    print(".\\GigiCompiler.exe UE_5_3_Module " + fileName + " " + outDirName)
    returncode = subprocess.run(".\\GigiCompiler.exe UE_5_3_Module " + fileName + " " + outDirName, shell=True, check=True).returncode
    print("returned " + str(returncode))
    print("")
