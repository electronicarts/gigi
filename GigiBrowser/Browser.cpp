///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Browser.h"

#include "ThreadedWork.h"
#include "Jobs.h"
#include "GigiViewerDX12/DX12Utils/CreateResources.h"
#include "Utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>

#include "shlobj_core.h"

bool g_logOpen = false;

bool ShowErrorMessage(const char* fmt, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	Browser::Log(true, "%s", buffer);
	return true;
}

static bool Spinner(const char* label, float radius, float thickness, const ImU32& color) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	// Render
	window->DrawList->PathClear();

	int num_segments = 30;
	int start = (int)abs(ImSin((float)g.Time * 1.8f) * (num_segments - 5));

	const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
	const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

	const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

	for (int i = 0; i < num_segments; i++) {
		const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
		window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + (float)g.Time * 8) * radius,
			centre.y + ImSin(a + (float)g.Time * 8) * radius));
	}

	window->DrawList->PathStroke(color, false, thickness);
	return true;
}

static const char* EnumToString(Browser::DownloadedStatus status)
{
	switch (status)
	{
		case Browser::DownloadedStatus::Any: return "Any";
		case Browser::DownloadedStatus::Downloaded: return "Downloaded";
		case Browser::DownloadedStatus::NotDownloaded: return "NotDownloaded";
	}
	return nullptr;
}

void Browser::MakeInitialLayout()
{
	ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();

	ImVec2 vpSize = ImGui::GetMainViewport()->Size;
	if (vpSize.x <= 0.0f || vpSize.y <= 0.0f)
		return;

	if (!m_resetLayout)
		return;
	m_resetLayout = false;

	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton;

	ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
	ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

	ImGuiID dockspace_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.2f, nullptr, &dockspace_id);

	ImGui::DockBuilderDockWindow("Gigi Browser", dockspace_id);
	ImGui::DockBuilderDockWindow("Log", dockspace_top);

	ImGui::DockBuilderGetNode(dockspace_id)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoTabBar;
	ImGui::DockBuilderGetNode(dockspace_top)->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;

	ImGui::DockBuilderFinish(dockspace_id);
}

void Browser::Log(bool error, const char* fmt, ...)
{
	if (!s_singularBrowser)
		return;

	char buffer[4096];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	LogLine newLine;
	newLine.error = error;
	newLine.text = buffer;
	s_singularBrowser->m_log.push_back(newLine);

	// Also show it in the output window
	OutputDebugStringA(error ? "Error: " : "Log: ");
	OutputDebugStringA(buffer);
	OutputDebugStringA("\n");
}

void Browser::TickWorkerThreads()
{
	// Handle the main thread work that needs doing after the worker thread work is done
	m_workerThreads.ProcessFinishedJobs();

	// Refresh the search results if any of the jobs requested it
	if (m_refreshSearchResults)
	{
		GetSearchResults(true);
		m_refreshSearchResults = false;
	}
}

