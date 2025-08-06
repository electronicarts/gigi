///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ThreadedWork.h"
#include "Schemas/Types.h"
#include <sstream>

// Removes any local changes and returns it to a pristine state.
// Also does a pull and updates the details file / database, for techniques with a branch instead of a hash for a commit.
class Job_CleanTechnique : public JobBase
{
public:
	Job_CleanTechnique(const BrowserCachedTechnique& techniqueInfo);
	void ExecuteWorkerThread(WorkerThreads& workerThreads) override final;
	void ExecuteMainThread() override final;

private:
	BrowserCachedTechnique m_techniqueInfo;
	std::stringstream m_errors;
	std::stringstream m_info;
};

// Removes a technique from both the disk and database, whether it's downloaded or not (minimal download).
class Job_DeleteTechnique : public JobBase
{
public:
	Job_DeleteTechnique(const BrowserCachedTechnique& techniqueInfo);
	void ExecuteWorkerThread(WorkerThreads& workerThreads) override final {}
	void ExecuteMainThread() override final;

private:
	BrowserCachedTechnique m_techniqueInfo;
};

// Changes the sparse checkout from being the minimum, to all files needed.
// Also updates database to remember that it's been downloaded.
class Job_DownloadTechnique : public JobBase
{
public:
	Job_DownloadTechnique(const BrowserCachedTechnique& techniqueInfo);
	void ExecuteWorkerThread(WorkerThreads& workerThreads) override final;
	void ExecuteMainThread() override final;

private:
	BrowserCachedTechnique m_techniqueInfo;
	std::stringstream m_errors;
	std::stringstream m_info;
};

// Adds a technique to the database.
// Stores whether it's been downloaded yet or not, as this may be happening due to a database version upgrade.
class Job_AddTechniqueMainThread : public JobBase
{
public:
	Job_AddTechniqueMainThread(const BrowserCachedTechnique& techniqueInfo);
	void ExecuteWorkerThread(WorkerThreads& workerThreads) override final;
	void ExecuteMainThread() override final;

private:
	BrowserCachedTechnique m_techniqueInfo;
};

// Gets the minimum info about a technique downloaded (details, license, screenshot) and adds it to the database.
class Job_UpdateTechnique : public JobBase
{
public:
	Job_UpdateTechnique(const BrowserServerInfo& serverInfo, const BrowserTechniqueSummary& summary);
	void ExecuteWorkerThread(WorkerThreads& workerThreads) override final;
	void ExecuteMainThread() override final;

private:
	BrowserServerInfo m_serverInfo;
	BrowserTechniqueSummary m_summary;
	std::stringstream m_errors;
	std::stringstream m_info;
};

// Gets the latest list of techniques from the server, and kicks off a Job_UpdateTechnique per technique that is not in the database
class Job_UpdateServer : public JobBase
{
public:
	Job_UpdateServer(const BrowserServerInfo& serverInfo);
	void ExecuteWorkerThread(WorkerThreads& workerThreads) override final;
	void ExecuteMainThread() override final;

private:
	BrowserServerInfo m_serverInfo;
	std::stringstream m_errors;
	std::stringstream m_info;
};
