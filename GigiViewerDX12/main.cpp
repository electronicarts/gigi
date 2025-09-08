///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS // for stbi_image_write

// =================================================
// NOTE: This is very heavily based on the imgui demo.
// =================================================

// Dear ImGui: standalone example application for DirectX 12
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important: to compile on 32-bit systems, the DirectX12 backend requires code to be compiled with '#define ImTextureID ImU64'.
// This is because we need ImTextureID to carry a 64-bit value and by default ImTextureID is defined as void*.
// This define is set in the example .vcxproj file and need to be replicated in your app or by adding it to your imconfig.h file.

// clang-format off
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>

#include "PreviewClient.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <tchar.h>
#include <string>
#include <stdarg.h>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>

#include "DX12Utils/Utils.h"
#include "DX12Utils/sRGB.h"
#include "DX12Utils/Camera.h"
#include "version.h"
#include "ImGuiHelper.h"

#include "ViewerPython.h"

#include <nfd.h>

#include "prng.h"

#include "RecentFiles.h"

#include "ImageReadback.h"
#include "ImageSave.h"
#include "BVH.h"
#include <comdef.h>
// clang-format on

#include <thread>

#ifdef _DEBUG
#define BUILD_FLAVOR "Debug"
#else
#define BUILD_FLAVOR "Release"
#endif

#define DX12_VALIDATION_ON_BY_DEFAULT() true
#define DX12_GPUVALIDATION_ON_BY_DEFAULT() false

#define DX12_BREAK_ON_WARN() false
#define DX12_BREAK_ON_CORRUPTION() false
#define DX12_BREAK_ON_ERROR() false

#define DX12_VALIDATION_LOG_ON_BY_DEFAULT() false

#define BREAK_ON_GIGI_ASSERTS() false

static const UINT D3D12SDKVersion_Preview = 717;
static const UINT D3D12SDKVersion_Retail = 616;

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12SDKVersion_Retail; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\external\\AgilitySDK\\Retail\\bin\\"; }

static const UUID ExperimentalFeaturesEnabled[] =
{
    D3D12ExperimentalShaderModels,
    D3D12CooperativeVectorExperiment
};

#include "Interpreter/GigiInterpreterPreviewWindowDX12.h"
#include "Interpreter/NodesShared.h"
#include "GigiEdit/StableSample.h"

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

#include "renderdoc_app.h"

#define ImDrawCallback_SetShaderFlags (ImDrawCallback)(-2)
#define ImDrawCallback_SetHistogramMin (ImDrawCallback)(-3)
#define ImDrawCallback_SetHistogramMax (ImDrawCallback)(-4)
static const uint64_t ImGuiShaderFlag_HideR = 1 << 0;
static const uint64_t ImGuiShaderFlag_HideG = 1 << 1;
static const uint64_t ImGuiShaderFlag_HideB = 1 << 2;
static const uint64_t ImGuiShaderFlag_HideA = 1 << 3;
static const uint64_t ImGuiShaderFlag_Checker = 1 << 4;
static const uint64_t ImGuiShaderFlag_Nearest = 1 << 5;
static const uint64_t ImGuiShaderFlag_UINTByteCountBit1 = 1 << 6;
static const uint64_t ImGuiShaderFlag_UINTByteCountBit2 = 1 << 7;
static const uint64_t ImGuiShaderFlag_UINTByteCountBit3 = 1 << 8;
static const uint64_t ImGuiShaderFlag_Signed = 1 << 9;
static const uint64_t ImGuiShaderFlag_Clamp = 1 << 10;
static const uint64_t ImGuiShaderFlag_notSRGB = 1 << 11; // show it as linear space, not sRGB

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator = nullptr;
    UINT64                  FenceValue = 0;
};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static bool g_useWarpAdapter = false;

HWND g_hwnd = NULL;
static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device14* g_pd3dDevice = NULL;
static ID3D12InfoQueue* g_pd3dInfoQueue = NULL;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
static ID3D12Fence* g_fence = NULL;
static HANDLE                       g_fenceEvent = NULL;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = NULL;
static HANDLE                       g_hSwapChainWaitableObject = NULL;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};
static DXGI_FORMAT g_swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

static const int c_imguiSRVHeapSize = 256;

static const int c_bufferRecordsPerPageHorizontal = 100;
static const int c_bufferRecordsPerPageVertical = 10;

static const float c_pi = 3.14159265359f;

static bool g_colorsShown[4] = { true, true, true, true };
static float g_histogramMinMax[2] = { 0.0f, 1.0f };
static float g_imageZoom = 1.0f;
static bool g_imageLinearFilter = true;

enum class SRGBSettings : int
{
    Auto = 0,
    On = 1,
    Off = 2
};
static SRGBSettings g_sRGB = SRGBSettings::Auto;

static bool g_hideUI = false;
static bool g_hideResourceNodes = true;  // in profiler, and render graph window. To reduce clutter of things that we don't care about.
static bool g_onlyShowWrites = true;     // Hide SRV, UAV before, etc. Only show the result of writes.

static bool g_fullscreen = false;

static std::string g_adapterName;
static std::string g_driverVersion;

static std::string g_commandLine;

static double g_startTime = 0.0f;
static int g_techniqueFrameIndex = 0;

static float g_forcedFrameDeltaTime = 0.0f;

static bool g_forceEnableProfiling = false;

static bool g_meshInfoOpenPopup = false;
static std::string g_meshInfoName = "";

static GGViewerConfig g_viewerConfig;

RecentFiles g_recentFiles("Software\\GigiViewer");
RecentFiles g_recentPythonScripts("Software\\GigiViewerPy");

// We always compile against external/AgilitySDK/Preview/include as a superset of all choices.
// The DLL's we choose at runtime can come from different places though.
enum class AgilitySDKChoice
{
    Retail,  // external/AgilitySDK/Retail/bin/
    Preview, // external/AgilitySDK/Preview/bin/
};

// RenderDoc
static HMODULE g_renderDocModule = NULL;
static RENDERDOC_API_1_6_0* g_renderDocAPI = nullptr;
static bool g_renderDocCaptureNextFrame = false;
static bool g_renderDocIsCapturing = false;
static bool g_renderDocLaunchUI = false;
static bool g_renderDocEnabled = true;
static bool g_pixCaptureEnabled = true;
static int g_renderDocFrameCaptureCount = 1;
static AgilitySDKChoice g_agilitySDKChoice = AgilitySDKChoice::Retail;

void RenderFrame(bool forceExecute);

static float Lerp(float A, float B, float t)
{
    return A * (1.0f - t) + B * t;
}

static double Lerp(double A, double B, double t)
{
    return A * (1.0 - t) + B * t;
}

static float InverseLerp(float A, float B, float v)
{
    // returns the time t of v between A and B.
    // AKA returns the percent that v is between A and B.
    return (v - A) / (B - A);
}

template <typename T>
static T Clamp(T x, T theMin, T theMax)
{
    if (x <= theMin)
        return theMin;
    else if (x >= theMax)
        return theMax;
    else
        return x;
}

void* PackFloatIntoPointer(float f)
{
    uint32_t p;
    memcpy(&p, &f, sizeof(float));
    return (void*)uint64_t(p);
}

static std::filesystem::path GetLocalAppDataPath()
{
    std::filesystem::path ret;
    wchar_t* path = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
    {
        CoTaskMemFree(path);
        MessageBoxA(nullptr, "Could not get app data local folder", "Error", MB_OK);
        return ret;
    }
    ret = std::filesystem::path(path) / "GigiViewer";
    CoTaskMemFree(path);

    return ret;
}