void Browser::GetSearchResults(bool refreshingResults)
{
	if (!refreshingResults)
	{
		m_lastSearch_Server = m_search_Server;
		m_lastSearch_Text = m_search_Text;
		m_lastSearch_Tags = m_search_Tags;
		m_lastSearch_Downloaded = m_search_Downloaded;
	}

	// Clear our search results
	m_searchResults.clear();

	// Build the search query
	std::stringstream query;
	std::vector<std::string> parameterStrings;
	query << "select * from Techniques";
	bool firstWhereClause = true;

	if (m_lastSearch_Server > 0)
	{
		query << (firstWhereClause ? " where " : " and ") << "Origin_Name = ?";
		parameterStrings.push_back(m_searchServers[m_lastSearch_Server - 1]);
		firstWhereClause = false;
	}

	if (!m_lastSearch_Text.empty())
	{
		query << (firstWhereClause ? " where " : " and ") << "(Details_Title like ? or Details_Description like ? or Details_Author like ? or SummaryHash like ?)";
		parameterStrings.push_back(std::string("%") + m_lastSearch_Text + std::string("%"));
		parameterStrings.push_back(std::string("%") + m_lastSearch_Text + std::string("%"));
		parameterStrings.push_back(std::string("%") + m_lastSearch_Text + std::string("%"));
		parameterStrings.push_back(std::string("%") + m_lastSearch_Text + std::string("%"));
		firstWhereClause = false;
	}

	if (!m_lastSearch_Tags.empty())
	{
		query << (firstWhereClause ? " where " : " and ") << "(Details_Tags like ?)";
		parameterStrings.push_back(std::string("%") + m_lastSearch_Tags + std::string("%"));
		firstWhereClause = false;
	}

	if (m_lastSearch_Downloaded != DownloadedStatus::Any)
	{
		query << (firstWhereClause ? " where " : " and ") << "Downloaded=" << (m_lastSearch_Downloaded == DownloadedStatus::Downloaded ? 1 : 0);
	}

	query << " order by Summary_ListedDate desc";

	// Do the query and gather results
	if (!Database::ExecuteSQL(query.str().c_str(), parameterStrings,
		[this](sqlite3_stmt* statement)
		{
			int column = 0;

			BrowserCachedTechnique technique;

			// skip summary hash
			column++;

			technique.Summary.Repo = (const char*)sqlite3_column_text(statement, column++);
			technique.Summary.Commit = (const char*)sqlite3_column_text(statement, column++);
			technique.Summary.DetailsFile = (const char*)sqlite3_column_text(statement, column++);
			technique.Summary.ListedDate = (const char*)sqlite3_column_text(statement, column++);

			technique.Details.Title = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.Description = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.Author = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.Tags = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.Technique = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.Website = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.Screenshot = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.License = (const char*)sqlite3_column_text(statement, column++);
			technique.Details.GigiVersion = (const char*)sqlite3_column_text(statement, column++);

			technique.Downloaded = (sqlite3_column_int(statement, column++) != 0);

			technique.Origin.Name = (const char*)sqlite3_column_text(statement, column++);
			technique.Origin.Repo = (const char*)sqlite3_column_text(statement, column++);
			technique.Origin.Branch = (const char*)sqlite3_column_text(statement, column++);
			technique.Origin.TechniqueList = (const char*)sqlite3_column_text(statement, column++);

			m_searchResults.push_back(technique);
		}
	))
	{
		Log(true, "Could not get search results");
	}
}

