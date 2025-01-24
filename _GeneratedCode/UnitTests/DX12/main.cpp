// Dear ImGui: standalone example application for DirectX 12

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important: to compile on 32-bit systems, the DirectX12 backend requires code to be compiled with '#define ImTextureID ImU64'.
// This is because we need ImTextureID to carry a 64-bit value and by default ImTextureID is defined as void*.
// This define is set in the example .vcxproj file and need to be replicated in your app or by adding it to your imconfig.h file.

// Gigi Modification Begin
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
// Gigi Modification End

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

// Gigi Modification Begin
#ifdef _DEBUG
    #define DX12_ENABLE_DEBUG_LAYER
    #define DX12_GPU_VALIDATION() false
    #define DX12_BREAK_ON_WARN() false
    #define DX12_BREAK_ON_CORRUPTION() true
    #define DX12_BREAK_ON_ERROR() true
#endif

#define MAKE_PIX_CAPTURE() false // if true, will emit a pix capture for every frame the unit tests run.

static const unsigned int c_renderSize[2] = { 1024, 768 };
// Gigi Modification End

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

// Gigi Modification Begin
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\AgilitySDK\\bin\\"; }

#include "DX12Utils/FileCache.h"
#include "spdlog/spdlog.h"
#include "DX12Utils/logfn.h"
#include "DX12Utils/Camera.h"
static void LogFunction(LogLevel level, const char* msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, msg, args);
    va_end(args);

    switch (level)
    {
        case LogLevel::Info: spdlog::info(buffer); break;
        case LogLevel::Warn: spdlog::warn(buffer); break;
        case LogLevel::Error: spdlog::error(buffer); break;
    }
}

#include "WinPixEventRuntime/pix3.h"
static void PerfEventBeginFn(const char* name, ID3D12GraphicsCommandList* commandList, int index)
{
    PIXBeginEvent(commandList, PIX_COLOR_INDEX(index), name);
}

static void PerfEventEndFn(ID3D12GraphicsCommandList* commandList)
{
    PIXEndEvent(commandList);
}

#if MAKE_PIX_CAPTURE()
static bool GetLatestWinPixGpuCapturerPath(std::wstring& path)
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    std::wstring pixSearchPath = programFilesPath + std::wstring(L"\\Microsoft PIX\\*");

    WIN32_FIND_DATAW findData;
    bool foundPixInstallation = false;
    wchar_t newestVersionFound[MAX_PATH];

    HANDLE hFind = FindFirstFileW(pixSearchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
                (findData.cFileName[0] != '.'))
            {
                if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
                {
                    foundPixInstallation = true;
                    StringCchCopyW(newestVersionFound, _countof(newestVersionFound), findData.cFileName);
                }
            }
        } while (FindNextFileW(hFind, &findData) != 0);
    }

    FindClose(hFind);

    if (!foundPixInstallation)
    {
        return false;
    }

    wchar_t output[MAX_PATH];
    StringCchCopyW(output, pixSearchPath.length(), pixSearchPath.data());
    StringCchCatW(output, MAX_PATH, &newestVersionFound[0]);
    StringCchCatW(output, MAX_PATH, L"\\WinPixGpuCapturer.dll");

    path = output;
    return true;
}
#endif

#include "DX12Utils/ReadbackHelper.h"
static DX12Utils::ReadbackHelper    g_readbackHelper;
// Gigi Modification End

// Gigi Modification Begin - Includes And Context
#include "UnitTests\Barrier\BarrierTest\public\technique.h"
#include "UnitTests\Barrier\BarrierTest\public\imgui.h"
#include "UnitTests\Barrier\BarrierTest\private\technique.h"
#include "UnitTests\Buffers\buffertest\public\technique.h"
#include "UnitTests\Buffers\buffertest\public\imgui.h"
#include "UnitTests\Buffers\buffertest\private\technique.h"
#include "UnitTests\Buffers\StructuredBuffer\public\technique.h"
#include "UnitTests\Buffers\StructuredBuffer\public\imgui.h"
#include "UnitTests\Buffers\StructuredBuffer\private\technique.h"
#include "UnitTests\Compute\boxblur\public\technique.h"
#include "UnitTests\Compute\boxblur\public\imgui.h"
#include "UnitTests\Compute\boxblur\private\technique.h"
#include "UnitTests\Compute\IndirectDispatch\public\technique.h"
#include "UnitTests\Compute\IndirectDispatch\public\imgui.h"
#include "UnitTests\Compute\IndirectDispatch\private\technique.h"
#include "UnitTests\Compute\ReadbackSequence\public\technique.h"
#include "UnitTests\Compute\ReadbackSequence\public\imgui.h"
#include "UnitTests\Compute\ReadbackSequence\private\technique.h"
#include "UnitTests\Compute\simple\public\technique.h"
#include "UnitTests\Compute\simple\public\imgui.h"
#include "UnitTests\Compute\simple\private\technique.h"
#include "UnitTests\Compute\SlangAutoDiff\public\technique.h"
#include "UnitTests\Compute\SlangAutoDiff\public\imgui.h"
#include "UnitTests\Compute\SlangAutoDiff\private\technique.h"
#include "UnitTests\CopyResource\CopyResourceTest\public\technique.h"
#include "UnitTests\CopyResource\CopyResourceTest\public\imgui.h"
#include "UnitTests\CopyResource\CopyResourceTest\private\technique.h"
#include "UnitTests\CopyResource\CopyResourceTest_FB\public\technique.h"
#include "UnitTests\CopyResource\CopyResourceTest_FB\public\imgui.h"
#include "UnitTests\CopyResource\CopyResourceTest_FB\private\technique.h"
#include "UnitTests\MeshShaders\Mesh\public\technique.h"
#include "UnitTests\MeshShaders\Mesh\public\imgui.h"
#include "UnitTests\MeshShaders\Mesh\private\technique.h"
#include "UnitTests\MeshShaders\MeshAmplification\public\technique.h"
#include "UnitTests\MeshShaders\MeshAmplification\public\imgui.h"
#include "UnitTests\MeshShaders\MeshAmplification\private\technique.h"
#include "UnitTests\MeshShaders\MeshAmplificationLines\public\technique.h"
#include "UnitTests\MeshShaders\MeshAmplificationLines\public\imgui.h"
#include "UnitTests\MeshShaders\MeshAmplificationLines\private\technique.h"
#include "UnitTests\Raster\NoVertex_NoIndex_NoInstance\public\technique.h"
#include "UnitTests\Raster\NoVertex_NoIndex_NoInstance\public\imgui.h"
#include "UnitTests\Raster\NoVertex_NoIndex_NoInstance\private\technique.h"
#include "UnitTests\Raster\simpleRaster\public\technique.h"
#include "UnitTests\Raster\simpleRaster\public\imgui.h"
#include "UnitTests\Raster\simpleRaster\private\technique.h"
#include "UnitTests\Raster\simpleRaster2\public\technique.h"
#include "UnitTests\Raster\simpleRaster2\public\imgui.h"
#include "UnitTests\Raster\simpleRaster2\private\technique.h"
#include "UnitTests\Raster\simpleRasterInSubgraph\public\technique.h"
#include "UnitTests\Raster\simpleRasterInSubgraph\public\imgui.h"
#include "UnitTests\Raster\simpleRasterInSubgraph\private\technique.h"
#include "UnitTests\Raster\simpleRaster_Lines\public\technique.h"
#include "UnitTests\Raster\simpleRaster_Lines\public\imgui.h"
#include "UnitTests\Raster\simpleRaster_Lines\private\technique.h"
#include "UnitTests\Raster\simpleRaster_Points\public\technique.h"
#include "UnitTests\Raster\simpleRaster_Points\public\imgui.h"
#include "UnitTests\Raster\simpleRaster_Points\private\technique.h"
#include "UnitTests\Raster\Stencil\public\technique.h"
#include "UnitTests\Raster\Stencil\public\imgui.h"
#include "UnitTests\Raster\Stencil\private\technique.h"
#include "UnitTests\Raster\VRS\public\technique.h"
#include "UnitTests\Raster\VRS\public\imgui.h"
#include "UnitTests\Raster\VRS\private\technique.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_NoInstance\public\technique.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_NoInstance\public\imgui.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_NoInstance\private\technique.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_YesInstanceStruct\public\technique.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_YesInstanceStruct\public\imgui.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_YesInstanceStruct\private\technique.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_YesInstanceType\public\technique.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_YesInstanceType\public\imgui.h"
#include "UnitTests\Raster\YesVertexStruct_NoIndex_YesInstanceType\private\technique.h"
#include "UnitTests\Raster\YesVertexStruct_YesIndex_NoInstance\public\technique.h"
#include "UnitTests\Raster\YesVertexStruct_YesIndex_NoInstance\public\imgui.h"
#include "UnitTests\Raster\YesVertexStruct_YesIndex_NoInstance\private\technique.h"
#include "UnitTests\Raster\YesVertexType_NoIndex_NoInstance\public\technique.h"
#include "UnitTests\Raster\YesVertexType_NoIndex_NoInstance\public\imgui.h"
#include "UnitTests\Raster\YesVertexType_NoIndex_NoInstance\private\technique.h"
#include "UnitTests\RayTrace\AnyHit\public\technique.h"
#include "UnitTests\RayTrace\AnyHit\public\imgui.h"
#include "UnitTests\RayTrace\AnyHit\private\technique.h"
#include "UnitTests\RayTrace\AnyHitSimple\public\technique.h"
#include "UnitTests\RayTrace\AnyHitSimple\public\imgui.h"
#include "UnitTests\RayTrace\AnyHitSimple\private\technique.h"
#include "UnitTests\RayTrace\IntersectionShader\public\technique.h"
#include "UnitTests\RayTrace\IntersectionShader\public\imgui.h"
#include "UnitTests\RayTrace\IntersectionShader\private\technique.h"
#include "UnitTests\RayTrace\simpleRT\public\technique.h"
#include "UnitTests\RayTrace\simpleRT\public\imgui.h"
#include "UnitTests\RayTrace\simpleRT\private\technique.h"
#include "UnitTests\RayTrace\simpleRT_inline\public\technique.h"
#include "UnitTests\RayTrace\simpleRT_inline\public\imgui.h"
#include "UnitTests\RayTrace\simpleRT_inline\private\technique.h"
#include "UnitTests\RayTrace\TwoRayGens\public\technique.h"
#include "UnitTests\RayTrace\TwoRayGens\public\imgui.h"
#include "UnitTests\RayTrace\TwoRayGens\private\technique.h"
#include "UnitTests\RayTrace\TwoRayGensSubgraph\public\technique.h"
#include "UnitTests\RayTrace\TwoRayGensSubgraph\public\imgui.h"
#include "UnitTests\RayTrace\TwoRayGensSubgraph\private\technique.h"
#include "UnitTests\SubGraph\SubGraphLoops\public\technique.h"
#include "UnitTests\SubGraph\SubGraphLoops\public\imgui.h"
#include "UnitTests\SubGraph\SubGraphLoops\private\technique.h"
#include "UnitTests\SubGraph\SubGraphTest\public\technique.h"
#include "UnitTests\SubGraph\SubGraphTest\public\imgui.h"
#include "UnitTests\SubGraph\SubGraphTest\private\technique.h"
#include "UnitTests\SubGraph\SubInSub\public\technique.h"
#include "UnitTests\SubGraph\SubInSub\public\imgui.h"
#include "UnitTests\SubGraph\SubInSub\private\technique.h"
#include "UnitTests\Textures\Mips_CS_2D\public\technique.h"
#include "UnitTests\Textures\Mips_CS_2D\public\imgui.h"
#include "UnitTests\Textures\Mips_CS_2D\private\technique.h"
#include "UnitTests\Textures\Mips_CS_2DArray\public\technique.h"
#include "UnitTests\Textures\Mips_CS_2DArray\public\imgui.h"
#include "UnitTests\Textures\Mips_CS_2DArray\private\technique.h"
#include "UnitTests\Textures\Mips_CS_3D\public\technique.h"
#include "UnitTests\Textures\Mips_CS_3D\public\imgui.h"
#include "UnitTests\Textures\Mips_CS_3D\private\technique.h"
#include "UnitTests\Textures\Mips_CS_Cube\public\technique.h"
#include "UnitTests\Textures\Mips_CS_Cube\public\imgui.h"
#include "UnitTests\Textures\Mips_CS_Cube\private\technique.h"
#include "UnitTests\Textures\Mips_DrawCall\public\technique.h"
#include "UnitTests\Textures\Mips_DrawCall\public\imgui.h"
#include "UnitTests\Textures\Mips_DrawCall\private\technique.h"
#include "UnitTests\Textures\Mips_RGS_2D\public\technique.h"
#include "UnitTests\Textures\Mips_RGS_2D\public\imgui.h"
#include "UnitTests\Textures\Mips_RGS_2D\private\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_2D\public\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_2D\public\imgui.h"
#include "UnitTests\Textures\Mips_ShaderToken_2D\private\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_2DArray\public\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_2DArray\public\imgui.h"
#include "UnitTests\Textures\Mips_ShaderToken_2DArray\private\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_3D\public\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_3D\public\imgui.h"
#include "UnitTests\Textures\Mips_ShaderToken_3D\private\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_Cube\public\technique.h"
#include "UnitTests\Textures\Mips_ShaderToken_Cube\public\imgui.h"
#include "UnitTests\Textures\Mips_ShaderToken_Cube\private\technique.h"
#include "UnitTests\Textures\Mips_VSPS_2D\public\technique.h"
#include "UnitTests\Textures\Mips_VSPS_2D\public\imgui.h"
#include "UnitTests\Textures\Mips_VSPS_2D\private\technique.h"
#include "UnitTests\Textures\Texture2DArrayRW_CS\public\technique.h"
#include "UnitTests\Textures\Texture2DArrayRW_CS\public\imgui.h"
#include "UnitTests\Textures\Texture2DArrayRW_CS\private\technique.h"
#include "UnitTests\Textures\Texture2DArrayRW_PS\public\technique.h"
#include "UnitTests\Textures\Texture2DArrayRW_PS\public\imgui.h"
#include "UnitTests\Textures\Texture2DArrayRW_PS\private\technique.h"
#include "UnitTests\Textures\Texture2DArrayRW_RGS\public\technique.h"
#include "UnitTests\Textures\Texture2DArrayRW_RGS\public\imgui.h"
#include "UnitTests\Textures\Texture2DArrayRW_RGS\private\technique.h"
#include "UnitTests\Textures\Texture2DRW_CS\public\technique.h"
#include "UnitTests\Textures\Texture2DRW_CS\public\imgui.h"
#include "UnitTests\Textures\Texture2DRW_CS\private\technique.h"
#include "UnitTests\Textures\Texture2DRW_PS\public\technique.h"
#include "UnitTests\Textures\Texture2DRW_PS\public\imgui.h"
#include "UnitTests\Textures\Texture2DRW_PS\private\technique.h"
#include "UnitTests\Textures\Texture2DRW_RGS\public\technique.h"
#include "UnitTests\Textures\Texture2DRW_RGS\public\imgui.h"
#include "UnitTests\Textures\Texture2DRW_RGS\private\technique.h"
#include "UnitTests\Textures\Texture3DRW_CS\public\technique.h"
#include "UnitTests\Textures\Texture3DRW_CS\public\imgui.h"
#include "UnitTests\Textures\Texture3DRW_CS\private\technique.h"
#include "UnitTests\Textures\Texture3DRW_PS\public\technique.h"
#include "UnitTests\Textures\Texture3DRW_PS\public\imgui.h"
#include "UnitTests\Textures\Texture3DRW_PS\private\technique.h"
#include "UnitTests\Textures\Texture3DRW_RGS\public\technique.h"
#include "UnitTests\Textures\Texture3DRW_RGS\public\imgui.h"
#include "UnitTests\Textures\Texture3DRW_RGS\private\technique.h"
#include "UnitTests\Textures\TextureCubeRW_CS\public\technique.h"
#include "UnitTests\Textures\TextureCubeRW_CS\public\imgui.h"
#include "UnitTests\Textures\TextureCubeRW_CS\private\technique.h"
#include "UnitTests\Textures\TextureCubeRW_PS\public\technique.h"
#include "UnitTests\Textures\TextureCubeRW_PS\public\imgui.h"
#include "UnitTests\Textures\TextureCubeRW_PS\private\technique.h"
#include "UnitTests\Textures\TextureCubeRW_RGS\public\technique.h"
#include "UnitTests\Textures\TextureCubeRW_RGS\public\imgui.h"
#include "UnitTests\Textures\TextureCubeRW_RGS\private\technique.h"
#include "UnitTests\Textures\TextureFormats\public\technique.h"
#include "UnitTests\Textures\TextureFormats\public\imgui.h"
#include "UnitTests\Textures\TextureFormats\private\technique.h"

