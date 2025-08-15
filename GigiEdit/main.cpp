///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
#include <application.h>
#include "platform.h"

#include <string>
#include <unordered_map>
#include <sstream>
#include <type_traits>

#include <imgui.h>
#include <imgui_node_editor.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <nfd.h>

#include "Schemas/Types.h"
#include "Schemas/JSON.h"
#include "EditorNodes.h"
#include "MakeTypePath.h"

#include "GigiCompilerLib/gigicompiler.h"
#include "Schemas/Visitor.h"
#include "visitors.h"
#include "StableSample.h"
#include "GigiCompilerLib/structParser.h"

#include "PreviewServer.h"
#include "ui/RecentFiles.h"
// clang-format on

// Prototypes for functions that the UI wants to call.
void OnShaderResourceAdd(const Shader& shader, const std::string& resourceName);
void OnShaderResourceDelete(const Shader& shader, const std::string& resourceName);
void OnShaderResourceRename(const Shader& shader, const std::string& oldName, const std::string& newName);
void OnNodeRename(const std::string& oldName, const std::string& newName);
void OnGoToShader(const char* name);
void OnGoToVariable(const char* name);
void OnGoToStruct(const char* name);

bool RefreshSubGraphNode(RenderGraphNode_Action_SubGraph& subGraphNode);
bool RefreshSubGraphNodes();

// clang-format off
#include "ui/commonUI.h"
#include "MakeUI.h"
#include "ui/pins.h"
// clang-format on

#ifdef _DEBUG
#define BUILD_FLAVOR() "Debug"
#else
#define BUILD_FLAVOR() "Release"
#endif

static const int c_listenPort = 7171;

int g_argc = 0;
char** g_argv = nullptr;

namespace ed = ax::NodeEditor;

constexpr int32_t g_groupNodesStartId = 100000;

RenderGraph g_renderGraph;
std::string g_renderGraphFileName = "";
bool g_renderGraphDirty = false;
bool g_renderGraphFirstFrame = false;
std::string g_currentWindowTitle;
GigiBuildFlavor g_gigiBuildFlavor = GigiBuildFlavor::DX12_Module;

CPreviewServer g_previewWindowServer;
PROCESS_INFORMATION g_previewWindow = { nullptr, nullptr, 0, 0 };
bool g_openPreviewWindowPaused = false;

ed::NodeId g_contextMenuNodeId;
ed::LinkId g_contextMenuLinkId;

int g_createdNodeIndex = -1;

struct DataWindowState
{
    bool show = true;
    size_t selectedIndex = 0;
    std::string searchQuery;
};

bool g_resetLayout = true;
struct ShowWindowsState
{
    bool Nodes = true;
    bool GraphProperties = true;
    bool NodeProperties = true;
    bool BuildLog = true;
    DataWindowState Variables;
    DataWindowState Shaders;
    DataWindowState Structs;
    DataWindowState FileCopies;
    DataWindowState Enums;
    DataWindowState SetVariables;
    DataWindowState RTHitGroups;
};
ShowWindowsState g_showWindows;

// Portions of this software were based on https://devblogs.microsoft.com/oldnewthing/20100125-00/?p=15183
HANDLE SetClipboardDataEx(UINT uFormat, void *pvData, DWORD cbData)
{
    if (OpenClipboard(NULL))
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

void EditorShowMessageBox(const char* msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, msg, args);
    va_end(args);

    MessageBoxA(nullptr, buffer, "GigiEdit", MB_OK);
}

bool LoadTextFile(const char* fileName, std::string& text)
{
	FILE* file = nullptr;
	fopen_s(&file, fileName, "rb");
	if (!file)
		return false;

	fseek(file, 0, SEEK_END);
	std::vector<char> data(ftell(file) + 1, 0);
	fseek(file, 0, SEEK_SET);

	size_t ret = fread(data.data(), 1, data.size() - 1, file);

	if (ret != data.size() - 1)
		return false;

	fclose(file);

	text = data.data();
	return true;
}

void OnShaderResourceAdd(const Shader& shader, const std::string& resourceName)
{
}

void OnShaderResourceDelete(const Shader& shader, const std::string& resourceName)
{
    // remove the pin for this resourceName from all nodes that use this shader
    std::vector<std::string> shaderNodes;
    for (int nodeIndex = 0; nodeIndex < g_renderGraph.nodes.size(); ++nodeIndex)
    {
        RenderGraphNode& node = g_renderGraph.nodes[nodeIndex];

        switch (node._index)
        {
            case RenderGraphNode::c_index_actionComputeShader:
            {
                if (node.actionComputeShader.shader.name != shader.name)
                    continue;

                shaderNodes.push_back(node.actionComputeShader.name);
                break;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                if (node.actionRayShader.shader.name != shader.name)
                    continue;
                shaderNodes.push_back(node.actionRayShader.name);
                break;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                if (node.actionDrawCall.pixelShader.name != shader.name
                    && node.actionDrawCall.vertexShader.name != shader.name
                    && node.actionDrawCall.amplificationShader.name != shader.name
                    && node.actionDrawCall.meshShader.name != shader.name)
                    continue;

                shaderNodes.push_back(node.actionDrawCall.name);
                break;
            }
        }
    }

    // Also remove any connections which connect to any of these nodes and pins.
    // They will show up as links which have a node but not a pin
    for (RenderGraphNode& node : g_renderGraph.nodes)
    {
        std::vector<NodePinInfo> pinInfo = GetNodePins(g_renderGraph, node);
        for (NodePinInfo& p : pinInfo)
        {
            if (!p.isInput)
                continue;

            for (const std::string& shaderNode : shaderNodes)
            {
                if (*p.inputNode == shaderNode && *p.inputNodePin == resourceName)
                {
                    *p.inputNode = "";
                    *p.inputNodePin = "";
                }
            }
        }
    }
}

void OnShaderResourceRename(const Shader& shader, const std::string& oldName, const std::string& newName)
{
    // For all nodes that use this shader...
    // 1) Rename the pin from the oldName to the newName in the node data (connections).
    // 2) put the name of the node into a list so we can update everything that links to this pin that got renamed
    std::vector<std::string> shaderNodes;
    for (int nodeIndex = 0; nodeIndex < g_renderGraph.nodes.size(); ++nodeIndex)
    {
        RenderGraphNode& node = g_renderGraph.nodes[nodeIndex];

        switch (node._index)
        {
            case RenderGraphNode::c_index_actionComputeShader:
            {
                if (node.actionComputeShader.shader.name != shader.name)
                    continue;
                shaderNodes.push_back(shader.name);

                for (NodePinConnection& connection : node.actionComputeShader.connections)
                {
                    if (connection.srcPin == oldName)
                        connection.srcPin = newName;
                }
                break;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                if (node.actionRayShader.shader.name != shader.name)
                    continue;
                shaderNodes.push_back(shader.name);

                for (NodePinConnection& connection : node.actionRayShader.connections)
                {
                    if (connection.srcPin == oldName)
                        connection.srcPin = newName;
                }
                break;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                if (node.actionDrawCall.vertexShader.name != shader.name
                    && node.actionDrawCall.pixelShader.name != shader.name
                    && node.actionDrawCall.meshShader.name != shader.name
                    && node.actionDrawCall.amplificationShader.name != shader.name)
                    continue;
                shaderNodes.push_back(shader.name);

                for (NodePinConnection& connection : node.actionDrawCall.connections)
                {
                    if (connection.srcPin == oldName)
                        connection.srcPin = newName;
                }
                break;
            }
        }
    }

    // Any node that linked to one of those nodes, and that pin, needs to have the pin renamed from the old name to the new name
    // in the node data (connections)
    for (RenderGraphNode& node : g_renderGraph.nodes)
    {
        std::vector<NodePinInfo> pinInfo = GetNodePins(g_renderGraph, node);
        for (NodePinInfo& p : pinInfo)
        {
            if (!p.isInput)
                continue;

            for (const std::string& shaderNode : shaderNodes)
            {
                if (*p.inputNode == shaderNode && *p.inputNodePin == oldName)
                    *p.inputNodePin = newName;
            }
        }
    }
}

void OnNodeRename(const std::string& oldName, const std::string& newName)
{
    // A node has changed names, so need to update anything that links to it, since the links are by name
    for (int nodeIndex = 0; nodeIndex < g_renderGraph.nodes.size(); ++nodeIndex)
    {
        RenderGraphNode& node = g_renderGraph.nodes[nodeIndex];

        std::vector<NodePinInfo> pinInfo = GetNodePins(g_renderGraph, node);
        for (NodePinInfo& p : pinInfo)
        {
            if (!p.isInput)
                continue;

            if (*p.inputNode == oldName)
                *p.inputNode = newName;

        }
    }

    if (oldName.empty())
        return;

    // let the rest of the render graph update.
    OnNodeRenameVisitor visitor(g_renderGraph, oldName, newName);
    Visit(g_renderGraph, visitor, "renderGraph");
}

void OnGoToShader(const char* name)
{
    for (size_t i = 0; i < g_renderGraph.shaders.size(); ++i)
    {
        if (!_stricmp(g_renderGraph.shaders[i].name.c_str(), name))
        {
            g_showWindows.Shaders.show = true;
            g_showWindows.Shaders.selectedIndex = i;
            ImGui::SetWindowFocus("Shaders");
            return;
        }
    }
}

void OnGoToVariable(const char* name)
{
    for (size_t i = 0; i < g_renderGraph.variables.size(); ++i)
    {
        if (!_stricmp(g_renderGraph.variables[i].name.c_str(), name))
        {
            g_showWindows.Variables.show = true;
            g_showWindows.Variables.selectedIndex = i;
            ImGui::SetWindowFocus("Variables");
            return;
        }
    }
}

void OnGoToStruct(const char* name)
{
    for (size_t i = 0; i < g_renderGraph.structs.size(); ++i)
    {
        if (!_stricmp(g_renderGraph.structs[i].name.c_str(), name))
        {
            g_showWindows.Structs.show = true;
            g_showWindows.Structs.selectedIndex = i;
            ImGui::SetWindowFocus("Structs");
            return;
        }
    }
}

void OnChangeGGFileName(bool preserveState)
{
    if (!g_previewWindowServer.IsFullyConnected())
        return;

    PreviewMsgServerToClient msg;
    msg._index = PreviewMsgServerToClient::c_index_loadGGFile;
    msg.loadGGFile.fileName = g_renderGraphFileName;
    msg.loadGGFile.preserveState = preserveState;
    g_previewWindowServer.Send(msg);
}

