///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <filesystem>

template <typename T>
class ThreadSafeQueue {
public:
    ~ThreadSafeQueue()
    {
    }

    void End()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_exit = true;
        m_cond.notify_all();
    }

    void Push(const T& message)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(message);
        m_cond.notify_one();
    }

    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        while (1)
        {
            if (m_exit)
            {
                return T{};
            }

            m_cond.wait(lock);

            if (!m_queue.empty())
            {
                T message = m_queue.front();
                m_queue.pop();
                return message;

            }
        }
    }

    T TryPop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return T{};

        T message = m_queue.front();
        m_queue.pop();
        return message;
    }

    size_t Count()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic_bool m_exit = false;
};

class JobBase
{
public:
    virtual void ExecuteWorkerThread(class WorkerThreads& workerThreads) = 0;
    virtual void ExecuteMainThread() = 0;

    static inline std::filesystem::path s_cacheDir;
};

class WorkerThreads
{
public:
    WorkerThreads()
    {
        s_workerThreads = this;

        int numThreads = std::max<unsigned int>(std::thread::hardware_concurrency() - 2, 1);
        m_threads.resize(numThreads);
        for (int i = 0; i < numThreads; ++i)
            m_threads[i] = std::thread(WorkerThreads::StaticThreadFunction);
    }

    // Call from main thread
    void Join()
    {
        m_jobsPending.End();
        m_jobsFinished.End();
        m_shutdown = true;
        for (std::thread& thread : m_threads)
            thread.join();
    }

    // Call from any thread
    template <typename T, typename... Args>
    void Add(Args&&... args)
    {
        m_jobsInFlight++;
        m_jobsPending.Push(new T(std::forward<Args>(args)...));
    }

    // Call from any thread
    bool HasPendingWork()
    {
        return m_jobsInFlight > 0;
    }

    int JobsInFlight()
    {
        return m_jobsInFlight;
    }

    // Call from main thread
    void ProcessFinishedJobs()
    {
        while (1)
        {
            JobBase* job = m_jobsFinished.TryPop();
            if (!job)
                return;

            job->ExecuteMainThread();
            delete job;
            m_jobsInFlight--;
        }
    }

private:
    static void StaticThreadFunction()
    {
        s_workerThreads->ThreadFunction();
    }

    void ThreadFunction()
    {
        while (!m_shutdown)
        {
            JobBase* job = m_jobsPending.Pop();
            if (!job)
                continue;

            job->ExecuteWorkerThread(*this);
            m_jobsFinished.Push(job);
        }
    }

private:
    std::atomic<bool> m_shutdown = false;
    std::atomic<int> m_jobsInFlight = 0;

    ThreadSafeQueue<JobBase*> m_jobsPending;
    ThreadSafeQueue<JobBase*> m_jobsFinished;

    std::vector<std::thread> m_threads;

    // The singular worker threads object
    inline static WorkerThreads* s_workerThreads = nullptr;
};
