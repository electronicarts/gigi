///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Jobs.h"

#include "Utils.h"
#include "Database.h"

// For best results - only thread safe operations in here, and visibility of thread safe operations.

static bool UpdateRepo(const char* path, const char* repoURL, const char* checkout, const char* sparseFileList, bool areDirectories)
{
	// Make sure the directory exists
	bool ret = true;
	std::filesystem::create_directories(path);

	// Switch to the new directory
	std::filesystem::path gitDir = std::filesystem::current_path() / path;

	// If the repo already exists, we just need to do a pull
	if (std::filesystem::exists(gitDir / ".git/"))
	{
		// Configure sparse checkout
		RunCommandLine(true, "external/git/cmd/git -C \"%s\" sparse-checkout set%s %s", gitDir.string().c_str(), areDirectories ? "" : " --no-cone", sparseFileList);

		// Pull
		RunCommandLine(true, "external/git/cmd/git -C \"%s\" pull", gitDir.string().c_str());
	}
	// else we need to set up the repo
	else
	{
		// Do a clone without checkout of depth 1
		RunCommandLine(true, "external/git/cmd/git -C \"%s\" clone --depth 1 --filter=blob:none --no-checkout %s .", gitDir.string().c_str(), repoURL);

		// Configure sparse checkout
		RunCommandLine(true, "external/git/cmd/git -C \"%s\" sparse-checkout set%s %s", gitDir.string().c_str(), areDirectories ? "" : " --no-cone", sparseFileList);

		// Do the check out
		RunCommandLine(true, "external/git/cmd/git -C \"%s\" checkout %s", gitDir.string().c_str(), checkout);
	}

	return ret;
}

static bool CleanRepo(const char* path)
{
	// Make sure the directory exists
	bool ret = true;

	// Switch to the new directory
	std::filesystem::path gitDir = std::filesystem::current_path() / path;

	// Reset
	RunCommandLine(true, "external/git/cmd/git -C \"%s\" reset --hard", gitDir.string().c_str());

	// Clean
	RunCommandLine(true, "external/git/cmd/git -C \"%s\" clean -xfd", gitDir.string().c_str());

	return ret;
}

void Job_CleanTechnique::ExecuteWorkerThread(WorkerThreads& workerThreads)
{
	std::filesystem::path techniquePath = s_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(m_techniqueInfo.Summary);
	CleanRepo(techniquePath.string().c_str());
}

void Job_DownloadTechnique::ExecuteWorkerThread(WorkerThreads& workerThreads)
{
	std::filesystem::path techniquePath = s_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(m_techniqueInfo.Summary);
	std::filesystem::path relativeRoot = std::filesystem::path(m_techniqueInfo.Summary.DetailsFile).replace_filename("");
	if (!UpdateRepo(techniquePath.string().c_str(), m_techniqueInfo.Summary.Repo.c_str(), m_techniqueInfo.Summary.Commit.c_str(), relativeRoot.string().c_str(), true))
	{
		m_errors << "Error downloading technique \"" << m_techniqueInfo.Summary.Commit << "\"";
		return;
	}

	std::vector<std::string> params;
	params.push_back(GetSummaryHashString(m_techniqueInfo.Summary));
	if (!Database::ExecuteSQL("update Techniques set Downloaded = 1 where SummaryHash = ?", params))
		m_errors << "Could not update database for technique \"" << m_techniqueInfo.Summary.Commit << "\"";
}

void Job_AddTechniqueMainThread::ExecuteWorkerThread(WorkerThreads& workerThreads)
{

}