BarrierTest::Context* m_BarrierTest = nullptr;
buffertest::Context* m_buffertest = nullptr;
StructuredBuffer::Context* m_StructuredBuffer = nullptr;
boxblur::Context* m_boxblur = nullptr;
IndirectDispatch::Context* m_IndirectDispatch = nullptr;
ReadbackSequence::Context* m_ReadbackSequence = nullptr;
simple::Context* m_simple = nullptr;
SlangAutoDiff::Context* m_SlangAutoDiff = nullptr;
CopyResourceTest::Context* m_CopyResourceTest = nullptr;
CopyResourceTest_FB::Context* m_CopyResourceTest_FB = nullptr;
Mesh::Context* m_Mesh = nullptr;
MeshAmplification::Context* m_MeshAmplification = nullptr;
MeshAmplificationLines::Context* m_MeshAmplificationLines = nullptr;
NoVertex_NoIndex_NoInstance::Context* m_NoVertex_NoIndex_NoInstance = nullptr;
simpleRaster::Context* m_simpleRaster = nullptr;
simpleRaster2::Context* m_simpleRaster2 = nullptr;
simpleRasterInSubgraph::Context* m_simpleRasterInSubgraph = nullptr;
simpleRaster_Lines::Context* m_simpleRaster_Lines = nullptr;
simpleRaster_Points::Context* m_simpleRaster_Points = nullptr;
Stencil::Context* m_Stencil = nullptr;
VRS::Context* m_VRS = nullptr;
YesVertexStruct_NoIndex_NoInstance::Context* m_YesVertexStruct_NoIndex_NoInstance = nullptr;
YesVertexStruct_NoIndex_YesInstanceStruct::Context* m_YesVertexStruct_NoIndex_YesInstanceStruct = nullptr;
YesVertexStruct_NoIndex_YesInstanceType::Context* m_YesVertexStruct_NoIndex_YesInstanceType = nullptr;
YesVertexStruct_YesIndex_NoInstance::Context* m_YesVertexStruct_YesIndex_NoInstance = nullptr;
YesVertexType_NoIndex_NoInstance::Context* m_YesVertexType_NoIndex_NoInstance = nullptr;
AnyHit::Context* m_AnyHit = nullptr;
AnyHitSimple::Context* m_AnyHitSimple = nullptr;
IntersectionShader::Context* m_IntersectionShader = nullptr;
simpleRT::Context* m_simpleRT = nullptr;
simpleRT_inline::Context* m_simpleRT_inline = nullptr;
TwoRayGens::Context* m_TwoRayGens = nullptr;
TwoRayGensSubgraph::Context* m_TwoRayGensSubgraph = nullptr;
SubGraphLoops::Context* m_SubGraphLoops = nullptr;
SubGraphTest::Context* m_SubGraphTest = nullptr;
SubInSub::Context* m_SubInSub = nullptr;
Mips_CS_2D::Context* m_Mips_CS_2D = nullptr;
Mips_CS_2DArray::Context* m_Mips_CS_2DArray = nullptr;
Mips_CS_3D::Context* m_Mips_CS_3D = nullptr;
Mips_CS_Cube::Context* m_Mips_CS_Cube = nullptr;
Mips_DrawCall::Context* m_Mips_DrawCall = nullptr;
Mips_RGS_2D::Context* m_Mips_RGS_2D = nullptr;
Mips_ShaderToken_2D::Context* m_Mips_ShaderToken_2D = nullptr;
Mips_ShaderToken_2DArray::Context* m_Mips_ShaderToken_2DArray = nullptr;
Mips_ShaderToken_3D::Context* m_Mips_ShaderToken_3D = nullptr;
Mips_ShaderToken_Cube::Context* m_Mips_ShaderToken_Cube = nullptr;
Mips_VSPS_2D::Context* m_Mips_VSPS_2D = nullptr;
Texture2DArrayRW_CS::Context* m_Texture2DArrayRW_CS = nullptr;
Texture2DArrayRW_PS::Context* m_Texture2DArrayRW_PS = nullptr;
Texture2DArrayRW_RGS::Context* m_Texture2DArrayRW_RGS = nullptr;
Texture2DRW_CS::Context* m_Texture2DRW_CS = nullptr;
Texture2DRW_PS::Context* m_Texture2DRW_PS = nullptr;
Texture2DRW_RGS::Context* m_Texture2DRW_RGS = nullptr;
Texture3DRW_CS::Context* m_Texture3DRW_CS = nullptr;
Texture3DRW_PS::Context* m_Texture3DRW_PS = nullptr;
Texture3DRW_RGS::Context* m_Texture3DRW_RGS = nullptr;
TextureCubeRW_CS::Context* m_TextureCubeRW_CS = nullptr;
TextureCubeRW_PS::Context* m_TextureCubeRW_PS = nullptr;
TextureCubeRW_RGS::Context* m_TextureCubeRW_RGS = nullptr;
TextureFormats::Context* m_TextureFormats = nullptr;

#include "UnitTestLogic.h"

bool g_doSubsetTest = false; // If true, it will only test the techniques set to true below

bool g_doTest_BarrierTest = false;
bool g_doTest_buffertest = false;
bool g_doTest_StructuredBuffer = false;
bool g_doTest_boxblur = false;
bool g_doTest_IndirectDispatch = false;
bool g_doTest_ReadbackSequence = false;
bool g_doTest_simple = false;
bool g_doTest_SlangAutoDiff = false;
bool g_doTest_CopyResourceTest = false;
bool g_doTest_CopyResourceTest_FB = false;
bool g_doTest_Mesh = false;
bool g_doTest_MeshAmplification = false;
bool g_doTest_MeshAmplificationLines = false;
bool g_doTest_NoVertex_NoIndex_NoInstance = false;
bool g_doTest_simpleRaster = false;
bool g_doTest_simpleRaster2 = false;
bool g_doTest_simpleRasterInSubgraph = false;
bool g_doTest_simpleRaster_Lines = false;
bool g_doTest_simpleRaster_Points = false;
bool g_doTest_Stencil = false;
bool g_doTest_VRS = false;
bool g_doTest_YesVertexStruct_NoIndex_NoInstance = false;
bool g_doTest_YesVertexStruct_NoIndex_YesInstanceStruct = false;
bool g_doTest_YesVertexStruct_NoIndex_YesInstanceType = false;
bool g_doTest_YesVertexStruct_YesIndex_NoInstance = false;
bool g_doTest_YesVertexType_NoIndex_NoInstance = false;
bool g_doTest_AnyHit = false;
bool g_doTest_AnyHitSimple = false;
bool g_doTest_IntersectionShader = false;
bool g_doTest_simpleRT = false;
bool g_doTest_simpleRT_inline = false;
bool g_doTest_TwoRayGens = false;
bool g_doTest_TwoRayGensSubgraph = false;
bool g_doTest_SubGraphLoops = false;
bool g_doTest_SubGraphTest = false;
bool g_doTest_SubInSub = false;
bool g_doTest_Mips_CS_2D = false;
bool g_doTest_Mips_CS_2DArray = false;
bool g_doTest_Mips_CS_3D = false;
bool g_doTest_Mips_CS_Cube = false;
bool g_doTest_Mips_DrawCall = false;
bool g_doTest_Mips_RGS_2D = false;
bool g_doTest_Mips_ShaderToken_2D = false;
bool g_doTest_Mips_ShaderToken_2DArray = false;
bool g_doTest_Mips_ShaderToken_3D = false;
bool g_doTest_Mips_ShaderToken_Cube = false;
bool g_doTest_Mips_VSPS_2D = false;
bool g_doTest_Texture2DArrayRW_CS = false;
bool g_doTest_Texture2DArrayRW_PS = false;
bool g_doTest_Texture2DArrayRW_RGS = false;
bool g_doTest_Texture2DRW_CS = false;
bool g_doTest_Texture2DRW_PS = false;
bool g_doTest_Texture2DRW_RGS = false;
bool g_doTest_Texture3DRW_CS = false;
bool g_doTest_Texture3DRW_PS = false;
bool g_doTest_Texture3DRW_RGS = false;
bool g_doTest_TextureCubeRW_CS = false;
bool g_doTest_TextureCubeRW_PS = false;
bool g_doTest_TextureCubeRW_RGS = false;
bool g_doTest_TextureFormats = false;
// Gigi Modification End

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue;
};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device*                g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap*        g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap*        g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue*          g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList*   g_pd3dCommandList = nullptr;
static ID3D12Fence*                 g_fence = nullptr;
static HANDLE                       g_fenceEvent = nullptr;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3*             g_pSwapChain = nullptr;
static HANDLE                       g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource*              g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

Camera g_camera;

// first half of key states are this frame, second half are last frame
uint8_t g_keyStates[512] = {};
uint8_t* g_keyStatesLastFrame = &g_keyStates[256];

