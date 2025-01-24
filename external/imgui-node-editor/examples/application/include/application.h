# pragma once
# include <imgui.h>
# include <string>
# include <memory>

struct Platform;
struct Renderer;

struct Application
{
    Application(const char* name);
    Application(const char* name, int argc, char** argv);
    ~Application();

    bool Create(int width = -1, int height = -1);

    int Run();

    void SetTitle(const char* title);

    bool Close();
    void Quit();

    const std::string& GetName() const;

    ImFont* DefaultFont() const;
    ImFont* HeaderFont() const;

    // Gigi Change Begin
    /*
    ImTextureID LoadTexture(const char* path);
    ImTextureID CreateTexture(const void* data, int width, int height);
    void        DestroyTexture(ImTextureID texture);
    int         GetTextureWidth(ImTextureID texture);
    int         GetTextureHeight(ImTextureID texture);
    */
    // Gigi Change End

    virtual void OnStart() {}
    virtual void OnStop() {}
    virtual void OnFrame(float deltaTime) {}

    virtual ImGuiWindowFlags GetWindowFlags() const;

    virtual bool CanClose() { return true; }

    // Gigi change begin
    std::string GetAndClearDragFile()
    {
        std::string ret = m_dragFile;
        m_dragFile.clear();
        return ret;
    }

    void SetDragFile(const char* file)
    {
        m_dragFile = file;
    }

protected:
	void SetStyleAndTheme(bool dark = true);
    // Gigi change end
    void RecreateFontAtlas();

    void Frame();

    std::string                 m_Name;
    std::string                 m_IniFilename;
    std::unique_ptr<Platform>   m_Platform;
    std::unique_ptr<Renderer>   m_Renderer;
    ImGuiContext*               m_Context = nullptr;
    ImFont*                     m_DefaultFont = nullptr;
    ImFont*                     m_HeaderFont = nullptr;

    // Gigi change begin
    std::string                 m_dragFile;
    // Gigi change end
};

int Main(int argc, char** argv);