# include "application.h"
# include "setup.h"
# include "platform.h"
# include "renderer.h"

// Gigi change Begin
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>
// Gigi change End

extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"
}


Application::Application(const char* name)
    : Application(name, 0, nullptr)
{
}

Application::Application(const char* name, int argc, char** argv)
    : m_Name(name)
    , m_Platform(CreatePlatform(*this))
    , m_Renderer(CreateRenderer())
{
    m_Platform->ApplicationStart(argc, argv);
}

Application::~Application()
{
    m_Renderer->Destroy();

    m_Platform->ApplicationStop();

    if (m_Context)
    {
        ImGui::DestroyContext(m_Context);
        m_Context= nullptr;
    }
}

bool Application::Create(int width /*= -1*/, int height /*= -1*/)
{
    m_Context = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_Context);

	// Gigi Changes Begin
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	// Gigi Changes End

    if (!m_Platform->OpenMainWindow("Application", width, height))
        return false;

    if (!m_Renderer->Create(*m_Platform))
        return false;

    m_IniFilename = m_Name + ".ini";

    //ImGuiIO& io = ImGui::GetIO(); // Gigi Changes Begin End
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.IniFilename = m_IniFilename.c_str();
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();
    SetStyleAndTheme();

    RecreateFontAtlas();

    m_Platform->AcknowledgeWindowScaleChanged();
    m_Platform->AcknowledgeFramebufferScaleChanged();

    OnStart();

    Frame();

    return true;
}

int Application::Run()
{
    m_Platform->ShowMainWindow();

    while (m_Platform->ProcessMainWindowEvents())
    {
        if (!m_Platform->IsMainWindowVisible())
            continue;

        Frame();
    }

    OnStop();

    return 0;
}

void Application::SetStyleAndTheme(bool dark)
{
	// Style
	ImGuiStyle* style = &ImGui::GetStyle();

	style->AntiAliasedFill = true;
	style->AntiAliasedLines = true;
	style->AntiAliasedLinesUseTex = true;

	style->WindowPadding = ImVec2(4.0f, 4.0f);
	style->FramePadding = ImVec2(4.0f, 4.0f);
	style->TabMinWidthForCloseButton = 0.1f;
	style->CellPadding = ImVec2(8.0f, 4.0f);
	style->ItemSpacing = ImVec2(8.0f, 3.0f);
	style->ItemInnerSpacing = ImVec2(2.0f, 4.0f);
	style->TouchExtraPadding = ImVec2(0.0f, 0.0f);
	style->IndentSpacing = 12;
	style->ScrollbarSize = 14;
	style->GrabMinSize = 10;

	style->WindowBorderSize = 1.0f;
	style->ChildBorderSize = 0.0f;
	style->PopupBorderSize = 1.5f;
	style->FrameBorderSize = 0.5f;
	style->TabBorderSize = 0.0f;

	style->WindowRounding = 0.0f;
	style->ChildRounding = 0.0f;
	style->FrameRounding = 0.0f;
	style->PopupRounding = 0.0f;
	style->ScrollbarRounding = 3.0f;
	style->GrabRounding = 0.0f;
	style->LogSliderDeadzone = 4.0f;
	style->TabRounding = 0.0f;

	style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style->WindowMenuButtonPosition = ImGuiDir_Right;
	style->ColorButtonPosition = ImGuiDir_Left;
	style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style->SelectableTextAlign = ImVec2(0.0f, 0.0f);
	style->DisplaySafeAreaPadding = ImVec2(8.0f, 8.0f);

	ImGuiColorEditFlags colorEditFlags = ImGuiColorEditFlags_AlphaBar
		| ImGuiColorEditFlags_AlphaPreviewHalf
		| ImGuiColorEditFlags_DisplayRGB
		| ImGuiColorEditFlags_InputRGB
		| ImGuiColorEditFlags_PickerHueBar
		| ImGuiColorEditFlags_Uint8;
	ImGui::SetColorEditOptions(colorEditFlags);

	// Colors
	ImVec4* colors = style->Colors;

	if (dark)
	{
		colors[ImGuiCol_Text] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.275f, 0.275f, 0.275f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.000f, 0.434f, 0.878f, 1.000f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.000f, 0.434f, 0.878f, 1.000f);
		colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.000f, 0.439f, 0.878f, 0.824f);
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 0.47f, 0.94f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.47f, 0.94f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.197f, 0.197f, 0.197f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.197f, 0.197f, 0.197f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.188f, 0.529f, 0.780f, 1.000f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.44f, 0.88f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
	else
	{
		colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
		colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.70f, 0.82f, 0.95f, 0.39f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.41f, 0.67f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.82f, 0.95f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.91f, 0.91f, 0.91f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.89f, 0.89f, 0.89f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.81f, 0.81f, 0.81f, 0.62f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.56f, 0.56f, 0.56f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.91f, 0.91f, 0.91f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.91f, 0.91f, 0.91f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}
}

