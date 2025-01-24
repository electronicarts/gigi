///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Jobs.h"

#include "Browser.h"
#include "Utils.h"
#include "Schemas/JSON.h"
#include "Database.h"

Job_CleanTechnique::Job_CleanTechnique(const BrowserCachedTechnique& techniqueInfo)
	: m_techniqueInfo(techniqueInfo)
{
	Browser::PendingDownload(GetSummaryHashString(m_techniqueInfo.Summary), true);
}

void Job_CleanTechnique::ExecuteMainThread()
{
	if (!m_info.str().empty())
		Browser::Log(false, "%s", m_info.str().c_str());

	if (!m_errors.str().empty())
		Browser::Log(true, "%s", m_info.str().c_str());

	Browser::SetRefreshSearchResults();

	Browser::PendingDownload(GetSummaryHashString(m_techniqueInfo.Summary), false);

	Browser::Log(false, "[%s] Technique \"%s\" cleaned", m_techniqueInfo.Origin.Name.c_str(), m_techniqueInfo.Details.Title.c_str());
}

Job_DownloadTechnique::Job_DownloadTechnique(const BrowserCachedTechnique& techniqueInfo)
	: m_techniqueInfo(techniqueInfo)
{
	Browser::PendingDownload(GetSummaryHashString(m_techniqueInfo.Summary), true);

	Browser::Log(false, "[%s] Downloading technique \"%s\"", m_techniqueInfo.Origin.Name.c_str(), m_techniqueInfo.Details.Title.c_str());
}

void Job_DownloadTechnique::ExecuteMainThread()
{
	if (!m_info.str().empty())
		Browser::Log(false, "%s", m_info.str().c_str());

	if (!m_errors.str().empty())
		Browser::Log(true, "%s", m_info.str().c_str());

	Browser::SetRefreshSearchResults();

	Browser::PendingDownload(GetSummaryHashString(m_techniqueInfo.Summary), false);

	Browser::Log(false, "[%s] Technique \"%s\" downloaded", m_techniqueInfo.Origin.Name.c_str(), m_techniqueInfo.Details.Title.c_str());
}

Job_AddTechniqueMainThread::Job_AddTechniqueMainThread(const BrowserCachedTechnique& techniqueInfo)
	: m_techniqueInfo(techniqueInfo)
{

}

void Job_AddTechniqueMainThread::ExecuteMainThread()
{
	// Insert this new record into the database.
	// If we updated the database schema, it could already be downloaded, so mark it that way if needed.
	bool downloaded = Browser::GetTechniquesAlreadyDownloaded().count(GetSummaryHashString(m_techniqueInfo.Summary)) > 0;
	std::stringstream query;
	query << "insert into Techniques ("
		"SummaryHash,"
		"Summary_Repo, Summary_Commit, Summary_DetailsFile, Summary_ListedDate,"
		"Details_Title, Details_Description, Details_Author, Details_Tags, Details_Technique, Details_Website, Details_Screenshot, Details_License, Details_GigiVersion,"
		"Downloaded,"
		"Origin_Name, Origin_Repo, Origin_Branch, Origin_TechniqueList )"
		"values ("
		"?, ?, ?, ?,"
		"?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
		<< (downloaded ? 1 : 0) << ","
		"?, ?, ?, ?)"
		;

	std::vector<std::string> params;

	params.push_back(GetSummaryHashString(m_techniqueInfo.Summary));

	params.push_back(m_techniqueInfo.Summary.Repo);
	params.push_back(m_techniqueInfo.Summary.Commit);
	params.push_back(m_techniqueInfo.Summary.DetailsFile);
	params.push_back(m_techniqueInfo.Summary.ListedDate);

	params.push_back(m_techniqueInfo.Details.Title);
	params.push_back(m_techniqueInfo.Details.Description);
	params.push_back(m_techniqueInfo.Details.Author);
	params.push_back(m_techniqueInfo.Details.Tags);
	params.push_back(m_techniqueInfo.Details.Technique);
	params.push_back(m_techniqueInfo.Details.Website);
	params.push_back(m_techniqueInfo.Details.Screenshot);
	params.push_back(m_techniqueInfo.Details.License);
	params.push_back(m_techniqueInfo.Details.GigiVersion);

	params.push_back(m_techniqueInfo.Origin.Name);
	params.push_back(m_techniqueInfo.Origin.Repo);
	params.push_back(m_techniqueInfo.Origin.Branch);
	params.push_back(m_techniqueInfo.Origin.TechniqueList);

	if (!Database::ExecuteSQL(query.str().c_str(), params))
		Browser::Log(true, "Could not insert into Techniques table");

	Browser::SetRefreshSearchResults();
}

Job_UpdateTechnique::Job_UpdateTechnique(const BrowserServerInfo& serverInfo, const BrowserTechniqueSummary& summary)
	: m_serverInfo(serverInfo)
	, m_summary(summary)
{

}

void Job_UpdateTechnique::ExecuteMainThread()
{
	if (!m_info.str().empty())
		Browser::Log(false, "%s", m_info.str().c_str());

	if (!m_errors.str().empty())
		Browser::Log(true, "%s", m_info.str().c_str());
}

Job_UpdateServer::Job_UpdateServer(const BrowserServerInfo& serverInfo)
	: m_serverInfo(serverInfo)
{
	Browser::Log(false, "[%s] Updating technique list", m_serverInfo.Name.c_str());
}

void Job_UpdateServer::ExecuteMainThread()
{
	if (!m_info.str().empty())
		Browser::Log(false, "%s", m_info.str().c_str());

	if (!m_errors.str().empty())
		Browser::Log(true, "%s", m_errors.str().c_str());

	Browser::Log(false, "[%s] Technique list updated", m_serverInfo.Name.c_str());
}