bool RefreshSubGraphNode(RenderGraphNode_Action_SubGraph& subGraphNode)
{
    // load the child graph
    std::string childFileName = (std::filesystem::path(g_renderGraph.editorFileName).remove_filename() / std::filesystem::path(subGraphNode.fileName)).string();
    RenderGraph childGraph;
    if (ReadFromJSONFile(childGraph, childFileName.c_str()))
    {
        if (childGraph.version != std::string(GIGI_VERSION()))
        {
            EditorShowMessageBox("Could not refresh subgraph data. File %s is version %s and couldn't be upgraded to version %s.", childFileName.c_str(), childGraph.version.c_str(), GIGI_VERSION());
            return false;
        }
    }
    else
    {
        //EditorShowMessageBox("Could not load subgraph \"%s\".", childFileName.c_str());
        return false;
    }

    // get the list of imported and exported resources
    SubGraphData subGraphData;
    for (const RenderGraphNode& childGraphNode : childGraph.nodes)
    {
        ResourceVisibility visibility;
        if (!GetResourceVisibility(childGraphNode, visibility))
            continue;

        switch (visibility)
        {
            case ResourceVisibility::Imported: subGraphData.importedResources.push_back(GetNodeName(childGraphNode)); break;
            case ResourceVisibility::Exported: subGraphData.exportedResources.push_back(GetNodeName(childGraphNode)); break;
        }
    }

    // get the variables from the sub graph
    subGraphData.variables.resize(childGraph.variables.size());
    for (size_t i = 0; i < childGraph.variables.size(); ++i)
    {
        subGraphData.variables[i].name = childGraph.variables[i].name;
        subGraphData.variables[i].visibility = childGraph.variables[i].visibility;
    }

    // if there is no change, don't mark the graph as dirty
    if (subGraphData == subGraphNode.subGraphData)
        return false;

    // Fix up connections array
    {
        // make an array for the new connections
        std::vector<NodePinConnection> newConnections(subGraphData.importedResources.size() + subGraphData.exportedResources.size());
        for (size_t i = 0; i < subGraphData.importedResources.size(); ++i)
            newConnections[i].srcPin = subGraphData.importedResources[i];
        for (size_t i = 0; i < subGraphData.exportedResources.size(); ++i)
            newConnections[i + subGraphData.importedResources.size()].srcPin = subGraphData.exportedResources[i];

        // Map old connections to new ones
        for (const NodePinConnection& oldConnection : subGraphNode.connections)
        {
            for (NodePinConnection& newConnection : newConnections)
            {
                if (newConnection.srcPin != oldConnection.srcPin)
                    continue;

                newConnection.dstNode = oldConnection.dstNode;
                newConnection.dstPin = oldConnection.dstPin;
                break;
            }
        }

        // Set the new connection
        subGraphNode.connections = newConnections;
    }

    // Fix up variable settings
    {
        // Map old variable settings to new ones
        std::vector<SubGraphVariableSettings> newVariableSettings(subGraphData.variables.size());
        for (size_t i = 0; i < newVariableSettings.size(); ++i)
        {
            // Find the old settings by name if we can
            bool found = false;
            for (const SubGraphVariableSettings& oldSettings : subGraphNode.variableSettings)
            {
                if (oldSettings.name == subGraphData.variables[i].name)
                {
                    newVariableSettings[i] = oldSettings;
                    found = true;
                    break;
                }
            }

            // Else it's a new entry, so set the name and visibility
            if (!found)
            {
                newVariableSettings[i].name = subGraphData.variables[i].name;
                newVariableSettings[i].visibility = subGraphData.variables[i].visibility;
            }
        }

        // set the new variable settings
        subGraphNode.variableSettings = newVariableSettings;
    }

    // Set the new sub graph data
    subGraphNode.subGraphData = subGraphData;

    // let the caller know that things got changed
    return true;
}

bool RefreshSubGraphNodes(void)
{
    bool dataChanged = false;
    for (RenderGraphNode& nodeBase : g_renderGraph.nodes)
    {
        if (nodeBase._index != RenderGraphNode::c_index_actionSubGraph)
            continue;
        dataChanged |= RefreshSubGraphNode(nodeBase.actionSubGraph);
    }
    return dataChanged;
}