// Mouse state is mouse x,y then left mouse down, right mouse down
float g_mouseState[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float g_mouseStateLastFrame[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

float g_cameraPos[3] = { 0.0f, 0.0f, 0.0f };
float g_cameraAltitudeAzimuth[2] = { 0.0f, 0.0f };

bool g_vsyncOn = true;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Gigi Modification Begin
    #if MAKE_PIX_CAPTURE()
        // Load WinPixGpuCapturer.dll so we can take programatic captures
        if (GetModuleHandleW(L"WinPixGpuCapturer.dll") == 0)
        {
            std::wstring pixPath;
            if (GetLatestWinPixGpuCapturerPath(pixPath))
            {
                LoadLibraryW(pixPath.c_str());
            }
        }
    #endif
    // Gigi Modification End

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);

    RECT windowSize = { 0, 0, (LONG)c_renderSize[0], (LONG)c_renderSize[1] };
    AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, false);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"UnitTests", WS_OVERLAPPEDWINDOW, 100, 100, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    // Gigi Modification Begin
    bool show_demo_window = false;
    bool show_simple_window = false;
    // Gigi Modification End
    // Gigi Modification Begin - Create Context
    if (!g_doSubsetTest || g_doTest_BarrierTest)
    {
        BarrierTest::Context::LogFn = &LogFunction;
        BarrierTest::Context::PerfEventBeginFn = &PerfEventBeginFn;
        BarrierTest::Context::PerfEventEndFn = &PerfEventEndFn;
        BarrierTest::Context::s_techniqueLocation = L".\\UnitTests\\Barrier\\BarrierTest\\";
        m_BarrierTest = BarrierTest::CreateContext(g_pd3dDevice);
        if (!m_BarrierTest)
        {
            printf("Could not create m_BarrierTest context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_buffertest)
    {
        buffertest::Context::LogFn = &LogFunction;
        buffertest::Context::PerfEventBeginFn = &PerfEventBeginFn;
        buffertest::Context::PerfEventEndFn = &PerfEventEndFn;
        buffertest::Context::s_techniqueLocation = L".\\UnitTests\\Buffers\\buffertest\\";
        m_buffertest = buffertest::CreateContext(g_pd3dDevice);
        if (!m_buffertest)
        {
            printf("Could not create m_buffertest context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_StructuredBuffer)
    {
        StructuredBuffer::Context::LogFn = &LogFunction;
        StructuredBuffer::Context::PerfEventBeginFn = &PerfEventBeginFn;
        StructuredBuffer::Context::PerfEventEndFn = &PerfEventEndFn;
        StructuredBuffer::Context::s_techniqueLocation = L".\\UnitTests\\Buffers\\StructuredBuffer\\";
        m_StructuredBuffer = StructuredBuffer::CreateContext(g_pd3dDevice);
        if (!m_StructuredBuffer)
        {
            printf("Could not create m_StructuredBuffer context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_boxblur)
    {
        boxblur::Context::LogFn = &LogFunction;
        boxblur::Context::PerfEventBeginFn = &PerfEventBeginFn;
        boxblur::Context::PerfEventEndFn = &PerfEventEndFn;
        boxblur::Context::s_techniqueLocation = L".\\UnitTests\\Compute\\boxblur\\";
        m_boxblur = boxblur::CreateContext(g_pd3dDevice);
        if (!m_boxblur)
        {
            printf("Could not create m_boxblur context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_IndirectDispatch)
    {
        IndirectDispatch::Context::LogFn = &LogFunction;
        IndirectDispatch::Context::PerfEventBeginFn = &PerfEventBeginFn;
        IndirectDispatch::Context::PerfEventEndFn = &PerfEventEndFn;
        IndirectDispatch::Context::s_techniqueLocation = L".\\UnitTests\\Compute\\IndirectDispatch\\";
        m_IndirectDispatch = IndirectDispatch::CreateContext(g_pd3dDevice);
        if (!m_IndirectDispatch)
        {
            printf("Could not create m_IndirectDispatch context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_ReadbackSequence)
    {
        ReadbackSequence::Context::LogFn = &LogFunction;
        ReadbackSequence::Context::PerfEventBeginFn = &PerfEventBeginFn;
        ReadbackSequence::Context::PerfEventEndFn = &PerfEventEndFn;
        ReadbackSequence::Context::s_techniqueLocation = L".\\UnitTests\\Compute\\ReadbackSequence\\";
        m_ReadbackSequence = ReadbackSequence::CreateContext(g_pd3dDevice);
        if (!m_ReadbackSequence)
        {
            printf("Could not create m_ReadbackSequence context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simple)
    {
        simple::Context::LogFn = &LogFunction;
        simple::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simple::Context::PerfEventEndFn = &PerfEventEndFn;
        simple::Context::s_techniqueLocation = L".\\UnitTests\\Compute\\simple\\";
        m_simple = simple::CreateContext(g_pd3dDevice);
        if (!m_simple)
        {
            printf("Could not create m_simple context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_SlangAutoDiff)
    {
        SlangAutoDiff::Context::LogFn = &LogFunction;
        SlangAutoDiff::Context::PerfEventBeginFn = &PerfEventBeginFn;
        SlangAutoDiff::Context::PerfEventEndFn = &PerfEventEndFn;
        SlangAutoDiff::Context::s_techniqueLocation = L".\\UnitTests\\Compute\\SlangAutoDiff\\";
        m_SlangAutoDiff = SlangAutoDiff::CreateContext(g_pd3dDevice);
        if (!m_SlangAutoDiff)
        {
            printf("Could not create m_SlangAutoDiff context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_CopyResourceTest)
    {
        CopyResourceTest::Context::LogFn = &LogFunction;
        CopyResourceTest::Context::PerfEventBeginFn = &PerfEventBeginFn;
        CopyResourceTest::Context::PerfEventEndFn = &PerfEventEndFn;
        CopyResourceTest::Context::s_techniqueLocation = L".\\UnitTests\\CopyResource\\CopyResourceTest\\";
        m_CopyResourceTest = CopyResourceTest::CreateContext(g_pd3dDevice);
        if (!m_CopyResourceTest)
        {
            printf("Could not create m_CopyResourceTest context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_CopyResourceTest_FB)
    {
        CopyResourceTest_FB::Context::LogFn = &LogFunction;
        CopyResourceTest_FB::Context::PerfEventBeginFn = &PerfEventBeginFn;
        CopyResourceTest_FB::Context::PerfEventEndFn = &PerfEventEndFn;
        CopyResourceTest_FB::Context::s_techniqueLocation = L".\\UnitTests\\CopyResource\\CopyResourceTest_FB\\";
        m_CopyResourceTest_FB = CopyResourceTest_FB::CreateContext(g_pd3dDevice);
        if (!m_CopyResourceTest_FB)
        {
            printf("Could not create m_CopyResourceTest_FB context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mesh)
    {
        Mesh::Context::LogFn = &LogFunction;
        Mesh::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mesh::Context::PerfEventEndFn = &PerfEventEndFn;
        Mesh::Context::s_techniqueLocation = L".\\UnitTests\\MeshShaders\\Mesh\\";
        m_Mesh = Mesh::CreateContext(g_pd3dDevice);
        if (!m_Mesh)
        {
            printf("Could not create m_Mesh context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_MeshAmplification)
    {
        MeshAmplification::Context::LogFn = &LogFunction;
        MeshAmplification::Context::PerfEventBeginFn = &PerfEventBeginFn;
        MeshAmplification::Context::PerfEventEndFn = &PerfEventEndFn;
        MeshAmplification::Context::s_techniqueLocation = L".\\UnitTests\\MeshShaders\\MeshAmplification\\";
        m_MeshAmplification = MeshAmplification::CreateContext(g_pd3dDevice);
        if (!m_MeshAmplification)
        {
            printf("Could not create m_MeshAmplification context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_MeshAmplificationLines)
    {
        MeshAmplificationLines::Context::LogFn = &LogFunction;
        MeshAmplificationLines::Context::PerfEventBeginFn = &PerfEventBeginFn;
        MeshAmplificationLines::Context::PerfEventEndFn = &PerfEventEndFn;
        MeshAmplificationLines::Context::s_techniqueLocation = L".\\UnitTests\\MeshShaders\\MeshAmplificationLines\\";
        m_MeshAmplificationLines = MeshAmplificationLines::CreateContext(g_pd3dDevice);
        if (!m_MeshAmplificationLines)
        {
            printf("Could not create m_MeshAmplificationLines context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_NoVertex_NoIndex_NoInstance)
    {
        NoVertex_NoIndex_NoInstance::Context::LogFn = &LogFunction;
        NoVertex_NoIndex_NoInstance::Context::PerfEventBeginFn = &PerfEventBeginFn;
        NoVertex_NoIndex_NoInstance::Context::PerfEventEndFn = &PerfEventEndFn;
        NoVertex_NoIndex_NoInstance::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\NoVertex_NoIndex_NoInstance\\";
        m_NoVertex_NoIndex_NoInstance = NoVertex_NoIndex_NoInstance::CreateContext(g_pd3dDevice);
        if (!m_NoVertex_NoIndex_NoInstance)
        {
            printf("Could not create m_NoVertex_NoIndex_NoInstance context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simpleRaster)
    {
        simpleRaster::Context::LogFn = &LogFunction;
        simpleRaster::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simpleRaster::Context::PerfEventEndFn = &PerfEventEndFn;
        simpleRaster::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\simpleRaster\\";
        m_simpleRaster = simpleRaster::CreateContext(g_pd3dDevice);
        if (!m_simpleRaster)
        {
            printf("Could not create m_simpleRaster context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simpleRaster2)
    {
        simpleRaster2::Context::LogFn = &LogFunction;
        simpleRaster2::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simpleRaster2::Context::PerfEventEndFn = &PerfEventEndFn;
        simpleRaster2::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\simpleRaster2\\";
        m_simpleRaster2 = simpleRaster2::CreateContext(g_pd3dDevice);
        if (!m_simpleRaster2)
        {
            printf("Could not create m_simpleRaster2 context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simpleRasterInSubgraph)
    {
        simpleRasterInSubgraph::Context::LogFn = &LogFunction;
        simpleRasterInSubgraph::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simpleRasterInSubgraph::Context::PerfEventEndFn = &PerfEventEndFn;
        simpleRasterInSubgraph::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\simpleRasterInSubgraph\\";
        m_simpleRasterInSubgraph = simpleRasterInSubgraph::CreateContext(g_pd3dDevice);
        if (!m_simpleRasterInSubgraph)
        {
            printf("Could not create m_simpleRasterInSubgraph context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simpleRaster_Lines)
    {
        simpleRaster_Lines::Context::LogFn = &LogFunction;
        simpleRaster_Lines::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simpleRaster_Lines::Context::PerfEventEndFn = &PerfEventEndFn;
        simpleRaster_Lines::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\simpleRaster_Lines\\";
        m_simpleRaster_Lines = simpleRaster_Lines::CreateContext(g_pd3dDevice);
        if (!m_simpleRaster_Lines)
        {
            printf("Could not create m_simpleRaster_Lines context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simpleRaster_Points)
    {
        simpleRaster_Points::Context::LogFn = &LogFunction;
        simpleRaster_Points::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simpleRaster_Points::Context::PerfEventEndFn = &PerfEventEndFn;
        simpleRaster_Points::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\simpleRaster_Points\\";
        m_simpleRaster_Points = simpleRaster_Points::CreateContext(g_pd3dDevice);
        if (!m_simpleRaster_Points)
        {
            printf("Could not create m_simpleRaster_Points context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Stencil)
    {
        Stencil::Context::LogFn = &LogFunction;
        Stencil::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Stencil::Context::PerfEventEndFn = &PerfEventEndFn;
        Stencil::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\Stencil\\";
        m_Stencil = Stencil::CreateContext(g_pd3dDevice);
        if (!m_Stencil)
        {
            printf("Could not create m_Stencil context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_VRS)
    {
        VRS::Context::LogFn = &LogFunction;
        VRS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        VRS::Context::PerfEventEndFn = &PerfEventEndFn;
        VRS::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\VRS\\";
        m_VRS = VRS::CreateContext(g_pd3dDevice);
        if (!m_VRS)
        {
            printf("Could not create m_VRS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_YesVertexStruct_NoIndex_NoInstance)
    {
        YesVertexStruct_NoIndex_NoInstance::Context::LogFn = &LogFunction;
        YesVertexStruct_NoIndex_NoInstance::Context::PerfEventBeginFn = &PerfEventBeginFn;
        YesVertexStruct_NoIndex_NoInstance::Context::PerfEventEndFn = &PerfEventEndFn;
        YesVertexStruct_NoIndex_NoInstance::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\YesVertexStruct_NoIndex_NoInstance\\";
        m_YesVertexStruct_NoIndex_NoInstance = YesVertexStruct_NoIndex_NoInstance::CreateContext(g_pd3dDevice);
        if (!m_YesVertexStruct_NoIndex_NoInstance)
        {
            printf("Could not create m_YesVertexStruct_NoIndex_NoInstance context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_YesVertexStruct_NoIndex_YesInstanceStruct)
    {
        YesVertexStruct_NoIndex_YesInstanceStruct::Context::LogFn = &LogFunction;
        YesVertexStruct_NoIndex_YesInstanceStruct::Context::PerfEventBeginFn = &PerfEventBeginFn;
        YesVertexStruct_NoIndex_YesInstanceStruct::Context::PerfEventEndFn = &PerfEventEndFn;
        YesVertexStruct_NoIndex_YesInstanceStruct::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\YesVertexStruct_NoIndex_YesInstanceStruct\\";
        m_YesVertexStruct_NoIndex_YesInstanceStruct = YesVertexStruct_NoIndex_YesInstanceStruct::CreateContext(g_pd3dDevice);
        if (!m_YesVertexStruct_NoIndex_YesInstanceStruct)
        {
            printf("Could not create m_YesVertexStruct_NoIndex_YesInstanceStruct context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_YesVertexStruct_NoIndex_YesInstanceType)
    {
        YesVertexStruct_NoIndex_YesInstanceType::Context::LogFn = &LogFunction;
        YesVertexStruct_NoIndex_YesInstanceType::Context::PerfEventBeginFn = &PerfEventBeginFn;
        YesVertexStruct_NoIndex_YesInstanceType::Context::PerfEventEndFn = &PerfEventEndFn;
        YesVertexStruct_NoIndex_YesInstanceType::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\YesVertexStruct_NoIndex_YesInstanceType\\";
        m_YesVertexStruct_NoIndex_YesInstanceType = YesVertexStruct_NoIndex_YesInstanceType::CreateContext(g_pd3dDevice);
        if (!m_YesVertexStruct_NoIndex_YesInstanceType)
        {
            printf("Could not create m_YesVertexStruct_NoIndex_YesInstanceType context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_YesVertexStruct_YesIndex_NoInstance)
    {
        YesVertexStruct_YesIndex_NoInstance::Context::LogFn = &LogFunction;
        YesVertexStruct_YesIndex_NoInstance::Context::PerfEventBeginFn = &PerfEventBeginFn;
        YesVertexStruct_YesIndex_NoInstance::Context::PerfEventEndFn = &PerfEventEndFn;
        YesVertexStruct_YesIndex_NoInstance::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\YesVertexStruct_YesIndex_NoInstance\\";
        m_YesVertexStruct_YesIndex_NoInstance = YesVertexStruct_YesIndex_NoInstance::CreateContext(g_pd3dDevice);
        if (!m_YesVertexStruct_YesIndex_NoInstance)
        {
            printf("Could not create m_YesVertexStruct_YesIndex_NoInstance context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_YesVertexType_NoIndex_NoInstance)
    {
        YesVertexType_NoIndex_NoInstance::Context::LogFn = &LogFunction;
        YesVertexType_NoIndex_NoInstance::Context::PerfEventBeginFn = &PerfEventBeginFn;
        YesVertexType_NoIndex_NoInstance::Context::PerfEventEndFn = &PerfEventEndFn;
        YesVertexType_NoIndex_NoInstance::Context::s_techniqueLocation = L".\\UnitTests\\Raster\\YesVertexType_NoIndex_NoInstance\\";
        m_YesVertexType_NoIndex_NoInstance = YesVertexType_NoIndex_NoInstance::CreateContext(g_pd3dDevice);
        if (!m_YesVertexType_NoIndex_NoInstance)
        {
            printf("Could not create m_YesVertexType_NoIndex_NoInstance context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_AnyHit)
    {
        AnyHit::Context::LogFn = &LogFunction;
        AnyHit::Context::PerfEventBeginFn = &PerfEventBeginFn;
        AnyHit::Context::PerfEventEndFn = &PerfEventEndFn;
        AnyHit::Context::s_techniqueLocation = L".\\UnitTests\\RayTrace\\AnyHit\\";
        m_AnyHit = AnyHit::CreateContext(g_pd3dDevice);
        if (!m_AnyHit)
        {
            printf("Could not create m_AnyHit context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_AnyHitSimple)
    {
        AnyHitSimple::Context::LogFn = &LogFunction;
        AnyHitSimple::Context::PerfEventBeginFn = &PerfEventBeginFn;
        AnyHitSimple::Context::PerfEventEndFn = &PerfEventEndFn;
        AnyHitSimple::Context::s_techniqueLocation = L".\\UnitTests\\RayTrace\\AnyHitSimple\\";
        m_AnyHitSimple = AnyHitSimple::CreateContext(g_pd3dDevice);
        if (!m_AnyHitSimple)
        {
            printf("Could not create m_AnyHitSimple context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_IntersectionShader)
    {
        IntersectionShader::Context::LogFn = &LogFunction;
        IntersectionShader::Context::PerfEventBeginFn = &PerfEventBeginFn;
        IntersectionShader::Context::PerfEventEndFn = &PerfEventEndFn;
        IntersectionShader::Context::s_techniqueLocation = L".\\UnitTests\\RayTrace\\IntersectionShader\\";
        m_IntersectionShader = IntersectionShader::CreateContext(g_pd3dDevice);
        if (!m_IntersectionShader)
        {
            printf("Could not create m_IntersectionShader context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simpleRT)
    {
        simpleRT::Context::LogFn = &LogFunction;
        simpleRT::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simpleRT::Context::PerfEventEndFn = &PerfEventEndFn;
        simpleRT::Context::s_techniqueLocation = L".\\UnitTests\\RayTrace\\simpleRT\\";
        m_simpleRT = simpleRT::CreateContext(g_pd3dDevice);
        if (!m_simpleRT)
        {
            printf("Could not create m_simpleRT context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_simpleRT_inline)
    {
        simpleRT_inline::Context::LogFn = &LogFunction;
        simpleRT_inline::Context::PerfEventBeginFn = &PerfEventBeginFn;
        simpleRT_inline::Context::PerfEventEndFn = &PerfEventEndFn;
        simpleRT_inline::Context::s_techniqueLocation = L".\\UnitTests\\RayTrace\\simpleRT_inline\\";
        m_simpleRT_inline = simpleRT_inline::CreateContext(g_pd3dDevice);
        if (!m_simpleRT_inline)
        {
            printf("Could not create m_simpleRT_inline context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_TwoRayGens)
    {
        TwoRayGens::Context::LogFn = &LogFunction;
        TwoRayGens::Context::PerfEventBeginFn = &PerfEventBeginFn;
        TwoRayGens::Context::PerfEventEndFn = &PerfEventEndFn;
        TwoRayGens::Context::s_techniqueLocation = L".\\UnitTests\\RayTrace\\TwoRayGens\\";
        m_TwoRayGens = TwoRayGens::CreateContext(g_pd3dDevice);
        if (!m_TwoRayGens)
        {
            printf("Could not create m_TwoRayGens context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_TwoRayGensSubgraph)
    {
        TwoRayGensSubgraph::Context::LogFn = &LogFunction;
        TwoRayGensSubgraph::Context::PerfEventBeginFn = &PerfEventBeginFn;
        TwoRayGensSubgraph::Context::PerfEventEndFn = &PerfEventEndFn;
        TwoRayGensSubgraph::Context::s_techniqueLocation = L".\\UnitTests\\RayTrace\\TwoRayGensSubgraph\\";
        m_TwoRayGensSubgraph = TwoRayGensSubgraph::CreateContext(g_pd3dDevice);
        if (!m_TwoRayGensSubgraph)
        {
            printf("Could not create m_TwoRayGensSubgraph context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_SubGraphLoops)
    {
        SubGraphLoops::Context::LogFn = &LogFunction;
        SubGraphLoops::Context::PerfEventBeginFn = &PerfEventBeginFn;
        SubGraphLoops::Context::PerfEventEndFn = &PerfEventEndFn;
        SubGraphLoops::Context::s_techniqueLocation = L".\\UnitTests\\SubGraph\\SubGraphLoops\\";
        m_SubGraphLoops = SubGraphLoops::CreateContext(g_pd3dDevice);
        if (!m_SubGraphLoops)
        {
            printf("Could not create m_SubGraphLoops context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_SubGraphTest)
    {
        SubGraphTest::Context::LogFn = &LogFunction;
        SubGraphTest::Context::PerfEventBeginFn = &PerfEventBeginFn;
        SubGraphTest::Context::PerfEventEndFn = &PerfEventEndFn;
        SubGraphTest::Context::s_techniqueLocation = L".\\UnitTests\\SubGraph\\SubGraphTest\\";
        m_SubGraphTest = SubGraphTest::CreateContext(g_pd3dDevice);
        if (!m_SubGraphTest)
        {
            printf("Could not create m_SubGraphTest context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_SubInSub)
    {
        SubInSub::Context::LogFn = &LogFunction;
        SubInSub::Context::PerfEventBeginFn = &PerfEventBeginFn;
        SubInSub::Context::PerfEventEndFn = &PerfEventEndFn;
        SubInSub::Context::s_techniqueLocation = L".\\UnitTests\\SubGraph\\SubInSub\\";
        m_SubInSub = SubInSub::CreateContext(g_pd3dDevice);
        if (!m_SubInSub)
        {
            printf("Could not create m_SubInSub context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_CS_2D)
    {
        Mips_CS_2D::Context::LogFn = &LogFunction;
        Mips_CS_2D::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_CS_2D::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_CS_2D::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_CS_2D\\";
        m_Mips_CS_2D = Mips_CS_2D::CreateContext(g_pd3dDevice);
        if (!m_Mips_CS_2D)
        {
            printf("Could not create m_Mips_CS_2D context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_CS_2DArray)
    {
        Mips_CS_2DArray::Context::LogFn = &LogFunction;
        Mips_CS_2DArray::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_CS_2DArray::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_CS_2DArray::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_CS_2DArray\\";
        m_Mips_CS_2DArray = Mips_CS_2DArray::CreateContext(g_pd3dDevice);
        if (!m_Mips_CS_2DArray)
        {
            printf("Could not create m_Mips_CS_2DArray context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_CS_3D)
    {
        Mips_CS_3D::Context::LogFn = &LogFunction;
        Mips_CS_3D::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_CS_3D::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_CS_3D::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_CS_3D\\";
        m_Mips_CS_3D = Mips_CS_3D::CreateContext(g_pd3dDevice);
        if (!m_Mips_CS_3D)
        {
            printf("Could not create m_Mips_CS_3D context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_CS_Cube)
    {
        Mips_CS_Cube::Context::LogFn = &LogFunction;
        Mips_CS_Cube::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_CS_Cube::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_CS_Cube::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_CS_Cube\\";
        m_Mips_CS_Cube = Mips_CS_Cube::CreateContext(g_pd3dDevice);
        if (!m_Mips_CS_Cube)
        {
            printf("Could not create m_Mips_CS_Cube context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_DrawCall)
    {
        Mips_DrawCall::Context::LogFn = &LogFunction;
        Mips_DrawCall::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_DrawCall::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_DrawCall::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_DrawCall\\";
        m_Mips_DrawCall = Mips_DrawCall::CreateContext(g_pd3dDevice);
        if (!m_Mips_DrawCall)
        {
            printf("Could not create m_Mips_DrawCall context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_RGS_2D)
    {
        Mips_RGS_2D::Context::LogFn = &LogFunction;
        Mips_RGS_2D::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_RGS_2D::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_RGS_2D::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_RGS_2D\\";
        m_Mips_RGS_2D = Mips_RGS_2D::CreateContext(g_pd3dDevice);
        if (!m_Mips_RGS_2D)
        {
            printf("Could not create m_Mips_RGS_2D context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_ShaderToken_2D)
    {
        Mips_ShaderToken_2D::Context::LogFn = &LogFunction;
        Mips_ShaderToken_2D::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_ShaderToken_2D::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_ShaderToken_2D::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_ShaderToken_2D\\";
        m_Mips_ShaderToken_2D = Mips_ShaderToken_2D::CreateContext(g_pd3dDevice);
        if (!m_Mips_ShaderToken_2D)
        {
            printf("Could not create m_Mips_ShaderToken_2D context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_ShaderToken_2DArray)
    {
        Mips_ShaderToken_2DArray::Context::LogFn = &LogFunction;
        Mips_ShaderToken_2DArray::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_ShaderToken_2DArray::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_ShaderToken_2DArray::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_ShaderToken_2DArray\\";
        m_Mips_ShaderToken_2DArray = Mips_ShaderToken_2DArray::CreateContext(g_pd3dDevice);
        if (!m_Mips_ShaderToken_2DArray)
        {
            printf("Could not create m_Mips_ShaderToken_2DArray context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_ShaderToken_3D)
    {
        Mips_ShaderToken_3D::Context::LogFn = &LogFunction;
        Mips_ShaderToken_3D::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_ShaderToken_3D::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_ShaderToken_3D::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_ShaderToken_3D\\";
        m_Mips_ShaderToken_3D = Mips_ShaderToken_3D::CreateContext(g_pd3dDevice);
        if (!m_Mips_ShaderToken_3D)
        {
            printf("Could not create m_Mips_ShaderToken_3D context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_ShaderToken_Cube)
    {
        Mips_ShaderToken_Cube::Context::LogFn = &LogFunction;
        Mips_ShaderToken_Cube::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_ShaderToken_Cube::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_ShaderToken_Cube::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_ShaderToken_Cube\\";
        m_Mips_ShaderToken_Cube = Mips_ShaderToken_Cube::CreateContext(g_pd3dDevice);
        if (!m_Mips_ShaderToken_Cube)
        {
            printf("Could not create m_Mips_ShaderToken_Cube context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Mips_VSPS_2D)
    {
        Mips_VSPS_2D::Context::LogFn = &LogFunction;
        Mips_VSPS_2D::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Mips_VSPS_2D::Context::PerfEventEndFn = &PerfEventEndFn;
        Mips_VSPS_2D::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Mips_VSPS_2D\\";
        m_Mips_VSPS_2D = Mips_VSPS_2D::CreateContext(g_pd3dDevice);
        if (!m_Mips_VSPS_2D)
        {
            printf("Could not create m_Mips_VSPS_2D context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture2DArrayRW_CS)
    {
        Texture2DArrayRW_CS::Context::LogFn = &LogFunction;
        Texture2DArrayRW_CS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture2DArrayRW_CS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture2DArrayRW_CS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture2DArrayRW_CS\\";
        m_Texture2DArrayRW_CS = Texture2DArrayRW_CS::CreateContext(g_pd3dDevice);
        if (!m_Texture2DArrayRW_CS)
        {
            printf("Could not create m_Texture2DArrayRW_CS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture2DArrayRW_PS)
    {
        Texture2DArrayRW_PS::Context::LogFn = &LogFunction;
        Texture2DArrayRW_PS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture2DArrayRW_PS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture2DArrayRW_PS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture2DArrayRW_PS\\";
        m_Texture2DArrayRW_PS = Texture2DArrayRW_PS::CreateContext(g_pd3dDevice);
        if (!m_Texture2DArrayRW_PS)
        {
            printf("Could not create m_Texture2DArrayRW_PS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture2DArrayRW_RGS)
    {
        Texture2DArrayRW_RGS::Context::LogFn = &LogFunction;
        Texture2DArrayRW_RGS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture2DArrayRW_RGS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture2DArrayRW_RGS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture2DArrayRW_RGS\\";
        m_Texture2DArrayRW_RGS = Texture2DArrayRW_RGS::CreateContext(g_pd3dDevice);
        if (!m_Texture2DArrayRW_RGS)
        {
            printf("Could not create m_Texture2DArrayRW_RGS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture2DRW_CS)
    {
        Texture2DRW_CS::Context::LogFn = &LogFunction;
        Texture2DRW_CS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture2DRW_CS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture2DRW_CS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture2DRW_CS\\";
        m_Texture2DRW_CS = Texture2DRW_CS::CreateContext(g_pd3dDevice);
        if (!m_Texture2DRW_CS)
        {
            printf("Could not create m_Texture2DRW_CS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture2DRW_PS)
    {
        Texture2DRW_PS::Context::LogFn = &LogFunction;
        Texture2DRW_PS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture2DRW_PS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture2DRW_PS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture2DRW_PS\\";
        m_Texture2DRW_PS = Texture2DRW_PS::CreateContext(g_pd3dDevice);
        if (!m_Texture2DRW_PS)
        {
            printf("Could not create m_Texture2DRW_PS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture2DRW_RGS)
    {
        Texture2DRW_RGS::Context::LogFn = &LogFunction;
        Texture2DRW_RGS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture2DRW_RGS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture2DRW_RGS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture2DRW_RGS\\";
        m_Texture2DRW_RGS = Texture2DRW_RGS::CreateContext(g_pd3dDevice);
        if (!m_Texture2DRW_RGS)
        {
            printf("Could not create m_Texture2DRW_RGS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture3DRW_CS)
    {
        Texture3DRW_CS::Context::LogFn = &LogFunction;
        Texture3DRW_CS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture3DRW_CS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture3DRW_CS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture3DRW_CS\\";
        m_Texture3DRW_CS = Texture3DRW_CS::CreateContext(g_pd3dDevice);
        if (!m_Texture3DRW_CS)
        {
            printf("Could not create m_Texture3DRW_CS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture3DRW_PS)
    {
        Texture3DRW_PS::Context::LogFn = &LogFunction;
        Texture3DRW_PS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture3DRW_PS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture3DRW_PS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture3DRW_PS\\";
        m_Texture3DRW_PS = Texture3DRW_PS::CreateContext(g_pd3dDevice);
        if (!m_Texture3DRW_PS)
        {
            printf("Could not create m_Texture3DRW_PS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_Texture3DRW_RGS)
    {
        Texture3DRW_RGS::Context::LogFn = &LogFunction;
        Texture3DRW_RGS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        Texture3DRW_RGS::Context::PerfEventEndFn = &PerfEventEndFn;
        Texture3DRW_RGS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\Texture3DRW_RGS\\";
        m_Texture3DRW_RGS = Texture3DRW_RGS::CreateContext(g_pd3dDevice);
        if (!m_Texture3DRW_RGS)
        {
            printf("Could not create m_Texture3DRW_RGS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_TextureCubeRW_CS)
    {
        TextureCubeRW_CS::Context::LogFn = &LogFunction;
        TextureCubeRW_CS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        TextureCubeRW_CS::Context::PerfEventEndFn = &PerfEventEndFn;
        TextureCubeRW_CS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\TextureCubeRW_CS\\";
        m_TextureCubeRW_CS = TextureCubeRW_CS::CreateContext(g_pd3dDevice);
        if (!m_TextureCubeRW_CS)
        {
            printf("Could not create m_TextureCubeRW_CS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_TextureCubeRW_PS)
    {
        TextureCubeRW_PS::Context::LogFn = &LogFunction;
        TextureCubeRW_PS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        TextureCubeRW_PS::Context::PerfEventEndFn = &PerfEventEndFn;
        TextureCubeRW_PS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\TextureCubeRW_PS\\";
        m_TextureCubeRW_PS = TextureCubeRW_PS::CreateContext(g_pd3dDevice);
        if (!m_TextureCubeRW_PS)
        {
            printf("Could not create m_TextureCubeRW_PS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_TextureCubeRW_RGS)
    {
        TextureCubeRW_RGS::Context::LogFn = &LogFunction;
        TextureCubeRW_RGS::Context::PerfEventBeginFn = &PerfEventBeginFn;
        TextureCubeRW_RGS::Context::PerfEventEndFn = &PerfEventEndFn;
        TextureCubeRW_RGS::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\TextureCubeRW_RGS\\";
        m_TextureCubeRW_RGS = TextureCubeRW_RGS::CreateContext(g_pd3dDevice);
        if (!m_TextureCubeRW_RGS)
        {
            printf("Could not create m_TextureCubeRW_RGS context");
            return 1;
        }
    }

    if (!g_doSubsetTest || g_doTest_TextureFormats)
    {
        TextureFormats::Context::LogFn = &LogFunction;
        TextureFormats::Context::PerfEventBeginFn = &PerfEventBeginFn;
        TextureFormats::Context::PerfEventEndFn = &PerfEventEndFn;
        TextureFormats::Context::s_techniqueLocation = L".\\UnitTests\\Textures\\TextureFormats\\";
        m_TextureFormats = TextureFormats::CreateContext(g_pd3dDevice);
        if (!m_TextureFormats)
        {
            printf("Could not create m_TextureFormats context");
            return 1;
        }
    }

    // Gigi Modification End

    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Gigi Modification Begin
        #if MAKE_PIX_CAPTURE()
        // Take a pix capture every frame
        {
            static int frameIndex = 0;
            wchar_t fileName[256];

            swprintf(fileName, L"_out.%i.wpix", frameIndex);
            HRESULT hr = PIXGpuCaptureNextFrames(fileName, 1);
            frameIndex++;
        }
        #endif
        // Gigi Modification End

        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Gigi Modification Begin - UI
        if (m_BarrierTest && ImGui::CollapsingHeader("BarrierTest"))
            BarrierTest::MakeUI(m_BarrierTest, g_pd3dCommandQueue);
        if (m_buffertest && ImGui::CollapsingHeader("buffertest"))
            buffertest::MakeUI(m_buffertest, g_pd3dCommandQueue);
        if (m_StructuredBuffer && ImGui::CollapsingHeader("StructuredBuffer"))
            StructuredBuffer::MakeUI(m_StructuredBuffer, g_pd3dCommandQueue);
        if (m_boxblur && ImGui::CollapsingHeader("boxblur"))
            boxblur::MakeUI(m_boxblur, g_pd3dCommandQueue);
        if (m_IndirectDispatch && ImGui::CollapsingHeader("IndirectDispatch"))
            IndirectDispatch::MakeUI(m_IndirectDispatch, g_pd3dCommandQueue);
        if (m_ReadbackSequence && ImGui::CollapsingHeader("ReadbackSequence"))
            ReadbackSequence::MakeUI(m_ReadbackSequence, g_pd3dCommandQueue);
        if (m_simple && ImGui::CollapsingHeader("simple"))
            simple::MakeUI(m_simple, g_pd3dCommandQueue);
        if (m_SlangAutoDiff && ImGui::CollapsingHeader("SlangAutoDiff"))
            SlangAutoDiff::MakeUI(m_SlangAutoDiff, g_pd3dCommandQueue);
        if (m_CopyResourceTest && ImGui::CollapsingHeader("CopyResourceTest"))
            CopyResourceTest::MakeUI(m_CopyResourceTest, g_pd3dCommandQueue);
        if (m_CopyResourceTest_FB && ImGui::CollapsingHeader("CopyResourceTest_FB"))
            CopyResourceTest_FB::MakeUI(m_CopyResourceTest_FB, g_pd3dCommandQueue);
        if (m_Mesh && ImGui::CollapsingHeader("Mesh"))
            Mesh::MakeUI(m_Mesh, g_pd3dCommandQueue);
        if (m_MeshAmplification && ImGui::CollapsingHeader("MeshAmplification"))
            MeshAmplification::MakeUI(m_MeshAmplification, g_pd3dCommandQueue);
        if (m_MeshAmplificationLines && ImGui::CollapsingHeader("MeshAmplificationLines"))
            MeshAmplificationLines::MakeUI(m_MeshAmplificationLines, g_pd3dCommandQueue);
        if (m_NoVertex_NoIndex_NoInstance && ImGui::CollapsingHeader("NoVertex_NoIndex_NoInstance"))
            NoVertex_NoIndex_NoInstance::MakeUI(m_NoVertex_NoIndex_NoInstance, g_pd3dCommandQueue);
        if (m_simpleRaster && ImGui::CollapsingHeader("simpleRaster"))
            simpleRaster::MakeUI(m_simpleRaster, g_pd3dCommandQueue);
        if (m_simpleRaster2 && ImGui::CollapsingHeader("simpleRaster2"))
            simpleRaster2::MakeUI(m_simpleRaster2, g_pd3dCommandQueue);
        if (m_simpleRasterInSubgraph && ImGui::CollapsingHeader("simpleRasterInSubgraph"))
            simpleRasterInSubgraph::MakeUI(m_simpleRasterInSubgraph, g_pd3dCommandQueue);
        if (m_simpleRaster_Lines && ImGui::CollapsingHeader("simpleRaster_Lines"))
            simpleRaster_Lines::MakeUI(m_simpleRaster_Lines, g_pd3dCommandQueue);
        if (m_simpleRaster_Points && ImGui::CollapsingHeader("simpleRaster_Points"))
            simpleRaster_Points::MakeUI(m_simpleRaster_Points, g_pd3dCommandQueue);
        if (m_Stencil && ImGui::CollapsingHeader("Stencil"))
            Stencil::MakeUI(m_Stencil, g_pd3dCommandQueue);
        if (m_VRS && ImGui::CollapsingHeader("VRS"))
            VRS::MakeUI(m_VRS, g_pd3dCommandQueue);
        if (m_YesVertexStruct_NoIndex_NoInstance && ImGui::CollapsingHeader("YesVertexStruct_NoIndex_NoInstance"))
            YesVertexStruct_NoIndex_NoInstance::MakeUI(m_YesVertexStruct_NoIndex_NoInstance, g_pd3dCommandQueue);
        if (m_YesVertexStruct_NoIndex_YesInstanceStruct && ImGui::CollapsingHeader("YesVertexStruct_NoIndex_YesInstanceStruct"))
            YesVertexStruct_NoIndex_YesInstanceStruct::MakeUI(m_YesVertexStruct_NoIndex_YesInstanceStruct, g_pd3dCommandQueue);
        if (m_YesVertexStruct_NoIndex_YesInstanceType && ImGui::CollapsingHeader("YesVertexStruct_NoIndex_YesInstanceType"))
            YesVertexStruct_NoIndex_YesInstanceType::MakeUI(m_YesVertexStruct_NoIndex_YesInstanceType, g_pd3dCommandQueue);
        if (m_YesVertexStruct_YesIndex_NoInstance && ImGui::CollapsingHeader("YesVertexStruct_YesIndex_NoInstance"))
            YesVertexStruct_YesIndex_NoInstance::MakeUI(m_YesVertexStruct_YesIndex_NoInstance, g_pd3dCommandQueue);
        if (m_YesVertexType_NoIndex_NoInstance && ImGui::CollapsingHeader("YesVertexType_NoIndex_NoInstance"))
            YesVertexType_NoIndex_NoInstance::MakeUI(m_YesVertexType_NoIndex_NoInstance, g_pd3dCommandQueue);
        if (m_AnyHit && ImGui::CollapsingHeader("AnyHit"))
            AnyHit::MakeUI(m_AnyHit, g_pd3dCommandQueue);
        if (m_AnyHitSimple && ImGui::CollapsingHeader("AnyHitSimple"))
            AnyHitSimple::MakeUI(m_AnyHitSimple, g_pd3dCommandQueue);
        if (m_IntersectionShader && ImGui::CollapsingHeader("IntersectionShader"))
            IntersectionShader::MakeUI(m_IntersectionShader, g_pd3dCommandQueue);
        if (m_simpleRT && ImGui::CollapsingHeader("simpleRT"))
            simpleRT::MakeUI(m_simpleRT, g_pd3dCommandQueue);
        if (m_simpleRT_inline && ImGui::CollapsingHeader("simpleRT_inline"))
            simpleRT_inline::MakeUI(m_simpleRT_inline, g_pd3dCommandQueue);
        if (m_TwoRayGens && ImGui::CollapsingHeader("TwoRayGens"))
            TwoRayGens::MakeUI(m_TwoRayGens, g_pd3dCommandQueue);
        if (m_TwoRayGensSubgraph && ImGui::CollapsingHeader("TwoRayGensSubgraph"))
            TwoRayGensSubgraph::MakeUI(m_TwoRayGensSubgraph, g_pd3dCommandQueue);
        if (m_SubGraphLoops && ImGui::CollapsingHeader("SubGraphLoops"))
            SubGraphLoops::MakeUI(m_SubGraphLoops, g_pd3dCommandQueue);
        if (m_SubGraphTest && ImGui::CollapsingHeader("SubGraphTest"))
            SubGraphTest::MakeUI(m_SubGraphTest, g_pd3dCommandQueue);
        if (m_SubInSub && ImGui::CollapsingHeader("SubInSub"))
            SubInSub::MakeUI(m_SubInSub, g_pd3dCommandQueue);
        if (m_Mips_CS_2D && ImGui::CollapsingHeader("Mips_CS_2D"))
            Mips_CS_2D::MakeUI(m_Mips_CS_2D, g_pd3dCommandQueue);
        if (m_Mips_CS_2DArray && ImGui::CollapsingHeader("Mips_CS_2DArray"))
            Mips_CS_2DArray::MakeUI(m_Mips_CS_2DArray, g_pd3dCommandQueue);
        if (m_Mips_CS_3D && ImGui::CollapsingHeader("Mips_CS_3D"))
            Mips_CS_3D::MakeUI(m_Mips_CS_3D, g_pd3dCommandQueue);
        if (m_Mips_CS_Cube && ImGui::CollapsingHeader("Mips_CS_Cube"))
            Mips_CS_Cube::MakeUI(m_Mips_CS_Cube, g_pd3dCommandQueue);
        if (m_Mips_DrawCall && ImGui::CollapsingHeader("Mips_DrawCall"))
            Mips_DrawCall::MakeUI(m_Mips_DrawCall, g_pd3dCommandQueue);
        if (m_Mips_RGS_2D && ImGui::CollapsingHeader("Mips_RGS_2D"))
            Mips_RGS_2D::MakeUI(m_Mips_RGS_2D, g_pd3dCommandQueue);
        if (m_Mips_ShaderToken_2D && ImGui::CollapsingHeader("Mips_ShaderToken_2D"))
            Mips_ShaderToken_2D::MakeUI(m_Mips_ShaderToken_2D, g_pd3dCommandQueue);
        if (m_Mips_ShaderToken_2DArray && ImGui::CollapsingHeader("Mips_ShaderToken_2DArray"))
            Mips_ShaderToken_2DArray::MakeUI(m_Mips_ShaderToken_2DArray, g_pd3dCommandQueue);
        if (m_Mips_ShaderToken_3D && ImGui::CollapsingHeader("Mips_ShaderToken_3D"))
            Mips_ShaderToken_3D::MakeUI(m_Mips_ShaderToken_3D, g_pd3dCommandQueue);
        if (m_Mips_ShaderToken_Cube && ImGui::CollapsingHeader("Mips_ShaderToken_Cube"))
            Mips_ShaderToken_Cube::MakeUI(m_Mips_ShaderToken_Cube, g_pd3dCommandQueue);
        if (m_Mips_VSPS_2D && ImGui::CollapsingHeader("Mips_VSPS_2D"))
            Mips_VSPS_2D::MakeUI(m_Mips_VSPS_2D, g_pd3dCommandQueue);
        if (m_Texture2DArrayRW_CS && ImGui::CollapsingHeader("Texture2DArrayRW_CS"))
            Texture2DArrayRW_CS::MakeUI(m_Texture2DArrayRW_CS, g_pd3dCommandQueue);
        if (m_Texture2DArrayRW_PS && ImGui::CollapsingHeader("Texture2DArrayRW_PS"))
            Texture2DArrayRW_PS::MakeUI(m_Texture2DArrayRW_PS, g_pd3dCommandQueue);
        if (m_Texture2DArrayRW_RGS && ImGui::CollapsingHeader("Texture2DArrayRW_RGS"))
            Texture2DArrayRW_RGS::MakeUI(m_Texture2DArrayRW_RGS, g_pd3dCommandQueue);
        if (m_Texture2DRW_CS && ImGui::CollapsingHeader("Texture2DRW_CS"))
            Texture2DRW_CS::MakeUI(m_Texture2DRW_CS, g_pd3dCommandQueue);
        if (m_Texture2DRW_PS && ImGui::CollapsingHeader("Texture2DRW_PS"))
            Texture2DRW_PS::MakeUI(m_Texture2DRW_PS, g_pd3dCommandQueue);
        if (m_Texture2DRW_RGS && ImGui::CollapsingHeader("Texture2DRW_RGS"))
            Texture2DRW_RGS::MakeUI(m_Texture2DRW_RGS, g_pd3dCommandQueue);
        if (m_Texture3DRW_CS && ImGui::CollapsingHeader("Texture3DRW_CS"))
            Texture3DRW_CS::MakeUI(m_Texture3DRW_CS, g_pd3dCommandQueue);
        if (m_Texture3DRW_PS && ImGui::CollapsingHeader("Texture3DRW_PS"))
            Texture3DRW_PS::MakeUI(m_Texture3DRW_PS, g_pd3dCommandQueue);
        if (m_Texture3DRW_RGS && ImGui::CollapsingHeader("Texture3DRW_RGS"))
            Texture3DRW_RGS::MakeUI(m_Texture3DRW_RGS, g_pd3dCommandQueue);
        if (m_TextureCubeRW_CS && ImGui::CollapsingHeader("TextureCubeRW_CS"))
            TextureCubeRW_CS::MakeUI(m_TextureCubeRW_CS, g_pd3dCommandQueue);
        if (m_TextureCubeRW_PS && ImGui::CollapsingHeader("TextureCubeRW_PS"))
            TextureCubeRW_PS::MakeUI(m_TextureCubeRW_PS, g_pd3dCommandQueue);
        if (m_TextureCubeRW_RGS && ImGui::CollapsingHeader("TextureCubeRW_RGS"))
            TextureCubeRW_RGS::MakeUI(m_TextureCubeRW_RGS, g_pd3dCommandQueue);
        if (m_TextureFormats && ImGui::CollapsingHeader("TextureFormats"))
            TextureFormats::MakeUI(m_TextureFormats, g_pd3dCommandQueue);
        // Gigi Modification End

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        // Gigi Modification Begin
        if (show_simple_window)
        // Gigi Modification End
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();

        FrameContext* frameCtx = WaitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtx->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource   = g_mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);

        // Gigi Modification Begin
        g_readbackHelper.OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        // Gigi Modification End

        // Gigi Modification Begin - OnNewFrame and Execute
        if (m_BarrierTest)
            BarrierTest::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_buffertest)
            buffertest::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_StructuredBuffer)
            StructuredBuffer::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_boxblur)
            boxblur::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_IndirectDispatch)
            IndirectDispatch::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_ReadbackSequence)
            ReadbackSequence::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simple)
            simple::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_SlangAutoDiff)
            SlangAutoDiff::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_CopyResourceTest)
            CopyResourceTest::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_CopyResourceTest_FB)
            CopyResourceTest_FB::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mesh)
            Mesh::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_MeshAmplification)
            MeshAmplification::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_MeshAmplificationLines)
            MeshAmplificationLines::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_NoVertex_NoIndex_NoInstance)
            NoVertex_NoIndex_NoInstance::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simpleRaster)
            simpleRaster::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simpleRaster2)
            simpleRaster2::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simpleRasterInSubgraph)
            simpleRasterInSubgraph::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simpleRaster_Lines)
            simpleRaster_Lines::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simpleRaster_Points)
            simpleRaster_Points::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Stencil)
            Stencil::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_VRS)
            VRS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_YesVertexStruct_NoIndex_NoInstance)
            YesVertexStruct_NoIndex_NoInstance::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_YesVertexStruct_NoIndex_YesInstanceStruct)
            YesVertexStruct_NoIndex_YesInstanceStruct::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_YesVertexStruct_NoIndex_YesInstanceType)
            YesVertexStruct_NoIndex_YesInstanceType::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_YesVertexStruct_YesIndex_NoInstance)
            YesVertexStruct_YesIndex_NoInstance::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_YesVertexType_NoIndex_NoInstance)
            YesVertexType_NoIndex_NoInstance::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_AnyHit)
            AnyHit::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_AnyHitSimple)
            AnyHitSimple::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_IntersectionShader)
            IntersectionShader::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simpleRT)
            simpleRT::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_simpleRT_inline)
            simpleRT_inline::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_TwoRayGens)
            TwoRayGens::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_TwoRayGensSubgraph)
            TwoRayGensSubgraph::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_SubGraphLoops)
            SubGraphLoops::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_SubGraphTest)
            SubGraphTest::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_SubInSub)
            SubInSub::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_CS_2D)
            Mips_CS_2D::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_CS_2DArray)
            Mips_CS_2DArray::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_CS_3D)
            Mips_CS_3D::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_CS_Cube)
            Mips_CS_Cube::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_DrawCall)
            Mips_DrawCall::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_RGS_2D)
            Mips_RGS_2D::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_ShaderToken_2D)
            Mips_ShaderToken_2D::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_ShaderToken_2DArray)
            Mips_ShaderToken_2DArray::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_ShaderToken_3D)
            Mips_ShaderToken_3D::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_ShaderToken_Cube)
            Mips_ShaderToken_Cube::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Mips_VSPS_2D)
            Mips_VSPS_2D::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture2DArrayRW_CS)
            Texture2DArrayRW_CS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture2DArrayRW_PS)
            Texture2DArrayRW_PS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture2DArrayRW_RGS)
            Texture2DArrayRW_RGS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture2DRW_CS)
            Texture2DRW_CS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture2DRW_PS)
            Texture2DRW_PS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture2DRW_RGS)
            Texture2DRW_RGS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture3DRW_CS)
            Texture3DRW_CS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture3DRW_PS)
            Texture3DRW_PS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_Texture3DRW_RGS)
            Texture3DRW_RGS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_TextureCubeRW_CS)
            TextureCubeRW_CS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_TextureCubeRW_PS)
            TextureCubeRW_PS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_TextureCubeRW_RGS)
            TextureCubeRW_RGS::OnNewFrame(NUM_FRAMES_IN_FLIGHT);
        if (m_TextureFormats)
            TextureFormats::OnNewFrame(NUM_FRAMES_IN_FLIGHT);

        if (m_BarrierTest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_BarrierTest, UnitTestEvent::PreExecute);
        if (m_buffertest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_buffertest, UnitTestEvent::PreExecute);
        if (m_StructuredBuffer)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_StructuredBuffer, UnitTestEvent::PreExecute);
        if (m_boxblur)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_boxblur, UnitTestEvent::PreExecute);
        if (m_IndirectDispatch)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_IndirectDispatch, UnitTestEvent::PreExecute);
        if (m_ReadbackSequence)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_ReadbackSequence, UnitTestEvent::PreExecute);
        if (m_simple)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simple, UnitTestEvent::PreExecute);
        if (m_SlangAutoDiff)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SlangAutoDiff, UnitTestEvent::PreExecute);
        if (m_CopyResourceTest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_CopyResourceTest, UnitTestEvent::PreExecute);
        if (m_CopyResourceTest_FB)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_CopyResourceTest_FB, UnitTestEvent::PreExecute);
        if (m_Mesh)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mesh, UnitTestEvent::PreExecute);
        if (m_MeshAmplification)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_MeshAmplification, UnitTestEvent::PreExecute);
        if (m_MeshAmplificationLines)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_MeshAmplificationLines, UnitTestEvent::PreExecute);
        if (m_NoVertex_NoIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_NoVertex_NoIndex_NoInstance, UnitTestEvent::PreExecute);
        if (m_simpleRaster)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster, UnitTestEvent::PreExecute);
        if (m_simpleRaster2)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster2, UnitTestEvent::PreExecute);
        if (m_simpleRasterInSubgraph)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRasterInSubgraph, UnitTestEvent::PreExecute);
        if (m_simpleRaster_Lines)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster_Lines, UnitTestEvent::PreExecute);
        if (m_simpleRaster_Points)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster_Points, UnitTestEvent::PreExecute);
        if (m_Stencil)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Stencil, UnitTestEvent::PreExecute);
        if (m_VRS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_VRS, UnitTestEvent::PreExecute);
        if (m_YesVertexStruct_NoIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_NoIndex_NoInstance, UnitTestEvent::PreExecute);
        if (m_YesVertexStruct_NoIndex_YesInstanceStruct)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_NoIndex_YesInstanceStruct, UnitTestEvent::PreExecute);
        if (m_YesVertexStruct_NoIndex_YesInstanceType)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_NoIndex_YesInstanceType, UnitTestEvent::PreExecute);
        if (m_YesVertexStruct_YesIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_YesIndex_NoInstance, UnitTestEvent::PreExecute);
        if (m_YesVertexType_NoIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexType_NoIndex_NoInstance, UnitTestEvent::PreExecute);
        if (m_AnyHit)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_AnyHit, UnitTestEvent::PreExecute);
        if (m_AnyHitSimple)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_AnyHitSimple, UnitTestEvent::PreExecute);
        if (m_IntersectionShader)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_IntersectionShader, UnitTestEvent::PreExecute);
        if (m_simpleRT)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRT, UnitTestEvent::PreExecute);
        if (m_simpleRT_inline)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRT_inline, UnitTestEvent::PreExecute);
        if (m_TwoRayGens)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TwoRayGens, UnitTestEvent::PreExecute);
        if (m_TwoRayGensSubgraph)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TwoRayGensSubgraph, UnitTestEvent::PreExecute);
        if (m_SubGraphLoops)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SubGraphLoops, UnitTestEvent::PreExecute);
        if (m_SubGraphTest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SubGraphTest, UnitTestEvent::PreExecute);
        if (m_SubInSub)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SubInSub, UnitTestEvent::PreExecute);
        if (m_Mips_CS_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_2D, UnitTestEvent::PreExecute);
        if (m_Mips_CS_2DArray)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_2DArray, UnitTestEvent::PreExecute);
        if (m_Mips_CS_3D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_3D, UnitTestEvent::PreExecute);
        if (m_Mips_CS_Cube)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_Cube, UnitTestEvent::PreExecute);
        if (m_Mips_DrawCall)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_DrawCall, UnitTestEvent::PreExecute);
        if (m_Mips_RGS_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_RGS_2D, UnitTestEvent::PreExecute);
        if (m_Mips_ShaderToken_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_2D, UnitTestEvent::PreExecute);
        if (m_Mips_ShaderToken_2DArray)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_2DArray, UnitTestEvent::PreExecute);
        if (m_Mips_ShaderToken_3D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_3D, UnitTestEvent::PreExecute);
        if (m_Mips_ShaderToken_Cube)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_Cube, UnitTestEvent::PreExecute);
        if (m_Mips_VSPS_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_VSPS_2D, UnitTestEvent::PreExecute);
        if (m_Texture2DArrayRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DArrayRW_CS, UnitTestEvent::PreExecute);
        if (m_Texture2DArrayRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DArrayRW_PS, UnitTestEvent::PreExecute);
        if (m_Texture2DArrayRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DArrayRW_RGS, UnitTestEvent::PreExecute);
        if (m_Texture2DRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DRW_CS, UnitTestEvent::PreExecute);
        if (m_Texture2DRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DRW_PS, UnitTestEvent::PreExecute);
        if (m_Texture2DRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DRW_RGS, UnitTestEvent::PreExecute);
        if (m_Texture3DRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture3DRW_CS, UnitTestEvent::PreExecute);
        if (m_Texture3DRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture3DRW_PS, UnitTestEvent::PreExecute);
        if (m_Texture3DRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture3DRW_RGS, UnitTestEvent::PreExecute);
        if (m_TextureCubeRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureCubeRW_CS, UnitTestEvent::PreExecute);
        if (m_TextureCubeRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureCubeRW_PS, UnitTestEvent::PreExecute);
        if (m_TextureCubeRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureCubeRW_RGS, UnitTestEvent::PreExecute);
        if (m_TextureFormats)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureFormats, UnitTestEvent::PreExecute);

        if (m_BarrierTest)
            BarrierTest::Execute(m_BarrierTest, g_pd3dDevice, g_pd3dCommandList);
        if (m_buffertest)
            buffertest::Execute(m_buffertest, g_pd3dDevice, g_pd3dCommandList);
        if (m_StructuredBuffer)
            StructuredBuffer::Execute(m_StructuredBuffer, g_pd3dDevice, g_pd3dCommandList);
        if (m_boxblur)
            boxblur::Execute(m_boxblur, g_pd3dDevice, g_pd3dCommandList);
        if (m_IndirectDispatch)
            IndirectDispatch::Execute(m_IndirectDispatch, g_pd3dDevice, g_pd3dCommandList);
        if (m_ReadbackSequence)
            ReadbackSequence::Execute(m_ReadbackSequence, g_pd3dDevice, g_pd3dCommandList);
        if (m_simple)
            simple::Execute(m_simple, g_pd3dDevice, g_pd3dCommandList);
        if (m_SlangAutoDiff)
            SlangAutoDiff::Execute(m_SlangAutoDiff, g_pd3dDevice, g_pd3dCommandList);
        if (m_CopyResourceTest)
            CopyResourceTest::Execute(m_CopyResourceTest, g_pd3dDevice, g_pd3dCommandList);
        if (m_CopyResourceTest_FB)
            CopyResourceTest_FB::Execute(m_CopyResourceTest_FB, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mesh)
            Mesh::Execute(m_Mesh, g_pd3dDevice, g_pd3dCommandList);
        if (m_MeshAmplification)
            MeshAmplification::Execute(m_MeshAmplification, g_pd3dDevice, g_pd3dCommandList);
        if (m_MeshAmplificationLines)
            MeshAmplificationLines::Execute(m_MeshAmplificationLines, g_pd3dDevice, g_pd3dCommandList);
        if (m_NoVertex_NoIndex_NoInstance)
            NoVertex_NoIndex_NoInstance::Execute(m_NoVertex_NoIndex_NoInstance, g_pd3dDevice, g_pd3dCommandList);
        if (m_simpleRaster)
            simpleRaster::Execute(m_simpleRaster, g_pd3dDevice, g_pd3dCommandList);
        if (m_simpleRaster2)
            simpleRaster2::Execute(m_simpleRaster2, g_pd3dDevice, g_pd3dCommandList);
        if (m_simpleRasterInSubgraph)
            simpleRasterInSubgraph::Execute(m_simpleRasterInSubgraph, g_pd3dDevice, g_pd3dCommandList);
        if (m_simpleRaster_Lines)
            simpleRaster_Lines::Execute(m_simpleRaster_Lines, g_pd3dDevice, g_pd3dCommandList);
        if (m_simpleRaster_Points)
            simpleRaster_Points::Execute(m_simpleRaster_Points, g_pd3dDevice, g_pd3dCommandList);
        if (m_Stencil)
            Stencil::Execute(m_Stencil, g_pd3dDevice, g_pd3dCommandList);
        if (m_VRS)
            VRS::Execute(m_VRS, g_pd3dDevice, g_pd3dCommandList);
        if (m_YesVertexStruct_NoIndex_NoInstance)
            YesVertexStruct_NoIndex_NoInstance::Execute(m_YesVertexStruct_NoIndex_NoInstance, g_pd3dDevice, g_pd3dCommandList);
        if (m_YesVertexStruct_NoIndex_YesInstanceStruct)
            YesVertexStruct_NoIndex_YesInstanceStruct::Execute(m_YesVertexStruct_NoIndex_YesInstanceStruct, g_pd3dDevice, g_pd3dCommandList);
        if (m_YesVertexStruct_NoIndex_YesInstanceType)
            YesVertexStruct_NoIndex_YesInstanceType::Execute(m_YesVertexStruct_NoIndex_YesInstanceType, g_pd3dDevice, g_pd3dCommandList);
        if (m_YesVertexStruct_YesIndex_NoInstance)
            YesVertexStruct_YesIndex_NoInstance::Execute(m_YesVertexStruct_YesIndex_NoInstance, g_pd3dDevice, g_pd3dCommandList);
        if (m_YesVertexType_NoIndex_NoInstance)
            YesVertexType_NoIndex_NoInstance::Execute(m_YesVertexType_NoIndex_NoInstance, g_pd3dDevice, g_pd3dCommandList);
        if (m_AnyHit)
            AnyHit::Execute(m_AnyHit, g_pd3dDevice, g_pd3dCommandList);
        if (m_AnyHitSimple)
            AnyHitSimple::Execute(m_AnyHitSimple, g_pd3dDevice, g_pd3dCommandList);
        if (m_IntersectionShader)
            IntersectionShader::Execute(m_IntersectionShader, g_pd3dDevice, g_pd3dCommandList);
        if (m_simpleRT)
            simpleRT::Execute(m_simpleRT, g_pd3dDevice, g_pd3dCommandList);
        if (m_simpleRT_inline)
            simpleRT_inline::Execute(m_simpleRT_inline, g_pd3dDevice, g_pd3dCommandList);
        if (m_TwoRayGens)
            TwoRayGens::Execute(m_TwoRayGens, g_pd3dDevice, g_pd3dCommandList);
        if (m_TwoRayGensSubgraph)
            TwoRayGensSubgraph::Execute(m_TwoRayGensSubgraph, g_pd3dDevice, g_pd3dCommandList);
        if (m_SubGraphLoops)
            SubGraphLoops::Execute(m_SubGraphLoops, g_pd3dDevice, g_pd3dCommandList);
        if (m_SubGraphTest)
            SubGraphTest::Execute(m_SubGraphTest, g_pd3dDevice, g_pd3dCommandList);
        if (m_SubInSub)
            SubInSub::Execute(m_SubInSub, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_CS_2D)
            Mips_CS_2D::Execute(m_Mips_CS_2D, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_CS_2DArray)
            Mips_CS_2DArray::Execute(m_Mips_CS_2DArray, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_CS_3D)
            Mips_CS_3D::Execute(m_Mips_CS_3D, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_CS_Cube)
            Mips_CS_Cube::Execute(m_Mips_CS_Cube, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_DrawCall)
            Mips_DrawCall::Execute(m_Mips_DrawCall, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_RGS_2D)
            Mips_RGS_2D::Execute(m_Mips_RGS_2D, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_ShaderToken_2D)
            Mips_ShaderToken_2D::Execute(m_Mips_ShaderToken_2D, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_ShaderToken_2DArray)
            Mips_ShaderToken_2DArray::Execute(m_Mips_ShaderToken_2DArray, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_ShaderToken_3D)
            Mips_ShaderToken_3D::Execute(m_Mips_ShaderToken_3D, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_ShaderToken_Cube)
            Mips_ShaderToken_Cube::Execute(m_Mips_ShaderToken_Cube, g_pd3dDevice, g_pd3dCommandList);
        if (m_Mips_VSPS_2D)
            Mips_VSPS_2D::Execute(m_Mips_VSPS_2D, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture2DArrayRW_CS)
            Texture2DArrayRW_CS::Execute(m_Texture2DArrayRW_CS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture2DArrayRW_PS)
            Texture2DArrayRW_PS::Execute(m_Texture2DArrayRW_PS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture2DArrayRW_RGS)
            Texture2DArrayRW_RGS::Execute(m_Texture2DArrayRW_RGS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture2DRW_CS)
            Texture2DRW_CS::Execute(m_Texture2DRW_CS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture2DRW_PS)
            Texture2DRW_PS::Execute(m_Texture2DRW_PS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture2DRW_RGS)
            Texture2DRW_RGS::Execute(m_Texture2DRW_RGS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture3DRW_CS)
            Texture3DRW_CS::Execute(m_Texture3DRW_CS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture3DRW_PS)
            Texture3DRW_PS::Execute(m_Texture3DRW_PS, g_pd3dDevice, g_pd3dCommandList);
        if (m_Texture3DRW_RGS)
            Texture3DRW_RGS::Execute(m_Texture3DRW_RGS, g_pd3dDevice, g_pd3dCommandList);
        if (m_TextureCubeRW_CS)
            TextureCubeRW_CS::Execute(m_TextureCubeRW_CS, g_pd3dDevice, g_pd3dCommandList);
        if (m_TextureCubeRW_PS)
            TextureCubeRW_PS::Execute(m_TextureCubeRW_PS, g_pd3dDevice, g_pd3dCommandList);
        if (m_TextureCubeRW_RGS)
            TextureCubeRW_RGS::Execute(m_TextureCubeRW_RGS, g_pd3dDevice, g_pd3dCommandList);
        if (m_TextureFormats)
            TextureFormats::Execute(m_TextureFormats, g_pd3dDevice, g_pd3dCommandList);

        if (m_BarrierTest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_BarrierTest, UnitTestEvent::PostExecute);
        if (m_buffertest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_buffertest, UnitTestEvent::PostExecute);
        if (m_StructuredBuffer)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_StructuredBuffer, UnitTestEvent::PostExecute);
        if (m_boxblur)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_boxblur, UnitTestEvent::PostExecute);
        if (m_IndirectDispatch)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_IndirectDispatch, UnitTestEvent::PostExecute);
        if (m_ReadbackSequence)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_ReadbackSequence, UnitTestEvent::PostExecute);
        if (m_simple)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simple, UnitTestEvent::PostExecute);
        if (m_SlangAutoDiff)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SlangAutoDiff, UnitTestEvent::PostExecute);
        if (m_CopyResourceTest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_CopyResourceTest, UnitTestEvent::PostExecute);
        if (m_CopyResourceTest_FB)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_CopyResourceTest_FB, UnitTestEvent::PostExecute);
        if (m_Mesh)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mesh, UnitTestEvent::PostExecute);
        if (m_MeshAmplification)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_MeshAmplification, UnitTestEvent::PostExecute);
        if (m_MeshAmplificationLines)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_MeshAmplificationLines, UnitTestEvent::PostExecute);
        if (m_NoVertex_NoIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_NoVertex_NoIndex_NoInstance, UnitTestEvent::PostExecute);
        if (m_simpleRaster)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster, UnitTestEvent::PostExecute);
        if (m_simpleRaster2)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster2, UnitTestEvent::PostExecute);
        if (m_simpleRasterInSubgraph)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRasterInSubgraph, UnitTestEvent::PostExecute);
        if (m_simpleRaster_Lines)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster_Lines, UnitTestEvent::PostExecute);
        if (m_simpleRaster_Points)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRaster_Points, UnitTestEvent::PostExecute);
        if (m_Stencil)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Stencil, UnitTestEvent::PostExecute);
        if (m_VRS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_VRS, UnitTestEvent::PostExecute);
        if (m_YesVertexStruct_NoIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_NoIndex_NoInstance, UnitTestEvent::PostExecute);
        if (m_YesVertexStruct_NoIndex_YesInstanceStruct)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_NoIndex_YesInstanceStruct, UnitTestEvent::PostExecute);
        if (m_YesVertexStruct_NoIndex_YesInstanceType)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_NoIndex_YesInstanceType, UnitTestEvent::PostExecute);
        if (m_YesVertexStruct_YesIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexStruct_YesIndex_NoInstance, UnitTestEvent::PostExecute);
        if (m_YesVertexType_NoIndex_NoInstance)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_YesVertexType_NoIndex_NoInstance, UnitTestEvent::PostExecute);
        if (m_AnyHit)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_AnyHit, UnitTestEvent::PostExecute);
        if (m_AnyHitSimple)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_AnyHitSimple, UnitTestEvent::PostExecute);
        if (m_IntersectionShader)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_IntersectionShader, UnitTestEvent::PostExecute);
        if (m_simpleRT)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRT, UnitTestEvent::PostExecute);
        if (m_simpleRT_inline)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_simpleRT_inline, UnitTestEvent::PostExecute);
        if (m_TwoRayGens)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TwoRayGens, UnitTestEvent::PostExecute);
        if (m_TwoRayGensSubgraph)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TwoRayGensSubgraph, UnitTestEvent::PostExecute);
        if (m_SubGraphLoops)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SubGraphLoops, UnitTestEvent::PostExecute);
        if (m_SubGraphTest)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SubGraphTest, UnitTestEvent::PostExecute);
        if (m_SubInSub)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_SubInSub, UnitTestEvent::PostExecute);
        if (m_Mips_CS_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_2D, UnitTestEvent::PostExecute);
        if (m_Mips_CS_2DArray)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_2DArray, UnitTestEvent::PostExecute);
        if (m_Mips_CS_3D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_3D, UnitTestEvent::PostExecute);
        if (m_Mips_CS_Cube)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_CS_Cube, UnitTestEvent::PostExecute);
        if (m_Mips_DrawCall)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_DrawCall, UnitTestEvent::PostExecute);
        if (m_Mips_RGS_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_RGS_2D, UnitTestEvent::PostExecute);
        if (m_Mips_ShaderToken_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_2D, UnitTestEvent::PostExecute);
        if (m_Mips_ShaderToken_2DArray)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_2DArray, UnitTestEvent::PostExecute);
        if (m_Mips_ShaderToken_3D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_3D, UnitTestEvent::PostExecute);
        if (m_Mips_ShaderToken_Cube)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_ShaderToken_Cube, UnitTestEvent::PostExecute);
        if (m_Mips_VSPS_2D)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Mips_VSPS_2D, UnitTestEvent::PostExecute);
        if (m_Texture2DArrayRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DArrayRW_CS, UnitTestEvent::PostExecute);
        if (m_Texture2DArrayRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DArrayRW_PS, UnitTestEvent::PostExecute);
        if (m_Texture2DArrayRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DArrayRW_RGS, UnitTestEvent::PostExecute);
        if (m_Texture2DRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DRW_CS, UnitTestEvent::PostExecute);
        if (m_Texture2DRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DRW_PS, UnitTestEvent::PostExecute);
        if (m_Texture2DRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture2DRW_RGS, UnitTestEvent::PostExecute);
        if (m_Texture3DRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture3DRW_CS, UnitTestEvent::PostExecute);
        if (m_Texture3DRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture3DRW_PS, UnitTestEvent::PostExecute);
        if (m_Texture3DRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_Texture3DRW_RGS, UnitTestEvent::PostExecute);
        if (m_TextureCubeRW_CS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureCubeRW_CS, UnitTestEvent::PostExecute);
        if (m_TextureCubeRW_PS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureCubeRW_PS, UnitTestEvent::PostExecute);
        if (m_TextureCubeRW_RGS)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureCubeRW_RGS, UnitTestEvent::PostExecute);
        if (m_TextureFormats)
            UnitTest(g_pd3dDevice, g_pd3dCommandList, g_readbackHelper, m_TextureFormats, UnitTestEvent::PostExecute);
        // Gigi Modification End

        // Gigi Modification Begin
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        // Gigi Modification End

        g_pd3dCommandList->Close();

        g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

        g_pSwapChain->Present(g_vsyncOn ? 1 : 0, 0);

        UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_pd3dCommandQueue->Signal(g_fence, fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frameCtx->FenceValue = fenceValue;
    }

    WaitForLastSubmittedFrame();

    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Gigi Modification Begin
    g_readbackHelper.Release();
    // Gigi Modification End

	// Gigi Modification Begin - Destroy Contexts
    if (m_BarrierTest)
    {
        BarrierTest::DestroyContext(m_BarrierTest);
        m_BarrierTest = nullptr;
    }
    if (m_buffertest)
    {
        buffertest::DestroyContext(m_buffertest);
        m_buffertest = nullptr;
    }
    if (m_StructuredBuffer)
    {
        StructuredBuffer::DestroyContext(m_StructuredBuffer);
        m_StructuredBuffer = nullptr;
    }
    if (m_boxblur)
    {
        boxblur::DestroyContext(m_boxblur);
        m_boxblur = nullptr;
    }
    if (m_IndirectDispatch)
    {
        IndirectDispatch::DestroyContext(m_IndirectDispatch);
        m_IndirectDispatch = nullptr;
    }
    if (m_ReadbackSequence)
    {
        ReadbackSequence::DestroyContext(m_ReadbackSequence);
        m_ReadbackSequence = nullptr;
    }
    if (m_simple)
    {
        simple::DestroyContext(m_simple);
        m_simple = nullptr;
    }
    if (m_SlangAutoDiff)
    {
        SlangAutoDiff::DestroyContext(m_SlangAutoDiff);
        m_SlangAutoDiff = nullptr;
    }
    if (m_CopyResourceTest)
    {
        CopyResourceTest::DestroyContext(m_CopyResourceTest);
        m_CopyResourceTest = nullptr;
    }
    if (m_CopyResourceTest_FB)
    {
        CopyResourceTest_FB::DestroyContext(m_CopyResourceTest_FB);
        m_CopyResourceTest_FB = nullptr;
    }
    if (m_Mesh)
    {
        Mesh::DestroyContext(m_Mesh);
        m_Mesh = nullptr;
    }
    if (m_MeshAmplification)
    {
        MeshAmplification::DestroyContext(m_MeshAmplification);
        m_MeshAmplification = nullptr;
    }
    if (m_MeshAmplificationLines)
    {
        MeshAmplificationLines::DestroyContext(m_MeshAmplificationLines);
        m_MeshAmplificationLines = nullptr;
    }
    if (m_NoVertex_NoIndex_NoInstance)
    {
        NoVertex_NoIndex_NoInstance::DestroyContext(m_NoVertex_NoIndex_NoInstance);
        m_NoVertex_NoIndex_NoInstance = nullptr;
    }
    if (m_simpleRaster)
    {
        simpleRaster::DestroyContext(m_simpleRaster);
        m_simpleRaster = nullptr;
    }
    if (m_simpleRaster2)
    {
        simpleRaster2::DestroyContext(m_simpleRaster2);
        m_simpleRaster2 = nullptr;
    }
    if (m_simpleRasterInSubgraph)
    {
        simpleRasterInSubgraph::DestroyContext(m_simpleRasterInSubgraph);
        m_simpleRasterInSubgraph = nullptr;
    }
    if (m_simpleRaster_Lines)
    {
        simpleRaster_Lines::DestroyContext(m_simpleRaster_Lines);
        m_simpleRaster_Lines = nullptr;
    }
    if (m_simpleRaster_Points)
    {
        simpleRaster_Points::DestroyContext(m_simpleRaster_Points);
        m_simpleRaster_Points = nullptr;
    }
    if (m_Stencil)
    {
        Stencil::DestroyContext(m_Stencil);
        m_Stencil = nullptr;
    }
    if (m_VRS)
    {
        VRS::DestroyContext(m_VRS);
        m_VRS = nullptr;
    }
    if (m_YesVertexStruct_NoIndex_NoInstance)
    {
        YesVertexStruct_NoIndex_NoInstance::DestroyContext(m_YesVertexStruct_NoIndex_NoInstance);
        m_YesVertexStruct_NoIndex_NoInstance = nullptr;
    }
    if (m_YesVertexStruct_NoIndex_YesInstanceStruct)
    {
        YesVertexStruct_NoIndex_YesInstanceStruct::DestroyContext(m_YesVertexStruct_NoIndex_YesInstanceStruct);
        m_YesVertexStruct_NoIndex_YesInstanceStruct = nullptr;
    }
    if (m_YesVertexStruct_NoIndex_YesInstanceType)
    {
        YesVertexStruct_NoIndex_YesInstanceType::DestroyContext(m_YesVertexStruct_NoIndex_YesInstanceType);
        m_YesVertexStruct_NoIndex_YesInstanceType = nullptr;
    }
    if (m_YesVertexStruct_YesIndex_NoInstance)
    {
        YesVertexStruct_YesIndex_NoInstance::DestroyContext(m_YesVertexStruct_YesIndex_NoInstance);
        m_YesVertexStruct_YesIndex_NoInstance = nullptr;
    }
    if (m_YesVertexType_NoIndex_NoInstance)
    {
        YesVertexType_NoIndex_NoInstance::DestroyContext(m_YesVertexType_NoIndex_NoInstance);
        m_YesVertexType_NoIndex_NoInstance = nullptr;
    }
    if (m_AnyHit)
    {
        AnyHit::DestroyContext(m_AnyHit);
        m_AnyHit = nullptr;
    }
    if (m_AnyHitSimple)
    {
        AnyHitSimple::DestroyContext(m_AnyHitSimple);
        m_AnyHitSimple = nullptr;
    }
    if (m_IntersectionShader)
    {
        IntersectionShader::DestroyContext(m_IntersectionShader);
        m_IntersectionShader = nullptr;
    }
    if (m_simpleRT)
    {
        simpleRT::DestroyContext(m_simpleRT);
        m_simpleRT = nullptr;
    }
    if (m_simpleRT_inline)
    {
        simpleRT_inline::DestroyContext(m_simpleRT_inline);
        m_simpleRT_inline = nullptr;
    }
    if (m_TwoRayGens)
    {
        TwoRayGens::DestroyContext(m_TwoRayGens);
        m_TwoRayGens = nullptr;
    }
    if (m_TwoRayGensSubgraph)
    {
        TwoRayGensSubgraph::DestroyContext(m_TwoRayGensSubgraph);
        m_TwoRayGensSubgraph = nullptr;
    }
    if (m_SubGraphLoops)
    {
        SubGraphLoops::DestroyContext(m_SubGraphLoops);
        m_SubGraphLoops = nullptr;
    }
    if (m_SubGraphTest)
    {
        SubGraphTest::DestroyContext(m_SubGraphTest);
        m_SubGraphTest = nullptr;
    }
    if (m_SubInSub)
    {
        SubInSub::DestroyContext(m_SubInSub);
        m_SubInSub = nullptr;
    }
    if (m_Mips_CS_2D)
    {
        Mips_CS_2D::DestroyContext(m_Mips_CS_2D);
        m_Mips_CS_2D = nullptr;
    }
    if (m_Mips_CS_2DArray)
    {
        Mips_CS_2DArray::DestroyContext(m_Mips_CS_2DArray);
        m_Mips_CS_2DArray = nullptr;
    }
    if (m_Mips_CS_3D)
    {
        Mips_CS_3D::DestroyContext(m_Mips_CS_3D);
        m_Mips_CS_3D = nullptr;
    }
    if (m_Mips_CS_Cube)
    {
        Mips_CS_Cube::DestroyContext(m_Mips_CS_Cube);
        m_Mips_CS_Cube = nullptr;
    }
    if (m_Mips_DrawCall)
    {
        Mips_DrawCall::DestroyContext(m_Mips_DrawCall);
        m_Mips_DrawCall = nullptr;
    }
    if (m_Mips_RGS_2D)
    {
        Mips_RGS_2D::DestroyContext(m_Mips_RGS_2D);
        m_Mips_RGS_2D = nullptr;
    }
    if (m_Mips_ShaderToken_2D)
    {
        Mips_ShaderToken_2D::DestroyContext(m_Mips_ShaderToken_2D);
        m_Mips_ShaderToken_2D = nullptr;
    }
    if (m_Mips_ShaderToken_2DArray)
    {
        Mips_ShaderToken_2DArray::DestroyContext(m_Mips_ShaderToken_2DArray);
        m_Mips_ShaderToken_2DArray = nullptr;
    }
    if (m_Mips_ShaderToken_3D)
    {
        Mips_ShaderToken_3D::DestroyContext(m_Mips_ShaderToken_3D);
        m_Mips_ShaderToken_3D = nullptr;
    }
    if (m_Mips_ShaderToken_Cube)
    {
        Mips_ShaderToken_Cube::DestroyContext(m_Mips_ShaderToken_Cube);
        m_Mips_ShaderToken_Cube = nullptr;
    }
    if (m_Mips_VSPS_2D)
    {
        Mips_VSPS_2D::DestroyContext(m_Mips_VSPS_2D);
        m_Mips_VSPS_2D = nullptr;
    }
    if (m_Texture2DArrayRW_CS)
    {
        Texture2DArrayRW_CS::DestroyContext(m_Texture2DArrayRW_CS);
        m_Texture2DArrayRW_CS = nullptr;
    }
    if (m_Texture2DArrayRW_PS)
    {
        Texture2DArrayRW_PS::DestroyContext(m_Texture2DArrayRW_PS);
        m_Texture2DArrayRW_PS = nullptr;
    }
    if (m_Texture2DArrayRW_RGS)
    {
        Texture2DArrayRW_RGS::DestroyContext(m_Texture2DArrayRW_RGS);
        m_Texture2DArrayRW_RGS = nullptr;
    }
    if (m_Texture2DRW_CS)
    {
        Texture2DRW_CS::DestroyContext(m_Texture2DRW_CS);
        m_Texture2DRW_CS = nullptr;
    }
    if (m_Texture2DRW_PS)
    {
        Texture2DRW_PS::DestroyContext(m_Texture2DRW_PS);
        m_Texture2DRW_PS = nullptr;
    }
    if (m_Texture2DRW_RGS)
    {
        Texture2DRW_RGS::DestroyContext(m_Texture2DRW_RGS);
        m_Texture2DRW_RGS = nullptr;
    }
    if (m_Texture3DRW_CS)
    {
        Texture3DRW_CS::DestroyContext(m_Texture3DRW_CS);
        m_Texture3DRW_CS = nullptr;
    }
    if (m_Texture3DRW_PS)
    {
        Texture3DRW_PS::DestroyContext(m_Texture3DRW_PS);
        m_Texture3DRW_PS = nullptr;
    }
    if (m_Texture3DRW_RGS)
    {
        Texture3DRW_RGS::DestroyContext(m_Texture3DRW_RGS);
        m_Texture3DRW_RGS = nullptr;
    }
    if (m_TextureCubeRW_CS)
    {
        TextureCubeRW_CS::DestroyContext(m_TextureCubeRW_CS);
        m_TextureCubeRW_CS = nullptr;
    }
    if (m_TextureCubeRW_PS)
    {
        TextureCubeRW_PS::DestroyContext(m_TextureCubeRW_PS);
        m_TextureCubeRW_PS = nullptr;
    }
    if (m_TextureCubeRW_RGS)
    {
        TextureCubeRW_RGS::DestroyContext(m_TextureCubeRW_RGS);
        m_TextureCubeRW_RGS = nullptr;
    }
    if (m_TextureFormats)
    {
        TextureFormats::DestroyContext(m_TextureFormats);
        m_TextureFormats = nullptr;
    }
    // Gigi Modification End

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = nullptr;
    // Gigi Modification Begin
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
    {
        pdx12Debug->EnableDebugLayer();
        #if DX12_GPU_VALIDATION()
        ID3D12Debug1* pdx12Debug1 = nullptr;
        if (SUCCEEDED(pdx12Debug->QueryInterface(IID_PPV_ARGS(&pdx12Debug1))))
        {
            pdx12Debug1->SetEnableGPUBasedValidation(true);
            pdx12Debug1->Release();
        }
        #endif
    }
    // Gigi Modification End
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));

        // Gigi Modification Begin
        D3D12_MESSAGE_ID hide[] =
        {
            // Don't care about the clear value not matching the fast clear value
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
        };
        D3D12_INFO_QUEUE_FILTER filter;
        memset(&filter, 0, sizeof(filter));
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        pInfoQueue->AddStorageFilterEntries(&filter);

        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, DX12_BREAK_ON_ERROR());
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, DX12_BREAK_ON_CORRUPTION());
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, DX12_BREAK_ON_WARN());
        pInfoQueue->Release();
        pdx12Debug->Release();
        // Gigi Modification End
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
        g_pd3dCommandList->Close() != S_OK)
        return false;

    if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (g_fenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, nullptr); g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_hSwapChainWaitableObject != nullptr) { CloseHandle(g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = nullptr; }
    if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
    if (g_fence) { g_fence->Release(); g_fence = nullptr; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
}

void WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources()
{
    UINT nextFrameIndex = g_frameIndex + 1;
    g_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->FenceValue = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        waitableObjects[1] = g_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(result) && "Failed to resize swapchain.");
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