static void GetDefaultHistogramRange(DXGI_FORMAT format, float& typeMin, float& typeMax, bool& isIntegral, bool& enforceMinMax)
{
    if (format == DXGI_FORMAT_R11G11B10_FLOAT)
    {
        isIntegral = false;
        enforceMinMax = false;
        typeMin = 0.0f;
        typeMax = 1.0f;
        return;
    }

    DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(format);

    switch (formatInfo.normType)
    {
        case DXGI_FORMAT_Info::NormType::UNorm:
        {
            isIntegral = false;
            enforceMinMax = true;
            typeMin = 0.0f;
            typeMax = 1.0f;
            return;
        }
        case DXGI_FORMAT_Info::NormType::SNorm:
        {
            isIntegral = false;
            enforceMinMax = true;
            typeMin = -1.0f;
            typeMax = 1.0f;
            return;
        }
    }

    switch (formatInfo.channelType)
    {
        case DXGI_FORMAT_Info::ChannelType::_uint8_t:
        {
            isIntegral = true;
            enforceMinMax = true;
            typeMin = 0.0f;
            typeMax = 255.0f;
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_uint16_t:
        {
            isIntegral = true;
            enforceMinMax = true;
            typeMin = 0.0f;
            typeMax = 65535.0f;
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_uint32_t:
        {
            isIntegral = true;
            enforceMinMax = true;
            typeMin = 0.0f;
            typeMax = 4294967295.0f;
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_int8_t:
        {
            isIntegral = true;
            enforceMinMax = true;
            typeMin = -128.0f;
            typeMax = 127.0f;
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_int16_t:
        {
            isIntegral = true;
            enforceMinMax = true;
            typeMin = -32768.0f;
            typeMax = 32767.0f;
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_int32_t:
        {
            isIntegral = true;
            enforceMinMax = true;
            typeMin = -2147483648.0f;
            typeMax = 2147483647.0f;
            break;
        }
        default:
        {
            isIntegral = false;
            enforceMinMax = false;
            typeMin = 0.0f;
            typeMax = 1.0f;
            break;
        }
    }
}

static void ShowToolTip(const char* tooltip, bool delay = false)
{
	if (!tooltip || !tooltip[0])
		return;

    if (delay && GImGui->HoveredIdTimer < 0.5f) // we also could use ImGui::GetStyle().HoverStationaryDelay
        return;

	//ImGui::SameLine();
	//ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[?]");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("%s", tooltip);
}

template <typename T>
static bool ShowGigiEnumDropDown(T& value, const char* label, const char* tooltip = nullptr)
{
    bool ret = false;

    std::vector<const char*> options;
    float comboWidth = 0.0f;
    for (int i = 0; i < EnumCount<T>(); ++i)
    {
        const char* label = EnumToString((T)i);
        options.push_back(label);
        comboWidth = std::max(comboWidth, ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f);
    }
    ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);

    int valueInt = (int)value;
    if (ImGui::Combo(label, &valueInt, options.data(), (int)options.size()))
    {
        value = (T)valueInt;
        ret = true;
    }
    if (tooltip && tooltip[0])
        ShowToolTip(tooltip);
    return ret;
}

std::string GetCPUName()
{
    int CPUInfo[4] = { -1 };
    unsigned   nExIds, i = 0;
    char CPUBrandString[0x40];
    // Get the information associated with each extended ID.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    for (i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        // Interpret CPU brand string
        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
    //string includes manufacturer, model and clockspeed
    return std::string(CPUBrandString);
}

struct ResourceViewState
{
    void Texture(int _nodeIndex, int _resourceIndex, RuntimeTypes::ViewableResource::Type textureType)
    {
        StoreLast();

        type = textureType;
        nodeIndex = _nodeIndex;
        resourceIndex = _resourceIndex;
    }

    void ConstantBuffer(int _nodeIndex, int _resourceIndex)
    {
        StoreLast();

        type = RuntimeTypes::ViewableResource::Type::ConstantBuffer;
        nodeIndex = _nodeIndex;
        resourceIndex = _resourceIndex;
    }

    void Buffer(int _nodeIndex, int _resourceIndex)
    {
        StoreLast();

        type = RuntimeTypes::ViewableResource::Type::Buffer;
        nodeIndex = _nodeIndex;
        resourceIndex = _resourceIndex;
    }

    void ViewLast()
    {
        if (nodeIndex == -1 || lastNodeIndex == -1 || resourceIndex == -1 || lastResourceIndex == -1)
            return;

        std::swap(type, lastType);
        std::swap(nodeIndex, lastNodeIndex);
        std::swap(resourceIndex, lastResourceIndex);
    }

    RuntimeTypes::ViewableResource::Type type = RuntimeTypes::ViewableResource::Type::Texture2D;
    int nodeIndex = -1;
    int resourceIndex = -1;

    RuntimeTypes::ViewableResource::Type lastType = RuntimeTypes::ViewableResource::Type::Texture2D;
    int lastNodeIndex = -1;
    int lastResourceIndex = -1;

    // These are in image space (pixels of viewed image)
    int mousePosImageX = 0;
    int mousePosImageY = 0;
    int mouseClickImageX = 0;
    int mouseClickImageY = 0;

    int mouseRegionRadiusPx = 0;

    bool mouseWasDownLastFrame = false;
    float systemVarMouse[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float systemVarMouseState[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float systemVarMouseStateLastFrame[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    Camera camera;

    void StoreLast()
    {
        lastType = type;
        lastNodeIndex = nodeIndex;
        lastResourceIndex = resourceIndex;
    }
};
static ResourceViewState g_resourceView;

static GGUserFile_SystemVars g_systemVariables;
static std::vector<GGUserFileV2Snapshot> g_userSnapshots;
static int g_userSnapshotIndex = -1;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool g_isForEditor = false;
std::string g_editorIP = "";
std::string g_editorPort = "";
CPreviewClient g_previewClient;

std::string g_currentWindowTitle = "";
std::string g_renderGraphFileName = "";
bool g_saveGGUserFile = true;
bool g_disableGGUserSave = false;
GigiInterpreterPreviewWindowDX12 g_interpreter;

int g_executeTechniqueCount = 0;
int g_executeTechniqueCountRemain = 0;
bool g_executeTechnique = true;
bool g_techniquePaused = false;

bool g_logCollectedShaderAsserts = true;

uint32_t g_GPUDeviceIndex = 0;
std::string g_commandLineLoadGGFileName;
std::string g_runPyFileName;
std::vector<std::wstring> g_runPyArgs;
bool g_runPyFileAddToRecentScripts = true;

int g_syncInterval = 1;
bool g_debugLayerOn = DX12_VALIDATION_ON_BY_DEFAULT();
bool g_debugLayerShown = DX12_VALIDATION_LOG_ON_BY_DEFAULT();
bool g_GPUValidation = DX12_GPUVALIDATION_ON_BY_DEFAULT();
bool g_stablePowerState = false;
bool g_resetLayout = true;
bool g_profileMode = false;
bool g_readbackAll = false;

struct ShowWindowsState
{
    bool Log = true;
    bool ImportedResources = true;
    bool Shaders = true;
    bool SystemVariables = true;
    bool RenderGraph = true;
    bool Profiler = true;
    bool InternalVariables = false;
};
ShowWindowsState g_showWindows;

// Our state
bool show_demo_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
ImVec2 g_contentRegionSize = ImVec2(1.0f, 1.0f);

bool g_showCapsWindow = false;
bool g_showViewerSettings = false;

// first half are this frame, second half are last frame
uint8_t g_keyStates[512] = {};
uint8_t* g_keyStatesLastFrame = &g_keyStates[256];

// Portions of this software were based on https://devblogs.microsoft.com/oldnewthing/20100125-00/?p=15183
HANDLE SetClipboardDataEx(UINT uFormat, void *pvData, DWORD cbData)
{
    if (OpenClipboard(g_hwnd))
    {
        EmptyClipboard();
        if (uFormat == CF_BITMAP ||
            uFormat == CF_DSPBITMAP ||
            uFormat == CF_PALETTE ||
            uFormat == CF_METAFILEPICT ||
            uFormat == CF_DSPMETAFILEPICT ||
            uFormat == CF_ENHMETAFILE ||
            uFormat == CF_DSPENHMETAFILE ||
            uFormat == CF_OWNERDISPLAY) {
            return NULL; // these are not HGLOBAL format
        }
        HANDLE hRc = NULL;
        HGLOBAL hglob = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT,
            cbData);
        if (hglob) {
            void* pvGlob = GlobalLock(hglob);
            if (pvGlob) {
                CopyMemory(pvGlob, pvData, cbData);
                GlobalUnlock(hglob);
                hRc = SetClipboardData(uFormat, hglob);
            }
            if (!hRc) {
                DWORD blah = GetLastError();
                GlobalFree(hglob);
            }
        }
        CloseClipboard();
        return hRc;
    }
    else
        return NULL;
}

WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
void SetFullscreenMode(bool fullscreen)
{
    DWORD dwStyle = GetWindowLong(g_hwnd, GWL_STYLE);
    if (g_fullscreen)
    {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(g_hwnd, &g_wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(g_hwnd,
                MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(g_hwnd, GWL_STYLE,
                dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(g_hwnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } 
    else 
    {
        SetWindowLong(g_hwnd, GWL_STYLE,
            dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(g_hwnd, &g_wpPrev);
        SetWindowPos(g_hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

struct LogEntry
{
    //
    LogLevel level;
    //
    bool selected = false;
    // for multi line
    bool firstLine = true;
    //
    std::string msg;
};
std::vector<LogEntry> g_log;

void AddMultiLineLogEntry(LogLevel level, char* formattedMsg)
{
    std::istringstream iss(formattedMsg);

    std::string line;

    bool firstLine = true;

    while (std::getline(iss, line)) 
    {
        g_log.push_back({ level, false, firstLine, std::move(line) });
        firstLine = false;
    }
}

void Log(LogLevel level, const char* msg, ...)
{
    char formattedMsg[40960];

    va_list args;
    va_start(args, msg);
    vsnprintf_s(formattedMsg, _countof(formattedMsg)-1, msg, args);
    va_end(args);

    AddMultiLineLogEntry(level, formattedMsg);

    // tell the server about this log message
    if (g_previewClient.IsFullyConnected())
    {
        PreviewMsgClientToServer msg;
        msg._index = PreviewMsgClientToServer::c_index_log;
        msg.log.level = (PreviewMsgCS_Log_Level)level;
        msg.log.msg = formattedMsg;
        g_previewClient.Send(msg);
    }
}

void TryLoadRenderDocAPI()
{
    if (g_renderDocAPI)
    {
        return;
    }

	if (g_renderDocModule = LoadLibraryA(".\\renderdoc.dll"))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(g_renderDocModule, "RENDERDOC_GetAPI"));
		int result = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, reinterpret_cast<void**>(&g_renderDocAPI));

		if (result != 1)
		{
			g_renderDocAPI = nullptr;
			FreeLibrary(g_renderDocModule);
            g_renderDocModule = NULL;

            Log(LogLevel::Error, "Failed to get RenderDoc API!");
		}
        else
        {
            std::string filepathTemplate = g_interpreter.GetTempDirectory();
            g_renderDocAPI->SetCaptureFilePathTemplate(filepathTemplate.c_str());
            g_renderDocAPI->MaskOverlayBits(RENDERDOC_OverlayBits::eRENDERDOC_Overlay_None, 0);

            Log(LogLevel::Info, "Renderdoc loaded and capturing to \"%s\"", filepathTemplate.c_str());
        }
	}
    else
    {
		Log(LogLevel::Error, "Failed to find renderdoc.dll!");
    }
}

void TryBeginRenderDocCapture()
{
    if (!g_renderDocAPI)
    {
        return;
    }

	if (g_renderDocCaptureNextFrame)
	{
		g_renderDocIsCapturing = true;

		g_renderDocAPI->StartFrameCapture(nullptr, nullptr);
		g_renderDocFrameCaptureCount--;

		if (g_renderDocFrameCaptureCount == 0)
		{
			g_renderDocCaptureNextFrame = false;
		}
	}
}

void TryEndRenderDocCapture()
{
    if (!g_renderDocIsCapturing)
    {
        return;
    }

    uint32_t result = g_renderDocAPI->EndFrameCapture(nullptr, nullptr);

    if (g_renderDocLaunchUI)
    {
        if (g_renderDocAPI->ShowReplayUI() == 0)
        {
			g_renderDocAPI->LaunchReplayUI(1, nullptr);
        }
    }

    g_renderDocIsCapturing = false;
}

GigiInterpreterPreviewWindowDX12::ImportedResourceDesc GGUserFile_ImportedResource_To_ImportedResourceDesc(const GGUserFile_ImportedResource& inDesc, const std::filesystem::path& renderGraphDir)
{
    GigiInterpreterPreviewWindowDX12::ImportedResourceDesc outDesc;

    outDesc.isATexture = inDesc.isATexture;
    outDesc.resetEveryFrame = inDesc.resetEveryFrame;

    if (inDesc.isATexture)
    {
        if (!inDesc.texture.fileName.empty())
            outDesc.texture.fileName = (renderGraphDir / std::filesystem::path(inDesc.texture.fileName)).string();
        outDesc.texture.fileName = std::filesystem::weakly_canonical(outDesc.texture.fileName).string();

        outDesc.texture.fileIsSRGB = inDesc.texture.fileIsSRGB;
        outDesc.texture.makeMips = inDesc.texture.makeMips;
        outDesc.texture.size[0] = inDesc.texture.size[0];
        outDesc.texture.size[1] = inDesc.texture.size[1];
        outDesc.texture.size[2] = inDesc.texture.size[2];
        outDesc.texture.color[0] = inDesc.texture.color[0];
        outDesc.texture.color[1] = inDesc.texture.color[1];
        outDesc.texture.color[2] = inDesc.texture.color[2];
        outDesc.texture.color[3] = inDesc.texture.color[3];
        outDesc.texture.format = inDesc.texture.format;

        outDesc.texture.binaryDesc.size[0] = inDesc.texture.binaryDims[0];
        outDesc.texture.binaryDesc.size[1] = inDesc.texture.binaryDims[1];
        outDesc.texture.binaryDesc.size[2] = inDesc.texture.binaryDims[2];
        outDesc.texture.binaryDesc.format = inDesc.texture.binaryFormat;
    }
    else
    {
        if (!inDesc.buffer.fileName.empty())
            outDesc.buffer.fileName = (renderGraphDir / std::filesystem::path(inDesc.buffer.fileName)).string();
        outDesc.buffer.fileName = std::filesystem::weakly_canonical(outDesc.buffer.fileName).string();

        outDesc.buffer.CSVHeaderRow = inDesc.buffer.CSVHeaderRow;
        outDesc.buffer.structIndex = inDesc.buffer.structIndex;
        outDesc.buffer.type = inDesc.buffer.type;
        outDesc.buffer.count = inDesc.buffer.count;
        outDesc.buffer.RT_BuildFlags = inDesc.buffer.RT_BuildFlags;
        outDesc.buffer.BLASOpaque = inDesc.buffer.BLASOpaque;
        outDesc.buffer.BLASNoDuplicateAnyhitInvocations = inDesc.buffer.BLASNoDuplicateAnyhitInvocations;
        outDesc.buffer.BLASCullMode = inDesc.buffer.BLASCullMode;
        outDesc.buffer.IsAABBs = inDesc.buffer.IsAABBs;
        outDesc.buffer.cvData = inDesc.buffer.cvData;
        memcpy(outDesc.buffer.GeometryTransform, inDesc.buffer.GeometryTransform.data(), sizeof(float) * 16);
    }

    return outDesc;
}

GGUserFile_ImportedResource ImportedResourceDesc_To_GGUserFile_ImportedResource(const std::string& nodeName, const GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& inDesc, const std::filesystem::path& renderGraphDir)
{
    GGUserFile_ImportedResource outDesc;

    outDesc.nodeName = nodeName;
    outDesc.isATexture = inDesc.isATexture;
    outDesc.resetEveryFrame = inDesc.resetEveryFrame;

    if (inDesc.isATexture)
    {
        std::string relativeFileName = std::filesystem::relative(inDesc.texture.fileName, renderGraphDir).string();
        if (relativeFileName.empty() && !inDesc.texture.fileName.empty())
            relativeFileName = inDesc.texture.fileName;
        outDesc.texture.fileName = relativeFileName;

        outDesc.texture.fileIsSRGB = inDesc.texture.fileIsSRGB;
        outDesc.texture.makeMips = inDesc.texture.makeMips;
        outDesc.texture.size[0] = inDesc.texture.size[0];
        outDesc.texture.size[1] = inDesc.texture.size[1];
        outDesc.texture.size[2] = inDesc.texture.size[2];
        outDesc.texture.color[0] = inDesc.texture.color[0];
        outDesc.texture.color[1] = inDesc.texture.color[1];
        outDesc.texture.color[2] = inDesc.texture.color[2];
        outDesc.texture.color[3] = inDesc.texture.color[3];
        outDesc.texture.format = inDesc.texture.format;

        outDesc.texture.binaryDims[0] = inDesc.texture.binaryDesc.size[0];
        outDesc.texture.binaryDims[1] = inDesc.texture.binaryDesc.size[1];
        outDesc.texture.binaryDims[2] = inDesc.texture.binaryDesc.size[2];
        outDesc.texture.binaryFormat = inDesc.texture.binaryDesc.format;
    }
    else
    {
        std::string relativeFileName = std::filesystem::relative(inDesc.buffer.fileName, renderGraphDir).string();
        if (relativeFileName.empty() && !inDesc.buffer.fileName.empty())
            relativeFileName = inDesc.buffer.fileName;
        outDesc.buffer.fileName = relativeFileName;

        outDesc.buffer.CSVHeaderRow = inDesc.buffer.CSVHeaderRow;
        outDesc.buffer.structIndex = inDesc.buffer.structIndex;
        outDesc.buffer.type = inDesc.buffer.type;
        outDesc.buffer.count = inDesc.buffer.count;
        outDesc.buffer.RT_BuildFlags = inDesc.buffer.RT_BuildFlags;
        outDesc.buffer.BLASOpaque = inDesc.buffer.BLASOpaque;
        outDesc.buffer.BLASNoDuplicateAnyhitInvocations = inDesc.buffer.BLASNoDuplicateAnyhitInvocations;
        outDesc.buffer.BLASCullMode = inDesc.buffer.BLASCullMode;
        outDesc.buffer.IsAABBs = inDesc.buffer.IsAABBs;
        outDesc.buffer.cvData = inDesc.buffer.cvData;
        memcpy(outDesc.buffer.GeometryTransform.data(), inDesc.buffer.GeometryTransform, sizeof(float) * 16);
    }

    return outDesc;
}

void ScatterSnapshotData(const GGUserFileV2Snapshot& snapshot, bool switchingSnapshots, bool loadCamera, bool loadView, bool loadResources)
{
    if (loadCamera)
    {
        g_systemVariables.camera.cameraPos = snapshot.cameraPos;
        g_systemVariables.camera.cameraAltitudeAzimuth = snapshot.cameraAltitudeAzimuth;
        g_systemVariables.camera.cameraChanged = true;
    }

    if (loadView)
    {
        g_resourceView.type = (RuntimeTypes::ViewableResource::Type)snapshot.resourceViewType;
        g_resourceView.nodeIndex = snapshot.resourceViewNodeIndex;
        g_resourceView.resourceIndex = snapshot.resourceViewResourceIndex;
    }

    if (loadResources)
    {
        std::filesystem::path renderGraphDir = std::filesystem::path(g_renderGraphFileName).remove_filename();
        for (const GGUserFile_ImportedResource& inDesc : snapshot.importedResources)
        {
            GigiInterpreterPreviewWindowDX12::ImportedResourceDesc desc = GGUserFile_ImportedResource_To_ImportedResourceDesc(inDesc, renderGraphDir);

            if (!switchingSnapshots)
                desc.stale = true;

            desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;

            if (g_interpreter.m_importedResources.count(inDesc.nodeName) > 0)
            {
                desc.nodeIndex = g_interpreter.m_importedResources[inDesc.nodeName].nodeIndex;
                desc.resourceIndex = g_interpreter.m_importedResources[inDesc.nodeName].resourceIndex;
            }

            g_interpreter.m_importedResources[inDesc.nodeName] = desc;
        }
    }
}

void ScatterSnapshotVariables(const GGUserFileV2Snapshot& snapshot)
{
    for (const auto& savedVariable : snapshot.savedVariables)
    {
        int rtVarIndex = g_interpreter.GetRuntimeVariableIndex(savedVariable.name.c_str());
        if (rtVarIndex == -1)
            continue;
        g_interpreter.SetRuntimeVariableFromString(rtVarIndex, savedVariable.value.c_str());
    }
}

void GatherSnapshotData(GGUserFileV2Snapshot& snapshot)
{
    snapshot.resourceViewType = (int)g_resourceView.type;
    snapshot.resourceViewNodeIndex = g_resourceView.nodeIndex;
    snapshot.resourceViewResourceIndex = g_resourceView.resourceIndex;

    snapshot.cameraPos = g_systemVariables.camera.cameraPos;
    snapshot.cameraAltitudeAzimuth = g_systemVariables.camera.cameraAltitudeAzimuth;

    std::filesystem::path renderGraphDir = std::filesystem::path(g_renderGraphFileName).remove_filename();
    snapshot.importedResources.clear();
    for (const auto& it : g_interpreter.m_importedResources)
        snapshot.importedResources.push_back(ImportedResourceDesc_To_GGUserFile_ImportedResource(it.first, it.second, renderGraphDir));

    // sort the names of the imported resources so we can save them in a stable (alphabetical) order
    std::sort(snapshot.importedResources.begin(), snapshot.importedResources.end(),
        [](const GGUserFile_ImportedResource& a, const GGUserFile_ImportedResource& b)
        {
            return a.nodeName < b.nodeName;
        }
    );

    // Fill out the variables
    for (int varIndex = 0; varIndex < g_interpreter.GetRuntimeVariableCount(); ++varIndex)
    {
        const auto& rtVar = g_interpreter.GetRuntimeVariable(varIndex);

        // don't save const vars
        if (rtVar.variable->Const)
            continue;

        // don't save transient vars
        if (rtVar.variable->transient)
            continue;

        if (!rtVar.storage.overrideValue)
            continue;

        // don't save system variables
        if (rtVar.variable->name == g_systemVariables.iResolution_varName ||
            rtVar.variable->name == g_systemVariables.iTime_varName ||
            rtVar.variable->name == g_systemVariables.iTimeDelta_varName ||
            rtVar.variable->name == g_systemVariables.iFrameRate_varName ||
            rtVar.variable->name == g_systemVariables.iFrame_varName ||
            rtVar.variable->name == g_systemVariables.iMouse_varName ||
            rtVar.variable->name == g_systemVariables.MouseState_varName ||
            rtVar.variable->name == g_systemVariables.MouseStateLastFrame_varName ||
            rtVar.variable->name == g_systemVariables.ViewMtx_varName ||
            rtVar.variable->name == g_systemVariables.InvViewMtx_varName ||
            rtVar.variable->name == g_systemVariables.ProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.InvProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.ViewProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.InvViewProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.JitteredProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.InvJitteredProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.JitteredViewProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.InvJitteredViewProjMtx_varName ||
            rtVar.variable->name == g_systemVariables.CameraPos_varName ||
            rtVar.variable->name == g_systemVariables.CameraAltitudeAzimuth_varName ||
            rtVar.variable->name == g_systemVariables.CameraChanged_varName ||
            rtVar.variable->name == g_systemVariables.CameraJitter_varName ||
            rtVar.variable->name == g_systemVariables.CameraFOV_varName ||
            rtVar.variable->name == g_systemVariables.CameraNearPlane_varName ||
            rtVar.variable->name == g_systemVariables.CameraFarPlane_varName ||
            rtVar.variable->name == g_systemVariables.WindowSize_varName ||
            rtVar.variable->name == g_systemVariables.ShadingRateImageTileSize_varName)
            continue;

        GGUserFile_SavedVariable var;
        var.name = rtVar.variable->name;
        var.value = g_interpreter.GetRuntimeVariableValueAsString(varIndex);
        snapshot.savedVariables.push_back(var);
    }
}

void SaveGGUserFile()
{
    // if saving gguser files is disabled by script, don't do it
    if (g_disableGGUserSave)
        return;

    // If we didn't properly load & compile the render graph, saving the gguser file would corrupt it, so we don't want to do that
    if (!g_saveGGUserFile)
        return;

    // nothing to do if no file name
    if (g_renderGraphFileName.empty())
        return;

    // make .gguser file name
    std::string extension;
    size_t extensionStart = g_renderGraphFileName.find_last_of(".");
    if (extensionStart != std::string::npos)
        extension = g_renderGraphFileName.substr(extensionStart);

    std::string ggUserFileName;
    if (extension == ".gg")
        ggUserFileName = g_renderGraphFileName + "user";
    else
        ggUserFileName = g_renderGraphFileName + ".gguser";

    // Fill out the GGUserFile
    GGUserFileV2 ggUserData;
    GatherSnapshotData(ggUserData.snapshot);
    ggUserData.syncInterval = g_syncInterval;
    ggUserData.systemVars = g_systemVariables;
    ggUserData.snapshots = g_userSnapshots;

    // Save the data
    WriteToJSONFile(ggUserData, ggUserFileName.c_str());
}

// When we have more version we can string these conversions together
bool ConvertGGUserFile(const GGUserFileV1& oldFile, GGUserFileV2& newFile)
{
    newFile.syncInterval = oldFile.syncInterval;
    newFile.systemVars = oldFile.systemVars;

    newFile.snapshot.resourceViewType = oldFile.resourceViewType;
    newFile.snapshot.resourceViewNodeIndex = oldFile.resourceViewNodeIndex;
    newFile.snapshot.resourceViewResourceIndex = oldFile.resourceViewResourceIndex;
    newFile.snapshot.importedResources = oldFile.importedResources;
    newFile.snapshot.savedVariables = oldFile.savedVariables;

    return true;
}

GGUserFileV2 LoadGGUserFile()
{
    // nothing to do if no file name
    if (g_renderGraphFileName.empty())
        return GGUserFileV2();

    // make .gguser file name
    std::string extension;
    size_t extensionStart = g_renderGraphFileName.find_last_of(".");
    if (extensionStart != std::string::npos)
        extension = g_renderGraphFileName.substr(extensionStart);

    std::string ggUserFileName;
    if (extension == ".gg")
        ggUserFileName = g_renderGraphFileName + "user";
    else
        ggUserFileName = g_renderGraphFileName + ".gguser";

    // only try to load it if the file exists
    GGUserFileV2 ggUserData;
    FILE* file = nullptr;
    fopen_s(&file, ggUserFileName.c_str(), "rb");
    bool loadFailed = false;
    if (file)
    {
        fclose(file);
        GGUserFileVersionOnly ggUserDataVersion;
        if (ReadFromJSONFile(ggUserDataVersion, ggUserFileName.c_str()))
        {
            if (ggUserDataVersion.version == "1.0")
            {
                GGUserFileV1 ggUserDataOld;

                if (!ReadFromJSONFile(ggUserDataOld, ggUserFileName.c_str()) || !ConvertGGUserFile(ggUserDataOld, ggUserData))
                    loadFailed = true;
            }
            else if (ggUserDataVersion.version == "2.0")
            {
                if (!ReadFromJSONFile(ggUserData, ggUserFileName.c_str()))
                    loadFailed = true;
            }
            else
            {
                loadFailed = true;
            }
        }

        if (loadFailed)
            ggUserData = GGUserFileV2();
    }

    // restore the saved data
    g_interpreter.m_importedResources.clear();
    ScatterSnapshotData(ggUserData.snapshot, false, true, true, true);
    g_syncInterval = ggUserData.syncInterval;
    g_systemVariables = ggUserData.systemVars;
    g_userSnapshots = ggUserData.snapshots;
    g_userSnapshotIndex = -1;

    g_systemVariables.camera.cameraPos = g_systemVariables.camera.startingCameraPos;
    g_systemVariables.camera.cameraAltitudeAzimuth = g_systemVariables.camera.startingCameraAltitudeAzimuth;

    return ggUserData;
}

bool HandleOpenNonGGFile(const char* fileName)
{
    enum class Viewer
    {
        None,
        Texture,
        TextureNoModify, // For DDS
        Model,
        Ply,
    };

    struct ViewerMapping
    {
        const char* extension;
        Viewer viewer;
    };

    static const ViewerMapping s_mapping[] =
    {
        {".exr", Viewer::Texture},

        // stbi_image handles all these
        {".png", Viewer::Texture},
        {".hdr", Viewer::Texture},
        {".bmp", Viewer::Texture},
        {".jpg", Viewer::Texture},
        {".jpeg", Viewer::Texture},
        {".gif", Viewer::Texture},
        {".psd", Viewer::Texture},
        {".pic", Viewer::Texture},
        {".pnm", Viewer::Texture},
        {".tga", Viewer::Texture},

        {".dds", Viewer::TextureNoModify},

        {".obj", Viewer::Model},
        {".fbx", Viewer::Model},

        {".ply", Viewer::Ply},
    };

    // Figure out which viewer we should use
    Viewer viewer = Viewer::None;
    {
        std::filesystem::path p(fileName);
        std::string extension = p.extension().string();
        for (const ViewerMapping& mapping : s_mapping)
        {
            if (!_stricmp(mapping.extension, extension.c_str()))
            {
                viewer = mapping.viewer;
                break;
            }
        }

        if (viewer == Viewer::None)
            return false;
    }

    const char* folder = nullptr;
    switch (viewer)
    {
        case Viewer::Texture: folder = "Techniques/DataViewers/TextureViewer/"; break;
        case Viewer::TextureNoModify: folder = "Techniques/DataViewers/TextureViewerDDS/"; break;
        case Viewer::Model: folder = "Techniques/DataViewers/ModelViewer/"; break;
        case Viewer::Ply: folder = "Techniques/DataViewers/PlyViewer/"; break;
    }

    char currentDirectory[4096];
    GetCurrentDirectoryA(4096, currentDirectory);
    std::filesystem::path searchPath = std::filesystem::weakly_canonical(std::filesystem::path(currentDirectory) / std::filesystem::path(folder));

    char pid[256];
    sprintf_s(pid, "%i", _getpid());
    std::filesystem::path tempDir = std::filesystem::weakly_canonical(std::filesystem::temp_directory_path() / std::filesystem::path("GigiDataViewer") / std::filesystem::path(pid));

    // Remove everything aready there, to prevent stale things interfering
    // Then, make sure the directory is created
    std::filesystem::remove_all(tempDir);
    std::filesystem::create_directories(tempDir);

    std::filesystem::copy(searchPath, tempDir);

    // Run view.py, with this filename as a command line parameter
    g_runPyFileName = std::filesystem::weakly_canonical(tempDir / std::filesystem::path("view.py")).string();
    g_runPyArgs.clear();
    g_runPyArgs.push_back(ToWideString(fileName));
    g_runPyFileAddToRecentScripts = false;

    return true;
}

bool LoadGGFile(const char* fileName, bool preserveState, bool addToRecentFiles)
{
    // Handle non gg files
    {
        std::filesystem::path p(fileName);
        if (_stricmp(p.extension().string().c_str(), ".gg"))
            return HandleOpenNonGGFile(fileName);
    }

    if (addToRecentFiles)
        g_recentFiles.AddEntry(fileName);

    std::array<float, 3> cameraPos;
    std::array<float, 2> cameraAltitudeAzimuth;
    if (preserveState)
    {
        // Save camera
        cameraPos = g_systemVariables.camera.cameraPos;
        cameraAltitudeAzimuth = g_systemVariables.camera.cameraAltitudeAzimuth;
    }

    // save the old gg user file, and then load the new one after we change our file name
    SaveGGUserFile();
    g_renderGraphFileName = fileName;
    auto ggUserData = LoadGGUserFile();

    // clear if we should
    if (g_renderGraphFileName.empty())
    {
        Log(LogLevel::Info, "Interpreter cleared.\n");
        g_interpreter.Clear();
        return true;
    }

    // compile the file
    GigiCompileResult compileResult = g_interpreter.Compile(g_renderGraphFileName.c_str(), PostLoad_DX12);

    // Add the .gg file and any subgraph .gg files to the file watcher, whether or not compilation was successful.
    // Individual files (assets, shaders) are already added to the file watcher.
    // NOTE: compiling a gg file clears all file watches, so will purge any previous .gg file watching
    // watch the source gg file as well as all subgraph gg files
    if (!g_isForEditor)
    {
        g_interpreter.WatchGGFile(g_renderGraphFileName.c_str());
        for (const std::string& fileName : g_interpreter.GetRenderGraph().subGGGraphFileNames)
            g_interpreter.WatchGGFile(fileName.c_str());
    }

    // handle compilation failure and error reporting
    g_saveGGUserFile = compileResult == GigiCompileResult::OK;
    if (compileResult != GigiCompileResult::OK)
    {
        Log(LogLevel::Error, "Gigi Compile Error: %s (%i)\n", EnumToString(compileResult), (int)compileResult);
        g_interpreter.Clear();
        return false;
    }

    ScatterSnapshotVariables(ggUserData.snapshot);

    // Set all const variables to their const value
    for (const Variable& variable : g_interpreter.GetRenderGraph().variables)
    {
        if (!variable.Const)
            continue;

        int rtVarIndex = g_interpreter.GetRuntimeVariableIndex(variable.name.c_str());
        if (rtVarIndex == -1)
            continue;

        const Variable& variable = *g_interpreter.GetRuntimeVariable(rtVarIndex).variable;
        g_interpreter.SetRuntimeVariableFromString(rtVarIndex, variable.dflt.c_str());
    }

    // Reset any variables that are marked transient
    {
        int varCount = g_interpreter.GetRuntimeVariableCount();
        for (int index = 0; index < varCount; ++index)
        {
            auto& rtVar = g_interpreter.GetRuntimeVariable(index);
            if (rtVar.variable->transient)
                g_interpreter.SetRuntimeVariableToDflt(index);
        }
    }

    if (preserveState)
    {
        // Restore camera
        g_systemVariables.camera.cameraPos = cameraPos;
        g_systemVariables.camera.cameraAltitudeAzimuth = cameraAltitudeAzimuth;
    }

    Log(LogLevel::Info, "Gigi Compile Successful.\n");

    auto context = ImGui::GetCurrentContext();

    g_startTime = context->Time;
    g_techniqueFrameIndex = 0;

    OutputDebugStringA("Loaded GG File: ");
    OutputDebugStringA(fileName);
    OutputDebugStringA("\n");

    return true;
}

void ReloadGGFile(bool clearCachedFiles)
{
    if (clearCachedFiles)
        g_interpreter.ClearCachedFiles();

    LoadGGFile(g_renderGraphFileName.c_str(), true, true);
}

void OnServerMessage(const PreviewMsgSC_VersionResponse& msg)
{
    Log(LogLevel::Error, "got an unexpected VersionResponse message from the server!\n");
}

void OnServerMessage(const PreviewMsgSC_LoadGGFile& msg)
{
    Log(LogLevel::Info, "Server Message: LoadGGFile \"%s\"\n", msg.fileName.c_str());
    LoadGGFile(msg.fileName.c_str(), msg.preserveState, true);
}

void OnServerMessage(const PreviewMsg_Ping& msg)
{
    //Log(LogLevel::Info, "Ping\n");
}

void UpdateWindowTitle()
{
    std::string newWindowTitle;
    newWindowTitle = "Gigi Viewer (v" GIGI_VERSION_WITH_BUILD_NUMBER() " DX12 " BUILD_FLAVOR ")";
    if (!g_renderGraphFileName.empty())
    {
        newWindowTitle += " - ";
        newWindowTitle += g_renderGraphFileName;
    }

    if (g_currentWindowTitle == newWindowTitle)
        return;

    g_currentWindowTitle = newWindowTitle;
    SetWindowTextA(g_hwnd, g_currentWindowTitle.c_str());
}

void ImGuiRecentFiles()
{
    if (!g_recentFiles.GetEntries().empty())
    {
        if (ImGui::BeginMenu("Recent Files"))
        {
            for (const auto& el : g_recentFiles.GetEntries())
            {
                if (el.empty())
                    continue;

                if (ImGui::MenuItem(el.c_str()))
                {
                    // make a copy so we don't point to data we might change
                    std::string fileName = el;
                    LoadGGFile(fileName.c_str(), false, true);
                    break;
                }
            }
            ImGui::EndMenu();
        }
    }
}

void ImGuiRecentPythonScripts()
{
    if (!g_recentPythonScripts.GetEntries().empty())
    {
        if (ImGui::BeginMenu("Recent Python Scripts"))
        {
            for (const auto& el : g_recentPythonScripts.GetEntries())
            {
                if (el.empty())
                    continue;

                if (ImGui::MenuItem(el.c_str()))
                {
                    // make a copy so we don't point to data we might change
                    std::string fileName = el;
                    g_runPyFileName = fileName;
                    g_runPyArgs.clear();
                    break;
                }
            }
            ImGui::EndMenu();
        }
    }
}

void HandleMainMenu()
{
    if (g_hideUI)
        return;

    static bool imguiDemoOpen = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (!g_isForEditor)
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    nfdchar_t* outPath = nullptr;
                    if (NFD_OpenDialog("gg", "", &outPath) == NFD_OKAY)
                        LoadGGFile(outPath, false, true);
                }
            }

            ImGuiRecentFiles();

            ImGui::Separator();

            if (ImGui::MenuItem("Reload", "Ctrl+R"))
            {
                ReloadGGFile(false);
            }
            if (ImGui::MenuItem("Reload + Clear Cache", "Ctrl+C"))
            {
                ReloadGGFile(true);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Run Python Script"))
            {
                nfdchar_t* outPath = nullptr;
                if (NFD_OpenDialog("py", "", &outPath) == NFD_OKAY)
                {
                    g_runPyFileName = outPath;
                    g_runPyArgs.clear();
                }
            }

            ImGuiRecentPythonScripts();

#ifdef _DEBUG
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", nullptr, &imguiDemoOpen);
#endif

            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
                PostQuitMessage(0);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Hide UI", "Ctrl+U", &g_hideUI);

            if (ImGui::MenuItem("Fullscreen", "Ctrl+F", &g_fullscreen))
            {
                SetFullscreenMode(g_fullscreen);
            }

            ImGui::Separator();

            ImGui::MenuItem("Variables", "", &g_interpreter.m_showVariablesUI);
            ImGui::MenuItem("Imported Resources", "", &g_showWindows.ImportedResources);
            ImGui::MenuItem("Shaders", "", &g_showWindows.Shaders);
            ImGui::MenuItem("System Variables", "", &g_showWindows.SystemVariables);
            ImGui::MenuItem("Interpreter State", "", &g_showWindows.InternalVariables);
            ImGui::MenuItem("Log", "", &g_showWindows.Log);
            ImGui::MenuItem("Render Graph", "", &g_showWindows.RenderGraph);
            ImGui::MenuItem("Profiler", "", &g_showWindows.Profiler);

            ImGui::Separator();

            if (ImGui::MenuItem("Reset Layout", "", &g_resetLayout))
            {
                g_interpreter.m_showVariablesUI = true;
                g_showWindows = ShowWindowsState();
            }

            ImGui::Separator();

            ImGui::MenuItem("DX12 Capabilities", "", &g_showCapsWindow);

            #ifdef _DEBUG
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo Window", "", &show_demo_window);
            #endif

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginMenu("Sync Interval"))
            {
                bool selected = false;

                selected = (g_syncInterval == 0);
                if (ImGui::MenuItem("0 (VSync Off)", "", &selected))
                    g_syncInterval = 0;

                selected = (g_syncInterval == 1);
                if (ImGui::MenuItem("1 (Full FPS)", "", &selected))
                    g_syncInterval = 1;

                selected = (g_syncInterval == 2);
                if (ImGui::MenuItem("2 (1/2 FPS)", "", &selected))
                    g_syncInterval = 2;

                selected = (g_syncInterval == 3);
                if (ImGui::MenuItem("3 (1/3 FPS)", "", &selected))
                    g_syncInterval = 3;

                selected = (g_syncInterval == 4);
                if (ImGui::MenuItem("4 (1/4 FPS)", "", &selected))
                    g_syncInterval = 4;

                ImGui::EndMenu();
            }

            if (!g_debugLayerOn)
            {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }
            ImGui::MenuItem("Log DX12 Debug Layer", "", &g_debugLayerShown);
            if (!g_debugLayerOn)
            {
                ImGui::PopStyleVar();
                ImGui::PopItemFlag();
            }

            if (ImGui::MenuItem("Stable Power State", "", &g_stablePowerState))
                g_pd3dDevice->SetStablePowerState(g_stablePowerState);
            ShowToolTip("'Stable Power State' allows for more consistent GPU performance / profiling", true);

            ImGui::MenuItem("Profile mode", "", &g_profileMode);

            ImGui::MenuItem("Readback All Resources", "", &g_readbackAll);

            // Reload the file when these options are changed
            {
                if (ImGui::MenuItem("Allow Raytracing", "", &g_interpreter.m_allowRaytracing))
                    ReloadGGFile(false);

                if (ImGui::MenuItem("Compile Shaders For Debug", "", &g_interpreter.m_compileShadersForDebug))
                    ReloadGGFile(false);

                if (ImGui::MenuItem("WireFrame (DrawCalls only)", "", &g_interpreter.m_drawWireframe))
                    ReloadGGFile(false);
            }

            ImGui::MenuItem("Viewer Settings", "", &g_showViewerSettings);

            ImGui::EndMenu();
        }

        // Execution Mode
        {
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            if (g_executeTechniqueCount > 0)
            {
                if (ImGui::Button("Step"))
                    g_executeTechniqueCountRemain = g_executeTechniqueCount;

                if (g_executeTechniqueCountRemain > 0)
                {
                    g_executeTechnique = true;
                    g_executeTechniqueCountRemain--;
                }
                else
                {
                    g_executeTechnique = false;
                }
            }
            else
            {
                if (ImGui::Button(g_techniquePaused ? "Play" : "Pause"))
                {
                    g_techniquePaused = !g_techniquePaused;
                    g_executeTechnique = !g_techniquePaused;
                }
            }

            ImGui::SetNextItemWidth(50);
            ImGui::InputInt("Count", &g_executeTechniqueCount, 0);
        }

        // Pix & RenderDoc Capture
        if (g_pixCaptureEnabled || g_renderDocEnabled)
        {
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            static int captureFrames = 1;
            static std::wstring waitingToOpenFileName = L"";
            static bool openCapture = true;

            if (g_pixCaptureEnabled && ImGui::Button("Pix"))
            {
                std::filesystem::path tempDir = g_interpreter.GetTempDirectory();
                std::wstring fullFileName;
                int i = 0;
                while (1)
                {
                    char fileName[1024];
                    sprintf_s(fileName, "capture%i.wpix", i);

                    fullFileName = std::filesystem::weakly_canonical(tempDir / fileName).wstring();

                    if (!FileExists(fullFileName))
                        break;

                    i++;
                }

                HRESULT hr = PIXGpuCaptureNextFrames(fullFileName.c_str(), captureFrames);
                if (FAILED(hr))
                {
                    _com_error err(hr);
                    Log(LogLevel::Error, "Could not save pix capture to %ls:\n%s", fullFileName.c_str(), FromWideString(err.ErrorMessage()).c_str());
                }
                else
                {
                    Log(LogLevel::Info, "Pix capture saved to %ls", fullFileName.c_str());

                    if (openCapture)
                        waitingToOpenFileName = fullFileName;
                    else
                        waitingToOpenFileName = L"";
                }
            }

			if (g_renderDocEnabled && ImGui::Button("RenderDoc"))
			{
				g_renderDocCaptureNextFrame = true;
                g_renderDocFrameCaptureCount = captureFrames;
                g_renderDocLaunchUI = openCapture;

                Log(LogLevel::Info, "Renderdoc capture is being saved to %s", g_interpreter.GetTempDirectory().c_str());
			}

            ImGui::SetNextItemWidth(50);
            ImGui::InputInt("Frames", &captureFrames, 0);
            ImGui::Checkbox("Open Capture", &openCapture);

            if (!waitingToOpenFileName.empty() && FileExists(waitingToOpenFileName))
            {
                ShellExecuteW(NULL, L"open", waitingToOpenFileName.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                waitingToOpenFileName = L"";
            }
        }

        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        // Open Editor
        if (ImGui::Button("Editor"))
        {
            char commandLine[1024];
            if (g_renderGraphFileName.empty())
                sprintf_s(commandLine, "GigiEdit.exe");
            else
                sprintf_s(commandLine, "GigiEdit.exe \"%s\"", g_renderGraphFileName.c_str());

            STARTUPINFOA si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);

            PROCESS_INFORMATION pi;

            CreateProcessA(
                nullptr,
                commandLine,
                nullptr,
                nullptr,
                FALSE,
                0,
                nullptr,
                nullptr,
                &si,
                &pi);
        }

        // Open Viewer
        if (ImGui::Button("Viewer"))
        {
            STARTUPINFOA si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);

            PROCESS_INFORMATION pi;

            CreateProcessA(
                nullptr,
                (char*)"GigiViewerDX12.exe",
                nullptr,
                nullptr,
                FALSE,
                0,
                nullptr,
                nullptr,
                &si,
                &pi);
        }

        // Open Browser
        if (ImGui::Button("Browser"))
        {
            STARTUPINFOA si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);

            PROCESS_INFORMATION pi;

            CreateProcessA(
                nullptr,
                (char*)"GigiBrowser.exe",
                nullptr,
                nullptr,
                FALSE,
                0,
                nullptr,
                nullptr,
                &si,
                &pi);
        }

        // Show FPS
        {
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            ImGuiIO& io = ImGui::GetIO();
            static StableSample stableSample;
            stableSample.tick(io.DeltaTime, io.DeltaTime);
            float stableValue = stableSample.getStableAverage();
            if (stableValue != FLT_MAX)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                // option 1
//					ImGui::Text("        FPS: %.2f (%.2gms)", 1.0f / stableValue, 1000.0f * stableValue);
                    // option 2 
                ImGui::Text("FPS: %.2f (%.2g .. %.2g ms)", 1.0f / stableValue, 1000.0f * stableSample.getStableMin(), 1000.0f * stableSample.getStableMax());
                ImGui::PopStyleColor();
            }
        }

        ImGui::EndMainMenuBar();
    }

    if (imguiDemoOpen)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow(&imguiDemoOpen);
    }
}

void MakeInitialLayout(ImGuiID dockspace_id)
{
    if (!g_resetLayout)
        return;
    g_resetLayout = false;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton;

    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    ImGuiID dockspace_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
    ImGuiID dockspace_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

    ImGuiID dockspace_left_bottom = ImGui::DockBuilderSplitNode(dockspace_left, ImGuiDir_Down, 0.5f, nullptr, &dockspace_left);

    ImGuiID dockspace_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.333f, nullptr, &dockspace_id);

    ImGui::DockBuilderDockWindow("Resource View", dockspace_id);
    ImGui::DockBuilderDockWindow("Render Graph", dockspace_right);
    ImGui::DockBuilderDockWindow("Profiler", dockspace_right);
    ImGui::DockBuilderDockWindow("System Variables", dockspace_right);
    ImGui::DockBuilderDockWindow("Interpreter State", dockspace_right);
    ImGui::DockBuilderDockWindow("Imported Resources", dockspace_left_bottom);
    ImGui::DockBuilderDockWindow("Shaders", dockspace_left_bottom);
    ImGui::DockBuilderDockWindow("Variables", dockspace_left);
    ImGui::DockBuilderDockWindow("Log", dockspace_bottom);

    ImGui::DockBuilderGetNode(dockspace_id)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dockspace_right)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::DockBuilderGetNode(dockspace_left_bottom)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::DockBuilderGetNode(dockspace_left)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::DockBuilderGetNode(dockspace_bottom)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;

    ImGui::DockBuilderFinish(dockspace_id);
}

uint64_t GetImGuiImageShaderFlags(DXGI_FORMAT format, bool mainView)
{
    uint64_t flags = ImGuiShaderFlag_Checker | ImGuiShaderFlag_Clamp;

    if (!mainView || g_sRGB == SRGBSettings::Auto)
    {
        if (!Get_DXGI_FORMAT_Info(format).sRGB)
            flags |= ImGuiShaderFlag_notSRGB;
    }
    else if (g_sRGB == SRGBSettings::Off)
    {
        flags |= ImGuiShaderFlag_notSRGB;
    }

    // Depth stencil buffer formats, specifically wanting to see the stencil bits
    if (format == DXGI_FORMAT_X24_TYPELESS_G8_UINT || format == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT)
    {
        // The data is in the green channel
        flags |= ImGuiShaderFlag_HideR | ImGuiShaderFlag_HideB | ImGuiShaderFlag_HideA;

        // The texture format is 1 byte and will be between 0 and 255.
        flags |= ImGuiShaderFlag_UINTByteCountBit1;
    }
    else if (format == DXGI_FORMAT_R11G11B10_FLOAT)
    {
        flags |= ImGuiShaderFlag_HideA;
    }
    else
    {
        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(format);
        if (formatInfo.channelCount < 2)
            flags |= ImGuiShaderFlag_HideG;
        if (formatInfo.channelCount < 3)
            flags |= ImGuiShaderFlag_HideB;
        if (formatInfo.channelCount < 4)
            flags |= ImGuiShaderFlag_HideA;

        // If it isn't a normed format, we need to tell the shader how to convert the floating point value it gets into bytes
        if (formatInfo.normType == DXGI_FORMAT_Info::NormType::None)
        {
            switch (formatInfo.channelType)
            {
                case DXGI_FORMAT_Info::ChannelType::_uint8_t:
                {
                    // 1 bytes = 001
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    flags |= ImGuiShaderFlag_UINTByteCountBit1;
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_int8_t:
                {
                    // 1 bytes = 001
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    flags |= ImGuiShaderFlag_UINTByteCountBit1;

                    flags |= ImGuiShaderFlag_Signed;
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_uint16_t:
                {
                    // 2 bytes = 010
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit1;
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_int16_t:
                {
                    // 2 bytes = 010
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit1;

                    flags |= ImGuiShaderFlag_Signed;
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_uint32_t:
                {
                    // 4 bytes = 100
                    flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit1;
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_int32_t:
                {
                    // 4 bytes = 100
                    flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit2;
                    //flags |= ImGuiShaderFlag_UINTByteCountBit1;

                    flags |= ImGuiShaderFlag_Signed;
                    break;
                }
            }
        }
    }

    if (mainView)
    {
        if (!g_imageLinearFilter)
            flags |= ImGuiShaderFlag_Nearest;

        if (!g_colorsShown[0])
            flags |= ImGuiShaderFlag_HideR;
        if (!g_colorsShown[1])
            flags |= ImGuiShaderFlag_HideG;
        if (!g_colorsShown[2])
            flags |= ImGuiShaderFlag_HideB;
        if (!g_colorsShown[3])
            flags |= ImGuiShaderFlag_HideA;
    }

    return flags;
}

void ShowImageThumbnail(ID3D12Resource* resource, DXGI_FORMAT format, const int size[3], int viewableResourceNodeIndex, int viewableResourceTextureIndex, const char* label, RuntimeTypes::ViewableResource::Type textureType)
{
    if (!resource || g_profileMode)
        return;

    static const int c_thumbnailImageSize = 128;
    float scale = (float)std::max(size[0], size[1]);
    ImVec2 scaledSize(size[0] * float(c_thumbnailImageSize) / scale, size[1] * float(c_thumbnailImageSize) / scale);

    DescriptorTableCache::ResourceDescriptor desc;
    desc.m_resource = resource;
    desc.m_format = format;

    D3D12_GPU_DESCRIPTOR_HANDLE descTable;
    std::string error;
    if (g_interpreter.GetDescriptorTableCache_ImGui().GetDescriptorTable(g_pd3dDevice, g_interpreter.GetSRVHeapAllocationTracker_ImGui(), &desc, 1, descTable, error, HEAP_DEBUG_TEXT()))
    {
        DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(format);
        uint64_t shaderFlags = GetImGuiImageShaderFlags(format, false);

        // Get default histogram range, to show the image correctly in the thumbnail
        float typeHistogramMinMax[2] = { 0.0f, 1.0f };
        {
            bool isIntegral = false;
            bool enforceMinMax = false;
            GetDefaultHistogramRange(format, typeHistogramMinMax[0], typeHistogramMinMax[1], isIntegral, enforceMinMax);
        }

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetShaderFlags, (void*)(shaderFlags));
        ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMin, PackFloatIntoPointer(typeHistogramMinMax[0]));
        ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMax, PackFloatIntoPointer(typeHistogramMinMax[1]));
        if (ImGui::ImageButton((ImTextureID)descTable.ptr, scaledSize))
        {
            g_resourceView.Texture(viewableResourceNodeIndex, viewableResourceTextureIndex, textureType);
        }
        ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetShaderFlags, (void*)(0));
        ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMin, PackFloatIntoPointer(0.0f));
        ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMax, PackFloatIntoPointer(1.0f));
        if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
        {
            float my_tex_w = scaledSize[0];
            float my_tex_h = scaledSize[1];
            ImGuiIO& io = ImGui::GetIO();
            float region_sz = 32.0f;
            float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
            float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
            float zoom = 8.0f;
            if (region_x < 0.0f) { region_x = 0.0f; }
            else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
            if (region_y < 0.0f) { region_y = 0.0f; }
            else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }

            const char* typeLabel = "";
            switch (textureType)
            {
                case RuntimeTypes::ViewableResource::Type::Texture2D: typeLabel = "Texture2D"; break;
                case RuntimeTypes::ViewableResource::Type::Texture2DArray: typeLabel = "Texture2DArray"; break;
                case RuntimeTypes::ViewableResource::Type::Texture3D: typeLabel = "Texture3D"; break;
                case RuntimeTypes::ViewableResource::Type::TextureCube: typeLabel = "TextureCube"; break;
                case RuntimeTypes::ViewableResource::Type::Texture2DMS: typeLabel = "Texture2DMS"; break;
            }

            if (textureType == RuntimeTypes::ViewableResource::Type::Texture2D)
                ImGui::Text("%s\n%ix%i\n%s\n%s", label, size[0], size[1], formatInfo.name, typeLabel);
            else
                ImGui::Text("%s\n%ix%ix%i\n%s\n%s", label, size[0], size[1], size[2], formatInfo.name, typeLabel);

            // Get default histogram range, to show the image correctly in the thumbnail
            float typeHistogramMinMax[2] = { 0.0f, 1.0f };
            {
                bool isIntegral = false;
                bool enforceMinMax = false;
                GetDefaultHistogramRange(format, typeHistogramMinMax[0], typeHistogramMinMax[1], isIntegral, enforceMinMax);
            }

            ImGui::Text("(%.2f, %.2f) - (%.2f, %.2f)", region_x, region_y, region_x + region_sz, region_y + region_sz);
            ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
            ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetShaderFlags, (void*)(shaderFlags));
            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMin, PackFloatIntoPointer(typeHistogramMinMax[0]));
            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMax, PackFloatIntoPointer(typeHistogramMinMax[1]));
            ImGui::Image((ImTextureID)descTable.ptr, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetShaderFlags, (void*)(0));
            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMin, PackFloatIntoPointer(0.0f));
            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMax, PackFloatIntoPointer(1.0f));
            ImGui::EndTooltip();
        }
    }
}