struct Example :
    public Application
{
    using Application::Application;

    struct LogMessage
    {
		MessageType Type;
		std::string Msg;

        LogMessage(MessageType type, std::string_view msg) : Type(type), Msg(msg) {}
    };

    // FIXME: Use Ring Buffer instead
    std::vector<LogMessage> m_BuildOutputBuffer;
    std::vector<LogMessage> m_previewWindowLog;

    RecentFiles m_recentFiles;

    inline static Example* s_thisExample = nullptr;

    Example(const char* name, int argc, char** argv)
        : Application(name, argc, argv)
        , m_recentFiles((std::string("Software\\") + name).c_str())
    {
    }

    void OnStart() override
    {
        m_recentFiles.LoadAllEntries();

        s_thisExample = this;

        ed::Config config;
        config.SaveSettings = [](const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) { return true; };
        m_Context = ed::CreateEditor(&config);

        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        SetGigiPrintMessage(
            [] (MessageType messageType, const char* msg) {
                Example::s_thisExample->m_BuildOutputBuffer.emplace_back(messageType, msg);
            }
        );

        SetGigiHeadlessMode(true);
    }

    void OnStop() override
    {
        ed::DestroyEditor(m_Context);
    }

    void UpdateWindowTitle()
    {
        std::string newWindowTitle;
        newWindowTitle = "GigiEdit (v" GIGI_VERSION_WITH_BUILD_NUMBER() " " BUILD_FLAVOR() ") - ";
        if (!g_renderGraphFileName.empty())
            newWindowTitle += g_renderGraphFileName;
        else
            newWindowTitle += "<Untitled>";
        if (g_renderGraphDirty)
            newWindowTitle += "*";

        if (g_currentWindowTitle == newWindowTitle)
            return;

        g_currentWindowTitle = newWindowTitle;
        m_Platform->SetMainWindowTitle(g_currentWindowTitle.c_str());
    }

    void LoadJSONFile(const char* fileName)
    {
        m_recentFiles.AddEntry(fileName);

        RenderGraph renderGraph;
        if (ReadFromJSONFile(renderGraph, fileName))
        {
            /*
            if (renderGraph.version != std::string(GIGI_VERSION()))
            {
                EditorShowMessageBox("File %s is version %s, but needs to be %s.", fileName, renderGraph.version.c_str(), GIGI_VERSION());
            }
            else
                */
            {
                g_renderGraph = renderGraph;
                g_renderGraphDirty = renderGraph.versionUpgraded;
                g_renderGraphFirstFrame = true;
                g_renderGraphFileName = fileName;
                g_renderGraph.editorFileName = g_renderGraphFileName;
                ed::ClearSelection();

                OnChangeGGFileName(false);
            }

            // Reload any data from sub graph nodes
            g_renderGraphDirty |= RefreshSubGraphNodes();

            if (renderGraph.versionUpgraded && !renderGraph.versionUpgradedMessage.empty())
            {
                std::string fullMessage = "File Upgraded:\n" + renderGraph.versionUpgradedMessage;
                EditorShowMessageBox(fullMessage.c_str());
                m_BuildOutputBuffer.emplace_back(MessageType::Warn, fullMessage);
            }
        }
        else
        {
            EditorShowMessageBox("Could not load file %s", fileName);
        }
    }

    void SaveJSONFile(const char* fileName)
    {
		m_recentFiles.AddEntry(fileName);

        // Update node positions so we can save them
        for (int nodeIndex = 0; nodeIndex < (int)g_renderGraph.nodes.size(); ++nodeIndex)
        {
            ImVec2 pos = ed::GetNodePosition(nodeIndex + 1);
            SetNodeEditorPos(g_renderGraph.nodes[nodeIndex], { pos.x, pos.y });
        }

        WriteToJSONFile(g_renderGraph, fileName);
        g_renderGraphDirty = false;
    }

    void BuildGraph()
    {
        if (g_renderGraphDirty || g_renderGraphFileName.empty())
        {
            EditorShowMessageBox("This file must be saved before it can be built.");
        }
        else
        {
            //m_BuildOutputBuffer.clear();
            Backend backend;
            GigiBuildFlavorBackend(g_gigiBuildFlavor, backend);

            std::filesystem::path renderGraphDirectory(g_renderGraphFileName);
            renderGraphDirectory.replace_filename("");

            switch (backend)
            {
                // Build individual backends
                #include "external/df_serialize/_common.h"
                #define ENUM_BEGIN(_NAME, _DESCRIPTION)
                #define ENUM_ITEM(_NAME, _DESCRIPTION) case Backend::##_NAME: GigiCompile(g_gigiBuildFlavor, g_renderGraphFileName, std::filesystem::weakly_canonical(std::filesystem::path(renderGraphDirectory) / g_renderGraph.buildSettings.out##_NAME).string(), PostLoad_##_NAME, nullptr, false); break;
                #define ENUM_END()
                #include "Schemas/BackendList.h"
            }
		}
    }

    void CreateGraph()
    {
		if (!g_renderGraphDirty || AskForConfirmation("You have unsaved changes, are you sure you want to proceed?"))
		{
			g_renderGraph = RenderGraph();
			g_renderGraphDirty = false;
			g_renderGraphFileName = "";
			g_renderGraph.editorFileName = g_renderGraphFileName;
			ed::ClearSelection();

            OnChangeGGFileName(false);
		}
    }

    void OpenGraph()
    {
		if (!g_renderGraphDirty || AskForConfirmation("You have unsaved changes, are you sure you want to proceed?"))
		{
			// e.g. "C:\\gitlab\\gigi"
			std::filesystem::path defaultPath = std::filesystem::current_path();
			std::string exploreLocation = (defaultPath / "Techniques").u8string();

			nfdchar_t* outPath = nullptr;
			if (NFD_OpenDialog("gg", exploreLocation.c_str(), &outPath) == NFD_OKAY)
				LoadJSONFile(outPath);
		}
    }

    void SaveGraph(const char* filepath)
    {
        if (g_renderGraph.version.empty())
            g_renderGraph.version = GIGI_VERSION();

        if (filepath)
        {
            g_renderGraphFileName = filepath;
			SaveJSONFile(g_renderGraphFileName.c_str());
        }
        else
        {
            // try absolute path of current filename first
            std::string path = std::filesystem::path(g_renderGraphFileName).remove_filename().string();
			if (!path.empty() && path.back() == std::filesystem::path::preferred_separator)
				path.pop_back();  // remove trailing slash

            // fall back to absolute path with techniques folder
            if(path.empty())
	            path = (std::filesystem::current_path() / "Techniques").string();

			nfdchar_t* outPath = nullptr;
            if (NFD_SaveDialog("gg", (nfdchar_t*)path.c_str(), &outPath) == NFD_OKAY)
			{
				g_renderGraphFileName = outPath;

				// Add a .gg to the end of the file if it isn't already there
				std::string extension;
				size_t extensionStart = g_renderGraphFileName.find_last_of(".");
				if (extensionStart != std::string::npos)
					extension = g_renderGraphFileName.substr(extensionStart);
				if (extension != ".gg")
					g_renderGraphFileName += std::string(".gg");

				g_renderGraph.editorFileName = g_renderGraphFileName;

				SaveJSONFile(g_renderGraphFileName.c_str());
			}
        }

        OnChangeGGFileName(true);
    }

    void HandleMainMenu()
    {
        auto& io = ImGui::GetIO();

		static bool imguiDemoOpen = false;
		static bool structParserTest = false;

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 3.0f));

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                    CreateGraph();
                }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    OpenGraph();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S", false, !g_renderGraphFileName.empty()))
                {
                    SaveGraph(g_renderGraphFileName.c_str());
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                {
                    SaveGraph(nullptr);
                }
                ImGuiRecentFiles();

                ImGui::Separator();

				if (ImGui::MenuItem("Build", "Ctrl+B"))
					BuildGraph();

				ImGui::Separator();

                // very useful during development (e.g. find examples, find style name or tweak theme),
                // could be compiled out in Release
                #ifdef _DEBUG
                ImGui::MenuItem("ImGui Demo", nullptr, &imguiDemoOpen);
				ImGui::MenuItem("Struct Parser Test", nullptr, &structParserTest);
				ImGui::Separator();
                #endif

                if (ImGui::MenuItem("Exit"))
                {
                    if (!g_renderGraphDirty || AskForConfirmation("You have unsaved changes, are you sure you want to exit?"))
                        this->Close();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Reset Layout", "", &g_resetLayout))
                {
                    g_showWindows = ShowWindowsState();
                }

                ImGui::Separator();

                ImGui::MenuItem("Graph Properties", "", &g_showWindows.GraphProperties);
                ImGui::MenuItem("Nodes", "", &g_showWindows.Nodes);
                ImGui::MenuItem("Build Log", "", &g_showWindows.BuildLog);
                ImGui::MenuItem("Variables", "", &g_showWindows.Variables.show);
                ImGui::MenuItem("Shaders", "", &g_showWindows.Shaders.show);
                ImGui::MenuItem("Structs", "", &g_showWindows.Structs.show);
                ImGui::MenuItem("FileCopies", "", &g_showWindows.FileCopies.show);
                ImGui::MenuItem("Enums", "", &g_showWindows.Enums.show);
                ImGui::MenuItem("SetVariables", "", &g_showWindows.SetVariables.show);
                ImGui::MenuItem("RTHitGroups", "", &g_showWindows.RTHitGroups.show);

                ImGui::EndMenu();
            }

            // show build menu
            {
                std::vector<const char*> labels;
                std::vector<GigiBuildFlavor> labelFlavors;
                #define GIGI_BUILD_FLAVOR(BACKEND, FLAVOR, INTERNAL) \
                    if(!INTERNAL) \
                    { \
                        labels.push_back(#BACKEND " " #FLAVOR); \
                        labelFlavors.push_back(GigiBuildFlavor::BACKEND ## _ ## FLAVOR); \
                    }
                #include "GigiCompilerLib/GigiBuildFlavorList.h"
                #undef GIGI_BUILD_FLAVOR

                ImGui::Text("    ");

                std::string buildName = "Build ";
                if ((int)g_gigiBuildFlavor < labels.size())
                {
                    buildName += "\"";
                    buildName += labels[(int)g_gigiBuildFlavor];
                    buildName += "\" ";
                }

                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0.0f));
                    if (ImGui::Button(buildName.c_str()))
                    {
                        BuildGraph();
                    }
                    // make arrow button appear like small button
                    if (ImGui::ArrowButton("##ChooseBackend", ImGuiDir_Down))
                        //				if (ImGui::SmallButton("<"))
                        ImGui::OpenPopup("PopupBuildBackend");
					ImGui::PopStyleVar(2);
                }

				if (ImGui::BeginPopupContextItem("PopupBuildBackend"))
				{
                    for (size_t labelIndex = 0; labelIndex < labels.size(); ++labelIndex)
                    {
                        bool checked = (int)g_gigiBuildFlavor == labelIndex;
                        ImGui::MenuItem(labels[labelIndex], nullptr, &checked);
                        if (checked)
                            g_gigiBuildFlavor = labelFlavors[labelIndex];
                    }

					ImGui::EndPopup();
                }
            }

            // Preview window
            {
                // detect when the preview window is closed
                if (g_previewWindow.hProcess)
                {
                    DWORD exitCode;
                    if (!GetExitCodeProcess(g_previewWindow.hProcess, &exitCode) || exitCode != STILL_ACTIVE)
                        g_previewWindow.hProcess = nullptr;
                }

                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                bool previewWindowOpen = g_previewWindow.hProcess != nullptr;

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0.0f));
                if (ImGui::Button(previewWindowOpen ? "Close Viewer" : "Open Viewer"))
                {
                    char commandLine[1024];
                    if (g_renderGraphFileName.empty())
                        sprintf_s(commandLine, "GigiViewerDX12.exe -paused %i", g_openPreviewWindowPaused ? 1 : 0);
                    else
                        sprintf_s(commandLine, "GigiViewerDX12.exe -load \"%s\" -paused %i", g_renderGraphFileName.c_str(), g_openPreviewWindowPaused ? 1 : 0);

                    STARTUPINFOA si;
                    ZeroMemory(&si, sizeof(si));
                    si.cb = sizeof(si);

                    PROCESS_INFORMATION pi;

                    CreateProcessA(
                        nullptr,
                        (char*)commandLine,
                        nullptr,
                        nullptr,
                        FALSE,
                        0,
                        nullptr,
                        nullptr,
                        &si,
                        &pi);

                    /*
                    if (!previewWindowOpen && g_renderGraphFileName.empty())
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
                    else if (!previewWindowOpen)
                    {
                        m_previewWindowLog.emplace_back(MessageType::Info, "Starting Viewer");

                        do
                        {
                            // Start the Server
                            if (g_previewWindowServer.Start(c_listenPort) != 0)
                            {
                                m_previewWindowLog.emplace_back(MessageType::Error, "Could not start server");
                                break;
                            }
                            m_previewWindowLog.emplace_back(MessageType::Info, "PreviewServer started");

                            // Launch the process
                            char commandLine[1024];
                            sprintf_s(commandLine, "GigiViewerDX12.exe -editor 127.0.0.1 %i -paused %i", c_listenPort, g_openPreviewWindowPaused ? 1 : 0);
                            STARTUPINFOA si;
                            ZeroMemory(&si, sizeof(si));
                            si.cb = sizeof(si);

                            if (!CreateProcessA(
                                nullptr,
                                commandLine,
                                nullptr,
                                nullptr,
                                FALSE,
                                0,
                                nullptr,
                                nullptr,
                                &si,
                                &g_previewWindow))
                            {
                                m_previewWindowLog.emplace_back(MessageType::Error, "Could not start GigiViewerDX12.exe");
                                ZeroMemory(&g_previewWindow, sizeof(g_previewWindow));
                                g_previewWindowServer.Shutdown();
                                break;
                            }
                            m_previewWindowLog.emplace_back(MessageType::Info, "Launched GigiViewerDX12.exe");
                        }
                        while(0);
                    }
                    else
                    {
                        TerminateProcess(g_previewWindow.hProcess, 0);
                        ZeroMemory(&g_previewWindow, sizeof(g_previewWindow));
                        m_previewWindowLog.emplace_back(MessageType::Info, "GigiViewerDX12.exe Terminated");
                    }
                    */
                }
				ImGui::PopStyleVar(1);

                ImGui::Checkbox("Paused", &g_openPreviewWindowPaused);

                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0.0f));
                if (ImGui::Button("Open Editor"))
                {
                    STARTUPINFOA si;
                    ZeroMemory(&si, sizeof(si));
                    si.cb = sizeof(si);

                    PROCESS_INFORMATION pi;

                    CreateProcessA(
                        nullptr,
                        (char*)"GigiEdit.exe",
                        nullptr,
                        nullptr,
                        FALSE,
                        0,
                        nullptr,
                        nullptr,
                        &si,
                        &pi);
                }

                if (ImGui::Button("Open Browser"))
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
                ImGui::PopStyleVar(1);
            }

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            // Show FPS
            {
                static StableSample stableSample;
                stableSample.tick(io.DeltaTime, io.DeltaTime);
                float stableValue = stableSample.getStableAverage();
				if (stableValue != FLT_MAX)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                    // option 1
//					ImGui::Text("FPS: %.2f (%.2gms)", 1.0f / stableValue, 1000.0f * stableValue);
                    // option 2 
					ImGui::Text("FPS: %.2f (%.2g .. %.2g ms)", 1.0f / stableValue, 1000.0f * stableSample.getStableMin(), 1000.0f * stableSample.getStableMax());
					ImGui::PopStyleColor();
				}
            }

            //mainMenuBarSize = ImGui::GetWindowSize();

            ImGui::EndMenuBar();
        }

        // ImGuiStyleVar_FramePadding
		ImGui::PopStyleVar();

        if (imguiDemoOpen)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
            ImGui::ShowDemoWindow(&imguiDemoOpen);
        }

        if (structParserTest)
        {
			ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(400, 200), ImVec2(FLT_MAX, FLT_MAX));
			ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Struct Parser Test", &structParserTest)) 
			{
                static char input[10 * 1024];
                static bool first = true;

                if (first) 
                {
                    first = false;
                    std::string text;
                    LoadTextFile("GigiEdit/parserTest1.txt", text);
                    // to avoid issues with IMGUI \r\n handing and line counting for errors
                    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
                    if (text.size() < sizeof(input) - 1)
                        strcpy_s(input, text.data());
                }

                ImGui::InputTextMultiline("##input", input, sizeof(input), ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 3));

				static std::vector<Enum> enums;
                static std::vector<Struct> structs;
                static ParseErrorObject parseErrorObject;

                enums.clear();
                structs.clear();
				reparseStructDefinition(input, enums, structs, parseErrorObject);

                if (!parseErrorObject.errorMessage.empty())
                {
                    const char* errorLine = parseErrorObject.errorLine.c_str();
                    float y = ImGui::GetCursorPosY();
					ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", errorLine);
                    ImGui::SetCursorPos(ImVec2(ImGui::CalcTextSize(errorLine, errorLine + parseErrorObject.column - 1).x, y + ImGui::GetTextLineHeight() * 0.55f));
					ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "~~~");
                    ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "(%d, %d) Error: %s", parseErrorObject.line, parseErrorObject.column, parseErrorObject.errorMessage.c_str());
                }
                else 
                {
                    ImGui::TextColored(ImVec4(1, 1, 0.6f, 1), "enums: %d, structs:%d", enums.size(), structs.size());
                }
                
				ImGui::End();
            }
        }
    }

    void EnsureVariableExists(const std::string& name, VariableVisibility visibility, DataFieldType type, std::string dflt)
    {
        g_renderGraphDirty = true;

        int index = 0;
        while (index < g_renderGraph.variables.size() && g_renderGraph.variables[index].name != name)
            index++;

        if (index < g_renderGraph.variables.size())
        {
            Variable& variable = g_renderGraph.variables[index];
            variable.visibility = visibility;
            variable.type = type;
            variable.dflt = dflt;
            return;
        }

        Variable variable;
        variable.name = name;
        variable.visibility = visibility;
        variable.type = type;
        variable.dflt = dflt;
        g_renderGraph.variables.push_back(variable);
    }

	int32_t LevenshteinDistance(const std::string& str1, const std::string& str2)
	{
		int32_t m = (int32_t)str1.length();
		int32_t n = (int32_t)str2.length();
		std::vector<std::vector<int32_t>> dp(m + 1, std::vector<int32_t>(n + 1));

		// Initializing the first row and column as 0
		for (int32_t i = 0; i <= m; i++)
		{
			dp[i][0] = i;
		}
		for (int32_t j = 0; j <= n; j++)
		{
			dp[0][j] = j;
		}

		// Filling in the rest of the dp array
		for (int32_t i = 1; i <= m; i++)
		{
			for (int32_t j = 1; j <= n; j++)
			{
				int32_t insertion = dp[i][j - 1] + 1;
				int32_t deletion = dp[i - 1][j] + 1;
				int32_t match = dp[i - 1][j - 1];
				int32_t mismatch = dp[i - 1][j - 1] + 1;
				if (str1[i - 1] == str2[j - 1])
				{
					dp[i][j] = std::min(std::min(insertion, deletion), match);
				}
				else
				{
					dp[i][j] = std::min(std::min(insertion, deletion), mismatch);
				}
			}
		}
		return dp[m][n];
	}

    size_t StringFindForceLowercase(std::string str, std::string query)
    {
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return (uint8_t)std::tolower((int32_t)c); });
		std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) { return (uint8_t)std::tolower((int32_t)c); });

        return str.find(query);
    }

    void DrawMatchingStringBackground(const std::string& query, const std::string& text)
    {
        const size_t matchOffset = StringFindForceLowercase(text, query);

        if (matchOffset == std::string::npos)
        {
            return;
        }

        const std::string matchPrefix = text.substr(0, matchOffset);
        const std::string match = text.substr(matchOffset, query.size());

		const ImVec2 prefixSize = ImGui::CalcTextSize(matchPrefix.c_str());
		const ImVec2 matchSize = ImGui::CalcTextSize(match.c_str());
        const ImVec2 cursorPos = ImGui::GetCursorPos();
        const ImVec2 windowPos = ImGui::GetWindowPos();
        const float scrollX = ImGui::GetScrollX();
        const float scrollY = ImGui::GetScrollY();

        auto currentWindow = ImGui::GetCurrentWindow();

        const ImVec2 min = { cursorPos.x - scrollX + prefixSize.x, cursorPos.y - scrollY };
        const ImVec2 max = min + matchSize;

        currentWindow->DrawList->AddRectFilled(min + windowPos, max + windowPos, ImColor(100, 100, 100));
    }

    void ShowNodesWindow()
    {
        if (!g_showWindows.Nodes)
            return;

        if (!ImGui::Begin("Nodes", &g_showWindows.Nodes))
        {
            ImGui::End();
            return;
        }

        auto& io = ImGui::GetIO();

        float paneWidth = ImGui::GetContentRegionAvail().x;

        std::vector<ed::NodeId> selectedNodes;
        selectedNodes.resize(ed::GetSelectedObjectCount());
        int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
        selectedNodes.resize(nodeCount);

		static std::string searchQuery;

        // Make a sorted list of nodes
        struct NodeInfo
        {
            std::string name;
            int nodeId = 0;
        };
        std::vector<NodeInfo> sortedNodes;
        {
            int nodeId_ = 0;
            for (const RenderGraphNode& node : g_renderGraph.nodes)
            {
				nodeId_++;

                std::string nodeName = GetNodeName(node);
                if ((!searchQuery.empty() && StringFindForceLowercase(nodeName, searchQuery) == std::string::npos) || node._index == RenderGraphNode::c_index_reroute)
                {
                    continue;
                }

                sortedNodes.push_back({ nodeName, nodeId_ });
            }

            std::sort(sortedNodes.begin(), sortedNodes.end(),
                [](const NodeInfo& a, const NodeInfo& b)
                {
                    return a.name < b.name;
                }
            );

            // Sort by levenshtein distance
            if (!searchQuery.empty())
            {
                std::unordered_map<std::string, int32_t> levenshteinScore;

                for (const auto& node : sortedNodes)
                {
                    levenshteinScore[node.name] = LevenshteinDistance(searchQuery, node.name);
                }

				std::sort(sortedNodes.begin(), sortedNodes.end(),
					[&levenshteinScore](const NodeInfo& a, const NodeInfo& b)
					{
						return levenshteinScore.at(a.name) < levenshteinScore.at(b.name);
					}
				);
            }
        }

		ImGui::GetWindowDrawList()->AddRectFilled(
			ImGui::GetCursorScreenPos(),
			ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
			ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
		ImGui::Spacing(); ImGui::SameLine();
		char buffer[256];
		sprintf_s(buffer, "%i Nodes", (int)sortedNodes.size());
		ImGui::TextUnformatted(buffer);

        // Search bar
        {
            constexpr float searchBarHeight = 32.f;
            ImGui::BeginChild("##nodesSearchBar", { ImGui::GetContentRegionAvail().x, searchBarHeight });
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x);
            ImGui::InputTextWithHint("##searchInputStr", "Search...", &searchQuery);
            ImGui::PopItemWidth();
            ImGui::EndChild();
        }

        // Scrollable window
        {
			const std::string scrollableId = "##nodesScrollable";
            if (ImGui::BeginChild(scrollableId.c_str()))
            {
				ImGui::Indent();

				// show the list of nodes
				for (const NodeInfo& nodeInfo : sortedNodes)
				{
					ImGui::PushID(nodeInfo.nodeId);
					auto start = ImGui::GetCursorScreenPos();

					ax::NodeEditor::NodeId nodeId(nodeInfo.nodeId);

					const std::string& nodeName = nodeInfo.name;

					DrawMatchingStringBackground(searchQuery, nodeName);

					bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), nodeId) != selectedNodes.end();
					if (ImGui::Selectable((nodeName + "##" + std::to_string(nodeInfo.nodeId)).c_str(), &isSelected))
					{
						if (io.KeyCtrl)
						{
							if (isSelected)
								ed::SelectNode(nodeId, true);
							else
								ed::DeselectNode(nodeId);
						}
						else
							ed::SelectNode(nodeId, false);

						ed::NavigateToSelection();
					}

					ImGui::PopID();
				}
				ImGui::Unindent();
            }
            ImGui::EndChild();
        }

        ImGui::End();
    }

    void ShowBuildLogWindow()
    {
        if (!g_showWindows.BuildLog)
            return;

        if (!ImGui::Begin("Build Log", &g_showWindows.BuildLog))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Copy"))
        {
            Example::s_thisExample->m_BuildOutputBuffer.emplace_back(MessageType::Warn, "what's up?");
            Example::s_thisExample->m_BuildOutputBuffer.emplace_back(MessageType::Warn, "bro.");

            constexpr const char* msgType[3] = { "[Info] ", "[Warning] ", "[Error] " };

            std::ostringstream fullText;

            for (const auto& msg : m_BuildOutputBuffer)
            {
                int msgTypeIndex = static_cast<int>(msg.Type);
                fullText << msgType[msgTypeIndex];
                fullText << msg.Msg << "\n";
            }

            SetClipboardDataEx(CF_TEXT, (void*)fullText.str().c_str(), (DWORD)fullText.str().length() + 1);
        }

        ImGui::SameLine();

        if (ImGui::Button("Clear"))
            m_BuildOutputBuffer.clear();

        ImGui::BeginChild("##BuildLogScrollableRegion");
		constexpr const char* msgType[3] = { "", "[Warning] ", "[Error] " };
        const ImVec4 msgColor[3] = { ImGui::GetStyleColorVec4(ImGuiCol_Text), { 0.8f, 0.7f, 0.2f, 1.0f}, { 0.8f, 0.2f, 0.2f, 1.0f } };
        for (const auto& msg : m_BuildOutputBuffer)
        {
            int msgTypeIndex = static_cast<int>(msg.Type);
			ImGui::PushStyleColor(ImGuiCol_Text, msgColor[msgTypeIndex]);
			ImGui::TextWrapped("%s%s", msgType[msgTypeIndex], msg.Msg.c_str());
			ImGui::PopStyleColor();
        }
        ImGui::EndChild();

    // overlaps with the build log in a weird way. It isn't very important so hiding it for now.
#if 0
        if (ImGui::CollapsingHeader("Viewer Log"))
        {
            if (ImGui::Button("Clear"))
                m_previewWindowLog.clear();

            ImGui::Indent();
            ImGui::BeginChild("##BuildLogScrollableRegion");
            constexpr const char* msgType[3] = { "", "[Warning] ", "[Error] " };
            const ImVec4 msgColor[3] = { ImGui::GetStyleColorVec4(ImGuiCol_Text), { 0.8f, 0.7f, 0.2f, 1.0f}, { 0.8f, 0.2f, 0.2f, 1.0f } };
            for (const auto& msg : m_previewWindowLog)
            {
                int msgTypeIndex = static_cast<int>(msg.Type);
                ImGui::PushStyleColor(ImGuiCol_Text, msgColor[msgTypeIndex]);
                ImGui::TextWrapped("%s%s", msgType[msgTypeIndex], msg.Msg.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();
            ImGui::Unindent();
        }
#endif

        ImGui::End();
    }

    void ShowNodePropertiesWindow()
    {
        if (!g_showWindows.NodeProperties)
            return;

        if (!ImGui::Begin("Node Properties", &g_showWindows.NodeProperties))
        {
            ImGui::End();
            return;
        }

        int nodeCount = ed::GetSelectedNodes(nullptr, 0);
        int linkCount = ed::GetSelectedLinks(nullptr, 0);

        if (linkCount == 1)
        {
            ed::LinkId selectedLinkId;
            ed::GetSelectedLinks(&selectedLinkId, 1);

            float paneWidth = ImGui::GetContentRegionAvail().x;

            // Specific link property editing
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetCursorScreenPos(),
                ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
                ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
            ImGui::Spacing(); ImGui::SameLine();

            bool linkValid = false;
            if (m_links.count((int)selectedLinkId.Get()) > 0)
            {
                const LinkInfo& linkInfo = m_links[(int)selectedLinkId.Get()];

                int resourceNodeIndex = GetResourceNodeIndexForPin(g_renderGraph, linkInfo.srcNodeName.c_str(), linkInfo.srcPin.c_str());
                if (resourceNodeIndex != -1)
                {
                    const RenderGraphNode& node = g_renderGraph.nodes[resourceNodeIndex];
                    linkValid = true;
                    ImGui::Text("%s.%s Properties", linkInfo.destNodeName.c_str(), linkInfo.destPin.c_str());
                }
            }

            if (linkValid)
            {
                ImGui::Indent();

                const LinkInfo& linkInfo = m_links[(int)selectedLinkId.Get()];

                int linkedNodeIndex = GetNodeIndexByName(g_renderGraph, linkInfo.destNodeName.c_str());
                if (linkedNodeIndex >= 0)
                {
                    std::vector<NodePinInfo> nodePinInfo = GetNodePins(g_renderGraph, g_renderGraph.nodes[linkedNodeIndex]);

                    int pinIndex = GetNodePinIndexByName(nodePinInfo, linkInfo.destPin.c_str());
                    if (pinIndex >= 0)
                        g_renderGraphDirty |= ShowUI(g_renderGraph, nullptr, nullptr, *nodePinInfo[pinIndex].linkProperties, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes));
                }

                ImGui::Unindent();
            }

        }
        else if (nodeCount == 1)
        {
            ed::NodeId selectedNodeId;
            ed::GetSelectedNodes(&selectedNodeId, 1);

            if (selectedNodeId.Get() >= g_groupNodesStartId)
            {
                ImGui::TextUnformatted("Group Properties");

                const int32_t nodeIndex = int32_t(selectedNodeId.Get()) - g_groupNodesStartId;
                EditorGroupNode& groupNode = g_renderGraph.editorGroupNodes.at(nodeIndex);

				constexpr int32_t strBufferSize = 4096;

                char buf[strBufferSize];
                strcpy_s(buf, strBufferSize, groupNode.name.c_str());

                if (ImGui::InputText("Name", buf, strBufferSize))
                {
                    groupNode.name = buf;
                }

                ImGui::ColorEdit3("Color", groupNode.color.data());
            }
            else
            {
				float paneWidth = ImGui::GetContentRegionAvail().x;

				// Specific node property editing
				ImGui::GetWindowDrawList()->AddRectFilled(
					ImGui::GetCursorScreenPos(),
					ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
					ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
				ImGui::Spacing(); ImGui::SameLine();

				RenderGraphNode& node = g_renderGraph.nodes[selectedNodeId.Get() - 1];

				std::string title = GetNodeTypeName(node);
				title += " Properties";

                char buffer[256];
                sprintf_s(buffer, " (Node %i)", int(selectedNodeId.Get()) - 1);
                title += buffer;

				std::string oldNodeName = GetNodeName(node);

                switch (node._index)
                {
                    case RenderGraphNode::c_index_actionComputeShader:
                    {
                        RenderGraphNode_Action_ComputeShader& nodeComputeShader = node.actionComputeShader;
                        int shaderIndex = GetShaderIndexByName(g_renderGraph, ShaderType::Compute, nodeComputeShader.shader.name.c_str());
                        if (shaderIndex != -1)
                        {
                            const Shader& shader = g_renderGraph.shaders[shaderIndex];
                            nodeComputeShader.dispatchSize.shaderNumThreads = shader.NumThreads;
                        }
                        break;
                    }
                }

				ImGui::TextUnformatted(title.c_str());
				ImGui::Indent();
				g_renderGraphDirty |= ShowUI(g_renderGraph, nullptr, nullptr, node, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes));

				std::string newNodeName = GetNodeName(node);

				if (oldNodeName != newNodeName)
				{
					// Force the new name to be unique or else updating node connections breaks connections. Workaround til we have a better fix.
					// We should be linking together by index probably, instead of by name.
					while (1)
					{
						int nodeNameCount = 0;
						for (const RenderGraphNode& node : g_renderGraph.nodes)
						{
							if (GetNodeName(node) == newNodeName)
								nodeNameCount++;
						}

						if (nodeNameCount == 1)
							break;

						newNodeName += " ";
						SetNodeName(node, newNodeName);
					}

					SetNodeName(node, oldNodeName);
					OnNodeRename(oldNodeName, newNodeName);
					SetNodeName(node, newNodeName);
				}

				ImGui::Unindent();
            }
        }

        ImGui::End();
    }

    template <typename T, typename = int>
    struct HasName : std::false_type { };

    template <typename T>
    struct HasName <T, decltype((void)T::name, 0)> : std::true_type { };

    template <typename T>
    void CopyDataItemButNotName(T& dest, const T& src)
    {
        std::string name;
        if constexpr (HasName<T>())
            name = dest.name;
        dest = src;
        if constexpr (HasName<T>())
            dest.name = name;
    }

    template <typename T, typename LAMBDA>
    void ShowDataWindow(const char* dataName, std::vector<T>& dataList, DataWindowState& windowState, TypePathEntry path, T& newItem, const LAMBDA& NameLambda)
    {
        bool& showWindow = windowState.show;

        if (!showWindow)
            return;

        if (!ImGui::Begin(dataName, &showWindow, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGui::End();
            return;
        }

        size_t& selectedIndex = windowState.selectedIndex;
        if (selectedIndex >= dataList.size())
            selectedIndex = 0;

        ImGui::BeginTable("Table", 2);

        ImGui::TableSetupColumn("0", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text(dataName);

        // New button
        bool wantsNew = false;
        if (ImGui::SmallButton("New"))
            wantsNew = true;

        ImGui::SameLine();
        size_t duplicateIndex = ~0;
        if (ImGui::SmallButton("Duplicate"))
            duplicateIndex = selectedIndex;

        // Delete button, but delete after we are done with the items
        ImGui::SameLine();
        size_t deleteIndex = ~0;
        if (ImGui::SmallButton("Delete"))
            deleteIndex = selectedIndex;

        // up, down, top, bottom
        bool wantsMoveUp = false;
        bool wantsMoveDown = false;
        bool wantsMoveTop = false;
        bool wantsMoveBottom = false;
        {
            ImGui::SameLine();
			ImGui_Enabled enabled(selectedIndex != 0);
			if (ArrowButton2("Up", ImGuiDir_Up, true, false))
                wantsMoveUp = true;
			ShowUIToolTip("Up");
        }
        {
            ImGui::SameLine();
			ImGui_Enabled enabled(selectedIndex + 1 < dataList.size());
			if (ArrowButton2("Down", ImGuiDir_Down, true, false))
                wantsMoveDown = true;
			ShowUIToolTip("Down");
        }
		{
            ImGui::SameLine();
			ImGui_Enabled enabled(selectedIndex != 0);
			if (ArrowButton2("Up to Top", ImGuiDir_Up, true, true))
                wantsMoveTop = true;
			ShowUIToolTip("Up To Top");
        }
		{
            ImGui::SameLine();
			ImGui_Enabled enabled(selectedIndex + 1 < dataList.size());
			if (ArrowButton2("Down to Bottom", ImGuiDir_Down, true, true))
                wantsMoveBottom = true;
			ShowUIToolTip("Down to Bottom");
        }

        // Search bar
        {
			constexpr float searchBarHeight = 32.f;
            std::string childId = "##" + std::string(dataName) + "SearchBar";
            std::string inputId = "##" + std::string(dataName) + "searchInputStr";

			ImGui::BeginChild(childId.c_str(), {ImGui::GetContentRegionAvail().x, searchBarHeight});
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x);
			ImGui::InputTextWithHint(inputId.c_str(), "Search...", &windowState.searchQuery);
			ImGui::PopItemWidth();
			ImGui::EndChild();
        }

		struct DataInfo
		{
			std::string name;
			size_t index;
		};

		std::vector<DataInfo> filteredData;
        filteredData.reserve(dataList.size());

        for (size_t index = 0; index < dataList.size(); index++)
        {
            std::string dataName = NameLambda(index, dataList[index]);
            if (dataName.empty())
            {
                dataName = "<empty>";
            }

            if (!windowState.searchQuery.empty() && StringFindForceLowercase(dataName, windowState.searchQuery) == std::string::npos)
            {
                continue;
            }

            filteredData.push_back({ dataName, index });
        }

        // Sort by levenshtein distance
        if (!windowState.searchQuery.empty())
        {
			std::unordered_map<std::string, int32_t> levenshteinScore;

			for (const auto& data : filteredData)
			{
				levenshteinScore[data.name] = LevenshteinDistance(windowState.searchQuery, data.name);
			}

			std::sort(filteredData.begin(), filteredData.end(),
				[&levenshteinScore](const DataInfo& a, const DataInfo& b)
				{
					return levenshteinScore.at(a.name) < levenshteinScore.at(b.name);
				}
			);
        }

        // show the list of items and let them be selectable
        if (ImGui::BeginListBox("##items", ImVec2(300.0f, -FLT_MIN)))
        {
            for (size_t index = 0; index < filteredData.size(); ++index)
            {
                auto& data = filteredData.at(index);

                ImGui::PushID(data.name.c_str());

                DrawMatchingStringBackground(windowState.searchQuery, data.name);

                bool isSelected = data.index == selectedIndex;
                if (ImGui::Selectable(data.name.c_str(), &isSelected))
                    selectedIndex = data.index;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();

                ShowUIToolTip(data.name.c_str());

                ImGui::PopID();
            }

            ImGui::EndListBox();
        }

        ImGui::TableNextColumn();

        const std::string scrollableId = std::string(dataName) + "##scrollable";
        if (ImGui::BeginChild(scrollableId.c_str()))
        {
			if (selectedIndex < dataList.size())
				g_renderGraphDirty |= ShowUI(g_renderGraph, nullptr, nullptr, dataList[selectedIndex], path);

			// delete an item if we should
			if (deleteIndex < dataList.size())
			{
				dataList.erase(dataList.begin() + deleteIndex);
				g_renderGraphDirty = true;
				selectedIndex = std::min(selectedIndex, dataList.size() - 1);
			}

			// duplicate an item if we should
			if (duplicateIndex < dataList.size())
			{
				CopyDataItemButNotName(newItem, dataList[duplicateIndex]);
				dataList.push_back(newItem);
				g_renderGraphDirty = true;
				selectedIndex = dataList.size() - 1;
			}

			// create an item if we should
			if (wantsNew)
			{
				dataList.push_back(newItem);
				g_renderGraphDirty = true;
				selectedIndex = dataList.size() - 1;
			}

			// Move Up
			if (wantsMoveUp)
			{
				std::swap(dataList[selectedIndex - 1], dataList[selectedIndex]);
				g_renderGraphDirty = true;
				selectedIndex = selectedIndex - 1;
			}

			// Move Down
			if (wantsMoveDown)
			{
				std::swap(dataList[selectedIndex + 1], dataList[selectedIndex]);
				g_renderGraphDirty = true;
				selectedIndex = selectedIndex + 1;
			}

			// Move Top
			if (wantsMoveTop)
			{
				for (size_t i = selectedIndex; i > 0; i--)
					std::swap(dataList[i - 1], dataList[i]);
				g_renderGraphDirty = true;
				selectedIndex = 0;
			}

			// Move Bottom
			if (wantsMoveBottom)
			{
				for (size_t i = selectedIndex; (i + 1) < dataList.size(); ++i)
					std::swap(dataList[i], dataList[i + 1]);
				g_renderGraphDirty = true;
				selectedIndex = dataList.size() - 1;
			}
        }
        ImGui::EndChild();

        ImGui::EndTable();

        ImGui::End();
    }

    static inline std::string IndexToString(size_t index)
    {
        char buffer[64];
        sprintf_s(buffer, "%zi", index);
        return std::string(buffer);
    }

    void ShowVariablesWindow()
    {
        Variable newItem;
        newItem.name = "NewVariable";
        ShowDataWindow("Variables", g_renderGraph.variables, g_showWindows.Variables, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_variables), newItem, [](size_t index, const Variable& item) { return item.name; });
    }

    void ShowShadersWindow()
    {
        Shader newItem;
        newItem.name = "NewShader";
        ShowDataWindow("Shaders", g_renderGraph.shaders, g_showWindows.Shaders, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_shaders), newItem, [](size_t index, const Shader& item) { return item.name; });
    }

    void ShowStructsWindow()
    {
        Struct newItem;
        newItem.name = "NewStruct";
        ShowDataWindow("Structs", g_renderGraph.structs, g_showWindows.Structs, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_structs), newItem, [](size_t index, const Struct& item) { return item.name; });
    }

    void ShowFileCopiesWindow()
    {
        FileCopy newItem;
        ShowDataWindow("FileCopies", g_renderGraph.fileCopies, g_showWindows.FileCopies, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_fileCopies), newItem, [](size_t index, const FileCopy& item) { return item.fileName; });
    }

    void ShowEnumsWindow()
    {
        Enum newItem;
        newItem.name = "NewEnum";
        ShowDataWindow("Enums", g_renderGraph.enums, g_showWindows.Enums, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_enums), newItem, [](size_t index, const Enum& item) { return item.name; });
    }

    void ShowSetVariablesWindow()
    {
        auto GetRHS = [](const std::string& literal, const TextureOrBufferNodeReference& node, const VariableReference& variable, int index)
        {
            std::stringstream ret;

            if (!node.name.empty())
                ret << node.name;
            else if (!variable.name.empty())
                ret << variable.name;
            else
                ret << "(" << literal << ")";

            if (index != -1)
                ret << "[" << index << "]";

            return ret.str();
        };

        auto NameLambda = [&](size_t index, const SetVariable& item)
        {
            std::stringstream name;
            name << index << ") " << item.destination.name;
            if (item.destinationIndex != -1)
                name << "[" << item.destinationIndex << "]";
            name << "=";

            switch (item.op)
            {
                case SetVariableOperator::Add:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "+";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::Subtract:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "-";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::Multiply:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "*";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::Divide:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "/";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::Modulo:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "%";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::PowerOf2GE:
                {
                    name << "PowerOf2GE(";
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << ")";
                    break;
                }
                case SetVariableOperator::Minimum:
                {
                    name << "Min(";
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << ",";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    name << ")";
                    break;
                }
                case SetVariableOperator::Maximum:
                {
                    name << "Max(";
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << ",";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    name << ")";
                    break;
                }
                case SetVariableOperator::BitwiseOr:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "|";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::BitwiseAnd:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "&";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::BitwiseXor:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    name << "^";
                    name << GetRHS(item.BLiteral, item.BNode, item.BVar, item.BVarIndex);
                    break;
                }
                case SetVariableOperator::BitwiseNot:
                {
                    name << "!";
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    break;
                }
                case SetVariableOperator::Noop:
                {
                    name << GetRHS(item.ALiteral, item.ANode, item.AVar, item.AVarIndex);
                    break;
                }
            }

            return name.str();
        };

        SetVariable newItem;
        ShowDataWindow("SetVariables", g_renderGraph.setVars, g_showWindows.SetVariables, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_setVars), newItem, NameLambda);
    }

    void ShowRTHitGroupsWindow()
    {
        RTHitGroup newItem;
        newItem.name = "NewRTHitGroup";
        ShowDataWindow("RTHitGroups", g_renderGraph.hitGroups, g_showWindows.RTHitGroups, TypePaths::Make(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_hitGroups), newItem, [](size_t index, const RTHitGroup& item) { return item.name; });
    }

    void ShowGraphPropertiesWindow()
    {
        if (!g_showWindows.GraphProperties)
            return;

        if (!ImGui::Begin("Graph Properties", &g_showWindows.GraphProperties))
        {
            ImGui::End();
            return;
        }

        bool keypressesAllowed = !ImGui::GetIO().WantTextInput;
        if (ImGui::Button("Zoom to Content (F)") || (keypressesAllowed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F))))
            ed::NavigateToContent();

        ImGui::SameLine();
        if (ImGui::Button("Add System Vars"))
        {
            EnsureVariableExists("MouseState", VariableVisibility::Host, DataFieldType::Float4, "0.0f, 0.0f, 0.0f, 0.0f");
            EnsureVariableExists("MouseStateLastFrame", VariableVisibility::Host, DataFieldType::Float4, "0.0f, 0.0f, 0.0f, 0.0f");
            EnsureVariableExists("iResolution", VariableVisibility::Host, DataFieldType::Float3, "0.0f, 0.0f, 0.0f");
            EnsureVariableExists("iTime", VariableVisibility::Host, DataFieldType::Float, "0.0f");
            EnsureVariableExists("iTimeDelta", VariableVisibility::Host, DataFieldType::Float, "0.0f");
            EnsureVariableExists("iFrameRate", VariableVisibility::Host, DataFieldType::Float, "0.0f");
            EnsureVariableExists("iFrame", VariableVisibility::Host, DataFieldType::Int, "0");
            EnsureVariableExists("iMouse", VariableVisibility::Host, DataFieldType::Float4, "0.0f, 0.0f, 0.0f, 0.0f");
            EnsureVariableExists("ViewMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("InvViewMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("ProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("InvProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("JitteredProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("InvJitteredProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("ViewProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("InvViewProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("JitteredViewProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("InvJitteredViewProjMtx", VariableVisibility::Host, DataFieldType::Float4x4, "1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f");
            EnsureVariableExists("CameraPos", VariableVisibility::Host, DataFieldType::Float3, "0.0f, 0.0f, 0.0f");
            EnsureVariableExists("CameraAltitudeAzimuth", VariableVisibility::Host, DataFieldType::Float2, "0.0f, 0.0f");
            EnsureVariableExists("CameraChanged", VariableVisibility::Host, DataFieldType::Bool, "false");
            EnsureVariableExists("CameraJitter", VariableVisibility::Host, DataFieldType::Float2, "0.5f, 0.5f");
            EnsureVariableExists("CameraNearPlane", VariableVisibility::Host, DataFieldType::Float, "0.0f");
            EnsureVariableExists("CameraFarPlane", VariableVisibility::Host, DataFieldType::Float, "0.0f");
            EnsureVariableExists("ShadingRateImageTileSize", VariableVisibility::Host, DataFieldType::Uint, "16");
            EnsureVariableExists("WindowSize", VariableVisibility::Host, DataFieldType::Float2, "1.0f, 1.0f");
        }

        // Global property editing
        g_renderGraphDirty |= ShowUI(g_renderGraph, nullptr, nullptr, g_renderGraph, TypePaths::cEmpty);

        // todo: move
        if (g_renderGraphDirty)
        {
            reparseStructDefinitions();
        }

        ImGui::End();
    }

    void reparseStructDefinitions()
    {
        for (Struct& cb : g_renderGraph.structs)
        {
            std::vector<Struct> structs;
            ParseErrorObject parseErrorObject;
            if (!reparseStructDefinition(cb.definition.c_str(), g_renderGraph.enums, structs, parseErrorObject))
            {
                OutputDebugStringA(parseErrorObject.errorMessage.c_str());
                OutputDebugStringA("\n");
            }
            // at the moment we only support one struct in per definition
            if (structs.size() == 1)
                cb = structs.front();
        }
    }

    void HandleDoubleClick(int linkId)
    {
		LinkInfo& linkInfo = m_links[linkId];

        // Create reroute node
		char newNodeName[64];
		int nextNodeIndex = 0;
		while (true)
		{
			nextNodeIndex++;
			sprintf_s(newNodeName, "Node %i", nextNodeIndex);
			bool nameExists = false;
			for (const RenderGraphNode& node : g_renderGraph.nodes)
			{
				if (GetNodeName(node) == newNodeName)
				{
					nameExists = true;
					break;
				}
			}
				if (!nameExists)
					break;
		}
        RenderGraphNode newNode;
        newNode._index = RenderGraphNode::c_index_reroute;
        newNode.reroute.name = newNodeName;
        g_renderGraph.nodes.push_back(newNode);
        m_newNodePositions[(int)g_renderGraph.nodes.size()] = ed::ScreenToCanvas(ImGui::GetMousePos());
        g_renderGraphDirty = true;
        g_createdNodeIndex = (int)g_renderGraph.nodes.size();

        // Make sure reroute pin exists
        {
			g_renderGraph.nodes.back().reroute.connections.resize(1);
			g_renderGraph.nodes.back().reroute.connections[0].srcPin = "Pin";
        }

        int destNodeIndex = GetNodeIndexByName(g_renderGraph, linkInfo.destNodeName.c_str());

		if (destNodeIndex >= 0)
		{
			std::vector<NodePinInfo> nodePinInfo = GetNodePins(g_renderGraph, g_renderGraph.nodes[destNodeIndex]);

			int destPinIndex = GetNodePinIndexByName(nodePinInfo, linkInfo.destPin.c_str());
			if (destPinIndex >= 0)
			{
				*nodePinInfo[destPinIndex].inputNode = newNodeName;
				*nodePinInfo[destPinIndex].inputNodePin = g_renderGraph.nodes.back().reroute.connections.back().srcPin;
			}
		}

        std::vector<NodePinInfo> nodePinInfo = GetNodePins(g_renderGraph, g_renderGraph.nodes.back());
        *nodePinInfo[0].inputNode = linkInfo.srcNodeName;
        *nodePinInfo[0].inputNodePin = linkInfo.srcPin;
    }

    void HandleDoubleClick(const RenderGraphNode& nodeBase)
    {
        std::filesystem::path defaultPath = std::filesystem::path(g_renderGraph.editorFileName).remove_filename();

        switch (nodeBase._index)
        {
            case RenderGraphNode::c_index_actionSubGraph:
            {
                const RenderGraphNode_Action_SubGraph& node = nodeBase.actionSubGraph;

                std::filesystem::path fullFileName = (defaultPath / std::filesystem::path(node.fileName));

                // Launch the process
                char commandLine[1024];
                sprintf_s(commandLine, "\"GigiEdit.exe\" \"%s\"", fullFileName.string().c_str());
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
                break;
            }
            case RenderGraphNode::c_index_actionComputeShader:
            {
                const RenderGraphNode_Action_ComputeShader& node = nodeBase.actionComputeShader;

                for (const Shader& shader : g_renderGraph.shaders)
                {
                    if (shader.name == node.shader.name)
                    {
                        std::filesystem::path fullFileName = (defaultPath / std::filesystem::path(shader.fileName));
                        ShellExecuteA(NULL, "open", fullFileName.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
                        return;
                    }
                }
                break;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                const RenderGraphNode_Action_RayShader& node = nodeBase.actionRayShader;

                for (const Shader& shader : g_renderGraph.shaders)
                {
                    if (shader.name == node.shader.name)
                    {
                        std::filesystem::path fullFileName = (defaultPath / std::filesystem::path(shader.fileName));
                        ShellExecuteA(NULL, "open", fullFileName.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
                        return;
                    }
                }
                break;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                const RenderGraphNode_Action_DrawCall& node = nodeBase.actionDrawCall;

                for (const Shader& shader : g_renderGraph.shaders)
                {
                    if (shader.name == node.vertexShader.name || shader.name == node.pixelShader.name || shader.name == node.amplificationShader.name || shader.name == node.meshShader.name)
                    {
                        std::filesystem::path fullFileName = (defaultPath / std::filesystem::path(shader.fileName));
                        ShellExecuteA(NULL, "open", fullFileName.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
                    }
                }
                break;
            }
        }
    }

    void OnClientMessage(const PreviewMsg_Ping& msg)
    {

    }

    void OnClientMessage(const PreviewMsgCS_Version& msg)
    {

    }

    void OnClientMessage(const PreviewMsgCS_Log& msg)
    {
        m_previewWindowLog.emplace_back((MessageType)msg.level, msg.msg);
    }


    void DockBuilderSplit3Ways(ImGuiID node_id, ImGuiDir split_dir, float percent1, float percent2, ImGuiID& a, ImGuiID& b, ImGuiID& c)
    {
        float ratio1 = percent1;
        float ratio2 = percent2 / (1.0f - percent1);

        ImGuiID temp;
        ImGui::DockBuilderSplitNode(node_id, split_dir, ratio1, &a, &temp);
        ImGui::DockBuilderSplitNode(temp, split_dir, ratio2, &b, &c);
    }

    void MakeInitialLayout(ImGuiID dockspace_id)
    {
        if (!g_resetLayout)
            return;
        g_resetLayout = false;

        // The main docking space
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);


        ImGuiID dockspaceLeft, dockspaceCenter, dockspaceRight;
        DockBuilderSplit3Ways(dockspace_id, ImGuiDir_Left, 0.2f, 0.6f, dockspaceLeft, dockspaceCenter, dockspaceRight);

        ImGuiID dockspaceCenterBottom, dockspaceCenterTop;
        ImGui::DockBuilderSplitNode(dockspaceCenter, ImGuiDir_Down, 0.3f, &dockspaceCenterBottom, &dockspaceCenterTop);

        ImGuiID dockspaceLeftBottom, dockspaceLeftTop;
        ImGui::DockBuilderSplitNode(dockspaceLeft, ImGuiDir_Down, 0.5f, &dockspaceLeftBottom, &dockspaceLeftTop);


        ImGui::DockBuilderDockWindow("Graph Properties", dockspaceLeftTop);
        ImGui::DockBuilderDockWindow("Nodes", dockspaceLeftBottom);

        ImGui::DockBuilderDockWindow("Graph", dockspaceCenterTop);
        ImGui::DockBuilderDockWindow("Build Log", dockspaceCenterBottom);

        ImGui::DockBuilderDockWindow("Node Properties", dockspaceRight);
        ImGui::DockBuilderDockWindow("Variables", dockspaceCenterBottom);
        ImGui::DockBuilderDockWindow("Shaders", dockspaceCenterBottom);

        ImGui::DockBuilderDockWindow("Structs", dockspaceCenterBottom);
        ImGui::DockBuilderDockWindow("FileCopies", dockspaceCenterBottom);
        ImGui::DockBuilderDockWindow("Enums", dockspaceCenterBottom);
        ImGui::DockBuilderDockWindow("SetVariables", dockspaceCenterBottom);
        ImGui::DockBuilderDockWindow("RTHitGroups", dockspaceCenterBottom);

        ImGui::DockBuilderGetNode(dockspaceLeftBottom)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
        ImGui::DockBuilderGetNode(dockspaceLeftTop)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
        ImGui::DockBuilderGetNode(dockspaceRight)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
        ImGui::DockBuilderGetNode(dockspaceCenterTop)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
        ImGui::DockBuilderGetNode(dockspaceCenterBottom)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;

        ImGui::DockBuilderFinish(dockspace_id);
    }

    void DeleteGroupNode(int deleteIndex)
    {
        // Update all nodes with new indices
        std::vector<EditorGroupNode> tempGroupNodes = g_renderGraph.editorGroupNodes;

        int32_t realIndex = deleteIndex - g_groupNodesStartId;
        g_renderGraph.editorGroupNodes.erase(g_renderGraph.editorGroupNodes.begin() + realIndex);

        for (int32_t index = realIndex; index < g_renderGraph.editorGroupNodes.size(); ++index)
        {
            int32_t newId = g_groupNodesStartId + index;

            EditorGroupNode& currentNode = g_renderGraph.editorGroupNodes[index];

            currentNode = tempGroupNodes[index + 1];
            currentNode.id = newId;

            m_newNodePositions[newId] = ImVec2(currentNode.position[0], currentNode.position[1]);
            m_newGroupSizes[newId] = ImVec2(currentNode.size[0], currentNode.size[1]);

            ed::SetNodePosition(newId, m_newNodePositions[newId]);
            ed::SetGroupSize(newId, m_newGroupSizes[newId]);
        }
    }

    void DeleteNode(int deleteIndex)
    {
        std::string deleteName = GetNodeName(g_renderGraph.nodes[deleteIndex]);

        // Clear any pin connection that refers to this node
        for (RenderGraphNode& node : g_renderGraph.nodes)
        {
            std::vector<NodePinInfo> nodePinInfo = GetNodePins(g_renderGraph, node);
            for (NodePinInfo& info : nodePinInfo)
            {
                if (!info.isInput)
                    continue;

                if (*info.inputNode == deleteName)
                {
                    *info.inputNode = "";
                    *info.inputNodePin = "";
                }
            }
        }

        // delete the node
        g_renderGraph.nodes.erase(g_renderGraph.nodes.begin() + deleteIndex);

        // make sure nodes keep their location, after changing node id
        for (int index = deleteIndex; index < g_renderGraph.nodes.size(); ++index)
        {
            ImVec2 pos = ed::GetNodePosition(index + 2);
            m_newNodePositions[index + 1] = pos;
            ed::SetNodePosition(index + 1, pos);
        }
    }

    void DeleteLink(int linkId)
    {
        const LinkInfo& linkInfo = m_links[linkId];
        int destNodeIndex = GetNodeIndexByName(g_renderGraph, linkInfo.destNodeName.c_str());
        if (destNodeIndex >= 0)
        {
            std::vector<NodePinInfo> nodePinInfo = GetNodePins(g_renderGraph, g_renderGraph.nodes[destNodeIndex]);

            int destPinIndex = GetNodePinIndexByName(nodePinInfo, linkInfo.destPin.c_str());
            if (destPinIndex >= 0)
            {
                *nodePinInfo[destPinIndex].inputNode = "";
                *nodePinInfo[destPinIndex].inputNodePin = "";
            }
        }
    }

    ax::NodeEditor::Utilities::NodeStyle GetNodeStyle(const RenderGraphNode& node)
    {
        auto GetColorFromArray = [](const std::array<float, 3>& arr, int32_t alpha) -> ImColor
        {
            return ImColor(arr[0], arr[1], arr[2], float(alpha) / 255.f);
        };

		// Default colors are from Frostbite FrameGraph
        // slide 17 https://www.slideshare.net/DICEStudio/framegraph-extensible-rendering-architecture-in-frostbite

        ax::NodeEditor::Utilities::NodeStyle result;
        result.rounding = 12.f;
        result.color = GetColorFromArray(g_renderGraph.styleSettings.actionNodeColor, 128);

        if (node._index == RenderGraphNode::c_index_reroute)
        {
            result.color = ImColor(1.f, 1.f, 1.f, 0.78f);
        }

        if (GetNodeIsResourceNode(node))
        {
            result.rounding = 0.f;

            bool isTransient = false;

            // Use a different color for non transient resources.
            if (node._index == RenderGraphNode::c_index_resourceBuffer)
            {
                isTransient = node.resourceBuffer.transient;
            }
            else if (node._index == RenderGraphNode::c_index_resourceTexture)
            {
                isTransient = node.resourceTexture.transient;
            }

            result.color = isTransient ? GetColorFromArray(g_renderGraph.styleSettings.resourceNodeColor, 128) : GetColorFromArray(g_renderGraph.styleSettings.nonTransientResourceNodeColor, 128);
        }

        return result;
    }

    void OnFrame(float deltaTime) override
    {
        //ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(nullptr);

        ImGuiID dockspace_id = ImGui::DockSpace(ImGui::GetID("MyDockSpace"));

        MakeInitialLayout(dockspace_id);

        {
            bool wasFullyConnected = g_previewWindowServer.IsFullyConnected();
            if (!g_previewWindowServer.Tick())
                m_previewWindowLog.emplace_back(MessageType::Info, "PreviewServer Shutdown");
            bool isFullyConnected = g_previewWindowServer.IsFullyConnected();
            if (!wasFullyConnected && isFullyConnected)
            {
                m_previewWindowLog.emplace_back(MessageType::Info, std::string("Client connected. Version ") + g_previewWindowServer.GetClientVersion() + "\n");
                OnChangeGGFileName(false);
            }

            // process any messages from the client
            PreviewMsgClientToServer msg;
            while (g_previewWindowServer.PopMessage(msg))
            {
                switch(msg._index)
                {
                    // clang-format off
                    #include "external/df_serialize/_common.h"
                    #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) case PreviewMsgClientToServer::c_index_##_NAME: OnClientMessage(msg.##_NAME); break;
                    #include "external/df_serialize/_fillunsetdefines.h"
                    #include "Schemas/PreviewWindow/PreviewMsgClientToServerVariant.h"
                    // clang-format on
                }
            }
        }

        UpdateWindowTitle();

        ed::SetCurrentEditor(m_Context);
        // theme
		ed::GetStyle().Colors[ed::StyleColor_HovNodeBorder] = ImColor(255, 255, 255, 64);
		ed::GetStyle().Colors[ed::StyleColor_NodeSelRect] = ImColor(128, 128, 128, 64);
		ed::GetStyle().Colors[ed::StyleColor_NodeSelRectBorder] = ImColor(128, 128, 128, 64);

        // handle node double click
        {
            auto nodeId = ed::GetDoubleClickedNode();
            if (nodeId && nodeId.Get() < g_groupNodesStartId)
                HandleDoubleClick(g_renderGraph.nodes[nodeId.Get() - 1]);
        }

        // handle link double click
        {
            auto linkId = ed::GetDoubleClickedLink();
            if (linkId)
            {
                HandleDoubleClick((int)linkId.Get());
            }
        }


        // load file specified on command line, if given
        {
            static bool firstFrame = true;
            if (firstFrame && g_argc > 1)
                LoadJSONFile(g_argv[1]);
            firstFrame = false;
        }

        // Shortcuts
		auto& io = ImGui::GetIO();
        if (io.KeyCtrl)
        {
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_B)))
			{
				BuildGraph();
		    }
            else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_N)))
            {
                CreateGraph();
            }
			else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O)))
			{
				OpenGraph();
			}
			else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
			{
				SaveGraph((io.KeyShift || g_renderGraphFileName.empty()) ? nullptr : g_renderGraphFileName.c_str());
			}
        }

        // Drag and drop file loading
        {
            std::string dragFileName = GetAndClearDragFile();
            if (!dragFileName.empty())
            {
                if (!g_renderGraphDirty || AskForConfirmation("You have unsaved changes, are you sure you want to proceed?"))
                    LoadJSONFile(dragFileName.c_str());
            }
        }

        HandleMainMenu();

        ShowShadersWindow();
        ShowVariablesWindow();
        ShowStructsWindow();
        ShowFileCopiesWindow();
        ShowEnumsWindow();
        ShowSetVariablesWindow();
        ShowRTHitGroupsWindow();

        ShowBuildLogWindow();
        ShowGraphPropertiesWindow();
        ShowNodePropertiesWindow();
        ShowNodesWindow();

        if (g_createdNodeIndex != -1 && ed::NodeExists(g_createdNodeIndex))
        {
            ed::SelectNode(g_createdNodeIndex);
            g_createdNodeIndex = -1;
        }

        ImGui::Begin("Graph");

        ed::Begin("Graph", ImVec2(0.0, 0.0f));

		auto alpha = ImGui::GetStyle().Alpha;

		ax::NodeEditor::Utilities::BlueprintNodeBuilder builder(nullptr, 1, 1);

        // Draw the nodes
        m_pins.clear();
        int uniquePinId = 1;
        int nodeId = 1;
        int uniqueId = (int)g_renderGraph.nodes.size() + 1;
        for (RenderGraphNode& node : g_renderGraph.nodes)
        {
            int nodeEditorId = nodeId++;

            bool nodeDisabled = false;
            DispatchLambdaAction(node,
                [&nodeDisabled](RenderGraphNode_ActionBase& node)
                {
                    nodeDisabled = node.condition.alwaysFalse;
                }
            );

            builder.Begin(nodeEditorId, GetNodeStyle(node), nodeDisabled);

            if (g_renderGraphFirstFrame)
            {
                std::array<float, 2> editorPos = GetNodeEditorPos(node);
                ed::SetNodePosition(nodeEditorId, ImVec2(editorPos[0], editorPos[1]));
            }

			std::string nodeHeader = GetNodeShorterTypeName(node) + ": " + GetNodeName(node);

            if (node._index != RenderGraphNode::c_index_reroute)
            {
				ImGui::Text(nodeHeader.c_str());
            }

            std::vector<NodePinInfo> pinInfo = GetNodePins(g_renderGraph, node);

            // Calculate the maximum width of each pin, and the nodeHeader
            float maxRowSize = 0.f;

            if (node._index != RenderGraphNode::c_index_reroute)
            {
                maxRowSize = ImGui::CalcTextSize(nodeHeader.c_str()).x;
				for (NodePinInfo& info : pinInfo)
					maxRowSize = std::max(maxRowSize, 48.0f + ImGui::CalcTextSize((info.name + info.accessLabel).c_str()).x);
            }
            else
            {
                maxRowSize = 50.f;
            }

            for (NodePinInfo& info : pinInfo)
            {
                float nodeStartX = ImGui::GetCursorPosX();

                uniquePinId++;

				int pinEditorId = uniqueId++;

                if (info.isInput)
                {
                    ed::BeginPin(pinEditorId, ed::PinKind::Input);
    				// 0.5, 0.5 to get curve coming out of the pin middle
                    ed::PinPivotAlignment(ImVec2(0.5f, 0.5f));
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

                    // left side
					DrawPinIcon(info, node._index == RenderGraphNode::c_index_reroute, false /*IsPinLinked(info.ID)*/, (int)(alpha * 255));
					ImGui::PopStyleVar();


                    ed::EndPin();

                    *info.inputEditorId = pinEditorId;

                    EditorPinInfo pinInfo;
                    pinInfo.isInputPin = true;
                    pinInfo.node = GetNodeName(g_renderGraph.nodes[nodeEditorId - 1]);
                    pinInfo.pin = info.name;
                    m_pins[pinEditorId] = pinInfo;

                    ImGui::SameLine();
                }

                if (node._index != RenderGraphNode::c_index_reroute)
                {
					ImGui::Text((info.name + info.accessLabel).c_str());
                }

                ImGui::SameLine();
                float nodeAtX = ImGui::GetCursorPosX();
                ImGui::Dummy(ImVec2{ maxRowSize - (nodeAtX - nodeStartX) - 32.0f, 1 });
                ImGui::SameLine();

                pinEditorId = uniqueId++;
                ed::BeginPin(pinEditorId, ed::PinKind::Output);

                // 0.5, 0.5 to get curve coming out of the pin middle
				ed::PinPivotAlignment(ImVec2(0.5f, 0.5f));
                
				ed::PinPivotSize(ImVec2(0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

                // right side

				DrawPinIcon(info, node._index == RenderGraphNode::c_index_reroute, false/*IsPinLinked(output.ID)*/, (int)(alpha * 255));
				ImGui::PopStyleVar();

                ed::EndPin();

                *info.outputEditorId = pinEditorId;

                EditorPinInfo pinInfo;
                pinInfo.isInputPin = false;
                pinInfo.node = GetNodeName(g_renderGraph.nodes[nodeEditorId - 1]);
                pinInfo.pin = info.name;
                m_pins[pinEditorId] = pinInfo;
            }

			builder.End();
        }

		// Set group node sizes
		{
			for (const auto& info : m_newGroupSizes)
			{
				ed::SetGroupSize(info.first, info.second);
			}

			m_newGroupSizes.clear();
		}

        // Draw groups
        int32_t groupId = 0;
        for (auto& groupNode : g_renderGraph.editorGroupNodes)
        {
            if (g_renderGraphFirstFrame)
            {
                groupNode.id = g_groupNodesStartId + groupId++;

                ed::SetNodePosition(groupNode.id, ImVec2(groupNode.position[0], groupNode.position[1]));
                ed::SetGroupSize(groupNode.id, ImVec2(groupNode.size[0], groupNode.size[1]));
            }

			const float commentAlpha = 0.75f;

            ImColor groupColor = ImColor(groupNode.color[0], groupNode.color[1], groupNode.color[2], groupNode.color[3]);

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
			ed::PushStyleColor(ed::StyleColor_NodeBg, groupColor);
			ed::PushStyleColor(ed::StyleColor_NodeBorder, groupColor);
			ed::BeginNode(groupNode.id);
			ImGui::PushID(groupNode.id);
			ImGui::TextUnformatted(groupNode.name.c_str());
			ed::Group(ImVec2(groupNode.size[0], groupNode.size[1]));
			ImGui::PopID();
			ed::EndNode();
			ed::PopStyleColor(2);
			ImGui::PopStyleVar();

            if (ed::BeginGroupHint(groupNode.id))
            {
                auto min = ed::GetGroupMin();

                auto size = ed::GetNodeSize(groupNode.id);
                auto position = ed::GetNodePosition(groupNode.id);

                groupNode.size = { size.x, size.y };
                groupNode.position = { position.x, position.y };

				ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
				ImGui::BeginGroup();
				ImGui::TextUnformatted(groupNode.name.c_str());
				ImGui::EndGroup();

				auto drawList = ed::GetHintBackgroundDrawList();

				ImRect hintBounds = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				ImRect hintFrameBounds = hintBounds;
                hintFrameBounds.Min.x -= 8;
                hintFrameBounds.Min.y -= 4;
                hintFrameBounds.Max.x += 8;
                hintFrameBounds.Max.y += 4;

				drawList->AddRectFilled(
					hintFrameBounds.GetTL(),
					hintFrameBounds.GetBR(),
                    groupColor, 4.0f);

				drawList->AddRect(
					hintFrameBounds.GetTL(),
					hintFrameBounds.GetBR(),
                    groupColor, 4.0f);
            }
            ed::EndGroupHint();
        }

        // set any new node positions that we need to
        {
            for (auto & info: m_newNodePositions)
                ed::SetNodePosition(info.first, info.second);
            m_newNodePositions.clear();
        }

        // Make the links: input pins are responsible for linking from their source.
        {
            m_links.clear();
            for (RenderGraphNode& node : g_renderGraph.nodes)
            {
                std::vector<NodePinInfo> pinInfo = GetNodePins(g_renderGraph, node);
                for (const NodePinInfo& info : pinInfo)
                {
                    if (!info.isInput)
                        continue;

                    int connectionNodeIndex = GetNodeIndexByName(g_renderGraph, info.inputNode->c_str());
                    if (connectionNodeIndex == -1)
                        continue;

                    std::vector<NodePinInfo> connectionPinInfo = GetNodePins(g_renderGraph, g_renderGraph.nodes[connectionNodeIndex]);
                    int connectionPinIndex = GetNodePinIndexByName(connectionPinInfo, info.inputNodePin->c_str());
                    if (connectionPinIndex < 0)
                        continue;

                    int linkId = uniqueId++;

                    // curved wire
                    ed::Link(linkId, *connectionPinInfo[connectionPinIndex].outputEditorId, *info.inputEditorId);

                    LinkInfo linkInfo;
                    linkInfo.srcNodeName = *info.inputNode;
                    linkInfo.srcPin = *info.inputNodePin;
                    linkInfo.destNodeName = GetNodeName(node);
                    linkInfo.destPin = info.name;
                    m_links[linkId] = linkInfo;
                }
            }
        }

        // Handle creation action, returns true if editor want to create new object (node or link)
        if (ed::BeginCreate())
        {
            auto showLabel = [](const char* label, ImColor color)
            {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                auto size = ImGui::CalcTextSize(label);

                auto padding = ImGui::GetStyle().FramePadding;
                auto spacing = ImGui::GetStyle().ItemSpacing;

                ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                auto rectMin = ImGui::GetCursorScreenPos() - padding;
                auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                auto drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
                ImGui::TextUnformatted(label);
            };

            ed::PinId inputPinId, outputPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
            {
                if (inputPinId && outputPinId)
                {
                    EditorPinInfo& src = m_pins[(int)inputPinId.Get()];
                    EditorPinInfo& dest = m_pins[(int)outputPinId.Get()];

                    if (src.isInputPin == dest.isInputPin || src.node == dest.node)
                    {
                        ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    }
                    else if (ed::AcceptNewItem())
                    {
                        if (!dest.isInputPin)
                            std::swap(src, dest);

                        int destNodeId = GetNodeIndexByName(g_renderGraph, dest.node.c_str());
                        std::vector<NodePinInfo> destNodeInfo = GetNodePins(g_renderGraph, g_renderGraph.nodes[destNodeId]);

                        int destPinIndex = GetNodePinIndexByName(destNodeInfo, dest.pin.c_str());
                        if (destPinIndex >= 0)
                        {
                            *destNodeInfo[destPinIndex].inputNode = src.node;
                            *destNodeInfo[destPinIndex].inputNodePin = src.pin;
                            g_renderGraphDirty = true;
                        }

                    }
                }
            }
        }
        ed::EndCreate();

        if (ed::BeginDelete())
        {
            // Delete links
            {
                // Gather links to delete
                std::vector<int> deleteLinkIds;
                ed::LinkId linkId = 0;
                while (ed::QueryDeletedLink(&linkId))
                {
                    if (ed::AcceptDeletedItem())
                    {
                        g_renderGraphDirty = true;
                        deleteLinkIds.push_back((int)linkId.Get());
                    }
                }

                // delete links
                for (int linkId : deleteLinkIds)
                {
                    DeleteLink(linkId);
                }
            }

            // Delete nodes
            {
                // Gather links to delete
                std::vector<int> deleteNodeIds;
                ed::NodeId nodeId = 0;
                while (ed::QueryDeletedNode(&nodeId))
                {
                    if (ed::AcceptDeletedItem())
                    {
                        g_renderGraphDirty = true;
                        deleteNodeIds.push_back((int)nodeId.Get());
                    }
                }

                // Delete nodes from highest to lowest index
                std::sort(deleteNodeIds.begin(), deleteNodeIds.end(), [](int a, int b) { return a >= b; });

                // delete each node
                for (int deleteIndex : deleteNodeIds)
                {
                    if (deleteIndex >= g_groupNodesStartId)
                    {
                        DeleteGroupNode(deleteIndex);
                    }
                    else
                    {
						DeleteNode(deleteIndex - 1);
                    }
                }
            }
        }
        ed::EndDelete();

        // Menus
        ed::LinkId hoveredLink = 0;
        ImVec2 openPopupPosition = ImGui::GetMousePos();
        {
            ed::Suspend();
            if (ed::ShowBackgroundContextMenu())
                ImGui::OpenPopup("Create New Node");
            if (ed::ShowNodeContextMenu(&g_contextMenuNodeId))
                ImGui::OpenPopup("Node Context Menu");
            if (ed::ShowLinkContextMenu(&g_contextMenuLinkId))
                ImGui::OpenPopup("Link Context Menu");
            hoveredLink = ed::GetHoveredLink();
            ed::Resume();
        }

        ed::Suspend();
        if (hoveredLink.Get() != 0)
        {
            if (ImGui::IsPopupOpen("ShowLinkInfo") || !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
            {
                ImGui::OpenPopup("ShowLinkInfo");
                ImGui::SetNextWindowPos(ImGui::GetMousePos() + ImVec2(20, 0));
                if (ImGui::Begin("ShowLinkInfo", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
                {
                    if (m_links.count((int)hoveredLink.Get()) > 0)
                    {
                        const LinkInfo& linkInfo = m_links[(int)hoveredLink.Get()];

                        int resourceNodeIndex = GetResourceNodeIndexForPin(g_renderGraph, linkInfo.srcNodeName.c_str(), linkInfo.srcPin.c_str());
                        if (resourceNodeIndex != -1)
                        {
                            const RenderGraphNode& node = g_renderGraph.nodes[resourceNodeIndex];
                            ImGui::Text((GetNodeShorterTypeName(node) + ": " + GetNodeName(node)).c_str());
                        }
                    }

                    ImGui::End();
                }
            }
        }
        if (ImGui::BeginPopup("Node Context Menu"))
        {
            auto newNodePostion = openPopupPosition;
            int nodeId = int(g_contextMenuNodeId.Get()) - 1;
            if (nodeId >= 0 && nodeId < g_renderGraph.nodes.size())
            {
                RenderGraphNode& node = g_renderGraph.nodes[nodeId];

                ImGui::Text("%s: %s", GetNodeShorterTypeName(node).c_str(), GetNodeName(node).c_str());

                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));

                DispatchLambdaAction(node,
                    [](RenderGraphNode_ActionBase& node)
                    {
                        if (!node.condition.alwaysFalse)
                        {
                            if (ImGui::MenuItem("Disable Node"))
                            {
                                node.condition.alwaysFalse = !node.condition.alwaysFalse;
                                g_renderGraphDirty = true;
                            }
                        }
                        else
                        {
                            if (ImGui::MenuItem("Enable Node"))
                            {
                                node.condition.alwaysFalse = !node.condition.alwaysFalse;
                                g_renderGraphDirty = true;
                            }
                        }
                    }
                );

                if (ImGui::MenuItem("Duplicate"))
                {
                    // make a unique name for the node
                    char newNodeName[64];
                    {
                        int nextNodeIndex = 0;
                        while (true)
                        {
                            nextNodeIndex++;
                            sprintf_s(newNodeName, "%s (%i)", GetNodeName(node).c_str(), nextNodeIndex);
                            bool nameExists = false;
                            for (const RenderGraphNode& node : g_renderGraph.nodes)
                            {
                                if (GetNodeName(node) == newNodeName)
                                {
                                    nameExists = true;
                                    break;
                                }
                            }
                            if (!nameExists)
                                break;
                        }
                    }

                    // make the new node
                    RenderGraphNode newNode;
                    newNode = node;
                    SetNodeName(newNode, newNodeName);

                    // disconnect any connections
                    std::vector<NodePinInfo> pinInfo = GetNodePins(g_renderGraph, newNode);
                    for (NodePinInfo& info : pinInfo)
                    {
                        if (info.inputNode)
                            *info.inputNode = "";

                        if (info.inputNodePin)
                            *info.inputNodePin = "";

                        if (info.inputEditorId)
                            *info.inputEditorId = -1;

                        if (info.outputEditorId)
                            *info.outputEditorId = -1;
                    }

                    // Add the new node to the render graph and give it a position in the editor
                    g_renderGraph.nodes.push_back(newNode);
                    m_newNodePositions[(int)g_renderGraph.nodes.size()] = newNodePostion;

                    // Mark this as the created node, so it will be selected
                    g_createdNodeIndex = (int)g_renderGraph.nodes.size();

                    g_renderGraphDirty = true;
                }

                if (ImGui::MenuItem("Delete"))
                {
                    ed::ClearSelection();
                    
                    std::string deleteName = GetNodeName(node);
                    int deleteIndex = GetNodeIndexByName(g_renderGraph, deleteName.c_str());
                    DeleteNode(deleteIndex);
                }

                ImGui::PopStyleColor();
            }
            else
                ImGui::Text("Invalid Node");

            ImGui::EndPopup();
        }
        if (ImGui::BeginPopup("Link Context Menu"))
        {
            ImGui::Text("Link");

            ImGui::Separator();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));

            if (ImGui::MenuItem("Delete"))
            {
                ed::ClearSelection();

                int linkId = int(g_contextMenuLinkId.Get());
                DeleteLink(linkId);
            }

            ImGui::PopStyleColor();

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Create New Node"))
        {
            auto newNodePostion = openPopupPosition;

            ImGui::Text("Create Node");

            ImGui::Separator();

            #include "external/df_serialize/_common.h"
            #define VARIANT_SEP()
            #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
                if (_TYPE::c_isResourceNode) \
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 128, 255, 255)); \
                else \
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 128, 64, 255)); \
                if (_TYPE::c_showInEditor && ImGui::MenuItem(_TYPE::c_editorName.c_str())) \
                { \
                    char newNodeName[64]; \
                    int nextNodeIndex = 0; \
                    while (true) \
                    { \
                        nextNodeIndex++; \
                        sprintf_s(newNodeName, "Node %i", nextNodeIndex); \
                        bool nameExists = false; \
                        for (const RenderGraphNode& node : g_renderGraph.nodes) \
                        { \
                            if (GetNodeName(node) == newNodeName) \
                            { \
                                nameExists = true; \
                                break; \
                            } \
                        } \
                        if (!nameExists) \
                            break; \
                    } \
                    RenderGraphNode newNode; \
                    newNode._index = RenderGraphNode::c_index_##_NAME; \
                    newNode._NAME.name = newNodeName; \
                    g_renderGraph.nodes.push_back(newNode); \
                    m_newNodePositions[(int)g_renderGraph.nodes.size()] = newNodePostion; \
                    g_renderGraphDirty = true; \
                    g_createdNodeIndex = (int)g_renderGraph.nodes.size(); \
                } \
                ImGui::PopStyleColor();
            // clang-format off
            #include "external/df_serialize/_fillunsetdefines.h"
            #include "Schemas/RenderGraphNodesVariant.h"
            // clang-format on

            ImGui::Separator();

            if (ImGui::MenuItem("Group"))
            {
                const ImVec2 defaultSize = ImVec2(380, 154);

                auto& node = g_renderGraph.editorGroupNodes.emplace_back();
                node.position = { newNodePostion.x, newNodePostion.y };
                node.size = { defaultSize.x, defaultSize.y };
                node.name = "New Group";
                node.id = g_groupNodesStartId + int32_t(g_renderGraph.editorGroupNodes.size()) - 1;

                ed::SetNodePosition(node.id, newNodePostion);
                ed::SetGroupSize(node.id, defaultSize);

                g_renderGraphDirty = true;
            }

            ImGui::EndPopup();
        }
        ed::Resume();

        ed::End();

        if (g_renderGraphFirstFrame)
            ed::NavigateToContent();

        ed::SetCurrentEditor(nullptr);

        g_renderGraphFirstFrame = false;

        ImGui::End();
    }

    ed::EditorContext* m_Context = nullptr;

    struct EditorPinInfo
    {
        bool isInputPin = false;
        std::string node;
        std::string pin;
    };
    std::unordered_map<int, EditorPinInfo> m_pins;

	std::unordered_map<int, ImVec2> m_newNodePositions;
	std::unordered_map<int, ImVec2> m_newGroupSizes;

    struct LinkInfo
    {
        std::string srcNodeName;
        std::string srcPin;
        std::string destNodeName;
        std::string destPin;
    };

    std::unordered_map<int, LinkInfo> m_links;


	void ImGuiRecentFiles()
	{
		if (!m_recentFiles.GetEntries().empty())
		{
			if (ImGui::BeginMenu("Recent Files"))
			{
				for (const auto& el : m_recentFiles.GetEntries())
				{
                    if (el.empty())
                        continue;

					if (ImGui::MenuItem(el.c_str()))
					{
                        // make a copy so we don't point to data we might change
                        std::string fileName = el;
						LoadJSONFile(fileName.c_str());
                        break;
					}
				}
				ImGui::EndMenu();
			}
		}
	}
};

int Main(int argc, char** argv)
{
    g_argc = argc;
    g_argv = argv;

    Example example("GigiEdit", argc, argv);

    if (example.Create())
        return example.Run();

    if (g_previewWindow.hProcess != nullptr)
        TerminateProcess(g_previewWindow.hProcess, 0);

    return 0;
}

// TODO: not sure how to make the viewport stuff work correctly. ImGuiConfigFlags_ViewportsEnable in application.cpp is commented out.