bool Browser::Init(ID3D12Device* device, int maxFramesInFlight, ID3D12DescriptorHeap* ImGuiSRVHeap, int ImGuiSRVHeapDescriptorCount, int ImGuiSRVHeapDescriptorSize, int argc, char** argv)
{
	// Get the app data local path, so we can put our technique cache there
	{
		wchar_t* path = nullptr;
		if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
		{
			CoTaskMemFree(path);
			MessageBoxA(nullptr, "Could not get app data local folder", "Error", MB_OK);
			Log(true, "Could not get app data local folder.\n");
			return false;
		}
		m_cacheDir = std::filesystem::path(path) / "GigiBrowser";
		CoTaskMemFree(path);

		JobBase::s_cacheDir = m_cacheDir;
	}

	m_maxFramesInFlight = maxFramesInFlight;
	m_device = device;
	m_descriptorTableCache_imgui.Init(maxFramesInFlight);
	m_SRVHeapAllocationTracker_imgui.Init(maxFramesInFlight, ImGuiSRVHeap, ImGuiSRVHeapDescriptorCount, ImGuiSRVHeapDescriptorSize);
	m_SRVHeapAllocationTracker_imgui.MarkIndexAllocated(0, HEAP_DEBUG_TEXT()); // used by the imgui font texture
	m_SRVHeapAllocationTracker_imgui.MarkIndexAllocated(1, HEAP_DEBUG_TEXT()); // Also used by imgui

	// (Re)Initialize the database, as needed
	if (!Database::Open(m_cacheDir / "techniques.db", m_techniquesThatWereAlreadyDownloaded))
	{
		MessageBoxA(nullptr, "Could not open technique database, or create it.", "Error Opening Database", MB_OK);
		Log(true, "Could not open database.\n");
		return false;
	}

	// Load the servers from browserservers.txt
	std::vector<BrowserServerInfo> serverList;
	{
		// If no servers we won't list any
		std::vector<char> data;
		if (!LoadTextFile("browserservers.txt", data) || data.size() == 0)
			return true;

		// Get the data
		char* cursor = data.data();
		while (1)
		{
			BrowserServerInfo serverInfo;
			bool success =
				GetStringUntilWhitespace(serverInfo.Repo, cursor) &&
				GetStringUntilWhitespace(serverInfo.Branch, cursor) &&
				GetStringUntilWhitespace(serverInfo.TechniqueList, cursor) &&
				GetStringUntilNewline(serverInfo.Name, cursor)  // name last so that it can have spaces in it - it's the rest of the line
			;

			if (!success)
				break;

			serverList.push_back(serverInfo);
		}
	}

	// Handle -Technique command line parameter, to test a technique's setup before adding it to a server list.
	bool testingTechnique = false;
	if (argc > 1 && !strcmp(argv[1], "-Technique"))
	{
		if (argc != 5)
		{
			Log(true, "Wrong number of arguments for -Technique command line parameter. Usage is: -Technique <Repo> <Commit> <DetailsFile>");
		}
		else
		{
			// Fake some server info
			BrowserServerInfo serverInfo;
			serverInfo.Name = "LocalTest";

			// Fill out the summary information
			BrowserTechniqueSummary summary;
			summary.Repo = argv[2];
			summary.Commit = argv[3];
			summary.DetailsFile = argv[4];

			// Make sure this entry doesn't exist in the database
			std::vector<std::string> params;
			params.push_back(GetSummaryHashString(summary));
			Database::ExecuteSQL("delete from Techniques where SummaryHash = ?", params);

			// Make sure this entry doesn't have a git repo
			std::filesystem::path techniquePath = m_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(summary);
			std::filesystem::remove_all(techniquePath);

			// Add this technique
			m_workerThreads.Add<Job_UpdateTechnique>(serverInfo, summary);

			testingTechnique = true;
		}
	}

	// Make the sorted list of servers that can be searched - A union of servers in the server list text file, and the database.
	{
		std::unordered_set<std::string> searchServers;

		if (testingTechnique)
			searchServers.insert("LocalTest");

		Database::ExecuteSQL("select distinct Origin_Name from Techniques",
			[&searchServers](sqlite3_stmt* statement)
			{
				searchServers.insert((const char*)sqlite3_column_text(statement, 0));
			}
		);
		for (BrowserServerInfo& serverInfo : serverList)
			searchServers.insert(serverInfo.Name);

		for (const std::string& serverName : searchServers)
			m_searchServers.push_back(serverName);
		std::sort(m_searchServers.begin(), m_searchServers.end());
	}

	// Get the technique list for each server
	for (BrowserServerInfo& serverInfo : serverList)
		m_workerThreads.Add<Job_UpdateServer>(serverInfo);

	// If we are testing a technique, show only the "LocalTest" items by default
	if (testingTechnique)
	{
		int index = -1;
		for (const std::string& s : m_searchServers)
		{
			index++;
			if (s == "LocalTest")
			{
				m_search_Server = index + 1;
				break;
			}
		}
	}

	GetSearchResults(false);

	return true;
}

void Browser::Release()
{
	m_uploadBufferTracker.Release();

	m_descriptorTableCache_imgui.Release(m_SRVHeapAllocationTracker_imgui);

	// Ensure the heaps are clear
	m_SRVHeapAllocationTracker_imgui.FlushFreeLists();
	if (m_SRVHeapAllocationTracker_imgui.AllocatedCount() != 2) // two used by imgui
		assert(false);
	m_SRVHeapAllocationTracker_imgui.Release();

	// Release all texture resources
	for (auto& it : m_textureResources)
	{
		it.second->Release();
	}
	m_textureResources.clear();
}