void ShowVariableDropDown(const std::string& label, DataFieldType type, std::string& value)
{
    std::vector<std::string> labelsStr;
    labelsStr.push_back(value);
    labelsStr.push_back("");
    for (const Variable& variable : g_interpreter.GetRenderGraph().variables)
    {
        if (variable.type == type)// && variable.name != value)
            labelsStr.push_back(variable.name);
    }

    float comboWidth = 0.0f;
    std::vector<char> labels;
    for (std::string& s : labelsStr)
    {
        comboWidth = std::max(comboWidth, ImGui::CalcTextSize(s.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
        if (s.empty())
            labels.push_back(' ');
        for (char c : s)
            labels.push_back(c);
        labels.push_back(0);
    }
    labels.push_back(0);

    //std::string fakeLabel = std::string("##") + label;
    //ImGui::Text("%s", label.c_str());
    //ImGui::SameLine();

    int selection = 0;
    ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
    if (ImGui::Combo(label.c_str(), &selection, labels.data(), (int)labelsStr.size()))
        value = labelsStr[selection];
}

void ShowNodeDropDown(const std::string& label, int nodeType, std::string& value)
{
    const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();

    std::vector<std::string> labelsStr;
    labelsStr.push_back(value);
    labelsStr.push_back("");
    for (int nodeIndex : g_interpreter.GetRenderGraph().flattenedNodeList)
    {
        const RenderGraphNode& node = renderGraph.nodes[nodeIndex];
        if (node._index != nodeType)
            continue;

        // Get the node name
        std::string nodeName;
        switch(node._index)
        {
            #include "external/df_serialize/_common.h"
            #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) case RenderGraphNode::c_index_##_NAME: nodeName = node.##_NAME.name; break;
            // clang-format off
            #include "external/df_serialize/_fillunsetdefines.h"
            #include "Schemas/RenderGraphNodesVariant.h"
            // clang-format on
        }

        //if (nodeName == value)
            //continue;

        labelsStr.push_back(nodeName);
    }

    float comboWidth = 0.0f;
    std::vector<char> labels;
    for (std::string& s : labelsStr)
    {
        comboWidth = std::max(comboWidth, ImGui::CalcTextSize(s.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
        if (s.empty())
            labels.push_back(' ');
        for (char c : s)
            labels.push_back(c);
        labels.push_back(0);
    }
    labels.push_back(0);

    //std::string fakeLabel = std::string("##") + label;
    //ImGui::Text("%s", label.c_str());
    //ImGui::SameLine();

    int selection = 0;
    ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
    if (ImGui::Combo(label.c_str(), &selection, labels.data(), (int)labelsStr.size()))
        value = labelsStr[selection];
}

// if assignVariable is false, it will assign "value" the value from the variable
template <typename T>
bool AssignVariable(const char* name, DataFieldType type, T value)
{
    const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();

    int varIndex = -1;
    for (const auto& variable : renderGraph.variables)
    {
        varIndex++;
        if (varIndex >= g_interpreter.GetRuntimeVariableCount())
            break;

        if (variable.name == name && variable.type == type)
        {
            auto& rtVar = g_interpreter.GetRuntimeVariable(varIndex);

//            Variable& nonConstVar = (Variable&)(rtVar.variable);
//            nonConstVar.system = true;
            rtVar.storage.systemValue = true;

            memcpy(rtVar.storage.value, &value, rtVar.storage.size);
            return true;
        }
    }

    return false;
}

// fill out the key states using the keys defined in the viewer config
static Camera::KeyStates MakeCameraKeyStates()
{
    Camera::KeyStates ret;

    auto IsLetter = [](const std::string& s) -> bool
    {
        return (s.length() == 1 && s[0] >= 'A' && s[0] <= 'Z');
    };

    auto KeyIsPressed = [&](const std::string& key) -> bool
    {
        if (IsLetter(key))
            return g_keyStates[key[0]];

        if (key == "Shift")
            return g_keyStates[VK_SHIFT];

        if (key == "Control")
            return g_keyStates[VK_CONTROL];

        return false;
    };

    ret.forward = KeyIsPressed(g_viewerConfig.keyCameraForward) || (g_keyStates[VK_UP] != 0);
    ret.back = KeyIsPressed(g_viewerConfig.keyCameraBackward) || (g_keyStates[VK_DOWN] != 0);
    ret.left = KeyIsPressed(g_viewerConfig.keyCameraLeft) || (g_keyStates[VK_LEFT] != 0);
    ret.right = KeyIsPressed(g_viewerConfig.keyCameraRight) || (g_keyStates[VK_RIGHT] != 0);
    ret.up = KeyIsPressed(g_viewerConfig.keyCameraUp) || (g_keyStates[VK_PRIOR] != 0);
    ret.down = KeyIsPressed(g_viewerConfig.keyCameraDown) || (g_keyStates[VK_NEXT] != 0);
    ret.fast = KeyIsPressed(g_viewerConfig.keyCameraFast);
    ret.slow = KeyIsPressed(g_viewerConfig.keyCameraSlow);

    return ret;
}

void SynchronizeSystemVariables()
{
    auto context = ImGui::GetCurrentContext();

    // Resolution
    {
        bool exists = false;
        auto rtTex = g_interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(g_systemVariables.iResolution_textureName.c_str(), exists);
        if (exists)
        {
            struct Res
            {
                float resolution[3];
            };
            Res resolution;
            resolution.resolution[0] = (float)rtTex.m_size[0];
            resolution.resolution[1] = (float)rtTex.m_size[1];
            resolution.resolution[2] = (float)rtTex.m_size[2];
            AssignVariable(g_systemVariables.iResolution_varName.c_str(), DataFieldType::Float3, resolution);
        }
    }

    // WindowSize, set to the texture preview region size in this Viewer
    {
        struct Size
        {
            float size[2];
        };
        Size size;

        size.size[0] = g_contentRegionSize.x;
        size.size[1] = g_contentRegionSize.y;
        AssignVariable(g_systemVariables.WindowSize_varName.c_str(), DataFieldType::Float2, size);
    }

    // Time
    AssignVariable(g_systemVariables.iTime_varName.c_str(), DataFieldType::Float, (float)(context->Time - g_startTime));

    // Frame Delta
    {
        if (g_forcedFrameDeltaTime > 0.0f)
        {
            AssignVariable(g_systemVariables.iTimeDelta_varName.c_str(), DataFieldType::Float, g_forcedFrameDeltaTime);
            AssignVariable(g_systemVariables.iFrameRate_varName.c_str(), DataFieldType::Float, 1.0f / g_forcedFrameDeltaTime);
        }
        else
        {
            AssignVariable(g_systemVariables.iTimeDelta_varName.c_str(), DataFieldType::Float, context->IO.DeltaTime);
            AssignVariable(g_systemVariables.iFrameRate_varName.c_str(), DataFieldType::Float, context->IO.Framerate);
        }
    }

    // Frame Index
    AssignVariable(g_systemVariables.iFrame_varName.c_str(), DataFieldType::Int, g_techniqueFrameIndex);

    // Mouse
    {
        struct Mouse
        {
            float mouse[4];
        };
        Mouse mouse;

        // iMouse
        mouse.mouse[0] = g_resourceView.systemVarMouse[0];
        mouse.mouse[1] = g_resourceView.systemVarMouse[1];
        mouse.mouse[2] = g_resourceView.systemVarMouse[2];
        mouse.mouse[3] = g_resourceView.systemVarMouse[3];
        AssignVariable(g_systemVariables.iMouse_varName.c_str(), DataFieldType::Float4, mouse);

        // Mouse State
        mouse.mouse[0] = g_resourceView.systemVarMouseState[0];
        mouse.mouse[1] = g_resourceView.systemVarMouseState[1];
        mouse.mouse[2] = g_resourceView.systemVarMouseState[2];
        mouse.mouse[3] = g_resourceView.systemVarMouseState[3];
        AssignVariable(g_systemVariables.MouseState_varName.c_str(), DataFieldType::Float4, mouse);

        // Mouse State Last Frame
        mouse.mouse[0] = g_resourceView.systemVarMouseStateLastFrame[0];
        mouse.mouse[1] = g_resourceView.systemVarMouseStateLastFrame[1];
        mouse.mouse[2] = g_resourceView.systemVarMouseStateLastFrame[2];
        mouse.mouse[3] = g_resourceView.systemVarMouseStateLastFrame[3];
        AssignVariable(g_systemVariables.MouseStateLastFrame_varName.c_str(), DataFieldType::Float4, mouse);
    }

    // Camera Logic
    {
        g_resourceView.camera.m_flySpeed = g_systemVariables.camera.flySpeed;
        g_resourceView.camera.m_leftHanded = g_systemVariables.camera.leftHanded;
        g_resourceView.camera.m_mouseSensitivity = g_systemVariables.camera.mouseSensitivity;
        g_resourceView.camera.Update(MakeCameraKeyStates(), g_resourceView.systemVarMouseState, g_resourceView.systemVarMouseStateLastFrame, ImGui::GetIO().DeltaTime, g_systemVariables.camera.cameraPos.data(), g_systemVariables.camera.cameraAltitudeAzimuth.data(), g_systemVariables.camera.cameraChanged);

        AssignVariable(g_systemVariables.CameraChanged_varName.c_str(), DataFieldType::Bool, g_systemVariables.camera.cameraChanged);
        g_systemVariables.camera.cameraChanged = false;
    }

    // Assign camera system variables
    {
        // Make the projection and jittered projection matrix
        DirectX::XMMATRIX projMtx, jitteredProjMtx;
        Vec2 jitter;
        {
            // Get the resolution if we can
            float resolution[2] = { 1, 1 };
            {
                bool exists = false;
                auto rtTex = g_interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(g_systemVariables.ProjMtx_textureName.c_str(), exists);
                if (exists)
                {
                    resolution[0] = (float)rtTex.m_size[0];
                    resolution[1] = (float)rtTex.m_size[1];
                }
            }

            // Get the projection matrix
            projMtx = g_resourceView.camera.GetProjMatrix(g_systemVariables.camera.FOV, resolution, g_systemVariables.camera.nearPlane, g_systemVariables.camera.farPlane, g_systemVariables.camera.reverseZ, g_systemVariables.camera.perspective);

            // Subpixel jitter
            jitteredProjMtx = projMtx;
            {
                jitter[0] = jitter[1] = 0.5f;
                int jitterIndex = g_techniqueFrameIndex;
                if (g_systemVariables.camera.jitterLength > 0)
                    jitterIndex = jitterIndex % g_systemVariables.camera.jitterLength;
                switch(g_systemVariables.camera.jitterType)
                {
                    case GGUserFile_CameraJitterType::None:
                    {
                        jitter[0] = jitter[1] = 0.5f;
                        break;
                    }
                    case GGUserFile_CameraJitterType::UniformWhite:
                    {
                        uint32_t rng = wang_hash_init(jitterIndex, 53637, 1927349);
                        jitter[0] = wang_hash_float01(rng);
                        jitter[1] = wang_hash_float01(rng);
                        break;
                    }
                    case GGUserFile_CameraJitterType::Halton23:
                    {
                        // jitterIndex + 1 to avoid the (0,0) at index 0
                        jitter[0] = halton(jitterIndex + 1, 2);
                        jitter[1] = halton(jitterIndex + 1, 3);
                        break;
                    }
                }

                jitter[0] -= 0.5f;
                jitter[1] -= 0.5f;

                jitter[0] /= resolution[0];
                jitter[1] /= resolution[1];

                // Times 2 because the projection matrix maps on screen points to be between -1 and +1, not 0 to 1, so the pixels are twice as big for jitter purposes.
                jitteredProjMtx.r[2].m128_f32[0] += jitter[0] * 2.0f;
                jitteredProjMtx.r[2].m128_f32[1] += jitter[1] * 2.0f;
            }
        }

        {
            DirectX::XMMATRIX invProjMtx = DirectX::XMMatrixInverse(nullptr, projMtx);
            DirectX::XMMATRIX invJitteredProjMtx = DirectX::XMMatrixInverse(nullptr, jitteredProjMtx);

            DirectX::XMMATRIX viewMtx = g_resourceView.camera.GetViewMatrix(g_systemVariables.camera.cameraPos.data(), g_systemVariables.camera.cameraAltitudeAzimuth.data());
            DirectX::XMMATRIX invViewMtx = DirectX::XMMatrixInverse(nullptr, viewMtx);

            DirectX::XMMATRIX viewProjMtx = viewMtx * projMtx;
            DirectX::XMMATRIX invViewProjMtx = XMMatrixInverse(nullptr, viewProjMtx);

            DirectX::XMMATRIX jitteredViewProjMtx = viewMtx * jitteredProjMtx;
            DirectX::XMMATRIX invJitteredViewProjMtx = XMMatrixInverse(nullptr, jitteredViewProjMtx);

            // Camera Matrices
            {
                AssignVariable(g_systemVariables.ViewMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(viewMtx));
                AssignVariable(g_systemVariables.InvViewMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(invViewMtx));

                AssignVariable(g_systemVariables.ProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(projMtx));
                AssignVariable(g_systemVariables.InvProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(invProjMtx));

                AssignVariable(g_systemVariables.JitteredProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(jitteredProjMtx));
                AssignVariable(g_systemVariables.InvJitteredProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(invJitteredProjMtx));

                AssignVariable(g_systemVariables.ViewProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(viewProjMtx));
                AssignVariable(g_systemVariables.InvViewProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(invViewProjMtx));

                AssignVariable(g_systemVariables.JitteredViewProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(jitteredViewProjMtx));
                AssignVariable(g_systemVariables.InvJitteredViewProjMtx_varName.c_str(), DataFieldType::Float4x4, XMMatrixTranspose(invJitteredViewProjMtx));
            }

            // Camera Position and altitude azimuth
            // Variables read/write
            AssignVariable(g_systemVariables.CameraPos_varName.c_str(), DataFieldType::Float3, g_systemVariables.camera.cameraPos);
            AssignVariable(g_systemVariables.CameraAltitudeAzimuth_varName.c_str(), DataFieldType::Float2, g_systemVariables.camera.cameraAltitudeAzimuth);

            // Camera FOV, near plance and far plane
            AssignVariable(g_systemVariables.CameraFOV_varName.c_str(), DataFieldType::Float, g_systemVariables.camera.FOV);
            AssignVariable(g_systemVariables.CameraNearPlane_varName.c_str(), DataFieldType::Float, g_systemVariables.camera.nearPlane);
            AssignVariable(g_systemVariables.CameraFarPlane_varName.c_str(), DataFieldType::Float, g_systemVariables.camera.farPlane);

            // camera jitter
            AssignVariable(g_systemVariables.CameraJitter_varName.c_str(), DataFieldType::Float2, jitter);

            // Shading rate image tile size
            AssignVariable(g_systemVariables.ShadingRateImageTileSize_varName.c_str(), DataFieldType::Uint, g_interpreter.GetOptions6().ShadingRateImageTileSize);
        }
    }

    // Clear all onUserChanged variables
    const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
    for (const Variable& variable : renderGraph.variables)
    {
        if (variable.onUserChange.variableIndex == -1)
            continue;

        AssignVariable(variable.onUserChange.name.c_str(), DataFieldType::Bool, false);
    }
}

bool DropDownBoolean(const char* label, const char* option1, const char* option2, bool& b)
{
    const char* items[] = { option1, option2 };

    int selected = b ? 1 : 0;
    if (ImGui::Combo(label, &selected, items, 2))
    {
        b = (selected == 1);
        return true;
    }

    return false;
}

template <typename ENUM>
bool DropDownEnum(const char* label, ENUM& e)
{
    size_t count = EnumCount<ENUM>();
    std::vector<const char*> items(count);
    for (size_t i = 0; i < count; ++i)
        items[i] = EnumToString((ENUM)i);

    int value = (int)e;
    if (ImGui::Combo(label, &value, items.data(), (int)count))
    {
        e = (ENUM)value;
        return true;
    }

    return false;
}

struct timer
{
    unsigned long long elapsed_millisecs() const
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now() - start_time).count();
    }

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
};

void ShowInternalVariables()
{
    if (!g_showWindows.InternalVariables || g_hideUI)
        return;

    if (!ImGui::Begin("Interpreter State", &g_showWindows.InternalVariables))
    {
        ImGui::End();
        return;
    }

    //hold values as they don't change that often to reduce CPU usage when vieweing interpreter states (internal variables)
    static timer t; // note: static
    static size_t c_texture_cache_val; //m_textures
    static size_t c_obj_cache_val; //m_objs
    static size_t c_fbx_cache_val; //m_fbxs
    static size_t c_ply_cache_val; //m_plys
    static size_t c_upload_buffer_in_use_val; //m_uploadBufferTracker
    static size_t c_upload_buffer_free_size_val; //m_uploadBufferTracker
    static size_t c_file_watcher_val; //m_fileWatcher
    static size_t c_delayed_release_tracker_val; //m_delayedReleaseTracker
    static size_t c_descriptor_table_cache_val; //m_descriptorTableCache
    static size_t c_srv_heap_allocation_tracker_val; //m_SRVHeapAllocationTracker
    static size_t c_rtv_heap_allocation_tracker_val; //m_RTVHeapAllocationTracker
    static size_t c_dsv_heap_allocation_tracker_val; //m_DSVHeapAllocationTracker
    static size_t c_file_cache_val; //m_files
    static float file_total_size; //m_files

    const int interval_millisecs_delay = 3000; // x seconds to refresh
    const auto elapsed = t.elapsed_millisecs();
    const bool has_elapsed = elapsed >= interval_millisecs_delay;

    if (c_texture_cache_val <= 0 || has_elapsed) {
        c_texture_cache_val = g_interpreter.getTextureCache().getCache().size();
    }
    if (c_obj_cache_val <= 0 || has_elapsed) {
        c_obj_cache_val = g_interpreter.getObjCache().getCache().size();
    }
    if (c_fbx_cache_val <= 0 || has_elapsed) {
        c_fbx_cache_val = g_interpreter.getFBXCache().getCache().size();
    }
    if (c_ply_cache_val <= 0 || has_elapsed) {
        c_ply_cache_val = g_interpreter.getPLYCache().getCache().size();
    }
    if (c_file_cache_val <= 0 || has_elapsed) {
        c_file_cache_val = g_interpreter.getFileCache().getCache().size();
        file_total_size = 0;
        for (auto const& ent1 : g_interpreter.getFileCache().getCache()) {
            //auto const& key = ent1.first;
            auto const& val = ent1.second;
            file_total_size += val.GetSize();
        }
    }
    if (c_upload_buffer_in_use_val <= 0 || has_elapsed) {
        c_upload_buffer_in_use_val = g_interpreter.getUploadBufferTracker().getInUseSize();
        c_upload_buffer_free_size_val = g_interpreter.getUploadBufferTracker().getFreeSize();
    }
    if (c_file_watcher_val <= 0 || has_elapsed) {
        c_file_watcher_val = g_interpreter.getFileWatcher().getTrackedFiles().size();
    }
    if (c_delayed_release_tracker_val <= 0 || has_elapsed) {
        c_delayed_release_tracker_val = g_interpreter.getDelayedReleaseTracker().getObjectSize();
    }
    if (c_descriptor_table_cache_val <= 0 || has_elapsed) {
        c_descriptor_table_cache_val = g_interpreter.getDescriptorTableCache().getCacheSize();
    }
    if (c_srv_heap_allocation_tracker_val <= 0 || has_elapsed) {
        c_srv_heap_allocation_tracker_val = g_interpreter.getSRVHeapAllocationTracker().AllocatedCount();
    }
    if (c_rtv_heap_allocation_tracker_val <= 0 || has_elapsed) {
        c_rtv_heap_allocation_tracker_val = g_interpreter.getRTVHeapAllocationTracker().AllocatedCount();
    }
    if (c_dsv_heap_allocation_tracker_val <= 0 || has_elapsed) {
        c_dsv_heap_allocation_tracker_val = g_interpreter.getDSVHeapAllocationTracker().AllocatedCount();
    }

    if (ImGui::BeginTable("internal stats", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Upload Buffer (m_uploadBufferTracker)");
        ImGui::TableNextColumn();
        ImGui::Text("In use: %d, free: %d", c_upload_buffer_in_use_val, c_upload_buffer_free_size_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Texture Cache (m_textures)");
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_texture_cache_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Object Cache (m_objs)");
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_obj_cache_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("FBX Cache (m_fbxs)");
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_fbx_cache_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("PLY Cache (m_plys)");
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_ply_cache_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("File Cache (m_files)");
        ImGui::TableNextColumn();        
        ImGui::Text("files:%d, total_size: %3.0f", c_file_cache_val, file_total_size);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Tracked Files (m_fileWatcher)");
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_file_watcher_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("m_delayedReleaseTracker");
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_delayed_release_tracker_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("m_descriptorTableCache");
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_descriptor_table_cache_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("m_SRVHeapAllocationTracker"); //Shader Resource View
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_srv_heap_allocation_tracker_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("m_RTVHeapAllocationTracker"); //Render Target View
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_rtv_heap_allocation_tracker_val);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("m_DSVHeapAllocationTracker"); //Depth Stencil View
        ImGui::TableNextColumn();
        ImGui::Text("%d", c_dsv_heap_allocation_tracker_val);
        ImGui::EndTable();
    }

    if (ImGui::CollapsingHeader("File Cache"))
    {
        //hold values as they don't change that often to reduce CPU usage when vieweing interpreter states (internal variables)
        static std::vector<std::string> c_file_cache_details_file_display;
        static std::vector<size_t> c_file_cache_details_size;
        if (has_elapsed || c_file_cache_details_file_display.size() == 0) {
            c_file_cache_details_file_display = {};
            c_file_cache_details_size = {};
            for (auto const& ent1 : g_interpreter.getFileCache().getCache()) {
                auto const& key = ent1.first;
                auto const& val = ent1.second;
                std::string base_filename = key.substr(key.find_last_of("/\\") + 1);
                const char* for_display = base_filename.c_str();              
                c_file_cache_details_file_display.push_back(for_display);
                c_file_cache_details_size.push_back(val.GetSize());
            }
        }
        if (ImGui::BeginTable("file cache details", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            for (int rowCtr = 0; rowCtr < c_file_cache_val; rowCtr++) {               
                ImGui::Text("%d", rowCtr+1);
                ImGui::TableNextColumn();
                const char* full_display = c_file_cache_details_file_display[rowCtr].c_str();
                ImGui::TextUnformatted(full_display);
                ImGui::TableNextColumn();
                ImGui::Text("%d", c_file_cache_details_size[rowCtr]);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("Tracked Files"))
    {
        static int filterSelected = -1;
        if (ImGui::BeginCombo("Type", filterSelected == -1 ? "All" : EnumToString((GigiInterpreterPreviewWindowDX12::FileWatchOwner)filterSelected)))
        {
            if (ImGui::Selectable("All", filterSelected == -1))
                filterSelected = -1;

            for (int typeIndex = 0; typeIndex < (int)GigiInterpreterPreviewWindowDX12::FileWatchOwner::Count; ++typeIndex)
            {
                if (ImGui::Selectable(EnumToString((GigiInterpreterPreviewWindowDX12::FileWatchOwner)typeIndex), filterSelected == typeIndex))
                    filterSelected = typeIndex;
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginTable("tracked files details", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
        	ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            int index = 0;
            for (auto const& entry : g_interpreter.getFileWatcher().getTrackedFiles())
            {
                auto const& key = entry.first;
                auto const& val = entry.second;

                if (filterSelected != -1 && (int)val.data != filterSelected)
                    continue;

                ImGui::Text("%d", index);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(key.substr(key.find_last_of("/\\") + 1).c_str());
                ShowToolTip(key.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(EnumToString(val.data));
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                index++;
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("OBJ Cache"))
    {
        //hold values as they don't change that often to reduce CPU usage when vieweing interpreter states (internal variables)
        static std::vector<std::string> c_obj_cache_details_file_display;
        static std::vector<size_t> c_obj_cache_details_shapes;
        static std::vector<size_t> c_obj_cache_details_materials;
        static std::vector<size_t> c_obj_cache_details_vertices;
        if (has_elapsed || c_obj_cache_details_file_display.size() == 0) {
            c_obj_cache_details_file_display = {};
            c_obj_cache_details_shapes = {};
            c_obj_cache_details_materials = {};
            c_obj_cache_details_vertices = {};
            for (auto const& ent1 : g_interpreter.getObjCache().getCache()) {
                auto const& key = ent1.first;
                auto const& val = ent1.second;
                std::string base_filename = key.substr(key.find_last_of("/\\") + 1);
                const char* for_display = base_filename.c_str();
                c_obj_cache_details_file_display.push_back(for_display);
                c_obj_cache_details_shapes.push_back(val.shapes.size());
                c_obj_cache_details_materials.push_back(val.materials.size());
                c_obj_cache_details_vertices.push_back(val.flattenedVertices.size());
            }
        }
        if (ImGui::BeginTable("object cache details", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
        	ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Shapes", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Materials", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Flattened Vertices", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            for (int rowCtr = 0; rowCtr < c_obj_cache_val; rowCtr++) {
                ImGui::Text("%d", rowCtr + 1);
                ImGui::TableNextColumn();
                const char* full_display = c_obj_cache_details_file_display[rowCtr].c_str();
                ImGui::TextUnformatted(full_display);
                ImGui::TableNextColumn();
                ImGui::Text("%d", c_obj_cache_details_shapes[rowCtr]);
                ImGui::TableNextColumn();
                ImGui::Text("%d", c_obj_cache_details_materials[rowCtr]);
                ImGui::TableNextColumn();
                ImGui::Text("%d", c_obj_cache_details_vertices[rowCtr]);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("FBX Cache"))
    {
        //hold values as they don't change that often to reduce CPU usage when vieweing interpreter states (internal variables)
        static std::vector<std::string> c_fbx_cache_details_file_display;
        static std::vector<size_t> c_fbx_cache_details_vertices;
        if (has_elapsed || c_fbx_cache_details_file_display.size() == 0) {
            c_fbx_cache_details_file_display = {};
            c_fbx_cache_details_vertices = {};
            for (auto const& ent1 : g_interpreter.getFBXCache().getCache()) {
                auto const& key = ent1.first;
                auto const& val = ent1.second;
                std::string base_filename = key.substr(key.find_last_of("/\\") + 1);
                const char* for_display = base_filename.c_str();
                c_fbx_cache_details_file_display.push_back(for_display);
                c_fbx_cache_details_vertices.push_back(val.flattenedVertices.size());
            }
        }
        if (ImGui::BeginTable("FBX cache details", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Flattened Vertices", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            int rowCtr = 0;
            for (int rowCtr = 0; rowCtr < c_fbx_cache_val; rowCtr++) {
                ImGui::Text("%d", rowCtr + 1);
                ImGui::TableNextColumn();
                const char* full_display = c_fbx_cache_details_file_display[rowCtr].c_str();
                ImGui::TextUnformatted(full_display);
                ImGui::TableNextColumn();
                ImGui::Text("%d", c_fbx_cache_details_vertices[rowCtr]);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("PLY Cache"))
    {
        //hold values as they don't change that often to reduce CPU usage when vieweing interpreter states (internal variables)
        static std::vector<std::string> c_ply_cache_details_file_display;
        static std::vector<size_t> c_ply_cache_details_vertices;
        if (has_elapsed || c_ply_cache_details_file_display.size() == 0) {
            c_ply_cache_details_file_display = {};
            c_ply_cache_details_vertices = {};
            for (auto const& ent1 : g_interpreter.getPLYCache().getCache()) {
                auto const& key = ent1.first;
                auto const& val = ent1.second;
                std::string base_filename = key.substr(key.find_last_of("/\\") + 1);
                const char* for_display = base_filename.c_str();
                c_ply_cache_details_file_display.push_back(for_display);
                unsigned int total = 0;
                for (const auto& elementGroup : val.elementGroups)
                    total += elementGroup.count;
                c_ply_cache_details_vertices.push_back(total);
            }
        }
        if (ImGui::BeginTable("PLY cache details", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Flattened Vertices", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            int rowCtr = 0;
            for (int rowCtr = 0; rowCtr < c_ply_cache_val; rowCtr++) {
                ImGui::Text("%d", rowCtr + 1);
                ImGui::TableNextColumn();
                const char* full_display = c_ply_cache_details_file_display[rowCtr].c_str();
                ImGui::TextUnformatted(full_display);
                ImGui::TableNextColumn();
                ImGui::Text("%d", c_ply_cache_details_vertices[rowCtr]);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("Texture Cache"))
    {
        //hold values as they don't change that often to reduce CPU usage when vieweing interpreter states (internal variables)
        static std::vector<std::string> c_texture_cache_details_file_display;
        static std::vector<size_t> c_texture_cache_details_pixels;
        if (has_elapsed || c_texture_cache_details_file_display.size() == 0) {
            c_texture_cache_details_file_display = {};
            c_texture_cache_details_pixels = {};
            for (auto const& ent1 : g_interpreter.getTextureCache().getCache()) {
                auto const& key = ent1.first;
                auto const& val = ent1.second;
                std::string base_filename = key.substr(key.find_last_of("/\\") + 1);
                const char* for_display = base_filename.c_str();
                c_texture_cache_details_file_display.push_back(for_display);
                c_texture_cache_details_pixels.push_back(val.PixelCount());
            }
        }
        if (ImGui::BeginTable("Texture cache details", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Pixels", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            for (int rowCtr = 0; rowCtr < c_texture_cache_val; rowCtr++) {
                ImGui::Text("%d", rowCtr + 1);
                ImGui::TableNextColumn();
                const char* full_display = c_texture_cache_details_file_display[rowCtr].c_str();
                ImGui::TextUnformatted(full_display);
                ImGui::TableNextColumn();
                ImGui::Text("%d", c_texture_cache_details_pixels[rowCtr]);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }

    if (has_elapsed) {
        t = {};
    }

    ImGui::End();
}

void ShowSystemVariables()
{
    if (!g_showWindows.SystemVariables || g_hideUI)
        return;

    if (!ImGui::Begin("System Variables", &g_showWindows.SystemVariables))
    {
        ImGui::End();
        return;
    }

    // Standard variables
    {
        ImGui::SeparatorText("Mouse and Keyboard");

        ShowVariableDropDown("Mouse State", DataFieldType::Float4, g_systemVariables.MouseState_varName);
        ShowToolTip("Float4: xy is mouse position. z is left mouse button state. w is right mouse button state.");

        ShowVariableDropDown("Mouse State Last Frame", DataFieldType::Float4, g_systemVariables.MouseStateLastFrame_varName);
        ShowToolTip("Float4: Last Frame Mouse State. xy is mouse position. z is left mouse button state. w is right mouse button state.");

        ShowNodeDropDown("Key States Buffer", RenderGraphNode::c_index_resourceBuffer, g_systemVariables.KeyState_bufferName);
        ShowToolTip("A buffer of 512 UINT32s.");
    }

    // Shadertoy variables
    {
        ImGui::SeparatorText("Same behaviors as shadertoy");

        ShowVariableDropDown("##Resolution Var", DataFieldType::Float3, g_systemVariables.iResolution_varName);
        ImGui::SameLine();
        ShowNodeDropDown("Resolution", RenderGraphNode::c_index_resourceTexture, g_systemVariables.iResolution_textureName);
        ShowToolTip("Float3: resolution in pixels. The texture resource node to use as the source of the resolution is required as well.");

        ShowVariableDropDown("WindowSize", DataFieldType::Float2, g_systemVariables.WindowSize_varName);
        ShowToolTip("Float2: window size in pixels. In this viewer, the size of the texture preview region.");

        ShowVariableDropDown("Time", DataFieldType::Float, g_systemVariables.iTime_varName);
        ShowToolTip("Float: in seconds");

        ShowVariableDropDown("Frame Time", DataFieldType::Float, g_systemVariables.iTimeDelta_varName);
        ShowToolTip("Float: time since last frame, in seconds");

        ShowVariableDropDown("FPS", DataFieldType::Float, g_systemVariables.iFrameRate_varName);
        ShowToolTip("Float: how many frames are rendered each second");

        ShowVariableDropDown("Frame", DataFieldType::Int, g_systemVariables.iFrame_varName);
        ShowToolTip("Int: The current frame number");

        ShowVariableDropDown("Mouse", DataFieldType::Float4, g_systemVariables.iMouse_varName);
        ShowToolTip("Float4: xy is where user last clicked.  zw is the click location for a single frame when the user clicks, else is zero.");
    }

    // other variables
    {
        ImGui::SeparatorText("Misc Variables");

        ShowVariableDropDown("VRS Image Tile Size", DataFieldType::Uint, g_systemVariables.ShadingRateImageTileSize_varName);
    }

    // Camera variables
    {
        ImGui::SeparatorText("Camera Variables");

        ShowVariableDropDown("CameraPos", DataFieldType::Float3, g_systemVariables.CameraPos_varName);
        ShowVariableDropDown("CameraAltitudeAzimuth", DataFieldType::Float2, g_systemVariables.CameraAltitudeAzimuth_varName);
        ShowVariableDropDown("Camera Changed", DataFieldType::Bool, g_systemVariables.CameraChanged_varName);

        ShowVariableDropDown("View Matrix", DataFieldType::Float4x4, g_systemVariables.ViewMtx_varName);
        ShowVariableDropDown("Inverse View Matrix", DataFieldType::Float4x4, g_systemVariables.InvViewMtx_varName);

        ShowVariableDropDown("Projection Matrix", DataFieldType::Float4x4, g_systemVariables.ProjMtx_varName);
        ShowVariableDropDown("Inverse Projection Matrix", DataFieldType::Float4x4, g_systemVariables.InvProjMtx_varName);

        ShowVariableDropDown("View Projection Matrix", DataFieldType::Float4x4, g_systemVariables.ViewProjMtx_varName);
        ShowVariableDropDown("Inverse View Projection Matrix", DataFieldType::Float4x4, g_systemVariables.InvViewProjMtx_varName);

        ShowVariableDropDown("Jittered Projection Matrix", DataFieldType::Float4x4, g_systemVariables.JitteredProjMtx_varName);
        ShowVariableDropDown("Inverse Jittered Projection Matrix", DataFieldType::Float4x4, g_systemVariables.InvJitteredProjMtx_varName);

        ShowVariableDropDown("Jittered View Projection Matrix", DataFieldType::Float4x4, g_systemVariables.JitteredViewProjMtx_varName);
        ShowVariableDropDown("Inverse Jittered View Projection Matrix", DataFieldType::Float4x4, g_systemVariables.InvJitteredViewProjMtx_varName);

        ShowVariableDropDown("Camera Jitter", DataFieldType::Float2, g_systemVariables.CameraJitter_varName);
        ShowVariableDropDown("Camera FOV", DataFieldType::Float, g_systemVariables.CameraFOV_varName);
        ShowVariableDropDown("Camera Near Z", DataFieldType::Float, g_systemVariables.CameraNearPlane_varName);
        ShowVariableDropDown("Camera Far Z", DataFieldType::Float, g_systemVariables.CameraFarPlane_varName);
    }

    // Camera settings
    {
        ImGui::SeparatorText("Camera Settings");

        ShowNodeDropDown("Proj Mtx Texture", RenderGraphNode::c_index_resourceTexture, g_systemVariables.ProjMtx_textureName);
        ShowToolTip("The projection matrix needs a resolution. Choose a texture to use as a source of that resolution.");
        DropDownBoolean("##Projective", "Orthographic", "Perspective", g_systemVariables.camera.perspective);
        DropDownBoolean("##LeftHanded", "Right Handed", "Left Handed", g_systemVariables.camera.leftHanded);
        DropDownBoolean("##ReverseZ", "Normal Z", "Reversed Z", g_systemVariables.camera.reverseZ);
        ImGui::InputFloat("Near Z", &g_systemVariables.camera.nearPlane);
        ImGui::InputFloat("Far Z", &g_systemVariables.camera.farPlane);
        ImGui::InputFloat("FOV", &g_systemVariables.camera.FOV);
        ShowToolTip("Vertical FOV, in degrees");

        DropDownEnum("Jitter Sequence", g_systemVariables.camera.jitterType);
        ShowToolTip("The sequence used for the jittered projection matrix.");
        ImGui::InputInt("Jitter Sequence Length", &g_systemVariables.camera.jitterLength);
        ShowToolTip("The length of the sequence used for the jittered projection matrix. 0 for infinite.");

        ImGui::InputFloat("Fly Speed", &g_systemVariables.camera.flySpeed);
        ImGui::InputFloat("Mouse Sensitivity", &g_systemVariables.camera.mouseSensitivity);

        ImGui::InputFloat3("Start Camera Pos", &g_systemVariables.camera.startingCameraPos[0]);
        ImGui::InputFloat2("Start Altitude Azimuth", &g_systemVariables.camera.startingCameraAltitudeAzimuth[0]);

        if (ImGui::Button("Reset Camera"))
        {
            g_systemVariables.camera.cameraPos = g_systemVariables.camera.startingCameraPos;
            g_systemVariables.camera.cameraAltitudeAzimuth = g_systemVariables.camera.startingCameraAltitudeAzimuth;
            g_systemVariables.camera.cameraChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Set Starting Camera"))
        {
            g_systemVariables.camera.startingCameraPos = g_systemVariables.camera.cameraPos;
            g_systemVariables.camera.startingCameraAltitudeAzimuth = g_systemVariables.camera.cameraAltitudeAzimuth;
        }

        ImGui::InputFloat3("Camera Pos", &g_systemVariables.camera.cameraPos[0]);
        ImGui::InputFloat2("Altitude Azimuth", &g_systemVariables.camera.cameraAltitudeAzimuth[0]);
    }

    ImGui::End();
}

void ShowShaders()
{
    if (!g_showWindows.Shaders || g_hideUI)
        return;

    if (!ImGui::Begin("Shaders", &g_showWindows.Shaders))
    {
        ImGui::End();
        return;
    }

    const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();

	ImGui::Text("Shaders:");
	ImGui::PushID("Shaders:");
	ImGui::Indent();

	// get a sorted list of scopes
	std::vector<std::string> scopesSorted;
	{
		// find unique scopes
		std::unordered_set<std::string> scopes;
		for (const Shader& shader : renderGraph.shaders)
			scopes.insert(shader.scope);

		// sort
		for (const std::string& scope : scopes)
			scopesSorted.push_back(scope);
		std::sort(scopesSorted.begin(), scopesSorted.end());
	}

	// The list of unique shader file copies (shader includes)
	std::map<std::string, const FileCopy*> uniqueShaderFileCopies;
	for (const FileCopy& fileCopy : renderGraph.fileCopies)
	{
		if (fileCopy.type != FileCopyType::Shader)
			continue;
		uniqueShaderFileCopies.insert({ fileCopy.fileName, &fileCopy });
	}

	for (const std::string& scope : scopesSorted)
	{
		// get a sorted list of shaders in this scope
		std::vector<const Shader*> sortedShaders;
		for (const Shader& shader : renderGraph.shaders)
		{
			if (shader.scope == scope)
				sortedShaders.push_back(&shader);
		}
		std::sort(sortedShaders.begin(), sortedShaders.end(),
			[](const Shader* A, const Shader* B)
			{
				return A->originalName < B->originalName;
			}
		);
		if (sortedShaders.size() == 0)
			continue;

		if (!scope.empty())
		{
			std::string scopeLabel = scope.substr(0, scope.length() - 1);
			if (!ImGui::CollapsingHeader(scopeLabel.c_str()))
				continue;

			ImGui::Indent();
			ImGui::PushID(scope.c_str());
		}

		if (ImGui::BeginTable("ShadersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
		{
			for (const Shader* shader : sortedShaders)
			{
				const char* name = shader->originalName.c_str();
				ImGui::PushID(name);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
                if (ImGui::SmallButton("Source"))
                {
                    std::string fileName = shader->fileName;
                    if (std::filesystem::path(fileName).is_relative())
                        fileName = std::filesystem::weakly_canonical(renderGraph.baseDirectory + fileName).string();
                    ShellExecuteA(NULL, "open", fileName.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                }
				ImGui::TableSetColumnIndex(1);
                if (ImGui::SmallButton("Post"))
                {
                    std::string fileName = shader->destFileName.empty() ? shader->fileName : shader->destFileName;
                    if (std::filesystem::path(fileName).is_relative())
                        fileName = std::filesystem::weakly_canonical(g_interpreter.GetTempDirectory() + "shaders\\" + fileName).string();
                    ShellExecuteA(NULL, "open", fileName.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                }
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(EnumToString(shader->type));
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%s", name);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("fileName: \"%s\"\n\ndestFileName: \"%s\"",
						shader->fileName.c_str(),
						shader->destFileName.c_str());

				ImGui::PopID();
			}

			for (const auto& el : uniqueShaderFileCopies)
			{
				const char* name = el.first.c_str();
				const FileCopy& fileCopy = *el.second;

				ImGui::PushID(name);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
                if (ImGui::SmallButton("Source"))
                {
                    std::string fileName = fileCopy.fileName;
                    if (std::filesystem::path(fileName).is_relative())
                        fileName = std::filesystem::weakly_canonical(renderGraph.baseDirectory + fileName).string();
                    else
                        fileName = std::filesystem::weakly_canonical(fileName).string();
                    ShellExecuteA(NULL, "open", fileName.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                }
				ImGui::TableSetColumnIndex(1);
                if (ImGui::SmallButton("Post"))
                {
                    std::string fileName = fileCopy.destFileName.empty() ? fileCopy.fileName : fileCopy.destFileName;
                    if (std::filesystem::path(fileName).is_relative())
                        fileName = std::filesystem::weakly_canonical(g_interpreter.GetTempDirectory() + "shaders\\" + fileName).string();
                    ShellExecuteA(NULL, "open", fileName.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                }
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("FileCopy");
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%s", name);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("destFileName: \"%s\"", fileCopy.destFileName.empty() ? fileCopy.fileName.c_str() : fileCopy.destFileName.c_str());

				ImGui::PopID();
			}

			ImGui::EndTable();
		}
		if (!scope.empty())
		{
			ImGui::PopID();
			ImGui::Unindent();
		}
	}

	ImGui::Unindent();
	ImGui::PopID();

	ImGui::End();
}

void ShowImportedResources()
{
    if (!g_showWindows.ImportedResources || g_hideUI)
        return;

    if (!ImGui::Begin("Imported Resources", &g_showWindows.ImportedResources))
    {
        ImGui::End();
        return;
    }

    // Put the imported resources into alphabetical order
    struct ImportedResourceInfo
    {
        std::string name;
        std::string originalName;
        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc* desc = nullptr;
    };
    std::vector<ImportedResourceInfo> importedResourcesSorted;

    for (auto& pair : g_interpreter.m_importedResources)
    {
        if (pair.second.nodeIndex == -1 || pair.second.resourceIndex == -1)
            continue;

        ImportedResourceInfo newInfo;
        newInfo.name = pair.first;
        newInfo.originalName = GetNodeOriginalName(g_interpreter.GetRenderGraph().nodes[pair.second.nodeIndex]).c_str();
        newInfo.desc = &pair.second;
        importedResourcesSorted.push_back(newInfo);
    }

    std::sort(importedResourcesSorted.begin(), importedResourcesSorted.end(),
        [](const ImportedResourceInfo& A, const ImportedResourceInfo& B)
        {
            return A.originalName < B.originalName;
        }
    );

    // show the imported resources
    for (ImportedResourceInfo& importedResourceInfo : importedResourcesSorted)
    {
        ImGui::Separator();

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = *importedResourceInfo.desc;

        ImGui::PushID(importedResourceInfo.name.c_str());

        if (importedResourceInfo.name != importedResourceInfo.originalName.c_str())
            ImGui::Text("%s (%s)", importedResourceInfo.name.c_str(), importedResourceInfo.originalName.c_str());
        else
            ImGui::Text("%s", importedResourceInfo.name.c_str());

        // Doesn't need to mark the desc as dirty. Doesn't change texture creation logic
        ImGui::Checkbox("Reset Every Frame", &desc.resetEveryFrame);
        ShowToolTip(
            "If checked, the render graph will get a clean version of this resource every frame (transient).\n"
            "Otherwise, the technique can modify this resource over time. (non transient)"
        );

        // If this is a texture
        if (desc.isATexture)
        {
            // Image File
            if (ImGui_File("File", desc.texture.fileName, "jpeg,jpg,png,bmp,hdr,psd,tga,gif,pic,pgm,ppm,exr,dds,bin"))
            {
                desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
            }
            ShowToolTip("This file is saved and loaded as relative to the .gg file");

            if (ImGui_Checkbox("Source is sRGB", &desc.texture.fileIsSRGB))
                desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
            ShowToolTip(
                "If the source file or source color is sRGB, this should be true.  This should be true in general unless you\n"
                "are loading an HDR image format, or if your source file is data (like normals), not a color."
            );

            if (ImGui_Checkbox("Make Mips", &desc.texture.makeMips))
                desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
            ShowToolTip("If true, mips will be made for this texture");

            // format
            {
                static const char* labels[] = {
                    
                    #include "external/df_serialize/_common.h"
                    #define ENUM_ITEM(_NAME, _DESCRIPTION) #_NAME,
                    // clang-format off
                    #include "external/df_serialize/_fillunsetdefines.h"
                    #include "Schemas/TextureFormats.h"
                    // clang-format on
                };

                if (ImGui::Combo("Format", (int*)&desc.texture.format, labels, _countof(labels)))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                ShowToolTip(
                    "This is the format you'd like the loaded image to be converted to.\n"
                    "If this format is the same as the image format, no conversion happens."
                );
            }

            if (ImGui::ColorEdit4("Color", desc.texture.color, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar))
                desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
            ShowToolTip(
                "The color to tint the loaded image. If you want a solid color image,\n"
                "you can set the size and it'll be this color, without loading anything from disk."
            );

            // Constant color texture settings - disabled when a filename is given
            const RuntimeTypes::RenderGraphNode_Resource_Texture& texInfo = g_interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(importedResourceInfo.name.c_str());
            {
                const bool textureLoaded = !desc.texture.fileName.empty();

                if (textureLoaded)
                    ImGui::BeginDisabled(true);

                if (desc.texture.textureType == RuntimeTypes::ViewableResource::Type::Texture2DArray || desc.texture.textureType == RuntimeTypes::ViewableResource::Type::Texture3D)
                {
                    if (ImGui::InputInt3("Size", desc.texture.size))
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }
                else
                {
                    if (ImGui::InputInt2("Size", desc.texture.size))
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }

                if (desc.texture.textureType == RuntimeTypes::ViewableResource::Type::TextureCube && desc.texture.size[2] != 6)
                {
                    desc.texture.size[2] = 6;
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }

                if (textureLoaded)
                    ImGui::EndDisabled();

                ShowToolTip("Set the size of the solid color texture");
            }

            if (ImGui::CollapsingHeader("Load Binary File"))
            {
                ImGui::PushID("LoadBinaryFile");

                // Size
                if (desc.texture.textureType == RuntimeTypes::ViewableResource::Type::Texture2DArray || desc.texture.textureType == RuntimeTypes::ViewableResource::Type::Texture3D)
                {
                    if (ImGui::InputInt3("Size", desc.texture.binaryDesc.size))
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }
                else
                {
                    if (ImGui::InputInt2("Size", desc.texture.binaryDesc.size))
                    {
                        desc.texture.binaryDesc.size[2] = 1;
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                    }
                }
                ShowToolTip("The dimensions of the data in the binary file");

                // Format
                {
                    static const char* labels[] = {
                        #include "external/df_serialize/_common.h"
                        #define ENUM_ITEM(_NAME, _DESCRIPTION) #_NAME,
                        // clang-format off
                        #include "external/df_serialize/_fillunsetdefines.h"
                        #include "Schemas/TextureFormats.h"
                        // clang-format on
                    };

                    if (ImGui::Combo("Format", (int*)&desc.texture.binaryDesc.format, labels, _countof(labels)))
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                    ShowToolTip("This is the format of the data in the binary file.");
                }

                ImGui::PopID();
            }

            // Show the image, if it's fully created and ready to show
            if (texInfo.IsCreated())
            {
                // NOTE: we can't show the m_resourceInitialState here for 3d textures (and shouldn't for 2d texture arrays and cube maps)
                // We could find the node and do what the render graph does, which ends up showing a slice of whatever the asset is.
                //D3D12_RESOURCE_DESC d = texInfo.m_resourceInitialState->GetDesc();
                //ShowImageThumbnail(texInfo.m_resourceInitialState, texInfo.m_format, texInfo.m_size, pair.second.nodeIndex, pair.second.resourceIndex, pair.first.c_str(), desc.texture.textureType);
            }

            if (ImGui::Button(importedResourceInfo.originalName.c_str()))
                g_resourceView.Buffer(desc.nodeIndex, desc.resourceIndex);
        }
        // Else this is a buffer, not a texture
        else
        {
            // File
            if (ImGui_File("File", desc.buffer.fileName, ""))
            {
                desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
            }
            ShowToolTip("This file is saved and loaded as relative to the .gg file");

            if (ImGui::Checkbox("CSV Header Row", &desc.buffer.CSVHeaderRow))
                desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
            ShowToolTip(
                "If loading a CSV file, check this box if there is a header row. Doing so will skip everything\n"
                "In the file up to the first newline character."
            );

            const RenderGraphNode_Resource_Buffer& bufferNode = g_interpreter.GetRenderGraph().nodes[desc.nodeIndex].resourceBuffer;

            // Ray tracing specific stuff
            if (bufferNode.accessedAs & (1 << (unsigned int)(ShaderResourceAccessType::RTScene)))
            {
                // Build flags
                std::vector<const char*> options;
                float comboWidth = 0.0f;
                for (int i = 0; i < EnumCount<GGUserFile_TLASBuildFlags>(); ++i)
                {
                    const char* label = EnumToString((GGUserFile_TLASBuildFlags)i);
                    options.push_back(label);
                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f);
                }
                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);

                int value = (int)desc.buffer.RT_BuildFlags;
                if (ImGui::Combo("BLAS Build Flags", &value, options.data(), (int)options.size()))
                {
                    desc.buffer.RT_BuildFlags = (GGUserFile_TLASBuildFlags)value;
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }
                ShowToolTip("build flags for BLAS. Translates to D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE etc.");

                if (ImGui::Checkbox("Opaque Geometry (BLAS)", &desc.buffer.BLASOpaque))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                ShowToolTip("When true, will act as if there is no any hit shader, even if there is. Makes ray tracing faster. Can also be set in shader using RAY_FLAG_FORCE_OPAQUE.");

                if (ImGui::Checkbox("No Duplicate AnyHit Invocation", &desc.buffer.BLASNoDuplicateAnyhitInvocations))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                ShowToolTip("By default, ray tracing APIs may call an any hit shader more than once for the same geometry, as a BVH traversal optimization. Turn this on to disable that.");

                // cull mode
                {
                    std::vector<const char*> options;
                    float comboWidth = 0.0f;
                    for (int i = 0; i < EnumCount<GGUserFile_BLASCullMode>(); ++i)
                    {
                        const char* label = EnumToString((GGUserFile_BLASCullMode)i);
                        options.push_back(label);
                        comboWidth = std::max(comboWidth, ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f);
                    }
                    ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);

                    int value = (int)desc.buffer.BLASCullMode;
                    if (ImGui::Combo("BLAS Cull Mode", &value, options.data(), (int)options.size()))
                    {
                        desc.buffer.BLASCullMode = (GGUserFile_BLASCullMode)value;
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                    }
                    ShowToolTip("Controls D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE and D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE");
                }

                if (ImGui::Checkbox("Is AABBs", &desc.buffer.IsAABBs))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                ShowToolTip("Set to true if the buffer has AABB data in it, for ray tracing with intersection shaders. Format is Min XYZ, Max XYZ.");
            }

            // Type
            {
                if (desc.buffer.structIndex != -1)
                    ImGui::BeginDisabled(true);

                std::vector<std::string> labelsStr;
                labelsStr.push_back("");
                for (int i = 0; i < EnumCount<DataFieldType>() - 1; ++i)
                    labelsStr.push_back(EnumToString((DataFieldType)i));

                float comboWidth = 0.0f;
                std::vector<char> labels;
                for (std::string& s : labelsStr)
                {
                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(s.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
                    if (s.empty())
                        labels.push_back(' ');
                    for (char c : s)
                        labels.push_back(c);
                    labels.push_back(0);
                }
                labels.push_back(0);

                int selected = (desc.buffer.type == DataFieldType::Count) ? 0 : (int)desc.buffer.type + 1;
                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
                if (ImGui::Combo("Type", &selected, labels.data(), (int)labelsStr.size()))
                {
                    desc.buffer.type = (selected == 0) ? DataFieldType::Count : (DataFieldType)(selected - 1);
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }

                ShowToolTip("The type, if this is not a structured buffer");

                if (desc.buffer.structIndex != -1)
                    ImGui::EndDisabled();
            }

            // Struct Index
            {
                if (desc.buffer.type != DataFieldType::Count)
                    ImGui::BeginDisabled(true);

                std::vector<std::string> labelsStr;
                labelsStr.push_back("");
                for (const Struct& s : g_interpreter.GetRenderGraph().structs)
                    labelsStr.push_back(s.name);

                float comboWidth = 0.0f;
                std::vector<char> labels;
                for (std::string& s : labelsStr)
                {
                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(s.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
                    if (s.empty())
                        labels.push_back(' ');
                    for (char c : s)
                        labels.push_back(c);
                    labels.push_back(0);
                }
                labels.push_back(0);

                int selected = desc.buffer.structIndex + 1;
                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
                if (ImGui::Combo("Struct", &selected, labels.data(), (int)labelsStr.size()))
                {
                    desc.buffer.structIndex = selected - 1;
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }

                ShowToolTip("The struct, if this is a structured buffer");

                if (desc.buffer.type != DataFieldType::Count)
                    ImGui::EndDisabled();
            }

            // Count
            {
                if (!desc.buffer.fileName.empty())
                    ImGui::BeginDisabled(true);

                if (ImGui::InputInt("Count", &desc.buffer.count))
                {
                    desc.buffer.count = std::max(desc.buffer.count, 1);
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }

                if (!desc.buffer.fileName.empty())
                    ImGui::EndDisabled();
            }

            // Geometry Transform Matrix
            {
                if (ImGui::CollapsingHeader("Geometry Transform"))
                {
                    if (ImGui::InputFloat4("row0", &desc.buffer.GeometryTransform[0]) ||
                        ImGui::InputFloat4("row1", &desc.buffer.GeometryTransform[4]) ||
                        ImGui::InputFloat4("row2", &desc.buffer.GeometryTransform[8]) ||
                        ImGui::InputFloat4("row3", &desc.buffer.GeometryTransform[12]))
                    {
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                    }
                }
            }

            // Cooperative Vector
            if (g_agilitySDKChoice == AgilitySDKChoice::Preview)
            {
                ImGui::Indent();
                ImGui::Text("Cooperative Vectors");
                ImGui::PushID("Cooperative Vectors");

                // enabled
                if (ImGui::Checkbox("Convert", &desc.buffer.cvData.convert))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                ShowToolTip("If true, will do the data conversion described below");

                // width / height
                {
                    int wh[2] = { (int)desc.buffer.cvData.width, (int)desc.buffer.cvData.height };
                    if (ImGui::InputInt2("Width Height", wh))
                    {
                        desc.buffer.cvData.width = (unsigned int)std::max(wh[0], 0);
                        desc.buffer.cvData.height = (unsigned int)std::max(wh[1], 0);
                        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                    }
                    ShowToolTip("How many columns and rows in the matrix or vector");
                }

                // source type
                if (ShowGigiEnumDropDown(desc.buffer.cvData.srcType, "Source Type", "The data type of the source data."))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;

                // source layout
                if (ShowGigiEnumDropDown(desc.buffer.cvData.srcLayout, "Source Layout", "The layout of the source data."))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;

                // dest type
                if (ShowGigiEnumDropDown(desc.buffer.cvData.destType, "Dest Type", "The data type you want it to be converted to before use in the shader."))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;

                // dest layout
                if (ShowGigiEnumDropDown(desc.buffer.cvData.destLayout, "Dest Layout", "The layout you want it to be converted to before use in the shader."))
                    desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;

                ImGui::PopID();
                ImGui::Unindent();
            }

            // Mesh Details
            {
                bool exists = false;
                RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData = g_interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(importedResourceInfo.originalName.c_str(), exists);
                if (exists)
                {
                    if (runtimeData.materials.size() > 0)
                    {
                        if (ImGui::Button("Mesh Info"))
                        {
                            g_meshInfoOpenPopup = true;
                            g_meshInfoName = importedResourceInfo.originalName;
                        }

                        ImGui::SameLine();
                    }
                }
            }

            if (ImGui::Button(importedResourceInfo.originalName.c_str()))
                g_resourceView.Buffer(desc.nodeIndex, desc.resourceIndex);
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void ShowImGuiWindows()
{
    if (g_meshInfoOpenPopup)
    {
        ImGui::OpenPopup("Mesh Info Popup");
        g_meshInfoOpenPopup = false;
    }

    if (ImGui::BeginPopup("Mesh Info Popup", ImGuiWindowFlags_AlwaysAutoResize))
    {
        bool exists = false;
        RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeData = g_interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(g_meshInfoName.c_str(), exists);
        if (exists)
        {
            ImGui::Text("%s", g_meshInfoName.c_str());

            ImGui::Text("Materials: %i", (int)runtimeData.materials.size());
            if (ImGui::BeginTable("Materials", 2, ImGuiTableFlags_Borders))
            {
                for (size_t i = 0; i < runtimeData.materials.size(); ++i)
                {
                    const auto& materialInfo = runtimeData.materials[i];

                    if (!materialInfo.used)
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 128));

                    ImGui::TableNextColumn();
                    ImGui::Text("%i", (int)i);
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", materialInfo.name.c_str());

                    if (!materialInfo.used)
                        ImGui::PopStyleColor();
                }
                ImGui::EndTable();
            }
        }
        if (ImGui::Button("Copy"))
        {
            std::ostringstream text;
            for (size_t i = 0; i < runtimeData.materials.size(); ++i)
            {
                const auto& materialInfo = runtimeData.materials[i];
                text << i << " " << materialInfo.name;
                if (!materialInfo.used)
                    text << " (unused)";
                text << "\n";

            }
            std::string textStr = text.str();
            SetClipboardDataEx(CF_TEXT, (void*)textStr.c_str(), (DWORD)textStr.length() + 1);
        }
        //if (ImGui::Button("OK"))
            //ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    if (g_showViewerSettings)
    {
        if (ImGui::Begin("Viewer Settings", &g_showViewerSettings, 0))
        {
            bool changed = false;

            if (ImGui::Button("Reset To Defaults"))
            {
                g_viewerConfig = GGViewerConfig();
                changed = true;
            }

            ImGui::SeparatorText("Camera Key Bindings");

            auto KeyBindingDropDown = [](const char* label, std::string& selectedValue, const char* tooltip = nullptr) -> bool
            {
                bool ret = false;
                if (ImGui::BeginCombo(label, selectedValue.c_str(), ImGuiComboFlags_None))
                {
                    for (int index = 0; index < 28; ++index)
                    {
                        std::string shownLabel;
                        if (index < 26)
                        {
                            shownLabel = 'A' + index;
                        }
                        switch (index)
                        {
                            case 26: shownLabel = "Shift"; break;
                            case 27: shownLabel = "Control"; break;
                        }

                        const bool is_selected = (selectedValue == shownLabel);
                        if (ImGui::Selectable(shownLabel.c_str(), is_selected))
                        {
                            selectedValue = shownLabel;
                            ret = true;
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                ShowToolTip(tooltip);

                return ret;
            };

            changed |= KeyBindingDropDown("Forward", g_viewerConfig.keyCameraForward, "Up arrow also works");
            changed |= KeyBindingDropDown("Left", g_viewerConfig.keyCameraLeft, "Left arrow also works");
            changed |= KeyBindingDropDown("Backward", g_viewerConfig.keyCameraBackward, "Down arrow also works");
            changed |= KeyBindingDropDown("Right", g_viewerConfig.keyCameraRight, "Right arrow also works");
            changed |= KeyBindingDropDown("Up", g_viewerConfig.keyCameraUp, "Page up also works");
            changed |= KeyBindingDropDown("Down", g_viewerConfig.keyCameraDown, "Page down also works");
            changed |= KeyBindingDropDown("Fast", g_viewerConfig.keyCameraFast);
            changed |= KeyBindingDropDown("Slow", g_viewerConfig.keyCameraSlow);

            if (changed)
            {
                std::filesystem::path dir = GetLocalAppDataPath();
                std::filesystem::create_directories(dir);
                WriteToJSONFile(g_viewerConfig, (dir / "ViewerConfig.json").string().c_str());
            }
        }

        ImGui::End();
    }

    if (g_showCapsWindow)
    {
        if (ImGui::Begin("DX12 Capabilities", &g_showCapsWindow, 0))
        {
            ImGui::Text("GPU: %s (driver %s)", g_adapterName.c_str(), g_driverVersion.c_str());
            ImGui::Text("CPU: %s", GetCPUName().c_str());

            // Options 5
            ImGui::SeparatorText("Options 5");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS5& options = g_interpreter.GetOptions5();
                ImGui::Text("SRVOnlyTiledResourceTier3: %s", options.SRVOnlyTiledResourceTier3 ? "True" : "False");
                switch (options.RenderPassesTier)
                {
                    case D3D12_RENDER_PASS_TIER_0: ImGui::Text("RenderPassesTier: 0"); break;
                    case D3D12_RENDER_PASS_TIER_1: ImGui::Text("RenderPassesTier: 1"); break;
                    case D3D12_RENDER_PASS_TIER_2: ImGui::Text("RenderPassesTier: 2"); break;
                    default: ImGui::Text("RenderPassesTier: Unknown");
                }

                switch (options.RaytracingTier)
                {
                    case D3D12_RAYTRACING_TIER_NOT_SUPPORTED: ImGui::Text("RaytracingTier: Unsupported"); break;
                    case D3D12_RAYTRACING_TIER_1_0: ImGui::Text("RaytracingTier: 1.0"); break;
                    case D3D12_RAYTRACING_TIER_1_1: ImGui::Text("RaytracingTier: 1.1"); break;
                    default: ImGui::Text("RaytracingTier: Unknown");
                }
            }

            // Options 6
            ImGui::SeparatorText("Options 6");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS6& options = g_interpreter.GetOptions6();

                ImGui::Text("AdditionalShadingRatesSupported: %s", options.AdditionalShadingRatesSupported ? "True" : "False");
                ImGui::Text("PerPrimitiveShadingRateSupportedWithViewportIndexing: %s", options.PerPrimitiveShadingRateSupportedWithViewportIndexing ? "True" : "False");

                switch (options.VariableShadingRateTier)
                {
                    case D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED: ImGui::Text("VariableShadingRateTier: Unsupported"); break;
                    case D3D12_VARIABLE_SHADING_RATE_TIER_1: ImGui::Text("VariableShadingRateTier: 1"); break;
                    case D3D12_VARIABLE_SHADING_RATE_TIER_2: ImGui::Text("VariableShadingRateTier: 2"); break;
                    default: ImGui::Text("VariableShadingRateTier: Unknown");
                }

                ImGui::Text("ShadingRateImageTileSize: %u", options.ShadingRateImageTileSize);
                ImGui::Text("BackgroundProcessingSupported: %s", options.BackgroundProcessingSupported ? "True" : "False");
            }

            // Options 7
            ImGui::SeparatorText("Options 7");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS7& options = g_interpreter.GetOptions7();

                switch (options.MeshShaderTier)
                {
                    case D3D12_MESH_SHADER_TIER_NOT_SUPPORTED: ImGui::Text("MeshShaderTier: Unsupported"); break;
                    case D3D12_MESH_SHADER_TIER_1: ImGui::Text("MeshShaderTier: 1"); break;
                    default: ImGui::Text("MeshShaderTier: Unknown");
                }

                switch (options.SamplerFeedbackTier)
                {
                    case D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED: ImGui::Text("SamplerFeedbackTier: Unsupported"); break;
                    case D3D12_SAMPLER_FEEDBACK_TIER_0_9: ImGui::Text("SamplerFeedbackTier: 0.9"); break;
                    case D3D12_SAMPLER_FEEDBACK_TIER_1_0: ImGui::Text("SamplerFeedbackTier: 1.0"); break;
                    default: ImGui::Text("SamplerFeedbackTier: Unknown");
                }
            }

            // Options 8
            ImGui::SeparatorText("Options 8");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS8& options = g_interpreter.GetOptions8();
                ImGui::Text("UnalignedBlockTexturesSupported: %s", options.UnalignedBlockTexturesSupported ? "True" : "False");
            }

            // Options 9
            ImGui::SeparatorText("Options 9");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS9& options = g_interpreter.GetOptions9();
                ImGui::Text("MeshShaderPipelineStatsSupported: %s", options.MeshShaderPipelineStatsSupported ? "True" : "False");
                ImGui::Text("MeshShaderSupportsFullRangeRenderTargetArrayIndex: %s", options.MeshShaderSupportsFullRangeRenderTargetArrayIndex ? "True" : "False");
                ImGui::Text("AtomicInt64OnTypedResourceSupported: %s", options.AtomicInt64OnTypedResourceSupported ? "True" : "False");
                ImGui::Text("AtomicInt64OnGroupSharedSupported: %s", options.AtomicInt64OnGroupSharedSupported ? "True" : "False");
                ImGui::Text("DerivativesInMeshAndAmplificationShadersSupported: %s", options.DerivativesInMeshAndAmplificationShadersSupported ? "True" : "False");

                switch (options.WaveMMATier)
                {
                    case D3D12_WAVE_MMA_TIER_NOT_SUPPORTED: ImGui::Text("WaveMMATier: Unsupported"); break;
                    case D3D12_WAVE_MMA_TIER_1_0: ImGui::Text("WaveMMATier: 1.0"); break;
                    default: ImGui::Text("WaveMMATier: Unknown");
                }
            }

            // Options 10
            ImGui::SeparatorText("Options 10");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS10& options = g_interpreter.GetOptions10();
                ImGui::Text("VariableRateShadingSumCombinerSupported: %s", options.VariableRateShadingSumCombinerSupported ? "True" : "False");
                ImGui::Text("MeshShaderPerPrimitiveShadingRateSupported: %s", options.MeshShaderPerPrimitiveShadingRateSupported ? "True" : "False");
            }

            // Options 11
            ImGui::SeparatorText("Options 11");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS11& options = g_interpreter.GetOptions11();
                ImGui::Text("AtomicInt64OnDescriptorHeapResourceSupported: %s", options.AtomicInt64OnDescriptorHeapResourceSupported ? "True" : "False");
            }

            // Options Experimental
            ImGui::SeparatorText("Options Experimental");
            {
                const D3D12_FEATURE_DATA_D3D12_OPTIONS_EXPERIMENTAL& options = g_interpreter.GetOptionsExperimental();

                const char* CVTier = "Unknown";
                switch (options.CooperativeVectorTier)
                {
                    case D3D12_COOPERATIVE_VECTOR_TIER_NOT_SUPPORTED: CVTier = "None"; break;
                    case D3D12_COOPERATIVE_VECTOR_TIER_1_0: CVTier = "1.0"; break;
                    case D3D12_COOPERATIVE_VECTOR_TIER_1_1: CVTier = "1.1"; break;
                }

                ImGui::Text("Cooperative Vector Tier: %s", CVTier);
            }

            // TODO: do the rest at some point!
        }

        ImGui::End();
    }

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
}

// @param index e.g. ImGuiCol_Header
// @param fade 0..1, 0:fade out completely .. 1:no fade out
void PushStyleColorFade(uint32_t index, float fade)
{
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 color = style.Colors[index];

    color.w *= fade;

    ImGui::PushStyleColor(index, color);
}

void CopyToClipboard(bool copyAll, bool withType)
{
    std::ostringstream text;
    constexpr const char* msgType[3] = { "", "[Warning] ", "[Error] " };
    for (const auto& msg : g_log)
    {
        if (!copyAll && !msg.selected)
            continue;

        if(withType)
            text << msgType[static_cast<int>(msg.level)];

        text << msg.msg.c_str() << "\n";
    }

    std::string textStr = text.str();

    SetClipboardDataEx(CF_TEXT, (void*)textStr.c_str(), (DWORD)textStr.length() + 1);
}

void PrintMessageType(ImVec4 color, const char* txt, float width)
{
    ImVec2 text_pos = ImGui::GetCursorScreenPos();

    // background color fill
    ImGui::GetWindowDrawList()->AddRectFilled(
        text_pos,
        ImVec2(text_pos.x + width, text_pos.y + ImGui::GetFontSize()),
        ImGui::ColorConvertFloat4ToU32(color)
    );

    // black text for contrast
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    ImGui::TextUnformatted(txt);
    ImGui::PopStyleColor();
}

void ShowLog()
{
    if (!g_showWindows.Log || g_hideUI)
        return;

    if (!ImGui::Begin("Log", &g_showWindows.Log))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Copy all"))
    {
        CopyToClipboard(true, true);
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear"))
        g_log.clear();

    ImGui::SameLine();
    if (ImGui::Button("Open Temp Folder"))
        ShellExecuteA(NULL, "explore", g_interpreter.GetTempDirectory().c_str(), NULL, NULL, SW_SHOWDEFAULT);

    ImGui::BeginChild("##LogScrollableRegion", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // minimal Text for nicer visuals
    constexpr const char* msgType[3] = { "", " W ", " E " };
    const ImVec4 msgColor[3] = { ImGui::GetStyleColorVec4(ImGuiCol_Text), { 0.8f, 0.7f, 0.2f, 1.0f}, { 0.8f, 0.2f, 0.2f, 1.0f } };

    // find longest item
    float widthColumn0 = 0;
    for (int i = 0; i < sizeof(msgType) / sizeof(*msgType); ++i)
        widthColumn0 = std::max(widthColumn0, ImGui::CalcTextSize(msgType[i]).x);

    const float spaceGap = ImGui::CalcTextSize(" ").x;

    // make selection visually more pleasing
    const float fade = 0.2f;
    PushStyleColorFade(ImGuiCol_Header, fade);
    PushStyleColorFade(ImGuiCol_HeaderHovered, fade);
    PushStyleColorFade(ImGuiCol_HeaderActive, fade);

    // todo: this can be optimized to only render the visible range
    for (size_t index = 0, count = g_log.size(); index < count; ++index)
    {
        auto& msg = g_log[index];

        ImGui::PushID((int)index);

        int msgTypeIndex = static_cast<int>(msg.level);

        const float x = ImGui::GetCursorPosX();

        if (msgTypeIndex && msg.firstLine)
        {
            PrintMessageType(msgColor[msgTypeIndex], msgType[msgTypeIndex], widthColumn0);
            ImGui::SameLine();
        }
        ImGui::SetCursorPosX(x + widthColumn0 + spaceGap);

        ImGui::PushStyleColor(ImGuiCol_Text, msgColor[msgTypeIndex]);

        bool state = ImGui::Selectable(msg.msg.c_str(), &msg.selected);

        ImGui::PopStyleColor();

        if (state) 
        {
            static size_t lastSelectedIndex = -1;

            if (ImGui::GetIO().KeyShift) // Range-select with Shift
            {
                // if there was a last selected
                if ((uint32_t)lastSelectedIndex < g_log.size() )
                {
                    size_t minIndex = std::min(lastSelectedIndex, index);
                    size_t maxIndex = std::max(lastSelectedIndex, index);

                    // select range
                    for (size_t index2 = 0, count2 = g_log.size(); index2 < count2; ++index2)
                        g_log[index2].selected = (index2 >= minIndex && index2 <= maxIndex);
                }
            }
            else if (ImGui::GetIO().KeyCtrl) // Multi-select with Ctrl
            {
                // already handled by Selectable() function call
            }
            else
            {
                // Single select
                for (auto& msg2 : g_log)
                    msg2.selected = false;
                msg.selected = true;
            }
            lastSelectedIndex = index;
        }
        ImGui::PopID();

        // Autoscroll
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopStyleColor(3);

    // Right-click context menu
    if (ImGui::BeginPopupContextWindow("RightClickAnywhere", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Copy Selection", "Ctrl+C"))
            CopyToClipboard(false, false);

        if (ImGui::MenuItem("Copy Selection with message type"))
            CopyToClipboard(false, true);

        ImGui::Separator();

        if (ImGui::MenuItem("Copy All with message type"))
            CopyToClipboard(true, true);

        if (ImGui::MenuItem("Clear All"))
            g_log.clear();

        ImGui::EndPopup();
    }


    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C))
            CopyToClipboard(false, false);
    }

    ImGui::EndChild();

    ImGui::End();
}

void AutoHistogram(ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int zIndex, int mipIndex, float& histogramMin, float& histogramMax)
{
    // Get the decoded image data
    DXGI_FORMAT decodedFormat;
    std::vector<unsigned char> decodedPixels;
    int decodedWidth = 0;
    int decodedHeight = 0;
    if (!ImageReadback::GetDecodedImageSlice(g_pd3dDevice, readbackResource, resourceOriginalDesc, zIndex, mipIndex, decodedPixels, decodedFormat, decodedWidth, decodedHeight))
    {
        Log(LogLevel::Error, __FUNCTION__ " Could not decode block compressed image\n");
        return;
    }

    float minValue[4] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
    float maxValue[4] = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };

    DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);

    switch (decodedFormatInfo.channelType)
    {
        case DXGI_FORMAT_Info::ChannelType::_uint8_t:
        {
            const uint8_t* value = (uint8_t*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    float v = InverseLerp(0.0f, 255.0f, float(*value));
                    minValue[i] = std::min(minValue[i], v);
                    maxValue[i] = std::max(maxValue[i], v);
                    value++;
                }
            }
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_int8_t:
        {
            const int8_t* value = (int8_t*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    float v = InverseLerp(-128.0f, 127.0f, float(*value));
                    minValue[i] = std::min(minValue[i], v);
                    maxValue[i] = std::max(maxValue[i], v);
                    value++;
                }
            }
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_uint16_t:
        {
            const uint16_t* value = (uint16_t*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    float v = InverseLerp(0.0f, 65535.0f, float(*value));
                    minValue[i] = std::min(minValue[i], v);
                    maxValue[i] = std::max(maxValue[i], v);
                    value++;
                }
            }
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_int16_t:
        {
            const int16_t* value = (int16_t*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    float v = InverseLerp(-32768.0f, 32767.0f, float(*value));
                    minValue[i] = std::min(minValue[i], v);
                    maxValue[i] = std::max(maxValue[i], v);
                    value++;
                }
            }
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_uint32_t:
        {
            const uint32_t* value = (uint32_t*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    float v = InverseLerp(0.0f, 4294967295.0f, float(*value));
                    minValue[i] = std::min(minValue[i], v);
                    maxValue[i] = std::max(maxValue[i], v);
                    value++;
                }
            }
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_int32_t:
        {
            const int32_t* value = (int32_t*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    float v = InverseLerp(-2147483648.0f, 2147483647.0f, float(*value));
                    minValue[i] = std::min(minValue[i], v);
                    maxValue[i] = std::max(maxValue[i], v);
                    value++;
                }
            }
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_float:
        {
            const float* value = (float*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    minValue[i] = std::min(minValue[i], *value);
                    maxValue[i] = std::max(maxValue[i], *value);
                    value++;
                }
            }
            break;
        }
        case DXGI_FORMAT_Info::ChannelType::_half:
        {
            const half* value = (half*)decodedPixels.data();
            for (int ix = 0; ix < decodedWidth * decodedHeight; ++ix)
            {
                for (int i = 0; i < decodedFormatInfo.channelCount; ++i)
                {
                    minValue[i] = std::min<float>(minValue[i], *value);
                    maxValue[i] = std::max<float>(maxValue[i], *value);
                    value++;
                }
            }
            break;
        }
        default:
        {
            Assert(false, "Unhandled Channel Type");
            return;
        }
    }

    // Use the min / max histogram of visible channels
    bool first = true;
    for (int i = 0; i < 4; ++i)
    {
        if (minValue[i] == FLT_MAX || maxValue[i] == -FLT_MAX)
            continue;

        if (!g_colorsShown[i])
            continue;

        if (first)
        {
            first = false;
            g_histogramMinMax[0] = minValue[i];
            g_histogramMinMax[1] = maxValue[i];
        }
        else
        {
            g_histogramMinMax[0] = std::min(g_histogramMinMax[0], minValue[i]);
            g_histogramMinMax[1] = std::max(g_histogramMinMax[1], maxValue[i]);
        }
    }
}

void ShowPixelValue(ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int x, int y, int z, int mipIndex, float cursorPosX, int radius)
{
    // read back the pixel data
    std::vector<unsigned char> pixelsUntypedData;
    DXGI_FORMAT viewFormat = DXGI_FORMAT_UNKNOWN;
    if (!ImageReadback::GetDecodedPixelsRectangle(g_pd3dDevice, readbackResource, resourceOriginalDesc, x - radius, x + radius + 1, y - radius, y + radius + 1, z, mipIndex, pixelsUntypedData, viewFormat))
        return;

    // Make sure we got enough data
    DXGI_FORMAT_Info viewFormatInfo = Get_DXGI_FORMAT_Info(viewFormat);
    if ((pixelsUntypedData.size() % viewFormatInfo.bytesPerPixel) != 0)
        return;

    // Make a string of the pixel values
    const unsigned char* pixelsUntyped = pixelsUntypedData.data();
    int pixelCount = (int)pixelsUntypedData.size() / viewFormatInfo.bytesPerPixel;
    float pixelColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    char str[256], *writer = str;
    bool isHDR = false;
    if (viewFormatInfo.format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS)
    {
        uint32_t* pixel = (uint32_t*)pixelsUntyped;

        float value = 0;
        for (int pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
            value = Lerp(value, float(double(pixel[pixelIndex]) / 16777216.0), 1.0f / float(pixelIndex + 1));

        sprintf(str, "%f", value);

        pixelColor[0] = value;
    }
    else if (viewFormatInfo.format == DXGI_FORMAT_R11G11B10_FLOAT)
    {
        float value[3] = { 0.0f, 0.0f, 0.0f };
        for (int pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
        {
            float rgb[3];
            R11G11B10tof32(*(uint32_t*)pixelsUntyped, rgb);
            value[0] = Lerp(value[0], rgb[0], 1.0f / float(pixelIndex + 1));
            value[1] = Lerp(value[1], rgb[1], 1.0f / float(pixelIndex + 1));
            value[2] = Lerp(value[2], rgb[2], 1.0f / float(pixelIndex + 1));
        }

		sprintf(str, "%f %f %f", value[0], value[1], value[2]);

        pixelColor[0] = value[0];
        pixelColor[1] = value[1];
        pixelColor[2] = value[2];
    }
    else
    {
        // Convert the rectangular region of pixels into doubles, average them, and convert them back to their source format for display logic below
        std::vector<unsigned char> pixelUntypedData;
        {
            std::vector<double> doublesRaw;
            if (!ConvertToDoubles(pixelsUntyped, pixelCount * viewFormatInfo.channelCount, viewFormatInfo.channelType, doublesRaw))
                return;

            std::vector<double> averagePixel(viewFormatInfo.channelCount, 0.0);
            for (int pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
            {
                for (int channelIndex = 0; channelIndex < viewFormatInfo.channelCount; ++channelIndex)
                {
                    double value = doublesRaw[pixelIndex * viewFormatInfo.channelCount + channelIndex];
                    averagePixel[channelIndex] = Lerp(averagePixel[channelIndex], value, 1.0 / double(pixelIndex + 1));
                }
            }

            ConvertFromDoubles(averagePixel, viewFormatInfo.channelType, pixelUntypedData);
        }
        const unsigned char* pixelUntyped = pixelUntypedData.data();

        // Display the pixels
        switch (viewFormatInfo.channelType)
        {
            case DXGI_FORMAT_Info::ChannelType::_uint8_t:
            {
                uint8_t* pixel = (uint8_t*)pixelUntyped;
                for (int i = 0; i < viewFormatInfo.channelCount; ++i)
                {
					sprintf(writer, "%u ", (unsigned int)pixel[i]);
                    writer += strlen(writer);

                    pixelColor[i] = float(pixel[i]) / 255.0f;
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_int8_t:
            {
                int8_t* pixel = (int8_t*)pixelUntyped;
                for (int i = 0; i < viewFormatInfo.channelCount; ++i)
                {
					sprintf(writer, "%i ", (int)pixel[i]);
					writer += strlen(writer);

                    pixelColor[i] = (float(pixel[i]) + 128.0f) / 255.0f;
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_int16_t:
            {
                int16_t* pixel = (int16_t*)pixelUntyped;
                for (int i = 0; i < viewFormatInfo.channelCount; ++i)
                {
					sprintf(writer, "%i ", (int)pixel[i]);
					writer += strlen(writer);

                    pixelColor[i] = float(pixel[i] + 32768.0f) / 65535.0f;
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_uint16_t:
            {
                uint16_t* pixel = (uint16_t*)pixelUntyped;
                for (int i = 0; i < viewFormatInfo.channelCount; ++i)
                {
					sprintf(writer, "%u ", (unsigned int)pixel[i]);
					writer += strlen(writer);

                    pixelColor[i] = float(pixel[i]) / 65535.0f;
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_uint32_t:
            {
                uint32_t* pixel = (uint32_t*)pixelUntyped;
                for (int i = 0; i < viewFormatInfo.channelCount; ++i)
                {
					sprintf(writer, "%u ", (unsigned int)pixel[i]);
					writer += strlen(writer);

                    pixelColor[i] = float(pixel[i]) / 4294967295.0f;
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_half:
            {
                half* pixel = (half*)pixelUntyped;
                for (int i = 0; i < viewFormatInfo.channelCount; ++i)
                {
					float f = pixel[i];
					sprintf(writer, "%f ", f);
					writer += strlen(writer);

                    pixelColor[i] = f;
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_float:
            {
                isHDR = true;
                float* pixel = (float*)pixelUntyped;
                for (int i = 0; i < viewFormatInfo.channelCount; ++i)
                {
					sprintf(writer, "%f ", pixel[i]);
					writer += strlen(writer);

                    pixelColor[i] = pixel[i];
                }
                break;
            }
            default:
            {
                Assert(false, "Unhandled Channel Type");
                break;
            }
        }
    }

    // show the actual pixel color
    if (viewFormatInfo.sRGB)
    {
        pixelColor[0] = SRGBToLinear(pixelColor[0]);
        pixelColor[1] = SRGBToLinear(pixelColor[1]);
        pixelColor[2] = SRGBToLinear(pixelColor[2]);
    }
    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaPreview;
    if (isHDR)
        flags |= ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;

    ImGui::SameLine();
    ImGui::SetCursorPosX(cursorPosX);

    // to get vertical alignment in the line
    const float size = ImGui::GetTextLineHeight();

    ImGui::ColorButton("Pixel Color", ImVec4{ pixelColor[0], pixelColor[1],pixelColor[2],pixelColor[3] }, flags, ImVec2(size, size));

    // If the format is a normed texture, we wrote it as int or uint, but we want to show it as float.
    // furthermore, snorm textures need to be converted from [0,1] to [-1,1]
    switch (viewFormatInfo.normType)
    {
        case DXGI_FORMAT_Info::NormType::UNorm:
        {
            writer = str;
            for (int i = 0; i < viewFormatInfo.channelCount; ++i)
            {
                sprintf(writer, "%f ", pixelColor[i]);
                writer += strlen(writer);
            }
            break;
        }
        case DXGI_FORMAT_Info::NormType::SNorm:
        {
            writer = str;
            for (int i = 0; i < viewFormatInfo.channelCount; ++i)
            {
                pixelColor[i] = pixelColor[i] * 2.0f - 1.0f;
                sprintf(writer, "%f ", pixelColor[i]);
                writer += strlen(writer);
            }
            break;
        }
    }

    // show value after color button to not have the UI move around when the content changes
    ImGui::SameLine();
	ImGui::TextUnformatted(str);
}

void SaveAsBin(const char* fileName, unsigned char* bytes, int count)
{
    std::filesystem::path p(fileName);
    if (!p.has_extension() || p.extension() != ".bin")
        p.replace_extension(".bin");

    FILE* file = nullptr;
    fopen_s(&file, p.string().c_str(), "wb");
    if (!file)
        return;

    fwrite(bytes, 1, count, file);

    fclose(file);
}

void SaveAsHex(const char* fileName, unsigned char* bytes, int count)
{
    std::filesystem::path p(fileName);
    if (!p.has_extension() || p.extension() != ".hex")
        p.replace_extension(".hex");

    FILE* file = nullptr;
    fopen_s(&file, p.string().c_str(), "wb");
    if (!file)
        return;

    if (count % 4 == 0)
    {
        const uint32_t* ptr = (const uint32_t*)bytes;

        fprintf(file, "uint32_t bytes[] = {");
        for (int i = 0; i < count / 4; ++i)
            fprintf(file, "%s0x%08X", (i > 0 ? ", " : ""), ptr[i]);
        fprintf(file, "};");
    }
    else
    {
        const uint8_t* ptr = (const uint8_t*)bytes;

        fprintf(file, "uint8_t bytes[] = {");
        for (int i = 0; i < count; ++i)
            fprintf(file, "%s0x%02X", (i > 0 ? "," : ""), ptr[i]);
        fprintf(file, "};");
    }

    fclose(file);
}

void SaveAsCSV(const char* fileName, const unsigned char* bytes, DXGI_FORMAT format, int formatCount, int count)
{
    std::filesystem::path p(fileName);
    if (!p.has_extension() || p.extension() != ".csv")
        p.replace_extension(".csv");

    FILE* file = nullptr;
    fopen_s(&file, p.string().c_str(), "wb");
    if (!file)
        return;

    DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(format);

    int actualChannelCount = formatCount * formatInfo.channelCount;

    // write the header row
    if (actualChannelCount > 1)
    {
        for (int c = 0; c < actualChannelCount; ++c)
            fprintf(file, "%s\"value%i\"", (c > 0 ? "," : ""), c);
        fprintf(file, "\n");
    }
    else
        fprintf(file, "\"value\"\n");

    int offset = 0;
    for (int index = 0; index < count; ++index)
    {
        switch (formatInfo.channelType)
        {
            case DXGI_FORMAT_Info::ChannelType::_uint8_t:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%u\"", (c > 0 ? "," : ""), *((const uint8_t*)&bytes[offset]));
                    offset += sizeof(uint8_t);
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_uint16_t:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%u\"", (c > 0 ? "," : ""), *((const uint16_t*)&bytes[offset]));
                    offset += sizeof(uint16_t);
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_uint32_t:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%u\"", (c > 0 ? "," : ""), *((const uint32_t*)&bytes[offset]));
                    offset += sizeof(uint32_t);
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_int8_t:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%i\"", (c > 0 ? "," : ""), *((const int8_t*)&bytes[offset]));
                    offset += sizeof(uint8_t);
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_int16_t:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%i\"", (c > 0 ? "," : ""), *((const int16_t*)&bytes[offset]));
                    offset += sizeof(uint16_t);
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_int32_t:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%i\"", (c > 0 ? "," : ""), *((const int32_t*)&bytes[offset]));
                    offset += sizeof(uint32_t);
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_float:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%f\"", (c > 0 ? "," : ""), *((const float*)&bytes[offset]));
                    offset += sizeof(float);
                }
                break;
            }
            case DXGI_FORMAT_Info::ChannelType::_half:
            {
                for (int c = 0; c < actualChannelCount; ++c)
                {
                    fprintf(file, "%s\"%f\"", (c > 0 ? "," : ""), (float)*((half*)&bytes[offset]));
                    offset += sizeof(float);
                }
                break;
            }
        }

        fprintf(file, "\n");
    }

    fclose(file);
}

void SaveAsCSV(const char* fileName, unsigned char* bytes, const Struct& structDesc, int count)
{
    std::filesystem::path p(fileName);
    if (!p.has_extension() || p.extension() != ".csv")
        p.replace_extension(".csv");

    FILE* file = nullptr;
    fopen_s(&file, p.string().c_str(), "wb");
    if (!file)
        return;

    // Make the header line
    {
        bool firstField = true;
        for (const StructField& field : structDesc.fields)
        {
            DataFieldTypeInfoStruct fieldInfo = DataFieldTypeInfo(field.type);
            int componentCount = fieldInfo.componentCount;
            if (componentCount > 1)
            {
                for (int i = 0; i < componentCount; ++i)
                {
                    fprintf(file, "%s\"%s%i\"", (firstField ? "" : ","), field.name.c_str(), i);
                    firstField = false;
                }
            }
            else
            {
                fprintf(file, "%s\"%s\"", (firstField ? "" : ","), field.name.c_str());
                firstField = false;
            }
        }
        fprintf(file, "\n");
    }

    const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();

    int offset = 0;
    for (int index = 0; index < count; ++index)
    {
        bool firstField = true;
        for (const StructField& field : structDesc.fields)
        {
            DataFieldTypeInfoStruct fieldInfo = DataFieldTypeInfo(field.type);

            switch (fieldInfo.componentType)
            {
                case DataFieldComponentType::_int:
                {
                    int* data = (int*)&bytes[offset];
                    for (int i = 0; i < fieldInfo.componentCount; ++i)
                    {
                        if (field.enumIndex >= 0)
                            fprintf(file, "%s\"%s\"", (firstField ? "" : ","), renderGraph.enums[field.enumIndex].items[data[i]].displayLabel.c_str());
                        else
                            fprintf(file, "%s\"%i\"", (firstField ? "" : ","), data[i]);
                        firstField = false;
                    }
                    break;
                }
                case DataFieldComponentType::_uint16_t:
                {
                    uint16_t* data = (uint16_t*)&bytes[offset];
                    for (int i = 0; i < fieldInfo.componentCount; ++i)
                    {
                        fprintf(file, "%s\"%u\"", (firstField ? "" : ","), data[i]);
                        firstField = false;
                    }
                    break;
                }
                case DataFieldComponentType::_uint32_t:
                {
                    uint32_t* data = (uint32_t*)&bytes[offset];
                    for (int i = 0; i < fieldInfo.componentCount; ++i)
                    {
                        if (field.type == DataFieldType::Bool)
                            fprintf(file, "%s\"%s\"", (firstField ? "" : ","), data[i] ? "True" : "False");
                        else
                            fprintf(file, "%s\"%u\"", (firstField ? "" : ","), data[i]);
                        firstField = false;
                    }
                    break;
                }
                case DataFieldComponentType::_float:
                {
                    float* data = (float*)&bytes[offset];
                    for (int i = 0; i < fieldInfo.componentCount; ++i)
                    {
                        fprintf(file, "%s\"%f\"", (firstField ? "" : ","), data[i]);
                        firstField = false;
                    }
                    break;
                }
                default:
                {
                    Assert(false, "Unhandled DataFieldComponentType");
                }
            }

            offset += (int)field.sizeInBytes;
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void ShowTypedBuffer(unsigned char* bytes, DXGI_FORMAT format, int formatCount, int count, bool showAsHex)
{
    DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(format);

    int actualChannelCount = formatCount * formatInfo.channelCount;

    ImGui::Text("Type: %s", formatInfo.name);
    ImGui::Text("Data:");

    // calculate how many pages there are
    int numPages = (count + c_bufferRecordsPerPageHorizontal - 1) / c_bufferRecordsPerPageHorizontal;
    static int currentPage = 0;
    if (currentPage < 0 || currentPage >= numPages)
        currentPage = 0;

    // Show the pages
    if (numPages > 1)
    {
        ImGui::Text("Page:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(75);
        ImGui::InputInt("##Page", &currentPage, 0);

        ImGui::SameLine();
        if (ImGui::SmallButton("<<"))
            currentPage = 0;

        ImGui::SameLine();
        if (ImGui::SmallButton("<"))
            currentPage--;

        int minPage = std::max(currentPage - 5, 0);
        int maxPage = std::min(currentPage + 5, numPages - 1);

        for (int pageIndex = minPage; pageIndex <= maxPage; ++pageIndex)
        {
            char label[256];
            sprintf_s(label, "%i", pageIndex);
            ImGui::SameLine();

            if (pageIndex == currentPage)
            {
                ImGui::Text(label);
            }
            else
            {
                if (ImGui::SmallButton(label))
                    currentPage = pageIndex;
            }
        }

        ImGui::SameLine();
        if (ImGui::SmallButton(">"))
            currentPage++;

        ImGui::SameLine();
        if (ImGui::SmallButton(">>"))
            currentPage = numPages - 1;

        // make sure the current page is in range
        currentPage = std::max(currentPage, 0);
        currentPage = std::min(currentPage, numPages - 1);
    }

    if (ImGui::BeginTable("fields", 2 + actualChannelCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
        for (int i = 0; i < actualChannelCount; ++i)
        {
            char buffer[256];
            if (actualChannelCount > 1)
                sprintf_s(buffer, "Value %i", i);
            else
                sprintf_s(buffer, "Value");
            ImGui::TableSetupColumn(buffer , ImGuiTableColumnFlags_WidthStretch);
        }
        ImGui::TableHeadersRow();

        int indexStart = currentPage * c_bufferRecordsPerPageHorizontal;
        int indexEnd = std::min(count, indexStart + c_bufferRecordsPerPageHorizontal);

        int rowBytes = 0;
        switch (formatInfo.channelType)
        {
            case DXGI_FORMAT_Info::ChannelType::_uint8_t: rowBytes = sizeof(uint8_t) * actualChannelCount; break;
            case DXGI_FORMAT_Info::ChannelType::_uint16_t: rowBytes = sizeof(uint16_t) * actualChannelCount; break;
            case DXGI_FORMAT_Info::ChannelType::_uint32_t: rowBytes = sizeof(uint32_t) * actualChannelCount; break;
            case DXGI_FORMAT_Info::ChannelType::_int8_t: rowBytes = sizeof(int8_t) * actualChannelCount; break;
            case DXGI_FORMAT_Info::ChannelType::_int16_t: rowBytes = sizeof(int16_t) * actualChannelCount; break;
            case DXGI_FORMAT_Info::ChannelType::_int32_t: rowBytes = sizeof(int32_t) * actualChannelCount; break;
            case DXGI_FORMAT_Info::ChannelType::_float: rowBytes = sizeof(float) * actualChannelCount; break;
            case DXGI_FORMAT_Info::ChannelType::_half: rowBytes = sizeof(half) * actualChannelCount; break;
        }

        for (int index = indexStart; index < indexEnd; ++index)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%i", index);

            ImGui::TableNextColumn();
            ImGui::Text("%i", index * rowBytes);

            unsigned char* rowBegin = &bytes[index * rowBytes];

            switch (formatInfo.channelType)
            {
                case DXGI_FORMAT_Info::ChannelType::_uint8_t:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text((showAsHex ? "%.2X" : "%u"), (unsigned int)((uint8_t*)rowBegin)[c]);
                    }
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_uint16_t:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text((showAsHex ? "%.4X" : "%u"), (unsigned int)((uint16_t*)rowBegin)[c]);
                    }
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_uint32_t:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text((showAsHex ? "%.8X" : "%u"), (unsigned int)((uint32_t*)rowBegin)[c]);
                    }
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_int8_t:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text((showAsHex ? "%.2X" : "%i"), (unsigned int)((int8_t*)rowBegin)[c]);
                    }
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_int16_t:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text((showAsHex ? "%.4X" : "%i"), (unsigned int)((int16_t*)rowBegin)[c]);
                    }
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_int32_t:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text((showAsHex ? "%.8X" : "%i"), (unsigned int)((int32_t*)rowBegin)[c]);
                    }
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_float:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        if(showAsHex)
                            ImGui::Text("%.8X", ((unsigned int*)rowBegin)[c]);
                        else
                            ImGui::Text("%f", ((float*)rowBegin)[c]);
                    }
                    break;
                }
                case DXGI_FORMAT_Info::ChannelType::_half:
                {
                    for (int c = 0; c < actualChannelCount; ++c)
                    {
                        ImGui::TableNextColumn();
                        if (showAsHex)
                            ImGui::Text("%.8X", (float)((half*)rowBegin)[c]);
                        else
                            ImGui::Text("%f", (float)((half*)rowBegin)[c]);
                    }
                    break;
                }
            }
        }

        ImGui::EndTable();
    }
}

void ShowStructuredBuffer(const RenderGraph& renderGraph, unsigned char* bytes, const Struct& structDesc, int count, bool showAsHex, bool showVertically)
{
    int bufferRecordsPerPage = showVertically ? c_bufferRecordsPerPageVertical : c_bufferRecordsPerPageHorizontal;

    {
        int offset = 0;
        ImGui::Text("Type: Struct (%i bytes)", (int)structDesc.sizeInBytes);
        if (ImGui::BeginTable("desc", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Semantic", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Comment", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableHeadersRow();

            for (const StructField& field : structDesc.fields)
            {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%i", offset);

                ImGui::TableNextColumn();
                ImGui::Text("%s", EnumToString(field.type));

                ImGui::TableNextColumn();
                ImGui::Text("%s", field.name.c_str());

                ImGui::TableNextColumn();
                if (field.semantic != StructFieldSemantic::Count)
                    ImGui::Text("%s %i", EnumToString(field.semantic), field.semanticIndex);

                ImGui::TableNextColumn();
                ImGui::Text("%s", field.comment.c_str());

                offset += (int)field.sizeInBytes;
            }

            ImGui::EndTable();
        }
    }

    // Show the data
    ImGui::Text("Data:");

    // Calculate how many pages there are
    int numPages = (count + bufferRecordsPerPage - 1) / bufferRecordsPerPage;
    static int currentPage = 0;
    if (currentPage < 0 || currentPage >= numPages)
        currentPage = 0;

    // Show the pages
    if (numPages > 1)
    {
        ImGui::Text("Page:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(75);
        ImGui::InputInt("##Page", &currentPage, 0);

        ImGui::SameLine();
        if (ImGui::SmallButton("<<"))
            currentPage = 0;

        ImGui::SameLine();
        if (ImGui::SmallButton("<"))
            currentPage--;

        int minPage = std::max(currentPage - 5, 0);
        int maxPage = std::min(currentPage + 5, numPages - 1);

        for (int pageIndex = minPage; pageIndex <= maxPage; ++pageIndex)
        {
            char label[256];
            sprintf_s(label, "%i", pageIndex);
            ImGui::SameLine();

            if (pageIndex == currentPage)
            {
                ImGui::Text(label);
            }
            else
            {
                if (ImGui::SmallButton(label))
                    currentPage = pageIndex;
            }
        }

        ImGui::SameLine();
        if (ImGui::SmallButton(">"))
            currentPage++;

        ImGui::SameLine();
        if (ImGui::SmallButton(">>"))
            currentPage = numPages - 1;

        // make sure the current page is in range
        currentPage = std::max(currentPage, 0);
        currentPage = std::min(currentPage, numPages - 1);
    }

    int indexStart = currentPage * bufferRecordsPerPage;
    int indexEnd = std::min(count, indexStart + bufferRecordsPerPage);
    int rowBytes = (int)structDesc.sizeInBytes;

    if (showVertically)
    {
        if (ImGui::BeginTable("fields", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableHeadersRow();

            for (int index = indexStart; index < indexEnd; ++index)
            {
                unsigned char* fieldBytes = &bytes[index * rowBytes];
                int rowOffset = 0;

                for (const StructField& field : structDesc.fields)
                {
                    DataFieldTypeInfoStruct fieldInfo = DataFieldTypeInfo(field.type);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%i", index);

                    ImGui::TableNextColumn();
                    ImGui::Text("%i", index * rowBytes + rowOffset);

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(field.name.c_str());

                    ImGui::TableNextColumn();
                    switch (fieldInfo.componentType)
                    {
                        case DataFieldComponentType::_int:
                        {
                            int* data = (int*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                if (field.enumIndex >= 0)
                                {
                                    const Enum& e = renderGraph.enums[field.enumIndex];
                                    const char* displayLabel = (data[i] < e.items.size()) ? e.items[data[i]].displayLabel.c_str() : "<invalid>";
                                    ImGui::Text((showAsHex ? "%.8X (%s)" : "%i (%s)"), data[i], displayLabel);
                                }
                                else
                                    ImGui::Text((showAsHex ? "%.8X" : "%i"), data[i]);
                            }
                            break;
                        }
                        case DataFieldComponentType::_uint16_t:
                        {
                            uint16_t* data = (uint16_t*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                ImGui::Text((showAsHex ? "%.4X" : "%u"), (unsigned int)data[i]);
                            }
                            break;
                        }
                        case DataFieldComponentType::_uint32_t:
                        {
                            uint32_t* data = (uint32_t*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                if (field.type == DataFieldType::Bool && !showAsHex)
                                    ImGui::Text("%s", data[i] ? "True" : "False");
                                else
                                    ImGui::Text((showAsHex ? "%.8X" : "%u"), (unsigned int)data[i]);
                            }
                            break;
                        }
                        case DataFieldComponentType::_float:
                        {
                            int sameLineCount = 1;

                            switch (field.type)
                            {
                                case DataFieldType::Float2: sameLineCount = 2; break;
                                case DataFieldType::Float3: sameLineCount = 3; break;
                                case DataFieldType::Float4: sameLineCount = 4; break;
                                case DataFieldType::Float4x4: sameLineCount = 4; break;
                            }

                            float* data = (float*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                if (showAsHex)
                                    ImGui::Text("%.8X", *(unsigned int*)&data[i]);
                                else
                                    ImGui::Text("%f", data[i]);

                                if ((i + 1 < componentCount) && (i % sameLineCount) != sameLineCount - 1)
                                    ImGui::SameLine();
                            }
                            break;
                        }
                    }

                    fieldBytes += (int)field.sizeInBytes;

                    rowOffset += fieldInfo.typeBytes;
                }
            }

            ImGui::EndTable();
        }
    }
    else
    {
        int columnCount = 0;
        for (const StructField& field : structDesc.fields)
            columnCount += DataFieldTypeInfo(field.type).componentCount;

        if (ImGui::BeginTable("fields", 2 + columnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);

            for (const StructField& field : structDesc.fields)
            {
                int componentCount = DataFieldTypeInfo(field.type).componentCount;
                for (int i = 0; i < componentCount; ++i)
                {
                    char buffer[256];
                    if (componentCount > 1)
                        sprintf_s(buffer, "%s %i", field.name.c_str(), i);
                    else
                        sprintf_s(buffer, "%s", field.name.c_str());
                    ImGui::TableSetupColumn(buffer, ImGuiTableColumnFlags_WidthStretch);
                }
            }

            ImGui::TableHeadersRow();

            for (int index = indexStart; index < indexEnd; ++index)
            {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%i", index);

                ImGui::TableNextColumn();
                ImGui::Text("%i", index * rowBytes);

                unsigned char* fieldBytes = &bytes[index * rowBytes];

                for (const StructField& field : structDesc.fields)
                {
                    DataFieldTypeInfoStruct fieldInfo = DataFieldTypeInfo(field.type);

                    switch (fieldInfo.componentType)
                    {
                        case DataFieldComponentType::_int:
                        {
                            int* data = (int*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                ImGui::TableNextColumn();
                                if (field.enumIndex >= 0)
                                {
                                    const Enum& e = renderGraph.enums[field.enumIndex];
                                    const char* displayLabel = (data[i] < e.items.size()) ? e.items[data[i]].displayLabel.c_str() : "<invalid>";
                                    ImGui::Text((showAsHex ? "%.8X (%s)" : "%i (%s)"), data[i], displayLabel);
                                }
                                else
                                    ImGui::Text((showAsHex ? "%.8X" : "%i"), data[i]);
                            }
                            break;
                        }
                        case DataFieldComponentType::_uint16_t:
                        {
                            uint16_t* data = (uint16_t*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                ImGui::TableNextColumn();
                                ImGui::Text((showAsHex ? "%.4X" : "%u"), (unsigned int)data[i]);
                            }
                            break;
                        }
                        case DataFieldComponentType::_uint32_t:
                        {
                            uint32_t* data = (uint32_t*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                ImGui::TableNextColumn();
                                if (field.type == DataFieldType::Bool && !showAsHex)
                                    ImGui::Text("%s", data[i] ? "True" : "False");
                                else
                                    ImGui::Text((showAsHex ? "%.8X" : "%u"), (unsigned int)data[i]);
                            }
                            break;
                        }
                        case DataFieldComponentType::_float:
                        {
                            float* data = (float*)fieldBytes;
                            int componentCount = fieldInfo.componentCount;
                            for (int i = 0; i < componentCount; ++i)
                            {
                                ImGui::TableNextColumn();
                                if (showAsHex)
                                    ImGui::Text("%.8X", *(unsigned int*)&data[i]);
                                else
                                    ImGui::Text("%f", data[i]);
                            }
                            break;
                        }
                    }

                    fieldBytes += (int)field.sizeInBytes;
                }
            }
            ImGui::EndTable();
        }
    }
}

void CopyImageToClipBoard(ID3D12Resource* readbackResource, D3D12_RESOURCE_DESC resourceOriginalDesc, int zIndex, int mipIndex)
{
    DXGI_FORMAT decodedFormat;
    std::vector<unsigned char> decodedPixels;
    int decodedWidth = 0;
    int decodedHeight = 0;
    if (!ImageReadback::GetDecodedImageSlice(g_pd3dDevice, readbackResource, resourceOriginalDesc, zIndex, mipIndex, decodedPixels, decodedFormat, decodedWidth, decodedHeight))
    {
        Log(LogLevel::Error, __FUNCTION__ " Could not decode block compressed image\n");
        return;
    }

    // If the decoded format isn't RGBAU8, convert it to that!
    DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
    if (decodedFormatInfo.channelCount != 4 || decodedFormatInfo.channelType != DXGI_FORMAT_Info::ChannelType::_uint8_t)
    {
        // Convert
        std::vector<double> scratchMemory;
        std::vector<unsigned char> convertedPixels(decodedWidth * decodedHeight * 4);
        for (size_t pixelIndex = 0; pixelIndex < decodedWidth * decodedHeight; ++pixelIndex)
        {
            unsigned char* destPixel = &convertedPixels[pixelIndex * 4];
            destPixel[0] = 0;
            destPixel[1] = 0;
            destPixel[2] = 0;
            destPixel[3] = 255;

            for (size_t channelIndex = 0; channelIndex < decodedFormatInfo.channelCount; ++channelIndex)
            {
                unsigned char* srcPixel = &decodedPixels[pixelIndex * decodedFormatInfo.bytesPerPixel + channelIndex * decodedFormatInfo.bytesPerChannel];
                switch (decodedFormatInfo.channelType)
                {
                    case DXGI_FORMAT_Info::ChannelType::_uint8_t: destPixel[channelIndex] = *srcPixel; break;
                    default:
                    {
                        if (!ConvertToDoubles(srcPixel, 1, decodedFormatInfo.channelType, scratchMemory))
                        {
                            Log(LogLevel::Error, __FUNCTION__ " Could not convert format \"%s\" to doubles\n", decodedFormatInfo.name);
                            return;
                        }

                        // linear to sRGB conversion
                        if (channelIndex != 3)
                            scratchMemory[0] = LinearTosRGB(scratchMemory[0]);

                        if(!ConvertFromDoubles(scratchMemory, DXGI_FORMAT_Info::ChannelType::_uint8_t, &destPixel[channelIndex]))
                        {
                            Log(LogLevel::Error, __FUNCTION__ " Could not convert from doubles to uint8\n");
                            return;
                        }

                        break;
                    }
                }
            }
        }

        // set the new pixels
        decodedPixels = convertedPixels;

        // remember that our pixels are the new format
        decodedFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
    }

    // can we make this work with pixel formats other than U8? (like HDR images, i guess)
    constexpr uint32_t kBytesPerPixel = 4;
    constexpr WORD kBitsPerPixel = 8 * kBytesPerPixel;
    BITMAPV5HEADER header = {
        .bV5Size = sizeof(header),
        .bV5Width = decodedWidth,
        .bV5Height = decodedHeight,
        .bV5Planes = 1,
        .bV5BitCount = kBitsPerPixel,
        .bV5Compression = BI_BITFIELDS, // Should this be BI_RGB?
        .bV5RedMask = 0x000000ff,
        .bV5GreenMask = 0x0000ff00,
        .bV5BlueMask = 0x00ff0000,
        .bV5AlphaMask = 0xff000000,
        .bV5CSType = LCS_WINDOWS_COLOR_SPACE,  // Should this be LCS_sRGB sometimes?
    };

    HGLOBAL hglob = GlobalAlloc(GMEM_MOVEABLE, sizeof(header) + decodedWidth * decodedHeight * 4);
    if (hglob)
    {
        void* buffer = GlobalLock(hglob);
        if (buffer)
        {
            CopyMemory(buffer, &header, sizeof(header));

            for (int y = 0; y < decodedHeight; ++y)
            {
                const unsigned char* src = &decodedPixels[y * decodedWidth * decodedFormatInfo.bytesPerPixel];
                unsigned char* dest = (unsigned char*)buffer + sizeof(header) + (decodedHeight - 1 - y) * decodedWidth * 4;

                for (int x = 0; x < decodedWidth; ++x)
                {
                    CopyMemory(dest, src, decodedFormatInfo.bytesPerPixel);

                    switch (decodedFormatInfo.channelCount)
                    {
                        case 1: dest[1] = 0;
                        case 2: dest[2] = 0;
                        case 3: dest[3] = 1;
                    }

                    src += decodedFormatInfo.bytesPerPixel;
                    dest += 4;
                }
            }

            GlobalUnlock(hglob);
        }
        else
        {
            GlobalFree(hglob);
        }

        if (OpenClipboard(g_hwnd))
        {
            EmptyClipboard();
            SetClipboardData(CF_DIBV5, hglob);
            CloseClipboard();
        }
        else
        {
            GlobalFree(hglob);
        }
    }
}

void ShowResourceView()
{
    // See if we have a texture selected, so we can turn off mouse wheel scroll if so, since that contols zoom.
    bool textureSelected = false;
    const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
    if (g_resourceView.nodeIndex >= 0 && g_resourceView.nodeIndex < renderGraph.nodes.size() && g_resourceView.resourceIndex >= 0)
    {
        const RenderGraphNode& node = renderGraph.nodes[g_resourceView.nodeIndex];
        textureSelected = node._index == RenderGraphNode::c_index_resourceTexture;
    }

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_HorizontalScrollbar;

    if (textureSelected)
        windowFlags |= ImGuiWindowFlags_NoScrollWithMouse; // good for 2d resource mouse wheel zoom but maybe not for other types

    if (!ImGui::Begin("Resource View", nullptr, windowFlags))
    {
        ImGui::End();
        return;
    }

    // show snapshot drop down, even when UI is hidden, as that user may want to use it.
    {
        std::vector<const char*> snapshots;
        for (const GGUserFileV2Snapshot& snapshot : g_userSnapshots)
            snapshots.push_back(snapshot.name.c_str());

        std::sort(snapshots.begin(), snapshots.end(), [](const char* A, const char* B) { return strcmp(A, B) < 0; });

        float comboWidth = ImGui::CalcTextSize("Snapshots").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);

        int deleteIndex = -1;
        if (ImGui::BeginCombo("##Snapshots", "Snapshots", ImGuiComboFlags_HeightLargest))
        {
            if (ImGui::BeginTable("snapshots", 2, ImGuiTableFlags_None))
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);

                for (int snapShotNameIndex = 0; snapShotNameIndex < snapshots.size(); ++snapShotNameIndex)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    const char* snapshotName = snapshots[snapShotNameIndex];

                    ImGui::Text(snapshotName);

                    ImGui::TableNextColumn();

                    ImGui::PushID(snapShotNameIndex);

                    for (int i = 0; i < g_userSnapshots.size(); ++i)
                    {
                        if (!strcmp(g_userSnapshots[i].name.c_str(), snapshotName))
                        {
                            ImGui::Checkbox("Vars", &g_userSnapshots[i].loadVars);
                            ImGui::SameLine();

                            ImGui::Checkbox("Camera", &g_userSnapshots[i].loadCamera);
                            ImGui::SameLine();

                            ImGui::Checkbox("Resources", &g_userSnapshots[i].loadResources);
                            ImGui::SameLine();

                            ImGui::Checkbox("View", &g_userSnapshots[i].loadView);
                            ImGui::SameLine();

                            break;
                        }
                    }

                    if (ImGui::Button("Load"))
                    {
                        for (int i = 0; i < g_userSnapshots.size(); ++i)
                        {
                            if (!strcmp(g_userSnapshots[i].name.c_str(), snapshotName))
                            {
                                if (g_userSnapshots[i].loadView) // make this work with the A/B button
                                    g_resourceView.StoreLast();

                                ScatterSnapshotData(g_userSnapshots[i], true, g_userSnapshots[i].loadCamera, g_userSnapshots[i].loadView, g_userSnapshots[i].loadResources);

                                if (g_userSnapshots[i].loadVars)
                                    ScatterSnapshotVariables(g_userSnapshots[i]);
                                break;
                            }
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Save"))
                    {
                        for (int i = 0; i < g_userSnapshots.size(); ++i)
                        {
                            if (!strcmp(g_userSnapshots[i].name.c_str(), snapshotName))
                            {
                                GatherSnapshotData(g_userSnapshots[i]);
                                break;
                            }
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete"))
                        deleteIndex = snapShotNameIndex;

                    ImGui::PopID();
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                static char newSnapshotName[512] = { 0 };
                ImGui::InputText("##NewSnapshot", newSnapshotName, _countof(newSnapshotName));
                ImGui::SameLine();
                if (ImGui::Button("Save") && newSnapshotName[0] != 0)
                {
                    int existingIndex = -1;
                    for (int i = 0; i < g_userSnapshots.size(); ++i)
                    {
                        if (!strcmp(g_userSnapshots[i].name.c_str(), newSnapshotName))
                        {
                            existingIndex = i;
                            break;
                        }
                    }

                    if (existingIndex == -1)
                    {
                        existingIndex = (int)g_userSnapshots.size();
                        g_userSnapshots.resize(existingIndex + 1);
                        g_userSnapshots[existingIndex].name = newSnapshotName;
                    }

                    GatherSnapshotData(g_userSnapshots[existingIndex]);

                    newSnapshotName[0] = 0;
                }

                ImGui::TableNextColumn();

                ImGui::EndTable();

                ImGui::EndCombo();

                if (deleteIndex >= 0)
                {
                    g_userSnapshots.erase(
                        std::remove_if(g_userSnapshots.begin(), g_userSnapshots.end(),
                            [&](const GGUserFileV2Snapshot& snapshot)
                            {
                                return snapshot.name == snapshots[deleteIndex];
                            }
                        ),
                        g_userSnapshots.end()
                    );
                }
            }
        }
    }

    // show the selected resource
    if (g_resourceView.nodeIndex >= 0 && g_resourceView.nodeIndex < renderGraph.nodes.size() && g_resourceView.resourceIndex >= 0)
    {
        // Get information from the node and node runtime data
        std::vector<RuntimeTypes::ViewableResource>* viewableResources_ = nullptr;
        std::string nodeName;
        g_interpreter.RuntimeNodeDataLambda(
            renderGraph.nodes[g_resourceView.nodeIndex],
            [&](auto node, auto* runtimeData)
            {
                nodeName = node.name;
                if (runtimeData)
                    viewableResources_ = &(runtimeData->m_viewableResources);
            }
        );

        if (viewableResources_ && viewableResources_->size() > g_resourceView.resourceIndex)
        {
            RuntimeTypes::ViewableResource& res = (*viewableResources_)[g_resourceView.resourceIndex];
            if (!g_profileMode)
            {
                res.m_wantsToBeViewed = true;
                res.m_wantsToBeReadBack = true;
            }

            if (!g_hideUI)
            {
                ImGui::SameLine();
                if (ImGui::Button("A/B"))
                    g_resourceView.ViewLast();
            }

            if (!g_hideUI)
            {
                const char* typeLabel = "";
                switch (res.m_type)
                {
                    case RuntimeTypes::ViewableResource::Type::Texture2D: typeLabel = "Texture2D"; break;
                    case RuntimeTypes::ViewableResource::Type::Texture2DArray: typeLabel = "Texture2DArray"; break;
                    case RuntimeTypes::ViewableResource::Type::Texture2DMS: typeLabel = "Texture2DMS"; break;
                    case RuntimeTypes::ViewableResource::Type::Texture3D: typeLabel = "Texture3D"; break;
                    case RuntimeTypes::ViewableResource::Type::TextureCube: typeLabel = "TextureCube"; break;
                    case RuntimeTypes::ViewableResource::Type::ConstantBuffer: typeLabel = "ConstantBuffer"; break;
                    case RuntimeTypes::ViewableResource::Type::Buffer: typeLabel = "Buffer"; break;
                }
                ImGui::Text("%s: %s", typeLabel, res.m_displayName.empty() ? nodeName.c_str() : res.m_displayName.c_str());

//                ImGui::SameLine();
                if (ImGui::Button("Copy Name"))
                    SetClipboardDataEx(CF_TEXT, (void*)res.m_displayName.c_str(), (DWORD)res.m_displayName.length() + 1);
            }

            if(res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DMS)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "\n  MSAA preview is not implemented yet.\n  You can implement a custom resolve shader.");
                ImGui::End();
                return;
            }

            static bool showAsHex = false;
            static bool showBufferView = true;

            if (res.m_resource)
            {
                switch (res.m_type)
                {
                    case RuntimeTypes::ViewableResource::Type::Texture2D:
                    case RuntimeTypes::ViewableResource::Type::Texture2DArray:
                    case RuntimeTypes::ViewableResource::Type::Texture3D:
                    case RuntimeTypes::ViewableResource::Type::TextureCube:
                    case RuntimeTypes::ViewableResource::Type::Texture2DMS:
                    {
                        if (!g_hideUI)
                        {
                            // Figure out what kind of texture data we have, once it's decoded (if it's block compressed)
                            DXGI_FORMAT decodedFormat = ImageReadback::GetDecodedFormat(res.m_format);
                            DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);

                            // Images can always be copied
                            ImGui::SameLine();
                            if (ImGui::Button("Copy Image"))
                                CopyImageToClipBoard(res.m_resourceReadback, res.m_origResourceDesc, res.m_arrayIndex, res.m_mipIndex);

                            // The types that can be saved as
                            enum class SaveAsType
                            {
                                PNG,
                                CSV,
                                EXR,
                                HDR,
                                DDS_BC4,
                                DDS_BC5,
                                DDS_BC6,
                                DDS_BC7,
                                Binary,
                            };

                            struct SaveAsTypeInfo
                            {
                                const char* label;
                                const char* extension;
                            };

                            static const SaveAsTypeInfo c_saveAsTypeInfo[] =
                            {
                                { "png", "png" },
                                { "csv", "csv" },
                                { "exr", "exr" },
                                { "hdr", "hdr" },
                                { "dds (BC4)", "dds" },
                                { "dds (BC5)", "dds" },
                                { "dds (BC6)", "dds" },
                                { "dds (BC7)", "dds" },
                                { "binary", "bin" },
                            };

                            // figure out what we can save as
                            bool canSaveAsPNG_BC457 = (decodedFormatInfo.channelType == DXGI_FORMAT_Info::ChannelType::_uint8_t);
                            bool canSaveAsEXR_HDR_BC6 = (decodedFormatInfo.channelType == DXGI_FORMAT_Info::ChannelType::_float);

                            // make the option list
                            std::vector<SaveAsType> options;
                            if (canSaveAsPNG_BC457)
                            {
                                options.push_back(SaveAsType::PNG);
                                options.push_back(SaveAsType::DDS_BC4);
                                options.push_back(SaveAsType::DDS_BC5);
                                options.push_back(SaveAsType::DDS_BC7);
                            }
                            if (canSaveAsEXR_HDR_BC6)
                            {
                                options.push_back(SaveAsType::EXR);
                                options.push_back(SaveAsType::HDR);
                                options.push_back(SaveAsType::DDS_BC6);
                            }
                            options.push_back(SaveAsType::CSV);
                            options.push_back(SaveAsType::Binary);

                            // figure out which is selected
                            static std::string s_saveOptionStr;
                            SaveAsType saveAsType = options[0];
                            int saveAsOptionIndex = 0;
                            for (size_t optionIndex = 0; optionIndex < options.size(); ++optionIndex)
                            {
                                if (c_saveAsTypeInfo[(int)options[optionIndex]].label == s_saveOptionStr)
                                {
                                    saveAsType = options[optionIndex];
                                    saveAsOptionIndex = (int)optionIndex;
                                }
                            }

                            // Show the UI and do the work
                            {
                                static bool s_saveAll = false;
                                static bool s_bc45_signed = false;
                                static bool s_bc6_signed = false;
                                static bool s_bc7_srgb = true;

                                ImGui::SameLine();

                                ImGuiStyle& style = ImGui::GetStyle();
                                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

                                std::string saveAsStr = "Save as " + std::string(c_saveAsTypeInfo[(int)saveAsType].label);
                                nfdchar_t* outPath = nullptr;
                                if (ImGui::Button(saveAsStr.c_str()) && NFD_SaveDialog(c_saveAsTypeInfo[(int)saveAsType].extension, "", &outPath) == NFD_OKAY)
                                {
                                    ImageSave::Options saveOptions;
                                    saveOptions.zIndex = res.m_arrayIndex;
                                    saveOptions.mipIndex = res.m_mipIndex;
                                    saveOptions.saveAll = s_saveAll;
                                    saveOptions.isCubeMap = (res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube);
                                    saveOptions.bc45.isSigned = s_bc45_signed;
                                    saveOptions.bc6.isSigned = s_bc6_signed;
                                    saveOptions.bc7.sRGB = s_bc7_srgb;

                                    bool success = false;
                                    switch (saveAsType)
                                    {
                                        case SaveAsType::PNG: success = ImageSave::SaveAsPng(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::CSV: success = ImageSave::SaveAsCSV(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::EXR: success = ImageSave::SaveAsEXR(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::HDR: success = ImageSave::SaveAsHDR(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::DDS_BC4: success = ImageSave::SaveAsDDS_BC4(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::DDS_BC5: success = ImageSave::SaveAsDDS_BC5(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::DDS_BC6: success = ImageSave::SaveAsDDS_BC6(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::DDS_BC7: success = ImageSave::SaveAsDDS_BC7(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                        case SaveAsType::Binary: success = ImageSave::SaveAsBinary(outPath, g_pd3dDevice, res.m_resourceReadback, res.m_origResourceDesc, saveOptions); break;
                                    }

                                    if (!success)
                                    {
                                        Log(LogLevel::Error, __FUNCTION__ " Could not save file \"%s\"\n", outPath);
                                    }
                                }

                                ImGui::SameLine();
                                if (ImGui::ArrowButton("##ChooseSaveAs", ImGuiDir_Down))
                                    ImGui::OpenPopup("PopupChooseSaveAs");

                                ImGui::PopStyleVar(1);

                                if (saveAsType == SaveAsType::DDS_BC4 || saveAsType == SaveAsType::DDS_BC5)
                                {
                                    ImGui::SameLine();
                                    ImGui::Checkbox("Signed", &s_bc45_signed);
                                    ShowToolTip("If true, saves as snorm, else saves as unorm");
                                }

                                if (saveAsType == SaveAsType::DDS_BC6)
                                {
                                    ImGui::SameLine();
                                    ImGui::Checkbox("Signed", &s_bc6_signed);
                                    ShowToolTip("If true, saves as DXGI_FORMAT_BC6H_SF16, else saves as DXGI_FORMAT_BC6H_UF16.");
                                }

                                if (saveAsType == SaveAsType::DDS_BC7)
                                {
                                    ImGui::SameLine();
                                    ImGui::Checkbox("sRGB", &s_bc7_srgb);
                                    ShowToolTip("If true, saves as DXGI_FORMAT_BC7_UNORM_SRGB, else saves as DXGI_FORMAT_BC7_UNORM.");
                                }

                                if (res.m_size[2] > 1 || res.m_numMips > 1)
                                {
                                    ImGui::SameLine();
                                    ImGui::Checkbox("Save All", &s_saveAll);
                                    ShowToolTip("Saves all slices and mips if true, else only saves the current one");
                                }

                                if (ImGui::BeginPopupContextItem("PopupChooseSaveAs"))
                                {
                                    for (size_t optionIndex = 0; optionIndex < options.size(); ++optionIndex)
                                    {
                                        bool checked = saveAsOptionIndex == optionIndex;
                                        if (ImGui::MenuItem(c_saveAsTypeInfo[(int)options[optionIndex]].label, nullptr, &checked))
                                            s_saveOptionStr = c_saveAsTypeInfo[(int)options[optionIndex]].label;
                                    }

                                    ImGui::EndPopup();
                                }
                            }

                            // Force mouse data to be in range (switching images could make it not be)
                            int mouseHoverX = Clamp(g_resourceView.mousePosImageX, 0, res.m_size[0] - 1);
                            int mouseHoverY = Clamp(g_resourceView.mousePosImageY, 0, res.m_size[1] - 1);
                            int mouseClickX = Clamp(g_resourceView.mouseClickImageX, 0, res.m_size[0] - 1);
                            int mouseClickY = Clamp(g_resourceView.mouseClickImageY, 0, res.m_size[1] - 1);

                            // Show size and format
                            if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2D)
                                ImGui::Text("%ix%i    %s", res.m_size[0], res.m_size[1], Get_DXGI_FORMAT_Info(res.m_origResourceDesc.Format).name);
                            else
                                ImGui::Text("%ix%ix%i    %s", res.m_size[0], res.m_size[1], res.m_size[2], Get_DXGI_FORMAT_Info(res.m_origResourceDesc.Format).name);

                            // Mip drop down
                            if (res.m_numMips > 1)
                            {
                                char buffer[32];
                                float comboWidth = 0.0f;
                                std::vector<std::string> optionsStr;
                                for (int i = 0; i < res.m_numMips; ++i)
                                {
                                    sprintf_s(buffer, "%i", i);
                                    optionsStr.push_back(buffer);
                                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(optionsStr[i].c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
                                }
                                std::vector<const char*> options;
                                for (int i = 0; i < res.m_numMips; ++i)
                                    options.push_back(optionsStr[i].c_str());

                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);

                                ImGui::Combo("Mip", &res.m_mipIndex, options.data(), (int)options.size());
                            }

                            // Z index drop down
                            if (res.m_type != RuntimeTypes::ViewableResource::Type::Texture2D)
                            {
                                const char* label = "Index";
                                switch (res.m_type)
                                {
                                    case RuntimeTypes::ViewableResource::Type::Texture2DArray: label = "Index"; break;
                                    case RuntimeTypes::ViewableResource::Type::Texture3D: label = "Slice"; break;
                                    case RuntimeTypes::ViewableResource::Type::TextureCube: label = "Face"; break;
                                    case RuntimeTypes::ViewableResource::Type::Texture2DMS: label = "MSAA Sample"; break;
                                }

                                float comboWidth = 0.0f;
                                std::vector<std::string> optionsStr;
                                std::vector<const char*> options;
                                if (res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube)
                                {
                                    options.push_back("Right (X+)");
                                    options.push_back("Left (X-)");
                                    options.push_back("Up (Y+)");
                                    options.push_back("Down (Y-)");
                                    options.push_back("Front (Z+)");
                                    options.push_back("Back (Z-)");
                                }
                                else
                                {
                                    int count = (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DMS) ?
                                        res.m_origResourceDesc.SampleDesc.Count : res.m_size[2];

                                    char buffer[32];
                                    for (int i = 0; i < count; ++i)
                                    {
                                        sprintf_s(buffer, "%i", i);
                                        optionsStr.push_back(buffer);
                                    }
                                    for (int i = 0; i < count; ++i)
                                        options.push_back(optionsStr[i].c_str());
                                }

                                for (const char* option : options)
                                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(option).x + ImGui::GetStyle().FramePadding.x * 2.0f);

                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);

                                ImGui::Combo(label, &res.m_arrayIndex, options.data(), (int)options.size());
                            }

                            {
                                float imgui_x = ImGui::GetStyle().WindowPadding.x + ImGui::CalcTextSize("Mouse Hover 12345,12345 ").x;

                                // Show the clicked pixel
                                ImGui::Text("Mouse Click %i,%i", mouseClickX, mouseClickY);
                                ShowPixelValue(res.m_resourceReadback, res.m_origResourceDesc, mouseClickX, mouseClickY, res.m_arrayIndex, res.m_mipIndex, imgui_x, g_resourceView.mouseRegionRadiusPx);

                                // Show the hovered pixel
                                ImGui::Text("Mouse Hover %i,%i", mouseHoverX, mouseHoverY);
                                ShowPixelValue(res.m_resourceReadback, res.m_origResourceDesc, mouseHoverX, mouseHoverY, res.m_arrayIndex, res.m_mipIndex, imgui_x, g_resourceView.mouseRegionRadiusPx);

                                // Clicked pixel radius
                                ImGui::SetNextItemWidth(100.0f);
                                ImGui::InputInt("Mouse Region Radius", &g_resourceView.mouseRegionRadiusPx);
                                ShowToolTip("The half width of the square selected by mouse click or mouse hover. Pixel values shown are averaged over the area", true);
                                g_resourceView.mouseRegionRadiusPx = std::max(g_resourceView.mouseRegionRadiusPx, 0);
                            }

                            {
                                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 0));

                                ImGui_CheckboxButton("R", &g_colorsShown[0], ImVec4(1, 0, 0, 1));
                                ImGui::SameLine();
                                ImGui_CheckboxButton("G", &g_colorsShown[1], ImVec4(0, 1, 0, 1));
                                ImGui::SameLine();
                                ImGui_CheckboxButton("B", &g_colorsShown[2], ImVec4(0, 0, 1, 1));
                                ImGui::SameLine();
                                ImGui_CheckboxButton("A", &g_colorsShown[3], ImVec4(1, 1, 1, 1));

                                ImGui::PopStyleVar();
                            }

                            ImGui::SameLine();
                            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(100.0f);

                            // Histogram min / max controls
                            // g_histogramMinMax[*] are always in [0,1], but the display may show that as [0,65535] or [-128,127] etc.
                            // Note: floating point values can go outside of the 0,1 range (but unorm / snorm values cannot)
                            {
                                // For histogram min / max, if we are dealing with an integer type, show it as an integer
                                float typeMin = 0.0f;
                                float typeMax = 1.0f;
                                bool showAsInteger = false;
                                bool enforceMinMax = false;
                                GetDefaultHistogramRange(res.m_format, typeMin, typeMax, showAsInteger, enforceMinMax);

                                const char* formatString = showAsInteger ? "%.0f" : "%.4f";
                                float sliderMin = enforceMinMax ? typeMin : 0.0f;
                                float sliderMax = enforceMinMax ? typeMax : 0.0f;
                                ImGuiSliderFlags_ flags = enforceMinMax ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None;

                                static float adjustedMinMax[2];
                                if (enforceMinMax)
                                {
                                    adjustedMinMax[0] = Lerp(typeMin, typeMax, Clamp(g_histogramMinMax[0], 0.0f, 1.0f));
                                    adjustedMinMax[1] = Lerp(typeMin, typeMax, Clamp(g_histogramMinMax[1], 0.0f, 1.0f));
                                }
                                else
                                {
                                    adjustedMinMax[0] = Lerp(typeMin, typeMax, g_histogramMinMax[0]);
                                    adjustedMinMax[1] = Lerp(typeMin, typeMax, g_histogramMinMax[1]);
                                }

                                if (ImGui::DragFloatRange2("Histogram", &adjustedMinMax[0], &adjustedMinMax[1], 0.001f * (typeMax - typeMin), sliderMin, sliderMax, formatString, 0, flags))
                                {
                                    g_histogramMinMax[0] = InverseLerp(typeMin, typeMax, adjustedMinMax[0]);
                                    g_histogramMinMax[1] = InverseLerp(typeMin, typeMax, adjustedMinMax[1]);
                                }
                            }

                            ImGui::SameLine();
                            if (ImGui::Button("Auto"))
                                AutoHistogram(res.m_resourceReadback, res.m_origResourceDesc, res.m_arrayIndex, res.m_mipIndex, g_histogramMinMax[0], g_histogramMinMax[1]);
                            ImGui::SameLine();
                            if (ImGui::Button("Reset"))
                            {
                                g_histogramMinMax[0] = 0.0f;
                                g_histogramMinMax[1] = 1.0f;
                            }

                            ImGui::SameLine();
                            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(50.0f);
                            ImGui::InputFloat("Zoom", &g_imageZoom);

                            ImGui::SameLine();
                            if (ImGui::Button("Fit"))
                            {
                                float scaleX = g_contentRegionSize.x / (float)res.m_size[0];
                                float scaleY = g_contentRegionSize.y / (float)res.m_size[1];
                                g_imageZoom = std::min(scaleX, scaleY);
                            }

                            ImGui::SameLine();
                            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                            ImGui::SameLine();
                            ImGui::Checkbox("Linear Filter", &g_imageLinearFilter);

                            // sRGB
                            {
                                static const char* sRGBLabels[] =
                                {
                                    "Auto",
                                    "sRGB",
                                    "Linear"
                                };

                                // Get the longest text width of the server names
                                float comboWidth = 0.0f;
                                for (const char* label : sRGBLabels)
                                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f);

                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
                                const char* selectedLabel = sRGBLabels[(int)g_sRGB];
                                if (ImGui::BeginCombo("sRGB", selectedLabel, ImGuiComboFlags_None))
                                {
                                    for (int n = 0; n < _countof(sRGBLabels); n++)
                                    {
                                        const bool is_selected = ((int)g_sRGB == n);
                                        if (ImGui::Selectable(sRGBLabels[n], is_selected))
                                            g_sRGB = (SRGBSettings)n;

                                        if (is_selected)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }
                            }
                        }

                        g_contentRegionSize = ImGui::GetContentRegionAvail();
                        // Remove the bottom padding to not invoke the scrollbar
                        g_contentRegionSize.y -= ImGui::GetStyle().WindowPadding.y * 0.5f;


                        // scrollable image portion
						ImGui::BeginChild("ChildL", ImGui::GetContentRegionAvail(), 0, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar);

						// Get mouse pos in image coordinates
						ImVec2 resourceLeftTopScreen = ImGui::GetCursorScreenPos();

						// show the image
						DescriptorTableCache::ResourceDescriptor desc;
						desc.m_resource = res.m_resource;
						desc.m_format = res.m_format;

						D3D12_GPU_DESCRIPTOR_HANDLE descTable;
						ImVec2 imagePosition = ImVec2{ 0.0f, 0.0f };
						ImVec2 imageSize = ImVec2{ 0.0f, 0.0f };
                        std::string error;
						if (g_interpreter.GetDescriptorTableCache_ImGui().GetDescriptorTable(g_pd3dDevice, g_interpreter.GetSRVHeapAllocationTracker_ImGui(), &desc, 1, descTable, error, HEAP_DEBUG_TEXT()))
						{
							uint64_t shaderFlags = GetImGuiImageShaderFlags(res.m_format, true);
							ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetShaderFlags, (void*)(shaderFlags));

							float typeMin = 0.0f;
							float typeMax = 1.0f;
                            bool isIntegral = false;
                            bool enforceMinMax = false;
                            GetDefaultHistogramRange(res.m_format, typeMin, typeMax, isIntegral, enforceMinMax);
							float adjustedMinMax[2];
                            if (enforceMinMax)
                            {
                                adjustedMinMax[0] = Lerp(typeMin, typeMax, Clamp(g_histogramMinMax[0], 0.0f, 1.0f));
                                adjustedMinMax[1] = Lerp(typeMin, typeMax, Clamp(g_histogramMinMax[1], 0.0f, 1.0f));
                            }
                            else
                            {
                                adjustedMinMax[0] = Lerp(typeMin, typeMax, g_histogramMinMax[0]);
                                adjustedMinMax[1] = Lerp(typeMin, typeMax, g_histogramMinMax[1]);
                            }
							ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMin, PackFloatIntoPointer(adjustedMinMax[0]));
                            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMax, PackFloatIntoPointer(adjustedMinMax[1]));

							imageSize = ImVec2{ (float)res.m_size[0] * g_imageZoom, (float)res.m_size[1] * g_imageZoom };
							imagePosition = ImGui::GetCursorScreenPos();

                            if (!g_profileMode)
								ImGui::Image((ImTextureID)descTable.ptr, ImVec2{ imageSize }, ImVec2{ 0.0f, 0.0f }, ImVec2{ 1.0f, 1.0f }, ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
							ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetShaderFlags, (void*)0);
							ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMin, PackFloatIntoPointer(0.0f));
                            ImGui::GetCurrentWindow()->DrawList->AddCallback(ImDrawCallback_SetHistogramMax, PackFloatIntoPointer(1.0f));
						}

                        // Draw a rectangle to show the mouse click location region
                        {

                            // Force mouse data to be in range (switching images could make it not be)
                            float mouseHoverX = resourceLeftTopScreen.x + (float)Clamp(g_resourceView.mousePosImageX, 0, res.m_size[0] - 1) * g_imageZoom;
                            float mouseHoverY = resourceLeftTopScreen.y + (float)Clamp(g_resourceView.mousePosImageY, 0, res.m_size[1] - 1) * g_imageZoom;
                            float mouseClickX = resourceLeftTopScreen.x + (float)Clamp(g_resourceView.mouseClickImageX, 0, res.m_size[0] - 1) * g_imageZoom;
                            float mouseClickY = resourceLeftTopScreen.y + (float)Clamp(g_resourceView.mouseClickImageY, 0, res.m_size[1] - 1) * g_imageZoom;

                            float selectionRadius = (float(g_resourceView.mouseRegionRadiusPx) + 1.0f) * g_imageZoom;

                            // Show the mouse click box
                            {
                                ImVec2 startPos = ImVec2{ mouseClickX + g_imageZoom - selectionRadius, mouseClickY + g_imageZoom - selectionRadius };
                                ImVec2 endPos = ImVec2{ mouseClickX + selectionRadius, mouseClickY + selectionRadius };
                                if (endPos.x - startPos.x >= 4.0f)
                                {
                                    static float s_selectionPhase = 0.0f;
                                    s_selectionPhase = std::fmodf(s_selectionPhase + ImGui::GetIO().DeltaTime, 1.0f);
                                    int color = int(32.0f + (sin(s_selectionPhase * 6.28f) * 0.5f + 0.5f) * 191.0f);

                                    ImVec2 imagePositionEnd = ImVec2{ imagePosition.x + imageSize.x, imagePosition.y + imageSize.y };
                                    ImGui::PushClipRect(imagePosition, imagePositionEnd, true);

                                    ImGui::GetCurrentWindow()->DrawList->AddRect(startPos, endPos, ImGui::GetColorU32(IM_COL32(color, color, 0, 128)));

                                    ImGui::PopClipRect();
                                }
                            }

                            // Show the mouse hover box, if there is a radius > 0.
                            if (g_resourceView.mouseRegionRadiusPx > 0)
                            {
                                ImVec2 startPos = ImVec2{ mouseHoverX + g_imageZoom - selectionRadius, mouseHoverY + g_imageZoom - selectionRadius };
                                ImVec2 endPos = ImVec2{ mouseHoverX + selectionRadius, mouseHoverY + selectionRadius };
                                if (endPos.x - startPos.x >= 4.0f)
                                {
                                    static float s_selectionPhase = 0.0f;
                                    s_selectionPhase = std::fmodf(s_selectionPhase + ImGui::GetIO().DeltaTime, 1.0f);
                                    int color = int(32.0f + (sin(s_selectionPhase * 6.28f) * 0.5f + 0.5f) * 191.0f);

                                    ImVec2 imagePositionEnd = ImVec2{ imagePosition.x + imageSize.x, imagePosition.y + imageSize.y };
                                    ImGui::PushClipRect(imagePosition, imagePositionEnd, true);

                                    ImGui::GetCurrentWindow()->DrawList->AddRect(startPos, endPos, ImGui::GetColorU32(IM_COL32(color, color, color, 128)));

                                    ImGui::PopClipRect();
                                }
                            }
                        }

                        // deal with mouse
                        // Only update hover and click location if the mouse is in the window
                        if (ImGui::IsWindowHovered())
                        {
                            ImVec2 mousePos = ImGui::GetMousePos();
                            mousePos[0] -= resourceLeftTopScreen[0];
                            mousePos[1] -= resourceLeftTopScreen[1];
                            //mousePos[1] = res.m_size[1] - mousePos[1];

							ImGuiIO& io = ImGui::GetIO();

                            if (io.MouseWheel)
                            {
                                ImVec2 oldTotalSize(res.m_size[0] * g_imageZoom, res.m_size[1] * g_imageZoom);
                                ImVec2 oldZoomCenterPx = mousePos;
								ImVec2 zoomCenterFrac(oldZoomCenterPx[0] / oldTotalSize[0], oldZoomCenterPx[1] / oldTotalSize[1]);


                                // how fast the mouse wheel is changing the zoom
                                const float basis = 1.25f;

                                float value = logf(g_imageZoom) / logf(basis);
                                value += io.MouseWheel;
                                // snap in log space makes it more deterministic
                                value = round(value);
                                g_imageZoom = powf(basis, value);
								g_imageZoom = ImClamp(g_imageZoom, 0.25f, 32.0f);

								ImVec2 newTotalSize(res.m_size[0] * g_imageZoom, res.m_size[1] * g_imageZoom);
								ImVec2 newZoomCenterPx(zoomCenterFrac[0] * newTotalSize[0], zoomCenterFrac[1] * newTotalSize[1]);
                                ImVec2 delta(newZoomCenterPx[0] - oldZoomCenterPx[0], newZoomCenterPx[1] - oldZoomCenterPx[1]);

                                float scrollX = ImGui::GetScrollX() + delta[0];
                                float scrollY = ImGui::GetScrollY() + delta[1];

                                ImGui::SetScrollX(scrollX);
								ImGui::SetScrollY(scrollY);
                            }

                            // only update hover and click location if the mouse is in range
                            //if (mousePos[0] >= 0 && mousePos[1] >= 0 && mousePos[0] < res.m_size[0] && mousePos[1] < res.m_size[1])
                            {
                                // set the mouse position
                                g_resourceView.mousePosImageX = (int)(mousePos.x / g_imageZoom);
                                g_resourceView.mousePosImageY = (int)(mousePos.y / g_imageZoom);

                                // set the click position if it's set
                                if (io.MouseDown[0])
                                {
                                    g_resourceView.mouseClickImageX = g_resourceView.mousePosImageX;
                                    g_resourceView.mouseClickImageY = g_resourceView.mousePosImageY;
                                }

                                memcpy(g_resourceView.systemVarMouseStateLastFrame, g_resourceView.systemVarMouseState, sizeof(g_resourceView.systemVarMouseState));
                                g_resourceView.systemVarMouseState[0] = float(g_resourceView.mousePosImageX);
                                g_resourceView.systemVarMouseState[1] = float(g_resourceView.mousePosImageY);
                                g_resourceView.systemVarMouseState[2] = io.MouseDown[0] ? 1.0f : 0.0f;
                                g_resourceView.systemVarMouseState[3] = io.MouseDown[1] ? 1.0f : 0.0f;

                                // Handle system variable mouse input
                                g_resourceView.systemVarMouse[2] = 0.0f;
                                g_resourceView.systemVarMouse[3] = 0.0f;
                                if (io.MouseDown[0])
                                {
                                    g_resourceView.systemVarMouse[0] = float(g_resourceView.mousePosImageX);
                                    g_resourceView.systemVarMouse[1] = float(g_resourceView.mousePosImageY);

                                    if (!g_resourceView.mouseWasDownLastFrame)
                                    {
                                        g_resourceView.systemVarMouse[2] = float(g_resourceView.mousePosImageX);
                                        g_resourceView.systemVarMouse[3] = float(g_resourceView.mousePosImageY);
                                    }
                                }
                                g_resourceView.mouseWasDownLastFrame = io.MouseDown[0];
                            }
                        }
                        else
                        {
							g_resourceView.systemVarMouseState[2] = 0.0f;
							g_resourceView.systemVarMouseState[3] = 0.0f;
                        }
						ImGui::EndChild();

                        
                        break;
                    }
                    case RuntimeTypes::ViewableResource::Type::ConstantBuffer:
                    {
                        if (res.m_structIndex >= 0 && res.m_structIndex < renderGraph.structs.size())
                        {
                            const Struct& structDesc = renderGraph.structs[res.m_structIndex];

                            // Read back the data
                            std::vector<unsigned char> bytes(res.m_size[0]);
                            unsigned char* data = nullptr;
                            res.m_resourceReadback->Map(0, nullptr, reinterpret_cast<void**>(&data));
                            memcpy(bytes.data(), data, res.m_size[0]);
                            res.m_resourceReadback->Unmap(0, nullptr);

                            ImGui::Text("View As");
                            ImGui::SameLine();
                            ImGui::Checkbox("Hex", &showAsHex);

                            ImGui::Text("%i bytes (%i bytes padded)", structDesc.sizeInBytes, res.m_size[0]);

                            // Option to save the buffer
                            ImGui::SameLine();
                            nfdchar_t* outPath = nullptr;
                            if (ImGui::Button("Save as .csv") && NFD_SaveDialog("csv", "", &outPath) == NFD_OKAY)
                                SaveAsCSV(outPath, bytes.data(), structDesc, 1);
                            ImGui::SameLine();
                            if (ImGui::Button("Save as .bin") && NFD_SaveDialog("bin", "", &outPath) == NFD_OKAY)
                                SaveAsBin(outPath, bytes.data(), (int)bytes.size());
                            ImGui::SameLine();
                            if (ImGui::Button("Save as .hex") && NFD_SaveDialog("hex", "", &outPath) == NFD_OKAY)
                                SaveAsHex(outPath, bytes.data(), (int)bytes.size());

                            ShowStructuredBuffer(renderGraph, bytes.data(), structDesc, 1, showAsHex, true);
                        }
                        break;
                    }
                    case RuntimeTypes::ViewableResource::Type::Buffer:
                    {
                        // Calculate the view range
                        unsigned int bufferViewBegin = 0;
                        unsigned int bufferViewCount = res.m_count;
                        unsigned int bufferViewItemSize = (res.m_structIndex != -1)
                            ? (unsigned int)renderGraph.structs[res.m_structIndex].sizeInBytes
                            : (unsigned int)Get_DXGI_FORMAT_Info(res.m_format).bytesPerPixel;

                        if (showBufferView && (res.m_bufferViewBegin > 0 || res.m_bufferViewCount > 0))
                        {
                            bufferViewBegin = res.m_bufferViewBegin;
                            bufferViewCount = res.m_bufferViewCount;
                        }

                        // Gather the view info struct data
                        struct ViewInfo
                        {
                            int structIndex = -1;
                            DXGI_FORMAT format = DXGI_FORMAT_FORCE_UINT;
                            int formatCount = 1;
                            int count = 0;
                            int size = 0;
                            bool showAsHex = false;
                            bool showStructuredBuffersVertically = true;

                        };
                        ViewInfo viewInfo;
                        viewInfo.structIndex = res.m_structIndex;
                        viewInfo.format = res.m_format;
                        viewInfo.formatCount = res.m_formatCount;
                        viewInfo.count = bufferViewCount;
                        viewInfo.size = bufferViewCount * bufferViewItemSize;

                        // Read back the data
                        std::vector<unsigned char> bytes(viewInfo.size);
                        {
                            D3D12_RANGE readRange;
                            readRange.Begin = bufferViewBegin * bufferViewItemSize;
                            readRange.End = (bufferViewBegin + bufferViewCount) * bufferViewItemSize;

                            D3D12_RANGE writeRange;
                            writeRange.Begin = 1;
                            writeRange.End = 0;

                            unsigned char* data = nullptr;
                            res.m_resourceReadback->Map(0, &readRange, reinterpret_cast<void**>(&data));
                            memcpy(bytes.data(), &data[readRange.Begin], readRange.End - readRange.Begin);
                            res.m_resourceReadback->Unmap(0, &writeRange);
                        }

                        // handle choosing to view the data differently
                        {
                            ImGui::Text("View As");

                            // Structs
                            {
                                static int viewAsStructIndex = 0;

                                std::vector<std::string> structNames;
                                structNames.push_back("");
                                for (const Struct& s : renderGraph.structs)
                                    structNames.push_back(s.name);

                                float comboWidth = 0.0f;
                                std::vector<const char*> structOptions;
                                for (const std::string& s : structNames)
                                {
                                    structOptions.push_back(s.c_str());
                                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(s.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
                                }

                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
                                ImGui::Combo("##ViewAsStruct", &viewAsStructIndex, structOptions.data(), (int)structOptions.size());

                                if (viewAsStructIndex > 0)
                                {
                                    viewInfo.structIndex = viewAsStructIndex - 1;
                                    viewInfo.count = viewInfo.size / (int)renderGraph.structs[viewInfo.structIndex].sizeInBytes;
                                }
                            }

                            // Types
                            {
                                static int viewAsTypeIndex = 0;

                                std::vector<std::string> typeNames;
                                typeNames.push_back("");
                                for (int i = 0; i < EnumCount<DataFieldType>() - 1; ++i)
                                    typeNames.push_back(EnumToString((DataFieldType)i));

                                float comboWidth = 0.0f;
                                std::vector<const char*> typeOptions;
                                for (const std::string& s : typeNames)
                                {
                                    typeOptions.push_back(s.c_str());
                                    comboWidth = std::max(comboWidth, ImGui::CalcTextSize(s.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
                                }

                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
                                ImGui::Combo("##ViewAsType", &viewAsTypeIndex, typeOptions.data(), (int)typeOptions.size());

                                if (viewAsTypeIndex > 0)
                                {
                                    DataFieldTypeInfoStructDX12 typeInfo = DataFieldTypeInfoDX12((DataFieldType)(viewAsTypeIndex - 1));
                                    viewInfo.structIndex = -1;
                                    viewInfo.format = typeInfo.typeFormat;
                                    viewInfo.formatCount = typeInfo.typeFormatCount;

                                    DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(viewInfo.format);
                                    viewInfo.count = viewInfo.size / (formatInfo.bytesPerPixel * viewInfo.formatCount);
                                }
                            }

                            ImGui::SameLine();
                            ImGui::Checkbox("Hex", &showAsHex);
                            viewInfo.showAsHex = showAsHex;

                            ImGui::SameLine();
                            ImGui::Checkbox("View", &showBufferView);
                            ShowToolTip("If true, only shows what was in the buffer view. If false, shows the entire resource.");
                        }

                        // See if we can save this as a BVH for WebGPU
                        const unsigned char* BVHFirstPos = nullptr;
                        size_t BVHPosStride = 0;
                        size_t BVHPosCount = 0;
                        if (bytes.size() > 0)
                        {
                            DXGI_FORMAT BVHVertexFormat = DXGI_FORMAT_FORCE_UINT;
                            size_t BVHVertexOffset = 0;

                            // Get the information about the position data
                            if (viewInfo.structIndex != -1)
                            {
                                for (const StructField& field : renderGraph.structs[viewInfo.structIndex].fields)
                                {
                                    if (field.semantic == StructFieldSemantic::Position)
                                    {
                                        BVHVertexFormat = DataFieldTypeInfoDX12(field.type).typeFormat;
                                        break;
                                    }
                                    BVHVertexOffset += field.sizeInBytes;
                                }

                                BVHPosStride = renderGraph.structs[viewInfo.structIndex].sizeInBytes;
                            }
                            else
                            {
                                BVHVertexFormat = viewInfo.format;
                                BVHPosStride = Get_DXGI_FORMAT_Info(BVHVertexFormat).bytesPerPixel;
                            }

                            // Only allow float and float3 format
                            switch (BVHVertexFormat)
                            {
                                case DXGI_FORMAT_R32G32B32_FLOAT:
                                case DXGI_FORMAT_R32_FLOAT:
                                {
                                    BVHFirstPos = &bytes[BVHVertexOffset];
                                    BVHPosCount = bytes.size() / BVHPosStride;
                                    break;
                                }
                                default:
                                {
                                    BVHVertexFormat = DXGI_FORMAT_FORCE_UINT;
                                }
                            }
                        }

                        // Typed buffer
                        if (viewInfo.structIndex == -1)
                        {
                            // Option to save the buffer
                            ImGui::Text("%i items, %i bytes", viewInfo.count, viewInfo.size);
                            ImGui::SameLine();
                            nfdchar_t* outPath = nullptr;
                            if (ImGui::Button("Save as .csv") && NFD_SaveDialog("csv", "", &outPath) == NFD_OKAY)
                                SaveAsCSV(outPath, bytes.data(), viewInfo.format, viewInfo.formatCount, viewInfo.count);
                            ImGui::SameLine();
                            if (ImGui::Button("Save as .bin") && NFD_SaveDialog("bin", "", &outPath) == NFD_OKAY)
                                SaveAsBin(outPath, bytes.data(), (int)bytes.size());
                            ImGui::SameLine();
                            if (ImGui::Button("Save as .hex") && NFD_SaveDialog("hex", "", &outPath) == NFD_OKAY)
                                SaveAsHex(outPath, bytes.data(), (int)bytes.size());

                            // Save as BVH, for WebGPU
                            if (BVHFirstPos != nullptr)
                            {
                                ImGui::SameLine();
                                if (ImGui::Button("Save as .bvh") && NFD_SaveDialog("bvh", "", &outPath) == NFD_OKAY)
                                    SaveAsBVH(outPath, BVHFirstPos, BVHPosStride, BVHPosCount);
                            }

                            // Show it
                            ShowTypedBuffer(bytes.data(), viewInfo.format, viewInfo.formatCount, viewInfo.count, viewInfo.showAsHex);
                        }
                        // Structured buffer
                        else
                        {
                            static bool showStructuredBuffersVertically = false;
                            ImGui::SameLine();
                            ImGui::Checkbox("Show Vertically", &showStructuredBuffersVertically);
                            viewInfo.showStructuredBuffersVertically = showStructuredBuffersVertically;

                            const Struct& structDesc = renderGraph.structs[viewInfo.structIndex];

                            // Option to save the buffer
                            ImGui::Text("%i items, %i bytes", viewInfo.count, viewInfo.size);
                            ImGui::SameLine();
                            nfdchar_t* outPath = nullptr;
                            if (ImGui::Button("Save as .csv") && NFD_SaveDialog("csv", "", &outPath) == NFD_OKAY)
                                SaveAsCSV(outPath, bytes.data(), structDesc, viewInfo.count);
                            ImGui::SameLine();
                            if (ImGui::Button("Save as .bin") && NFD_SaveDialog("bin", "", &outPath) == NFD_OKAY)
                                SaveAsBin(outPath, bytes.data(), (int)bytes.size());
                            ImGui::SameLine();
                            if (ImGui::Button("Save as .hex") && NFD_SaveDialog("hex", "", &outPath) == NFD_OKAY)
                                SaveAsHex(outPath, bytes.data(), (int)bytes.size());

                            // Save as BVH, for WebGPU
                            if (BVHFirstPos != nullptr)
                            {
                                ImGui::SameLine();
                                if (ImGui::Button("Save as .bvh") && NFD_SaveDialog("bvh", "", &outPath) == NFD_OKAY)
                                    SaveAsBVH(outPath, BVHFirstPos, BVHPosStride, BVHPosCount);
                            }

                            // Show it
                            ShowStructuredBuffer(renderGraph, bytes.data(), structDesc, viewInfo.count, viewInfo.showAsHex, viewInfo.showStructuredBuffersVertically);
                        }
                        break;
                    }
                }
            }
            else
                ImGui::Text("Resource not yet created");
        }
    }
    else
    {
        ImGui::Text("No resource selected");
    }

    ImGui::End();
}

void ShowProfilerWindow()
{
    g_interpreter.m_enableProfiling = g_forceEnableProfiling;

    if (!g_showWindows.Profiler || g_hideUI)
        return;

    if (!ImGui::Begin("Profiler", &g_showWindows.Profiler))
    {
        g_interpreter.m_enableProfiling = (false || g_forceEnableProfiling);
        ImGui::End();
        return;
    }

    ImGui::Checkbox("Hide Resource Nodes", &g_hideResourceNodes);

    g_interpreter.m_enableProfiling = true;

    static std::vector<StableSample> stableSamples;
    static bool stableProfiling = true;
    static int accumFrames = 0;
    static bool pauseAccum = false;
    static int accumLimit = 0;

    if (!pauseAccum)
        accumFrames++;

    static std::unordered_map<std::string, float> accumCPU;
    static std::unordered_map<std::string, float> accumGPU;
    static std::unordered_map<std::string, bool> sumCheckBoxes;
    bool wantsSum = false;
    float sums[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (ImGui::BeginTable("profiling", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("CPU ms", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("GPU ms", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("A. CPU", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("A. GPU", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Sum", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        int entryIndex = -1;
        for (const Profiler::ProfilingEntry& entry : g_interpreter.GetProfilingData())
        {
            entryIndex++;

            // skip resource nodes if we should
            if (g_hideResourceNodes && entry.isResourceNode)
                continue;

            if (stableSamples.size() < (entryIndex + 1) * 2)
                stableSamples.resize((entryIndex + 1) * 2);
            StableSample& stableSampleCPU = stableSamples[entryIndex * 2 + 0];
            StableSample& stableSampleGPU = stableSamples[entryIndex * 2 + 1];

            stableSampleCPU.tick(entry.CPUDurationSeconds * 1000.0f, io.DeltaTime);
            stableSampleGPU.tick(entry.GPUDurationSeconds * 1000.0f, io.DeltaTime);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(entry.label.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%0.3f", stableProfiling ? stableSampleCPU.getStableAverage() : stableSampleCPU.getCurrentValue());
            ImGui::TableNextColumn();
            ImGui::Text("%0.3f", stableProfiling ? stableSampleGPU.getStableAverage() : stableSampleGPU.getCurrentValue());

            // Accumulated CPU
            {
                ImGui::TableNextColumn();
                float value = accumCPU[entry.label];
                if (!pauseAccum)
                {
                    value = Lerp(value, entry.CPUDurationSeconds * 1000.0f, 1.0f / float(accumFrames));
                    accumCPU[entry.label] = value;
                }
                ImGui::Text("%0.3f", value);
            }

            // Accumulated GPU
            {
                ImGui::TableNextColumn();
                float value = accumGPU[entry.label];
                if (!pauseAccum)
                {
                    value = Lerp(value, entry.GPUDurationSeconds * 1000.0f, 1.0f / float(accumFrames));
                    accumGPU[entry.label] = value;
                }
                ImGui::Text("%0.3f", value);
            }

            // Sum checkbox
            {
                ImGui::TableNextColumn();

                if (entry.label != "Total")
                {
                    ImGui::PushID(entry.label.c_str());
                    ImGui::PushID("sumcb");
                    bool includeInSum = sumCheckBoxes[entry.label];
                    ImGui::Checkbox("", &includeInSum);
                    sumCheckBoxes[entry.label] = includeInSum;
                    ImGui::PopID();
                    ImGui::PopID();

                    if (includeInSum)
                    {
                        wantsSum = true;
                        sums[0] += stableProfiling ? stableSampleCPU.getStableAverage() : stableSampleCPU.getCurrentValue();
                        sums[1] += stableProfiling ? stableSampleGPU.getStableAverage() : stableSampleGPU.getCurrentValue();
                        sums[2] += accumCPU[entry.label];
                        sums[3] += accumGPU[entry.label];
                    }
                }
            }
        }
        // sum row
        if (wantsSum)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Selection Sum");

            ImGui::TableNextColumn();
            ImGui::Text("%0.3f", sums[0]);

            ImGui::TableNextColumn();
            ImGui::Text("%0.3f", sums[1]);

            ImGui::TableNextColumn();
            ImGui::Text("%0.3f", sums[2]);

            ImGui::TableNextColumn();
            ImGui::Text("%0.3f", sums[3]);

            ImGui::TableNextColumn();
        }
        ImGui::EndTable();

        ImGui::Checkbox("Stabilize", &stableProfiling);

        if (ImGui::Button((pauseAccum ? "Unpause Accum" : "Pause Accum")))
            pauseAccum = !pauseAccum;
        ImGui::SameLine();
        char buttonText[1024];
        sprintf_s(buttonText, "Reset Accum (%i frames)###ResetAccum", accumFrames);
        if (ImGui::Button(buttonText) || g_techniqueFrameIndex < accumFrames)
        {
            pauseAccum = false;
            accumFrames = 0;
        }
        if (ImGui::InputInt("Accum Limit", &accumLimit))
            pauseAccum = false;
        if (accumLimit > 0 && accumFrames >= accumLimit)
            pauseAccum = true;

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::TextWrapped(
            "Warning: Profiling is affected by features in this viewer. "
            "For more accurate results turn on profile mode under the settings menu or the checkbox below, to reduce copies, readbacks, and turn off vsync.\n"
            "You may also wish to turn on stable power state.\n"
            "\nNote: Generated code may have different performance characteristics.\n"
        );
        ImGui::PopStyleColor();

        ImGui::Checkbox("Profile Mode", &g_profileMode);

        if (g_debugLayerOn)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 128, 0, 255));
            ImGui::TextWrapped("\nThe DX12 debug layer is enabled which affects performance. You can disable it by running the viewer with the -nodebuglayer command line option.");
            ImGui::PopStyleColor();
        }
    }

    ImGui::End();
}

void ShowRenderGraphWindow()
{
    if (!g_showWindows.RenderGraph || g_hideUI)
        return;

    if (!ImGui::Begin("Render Graph", &g_showWindows.RenderGraph))
    {
        ImGui::End();
        return;
    }

    ImGui::Checkbox("Hide Resource Nodes", &g_hideResourceNodes);
    ImGui::Checkbox("Only Show Writes", &g_onlyShowWrites);

    // loop through all the nodes in flattened render graph order
    const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
    for (int nodeIndex: renderGraph.flattenedNodeList)
    {
        // skip resource nodes if we should
        if (g_hideResourceNodes)
        {
            switch (renderGraph.nodes[nodeIndex]._index)
            {
                case RenderGraphNode::c_index_resourceBuffer:
                case RenderGraphNode::c_index_resourceTexture:
                case RenderGraphNode::c_index_resourceShaderConstants:
                {
                    continue;
                }
            }
        }

        // Get information from the node and node runtime data
        std::string nodeTypeName;
        std::string nodeName;
        std::vector<RuntimeTypes::ViewableResource>* viewableResources_ = nullptr;
        std::string renderGraphText;
        bool nodeIsInErrorState = false;
        bool nodeConditionIsFalse = false;
        g_interpreter.RuntimeNodeDataLambda(
            renderGraph.nodes[nodeIndex],
            [&] (auto node, auto* runtimeData)
            {
                nodeName = node.name;
                nodeTypeName = node.c_shortTypeName;
                if (runtimeData)
                {
                    viewableResources_ = &(runtimeData->m_viewableResources);
                    renderGraphText = runtimeData->m_renderGraphText;
                    nodeIsInErrorState = runtimeData->m_inErrorState;
                    nodeConditionIsFalse = !runtimeData->m_conditionIsTrue;
                }
            }
        );

        // collapsing header
        {
            if (nodeIsInErrorState)
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
            else if (nodeConditionIsFalse)
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

            char nodeLabel[512];
            sprintf_s(nodeLabel, "%s: %s", nodeTypeName.c_str(), nodeName.c_str());
            bool collapsingHeaderOpen = ImGui::CollapsingHeader(nodeLabel);

            if (nodeIsInErrorState || nodeConditionIsFalse)
                ImGui::PopStyleColor();

            if (!collapsingHeaderOpen)
                continue;
        }

        if (!renderGraphText.empty())
        {
            ImGui::TextUnformatted(renderGraphText.c_str());
            ImGui::Separator();
        }

        // If we have viewable resources, show them
        if (viewableResources_)
        {
            std::vector<RuntimeTypes::ViewableResource>& viewableResources = *viewableResources_;

            bool msaaErrorShown = false;

            int textureIndex = -1;
            for (RuntimeTypes::ViewableResource& viewableResource : viewableResources)
            {
                textureIndex++;

                if (g_onlyShowWrites && !viewableResource.m_isResultOfWrite)
                    continue;

                if (viewableResource.m_hideFromUI || viewableResource.m_displayName.empty())
                    continue;

                if (!g_profileMode && (viewableResource.m_type == RuntimeTypes::ViewableResource::Type::Texture2D || viewableResource.m_type == RuntimeTypes::ViewableResource::Type::Texture2DArray || viewableResource.m_type == RuntimeTypes::ViewableResource::Type::Texture3D || viewableResource.m_type == RuntimeTypes::ViewableResource::Type::TextureCube || viewableResource.m_type == RuntimeTypes::ViewableResource::Type::Texture2DMS))
                    viewableResource.m_wantsToBeViewed = true;

                switch (viewableResource.m_type)
                {
                    case RuntimeTypes::ViewableResource::Type::Texture2D:
                    case RuntimeTypes::ViewableResource::Type::Texture2DArray:
                    case RuntimeTypes::ViewableResource::Type::Texture3D:
                    case RuntimeTypes::ViewableResource::Type::TextureCube:
                    {
                        if (viewableResource.m_resource)
                        {
                            ShowImageThumbnail(viewableResource.m_resource, viewableResource.m_format, viewableResource.m_size, nodeIndex, textureIndex, viewableResource.m_displayName.c_str(), RuntimeTypes::ViewableResource::Type::Texture2D);
                            ImGui::SameLine();
                            ImGui::Text("%s", viewableResource.m_displayName.c_str());
                        }
                        else
                        {
                            if (ImGui::Button(viewableResource.m_displayName.c_str()))
                                g_resourceView.ConstantBuffer(nodeIndex, textureIndex);
                        }
                        break;
                    }
                    case RuntimeTypes::ViewableResource::Type::ConstantBuffer:
                    {
                        if (ImGui::Button(viewableResource.m_displayName.c_str()))
                            g_resourceView.ConstantBuffer(nodeIndex, textureIndex);
                        break;
                    }
                    case RuntimeTypes::ViewableResource::Type::Buffer:
                    {
                        if (ImGui::Button(viewableResource.m_displayName.c_str()))
                            g_resourceView.Buffer(nodeIndex, textureIndex);
                        break;
                    }
                    case RuntimeTypes::ViewableResource::Type::Texture2DMS:
                    {
                        if(!msaaErrorShown)
                            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "MSAA preview is not implemented yet.");
                        msaaErrorShown = true;
                        break;
                    }
                }
            }
        }
    }

    ImGui::End();
}

class Python : public PythonInterface
{
public:
    bool LoadGG(const char* fileName) override final
    {
        ClearWantsReadback();
        return LoadGGFile(fileName, false, false);
    }

    void RequestExit(int exitCode) override final
    {
        PostQuitMessage(exitCode);
    }

    void SetHideUI(bool set) override final
    {
        g_hideUI = set;
    }

    void SetVSync(bool set) override final
    {
        g_syncInterval = set ? 1 : 0;
    }

    void SetSyncInterval(int syncInterval) override final
    {
        g_syncInterval = syncInterval;
    }

    void SetStablePowerState(bool set) override final
    {
        g_stablePowerState = set;
    }

    void SetProfilingMode(bool set) override final
    {
        g_profileMode = set;
    }

    bool SetVariable(const char* varName, const char* varValue) override final
    {
        for (const Variable& var : g_interpreter.GetRenderGraph().variables)
        {
            if (!_stricmp(varName, var.name.c_str()))
            {
                int varIndex = g_interpreter.GetRuntimeVariableIndex(varName);
                if (varIndex == -1)
                    return false;

                g_interpreter.SetRuntimeVariableFromString(varIndex, varValue);
                return true;
            }
        }
        return false;
    }

    bool GetVariable(const char* varName, std::string& value) override final
    {
        for (const Variable& var : g_interpreter.GetRenderGraph().variables)
        {
            if (!_stricmp(varName, var.name.c_str()))
            {
                int varIndex = g_interpreter.GetRuntimeVariableIndex(varName);
                if (varIndex == -1)
                    return false;

                value = g_interpreter.GetRuntimeVariableValueAsString(varIndex);
                return true;
            }
        }
        return false;
    }

    void SetDisableGGUserSave(bool set) override final
    {
        g_disableGGUserSave = set;
    }

    void Log(LogLevel level, const char* msg, ...) override final
    {
        char formattedMsg[40960];

        va_list args;
        va_start(args, msg);
        vsprintf_s(formattedMsg, msg, args);
        va_end(args);

        AddMultiLineLogEntry(level, formattedMsg);

        // tell the server about this log message
        if (g_previewClient.IsFullyConnected())
        {
            PreviewMsgClientToServer msg;
            msg._index = PreviewMsgClientToServer::c_index_log;
            msg.log.level = (PreviewMsgCS_Log_Level)level;
            msg.log.msg = formattedMsg;
            g_previewClient.Send(msg);
        }
    }

    void SetWantReadback(const char* viewableResourceName, bool wantsReadback) override final
    {
        if (wantsReadback)
            m_wantsReadback.insert(viewableResourceName);
        else
            m_wantsReadback.erase(viewableResourceName);
    }

    bool Readback(const char* viewableResourceName, int arrayIndex, int mipIndex, GigiArray& data) override final
    {
        // If they didn't say that it wants to be read back, we won't have read it back!
        if (m_wantsReadback.count(viewableResourceName) == 0)
        {
            Log(LogLevel::Error, "Python: Host.Readback() tried to read back resource \"%s\" without calling SetWantReadback() first.", viewableResourceName);
            return false;
        }

        // Get the viewable resource
        RuntimeTypes::ViewableResource* vr = nullptr;
        for (const RenderGraphNode& node : g_interpreter.GetRenderGraph().nodes)
        {
            g_interpreter.RuntimeNodeDataLambda(
                node,
                [&](auto node, auto* runtimeData)
                {
                    if (vr != nullptr || !runtimeData)
                        return;
                    for (RuntimeTypes::ViewableResource& viewableResource : runtimeData->m_viewableResources)
                    {
                        if (viewableResource.m_displayName == viewableResourceName)
                        {
                            vr = &viewableResource;
                            break;
                        }
                    }
                }
            );
        }

        // Not having the resource may be a temporary thing, due to "frames in flight" causing latency
        if (vr == nullptr || vr->m_resourceReadback == nullptr)
        {
            Log(LogLevel::Warn, "Python: Host.Readback() could not read back resource \"%s\".  It is either invalid or doesn't yet exist. You may need to run the technique before the resource exists.", viewableResourceName);
            return false;
        }

        bool isTexture = true;
        switch (vr->m_type)
        {
            case RuntimeTypes::ViewableResource::Type::Buffer:
            case RuntimeTypes::ViewableResource::Type::ConstantBuffer:
            {
                isTexture = false;
                break;
            }
        }

        if (isTexture)
        {
            // Get the decoded image data
            DXGI_FORMAT decodedFormat;
            std::vector<unsigned char> decodedPixels;
            int decodedWidth = 0;
            int decodedHeight = 0;
            int decodedDepth = 1;
            if (!ImageReadback::GetDecodedImageSliceAllDepths(g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, arrayIndex, mipIndex, decodedPixels, decodedFormat, decodedWidth, decodedHeight, decodedDepth))
            {
                Log(LogLevel::Warn, "Python: Host.Readback() could not decode resource slice \"%s\" %i %i.", viewableResourceName, arrayIndex, mipIndex);
                return false;
            }

            // get the type info
            DXGI_FORMAT_Info decodedFormatInfo = Get_DXGI_FORMAT_Info(decodedFormat);
            switch (decodedFormatInfo.channelType)
            {
                case DXGI_FORMAT_Info::ChannelType::_uint8_t: data.formatString = "B"; break;
                case DXGI_FORMAT_Info::ChannelType::_uint16_t: data.formatString = "H"; break;
                case DXGI_FORMAT_Info::ChannelType::_uint32_t: data.formatString = "I"; break;
                case DXGI_FORMAT_Info::ChannelType::_int8_t: data.formatString = "b"; break;
                case DXGI_FORMAT_Info::ChannelType::_int16_t: data.formatString = "h"; break;
                case DXGI_FORMAT_Info::ChannelType::_int32_t: data.formatString = "i"; break;
                case DXGI_FORMAT_Info::ChannelType::_float: data.formatString = "f"; break;
                case DXGI_FORMAT_Info::ChannelType::_half: data.formatString = "e"; break;
            }
            data.itemSize = decodedFormatInfo.bytesPerChannel;

            // copy the data
            data.data.resize(decodedPixels.size());
            memcpy(data.data.data(), decodedPixels.data(), data.data.size());

            // dimensions
            data.dims.push_back(decodedFormatInfo.channelCount);
            data.dims.push_back(decodedWidth);
            data.dims.push_back(decodedHeight);
            data.dims.push_back(decodedDepth);
            std::reverse(data.dims.begin(), data.dims.end());

            // strides
            data.strides.push_back(decodedFormatInfo.bytesPerChannel);                              // bytes to get from one channel to the next
            data.strides.push_back(decodedFormatInfo.bytesPerPixel);                                // bytes to increment x position
            data.strides.push_back(decodedWidth * decodedFormatInfo.bytesPerPixel);                 // bytes to increment y position
            data.strides.push_back(decodedHeight * decodedWidth * decodedFormatInfo.bytesPerPixel); // bytes to increment z position
            std::reverse(data.strides.begin(), data.strides.end());

            return true;
        }
        else
        {
            // Typed buffers
            if (vr->m_structIndex == -1)
            {
                // get the type info
                DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(vr->m_format);
                switch (formatInfo.channelType)
                {
                    case DXGI_FORMAT_Info::ChannelType::_uint8_t: data.formatString = "B"; break;
                    case DXGI_FORMAT_Info::ChannelType::_uint16_t: data.formatString = "H"; break;
                    case DXGI_FORMAT_Info::ChannelType::_uint32_t: data.formatString = "I"; break;
                    case DXGI_FORMAT_Info::ChannelType::_int8_t: data.formatString = "b"; break;
                    case DXGI_FORMAT_Info::ChannelType::_int16_t: data.formatString = "h"; break;
                    case DXGI_FORMAT_Info::ChannelType::_int32_t: data.formatString = "i"; break;
                    case DXGI_FORMAT_Info::ChannelType::_float: data.formatString = "f"; break;
                    case DXGI_FORMAT_Info::ChannelType::_half: data.formatString = "e"; break;
                }
                data.itemSize = formatInfo.bytesPerChannel;

                // Get the data
                {
                    // allocate
                    data.data.resize(vr->m_size[0]);

                    // map
                    char* values = nullptr;
                    vr->m_resourceReadback->Map(0, nullptr, (void**)&values);

                    // copy
                    memcpy(data.data.data(), values, data.data.size());

                    // unmap
                    D3D12_RANGE writeRange;
                    writeRange.Begin = 1;
                    writeRange.End = 0;
                    vr->m_resourceReadback->Unmap(0, &writeRange);
                }

                // dimensions
                data.dims.push_back(formatInfo.channelCount);
                data.dims.push_back(vr->m_count);
                std::reverse(data.dims.begin(), data.dims.end());

                // strides
                data.strides.push_back(formatInfo.bytesPerChannel);                 // bytes to get from one channel to the next
                data.strides.push_back(data.strides[0] * formatInfo.channelCount);  // bytes to increment x position
                std::reverse(data.strides.begin(), data.strides.end());

                return true;
            }
            else
            {
                // get the type info
                const Struct& desc = g_interpreter.GetRenderGraph().structs[vr->m_structIndex];
                for (const StructField& field : desc.fields)
                {
                    DataFieldTypeInfoStructDX12 typeInfo = DataFieldTypeInfoDX12(field.type);
                    DXGI_FORMAT_Info formatInfo = Get_DXGI_FORMAT_Info(typeInfo.componentFormat);

                    for (int i = 0; i < typeInfo.componentCount; ++i)
                    {
                        switch (formatInfo.channelType)
                        {
                            case DXGI_FORMAT_Info::ChannelType::_uint8_t: data.formatString += "B"; break;
                            case DXGI_FORMAT_Info::ChannelType::_uint16_t: data.formatString += "H"; break;
                            case DXGI_FORMAT_Info::ChannelType::_uint32_t: data.formatString += "I"; break;
                            case DXGI_FORMAT_Info::ChannelType::_int8_t: data.formatString += "b"; break;
                            case DXGI_FORMAT_Info::ChannelType::_int16_t: data.formatString += "h"; break;
                            case DXGI_FORMAT_Info::ChannelType::_int32_t: data.formatString += "i"; break;
                            case DXGI_FORMAT_Info::ChannelType::_float: data.formatString += "f"; break;
                            case DXGI_FORMAT_Info::ChannelType::_half: data.formatString += "e"; break;
                        }
                    }
                }
                data.itemSize = desc.sizeInBytes;

                // Get the data
                {
                    // allocate
                    if (vr->m_type == RuntimeTypes::ViewableResource::Type::ConstantBuffer)
                        data.data.resize(data.itemSize);
                    else
                        data.data.resize(vr->m_stride * vr->m_count);

                    // map
                    char* values = nullptr;
                    vr->m_resourceReadback->Map(0, nullptr, (void**)&values);

                    // copy
                    memcpy(data.data.data(), values, data.data.size());

                    // unmap
                    D3D12_RANGE writeRange;
                    writeRange.Begin = 1;
                    writeRange.End = 0;
                    vr->m_resourceReadback->Unmap(0, &writeRange);
                }

                // dimensions
                data.dims.push_back(data.data.size() / data.itemSize);

                // strides
                data.strides.push_back(data.itemSize);                 // bytes to get from one entry to the next

                return true;
            }
        }
    }

    RuntimeTypes::ViewableResource* PrepareSaveTexture(const char* viewableResourceName, const char* functionName)
    {
        // If they didn't say that it wants to be read back, we won't have read it back!
        if (m_wantsReadback.count(viewableResourceName) == 0)
        {
            Log(LogLevel::Error, "Python: %s tried to read back resource \"%s\" without calling SetWantReadback() first.", functionName, viewableResourceName);
            return nullptr;
        }

        // Get the viewable resource
        RuntimeTypes::ViewableResource* vr = nullptr;
        for (const RenderGraphNode& node : g_interpreter.GetRenderGraph().nodes)
        {
            g_interpreter.RuntimeNodeDataLambda(
                node,
                [&](auto node, auto* runtimeData)
                {
                    if (vr != nullptr || !runtimeData)
                        return;
                    for (RuntimeTypes::ViewableResource& viewableResource : runtimeData->m_viewableResources)
                    {
                        if (viewableResource.m_displayName == viewableResourceName)
                        {
                            vr = &viewableResource;
                            break;
                        }
                    }
                }
            );
        }

        // Not having the resource may be a temporary thing, due to "frames in flight" causing latency
        if (vr == nullptr || vr->m_resourceReadback == nullptr)
        {
            Log(LogLevel::Warn, "Python: %s could not read back resource \"%s\".  It is either invalid or doesn't yet exist. You may need to run the technique before the resource exists.", functionName, viewableResourceName);
            return nullptr;
        }

        bool isTexture = true;
        switch (vr->m_type)
        {
            case RuntimeTypes::ViewableResource::Type::Buffer:
            case RuntimeTypes::ViewableResource::Type::ConstantBuffer:
            {
                isTexture = false;
                break;
            }
        }

        if (!isTexture)
        {
            Log(LogLevel::Warn, "Python: Host." __FUNCTION__ " resource \"%s\" is not a texture.", viewableResourceName);
            return nullptr;
        }

        return vr;
    }

    bool SaveAsPNG(const char* fileName, const char* viewableResourceName, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        return ImageSave::SaveAsPng(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsDDS_BC4(const char* fileName, const char* viewableResourceName, bool signedData, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        options.bc45.isSigned = signedData;
        return ImageSave::SaveAsDDS_BC4(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsDDS_BC5(const char* fileName, const char* viewableResourceName, bool signedData, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        options.bc45.isSigned = signedData;
        return ImageSave::SaveAsDDS_BC5(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsDDS_BC6(const char* fileName, const char* viewableResourceName, bool signedData, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        options.bc6.isSigned = signedData;
        return ImageSave::SaveAsDDS_BC6(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsDDS_BC7(const char* fileName, const char* viewableResourceName, bool sRGB, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        options.bc7.sRGB = sRGB;
        return ImageSave::SaveAsDDS_BC7(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsEXR(const char* fileName, const char* viewableResourceName, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        return ImageSave::SaveAsEXR(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsHDR(const char* fileName, const char* viewableResourceName, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        return ImageSave::SaveAsHDR(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsCSV(const char* fileName, const char* viewableResourceName, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        return ImageSave::SaveAsCSV(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    bool SaveAsBinary(const char* fileName, const char* viewableResourceName, int arrayIndex, int mipIndex) override final
    {
        RuntimeTypes::ViewableResource* vr = PrepareSaveTexture(viewableResourceName, __FUNCTION__);
        if (!vr)
            return false;

        ImageSave::Options options;
        options.saveAll = (arrayIndex < 0);
        options.zIndex = arrayIndex;
        options.mipIndex = mipIndex;
        return ImageSave::SaveAsBinary(fileName, g_pd3dDevice, vr->m_resourceReadback, vr->m_origResourceDesc, options);
    }

    void RunTechnique(int runCount) override final
    {
        for (int i = 0; i < runCount; ++i)
            RenderFrame(true);
    }

    void SetFrameIndex(int frameIndex) override final
    {
        g_techniqueFrameIndex = frameIndex;
    }

    void WaitOnGPU() override final
    {
        WaitForLastSubmittedFrame();
    }

    void Pause(bool pause) override final
    {
        g_executeTechnique = !pause;
    }

    void PixCaptureNextFrames(const char* fileName, int frameCount) override final
    {
        std::wstring fileNameW = ToWideString(fileName);
        PIXGpuCaptureNextFrames(fileNameW.c_str(), frameCount);

        char currentDirectory[4096];
        GetCurrentDirectoryA(4096, currentDirectory);
        Log(LogLevel::Info, "Python triggered pix capture saved to %s (%i frames)", (std::filesystem::path(currentDirectory) / fileName).string().c_str(), frameCount);
    }

    void SetImportedBufferCSVHeaderRow(const char* bufferName, bool CSVHeaderRow) override final
    {
        if (g_interpreter.m_importedResources.count(bufferName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferCSVHeaderRow could not find imported buffer %s", bufferName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[bufferName];
        if (desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferCSVHeaderRow called for %s which is not a buffer", bufferName);
            return;
        }

        desc.buffer.CSVHeaderRow = CSVHeaderRow;
    }

    void SetImportedBufferCount(const char* bufferName, int count) override final
    {
        if (g_interpreter.m_importedResources.count(bufferName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferCount could not find imported buffer %s", bufferName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[bufferName];
        if (desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferCount called for %s which is not a buffer", bufferName);
            return;
        }

        desc.buffer.count = count;
    }

    void SetImportedBufferFile(const char* bufferName, const char* fileName) override final
    {
        if (g_interpreter.m_importedResources.count(bufferName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferFile could not find imported buffer %s", bufferName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[bufferName];
        if (desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferFile called for %s which is not a buffer", bufferName);
            return;
        }

        std::filesystem::path renderGraphDir = std::filesystem::path(g_renderGraphFileName).remove_filename();

        if (std::filesystem::path(fileName).is_relative())
            desc.buffer.fileName = (renderGraphDir / fileName).string();
        else
            desc.buffer.fileName = fileName;

        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedBufferStruct(const char* bufferName, const char* structName) override final
    {
        if (g_interpreter.m_importedResources.count(bufferName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferStruct could not find imported buffer %s", bufferName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[bufferName];
        if (desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferStruct called for %s which is not a buffer", bufferName);
            return;
        }

        const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
        int structIndex = -1;
        for (const Struct& s : renderGraph.structs)
        {
            structIndex++;
            if (!_stricmp(s.name.c_str(), structName))
            {
                desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                desc.buffer.structIndex = structIndex;
                return;
            }
        }

        Log(LogLevel::Error, "Python: SetImportedBufferStruct could not find a struct named %s", structName);
    }

    void SetImportedBufferType(const char* bufferName, DataFieldType type) override final
    {
        if (g_interpreter.m_importedResources.count(bufferName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferStruct could not find imported buffer %s", bufferName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[bufferName];
        if (desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedBufferStruct called for %s which is not a buffer", bufferName);
            return;
        }

        desc.buffer.type = type;
        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureFile(const char* textureName, const char* fileName) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureFile could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureFile called for %s which is not a texture", textureName);
            return;
        }

        if (fileName != nullptr && fileName[0] != 0)
        {
            std::filesystem::path renderGraphDir = std::filesystem::path(g_renderGraphFileName).remove_filename();

            if (std::filesystem::path(fileName).is_relative())
                desc.texture.fileName = (renderGraphDir / fileName).string();
            else
                desc.texture.fileName = fileName;
        }
        else
            desc.texture.fileName = "";

        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureSourceIsSRGB(const char* textureName, bool sourceIsSRGB) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureSourceIsSRGB could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureSourceIsSRGB called for %s which is not a texture", textureName);
            return;
        }

        desc.texture.fileIsSRGB = sourceIsSRGB;
        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureMakeMips(const char* textureName, bool makeMips) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureMakeMips could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureMakeMips called for %s which is not a texture", textureName);
            return;
        }

        desc.texture.makeMips = makeMips;
        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureFormat(const char* textureName, int textureFormat) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureFormat could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureFormat called for %s which is not a texture", textureName);
            return;
        }

        desc.texture.format = (TextureFormat)textureFormat;
        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureColor(const char* textureName, float R, float G, float B, float A) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureColor could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureColor called for %s which is not a texture", textureName);
            return;
        }

        desc.texture.color[0] = R;
        desc.texture.color[1] = G;
        desc.texture.color[2] = B;
        desc.texture.color[3] = A;

        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureSize(const char* textureName, int x, int y, int z) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureSize could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureSize called for %s which is not a texture", textureName);
            return;
        }

        desc.texture.size[0] = x;
        desc.texture.size[1] = y;
        desc.texture.size[2] = z;
        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureBinarySize(const char* textureName, int x, int y, int z) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureBinarySize could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureBinarySize called for %s which is not a texture", textureName);
            return;
        }

        desc.texture.binaryDesc.size[0] = x;
        desc.texture.binaryDesc.size[1] = y;
        desc.texture.binaryDesc.size[2] = z;
        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetImportedTextureBinaryFormat(const char* textureName, int textureFormat) override final
    {
        if (g_interpreter.m_importedResources.count(textureName) == 0)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureBinaryFormat could not find imported texture %s", textureName);
            return;
        }

        GigiInterpreterPreviewWindowDX12::ImportedResourceDesc& desc = g_interpreter.m_importedResources[textureName];
        if (!desc.isATexture)
        {
            Log(LogLevel::Error, "Python: SetImportedTextureBinaryFormat called for %s which is not a texture", textureName);
            return;
        }

        desc.texture.binaryDesc.format = (TextureFormat)textureFormat;
        desc.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
    }

    void SetFrameDeltaTime(float seconds) override final
    {
        g_forcedFrameDeltaTime = seconds;
    }

    void SetCameraPos(float X, float Y, float Z) override final
    {
        g_systemVariables.camera.cameraPos[0] = X;
        g_systemVariables.camera.cameraPos[1] = Y;
        g_systemVariables.camera.cameraPos[2] = Z;
    }

    void SetCameraFOV(float fov) override final
    {
        g_systemVariables.camera.FOV = fov;
    }

    void SetCameraAltitudeAzimuth(float altitude, float azimuth) override final
    {
        g_systemVariables.camera.cameraAltitudeAzimuth[0] = altitude;
        g_systemVariables.camera.cameraAltitudeAzimuth[1] = azimuth;
    }

    void SetCameraNearFarZ(float nearZ, float farZ) override final
    {
        g_systemVariables.camera.nearPlane = nearZ;
        g_systemVariables.camera.farPlane = farZ;
    }

    void SetCameraFlySpeed(float speed) override final
    {
        g_systemVariables.camera.flySpeed = speed;
    }

    void GetCameraPos(float& X, float& Y, float& Z) override final
    {
        X = g_systemVariables.camera.cameraPos[0];
        Y = g_systemVariables.camera.cameraPos[1];
        Z = g_systemVariables.camera.cameraPos[2];
    }

    void GetCameraAltitudeAzimuth(float& altitude, float& azimuth) override final
    {
        altitude = g_systemVariables.camera.cameraAltitudeAzimuth[0];
        azimuth = g_systemVariables.camera.cameraAltitudeAzimuth[1];
    }

    void WriteGPUResource(const char* viewableResourceName, int subresourceIndex, const char* data, size_t size) override final
    {
        g_interpreter.WriteGPUResource(viewableResourceName, subresourceIndex, data, size);
    }

    void ForceEnableProfiling(bool forceEnable) override final
    {
        g_forceEnableProfiling = forceEnable;
    }

    std::vector<ProfilingData> GetProfilingData() override final
    {
        std::vector<ProfilingData> ret;

        if (!g_forceEnableProfiling)
        {
            Log(LogLevel::Error, "GetProfilingData() was called, but ForceEnableProfiling() was not called first");
            return ret;
        }

        for (const Profiler::ProfilingEntry& entry : g_interpreter.GetProfilingData())
        {
            ProfilingData newData;
            newData.label = entry.label;
            newData.cpums = entry.CPUDurationSeconds * 1000.0f;
            newData.gpums = entry.GPUDurationSeconds * 1000.0f;
            ret.push_back(newData);
        }

        return ret;
    }

    bool IsResourceCreated(const char* resourceName) override final
    {
        if (g_interpreter.m_importedResources.count(resourceName) == 0)
        {
            Log(LogLevel::Error, "Python: IsResourceCreated could not find imported resource %s", resourceName);
            return false;
        }

        bool existsAsBuffer = false;
        RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeDataBuffer = g_interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceName, existsAsBuffer);

        bool existsAsTexture = false;
        RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeDataTexture = g_interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceName, existsAsTexture);

        return runtimeDataBuffer.m_resource || runtimeDataTexture.m_resource;
    }

    void SetViewedResource(const char* resourceName) override final
    {
        // RuntimeTypes::RenderGraphNode_Base
        g_interpreter.RuntimeDataLambda(
            [resourceName](auto& key, const RuntimeTypes::RenderGraphNode_Base& runtimeData)
            {
                int viewableResourceIndex = -1;
                for (const RuntimeTypes::ViewableResource& viewableResource : runtimeData.m_viewableResources)
                {
                    viewableResourceIndex++;
                    if (!_stricmp(viewableResource.m_displayName.c_str(), resourceName))
                    {
                        int nodeIndex = FrontEndNodesNoCaching::GetNodeIndexByName(g_interpreter.GetRenderGraph(), std::get<0>(key).c_str());
                        if (nodeIndex == -1)
                            continue;

                        const RenderGraphNode& nodeBase = g_interpreter.GetRenderGraph().nodes[nodeIndex];
                        switch (nodeBase._index)
                        {
                            case RenderGraphNode::c_index_resourceBuffer:
                            {
                                const RenderGraphNode_Resource_Buffer& node = nodeBase.resourceBuffer;
                                g_resourceView.Buffer(nodeIndex, viewableResourceIndex);
                                break;
                            }
                            case RenderGraphNode::c_index_resourceTexture:
                            {
                                const RenderGraphNode_Resource_Texture& node = nodeBase.resourceTexture;
                                RuntimeTypes::ViewableResource::Type type = RuntimeTypes::ViewableResource::Type::Texture2D;
                                switch (node.dimension)
                                {
                                    case TextureDimensionType::Texture2D: type = RuntimeTypes::ViewableResource::Type::Texture2D; break;
                                    case TextureDimensionType::Texture2DArray: type = RuntimeTypes::ViewableResource::Type::Texture2DArray; break;
                                    case TextureDimensionType::Texture3D: type = RuntimeTypes::ViewableResource::Type::Texture3D; break;
                                    case TextureDimensionType::TextureCube: type = RuntimeTypes::ViewableResource::Type::TextureCube; break;
                                    case TextureDimensionType::Texture2DMS: type = RuntimeTypes::ViewableResource::Type::Texture2DMS; break;
                                }
                                g_resourceView.Texture(nodeIndex, viewableResourceIndex, type);
                                break;
                            }
                        }
                    }
                }
            }
        );
    }

    void SetShaderAssertsLogging(bool set) override final
    {
        g_logCollectedShaderAsserts = set;
    }

    int GetCollectedShaderAssertsCount() override final
    {
        return (int)g_interpreter.getCollectedShaderAsserts().size();
    }

    int GetShaderAssertFormatStrId(int i) override final
    {
        const auto& asserts = g_interpreter.getCollectedShaderAsserts();
        return (int)asserts[i].formatStringId;
    }

    std::string GetShaderAssertFormatString(int i) override final
    {
        const auto& asserts = g_interpreter.getCollectedShaderAsserts();
        return asserts[i].fmt;
    }

    std::string GetShaderAssertDisplayName(int i) override final
    {
        const auto& asserts = g_interpreter.getCollectedShaderAsserts();
        return asserts[i].displayName;
    }

    std::string GetShaderAssertMsg(int i) override final
    {
        const auto& asserts = g_interpreter.getCollectedShaderAsserts();
        return asserts[i].msg;
    }

    int GGEnumValue(const char* enumName, const char* enumLabel) override final
    {
        const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
        for (const Enum& e : renderGraph.enums)
        {
            for (int index = 0; index < (int)e.items.size(); ++index)
            {
                if (!_stricmp(e.items[index].label.c_str(), enumLabel))
                    return index;
            }
            return -2;
        }
        return -1;
    }

    std::string GGEnumLabel(const char* enumName, int value) override final
    {
        const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
        for (const Enum& e : renderGraph.enums)
        {
            if (value >= 0 && value < e.items.size())
                return e.items[value].label;

            return "";
        }
        return "";
    }

    int GGEnumCount(const char* enumName) override final
    {
        const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
        for (const Enum& e : renderGraph.enums)
        {
            if (!_stricmp(e.name.c_str(), enumName))
            {
                return (int)e.items.size();
            }
        }
        return -1;
    }

    std::string GetGPUString() override final
    {
        char buffer[2048];
        sprintf_s(buffer, "%s (driver %s)", g_adapterName.c_str(), g_driverVersion.c_str());
        return buffer;
    }

    std::string GetAppCommandLine() override final
    {
        return g_commandLine;
    }

    std::string GetScriptLocation() override final
    {
        return m_scriptLocation;
    }

    // Helper functions and storage
    void Tick(void)
    {
        // Set the readback flags on resources
        const RenderGraph& renderGraph = g_interpreter.GetRenderGraph();
        for (const RenderGraphNode& node : renderGraph.nodes)
        {
            g_interpreter.RuntimeNodeDataLambda(
                node,
                [this] (auto node, auto* runtimeData)
                {
                    if (!runtimeData)
                        return;
                    for (RuntimeTypes::ViewableResource& viewableResource : runtimeData->m_viewableResources)
                    {
                        // If the g_readbackAll flag is set, mark everything as wanting readback
                        if (g_readbackAll)
                        {
                            viewableResource.m_wantsToBeViewed = true;
                            viewableResource.m_wantsToBeReadBack = true;
                            viewableResource.m_arrayIndex = 0;
                            viewableResource.m_mipIndex = 0;
                            continue;
                        }

                        // Else, only mark resources that have been explicitly asked for
                        if (m_wantsReadback.count(viewableResource.m_displayName) == 0)
                            continue;
                        viewableResource.m_wantsToBeViewed = true;
                        viewableResource.m_wantsToBeReadBack = true;
                        viewableResource.m_arrayIndex = 0;
                        viewableResource.m_mipIndex = 0;
                    }
                }
            );
        }
    }

    void OnExecuteFinished() override final
    {
        ClearWantsReadback();
    }

    void ClearWantsReadback()
    {
        m_wantsReadback.clear();
    }

    std::unordered_set<std::string> m_wantsReadback;
};

Python g_python;

void RenderFrame(bool forceExecute)
{
	TryBeginRenderDocCapture();

    SynchronizeSystemVariables();

    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(nullptr);
    MakeInitialLayout(dockspace_id);

    HandleMainMenu();

    ShowInternalVariables();
    ShowSystemVariables();
    ShowProfilerWindow();
    ShowRenderGraphWindow();
    ShowResourceView();

    g_interpreter.ShowUI(g_hideUI, g_techniquePaused);

    ShowLog();

    ShowShaders();

    ShowImportedResources();

    ShowImGuiWindows();

    // Rendering
    ImGui::Render();

    FrameContext* frameCtx = WaitForNextFrameResources();
    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    frameCtx->CommandAllocator->Reset();

    g_pd3dCommandList->Reset(frameCtx->CommandAllocator, NULL);

    std::vector<RuntimeTypes::ViewableResource*> assertsBuffers = g_interpreter.MarkShaderAssertsForReadback();

    // Run the Gigi technique if we should
    g_python.Tick();
    g_interpreter.Tick();
    if (g_executeTechnique || forceExecute)
    {
        g_interpreter.UploadDataToBuffer(g_pd3dCommandList, g_systemVariables.KeyState_bufferName.c_str(), g_keyStates);
        g_interpreter.Execute(g_pd3dCommandList);
        g_techniqueFrameIndex++;
    }

    g_interpreter.CollectShaderAsserts(assertsBuffers);
    if (g_logCollectedShaderAsserts)
        g_interpreter.LogCollectedShaderAsserts();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Render Dear ImGui graphics
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, NULL);
    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(NULL, (void*)g_pd3dCommandList);
    }

    g_pSwapChain->Present((!g_profileMode) ? g_syncInterval : 0, 0);

    UINT64 fenceValue = g_fenceLastSignaledValue + 1;
    g_pd3dCommandQueue->Signal(g_fence, fenceValue);
    g_fenceLastSignaledValue = fenceValue;
    frameCtx->FenceValue = fenceValue;

    TryEndRenderDocCapture();
}

// Main code
int main(int argc, char** argv)
{
    // Load recent files
    g_recentFiles.LoadAllEntries();
    g_recentPythonScripts.LoadAllEntries();

    // Load the viewer settings
    ReadFromJSONFile(g_viewerConfig, (GetLocalAppDataPath() / "ViewerConfig.json").string().c_str(), false);

    SetGigiHeadlessMode(!BREAK_ON_GIGI_ASSERTS());

    // build g_commandLine
    {
        // start from 1, to exclude the executable name
        for (int i = 1; i < argc; ++i)
        {
            std::string el = argv[i];
            if (!g_commandLine.empty())
                g_commandLine += " ";

            if (el.find(' ') != std::string::npos)
                g_commandLine += "\"" + el + "\"";
            else
                g_commandLine += el;
        }
    }

    // Parse command line parameters
    int argIndex = 0;
    while (argIndex < argc)
    {
        if (!_stricmp(argv[argIndex], "-editor"))
        {
            if (argc <= argIndex + 2)
            {
                printf("Not enough arguments given. Expected: -editor <IP> <Port>\n");
                return 1;
            }

            g_isForEditor = true;
            g_editorIP = argv[argIndex + 1];
            g_editorPort = argv[argIndex + 2];
            argIndex += 3;
        }
        else if (!_stricmp(argv[argIndex], "-paused"))
        {
            int value = 0;
            if (argc <= argIndex + 1 || sscanf_s(argv[argIndex + 1], "%i", &value) != 1)
            {
                printf("Not enough arguments given. Expected: -paused <0|1>\n");
                return 1;
            }
            g_executeTechnique = (value == 1) ? false : true;
            argIndex += 2;
        }
        else if (!_stricmp(argv[argIndex], "-load"))
        {
            if (argc <= argIndex + 1)
            {
                printf("Not enough arguments given. Expected: -load <ggFileName>\n");
                return 1;
            }
            g_commandLineLoadGGFileName = argv[argIndex + 1];
            argIndex += 2;
        }
        else if (!_stricmp(argv[argIndex], "-device"))
        {
            if (argc <= argIndex + 1)
            {
                printf("Not enough arguments given. Expected: -device <index>\n");
                return 1;
            }
            g_GPUDeviceIndex = atoi(argv[argIndex + 1]);
            argIndex += 2;
        }
        else if (!_stricmp(argv[argIndex], "-run"))
        {
            if (argc <= argIndex + 1)
            {
                printf("Not enough arguments given. Expected: -run <pyFileName>\n");
                return 1;
            }
            g_runPyFileName = argv[argIndex + 1];
            argIndex += 2;

            // NOTE: all arguments after -run are sent to the python script!
            g_runPyArgs.clear();
            for (int argumentIndex = argIndex; argumentIndex < argc; ++argumentIndex)
                g_runPyArgs.push_back(ToWideString(argv[argumentIndex]));

            break;
        }
        else if (!_stricmp(argv[argIndex], "-logdebuglayer"))
        {
            g_debugLayerShown = true;
            argIndex++;
        }
        else if (!_stricmp(argv[argIndex], "-nodebuglayer"))
        {
            g_debugLayerOn = false;
            argIndex++;
        }
        else if (!_stricmp(argv[argIndex], "-gpuvalidation"))
        {
            g_GPUValidation = true;
            argIndex++;
        }
        else if (!_stricmp(argv[argIndex], "-norenderdoc"))
        {
            g_renderDocEnabled = false;
            argIndex++;
        }
        else if (!_stricmp(argv[argIndex], "-nopixcapture"))
        {
            g_pixCaptureEnabled = false;
            argIndex++;
        }
        else if (!_stricmp(argv[argIndex], "-compileshadersfordebug"))
        {
            g_interpreter.m_compileShadersForDebug = true;
            argIndex++;
        }
        else if (!_stricmp(argv[argIndex], "-warpadapter"))
        {
            g_useWarpAdapter = true;
            argIndex++;
        }
        else if (!_stricmp(argv[argIndex], "-AgilitySDKPreview"))
        {
            g_agilitySDKChoice = AgilitySDKChoice::Preview;
            argIndex++;
        }
        else
        {
            argIndex++;
        }
    }

    if (g_pixCaptureEnabled)
    {
        PIXLoadLatestWinPixGpuCapturerLibrary();
        PIXSetHUDOptions(PIX_HUD_SHOW_ON_NO_WINDOWS);
    }

    if (g_renderDocEnabled)
    {
        TryLoadRenderDocAPI();
    }

    PythonInit(&g_python);

    if (g_isForEditor)
    {
        if (g_previewClient.Start(g_editorIP, g_editorPort) != 0)
        {
            printf("Could not connect to editor, shutting down.\n");
            return 1;
        }
    }

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Gigi Viewer - DX12", NULL};
    wc.hIcon = LoadIcon(GetModuleHandle(nullptr), IDI_APPLICATION);
    ::RegisterClassExW(&wc);

	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;
    int width = 1280;
    int height = 800;
	{
		// Very large screen resolutions (e.g. 32x9) make the window appear to be too far left.
		// This compensates for that.

		// assuming we start on the primary display
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // center
        if(screenWidth > width && screenHeight > height)
		{
			x = (screenWidth - width) / 2;
			y = (screenHeight - height) / 2;
		}
	}

    g_hwnd = ::CreateWindowW(wc.lpszClassName, L"Gigi Viewer - DX12 (Gigi v" GIGI_VERSION() ")", WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, wc.hInstance, NULL);
    DragAcceptFiles(g_hwnd, true);

	Log(LogLevel::Info, "Gigi Viewer " GIGI_VERSION_WITH_BUILD_NUMBER() " DX12 " BUILD_FLAVOR);

    // Initialize Direct3D
    if (!CreateDeviceD3D(g_hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(g_hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // adjustment to make log scroll revision more recognizable
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    }

    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    // Menus and Popups partial see through is cool but can be distracting.
    style.Colors[ImGuiCol_PopupBg].w = 1.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
        g_swapChainFormat, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    // Do gigi setup
    {
        SetGigiPrintMessage(
            [](MessageType messageType, const char* msg) {
                Log((LogLevel)messageType, "%s", msg);
            }
        );
    }

    // Init the interpreter
    {
        g_interpreter.SetLogFn(&Log);
        if (!g_interpreter.Init(g_pd3dDevice, g_pd3dCommandQueue, NUM_FRAMES_IN_FLIGHT, g_pd3dSrvDescHeap, c_imguiSRVHeapSize, (int)g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)))
        {
            Log(LogLevel::Error, "Could not initialize interpreter\n");
            CleanupDeviceD3D();
            return 1;
        }
    }

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // just to have something in the log making it clear where the content is located
    Log(LogLevel::Info, "GPU: '%s' (driver %s) RayTracing:%s",
        g_adapterName.c_str(),
        g_driverVersion.c_str(),
        g_interpreter.SupportsRaytracing() ? "Yes" : "No"
    );
    Log(LogLevel::Info, "CPU: '%s'", GetCPUName().c_str());

    // Main loop
    int mainRet = 0;
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                mainRet = (int)msg.wParam;
                done = true;
            }
        }
        if (done)
            break;

        // update key states
        {
            bool keypressesAllowed = !ImGui::GetIO().WantTextInput;
            memcpy(g_keyStatesLastFrame, g_keyStates, 256);
            if (keypressesAllowed)
            {
                GetKeyboardState(g_keyStates);
                for (int i = 0; i < 256; ++i)
                    g_keyStates[i] = (g_keyStates[i] & 128) != 0;
            }
            else
                memset(g_keyStates, 0, 256);
        }

        // make sure the window title is up to date
        UpdateWindowTitle();

        // process server stuff
        {
            // report when we are fully connected
            bool wasFullyConnected = g_previewClient.IsFullyConnected();
            if (!g_previewClient.Tick())
            {
                Log(LogLevel::Warn, "PreviewClient Shutdown\n");
                PostQuitMessage(0);
            }
            bool isFullyConnected = g_previewClient.IsFullyConnected();
            if (!wasFullyConnected && isFullyConnected)
                Log(LogLevel::Info, "Connected to server\n");

            // process any messages from the server
            PreviewMsgServerToClient msg;
            while (g_previewClient.PopMessage(msg))
            {
                switch(msg._index)
                {
                    #include "external/df_serialize/_common.h"
                    #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) case PreviewMsgServerToClient::c_index_##_NAME: OnServerMessage(msg.##_NAME); break;
                    // clang-format off
                    #include "external/df_serialize/_fillunsetdefines.h"
                    #include "Schemas/PreviewWindow/PreviewMsgServerToClientVariant.h"
                    // clang-format on
                }
            }
        }

        // Shortcuts
        auto& io = ImGui::GetIO();
        if (io.KeyCtrl)
        {
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_O)))
            {
                if (!g_isForEditor)
                {
                    nfdchar_t* outPath = nullptr;
                    if (NFD_OpenDialog("gg", "", &outPath) == NFD_OKAY)
                        LoadGGFile(outPath, false, true);
                }
            }
            else if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_R)))
            {
                ReloadGGFile(false);
            }
            else if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_C)))
            {
                ReloadGGFile(true);
            }
            else if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_U)))
            {
                g_hideUI = !g_hideUI;
            }
            else if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_F)))
            {
                g_fullscreen = !g_fullscreen;
                SetFullscreenMode(g_fullscreen);
            }
        }

        if (g_interpreter.SourceFilesModified())
            ReloadGGFile(false);

        RenderFrame(false);

        // handle dx12 debug layer messages
        if (g_pd3dInfoQueue)
        {
            if (g_debugLayerShown)
            {
                std::vector<char> messageBytes;
                UINT64 numMessages = g_pd3dInfoQueue->GetNumStoredMessages();
                for (UINT64 i = 0; i < numMessages; ++i)
                {
                    SIZE_T messageLen;
                    g_pd3dInfoQueue->GetMessage(i, nullptr, &messageLen);

                    messageBytes.resize(messageLen);
                    D3D12_MESSAGE* message = (D3D12_MESSAGE*)messageBytes.data();

                    g_pd3dInfoQueue->GetMessage(0, message, &messageLen);

                    LogLevel logLevel;
                    switch (message->Severity)
                    {
                        case D3D12_MESSAGE_SEVERITY_CORRUPTION: logLevel = LogLevel::Error;  break;
                        case D3D12_MESSAGE_SEVERITY_ERROR: logLevel = LogLevel::Error; break;
                        case D3D12_MESSAGE_SEVERITY_WARNING: logLevel = LogLevel::Warn; break;
                        case D3D12_MESSAGE_SEVERITY_INFO: logLevel = LogLevel::Info; break;
                        case D3D12_MESSAGE_SEVERITY_MESSAGE: logLevel = LogLevel::Info;  break;
                    }
                    Log(logLevel, "DX12: %s", message->pDescription);
                }
            }
            g_pd3dInfoQueue->ClearStoredMessages();
        }

        if (!g_commandLineLoadGGFileName.empty())
        {
            LoadGGFile(g_commandLineLoadGGFileName.c_str(), false, true);
            g_commandLineLoadGGFileName.clear();
        }

        if (!g_runPyFileName.empty())
        {
            if (g_runPyFileAddToRecentScripts)
                g_recentPythonScripts.AddEntry(g_runPyFileName.c_str());
            Log(LogLevel::Info, "Executing python script \"%s\"", g_runPyFileName.c_str());
            if (!PythonExecute(g_runPyFileName.c_str(), g_runPyArgs))
                Log(LogLevel::Error, "Could not execute python script \"%s\"", g_runPyFileName.c_str());
            else
                Log(LogLevel::Info, "Python script finished", g_runPyFileName.c_str());
            g_runPyFileName.clear();
            g_runPyArgs.clear();
            g_runPyFileAddToRecentScripts = true;
        }
    }

    WaitForLastSubmittedFrame();

    // Cleanup

    g_interpreter.Release();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(g_hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    SaveGGUserFile();

    PythonShutdown();

    if (g_renderDocEnabled)
    {
        FreeLibrary(g_renderDocModule);
    }

    return mainRet;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Handle Agility SDK Choice
    if (g_agilitySDKChoice == AgilitySDKChoice::Preview)
    {
        // SDK version
        {
            ID3D12SDKConfiguration1* sdkConfig1 = nullptr;
            D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&sdkConfig1));

            HRESULT hr = sdkConfig1->SetSDKVersion(D3D12SDKVersion_Preview, ".\\external\\AgilitySDK\\Preview\\bin\\");
            Log(LogLevel::Info, "D3D12SDKVersion: %u%s", D3D12SDKVersion_Preview, (D3D12SDKVersion_Retail != D3D12SDKVersion_Preview ? " (Preview)" : ""));

            if (FAILED(hr))
                return false;

            sdkConfig1->Release();
        }

        // Enable experimental features
        {
            HRESULT hr = D3D12EnableExperimentalFeatures(_countof(ExperimentalFeaturesEnabled), ExperimentalFeaturesEnabled, nullptr, nullptr);
            if (FAILED(hr))
                return false;
        }
    }
    else
    {
        Log(LogLevel::Info, "D3D12SDKVersion: %u", D3D12SDKVersion_Retail);
    }

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
    ID3D12Debug* pdx12Debug = NULL;
    if (g_debugLayerOn)
    {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        {
            pdx12Debug->EnableDebugLayer();

            ID3D12Debug1* pdx12Debug1 = nullptr;
            if (g_GPUValidation && SUCCEEDED(pdx12Debug->QueryInterface(IID_PPV_ARGS(&pdx12Debug1))))
            {
                pdx12Debug1->SetEnableGPUBasedValidation(true);
                pdx12Debug1->Release();
            }
        }
    }

    // Create the factory as debug if possible
    IDXGIFactory6* dxgiFactory = NULL;
    if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory))) &&
        FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory))))
    {
        return false;
    }

    {
        Log(LogLevel::Info, "GPU devices (command line -device <index>):");
        IDXGIAdapter1* adapter = nullptr;
        uint32_t index = 0;
        while (SUCCEEDED(dxgiFactory->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))))
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);
            std::string adapterName = FromWideString(desc.Description);
            Log(LogLevel::Info, "  %d. '%s'", index, adapterName.c_str());
            ++index;
        }
    }

    // Gather the adapters
    IDXGIAdapter1* adapter = nullptr;
    if ((g_useWarpAdapter && SUCCEEDED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)))) ||
        (!g_useWarpAdapter && SUCCEEDED(dxgiFactory->EnumAdapterByGpuPreference(g_GPUDeviceIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))))
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        adapter = adapter;
        g_adapterName = FromWideString(desc.Description);

        LARGE_INTEGER driverVersion;
        adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion);
        WORD nProduct = HIWORD(driverVersion.HighPart);
        WORD nVersion = LOWORD(driverVersion.HighPart);
        WORD nSubVersion = HIWORD(driverVersion.LowPart);
        WORD nBuild = LOWORD(driverVersion.LowPart);

        char buffer[256];
        sprintf(buffer, "%u.%u.%u.%u", nProduct, nVersion, nSubVersion, nBuild);
        g_driverVersion = buffer;
    }

    // Create device
    {
        const D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0
        };
        bool success = false;
        for (D3D_FEATURE_LEVEL featureLevel : featureLevels)
        {
            if (D3D12CreateDevice(adapter, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) == S_OK)
            {
                success = true;
                break;
            }
        }
        if (!success)
        {
            return false;
        }
    }
    adapter->Release();

    // [DEBUG] Setup debug interface to break on any warnings/errors
    if (pdx12Debug != NULL)
    {
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&g_pd3dInfoQueue));
        if (g_pd3dInfoQueue)
        {
            g_pd3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, DX12_BREAK_ON_ERROR());
            g_pd3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, DX12_BREAK_ON_CORRUPTION());
            g_pd3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, DX12_BREAK_ON_WARN());
            g_pd3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, false);
            g_pd3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_MESSAGE, false);

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
            g_pd3dInfoQueue->AddStorageFilterEntries(&filter);
        }

        pdx12Debug->Release();
    }

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

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaa;
    ZeroMemory(&msaa, sizeof(msaa));
    msaa.SampleCount = 4;
    if (g_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaa, sizeof(msaa)) != S_OK)
        return false;

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = c_imguiSRVHeapSize;
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

    if (g_agilitySDKChoice == AgilitySDKChoice::Preview)
    {
        ID3D12GraphicsCommandListPreview* commandList = nullptr;
        if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, NULL, IID_PPV_ARGS(&commandList)) != S_OK ||
            commandList->Close() != S_OK)
            return false;
        g_pd3dCommandList = commandList;
    }
    else
    {
        if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
            g_pd3dCommandList->Close() != S_OK)
            return false;
    }

    if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_fenceEvent == NULL)
        return false;

    {
        IDXGISwapChain1* swapChain1 = NULL;
        if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    dxgiFactory->Release();

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, NULL); g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_hSwapChainWaitableObject != NULL) { CloseHandle(g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = NULL; }
    if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = NULL; }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = NULL; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = NULL; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = NULL; }
    if (g_fence) { g_fence->Release(); g_fence = NULL; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = NULL; }
    if (g_pd3dInfoQueue) { g_pd3dInfoQueue->Release(); g_pd3dInfoQueue = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }

    IDXGIDebug1* pDebug = NULL;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
}

void CreateRenderTarget()
{
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    rtvDesc.Format = g_swapChainFormat;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        HRESULT hr;
        ID3D12Resource* pBackBuffer = NULL;
        hr = g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        assert(SUCCEEDED(hr) && "Failed to get swap chain back buffer.");
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &rtvDesc, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = NULL; }
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

    HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, NULL };
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
    case WM_DROPFILES:
    {
        unsigned int dropFileNameLen = DragQueryFileA((HDROP)wParam, 0, nullptr, 0);
        std::vector<char> dropFileName(dropFileNameLen + 1);
        unsigned int ret = DragQueryFileA((HDROP)wParam, 0, dropFileName.data(), (UINT)dropFileName.size());
        LoadGGFile(dropFileName.data(), false, true);
        return 0;
    }
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
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
