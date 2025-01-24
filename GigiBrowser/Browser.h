///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <filesystem>

#include "ThreadedWork.h"
#include "Database.h"
#include "Schemas/Types.h"
#include "GigiViewerDX12/DX12Utils/TextureCache.h"
#include "GigiViewerDX12/DX12Utils/FileCache.h"
#include "GigiViewerDX12/DX12Utils/HeapAllocationTracker.h"
#include "GigiViewerDX12/DX12Utils/DescriptorTableCache.h"
#include "GigiViewerDX12/DX12Utils/UploadBufferTracker.h"

class Browser
{
public:
	Browser()
	{
		s_singularBrowser = this;
	}

	~Browser()
	{
		while(m_workerThreads.HasPendingWork())
			TickWorkerThreads();
		m_workerThreads.Join();

		Database::Close();
		s_singularBrowser = nullptr;
	}

	bool Init(ID3D12Device* device, int maxFramesInFlight, ID3D12DescriptorHeap* ImGuiSRVHeap, int ImGuiSRVHeapDescriptorCount, int ImGuiSRVHeapDescriptorSize, int argc, char** argv);
	void Release();
	void ProcessPendingTextureLoads(ID3D12GraphicsCommandList* commandList);
	void Display();

	enum class DownloadedStatus
	{
		Any,
		Downloaded,
		NotDownloaded
	};

	// Static interface
public:
	static void Log(bool error, const char* fmt, ...);

	static const std::unordered_set<std::string>& GetTechniquesAlreadyDownloaded() { return s_singularBrowser->m_techniquesThatWereAlreadyDownloaded; }

	static void SetRefreshSearchResults() { s_singularBrowser->m_refreshSearchResults = true; }

	static void PendingDownload(const std::string& summaryHash, bool starting)
	{
		if (starting)
			s_singularBrowser->m_pendingDownloads.insert(summaryHash);
		else
			s_singularBrowser->m_pendingDownloads.erase(summaryHash);
	}

private:
	void MakeInitialLayout();
	void ShowBrowserWindow();
	void ShowLogWindow();

	void* GetDescriptorTableForImage(const char* path, int& width, int& height);

	void GetSearchResults(bool refreshingResults);

	void TickWorkerThreads();

private:
	struct LogLine
	{
		bool error = false;
		std::string text;
	};

	// DX12 stuff
	int m_maxFramesInFlight = 1;
	ID3D12Device* m_device = nullptr;

	// Loading textures off disk
	FileCache m_fileCache;
	TextureCache m_textureCache;
	std::unordered_map<std::string, ID3D12Resource*> m_textureResources;
	std::unordered_set<std::string> m_pendingTextureLoads;

	// ImGui SRV heap
	DescriptorTableCache m_descriptorTableCache_imgui;
	HeapAllocationTracker m_SRVHeapAllocationTracker_imgui;
	UploadBufferTracker m_uploadBufferTracker;

	// UI
	bool m_resetLayout = true;
	std::vector<LogLine> m_log;

	// So that things can log without needing the browser object
	static inline Browser* s_singularBrowser = nullptr;

	// The list of results being shown to the user, for the search params
	std::vector<BrowserCachedTechnique> m_searchResults;

	// The list of servers you can search for. A union of servers in the server list text file, and the database.
	std::vector<std::string> m_searchServers;

	// Search params
	int m_search_Server = 0;
	std::string m_search_Text;
	std::string m_search_Tags;
	DownloadedStatus m_search_Downloaded = DownloadedStatus::Any;

	int m_lastSearch_Server = 0;
	std::string m_lastSearch_Text;
	std::string m_lastSearch_Tags;
	DownloadedStatus m_lastSearch_Downloaded = DownloadedStatus::Any;

	bool m_refreshSearchResults = false;
	int m_search_Page = 0;

	static inline const int c_resultsPerPage = 5;

	// Used when upgrading the database
	std::unordered_set<std::string> m_techniquesThatWereAlreadyDownloaded;

	// Used when techniques are being downloaded
	std::unordered_set<std::string> m_pendingDownloads;

	// For doing work off the main thread and getting the results on the main thread
	WorkerThreads m_workerThreads;

	// Where the cache directory lives
	std::filesystem::path m_cacheDir;
};