void Browser::ProcessPendingTextureLoads(ID3D12GraphicsCommandList* commandList)
{
	for (const std::string& fileName : m_pendingTextureLoads)
	{
		// Get the pixel data
		TextureCache::Texture& tex = m_textureCache.Get(m_fileCache, fileName.c_str());
		if (!tex.Valid())
		{
			Log(true, "Could not load texture \"%s\"", fileName.c_str());
			continue;
		}

		if (tex.format != DXGI_FORMAT_R8G8B8A8_UNORM && tex.format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			Log(true, "texture \"%s\" is an unsupported format: \"%s\"", fileName.c_str(), Get_DXGI_FORMAT_Info(tex.format).name);
			continue;
		}

		// Create a resource
		unsigned int size[3] = { (unsigned int)tex.width, (unsigned int)tex.height, 1 };
		ID3D12Resource* resource = CreateTexture(m_device, size, 1, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, ResourceType::Texture2D, tex.fileName.c_str());
		m_textureResources[tex.fileName] = resource;

		// get info about layout of texture memory
		D3D12_RESOURCE_DESC desc = resource->GetDesc();
		std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)));
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
		unsigned int numRows = 0;
		size_t unalignedPitch = 0;
		m_device->GetCopyableFootprints(&desc, 0, 1, 0, layout, &numRows, &unalignedPitch, nullptr);
		size_t alignedPitch = layout->Footprint.RowPitch;
		size_t bufferSize = numRows * alignedPitch * layout->Footprint.Depth;

		// Create an upload buffer
		UploadBufferTracker::Buffer* uploadBuffer = m_uploadBufferTracker.GetBuffer(m_device, bufferSize, false);
		if (!uploadBuffer)
		{
			Log(true, "Could not get upload buffer for %s", tex.fileName.c_str());
			continue;
		}

		// map the memory
		D3D12_RANGE readRange;
		readRange.Begin = 1;
		readRange.End = 0;
		unsigned char* destPixels = nullptr;
		HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, reinterpret_cast<void**>(&destPixels));
		if (FAILED(hr))
			continue;

		// fill the upload buffer with pixel data.
		const unsigned char* src = tex.images[0].pixels.data();
		unsigned char* dest = destPixels;

		for (unsigned int i = 0; i < numRows; ++i)
		{
			memcpy(dest, src, unalignedPitch);
			src += unalignedPitch;
			dest += alignedPitch;
		}

		// unmap the memory
		uploadBuffer->buffer->Unmap(0, nullptr);

		// Copy the upload buffer into the resource
		{
			D3D12_TEXTURE_COPY_LOCATION src = {};
			src.pResource = uploadBuffer->buffer;
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = *layout;

			D3D12_TEXTURE_COPY_LOCATION dest = {};
			dest.pResource = resource;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dest.SubresourceIndex = 0;

			commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
		}

		// Transition the resource to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = resource;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			commandList->ResourceBarrier(1, &barrier);
		}
	}
	m_pendingTextureLoads.clear();
}

void* Browser::GetDescriptorTableForImage(const char* path, int& width, int& height)
{
	TextureCache::Texture& tex = m_textureCache.Get(m_fileCache, path);
	if (!tex.Valid())
		return nullptr;

	width = tex.width;
	height = tex.height;

	auto it = m_textureResources.find(tex.fileName);
	if (it == m_textureResources.end())
	{
		m_pendingTextureLoads.insert(tex.fileName);
		return nullptr;
	}

	DescriptorTableCache::ResourceDescriptor desc;
	desc.m_resource = it->second;
	desc.m_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	D3D12_GPU_DESCRIPTOR_HANDLE descTable;
	std::string error;
	if (!m_descriptorTableCache_imgui.GetDescriptorTable(m_device, m_SRVHeapAllocationTracker_imgui, &desc, 1, descTable, error, HEAP_DEBUG_TEXT()))
	{
		ShowErrorMessage("Could not get descriptor table: %s", error.c_str());
		return nullptr;
	}

	return (void*)descTable.ptr;
}