void Job_UpdateTechnique::ExecuteWorkerThread(WorkerThreads& workerThreads)
{
	// If we are upgrading database schema, the .git folder will already exist and be in the correct state.
	// This is true whether the technique has been downloaded or not
	std::filesystem::path techniquePath = s_cacheDir / std::filesystem::path("Techniques") / GetSummaryHashString(m_summary);
	bool gitAlreadyExists = std::filesystem::exists(techniquePath / ".git/");

	// Get the details file if we don't already have it
	if (!gitAlreadyExists)
	{
		if (!UpdateRepo(techniquePath.string().c_str(), m_summary.Repo.c_str(), m_summary.Commit.c_str(), m_summary.DetailsFile.c_str(), false))
		{
			m_errors << "Error getting technique \"" << m_summary.DetailsFile << "\" from server \"" << m_serverInfo.Name << "\"\n";
			return;
		}
	}

	BrowserCachedTechnique newTechnique;
	newTechnique.Origin = m_serverInfo;
	newTechnique.Summary = m_summary;

	std::filesystem::path detailsFilePath = techniquePath / m_summary.DetailsFile;
	if (!ReadFromJSONFile(newTechnique.Details, detailsFilePath.string().c_str()))
	{
		m_errors << "Error reading json file for technique \"" << m_summary.DetailsFile << "\" from server \"" << m_serverInfo.Name << "\"";
		return;
	}

	// Get the license file and screenshot too if we don't already have them
	if (!gitAlreadyExists)
	{
		std::filesystem::path relativeRoot = std::filesystem::path(m_summary.DetailsFile).replace_filename("");
		std::string screenshotRelative = (relativeRoot / newTechnique.Details.Screenshot).string();

		std::string allowedFiles = m_summary.DetailsFile + " " + screenshotRelative;
		if (!newTechnique.Details.License.empty())
			allowedFiles = allowedFiles + " " + (relativeRoot / newTechnique.Details.License).string();
		if (!UpdateRepo(techniquePath.string().c_str(), m_summary.Repo.c_str(), m_summary.Commit.c_str(), allowedFiles.c_str(), false))
		{
			m_errors << "Error getting technique \"" << m_summary.DetailsFile << "\" from server \"" << m_serverInfo.Name << "\"";
			return;
		}

		m_info << "[" << m_serverInfo.Name << "] New technique listed: \"" << newTechnique.Details.Title << "\"";
	}

	workerThreads.Add<Job_AddTechniqueMainThread>(newTechnique);
}

void Job_UpdateServer::ExecuteWorkerThread(WorkerThreads& workerThreads)
{
	// Update the repo for this server
	std::filesystem::path serverPath = s_cacheDir / std::filesystem::path("Servers") / m_serverInfo.Name;
	if (!UpdateRepo(serverPath.string().c_str(), m_serverInfo.Repo.c_str(), m_serverInfo.Branch.c_str(), m_serverInfo.TechniqueList.c_str(), false))
	{
		m_errors << "Error getting info for server \"" << m_serverInfo.Name << "\"\n";
		return;
	}

	// Parse the Techniques.txt file
	{
		std::filesystem::path techniqueList = serverPath / m_serverInfo.TechniqueList;

		// If there was an error, show it, but we can use whatever data we already have in the database
		BrowserTechniqueSummaries techniques;
		if (!ReadFromJSONFile(techniques, techniqueList.string().c_str()))
		{
			m_errors << "Could not read technique list for server \"" << m_serverInfo.Name << "\"\n";
			return;
		}

		// Add any techniques not already in the cache
		for (const BrowserTechniqueSummary& summary : techniques.Techniques)
		{
			// if this technique is already in the database, don't do anything
			bool alreadyExists = false;
			std::vector<std::string> params;
			params.push_back(GetSummaryHashString(summary));
			if (!Database::ExecuteSQL("select count(1) from Techniques where SummaryHash = ?", params,
				[&alreadyExists](sqlite3_stmt* statement)
				{
					alreadyExists = (sqlite3_column_int(statement, 0) > 0);
				}
			))
			{
				m_errors << "Could not get count of techniques";
				continue;
			}
			if (alreadyExists)
				continue;

			// Otherwise, add a job to update this technique
			workerThreads.Add<Job_UpdateTechnique>(m_serverInfo, summary);
		}
	}
}