void Application::RecreateFontAtlas()
{
    ImGuiIO& io = ImGui::GetIO();

    IM_DELETE(io.Fonts);

    io.Fonts = IM_NEW(ImFontAtlas);

    // Gigi change Begin

	float fontSize = 16.0f;

	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = false;
	iconsConfig.PixelSnapH = true;
	iconsConfig.OversampleH = iconsConfig.OversampleV = 4;
	iconsConfig.GlyphMinAdvanceX = 4.0f;
	iconsConfig.SizePixels = fontSize;

	// Get path to font, relative to executable, not working directory.
	std::filesystem::path fontPath;
	{
		char exePath[_MAX_PATH + 1];
		GetModuleFileNameA(NULL, exePath, _MAX_PATH);
		fontPath = exePath;
		fontPath.replace_filename("");
		fontPath = fontPath / "external" / "fonts" / "OpenSans-Regular.ttf";
	}

	m_DefaultFont = io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 16.0f, &iconsConfig);
	m_HeaderFont = m_DefaultFont;// io.Fonts->AddFontFromFileTTF("external/fonts/OpenSans-Regular.ttf", 20.0f, &iconsConfig);

	io.Fonts->TexGlyphPadding = 1;
	for (int n = 0; n < io.Fonts->ConfigData.Size; n++)
	{
		ImFontConfig* fontConfig = &io.Fonts->ConfigData[n];
		fontConfig->RasterizerMultiply = 1.0f;
	}

    /*
    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 4;
    config.PixelSnapH = false;

    m_DefaultFont = io.Fonts->AddFontFromFileTTF("data/Play-Regular.ttf", 18.0f, &config);
    m_HeaderFont  = io.Fonts->AddFontFromFileTTF("data/Cuprum-Bold.ttf",  20.0f, &config);
    */
    // Gigi change End

    io.Fonts->Build();
}

void Application::Frame()
{
    auto& io = ImGui::GetIO();

    if (m_Platform->HasWindowScaleChanged())
        m_Platform->AcknowledgeWindowScaleChanged();

    if (m_Platform->HasFramebufferScaleChanged())
    {
        RecreateFontAtlas();
        m_Platform->AcknowledgeFramebufferScaleChanged();
    }

    const float windowScale      = m_Platform->GetWindowScale();
    const float framebufferScale = m_Platform->GetFramebufferScale();

    if (io.WantSetMousePos)
    {
        io.MousePos.x *= windowScale;
        io.MousePos.y *= windowScale;
    }

    m_Platform->NewFrame();

    // Don't touch "uninitialized" mouse position
    if (io.MousePos.x > -FLT_MAX && io.MousePos.y > -FLT_MAX)
    {
        io.MousePos.x    /= windowScale;
        io.MousePos.y    /= windowScale;
    }
    io.DisplaySize.x /= windowScale;
    io.DisplaySize.y /= windowScale;

    io.DisplayFramebufferScale.x = framebufferScale;
    io.DisplayFramebufferScale.y = framebufferScale;

    m_Renderer->NewFrame();

    ImGui::NewFrame();

	// Gigi change begin
	if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);
		const auto windowBorderSize = ImGui::GetStyle().WindowBorderSize;
		const auto windowRounding = ImGui::GetStyle().WindowRounding;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::Begin("Content", nullptr, GetWindowFlags());
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, windowBorderSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, windowRounding);
	}
	// Gigi change end

    OnFrame(io.DeltaTime);

	// Gigi change begin
	if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
	{
		ImGui::PopStyleVar(2);
		ImGui::End();
		ImGui::PopStyleVar(2);
	}
	// Gigi change end

    // Rendering
    m_Renderer->Clear(ImColor(32, 32, 32, 255));
    ImGui::Render();
    m_Renderer->RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    m_Platform->FinishFrame();
}

void Application::SetTitle(const char* title)
{
    m_Platform->SetMainWindowTitle(title);
}

bool Application::Close()
{
    return m_Platform->CloseMainWindow();
}

void Application::Quit()
{
    m_Platform->Quit();
}

const std::string& Application::GetName() const
{
    return m_Name;
}

ImFont* Application::DefaultFont() const
{
    return m_DefaultFont;
}

ImFont* Application::HeaderFont() const
{
    return m_HeaderFont;
}

// Gigi change begin
/*
ImTextureID Application::LoadTexture(const char* path)
{
    int width = 0, height = 0, component = 0;
    if (auto data = stbi_load(path, &width, &height, &component, 4))
    {
        auto texture = CreateTexture(data, width, height);
        stbi_image_free(data);
        return texture;
    }
    else
        return nullptr;
}

ImTextureID Application::CreateTexture(const void* data, int width, int height)
{
    return m_Renderer->CreateTexture(data, width, height);
}

void Application::DestroyTexture(ImTextureID texture)
{
    m_Renderer->DestroyTexture(texture);
}

int Application::GetTextureWidth(ImTextureID texture)
{
    return m_Renderer->GetTextureWidth(texture);
}

int Application::GetTextureHeight(ImTextureID texture)
{
    return m_Renderer->GetTextureHeight(texture);
}
*/
// Gigi change end

ImGuiWindowFlags Application::GetWindowFlags() const
{
	return
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_MenuBar;  // Gigi Changes Begin End
}