void Browser::ShowBrowserWindow()
{
	if (!ImGui::Begin("Gigi Browser"))
	{
		ImGui::End();
		return;
	}

	// Search
	{
		ImGui::TextUnformatted("Technique Search");

		// Show information about how to share
		{
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
			ImGui::Text("[?]");
			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				ImGui::SetTooltip("For information about how to share techniques, or how the sharing system works,\nplease see UserDocumentation/GigiBrowser_Documentation.pdf");
		}

		if (ImGui::Button("Open Editor"))
			RunCommandLine(false, "GigiEdit.exe");

		ImGui::SameLine();
		if (ImGui::Button("Open Viewer"))
			RunCommandLine(false, "GigiViewerDX12.exe");

		ImGui::SameLine();
		if (ImGui::Button(g_logOpen ? "Hide Log" : "View Log"))
			g_logOpen = !g_logOpen;

		// Text
		{
			char buffer[4096];
			strcpy_s(buffer, m_search_Text.c_str());
			if (ImGui::InputText("Text", buffer, _countof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
				GetSearchResults(false);
			m_search_Text = buffer;
		}

		// Tag
		{
			char buffer[4096];
			strcpy_s(buffer, m_search_Tags.c_str());
			if (ImGui::InputText("Tags", buffer, _countof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
				GetSearchResults(false);
			m_search_Tags = buffer;
		}

		// Server drop down
		{
			// Get the longest text width of the server names
			float comboWidth = 0.0f;
			for (const std::string& serverName: m_searchServers)
				comboWidth = max(comboWidth, ImGui::CalcTextSize(serverName.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);

			// ensure server selection is in range
			m_search_Server = min(m_search_Server, (int)m_searchServers.size() + 1);

			// Show the labels
			ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
			const char* selectedLabel = (m_search_Server > 0) ? m_searchServers[m_search_Server - 1].c_str() : "Any";
			if (ImGui::BeginCombo("Server", selectedLabel, ImGuiComboFlags_None))
			{
				for (int n = 0; n <= (int)m_searchServers.size(); n++)
				{
					const bool is_selected = (m_search_Server == n);
					std::string displayString = (n > 0) ? m_searchServers[n - 1] : "Any";
					if (ImGui::Selectable(displayString.c_str(), is_selected))
						m_search_Server = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}

		// Downloaded drop down
		{
			float comboWidth = 0.0f;
			for (int i = 0; ; i++)
			{
				const char* label = EnumToString((DownloadedStatus)i);
				if (!label)
					break;

				comboWidth = max(comboWidth, ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f);
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
			if (ImGui::BeginCombo("Downloaded", EnumToString(m_search_Downloaded), ImGuiComboFlags_None))
			{
				for (int i = 0; ;i++)
				{
					const char* label = EnumToString((DownloadedStatus)i);
					if (!label)
						break;

					const bool is_selected = (m_search_Downloaded == (DownloadedStatus)i);
					if (ImGui::Selectable(label, is_selected))
						m_search_Downloaded = (DownloadedStatus)i;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
	}

	if (ImGui::Button("Clear"))
	{
		m_search_Server = 0;
		m_search_Text = "";
		m_search_Tags = "";
		m_search_Downloaded = DownloadedStatus::Any;
	}

	ImGui::SameLine();
	if (ImGui::Button("Search") || ImGui::IsKeyPressed(ImGuiKey_Enter))
		GetSearchResults(false);

	ImGui::Separator();

	ImGui::Text("%i search result%s", m_searchResults.size(), (m_searchResults.size() == 1 ? "" : "s"));

	{
		int jobsInFlight = m_workerThreads.JobsInFlight();
		if (jobsInFlight > 0)
		{
			ImGui::SameLine();
			ImGui::Text("     %i %s pending", jobsInFlight, (jobsInFlight == 1) ? "operation" : "operations");
			ImGui::SameLine();
			float spinnerHeight = ImGui::CalcTextSize("10 async operations in flight").y / 4.0f;
			Spinner("Testing", spinnerHeight, 2.0f, ImColor(0.37f, 0.47f, 0.57f, 1.0f));
		}
	}

	// Calculate values for paging
	const int c_numPages = ((int)m_searchResults.size() + c_resultsPerPage - 1) / c_resultsPerPage;
	m_search_Page = max(0, min(m_search_Page, c_numPages - 1));
	const int c_startTechniqueIndex = m_search_Page * c_resultsPerPage;
	const int c_endTechniqueIndex = min((m_search_Page + 1) * c_resultsPerPage, (int)m_searchResults.size());

	// Previous pages
	{
		bool disabled = (m_search_Page == 0);
		if (disabled)
			ImGui::BeginDisabled();

		if (ImGui::Button("<<"))
			m_search_Page = 0;

		ImGui::SameLine();
		if (ImGui::Button("<"))
			m_search_Page = max(m_search_Page - 1, 0);

		if (disabled)
			ImGui::EndDisabled();
	}

	ImGui::SameLine();
	ImGui::Text("Page %i / %i", m_search_Page + 1, c_numPages);

	// Next pages
	{
		bool disabled = (m_search_Page >= c_numPages - 1);
		if (disabled)
			ImGui::BeginDisabled();

		ImGui::SameLine();
		if (ImGui::Button(">"))
			m_search_Page = min(m_search_Page + 1, c_numPages - 1);

		ImGui::SameLine();
		if (ImGui::Button(">>"))
			m_search_Page = c_numPages - 1;

		if (disabled)
			ImGui::EndDisabled();
	}

	ImGui::Separator();

	if (ImGui::BeginChild("SearchResults"))
	{
		// Technique list
		static int s_popupWindowTechniqueIndex = 0;
		static bool s_showLicenseWindow = false;
		static bool s_showScreenshotWindow = false;

		static const float c_imageHeight = 256.0f;
		static const float c_maxImageWidth = c_imageHeight * 1.5f;

		if (ImGui::BeginTable("table", 2, ImGuiTableFlags_Borders))
		{
			ImGui::TableSetupColumn("0", ImGuiTableColumnFlags_WidthFixed, c_maxImageWidth);
			ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthStretch);

			for (int searchResultIndex = c_startTechniqueIndex; searchResultIndex < c_endTechniqueIndex; ++searchResultIndex)
			{
				BrowserCachedTechnique& technique = m_searchResults[searchResultIndex];

				ImGui::PushID(GetSummaryHashString(technique.Summary).c_str());

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				// make sure the image is loaded
				{
					std::filesystem::path techniquePath = m_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(technique.Summary) / technique.Summary.DetailsFile;
					std::filesystem::path rootPath = techniquePath.replace_filename("");
					std::filesystem::path screenshotPath = rootPath / technique.Details.Screenshot;

					int imageW = 0;
					int imageH = 0;
					void* descriptorTable = GetDescriptorTableForImage(screenshotPath.string().c_str(), imageW, imageH);

					if (descriptorTable)
					{
						float imageWidth = float(imageW) * c_imageHeight / float(imageH);
						ImVec2 uv0(0.0f, 0.0f);
						ImVec2 uv1(1.0f, 1.0f);
						if (imageWidth > c_maxImageWidth)
						{
							float trim = (imageWidth - c_maxImageWidth) / 2.0f;

							float trimU = trim / imageWidth;

							uv0.x = trimU;
							uv1.x = 1.0f - uv0.x;

							imageWidth = c_maxImageWidth;
						}
						else
						{
							float excess = (c_maxImageWidth - imageWidth) / 2.0f;
							ImGui::SameLine(excess);
						}
						if (ImGui::ImageButton("screenshotSmall", (ImTextureID)descriptorTable, ImVec2(imageWidth, c_imageHeight), uv0, uv1))
						{
							s_showScreenshotWindow = true;
							s_popupWindowTechniqueIndex = searchResultIndex;
						}
					}
				}

				ImGui::TableNextColumn();

				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(64, 128, 255, 255));
				ImGui::Text("%s", technique.Details.Title.c_str());
				ImGui::PopStyleColor();
				ImGui::Text("By");
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(64, 128, 255, 255));
				ImGui::Text("%s", technique.Details.Author.c_str());
				ImGui::PopStyleColor();

				ImGui::Text("Tags:");
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 255, 64, 255));
				ImGui::Text("%s", technique.Details.Tags.c_str());
				ImGui::PopStyleColor();

				ImGui::TextWrapped("%s", technique.Details.Description.c_str());

				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(96, 96, 96, 255));
				ImGui::Text("Gigi Version: %s", technique.Details.GigiVersion.c_str());
				ImGui::Text("Listed On: %s (%s)", technique.Origin.Name.c_str(), technique.Summary.ListedDate.c_str());
				ImGui::Text("%s (%s)\n%s", technique.Summary.Repo.c_str(), technique.Summary.Commit.c_str(), technique.Summary.DetailsFile.c_str());
				ImGui::Text("%s", GetSummaryHashString(technique.Summary).c_str());
				ImGui::PopStyleColor();

				//////////////////////////////////////////////////////////

				if (!technique.Downloaded)
				{
					bool pendingDownload = m_pendingDownloads.count(GetSummaryHashString(technique.Summary)) > 0;

					if (pendingDownload)
						ImGui::BeginDisabled();

					if (ImGui::Button(pendingDownload ? "Downloading..." : "Download"))
						m_workerThreads.Add<Job_DownloadTechnique>(technique);

					if (pendingDownload)
						ImGui::EndDisabled();
				}
				else
				{
					std::filesystem::path rootPath = (m_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(technique.Summary) / technique.Summary.DetailsFile).replace_filename("");
					std::filesystem::path techniquePath = rootPath / technique.Details.Technique;

					if (!technique.Details.Technique.empty())
					{
						if (ImGui::Button("Open In Editor"))
							RunCommandLine(false, "GigiEdit.exe \"%s\"", techniquePath.string().c_str());

						ImGui::SameLine();
						if (ImGui::Button("Open In Viewer"))
							RunCommandLine(false, "GigiViewerDX12.exe -load \"%s\"", techniquePath.string().c_str());
						ImGui::SameLine();
					}

					{
						bool pendingDownload = m_pendingDownloads.count(GetSummaryHashString(technique.Summary)) > 0;

						if (pendingDownload)
							ImGui::BeginDisabled();

						if (ImGui::Button(pendingDownload ? "Cleaning..." : "Clean"))
							m_workerThreads.Add<Job_CleanTechnique>(technique);

						if (pendingDownload)
							ImGui::EndDisabled();
						ImGui::SameLine();
					}

					if (ImGui::Button("Open Folder"))
					{
						techniquePath.replace_filename("");
						ShellExecuteA(NULL, "explore", techniquePath.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
					}
				}

				//////////////////////////////////////////////////////////

				if (!technique.Details.License.empty() && ImGui::Button("License"))
				{
					s_showLicenseWindow = true;
					s_popupWindowTechniqueIndex = searchResultIndex;
				}

				ImGui::SameLine();
				if (!technique.Details.Website.empty() && ImGui::Button("Website"))
				{
					ShellExecuteA(nullptr, nullptr, technique.Details.Website.c_str(), nullptr, nullptr, SW_SHOW);
				}

				ImGui::PopID();
			}

			ImGui::EndTable();
		}

		if (s_showLicenseWindow)
		{
			ImGui::OpenPopup("License");

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImVec2 windowSize = ImGui::GetMainViewport()->Size / 2;
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

			if (ImGui::BeginPopupModal("License", &s_showLicenseWindow))
			{
				const BrowserCachedTechnique& technique = m_searchResults[s_popupWindowTechniqueIndex];

				std::filesystem::path rootPath = (m_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(technique.Summary) / technique.Summary.DetailsFile).replace_filename("");
				std::filesystem::path licensePath = rootPath / technique.Details.License;

				FileCache::File& file = m_fileCache.Get(licensePath.string().c_str());

				if (file.Valid())
					ImGui::TextWrapped("%s", file.GetBytes());
				else
					ImGui::TextWrapped("Could not load license file: %s", licensePath.string().c_str());

				ImGui::EndPopup();
			}
		}

		if (s_showScreenshotWindow)
		{
			ImGui::OpenPopup("Screenshot");

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal("Screenshot", &s_showScreenshotWindow, ImGuiWindowFlags_AlwaysAutoResize))
			{
				const BrowserCachedTechnique& technique = m_searchResults[s_popupWindowTechniqueIndex];

				std::filesystem::path rootPath = (m_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(technique.Summary) / technique.Summary.DetailsFile).replace_filename("");
				std::filesystem::path screenshotPath = rootPath / technique.Details.Screenshot;

				int imageW = 0;
				int imageH = 0;
				void* descriptorTable = GetDescriptorTableForImage(screenshotPath.string().c_str(), imageW, imageH);

				ImGui::Image((ImTextureID)descriptorTable, ImVec2((float)imageW, (float)imageH));

				ImGui::EndPopup();
			}
		}
	}

	ImGui::EndChild();

	ImGui::End();
}

void Browser::ShowLogWindow()
{
	if (!g_logOpen)
		return;

	if (!ImGui::Begin("Log"))
	{
		ImGui::End();
		return;
	}

	for (const LogLine& line : m_log)
	{
		if (line.error)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));

		ImGui::TextUnformatted(line.text.c_str());

		if (line.error)
			ImGui::PopStyleColor();

		// Autoscroll
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
	}

	ImGui::End();
}

void Browser::Display()
{
	m_SRVHeapAllocationTracker_imgui.OnNewFrame();
	m_descriptorTableCache_imgui.OnNewFrame(m_SRVHeapAllocationTracker_imgui);
	m_uploadBufferTracker.OnNewFrame(m_maxFramesInFlight);

	TickWorkerThreads();

	MakeInitialLayout();

	ShowLogWindow();
	ShowBrowserWindow();
}