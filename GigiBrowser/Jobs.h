///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ThreadedWork.h"
#include "Schemas/Types.h"
#include <sstream>

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

class Job_AddTechniqueMainThread : public JobBase
{
public:
	Job_AddTechniqueMainThread(const BrowserCachedTechnique& techniqueInfo);
	void ExecuteWorkerThread(WorkerThreads& workerThreads) override final;
	void ExecuteMainThread() override final;

private:
	BrowserCachedTechnique m_techniqueInfo;
};

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
